#ifndef THERMAL_H
#define THERMAL_H

#include <Arduino.h>
#include "config.h"

// ============================================================================
// Thermal Sensing & State Estimation
// ONYX Bethlehem Labs + SSTA IP Holdings
// ============================================================================

// Sensor reading structure
struct SensorReadings {
    float temps[NUM_SENSORS];  // T1–T6 in °C
    bool  valid[NUM_SENSORS];  // Sensor health flags
    unsigned long timestamp;    // millis() at read time
};

// Vault thermal state
enum VaultPhase {
    VAULT_SOLID      = 0,  // T4 well below Tm
    VAULT_TRANSITION = 1,  // T4 near Tm (melting/solidifying)
    VAULT_LIQUID     = 2   // T4 well above Tm
};

struct ThermalState {
    float meltFraction;     // 0.0 (fully solid) to 1.0 (fully liquid)
    float vaultSOC;         // State-of-charge: 1.0 = full capacity, 0.0 = depleted
    VaultPhase phase;       // Current vault phase
    float energyStored_J;   // Estimated energy stored in vault
};

// PCM properties for paraffin (TooGet food-grade, Tm ~55°C)
#define PCM_MASS_G           36.15f
#define PCM_TM_C             55.0f      // Melt point (°C)
#define PCM_LATENT_J_G       180.0f     // Latent heat (J/g)
#define PCM_CP_SOLID         2.0f       // Specific heat solid (J/g·K)
#define PCM_CP_LIQUID        2.2f       // Specific heat liquid (J/g·K)
#define PCM_TRANSITION_HALF  3.0f       // Half-width of melt transition (°C)

// Total energy capacity
#define VAULT_LATENT_J       (PCM_MASS_G * PCM_LATENT_J_G)  // ~6507 J
#define VAULT_SENSIBLE_J     2500.0f                          // ~2.5 kJ over operating range
#define VAULT_TOTAL_J        (VAULT_LATENT_J + VAULT_SENSIBLE_J)

// Initialize ADC pins and calibration
void thermal_init();

// Read all sensors with oversampling and calibration
SensorReadings thermal_read();

// Estimate vault thermal state from T4 (core) and T5 (edge)
ThermalState thermal_estimate_state(float T4, float T5);

// Convert raw ADC to temperature using B-parameter model
float adc_to_celsius(int raw_adc);

#endif // THERMAL_H
