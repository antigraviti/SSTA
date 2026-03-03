/**
 * SessionInfo — Session timer, phase progress, and runtime display
 * ONYX Bethlehem Labs + SSTA IP Holdings
 */

import React from 'react';
import { View, Text, StyleSheet } from 'react-native';

interface Props {
  profileActive: boolean;
  phaseIndex: number;
  totalPhases: number;
  sessionElapsedS: number;
  therapeuticS: number;
  runtimeRemainS: number;
  meltFraction: number;
  currentMode: number;
}

const formatTime = (seconds: number): string => {
  const m = Math.floor(seconds / 60);
  const s = Math.floor(seconds % 60);
  return `${m}:${s.toString().padStart(2, '0')}`;
};

const SessionInfo: React.FC<Props> = ({
  profileActive, phaseIndex, totalPhases,
  sessionElapsedS, therapeuticS, runtimeRemainS,
  meltFraction, currentMode,
}) => {
  if (!profileActive && sessionElapsedS === 0) {
    return null;  // Don't render when idle
  }

  const modeLabel = currentMode === 1 ? 'COOL' : currentMode === 2 ? 'HEAT' : 'IDLE';
  const modeColor = currentMode === 1 ? '#29b6f6' : currentMode === 2 ? '#ef5350' : '#666';
  const meltPct = (meltFraction * 100).toFixed(0);

  return (
    <View style={styles.container}>
      {/* Phase progress */}
      {profileActive && (
        <View style={styles.phaseRow}>
          <Text style={styles.phaseLabel}>PHASE</Text>
          <View style={styles.phaseIndicators}>
            {Array.from({ length: totalPhases || 7 }, (_, i) => (
              <View
                key={i}
                style={[
                  styles.phaseDot,
                  i < phaseIndex && styles.phaseComplete,
                  i === phaseIndex && styles.phaseCurrent,
                  i === phaseIndex && { borderColor: modeColor },
                ]}
              />
            ))}
          </View>
          <Text style={[styles.phaseMode, { color: modeColor }]}>{modeLabel}</Text>
        </View>
      )}

      {/* Timers */}
      <View style={styles.timerRow}>
        <View style={styles.timerItem}>
          <Text style={styles.timerLabel}>SESSION</Text>
          <Text style={styles.timerValue}>{formatTime(sessionElapsedS)}</Text>
        </View>

        <View style={styles.timerItem}>
          <Text style={styles.timerLabel}>THERAPEUTIC</Text>
          <Text style={[styles.timerValue, styles.therapeuticValue]}>
            {formatTime(therapeuticS)}
          </Text>
          <Text style={styles.timerSub}>TEMP_THEN_HOLD</Text>
        </View>

        <View style={styles.timerItem}>
          <Text style={styles.timerLabel}>REMAINING</Text>
          <Text style={styles.timerValue}>
            {runtimeRemainS > 0 ? formatTime(runtimeRemainS) : '--:--'}
          </Text>
        </View>
      </View>

      {/* Melt fraction bar */}
      <View style={styles.meltRow}>
        <Text style={styles.meltLabel}>MELT FRACTION</Text>
        <View style={styles.meltBarBg}>
          <View style={[styles.meltBarFill, { width: `${meltPct}%` }]} />
        </View>
        <Text style={styles.meltValue}>{meltPct}%</Text>
      </View>
    </View>
  );
};

const styles = StyleSheet.create({
  container: {
    padding: 16,
    backgroundColor: '#1a1a2e',
    marginHorizontal: 12,
    borderRadius: 12,
    marginTop: 8,
  },
  phaseRow: {
    flexDirection: 'row',
    alignItems: 'center',
    marginBottom: 12,
  },
  phaseLabel: {
    color: '#888',
    fontSize: 11,
    marginRight: 8,
  },
  phaseIndicators: {
    flexDirection: 'row',
    flex: 1,
    justifyContent: 'center',
    gap: 6,
  },
  phaseDot: {
    width: 12,
    height: 12,
    borderRadius: 6,
    backgroundColor: '#333',
    borderWidth: 2,
    borderColor: '#555',
  },
  phaseComplete: {
    backgroundColor: '#4caf50',
    borderColor: '#4caf50',
  },
  phaseCurrent: {
    backgroundColor: '#1a1a2e',
    borderWidth: 3,
  },
  phaseMode: {
    fontSize: 12,
    fontWeight: '700',
    marginLeft: 8,
  },
  timerRow: {
    flexDirection: 'row',
    justifyContent: 'space-between',
    marginBottom: 12,
  },
  timerItem: {
    alignItems: 'center',
    flex: 1,
  },
  timerLabel: {
    color: '#888',
    fontSize: 10,
    marginBottom: 2,
  },
  timerValue: {
    color: '#fff',
    fontSize: 20,
    fontWeight: '700',
    fontVariant: ['tabular-nums'],
  },
  therapeuticValue: {
    color: '#4caf50',
  },
  timerSub: {
    color: '#666',
    fontSize: 8,
    marginTop: 1,
  },
  meltRow: {
    flexDirection: 'row',
    alignItems: 'center',
  },
  meltLabel: {
    color: '#888',
    fontSize: 10,
    marginRight: 8,
  },
  meltBarBg: {
    flex: 1,
    height: 6,
    backgroundColor: '#333',
    borderRadius: 3,
    overflow: 'hidden',
  },
  meltBarFill: {
    height: '100%',
    backgroundColor: '#ff9800',
    borderRadius: 3,
  },
  meltValue: {
    color: '#ff9800',
    fontSize: 11,
    fontWeight: '600',
    marginLeft: 8,
    width: 32,
    textAlign: 'right',
  },
});

export default SessionInfo;
