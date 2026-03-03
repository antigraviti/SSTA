/**
 * SettingsScreen — PID tuning, setpoint adjustment, device info
 * ONYX Bethlehem Labs + SSTA IP Holdings
 */

import React, { useState } from 'react';
import {
  View, Text, ScrollView, StyleSheet,
  TextInput, TouchableOpacity, Alert,
} from 'react-native';
import backpalBLE from '../services/BackPalBLE';

const SettingsScreen: React.FC = () => {
  const [kp, setKp] = useState('15.0');
  const [ki, setKi] = useState('0.8');
  const [kd, setKd] = useState('0.0');
  const [coolTarget, setCoolTarget] = useState('9.0');
  const [heatTarget, setHeatTarget] = useState('40.0');

  const sendPID = async () => {
    const kpVal = parseFloat(kp);
    const kiVal = parseFloat(ki);
    const kdVal = parseFloat(kd);

    if (isNaN(kpVal) || isNaN(kiVal) || isNaN(kdVal)) {
      Alert.alert('Invalid Input', 'Enter numeric values for PID gains.');
      return;
    }

    try {
      await backpalBLE.updatePID(kpVal, kiVal, kdVal, 0);
      Alert.alert('PID Updated', `Kp=${kpVal} Ki=${kiVal} Kd=${kdVal}`);
    } catch (e: any) {
      Alert.alert('Error', e.message);
    }
  };

  const sendCoolSetpoint = async () => {
    const val = parseFloat(coolTarget);
    if (isNaN(val) || val < 0 || val > 30) {
      Alert.alert('Invalid', 'Cool target must be 0–30°C');
      return;
    }
    try {
      await backpalBLE.setCoolMode(val);
    } catch (e: any) {
      Alert.alert('Error', e.message);
    }
  };

  const sendHeatSetpoint = async () => {
    const val = parseFloat(heatTarget);
    if (isNaN(val) || val < 25 || val > 44) {
      Alert.alert('Invalid', 'Heat target must be 25–44°C (safety limit 45°C)');
      return;
    }
    try {
      await backpalBLE.setHeatMode(val);
    } catch (e: any) {
      Alert.alert('Error', e.message);
    }
  };

  return (
    <ScrollView style={styles.root} contentContainerStyle={styles.content}>
      {/* PID Tuning */}
      <Text style={styles.sectionTitle}>PID TUNING</Text>
      <View style={styles.card}>
        <View style={styles.inputRow}>
          <View style={styles.inputGroup}>
            <Text style={styles.inputLabel}>Kp</Text>
            <TextInput
              style={styles.input}
              value={kp}
              onChangeText={setKp}
              keyboardType="numeric"
              placeholderTextColor="#555"
            />
          </View>
          <View style={styles.inputGroup}>
            <Text style={styles.inputLabel}>Ki</Text>
            <TextInput
              style={styles.input}
              value={ki}
              onChangeText={setKi}
              keyboardType="numeric"
              placeholderTextColor="#555"
            />
          </View>
          <View style={styles.inputGroup}>
            <Text style={styles.inputLabel}>Kd</Text>
            <TextInput
              style={styles.input}
              value={kd}
              onChangeText={setKd}
              keyboardType="numeric"
              placeholderTextColor="#555"
            />
          </View>
        </View>
        <TouchableOpacity style={styles.sendBtn} onPress={sendPID}>
          <Text style={styles.sendBtnText}>SEND PID GAINS</Text>
        </TouchableOpacity>
      </View>

      {/* Temperature Setpoints */}
      <Text style={styles.sectionTitle}>TEMPERATURE SETPOINTS</Text>
      <View style={styles.card}>
        <View style={styles.setpointRow}>
          <View style={styles.setpointGroup}>
            <Text style={[styles.inputLabel, { color: '#29b6f6' }]}>COOL TARGET (°C)</Text>
            <TextInput
              style={[styles.input, styles.inputWide]}
              value={coolTarget}
              onChangeText={setCoolTarget}
              keyboardType="numeric"
              placeholderTextColor="#555"
            />
            <TouchableOpacity style={[styles.sendBtn, styles.btnSmall]} onPress={sendCoolSetpoint}>
              <Text style={styles.sendBtnText}>SET COOL</Text>
            </TouchableOpacity>
          </View>
          <View style={styles.setpointGroup}>
            <Text style={[styles.inputLabel, { color: '#ef5350' }]}>HEAT TARGET (°C)</Text>
            <TextInput
              style={[styles.input, styles.inputWide]}
              value={heatTarget}
              onChangeText={setHeatTarget}
              keyboardType="numeric"
              placeholderTextColor="#555"
            />
            <TouchableOpacity style={[styles.sendBtn, styles.btnSmall]} onPress={sendHeatSetpoint}>
              <Text style={styles.sendBtnText}>SET HEAT</Text>
            </TouchableOpacity>
          </View>
        </View>
      </View>

      {/* Device Info */}
      <Text style={styles.sectionTitle}>DEVICE INFO</Text>
      <View style={styles.card}>
        <InfoRow label="Firmware" value="v3.0.0 (ESP32)" />
        <InfoRow label="MCU" value="ESP32-WROOM-32" />
        <InfoRow label="TEC Module" value="TEC1-12706 (40x40mm)" />
        <InfoRow label="Vault" value="SSTC v1B (51x51x31mm)" />
        <InfoRow label="PCM" value="Paraffin 36.15g (Tm ~55°C)" />
        <InfoRow label="Thermal Budget" value="~9 kJ" />
        <InfoRow label="Control Loop" value="10Hz" />
        <InfoRow label="PID Feedback" value="T6 (silicone surface)" />
        <InfoRow label="Safety Limit" value="T1 > 45°C cutoff" />
        <InfoRow label="Drive Method" value="Constant optimized DC" />
      </View>

      <View style={styles.footer}>
        <Text style={styles.footerText}>BackPal Contrast Therapy Device</Text>
        <Text style={styles.footerSub}>ONYX Bethlehem Labs + SSTA IP Holdings</Text>
      </View>
    </ScrollView>
  );
};

const InfoRow: React.FC<{ label: string; value: string }> = ({ label, value }) => (
  <View style={styles.infoRow}>
    <Text style={styles.infoLabel}>{label}</Text>
    <Text style={styles.infoValue}>{value}</Text>
  </View>
);

const styles = StyleSheet.create({
  root: {
    flex: 1,
    backgroundColor: '#0a0a1a',
  },
  content: {
    padding: 16,
    paddingBottom: 48,
  },
  sectionTitle: {
    color: '#888',
    fontSize: 12,
    fontWeight: '600',
    letterSpacing: 1,
    marginTop: 20,
    marginBottom: 8,
  },
  card: {
    backgroundColor: '#1a1a2e',
    borderRadius: 12,
    padding: 16,
  },
  inputRow: {
    flexDirection: 'row',
    justifyContent: 'space-between',
    marginBottom: 12,
  },
  inputGroup: {
    flex: 1,
    marginHorizontal: 4,
  },
  inputLabel: {
    color: '#aaa',
    fontSize: 11,
    marginBottom: 4,
    fontWeight: '500',
  },
  input: {
    backgroundColor: '#0a0a1a',
    borderWidth: 1,
    borderColor: '#333',
    borderRadius: 8,
    color: '#fff',
    fontSize: 16,
    paddingHorizontal: 12,
    paddingVertical: 8,
    textAlign: 'center',
    fontVariant: ['tabular-nums'],
  },
  inputWide: {
    textAlign: 'left',
  },
  sendBtn: {
    backgroundColor: '#2a4a6a',
    borderRadius: 8,
    paddingVertical: 10,
    alignItems: 'center',
    marginTop: 8,
  },
  btnSmall: {
    paddingVertical: 8,
  },
  sendBtnText: {
    color: '#4fc3f7',
    fontSize: 13,
    fontWeight: '700',
  },
  setpointRow: {
    flexDirection: 'row',
    gap: 16,
  },
  setpointGroup: {
    flex: 1,
  },
  infoRow: {
    flexDirection: 'row',
    justifyContent: 'space-between',
    paddingVertical: 6,
    borderBottomWidth: 1,
    borderBottomColor: '#222',
  },
  infoLabel: {
    color: '#888',
    fontSize: 13,
  },
  infoValue: {
    color: '#fff',
    fontSize: 13,
    fontWeight: '500',
  },
  footer: {
    alignItems: 'center',
    marginTop: 32,
  },
  footerText: {
    color: '#555',
    fontSize: 13,
  },
  footerSub: {
    color: '#444',
    fontSize: 11,
    marginTop: 2,
  },
});

export default SettingsScreen;
