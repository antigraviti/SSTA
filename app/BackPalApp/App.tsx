/**
 * BackPal Companion App — Root Component
 * ONYX Bethlehem Labs + SSTA IP Holdings
 */

import React from 'react';
import { StatusBar } from 'expo-status-bar';
import { NavigationContainer } from '@react-navigation/native';
import { createBottomTabNavigator } from '@react-navigation/bottom-tabs';
import { View, Text, StyleSheet } from 'react-native';
import DashboardScreen from './src/screens/DashboardScreen';
import SettingsScreen from './src/screens/SettingsScreen';

const Tab = createBottomTabNavigator();

// Simple text-based tab icons (no icon library dependency)
const TabIcon = ({ label, focused }: { label: string; focused: boolean }) => (
  <Text style={[styles.tabIcon, focused && styles.tabIconActive]}>{label}</Text>
);

export default function App() {
  return (
    <NavigationContainer
      theme={{
        dark: true,
        colors: {
          primary: '#4fc3f7',
          background: '#0a0a1a',
          card: '#0f0f24',
          text: '#fff',
          border: '#1a1a3a',
          notification: '#ef5350',
        },
        fonts: {
          regular: { fontFamily: 'System', fontWeight: '400' as const },
          medium: { fontFamily: 'System', fontWeight: '500' as const },
          bold: { fontFamily: 'System', fontWeight: '700' as const },
          heavy: { fontFamily: 'System', fontWeight: '900' as const },
        },
      }}
    >
      <StatusBar style="light" />
      <Tab.Navigator
        screenOptions={{
          headerStyle: { backgroundColor: '#0f0f24' },
          headerTintColor: '#fff',
          headerTitleStyle: { fontWeight: '700' },
          tabBarStyle: {
            backgroundColor: '#0f0f24',
            borderTopColor: '#1a1a3a',
          },
          tabBarActiveTintColor: '#4fc3f7',
          tabBarInactiveTintColor: '#666',
        }}
      >
        <Tab.Screen
          name="Dashboard"
          component={DashboardScreen}
          options={{
            headerTitle: 'BackPal',
            tabBarIcon: ({ focused }) => <TabIcon label="T" focused={focused} />,
          }}
        />
        <Tab.Screen
          name="Settings"
          component={SettingsScreen}
          options={{
            headerTitle: 'Settings',
            tabBarIcon: ({ focused }) => <TabIcon label="S" focused={focused} />,
          }}
        />
      </Tab.Navigator>
    </NavigationContainer>
  );
}

const styles = StyleSheet.create({
  tabIcon: {
    fontSize: 18,
    fontWeight: '800',
    color: '#666',
  },
  tabIconActive: {
    color: '#4fc3f7',
  },
});
