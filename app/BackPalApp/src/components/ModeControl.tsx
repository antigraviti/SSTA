/**
 * ModeControl — Device mode selection and power/setpoint control
 * ONYX Bethlehem Labs + SSTA IP Holdings
 */

import React, { useState } from 'react';
import { View, Text, TouchableOpacity, StyleSheet, Alert } from 'react-native';
import backpalBLE, { TECMode } from '../services/BackPalBLE';

interface Props {
  currentMode: number;      // 0=OFF, 1=COOL, 2=HEAT
  profileActive: boolean;
  currentSetpoint: number;
  isConnected: boolean;
}

const ModeControl: React.FC<Props> = ({ currentMode, profileActive, currentSetpoint, isConnected }) => {
  const [pendingMode, setPendingMode] = useState<TECMode | null>(null);

  const modeLabel = currentMode === 1 ? 'COOL' : currentMode === 2 ? 'HEAT' : 'OFF';
  const modeColor = currentMode === 1 ? '#29b6f6' : currentMode === 2 ? '#ef5350' : '#666';

  const sendMode = async (mode: TECMode, setpoint?: number) => {
    if (!isConnected) return;
    setPendingMode(mode);
    try {
      if (mode === 'OFF') {
        await backpalBLE.stopDevice();
      } else if (setpoint) {
        if (mode === 'COOL') {
          await backpalBLE.setCoolMode(setpoint);
        } else {
          await backpalBLE.setHeatMode(setpoint);
        }
      }
    } catch (e: any) {
      Alert.alert('Command Failed', e.message);
    }
    setPendingMode(null);
  };

  const startProfile = async () => {
    if (!isConnected) return;
    try {
      // Send command=3 (PROFILE) with profileId=1 (clinical)
      await backpalBLE.sendCommand('HEAT', 0, 0, 1);
    } catch (e: any) {
      Alert.alert('Profile Start Failed', e.message);
    }
  };

  return (
    <View style={styles.container}>
      {/* Current mode display */}
      <View style={styles.modeDisplay}>
        <Text style={styles.modeLabel}>MODE</Text>
        <Text style={[styles.modeValue, { color: modeColor }]}>{modeLabel}</Text>
        {profileActive && <Text style={styles.profileBadge}>CLINICAL</Text>}
      </View>

      {/* Setpoint display */}
      <View style={styles.setpointDisplay}>
        <Text style={styles.setpointLabel}>TARGET</Text>
        <Text style={styles.setpointValue}>{currentSetpoint.toFixed(1)}°C</Text>
      </View>

      {/* Control buttons */}
      <View style={styles.buttonRow}>
        <TouchableOpacity
          style={[styles.btn, styles.btnCool, currentMode === 1 && styles.btnActive]}
          onPress={() => sendMode('COOL', 9.0)}
          disabled={!isConnected || pendingMode !== null}
        >
          <Text style={styles.btnText}>COOL</Text>
          <Text style={styles.btnSub}>9°C</Text>
        </TouchableOpacity>

        <TouchableOpacity
          style={[styles.btn, styles.btnStop]}
          onPress={() => sendMode('OFF')}
          disabled={!isConnected || pendingMode !== null}
        >
          <Text style={styles.btnText}>STOP</Text>
        </TouchableOpacity>

        <TouchableOpacity
          style={[styles.btn, styles.btnHeat, currentMode === 2 && styles.btnActive]}
          onPress={() => sendMode('HEAT', 40.0)}
          disabled={!isConnected || pendingMode !== null}
        >
          <Text style={styles.btnText}>HEAT</Text>
          <Text style={styles.btnSub}>40°C</Text>
        </TouchableOpacity>
      </View>

      {/* Clinical profile button */}
      <TouchableOpacity
        style={[styles.profileBtn, profileActive && styles.profileBtnActive]}
        onPress={profileActive ? () => sendMode('OFF') : startProfile}
        disabled={!isConnected}
      >
        <Text style={styles.profileBtnText}>
          {profileActive ? 'STOP PROFILE' : 'START CLINICAL PROFILE'}
        </Text>
        <Text style={styles.profileBtnSub}>4×4min Heat + 1min Cool</Text>
      </TouchableOpacity>
    </View>
  );
};

const styles = StyleSheet.create({
  container: {
    padding: 16,
  },
  modeDisplay: {
    flexDirection: 'row',
    alignItems: 'center',
    justifyContent: 'center',
    marginBottom: 8,
  },
  modeLabel: {
    color: '#888',
    fontSize: 14,
    marginRight: 8,
  },
  modeValue: {
    fontSize: 28,
    fontWeight: '800',
  },
  profileBadge: {
    backgroundColor: '#7c4dff',
    color: '#fff',
    fontSize: 10,
    fontWeight: '700',
    paddingHorizontal: 8,
    paddingVertical: 2,
    borderRadius: 4,
    marginLeft: 10,
    overflow: 'hidden',
  },
  setpointDisplay: {
    alignItems: 'center',
    marginBottom: 16,
  },
  setpointLabel: {
    color: '#888',
    fontSize: 11,
  },
  setpointValue: {
    color: '#fff',
    fontSize: 18,
    fontWeight: '600',
  },
  buttonRow: {
    flexDirection: 'row',
    justifyContent: 'space-evenly',
    marginBottom: 16,
  },
  btn: {
    paddingVertical: 14,
    paddingHorizontal: 24,
    borderRadius: 12,
    alignItems: 'center',
    minWidth: 90,
  },
  btnCool: {
    backgroundColor: '#1a3a5c',
    borderWidth: 2,
    borderColor: '#29b6f6',
  },
  btnHeat: {
    backgroundColor: '#5c1a1a',
    borderWidth: 2,
    borderColor: '#ef5350',
  },
  btnStop: {
    backgroundColor: '#333',
    borderWidth: 2,
    borderColor: '#666',
  },
  btnActive: {
    opacity: 1,
    shadowColor: '#fff',
    shadowOffset: { width: 0, height: 0 },
    shadowOpacity: 0.3,
    shadowRadius: 8,
    elevation: 8,
  },
  btnText: {
    color: '#fff',
    fontSize: 16,
    fontWeight: '700',
  },
  btnSub: {
    color: '#aaa',
    fontSize: 11,
    marginTop: 2,
  },
  profileBtn: {
    backgroundColor: '#2a1a4e',
    borderWidth: 2,
    borderColor: '#7c4dff',
    borderRadius: 12,
    paddingVertical: 14,
    alignItems: 'center',
  },
  profileBtnActive: {
    backgroundColor: '#4a1a2e',
    borderColor: '#ef5350',
  },
  profileBtnText: {
    color: '#fff',
    fontSize: 15,
    fontWeight: '700',
  },
  profileBtnSub: {
    color: '#aaa',
    fontSize: 11,
    marginTop: 2,
  },
});

export default ModeControl;
