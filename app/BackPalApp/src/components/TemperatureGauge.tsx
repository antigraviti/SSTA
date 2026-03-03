/**
 * TemperatureGauge — Circular gauge component for displaying a temperature reading
 * ONYX Bethlehem Labs + SSTA IP Holdings
 */

import React from 'react';
import { View, Text, StyleSheet } from 'react-native';

interface Props {
  label: string;
  value: number | null;
  unit?: string;
  min?: number;
  max?: number;
  alertHigh?: number;
  alertLow?: number;
  size?: number;
}

const TemperatureGauge: React.FC<Props> = ({
  label,
  value,
  unit = '°C',
  min = 0,
  max = 60,
  alertHigh = 45,
  alertLow = 5,
  size = 90,
}) => {
  const isValid = value !== null && value > -900;
  const displayValue = isValid ? value!.toFixed(1) : '--.-';

  let color = '#4fc3f7';  // Default: cool blue
  if (isValid) {
    if (value! >= alertHigh) color = '#ef5350';       // Red: danger
    else if (value! >= 35) color = '#ff9800';          // Orange: warm
    else if (value! <= alertLow) color = '#7c4dff';    // Purple: very cold
    else if (value! <= 15) color = '#29b6f6';          // Light blue: cold
  }

  const invalidColor = '#555';

  return (
    <View style={[styles.container, { width: size, height: size }]}>
      <View style={[
        styles.ring,
        {
          width: size,
          height: size,
          borderRadius: size / 2,
          borderColor: isValid ? color : invalidColor,
        }
      ]}>
        <Text style={[styles.value, { color: isValid ? color : invalidColor, fontSize: size * 0.22 }]}>
          {displayValue}
        </Text>
        <Text style={[styles.unit, { color: isValid ? color : invalidColor }]}>{unit}</Text>
      </View>
      <Text style={styles.label}>{label}</Text>
    </View>
  );
};

const styles = StyleSheet.create({
  container: {
    alignItems: 'center',
    margin: 4,
  },
  ring: {
    borderWidth: 3,
    justifyContent: 'center',
    alignItems: 'center',
  },
  value: {
    fontWeight: '700',
    fontVariant: ['tabular-nums'],
  },
  unit: {
    fontSize: 10,
    opacity: 0.7,
  },
  label: {
    color: '#aaa',
    fontSize: 11,
    marginTop: 4,
    fontWeight: '500',
  },
});

export default TemperatureGauge;
