#include "thermal.h"

// ============================================================================
// Thermal Sensing & State Estimation
// ONYX Bethlehem Labs + SSTA IP Holdings
// ============================================================================

static const uint8_t sensorPins[NUM_SENSORS] = {
    PIN_T1, PIN_T2, PIN_T3, PIN_T4, PIN_T5, PIN_T6
};

static const float calOffsets[NUM_SENSORS] = {
    CAL_OFFSET_T1, CAL_OFFSET_T2, CAL_OFFSET_T3,
    CAL_OFFSET_T4, CAL_OFFSET_T5, CAL_OFFSET_T6
};

void thermal_init() {
    // ESP32 ADC1 configuration
    analogReadResolution(12);
    analogSetAttenuation(ADC_11db);  // Full 0–3.3V range

    for (int i = 0; i < NUM_SENSORS; i++) {
        pinMode(sensorPins[i], INPUT);
    }
}

float adc_to_celsius(int raw_adc) {
    if (raw_adc <= 0 || raw_adc >= (int)ADC_RESOLUTION) {
        return -999.0f;  // Invalid reading — open or shorted sensor
    }

    // Voltage divider: V_out = V_ref * R_therm / (R_ref + R_therm)
    // => R_therm = R_ref * raw / (ADC_MAX - raw)
    float resistance = REFERENCE_R * ((float)raw_adc / (ADC_RESOLUTION - (float)raw_adc));

    // B-parameter Steinhart-Hart: 1/T = 1/T0 + (1/B) * ln(R/R0)
    float steinhart = 1.0f / THERMISTOR_T0;
    steinhart += (1.0f / THERMISTOR_B) * log(resistance / THERMISTOR_R0);
    float tempK = 1.0f / steinhart;
    float tempC = tempK - 273.15f;

    // Sanity check
    if (tempC < -40.0f || tempC > 150.0f) {
        return -999.0f;
    }

    return tempC;
}

SensorReadings thermal_read() {
    SensorReadings readings;
    readings.timestamp = millis();

    for (int ch = 0; ch < NUM_SENSORS; ch++) {
        // Oversample for noise reduction
        uint32_t sum = 0;
        for (int s = 0; s < ADC_OVERSAMPLE; s++) {
            sum += analogRead(sensorPins[ch]);
        }
        int avg_adc = sum / ADC_OVERSAMPLE;

        float tempC = adc_to_celsius(avg_adc);

        if (tempC > -900.0f) {
            // Apply calibration offset
            readings.temps[ch] = tempC - calOffsets[ch];
            readings.valid[ch] = true;
        } else {
            readings.temps[ch] = -999.0f;
            readings.valid[ch] = false;
        }
    }

    return readings;
}

ThermalState thermal_estimate_state(float T4, float T5) {
    ThermalState state;

    // Melt fraction estimation using sigmoid model centered on Tm
    // Uses T4 (vault core) as primary indicator
    // Sigmoid: f = 1 / (1 + exp(-(T - Tm) / halfWidth))
    if (T4 < -900.0f) {
        // T4 sensor fault — use conservative defaults
        state.meltFraction = 0.5f;
        state.phase = VAULT_TRANSITION;
    } else {
        float x = (T4 - PCM_TM_C) / PCM_TRANSITION_HALF;
        state.meltFraction = 1.0f / (1.0f + expf(-x));

        if (state.meltFraction < 0.1f) {
            state.phase = VAULT_SOLID;
        } else if (state.meltFraction > 0.9f) {
            state.phase = VAULT_LIQUID;
        } else {
            state.phase = VAULT_TRANSITION;
        }
    }

    // Energy stored estimation
    // Sensible component: m * cp * (T4 - T_ref)
    float T_ref = 24.0f;  // Room temperature reference
    float cp = (state.phase == VAULT_LIQUID) ? PCM_CP_LIQUID : PCM_CP_SOLID;
    float sensible = PCM_MASS_G * cp * (T4 - T_ref);
    if (sensible < 0) sensible = 0;

    // Latent component: melt fraction * total latent capacity
    float latent = state.meltFraction * VAULT_LATENT_J;

    state.energyStored_J = sensible + latent;

    // State-of-charge: fraction of total capacity used
    // SOC = 1.0 means full (vault at baseline), 0.0 means depleted
    state.vaultSOC = 1.0f - (state.energyStored_J / VAULT_TOTAL_J);
    if (state.vaultSOC < 0.0f) state.vaultSOC = 0.0f;
    if (state.vaultSOC > 1.0f) state.vaultSOC = 1.0f;

    return state;
}
