#include "session.h"

// ============================================================================
// Therapeutic Session Manager — TEMP_THEN_HOLD Implementation
// ONYX Bethlehem Labs + SSTA IP Holdings
//
// TEMP_THEN_HOLD (D-FW-01): therapeutic time counts ONLY when T1 is
// within ±tolerance of the target temperature. This ensures clinically
// meaningful exposure measurement.
// ============================================================================

// Built-in BackPal Clinical profile: 4× (4min heat + 1min cool)
static const TherapyPhase clinicalPhases[] = {
    { TEC_HEAT, PROFILE_HEAT_TARGET_C, CLINICAL_HEAT_SEC },
    { TEC_COOL, PROFILE_COOL_TARGET_C, CLINICAL_COOL_SEC },
    { TEC_HEAT, PROFILE_HEAT_TARGET_C, CLINICAL_HEAT_SEC },
    { TEC_COOL, PROFILE_COOL_TARGET_C, CLINICAL_COOL_SEC },
    { TEC_HEAT, PROFILE_HEAT_TARGET_C, CLINICAL_HEAT_SEC },
    { TEC_COOL, PROFILE_COOL_TARGET_C, CLINICAL_COOL_SEC },
    { TEC_HEAT, PROFILE_HEAT_TARGET_C, CLINICAL_HEAT_SEC },
    // Final cool-down not included — device returns to IDLE after last heat
};
static const uint8_t CLINICAL_NUM_PHASES = sizeof(clinicalPhases) / sizeof(clinicalPhases[0]);

SessionManager::SessionManager()
    : _state(SESSION_IDLE)
    , _phases(nullptr)
    , _numPhases(0)
    , _currentPhase(0)
    , _sessionStartMs(0)
    , _lastUpdateMs(0)
    , _totalTherapeuticMs(0)
    , _phaseTherapeuticMs(0)
{}

void SessionManager::loadProfile(const TherapyPhase* phases, uint8_t numPhases) {
    _phases = phases;
    _numPhases = numPhases;
    _currentPhase = 0;
    _state = SESSION_IDLE;
}

void SessionManager::loadClinicalProfile() {
    loadProfile(clinicalPhases, CLINICAL_NUM_PHASES);
}

void SessionManager::start() {
    if (!_phases || _numPhases == 0) return;

    _state = SESSION_RUNNING;
    _currentPhase = 0;
    _sessionStartMs = millis();
    _lastUpdateMs = _sessionStartMs;
    _totalTherapeuticMs = 0;
    _phaseTherapeuticMs = 0;
}

void SessionManager::stop() {
    _state = SESSION_IDLE;
}

void SessionManager::pause() {
    if (_state == SESSION_RUNNING) {
        _state = SESSION_PAUSED;
    }
}

void SessionManager::resume() {
    if (_state == SESSION_PAUSED) {
        _state = SESSION_RUNNING;
        _lastUpdateMs = millis();  // Reset delta to avoid jump
    }
}

float SessionManager::update(float T1_current) {
    if (_state != SESSION_RUNNING || !_phases) {
        return 24.0f;  // Return room temp target when idle
    }

    unsigned long now = millis();
    unsigned long deltaMs = now - _lastUpdateMs;
    _lastUpdateMs = now;

    // TEMP_THEN_HOLD logic: accumulate therapeutic time only when
    // T1 is within tolerance of the current phase target
    float target = _phases[_currentPhase].targetTemp;
    float error = fabsf(T1_current - target);

    if (error <= PROFILE_TEMP_TOLERANCE) {
        _phaseTherapeuticMs += deltaMs;
        _totalTherapeuticMs += deltaMs;
    }

    // Check if current phase is complete
    uint32_t requiredMs = (uint32_t)_phases[_currentPhase].durationSec * 1000UL;
    if (_phaseTherapeuticMs >= requiredMs) {
        _advancePhase();
    }

    // Return current phase target for PID
    if (_state == SESSION_RUNNING && _currentPhase < _numPhases) {
        return _phases[_currentPhase].targetTemp;
    }
    return 24.0f;
}

SessionState SessionManager::getState() const {
    return _state;
}

uint8_t SessionManager::getCurrentPhaseIndex() const {
    return _currentPhase;
}

uint8_t SessionManager::getTotalPhases() const {
    return _numPhases;
}

TECMode SessionManager::getCurrentMode() const {
    if (_state != SESSION_RUNNING || !_phases || _currentPhase >= _numPhases) {
        return TEC_OFF;
    }
    return _phases[_currentPhase].mode;
}

float SessionManager::getCurrentTarget() const {
    if (_state != SESSION_RUNNING || !_phases || _currentPhase >= _numPhases) {
        return 24.0f;
    }
    return _phases[_currentPhase].targetTemp;
}

uint16_t SessionManager::getSessionElapsedSec() const {
    if (_sessionStartMs == 0) return 0;
    return (uint16_t)((millis() - _sessionStartMs) / 1000);
}

uint16_t SessionManager::getTherapeuticSec() const {
    return (uint16_t)(_totalTherapeuticMs / 1000);
}

uint16_t SessionManager::getPhaseTherapeuticSec() const {
    return (uint16_t)(_phaseTherapeuticMs / 1000);
}

uint16_t SessionManager::getPhaseDurationSec() const {
    if (!_phases || _currentPhase >= _numPhases) return 0;
    return _phases[_currentPhase].durationSec;
}

bool SessionManager::isPhaseComplete() const {
    if (!_phases || _currentPhase >= _numPhases) return false;
    uint32_t requiredMs = (uint32_t)_phases[_currentPhase].durationSec * 1000UL;
    return _phaseTherapeuticMs >= requiredMs;
}

void SessionManager::_advancePhase() {
    _currentPhase++;
    _phaseTherapeuticMs = 0;

    if (_currentPhase >= _numPhases) {
        _state = SESSION_COMPLETE;
    }
}
