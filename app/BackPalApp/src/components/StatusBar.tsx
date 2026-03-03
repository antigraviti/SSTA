/**
 * StatusBar — Connection status, battery, and vault SOC indicator
 * ONYX Bethlehem Labs + SSTA IP Holdings
 */

import React from 'react';
import { View, Text, StyleSheet } from 'react-native';
import { ConnectionState } from '../services/BackPalBLE';

interface Props {
  connectionState: ConnectionState;
  batteryPct: number;
  vaultSOC: number;
  safetyFlags: number;
}

const StatusBar: React.FC<Props> = ({ connectionState, batteryPct, vaultSOC, safetyFlags }) => {
  const connColor = connectionState === 'connected' ? '#4caf50'
                   : connectionState === 'scanning' || connectionState === 'connecting' ? '#ff9800'
                   : '#f44336';

  const connLabel = connectionState === 'connected' ? 'Connected'
                  : connectionState === 'scanning' ? 'Scanning...'
                  : connectionState === 'connecting' ? 'Connecting...'
                  : 'Disconnected';

  const hasFault = safetyFlags !== 0;

  return (
    <View style={styles.container}>
      <View style={styles.row}>
        {/* Connection indicator */}
        <View style={styles.item}>
          <View style={[styles.dot, { backgroundColor: connColor }]} />
          <Text style={[styles.text, { color: connColor }]}>{connLabel}</Text>
        </View>

        {/* Battery */}
        <View style={styles.item}>
          <Text style={styles.text}>
            BAT {batteryPct.toFixed(0)}%
          </Text>
        </View>

        {/* Vault SOC */}
        <View style={styles.item}>
          <Text style={styles.text}>
            VAULT {(vaultSOC * 100).toFixed(0)}%
          </Text>
        </View>

        {/* Safety */}
        {hasFault && (
          <View style={styles.item}>
            <Text style={styles.fault}>FAULT 0x{safetyFlags.toString(16).toUpperCase()}</Text>
          </View>
        )}
      </View>
    </View>
  );
};

const styles = StyleSheet.create({
  container: {
    backgroundColor: '#1a1a2e',
    paddingVertical: 8,
    paddingHorizontal: 16,
    borderBottomWidth: 1,
    borderBottomColor: '#2a2a4a',
  },
  row: {
    flexDirection: 'row',
    justifyContent: 'space-between',
    alignItems: 'center',
  },
  item: {
    flexDirection: 'row',
    alignItems: 'center',
  },
  dot: {
    width: 8,
    height: 8,
    borderRadius: 4,
    marginRight: 6,
  },
  text: {
    color: '#ccc',
    fontSize: 12,
    fontWeight: '500',
  },
  fault: {
    color: '#ef5350',
    fontSize: 12,
    fontWeight: '700',
  },
});

export default StatusBar;
