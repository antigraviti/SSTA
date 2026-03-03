/**
 * DashboardScreen — Main live monitoring and control interface
 * ONYX Bethlehem Labs + SSTA IP Holdings
 */

import React, { useState, useEffect, useRef, useCallback } from 'react';
import { View, Text, ScrollView, StyleSheet, TouchableOpacity, Alert } from 'react-native';
import backpalBLE, {
  TemperatureData, StatusData, ConnectionState
} from '../services/BackPalBLE';
import TemperatureGauge from '../components/TemperatureGauge';
import StatusBar from '../components/StatusBar';
import ModeControl from '../components/ModeControl';
import SessionInfo from '../components/SessionInfo';

const DashboardScreen: React.FC = () => {
  const [connState, setConnState] = useState<ConnectionState>('disconnected');
  const [temps, setTemps] = useState<TemperatureData | null>(null);
  const [status, setStatus] = useState<StatusData | null>(null);

  const connStateRef = useRef(connState);
  connStateRef.current = connState;

  // Set up BLE callbacks on mount
  useEffect(() => {
    backpalBLE.setCallbacks({
      onTemperatures: (data) => setTemps(data),
      onStatus: (data) => setStatus(data),
      onConnectionChange: (state) => setConnState(state),
      onError: (err) => console.warn('[BLE Error]', err),
    });

    return () => {
      backpalBLE.setCallbacks({});
    };
  }, []);

  const handleConnect = useCallback(async () => {
    if (connStateRef.current === 'connected') {
      await backpalBLE.disconnect();
      return;
    }
    if (connStateRef.current === 'scanning' || connStateRef.current === 'connecting') {
      return;
    }

    try {
      await backpalBLE.scanAndConnect();
    } catch (e: any) {
      Alert.alert('Connection Failed', e.message);
    }
  }, []);

  const isConnected = connState === 'connected';

  return (
    <View style={styles.root}>
      <StatusBar
        connectionState={connState}
        batteryPct={status?.batteryPct ?? 0}
        vaultSOC={status?.vaultSOC ?? 0}
        safetyFlags={status?.safetyFlags ?? 0}
      />

      <ScrollView style={styles.scroll} contentContainerStyle={styles.scrollContent}>
        {/* Connection button */}
        <TouchableOpacity style={styles.connectBtn} onPress={handleConnect}>
          <Text style={styles.connectBtnText}>
            {connState === 'connected' ? 'Disconnect'
              : connState === 'scanning' ? 'Scanning...'
              : connState === 'connecting' ? 'Connecting...'
              : 'Connect to BackPal'}
          </Text>
        </TouchableOpacity>

        {/* Temperature grid */}
        <View style={styles.section}>
          <Text style={styles.sectionTitle}>TEMPERATURES</Text>
          <View style={styles.tempGrid}>
            <TemperatureGauge label="T1 Skin" value={temps?.t1 ?? null} alertHigh={45} />
            <TemperatureGauge label="T2 TEC" value={temps?.t2 ?? null} alertHigh={80} />
            <TemperatureGauge label="T3 HS" value={temps?.t3 ?? null} />
            <TemperatureGauge label="T4 Core" value={temps?.t4 ?? null} />
            <TemperatureGauge label="T5 Edge" value={temps?.t5 ?? null} />
            <TemperatureGauge label="T6 Ctrl" value={temps?.t6 ?? null} alertHigh={45} alertLow={5} />
          </View>
        </View>

        {/* Power info */}
        <View style={styles.powerRow}>
          <View style={styles.powerItem}>
            <Text style={styles.powerLabel}>CURRENT</Text>
            <Text style={styles.powerValue}>{status?.currentAmps?.toFixed(2) ?? '--'} A</Text>
          </View>
          <View style={styles.powerItem}>
            <Text style={styles.powerLabel}>POWER</Text>
            <Text style={styles.powerValue}>{status?.powerWatts?.toFixed(1) ?? '--'} W</Text>
          </View>
          <View style={styles.powerItem}>
            <Text style={styles.powerLabel}>VOLTAGE</Text>
            <Text style={styles.powerValue}>
              {status?.currentAmps && status?.powerWatts
                ? (status.powerWatts / Math.max(status.currentAmps, 0.001)).toFixed(1)
                : '--'} V
            </Text>
          </View>
        </View>

        {/* Session info */}
        <SessionInfo
          profileActive={status?.profileActive ?? false}
          phaseIndex={status?.phaseIndex ?? 0}
          totalPhases={7}
          sessionElapsedS={status?.sessionElapsedS ?? 0}
          therapeuticS={status?.therapeuticS ?? 0}
          runtimeRemainS={status?.runtimeRemainS ?? 0}
          meltFraction={status?.meltFraction ?? 0}
          currentMode={status?.mode ?? 0}
        />

        {/* Mode controls */}
        <ModeControl
          currentMode={status?.mode ?? 0}
          profileActive={status?.profileActive ?? false}
          currentSetpoint={status?.setpoint ?? 24.0}
          isConnected={isConnected}
        />
      </ScrollView>
    </View>
  );
};

const styles = StyleSheet.create({
  root: {
    flex: 1,
    backgroundColor: '#0a0a1a',
  },
  scroll: {
    flex: 1,
  },
  scrollContent: {
    paddingBottom: 32,
  },
  connectBtn: {
    backgroundColor: '#1e3a5f',
    marginHorizontal: 16,
    marginTop: 12,
    paddingVertical: 12,
    borderRadius: 10,
    alignItems: 'center',
    borderWidth: 1,
    borderColor: '#2a5a8f',
  },
  connectBtnText: {
    color: '#4fc3f7',
    fontSize: 16,
    fontWeight: '700',
  },
  section: {
    marginTop: 16,
    paddingHorizontal: 12,
  },
  sectionTitle: {
    color: '#888',
    fontSize: 12,
    fontWeight: '600',
    letterSpacing: 1,
    marginBottom: 8,
    marginLeft: 4,
  },
  tempGrid: {
    flexDirection: 'row',
    flexWrap: 'wrap',
    justifyContent: 'center',
  },
  powerRow: {
    flexDirection: 'row',
    justifyContent: 'space-evenly',
    marginTop: 16,
    paddingHorizontal: 16,
  },
  powerItem: {
    alignItems: 'center',
  },
  powerLabel: {
    color: '#888',
    fontSize: 10,
    marginBottom: 2,
  },
  powerValue: {
    color: '#fff',
    fontSize: 16,
    fontWeight: '600',
    fontVariant: ['tabular-nums'],
  },
});

export default DashboardScreen;
