#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_INA219.h>

#include "config.h"
#include "thermal.h"
#include "tec_driver.h"
#include "pid_controller.h"
#include "ble_service.h"
#include "session.h"

// ============================================================================
// BackPal ESP32 Controller — Main Application
// ONYX Bethlehem Labs + SSTA IP Holdings
// Firmware v3.0.0
//
// Architecture:
//   - 10Hz control loop: read sensors → PID compute → drive TEC
//   - 1Hz telemetry: BLE notify + serial CSV output
//   - Thermal-state-aware control via vault SOC & melt fraction
//   - TEMP_THEN_HOLD therapeutic timer
//   - Safety limits enforced every control cycle
// ============================================================================

// --- Global Objects ---
TECDriver       tec;
PIDController   pid;
BLEService      ble;
SessionManager  session;
Adafruit_INA219 ina219(INA219_ADDR);

// --- State ---
SensorReadings  sensors;
ThermalState    vaultState;
RuntimeEstimate runtime;

bool     manualMode = true;     // true = app controls drive directly; false = PID
bool     systemFault = false;
uint8_t  safetyFlags = 0;
float    currentAmps = 0.0f;
float    busVoltage = 0.0f;
float    powerWatts = 0.0f;
float    batteryPct = 100.0f;

unsigned long lastControlMs = 0;
unsigned long lastTelemetryMs = 0;
unsigned long lastButtonMs = 0;

// --- LED Helpers ---

void setLED(bool r, bool g, bool b) {
    digitalWrite(PIN_LED_R, r ? HIGH : LOW);
    digitalWrite(PIN_LED_G, g ? HIGH : LOW);
    digitalWrite(PIN_LED_B, b ? HIGH : LOW);
}

void updateLED() {
    if (systemFault) {
        // Fast blink red on fault
        setLED((millis() / 200) % 2, false, false);
        return;
    }

    switch (tec.getMode()) {
        case TEC_COOL: setLED(false, false, true);  break;  // Blue
        case TEC_HEAT: setLED(true, false, false);   break;  // Red
        default:       setLED(false, true, false);   break;  // Green = idle
    }

    // Blink pattern when BLE connected
    if (ble.isConnected() && (millis() / 1000) % 3 == 0) {
        setLED(false, false, false);  // Brief off-blink every 3s
    }
}

// --- Safety Check ---

bool checkSafety() {
    safetyFlags = 0;

    // T1 over-temperature (skin safety — hard limit)
    if (sensors.valid[0] && sensors.temps[0] > SAFETY_MAX_SKIN_C) {
        safetyFlags |= 0x01;
    }

    // T1 under-temperature
    if (sensors.valid[0] && sensors.temps[0] < SAFETY_MIN_SKIN_C) {
        safetyFlags |= 0x02;
    }

    // Overcurrent
    if (currentAmps > SAFETY_MAX_CURRENT_A) {
        safetyFlags |= 0x04;
    }

    // T2 TEC face over-temperature
    if (sensors.valid[1] && sensors.temps[1] > SAFETY_MAX_TEC_C) {
        safetyFlags |= 0x08;
    }

    if (safetyFlags != 0) {
        tec.emergencyStop();
        systemFault = true;
        Serial.printf("[SAFETY] Fault! flags=0x%02X T1=%.1f T2=%.1f I=%.2fA\n",
                      safetyFlags, sensors.temps[0], sensors.temps[1], currentAmps);
        return false;
    }

    // Auto-clear fault if conditions return to safe range
    if (systemFault) {
        // Require all temps in safe range to clear
        bool safe = true;
        if (sensors.valid[0]) {
            safe &= (sensors.temps[0] < SAFETY_MAX_SKIN_C - 2.0f);
            safe &= (sensors.temps[0] > SAFETY_MIN_SKIN_C + 2.0f);
        }
        if (sensors.valid[1]) {
            safe &= (sensors.temps[1] < SAFETY_MAX_TEC_C - 5.0f);
        }
        if (safe && currentAmps < SAFETY_MAX_CURRENT_A * 0.8f) {
            systemFault = false;
            Serial.println("[SAFETY] Fault cleared");
        }
    }

    return true;
}

// --- Button Handler ---

void handleButton() {
    if (digitalRead(PIN_BUTTON) == LOW) {
        if (millis() - lastButtonMs > 300) {  // Debounce
            lastButtonMs = millis();

            // Cycle: OFF → COOL → HEAT → OFF
            switch (tec.getMode()) {
                case TEC_OFF:
                    tec.setMode(TEC_COOL);
                    tec.setDrive(128);  // 50% default
                    pid.setSetpoint(PROFILE_COOL_TARGET_C);
                    manualMode = false;
                    Serial.println("[BTN] COOL mode");
                    break;
                case TEC_COOL:
                    pid.reset();
                    tec.setMode(TEC_HEAT);
                    pid.setSetpoint(PROFILE_HEAT_TARGET_C);
                    Serial.println("[BTN] HEAT mode");
                    break;
                case TEC_HEAT:
                    pid.reset();
                    tec.setMode(TEC_OFF);
                    manualMode = true;
                    Serial.println("[BTN] OFF");
                    break;
            }
        }
    }
}

// --- BLE Command Processing ---

void processBLECommand() {
    if (!ble.hasNewCommand()) return;

    BLEControlCmd cmd = ble.getCommand();

    switch (cmd.command) {
        case 0:  // STOP
            tec.emergencyStop();
            session.stop();
            pid.reset();
            manualMode = true;
            Serial.println("[BLE] STOP");
            break;

        case 1:  // COOL
            pid.reset();
            tec.setMode(TEC_COOL);
            if (cmd.setpoint > 0.0f && cmd.setpoint < 50.0f) {
                pid.setSetpoint(cmd.setpoint);
                manualMode = false;
            } else {
                tec.setDrive(cmd.power);
                manualMode = true;
            }
            Serial.printf("[BLE] COOL sp=%.1f pwr=%d\n", cmd.setpoint, cmd.power);
            break;

        case 2:  // HEAT
            pid.reset();
            tec.setMode(TEC_HEAT);
            if (cmd.setpoint > 0.0f && cmd.setpoint < 50.0f) {
                pid.setSetpoint(cmd.setpoint);
                manualMode = false;
            } else {
                tec.setDrive(cmd.power);
                manualMode = true;
            }
            Serial.printf("[BLE] HEAT sp=%.1f pwr=%d\n", cmd.setpoint, cmd.power);
            break;

        case 3:  // PROFILE (clinical)
            pid.reset();
            session.loadClinicalProfile();
            session.start();
            manualMode = false;
            Serial.println("[BLE] PROFILE clinical");
            break;
    }
}

void processBLEPID() {
    if (!ble.hasNewPIDGains()) return;

    BLEPIDPacket update = ble.getPIDUpdate();

    if (update.Kp >= 0.0f && update.Ki >= 0.0f) {
        pid.setGains(update.Kp, update.Ki, update.Kd);
        if (update.setpoint > 0.0f && update.setpoint < 50.0f) {
            pid.setSetpoint(update.setpoint);
        }
        Serial.printf("[BLE] PID Kp=%.2f Ki=%.2f Kd=%.2f sp=%.1f\n",
                      update.Kp, update.Ki, update.Kd, update.setpoint);
    }
}

// --- Read Battery Voltage ---
// Note: GPIO4 is ADC2 — only reliable when BLE TX is idle.
// Use INA219 bus voltage as more reliable proxy when available.

void readBattery() {
    // Prefer INA219 bus voltage if available
    float vBus = busVoltage;
    if (vBus > 1.0f) {
        // INA219 reads voltage at the load side
        // Approximate battery voltage (close enough for SOC)
        float pct = (vBus - BATTERY_EMPTY_V) / (BATTERY_FULL_V - BATTERY_EMPTY_V) * 100.0f;
        if (pct > 100.0f) pct = 100.0f;
        if (pct < 0.0f) pct = 0.0f;
        batteryPct = pct;
    }
}

// --- Serial CSV Output (1Hz) ---

void outputCSV() {
    // SSTA canonical long-format CSV: timestamp, T1–T6, mode, drive, current, power,
    // setpoint, meltFrac, vaultSOC, phase, therapeuticSec, sessionSec
    Serial.printf("%lu,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%d,%d,%.3f,%.2f,%.2f,%.3f,%.3f,%d,%d,%d\n",
        sensors.timestamp,
        sensors.temps[0], sensors.temps[1], sensors.temps[2],
        sensors.temps[3], sensors.temps[4], sensors.temps[5],
        (int)tec.getMode(),
        (int)tec.getDrive(),
        currentAmps,
        busVoltage,
        powerWatts,
        pid.getSetpoint(),
        vaultState.meltFraction,
        vaultState.vaultSOC > 0.0f ? (int)(vaultState.vaultSOC * 100) : 0,
        session.getTherapeuticSec(),
        session.getSessionElapsedSec()
    );
}

// --- Setup ---

void setup() {
    Serial.begin(115200);
    Serial.println();
    Serial.println("=== BackPal ESP32 Controller v" FIRMWARE_VERSION " ===");
    Serial.println("ONYX Bethlehem Labs + SSTA IP Holdings");
    Serial.println();

    // GPIO setup
    pinMode(PIN_BUTTON, INPUT_PULLUP);
    pinMode(PIN_LED_R, OUTPUT);
    pinMode(PIN_LED_G, OUTPUT);
    pinMode(PIN_LED_B, OUTPUT);
    setLED(false, true, false);  // Green on startup

    // Initialize subsystems
    thermal_init();
    tec.init();
    Wire.begin();

    if (!ina219.begin()) {
        Serial.println("[WARN] INA219 not found — current sensing disabled");
    } else {
        ina219.setCalibration_32V_2A();
        Serial.println("[OK] INA219 initialized");
    }

    // Default PID gains for COOL mode
    pid.setGains(PID_KP_COOL, PID_KI_COOL, PID_KD_COOL);
    pid.setSetpoint(24.0f);  // Room temp = idle

    // Initialize BLE last (takes ~200ms)
    ble.init();
    Serial.println("[OK] BLE advertising as " DEVICE_NAME);

    // CSV header
    Serial.println("ms,T1,T2,T3,T4,T5,T6,mode,drive,I_A,V_bus,P_W,setpoint,meltFrac,SOC%,therapSec,sessSec");

    Serial.println("[OK] System ready");
}

// --- Main Loop ---

void loop() {
    unsigned long now = millis();

    // --- 10Hz Control Loop ---
    if (now - lastControlMs >= CONTROL_PERIOD_MS) {
        lastControlMs = now;

        // 1. Read all sensors
        sensors = thermal_read();

        // 2. Read INA219 (if available)
        currentAmps = ina219.getCurrent_mA() / 1000.0f;
        busVoltage = ina219.getBusVoltage_V();
        powerWatts = currentAmps * busVoltage;
        if (currentAmps < 0) currentAmps = -currentAmps;  // Absolute value

        // 3. Estimate vault thermal state from T4 and T5
        vaultState = thermal_estimate_state(sensors.temps[3], sensors.temps[4]);

        // 4. Safety check (every cycle)
        if (!checkSafety()) {
            // Fault state — TEC already stopped by checkSafety()
            updateLED();
            return;
        }

        // 5. Process BLE commands
        processBLECommand();
        processBLEPID();

        // 6. Session manager update (if profile running)
        if (session.getState() == SESSION_RUNNING) {
            float target = session.update(sensors.temps[0]);  // T1 for TEMP_THEN_HOLD
            pid.setSetpoint(target);

            TECMode phaseMode = session.getCurrentMode();
            if (phaseMode != tec.getMode()) {
                pid.reset();
                tec.setMode(phaseMode);

                // Switch gain sets on mode change
                if (phaseMode == TEC_HEAT) {
                    pid.setGains(PID_KP_HEAT, PID_KI_HEAT, PID_KD_HEAT);
                } else {
                    pid.setGains(PID_KP_COOL, PID_KI_COOL, PID_KD_COOL);
                }
            }
            manualMode = false;
        }

        if (session.getState() == SESSION_COMPLETE) {
            tec.setMode(TEC_OFF);
            pid.reset();
            Serial.println("[SESSION] Complete");
            session.stop();
        }

        // 7. PID control (if not manual mode)
        if (!manualMode && tec.getMode() != TEC_OFF) {
            // Primary feedback: T6 (silicone surface sensor) for prototype
            // In production: T1 with R-value model
            float feedback = sensors.valid[5] ? sensors.temps[5] : sensors.temps[0];
            float output = pid.compute(feedback, vaultState);
            tec.setDrive((uint8_t)output);
        }

        // 8. Runtime estimation
        runtime = pid.estimateRuntime(vaultState, currentAmps, busVoltage);

        // Button handling
        handleButton();

        // LED update
        updateLED();
    }

    // --- 1Hz Telemetry ---
    if (now - lastTelemetryMs >= 1000) {
        lastTelemetryMs = now;

        readBattery();
        outputCSV();

        // BLE notifications
        ble.notifyTemperatures(sensors);

        BLEStatusPacket status;
        status.mode = (uint8_t)tec.getMode();
        status.profileActive = (session.getState() == SESSION_RUNNING) ? 1 : 0;
        status.phaseIndex = session.getCurrentPhaseIndex();
        status.setpoint = pid.getSetpoint();
        status.vaultSOC = vaultState.vaultSOC;
        status.meltFraction = vaultState.meltFraction;
        status.runtimeRemainS = runtime.remainingSeconds;
        status.currentAmps = currentAmps;
        status.powerWatts = powerWatts;
        status.batteryPct = batteryPct;
        status.sessionElapsedS = session.getSessionElapsedSec();
        status.therapeuticS = session.getTherapeuticSec();
        status.safetyFlags = safetyFlags;

        ble.notifyStatus(status);
    }
}
