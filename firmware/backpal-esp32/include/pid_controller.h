#ifndef PID_CONTROLLER_H
#define PID_CONTROLLER_H

#include <Arduino.h>
#include "config.h"
#include "thermal.h"

// ============================================================================
// PID Controller with Thermal-State-Aware Gain Scheduling
// ONYX Bethlehem Labs + SSTA IP Holdings
//
// Key features distinguishing from prior art (US 11,419,753):
//   1. PCM melt fraction throttling
//   2. Scaffold saturation detection
//   3. Runtime prediction with alerts
//   4. Multi-mode gain scheduling based on thermal state
// ============================================================================

struct PIDGains {
    float Kp;
    float Ki;
    float Kd;
};

struct PIDState {
    float setpoint;          // Target temperature (°C)
    float error;             // Current error (setpoint - measured)
    float integral;          // Accumulated integral term
    float prevError;         // Previous error for derivative
    float output;            // Controller output (0-255)
    float throttle;          // Melt fraction throttle factor (0.0–1.0)
    unsigned long lastTime;  // Last computation timestamp
    PIDGains gains;          // Active gains (may be scheduled)
};

struct RuntimeEstimate {
    float remainingSeconds;    // Estimated seconds until vault depleted
    float depletionPercent;    // How much capacity has been used (0–100)
    bool  saturated;           // Scaffold saturation detected
    bool  alertLow;            // SOC < 20% — warn user
    bool  alertCritical;       // SOC < 5% — action needed
};

class PIDController {
public:
    PIDController();

    // Set target temperature
    void setSetpoint(float target_c);
    float getSetpoint() const;

    // Set gains directly
    void setGains(float Kp, float Ki, float Kd);
    PIDGains getGains() const;

    // Core PID computation — returns duty (0-255)
    // measuredTemp: primary feedback sensor (T6 for prototype)
    // state: current vault thermal state for gain scheduling + throttling
    float compute(float measuredTemp, const ThermalState& state);

    // Reset integral accumulator (call on mode change)
    void reset();

    // Runtime prediction based on vault SOC and current power draw
    RuntimeEstimate estimateRuntime(const ThermalState& state, float currentAmps, float voltage);

    // Get current PID state for telemetry
    PIDState getState() const;

private:
    PIDState _state;

    // Gain scheduling: select gains based on vault phase
    PIDGains _scheduleGains(VaultPhase phase) const;

    // Melt fraction throttle: reduce output as vault approaches saturation
    float _computeThrottle(float meltFraction) const;

    // Anti-windup clamp
    void _clampIntegral();

    // Base gains for each thermal state
    PIDGains _gainsSolid;
    PIDGains _gainsTransition;
    PIDGains _gainsLiquid;
};

#endif // PID_CONTROLLER_H
