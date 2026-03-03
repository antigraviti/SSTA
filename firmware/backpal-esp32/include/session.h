#ifndef SESSION_H
#define SESSION_H

#include <Arduino.h>
#include "config.h"
#include "tec_driver.h"

// ============================================================================
// Therapeutic Session Manager — TEMP_THEN_HOLD Timer Logic
// ONYX Bethlehem Labs + SSTA IP Holdings
//
// Implements BackPal Clinical profile and manual control modes.
// TEMP_THEN_HOLD: therapeutic time counts only when T1 is within
// tolerance of the target temperature. This is a firm design decision
// (D-FW-01) — do not revert to wall-clock timing.
// ============================================================================

// A single phase in a therapeutic profile
struct TherapyPhase {
    TECMode mode;           // HEAT or COOL
    float   targetTemp;     // Target skin temperature (°C)
    uint16_t durationSec;   // Required therapeutic seconds at target
};

// Session state
enum SessionState {
    SESSION_IDLE     = 0,
    SESSION_RUNNING  = 1,
    SESSION_PAUSED   = 2,
    SESSION_COMPLETE = 3,
    SESSION_FAULT    = 4
};

class SessionManager {
public:
    SessionManager();

    // Load a profile (array of phases)
    void loadProfile(const TherapyPhase* phases, uint8_t numPhases);

    // Load the built-in BackPal Clinical profile
    void loadClinicalProfile();

    // Start/stop/pause
    void start();
    void stop();
    void pause();
    void resume();

    // Call every control loop iteration with current T1 reading
    // Returns the target temperature for the current phase
    float update(float T1_current);

    // Getters
    SessionState getState() const;
    uint8_t getCurrentPhaseIndex() const;
    uint8_t getTotalPhases() const;
    TECMode getCurrentMode() const;
    float getCurrentTarget() const;
    uint16_t getSessionElapsedSec() const;
    uint16_t getTherapeuticSec() const;     // TEMP_THEN_HOLD accumulated time
    uint16_t getPhaseTherapeuticSec() const; // Time in-target for current phase
    uint16_t getPhaseDurationSec() const;    // Required duration for current phase
    bool isPhaseComplete() const;

private:
    SessionState _state;
    const TherapyPhase* _phases;
    uint8_t _numPhases;
    uint8_t _currentPhase;

    unsigned long _sessionStartMs;
    unsigned long _lastUpdateMs;

    // TEMP_THEN_HOLD accumulators (in milliseconds for precision)
    unsigned long _totalTherapeuticMs;
    unsigned long _phaseTherapeuticMs;

    void _advancePhase();
};

#endif // SESSION_H
