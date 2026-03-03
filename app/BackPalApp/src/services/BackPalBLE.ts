/**
 * BackPal BLE Service — React Native BLE communication layer
 * ONYX Bethlehem Labs + SSTA IP Holdings
 *
 * Connects to BackPal ESP32 controller via BLE GATT.
 * Handles characteristic subscriptions, command writes, and data parsing.
 */

import { BleManager, Device, Characteristic, BleError } from 'react-native-ble-plx';
import { Buffer } from 'buffer';

// Must match firmware config.h UUIDs
const SERVICE_UUID = '4fafc201-1fb5-459e-8fcc-c5c9c331914b';
const CHAR_TEMPS_UUID = 'beb5483e-36e1-4688-b7f5-ea07361b26a8';
const CHAR_CONTROL_UUID = 'beb5483e-36e1-4688-b7f5-ea07361b26a9';
const CHAR_STATUS_UUID = 'beb5483e-36e1-4688-b7f5-ea07361b26aa';
const CHAR_PID_UUID = 'beb5483e-36e1-4688-b7f5-ea07361b26ab';

const DEVICE_NAME = 'BackPal-01';

// --- Data Types (must match firmware packed structs) ---

export interface TemperatureData {
  t1: number; t2: number; t3: number;
  t4: number; t5: number; t6: number;
  timestamp: number;
}

export interface StatusData {
  mode: number;           // 0=OFF, 1=COOL, 2=HEAT
  profileActive: boolean;
  phaseIndex: number;
  setpoint: number;
  vaultSOC: number;       // 0.0–1.0
  meltFraction: number;   // 0.0–1.0
  runtimeRemainS: number;
  currentAmps: number;
  powerWatts: number;
  batteryPct: number;     // 0–100
  sessionElapsedS: number;
  therapeuticS: number;
  safetyFlags: number;
}

export interface PIDData {
  Kp: number; Ki: number; Kd: number;
  setpoint: number;
  integral: number;
  output: number;
}

export type TECMode = 'OFF' | 'COOL' | 'HEAT';
export type ConnectionState = 'disconnected' | 'scanning' | 'connecting' | 'connected';

export interface BackPalCallbacks {
  onTemperatures?: (data: TemperatureData) => void;
  onStatus?: (data: StatusData) => void;
  onConnectionChange?: (state: ConnectionState) => void;
  onError?: (error: string) => void;
}

class BackPalBLE {
  private manager: BleManager;
  private device: Device | null = null;
  private callbacks: BackPalCallbacks = {};
  private connectionState: ConnectionState = 'disconnected';

  constructor() {
    this.manager = new BleManager();
  }

  setCallbacks(cb: BackPalCallbacks) {
    this.callbacks = cb;
  }

  getConnectionState(): ConnectionState {
    return this.connectionState;
  }

  private setConnectionState(state: ConnectionState) {
    this.connectionState = state;
    this.callbacks.onConnectionChange?.(state);
  }

  // --- Scanning & Connection ---

  async scanAndConnect(): Promise<void> {
    this.setConnectionState('scanning');

    return new Promise((resolve, reject) => {
      const timeout = setTimeout(() => {
        this.manager.stopDeviceScan();
        this.setConnectionState('disconnected');
        reject(new Error('Scan timeout — no BackPal device found'));
      }, 15000);

      this.manager.startDeviceScan(
        [SERVICE_UUID],
        { allowDuplicates: false },
        async (error: BleError | null, scannedDevice: Device | null) => {
          if (error) {
            clearTimeout(timeout);
            this.setConnectionState('disconnected');
            this.callbacks.onError?.(`Scan error: ${error.message}`);
            reject(error);
            return;
          }

          if (scannedDevice?.name === DEVICE_NAME || scannedDevice?.localName === DEVICE_NAME) {
            clearTimeout(timeout);
            this.manager.stopDeviceScan();

            try {
              this.setConnectionState('connecting');
              await this.connectToDevice(scannedDevice);
              resolve();
            } catch (e) {
              this.setConnectionState('disconnected');
              reject(e);
            }
          }
        }
      );
    });
  }

  private async connectToDevice(scannedDevice: Device): Promise<void> {
    const connected = await scannedDevice.connect({ autoConnect: true });
    await connected.discoverAllServicesAndCharacteristics();
    this.device = connected;

    // Monitor disconnection
    this.device.onDisconnected((_error: BleError | null) => {
      this.device = null;
      this.setConnectionState('disconnected');
    });

    // Subscribe to temperature notifications
    this.device.monitorCharacteristicForService(
      SERVICE_UUID,
      CHAR_TEMPS_UUID,
      (error: BleError | null, char: Characteristic | null) => {
        if (error || !char?.value) return;
        const data = this.parseTemperatures(char.value);
        if (data) this.callbacks.onTemperatures?.(data);
      }
    );

    // Subscribe to status notifications
    this.device.monitorCharacteristicForService(
      SERVICE_UUID,
      CHAR_STATUS_UUID,
      (error: BleError | null, char: Characteristic | null) => {
        if (error || !char?.value) return;
        const data = this.parseStatus(char.value);
        if (data) this.callbacks.onStatus?.(data);
      }
    );

    this.setConnectionState('connected');
  }

  async disconnect(): Promise<void> {
    if (this.device) {
      await this.device.cancelConnection();
      this.device = null;
    }
    this.setConnectionState('disconnected');
  }

  // --- Commands ---

  async sendCommand(mode: TECMode, power: number = 128, setpoint: number = 0, profileId: number = 0): Promise<void> {
    if (!this.device) throw new Error('Not connected');

    const cmdByte = mode === 'OFF' ? 0 : mode === 'COOL' ? 1 : mode === 'HEAT' ? 2 : 3;

    // Pack BLEControlCmd: uint8 command, uint8 power, float32 setpoint, uint8 profileId
    const buf = Buffer.alloc(7);
    buf.writeUInt8(cmdByte, 0);
    buf.writeUInt8(power, 1);
    buf.writeFloatLE(setpoint, 2);
    buf.writeUInt8(profileId, 6);

    await this.device.writeCharacteristicWithResponseForService(
      SERVICE_UUID,
      CHAR_CONTROL_UUID,
      buf.toString('base64')
    );
  }

  async startClinicalProfile(): Promise<void> {
    return this.sendCommand('HEAT', 0, 0, 1);  // command=3 (PROFILE), profileId=1
  }

  async stopDevice(): Promise<void> {
    return this.sendCommand('OFF');
  }

  async setCoolMode(setpoint: number): Promise<void> {
    return this.sendCommand('COOL', 0, setpoint);
  }

  async setHeatMode(setpoint: number): Promise<void> {
    return this.sendCommand('HEAT', 0, setpoint);
  }

  async setManualPower(mode: TECMode, power: number): Promise<void> {
    return this.sendCommand(mode, power, 0);
  }

  async updatePID(Kp: number, Ki: number, Kd: number, setpoint: number): Promise<void> {
    if (!this.device) throw new Error('Not connected');

    // Pack BLEPIDPacket: 4× float32 (Kp, Ki, Kd, setpoint) + 2× float32 read-only
    const buf = Buffer.alloc(24);
    buf.writeFloatLE(Kp, 0);
    buf.writeFloatLE(Ki, 4);
    buf.writeFloatLE(Kd, 8);
    buf.writeFloatLE(setpoint, 12);
    // integral and output are read-only, fill with zeros
    buf.writeFloatLE(0, 16);
    buf.writeFloatLE(0, 20);

    await this.device.writeCharacteristicWithResponseForService(
      SERVICE_UUID,
      CHAR_PID_UUID,
      buf.toString('base64')
    );
  }

  // --- Parsers ---

  private parseTemperatures(base64: string): TemperatureData | null {
    try {
      const buf = Buffer.from(base64, 'base64');
      if (buf.length < 28) return null;

      return {
        t1: buf.readFloatLE(0),
        t2: buf.readFloatLE(4),
        t3: buf.readFloatLE(8),
        t4: buf.readFloatLE(12),
        t5: buf.readFloatLE(16),
        t6: buf.readFloatLE(20),
        timestamp: buf.readUInt32LE(24),
      };
    } catch {
      return null;
    }
  }

  private parseStatus(base64: string): StatusData | null {
    try {
      const buf = Buffer.from(base64, 'base64');
      if (buf.length < 35) return null;

      let offset = 0;
      const mode = buf.readUInt8(offset); offset += 1;
      const profileActive = buf.readUInt8(offset) !== 0; offset += 1;
      const phaseIndex = buf.readUInt8(offset); offset += 1;
      const setpoint = buf.readFloatLE(offset); offset += 4;
      const vaultSOC = buf.readFloatLE(offset); offset += 4;
      const meltFraction = buf.readFloatLE(offset); offset += 4;
      const runtimeRemainS = buf.readFloatLE(offset); offset += 4;
      const currentAmps = buf.readFloatLE(offset); offset += 4;
      const powerWatts = buf.readFloatLE(offset); offset += 4;
      const batteryPct = buf.readFloatLE(offset); offset += 4;
      const sessionElapsedS = buf.readUInt16LE(offset); offset += 2;
      const therapeuticS = buf.readUInt16LE(offset); offset += 2;
      const safetyFlags = buf.readUInt8(offset);

      return {
        mode, profileActive, phaseIndex, setpoint,
        vaultSOC, meltFraction, runtimeRemainS,
        currentAmps, powerWatts, batteryPct,
        sessionElapsedS, therapeuticS, safetyFlags,
      };
    } catch {
      return null;
    }
  }

  destroy() {
    this.manager.destroy();
  }
}

// Singleton instance
export const backpalBLE = new BackPalBLE();
export default backpalBLE;
