#include "tec_driver.h"

// ============================================================================
// TEC Driver — IBT-2 H-Bridge Constant DC Control
// ONYX Bethlehem Labs + SSTA IP Holdings
//
// Drives TEC1-12706 bidirectionally via IBT-2.
// Uses ESP32 LEDC peripheral to set constant DC voltage level.
// PWM switching noise is NOT used for thermal control (D-HW-02).
// The 20kHz LEDC frequency is solely for H-bridge commutation —
// the effective output is a steady DC level at the set duty.
// ============================================================================

TECDriver::TECDriver() : _mode(TEC_OFF), _drive(0) {}

void TECDriver::init() {
    // Configure enable pins as digital outputs
    pinMode(PIN_R_EN, OUTPUT);
    pinMode(PIN_L_EN, OUTPUT);
    digitalWrite(PIN_R_EN, LOW);
    digitalWrite(PIN_L_EN, LOW);

    // Configure LEDC channels for RPWM and LPWM
    ledcSetup(LEDC_CH_RPWM, LEDC_FREQ, LEDC_BITS);
    ledcSetup(LEDC_CH_LPWM, LEDC_FREQ, LEDC_BITS);
    ledcAttachPin(PIN_RPWM, LEDC_CH_RPWM);
    ledcAttachPin(PIN_LPWM, LEDC_CH_LPWM);

    // Start with outputs at zero
    ledcWrite(LEDC_CH_RPWM, 0);
    ledcWrite(LEDC_CH_LPWM, 0);

    _mode = TEC_OFF;
    _drive = 0;
}

void TECDriver::setMode(TECMode mode) {
    if (mode == _mode) return;

    // Always stop outputs before switching direction to prevent shoot-through
    _setOutputs(0, 0);
    _enableBridge(false);
    delay(5);  // Brief dead-time for H-bridge protection

    _mode = mode;

    if (mode != TEC_OFF) {
        _enableBridge(true);
        // Re-apply the current drive level in the new direction
        if (_drive > 0) {
            _applyDrive();
        }
    } else {
        _drive = 0;
    }
}

TECMode TECDriver::getMode() const {
    return _mode;
}

void TECDriver::setDrive(uint8_t level) {
    _drive = level;
    if (_mode != TEC_OFF) {
        _applyDrive();
    }
}

uint8_t TECDriver::getDrive() const {
    return _drive;
}

void TECDriver::emergencyStop() {
    _setOutputs(0, 0);
    _enableBridge(false);
    _mode = TEC_OFF;
    _drive = 0;
}

bool TECDriver::isActive() const {
    return (_mode != TEC_OFF) && (_drive > 0);
}

void TECDriver::_enableBridge(bool enable) {
    digitalWrite(PIN_R_EN, enable ? HIGH : LOW);
    digitalWrite(PIN_L_EN, enable ? HIGH : LOW);
}

void TECDriver::_setOutputs(uint8_t rpwm, uint8_t lpwm) {
    ledcWrite(LEDC_CH_RPWM, rpwm);
    ledcWrite(LEDC_CH_LPWM, lpwm);
}

void TECDriver::_applyDrive() {
    switch (_mode) {
        case TEC_COOL:
            // RPWM drives forward (cold side up toward skin)
            _setOutputs(_drive, 0);
            break;
        case TEC_HEAT:
            // LPWM drives reverse (hot side toward skin)
            _setOutputs(0, _drive);
            break;
        default:
            _setOutputs(0, 0);
            break;
    }
}
