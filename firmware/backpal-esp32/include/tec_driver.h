#ifndef TEC_DRIVER_H
#define TEC_DRIVER_H

#include <Arduino.h>
#include "config.h"

// ============================================================================
// TEC Driver — IBT-2 H-Bridge Control
// ONYX Bethlehem Labs + SSTA IP Holdings
//
// Drives TEC1-12706 via IBT-2 (BTS7960) in constant DC mode.
// PWM is superseded — LEDC used only to set constant DC level (duty=target).
// ============================================================================

enum TECMode {
    TEC_OFF  = 0,
    TEC_COOL = 1,  // Cold side UP (toward skin) — RPWM active
    TEC_HEAT = 2   // Reversed polarity — LPWM active
};

class TECDriver {
public:
    TECDriver();

    void init();

    // Set mode (OFF/COOL/HEAT) — handles enable pins and direction
    void setMode(TECMode mode);
    TECMode getMode() const;

    // Set drive level: 0-255 (constant DC via LEDC duty)
    void setDrive(uint8_t level);
    uint8_t getDrive() const;

    // Emergency stop — all outputs LOW, enables LOW
    void emergencyStop();

    // Is the driver currently active?
    bool isActive() const;

private:
    TECMode _mode;
    uint8_t _drive;

    void _enableBridge(bool enable);
    void _setOutputs(uint8_t rpwm, uint8_t lpwm);
    void _applyDrive();
};

#endif // TEC_DRIVER_H
