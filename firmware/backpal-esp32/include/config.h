#ifndef CONFIG_H
#define CONFIG_H

// ============================================================================
// BackPal ESP32 Controller — Configuration
// ONYX Bethlehem Labs + SSTA IP Holdings
// ============================================================================

// --- Device Identity ---
#define DEVICE_NAME          "BackPal-01"
#define FIRMWARE_VERSION     "3.0.0"

// --- Thermistor Configuration ---
// MF55-103F-B3950 NTC thermistors with 10kΩ reference resistors
// Steinhart-Hart via B-parameter model: 1/T = 1/T0 + (1/B)*ln(R/R0)
#define THERMISTOR_B         3950.0f    // B-coefficient (K)
#define THERMISTOR_R0        10000.0f   // Resistance at T0 (Ω)
#define THERMISTOR_T0        298.15f    // T0 = 25°C in Kelvin
#define REFERENCE_R          10000.0f   // Voltage divider reference resistor (Ω)
#define ADC_RESOLUTION       4095.0f    // ESP32 12-bit ADC
#define ADC_VREF             3.3f       // ADC reference voltage

// Thermistor ADC pins (ADC1 only — ADC2 unavailable during BLE)
#define PIN_T1               36   // ADC1_CH0 — Skin-side spreader (safety limit)
#define PIN_T2               39   // ADC1_CH3 — TEC hot face / vault contact
#define PIN_T3               34   // ADC1_CH6 — TEC cold face / heatsink
#define PIN_T4               35   // ADC1_CH7 — Vault center (PCM core)
#define PIN_T5               32   // ADC1_CH4 — Vault edge
#define PIN_T6               33   // ADC1_CH5 — Silicone surface (PID control sensor)
#define NUM_SENSORS          6

// Calibration offsets (°C) from initial equilibrium measurement
#define CAL_OFFSET_T1       -0.03f
#define CAL_OFFSET_T2        0.19f
#define CAL_OFFSET_T3        0.20f
#define CAL_OFFSET_T4       -0.02f
#define CAL_OFFSET_T5       -0.60f
#define CAL_OFFSET_T6        0.00f

// ADC oversampling — average N reads per channel for noise reduction
#define ADC_OVERSAMPLE       16

// --- IBT-2 H-Bridge Configuration ---
#define PIN_RPWM             25   // Forward drive (COOL mode)
#define PIN_LPWM             26   // Reverse drive (HEAT mode)
#define PIN_R_EN             27   // Right enable
#define PIN_L_EN             14   // Left enable

// LEDC PWM channels for constant DC output via analog write
#define LEDC_CH_RPWM         0
#define LEDC_CH_LPWM         1
#define LEDC_FREQ            20000   // 20kHz — above audible, good for IBT-2
#define LEDC_BITS            8       // 0-255 duty range

// --- INA219 Current Sensor ---
#define INA219_ADDR          0x40

// --- User Interface ---
#define PIN_BUTTON           13   // Momentary pushbutton (active LOW, internal pullup)
#define PIN_LED_R            16   // RGB LED red
#define PIN_LED_G            17   // RGB LED green
#define PIN_LED_B            18   // RGB LED blue

// --- Battery Monitoring ---
// 100kΩ + 33kΩ voltage divider: V_adc = V_bat * 33/(100+33) = V_bat * 0.2481
#define PIN_BATTERY          4    // ADC2_CH0 — read during BLE gaps
#define BATTERY_DIVIDER      0.2481f
#define BATTERY_FULL_V       12.6f   // 3S LiPo fully charged
#define BATTERY_EMPTY_V      9.0f    // 3S LiPo cutoff

// --- Safety Limits ---
#define SAFETY_MAX_SKIN_C    45.0f   // T1 hard cutoff (°C)
#define SAFETY_MIN_SKIN_C    5.0f    // T1 minimum (°C)
#define SAFETY_MAX_TEC_C     80.0f   // T2 TEC face limit (°C)
#define SAFETY_MAX_CURRENT_A 6.0f    // INA219 overcurrent limit (A)

// --- Control Loop ---
#define CONTROL_FREQ_HZ      10     // 10Hz control loop
#define CONTROL_PERIOD_MS     (1000 / CONTROL_FREQ_HZ)  // 100ms

// --- PID Defaults (tuned for T6 silicone surface sensor) ---
// COOL mode: TEC pumps heat from skin into vault
#define PID_KP_COOL          15.0f   // Proportional gain
#define PID_KI_COOL          0.8f    // Integral gain
#define PID_KD_COOL          0.0f    // Derivative gain (start with PI only)

// HEAT mode: TEC pumps heat from vault to skin
#define PID_KP_HEAT          12.0f
#define PID_KI_HEAT          0.6f
#define PID_KD_HEAT          0.0f

// Integral windup limits
#define PID_INTEGRAL_MAX      200.0f
#define PID_INTEGRAL_MIN     -200.0f

// Output limits (0-255 for LEDC duty)
#define PID_OUTPUT_MAX        255.0f
#define PID_OUTPUT_MIN        0.0f

// --- Therapeutic Profiles ---
// BackPal Clinical: alternating heat and cool phases
#define PROFILE_HEAT_TARGET_C 40.0f  // HEAT phase skin target (°C)
#define PROFILE_COOL_TARGET_C 9.0f   // COOL phase skin target (°C)
#define PROFILE_TEMP_TOLERANCE 2.0f  // ±°C for TEMP_THEN_HOLD timer

// BackPal Clinical default timing
#define CLINICAL_HEAT_SEC     240    // 4 minutes heat
#define CLINICAL_COOL_SEC     60     // 1 minute cool
#define CLINICAL_CYCLES       4      // 4 heat phases

// --- BLE Configuration ---
#define BLE_SERVICE_UUID              "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define BLE_CHAR_TEMPS_UUID           "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define BLE_CHAR_CONTROL_UUID         "beb5483e-36e1-4688-b7f5-ea07361b26a9"
#define BLE_CHAR_STATUS_UUID          "beb5483e-36e1-4688-b7f5-ea07361b26aa"
#define BLE_CHAR_PID_UUID             "beb5483e-36e1-4688-b7f5-ea07361b26ab"

// --- Data Logging ---
#define LOG_FREQ_HZ           1      // 1Hz CSV output over serial + BLE

#endif // CONFIG_H
