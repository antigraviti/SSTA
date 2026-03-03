#ifndef BLE_SERVICE_H
#define BLE_SERVICE_H

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include "config.h"
#include "thermal.h"
#include "tec_driver.h"
#include "pid_controller.h"

// ============================================================================
// BLE GATT Service for BackPal Companion App
// ONYX Bethlehem Labs + SSTA IP Holdings
//
// Service: BackPal Thermal Control
//   - Temperatures characteristic (notify): T1–T6 as packed floats
//   - Control characteristic (read/write): mode, power, start/stop
//   - Status characteristic (notify): vault SOC, phase, session info
//   - PID characteristic (read/write): gains, setpoint tuning
// ============================================================================

// Control command structure (written by app)
struct __attribute__((packed)) BLEControlCmd {
    uint8_t command;    // 0=STOP, 1=COOL, 2=HEAT, 3=PROFILE
    uint8_t power;      // 0-255 drive level (manual mode)
    float   setpoint;   // Target temp °C (PID mode)
    uint8_t profileId;  // 0=manual, 1=clinical
};

// Temperature packet (notified to app at 1Hz)
struct __attribute__((packed)) BLETempPacket {
    float t1, t2, t3, t4, t5, t6;
    uint32_t timestamp;  // millis() truncated to 32-bit
};

// Status packet (notified to app at 1Hz)
struct __attribute__((packed)) BLEStatusPacket {
    uint8_t  mode;             // TECMode enum
    uint8_t  profileActive;    // 0=idle, 1=running
    uint8_t  phaseIndex;       // Current phase in profile (0-based)
    float    setpoint;         // Current target °C
    float    vaultSOC;         // 0.0–1.0
    float    meltFraction;     // 0.0–1.0
    float    runtimeRemainS;   // Estimated seconds remaining
    float    currentAmps;      // TEC current from INA219
    float    powerWatts;       // TEC power from INA219
    float    batteryPct;       // Battery SOC 0–100%
    uint16_t sessionElapsedS;  // Total session time (s)
    uint16_t therapeuticS;     // TEMP_THEN_HOLD therapeutic time (s)
    uint8_t  safetyFlags;      // Bitfield: bit0=overtemp, bit1=undertemp, bit2=overcurrent
};

// PID tuning packet (read/write)
struct __attribute__((packed)) BLEPIDPacket {
    float Kp, Ki, Kd;
    float setpoint;
    float integral;      // Read-only: current integral term
    float output;        // Read-only: current PID output
};

class BLEService {
public:
    BLEService();

    void init();
    bool isConnected() const;

    // Send temperature data to app (call at 1Hz)
    void notifyTemperatures(const SensorReadings& readings);

    // Send status data to app (call at 1Hz)
    void notifyStatus(const BLEStatusPacket& status);

    // Check if app has written a new control command
    bool hasNewCommand() const;
    BLEControlCmd getCommand();

    // Check if app has written new PID gains
    bool hasNewPIDGains() const;
    BLEPIDPacket getPIDUpdate();

    // Connection event callbacks
    void onConnect();
    void onDisconnect();

    // Callbacks need access to internal state
    friend class ControlWriteCallback;
    friend class PIDWriteCallback;
    friend class ServerCallbacks;

private:
    BLEServer* _server;
    BLECharacteristic* _charTemps;
    BLECharacteristic* _charControl;
    BLECharacteristic* _charStatus;
    BLECharacteristic* _charPID;

    volatile bool _connected;
    volatile bool _newCommand;
    volatile bool _newPID;
    BLEControlCmd _lastCmd;
    BLEPIDPacket  _lastPID;
};

// Global BLE callbacks (required by ESP32 BLE API)
extern BLEService* g_bleServicePtr;

class ServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer* server) override;
    void onDisconnect(BLEServer* server) override;
};

class ControlWriteCallback : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic* pChar) override;
};

class PIDWriteCallback : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic* pChar) override;
};

#endif // BLE_SERVICE_H
