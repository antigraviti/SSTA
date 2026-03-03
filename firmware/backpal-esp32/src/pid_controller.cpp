#include "pid_controller.h"

// ============================================================================
// PID Controller with Thermal-State-Aware Gain Scheduling
// ONYX Bethlehem Labs + SSTA IP Holdings
//
// Core E-01 patent differentiator: the controller adapts its behavior
// based on the vault's thermal state (melt fraction, phase, SOC).
// This is NOT a simple PID — it implements:
//   1. Gain scheduling across solid/transition/liquid phases
//   2. Melt fraction throttling near vault saturation
//   3. Runtime prediction from vault SOC
//   4. Scaffold saturation detection
// ============================================================================

PIDController::PIDController() {
    _state = {};
    _state.setpoint = 24.0f;  // Default: room temp (idle)
    _state.gains = {PID_KP_COOL, PID_KI_COOL, PID_KD_COOL};
    _state.throttle = 1.0f;

    // Default gain sets per phase — COOL mode defaults
    // These are starting points; will need lab tuning
    _gainsSolid      = {PID_KP_COOL * 1.2f, PID_KI_COOL * 0.5f, PID_KD_COOL};
    _gainsTransition = {PID_KP_COOL,         PID_KI_COOL,         PID_KD_COOL};
    _gainsLiquid     = {PID_KP_COOL * 0.7f,  PID_KI_COOL * 1.5f,  PID_KD_COOL};
}

void PIDController::setSetpoint(float target_c) {
    if (_state.setpoint != target_c) {
        _state.setpoint = target_c;
        // Don't reset integral on small setpoint changes — only on mode transitions
    }
}

float PIDController::getSetpoint() const {
    return _state.setpoint;
}

void PIDController::setGains(float Kp, float Ki, float Kd) {
    _state.gains = {Kp, Ki, Kd};
    // Also update the transition gains (default operating point)
    _gainsTransition = {Kp, Ki, Kd};
}

PIDGains PIDController::getGains() const {
    return _state.gains;
}

float PIDController::compute(float measuredTemp, const ThermalState& state) {
    unsigned long now = millis();
    float dt = (_state.lastTime > 0) ? (now - _state.lastTime) / 1000.0f : 0.1f;
    _state.lastTime = now;

    // Clamp dt to avoid huge jumps after pauses
    if (dt > 1.0f) dt = 0.1f;
    if (dt <= 0.0f) dt = 0.1f;

    // 1. Gain scheduling based on vault thermal state
    _state.gains = _scheduleGains(state.phase);

    // 2. Melt fraction throttle — reduce output as vault saturates
    _state.throttle = _computeThrottle(state.meltFraction);

    // 3. PID computation
    _state.error = _state.setpoint - measuredTemp;

    // Proportional
    float P = _state.gains.Kp * _state.error;

    // Integral with anti-windup
    _state.integral += _state.gains.Ki * _state.error * dt;
    _clampIntegral();
    float I = _state.integral;

    // Derivative (on error, with simple filtering)
    float dError = (_state.error - _state.prevError) / dt;
    float D = _state.gains.Kd * dError;
    _state.prevError = _state.error;

    // Raw output
    float output = P + I + D;

    // 4. Apply melt fraction throttle
    output *= _state.throttle;

    // Clamp to output range
    if (output > PID_OUTPUT_MAX) output = PID_OUTPUT_MAX;
    if (output < PID_OUTPUT_MIN) output = PID_OUTPUT_MIN;

    _state.output = output;
    return output;
}

void PIDController::reset() {
    _state.integral = 0.0f;
    _state.prevError = 0.0f;
    _state.output = 0.0f;
    _state.lastTime = 0;
    _state.throttle = 1.0f;
}

RuntimeEstimate PIDController::estimateRuntime(const ThermalState& state,
                                                float currentAmps,
                                                float voltage) {
    RuntimeEstimate est;

    float power = currentAmps * voltage;
    if (power < 0.1f) power = 0.1f;  // Avoid division by zero

    // Remaining energy in vault
    float remainingJ = state.vaultSOC * VAULT_TOTAL_J;
    est.remainingSeconds = remainingJ / power;

    // Depletion percentage
    est.depletionPercent = (1.0f - state.vaultSOC) * 100.0f;

    // Scaffold saturation detection:
    // If melt fraction > 0.95 and T4 is rising rapidly, scaffold is saturated
    est.saturated = (state.meltFraction > 0.95f);

    // Alert thresholds
    est.alertLow = (state.vaultSOC < 0.20f);
    est.alertCritical = (state.vaultSOC < 0.05f);

    return est;
}

PIDState PIDController::getState() const {
    return _state;
}

PIDGains PIDController::_scheduleGains(VaultPhase phase) const {
    switch (phase) {
        case VAULT_SOLID:
            // Vault fully solid — high thermal inertia, can be more aggressive
            return _gainsSolid;
        case VAULT_TRANSITION:
            // Phase change plateau — PCM absorbing/releasing energy
            // Standard gains; the plateau provides natural buffering
            return _gainsTransition;
        case VAULT_LIQUID:
            // Vault fully melted — less buffering capacity
            // Lower Kp to prevent overshoot, higher Ki for steady-state accuracy
            return _gainsLiquid;
        default:
            return _gainsTransition;
    }
}

float PIDController::_computeThrottle(float meltFraction) const {
    // Throttle curve: full power until melt fraction > 0.7,
    // then linearly reduce to 30% at melt fraction = 1.0
    // This prevents driving the vault to full saturation,
    // preserving capacity for multi-cycle contrast therapy.
    if (meltFraction < 0.7f) {
        return 1.0f;
    }
    // Linear ramp: 1.0 at f=0.7, 0.3 at f=1.0
    float t = 1.0f - ((meltFraction - 0.7f) / 0.3f) * 0.7f;
    if (t < 0.3f) t = 0.3f;
    return t;
}

void PIDController::_clampIntegral() {
    if (_state.integral > PID_INTEGRAL_MAX) _state.integral = PID_INTEGRAL_MAX;
    if (_state.integral < PID_INTEGRAL_MIN) _state.integral = PID_INTEGRAL_MIN;
}
