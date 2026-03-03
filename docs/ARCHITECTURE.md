# BackPal Prototype — System Architecture
**ONYX Bethlehem Labs + SSTA IP Holdings**
**Revision:** 1.0 — March 2026

---

## SYSTEM OVERVIEW

```
┌─────────────────────────────────────────────────────────┐
│                   MOBILE APP (React Native)              │
│   ┌────────────┐  ┌────────────┐  ┌─────────────────┐  │
│   │ Dashboard   │  │  Controls  │  │   Settings/PID  │  │
│   │ T1–T6 live  │  │ COOL/HEAT  │  │   Kp, Ki, Kd    │  │
│   │ Power/SOC   │  │ STOP/PROFILE│ │  Setpoints      │  │
│   └──────┬──────┘  └─────┬──────┘  └───────┬─────────┘  │
│          └───────────────┼──────────────────┘            │
│                          │ BLE GATT                      │
└──────────────────────────┼───────────────────────────────┘
                           │
┌──────────────────────────┼───────────────────────────────┐
│                   ESP32 CONTROLLER                        │
│                          │                                │
│   ┌──────────────────────▼──────────────────────────┐    │
│   │              BLE Service Layer                    │    │
│   │  Temps (notify) │ Control (r/w) │ Status (notify)│    │
│   └──────┬──────────┴───────────────┴──────┬─────────┘    │
│          │                                  │             │
│   ┌──────▼──────┐    ┌─────────────┐   ┌──▼────────┐    │
│   │   Thermal   │    │    PID       │   │  Session  │    │
│   │   Sensing   │───▶│  Controller  │──▶│  Manager  │    │
│   │  T1–T6 ADC  │    │  Gain Sched  │   │ TEMP_HOLD │    │
│   │  INA219 I²C │    │  Melt Throt  │   │ Profiles  │    │
│   └─────────────┘    └──────┬───────┘   └───────────┘    │
│                             │ duty (0–255)                │
│                      ┌──────▼───────┐                     │
│                      │  TEC Driver  │                     │
│                      │  IBT-2 H-Brg │                     │
│                      │  Const DC    │                     │
│                      └──────┬───────┘                     │
│                             │                             │
│                      ┌──────▼───────┐                     │
│                      │   SAFETY     │                     │
│                      │ T1>45°C stop │                     │
│                      │ T1<5°C stop  │                     │
│                      │ I>6A stop    │                     │
│                      └──────────────┘                     │
└───────────────────────────────────────────────────────────┘
                           │
┌──────────────────────────┼───────────────────────────────┐
│                   THERMAL ASSEMBLY                        │
│                          │                                │
│   ┌──────────────────────▼──────────────────────────┐    │
│   │            TEC1-12706 (40×40mm)                  │    │
│   │         Cold side UP ← → Hot side DOWN           │    │
│   └──────────┬─────────────────────┬─────────────────┘    │
│              │                     │                      │
│   ┌──────────▼────────┐  ┌────────▼──────────────┐       │
│   │ Cu Spreader 90×90 │  │  SSTC Vault v1B       │       │
│   │ Silicone 0.9mm    │  │  51×51×31mm, 36g PCM  │       │
│   │ ──── SKIN ────    │  │  Al wool scaffold     │       │
│   └───────────────────┘  │  ~9 kJ thermal budget │       │
│                          └───────────────────────┘       │
└───────────────────────────────────────────────────────────┘
```

---

## KEY ARCHITECTURAL DECISIONS

| Decision | Choice | Rationale |
|----------|--------|-----------|
| MCU | ESP32-WROOM-32 | BLE + WiFi built-in, dual-core 240MHz, Arduino-compatible. Replaces Arduino Nano for wireless connectivity. |
| TEC Drive | Constant optimized DC via IBT-2 | 4.5× better stability than PWM (validated Feb 28, 2026). |
| PID Feedback | T6 (prototype), T1 (production) | T6 on silicone surface directly measures what patient feels. Production uses T1 + profiled R-value. |
| Safety Sensor | T1 (skin-side) | Closest to patient. Hard cutoff at 45°C. |
| Timer Logic | TEMP_THEN_HOLD | Therapeutic time counts only when T1 within ±2°C of target. Clinically meaningful. |
| Control Loop | 10Hz | Prevents 2–3s overshoot seen at 1Hz. |
| Mobile App | React Native (Expo) | Cross-platform. Leverages existing React experience from web interface. |
| BLE Protocol | Custom GATT with 4 characteristics | Packed binary structs for low latency. Temp + Status notify at 1Hz. |

---

## CONTROL ARCHITECTURE — E-01 DIFFERENTIATORS

The PID controller implements thermal-state-aware control to distinguish from prior art (US 11,419,753):

1. **Gain Scheduling** — PID gains adapt based on vault phase (solid / transition / liquid). During phase transition, the PCM plateau provides natural buffering; in liquid phase, lower Kp prevents overshoot.

2. **Melt Fraction Throttling** — As vault melt fraction exceeds 70%, TEC drive is linearly reduced to 30%. This preserves vault capacity for multi-cycle contrast therapy.

3. **Runtime Prediction** — Vault SOC (state-of-charge) combined with current power draw estimates remaining runtime. Alerts at <20% and <5% SOC.

4. **Scaffold Saturation Detection** — When melt fraction exceeds 95%, the system flags saturation. This protects the scaffold from thermal overload.

---

## BLE GATT STRUCTURE

**Service UUID:** `4fafc201-1fb5-459e-8fcc-c5c9c331914b`

| Characteristic | UUID | Properties | Payload |
|---------------|------|------------|---------|
| Temperatures | `...26a8` | Read, Notify | 6× float32 (T1–T6) + uint32 timestamp = 28 bytes |
| Control | `...26a9` | Read, Write | uint8 cmd + uint8 power + float32 setpoint + uint8 profileId = 7 bytes |
| Status | `...26aa` | Read, Notify | Mode, vault SOC, melt fraction, power, session timers = 35 bytes |
| PID Tuning | `...26ab` | Read, Write | 4× float32 (Kp,Ki,Kd,setpoint) + 2× float32 (integral,output) = 24 bytes |

---

## FILE STRUCTURE

```
SSTA/
├── CLAUDE.md                          # Project root instructions
├── ACTIVE_WORK.md                     # Open TODOs and current issues
├── DECISIONS.md                       # Resolved technical decisions
├── HARDWARE.md                        # Equipment specs and validated results
├── PORTFOLIO.md                       # Patent portfolio status
├── docs/
│   ├── ARCHITECTURE.md                # This file
│   └── BACKPAL_BOM.md                 # Component bill of materials
├── firmware/
│   └── backpal-esp32/
│       ├── platformio.ini             # PlatformIO build config
│       ├── include/
│       │   ├── config.h               # Pin defs, constants, BLE UUIDs
│       │   ├── thermal.h              # Sensor types and thermal state
│       │   ├── tec_driver.h           # IBT-2 H-bridge driver
│       │   ├── pid_controller.h       # PID with gain scheduling
│       │   ├── ble_service.h          # BLE GATT service
│       │   └── session.h              # Therapeutic session manager
│       └── src/
│           ├── main.cpp               # Application entry, 10Hz loop, 1Hz telemetry
│           ├── thermal.cpp            # ADC→°C, oversampling, state estimation
│           ├── tec_driver.cpp         # Constant DC drive via LEDC
│           ├── pid_controller.cpp     # PID compute, throttle, gain scheduling
│           ├── ble_service.cpp        # BLE init, notifications, command parsing
│           └── session.cpp            # TEMP_THEN_HOLD timer, profile execution
└── app/
    └── BackPalApp/
        ├── package.json               # Expo + React Native + BLE dependencies
        ├── app.json                   # Expo config, BLE permissions
        ├── tsconfig.json              # TypeScript config
        ├── App.tsx                    # Root: tab navigator (Dashboard, Settings)
        ├── index.js                   # Entry point
        └── src/
            ├── services/
            │   └── BackPalBLE.ts      # BLE manager: scan, connect, parse, commands
            ├── screens/
            │   ├── DashboardScreen.tsx # Live temps, power, session, controls
            │   └── SettingsScreen.tsx  # PID tuning, setpoints, device info
            └── components/
                ├── TemperatureGauge.tsx # Circular temp display with color coding
                ├── StatusBar.tsx        # Connection, battery, vault SOC
                ├── ModeControl.tsx      # COOL/HEAT/STOP buttons, profile launcher
                └── SessionInfo.tsx      # Phase progress, timers, melt fraction
```

---

## NEXT STEPS

### Firmware
- [ ] Flash ESP32 and validate sensor readings (T1–T6 ADC calibration)
- [ ] Tune PID gains on hardware with T6 feedback
- [ ] Validate HEAT and COOL modes with IBT-2 (resolve HW-01 if needed)
- [ ] Run BackPal Clinical profile end-to-end
- [ ] Add serial CSV logging with SHA-256 hash for patent documentation

### Mobile App
- [ ] Install Expo development build on test device
- [ ] Verify BLE connection and characteristic subscriptions
- [ ] Test command write (COOL/HEAT/STOP) end-to-end
- [ ] Add temperature history chart (time-series plot)
- [ ] Add manual power slider for open-loop testing

### Integration
- [ ] Validate BLE packet parsing matches firmware structs (byte alignment)
- [ ] Measure BLE notification latency (target <100ms)
- [ ] Test reconnection after BLE dropout
- [ ] Full clinical profile test with app monitoring
