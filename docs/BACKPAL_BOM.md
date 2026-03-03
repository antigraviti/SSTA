# BackPal Contrast Therapy Device — Prototype BOM
**ONYX Bethlehem Labs + SSTA IP Holdings**
**Revision:** 1.0 — March 2026
**Architecture:** ESP32 + IBT-2 + TEC1-12706 + SSTC v1B Vault

---

## ELECTRONICS

| # | Component | Part Number / Spec | Qty | Notes |
|---|-----------|-------------------|-----|-------|
| 1 | MCU | ESP32-WROOM-32 DevKitC v4 | 1 | Dual-core 240MHz, BLE 4.2 + WiFi, 34 GPIO. Replaces Arduino Nano for BLE connectivity. |
| 2 | H-Bridge | IBT-2 (BTS7960 dual) | 1 | Bidirectional TEC drive, 43A max. RPWM/LPWM + R_EN/L_EN control. |
| 3 | TEC Module | TEC1-12706 | 1 | 40×40mm Peltier, 6A max, 15.4V max. Cold side UP (skin), hot side DOWN (vault). |
| 4 | Current Sensor | INA219 breakout (I²C) | 1 | TEC current/power monitoring. I²C address 0x40. |
| 5 | Thermistors | MF55-103F-B3950 (NTC 10kΩ) | 6 | T1–T6 temperature sensing. B=3950K. Voltage divider with 10kΩ reference. |
| 6 | Battery | 3S1P LiPo (11.1V nominal) | 1 | ~2200mAh. Provides direct TEC drive voltage. |
| 7 | Battery Management | 3S BMS board | 1 | Balancing + over-discharge/charge protection. |
| 8 | Voltage Regulator | AMS1117-3.3 or onboard ESP32 | 1 | 3.3V rail for ESP32 from battery. DevKitC has onboard regulator from 5V USB. |
| 9 | Buck Converter | Mini-360 (MP2307) or equiv. | 1 | Step-down 11.1V → 5V for ESP32 VIN and logic supply. |
| 10 | Battery Voltage Divider | 100kΩ + 33kΩ resistors | 1 set | Divides battery voltage (9.0–12.6V) to ESP32 ADC range (0–3.3V). |
| 11 | Reference Resistors | 10kΩ ±1% | 6 | Thermistor voltage divider pairs (one per channel). |
| 12 | Decoupling Caps | 100nF ceramic | 6 | One per thermistor ADC input for noise filtering. |
| 13 | Pushbutton | Momentary tactile switch | 1 | User input: mode cycle / start-stop. |
| 14 | Status LED | RGB common-cathode 5mm | 1 | Mode indication: blue=COOL, red=HEAT, green=IDLE, blink=BLE connected. |
| 15 | Connectors | JST-XH 2.54mm headers | Assorted | Battery, thermistor, TEC connections. |

---

## THERMAL ASSEMBLY

| # | Component | Spec | Qty | Notes |
|---|-----------|------|-----|-------|
| 16 | SSTC Vault v1B | 51×51×31mm aluminum, 36.15g paraffin, Al wool scaffold | 1 | Primary thermal capacitor. ~9 kJ total budget. |
| 17 | Copper Spreader | 90×90×1.5mm Cu sheet | 1 | Thermal leverage λ ≈ 5.06. Bridges 40mm TEC to skin area. |
| 18 | Silicone Interface | 90×90×0.9mm medical-grade silicone | 1 | Skin-contact layer. ~4.4°C penalty with spreader (vs. ~22°C without). |
| 19 | Thermal Interface Material | Arctic MX-4 or equiv. | 2 joints | TEC↔vault face, TEC↔spreader face. |
| 20 | Heatsink (internal) | Aluminum, sized to vault | 1 | Hot-side thermal management within enclosure. |

---

## MECHANICAL / ENCLOSURE

| # | Component | Spec | Qty | Notes |
|---|-----------|------|-----|-------|
| 21 | Enclosure | 110×175×55mm, 3D-printed or CNC | 1 | Prototype housing. |
| 22 | Mounting Hardware | M3 screws, standoffs | Assorted | PCB and vault mounting. |
| 23 | Strap / Harness | Elastic with hook-and-loop | 1 | Body-worn attachment. |

---

## SENSOR PLACEMENT (PROTOTYPE)

| Channel | ADC Pin | Location | Purpose |
|---------|---------|----------|---------|
| T1 | GPIO36 (ADC1_CH0) | Skin-side spreader surface | Safety limit (>45°C cutoff). Production control sensor. |
| T2 | GPIO39 (ADC1_CH3) | TEC hot face / vault contact | TEC reference, fast thermal response. |
| T3 | GPIO34 (ADC1_CH6) | TEC cold face / heatsink | TEC ΔT measurement. Known reliability issues. |
| T4 | GPIO35 (ADC1_CH7) | Vault center (embedded in PCM) | PCM core temp, melt fraction proxy. |
| T5 | GPIO32 (ADC1_CH4) | Vault edge (~1mm from wall) | PCM gradient, edge effects. |
| T6 | GPIO33 (ADC1_CH5) | On silicone layer (atop Cu spreader) | PID control sensor (prototype only). Replaced by T1 + R-value model in production. |

**Note:** ESP32 ADC2 channels cannot be used while WiFi/BLE is active. All thermistors use ADC1 channels only.

---

## PIN ASSIGNMENT — ESP32

| Function | GPIO | Direction | Notes |
|----------|------|-----------|-------|
| IBT-2 RPWM | GPIO25 | OUT | Forward drive (COOL mode) |
| IBT-2 LPWM | GPIO26 | OUT | Reverse drive (HEAT mode) |
| IBT-2 R_EN | GPIO27 | OUT | Right enable (HIGH to enable) |
| IBT-2 L_EN | GPIO14 | OUT | Left enable (HIGH to enable) |
| INA219 SDA | GPIO21 | I/O | I²C data |
| INA219 SCL | GPIO22 | I/O | I²C clock |
| Button | GPIO13 | IN | Internal pullup. Active LOW. |
| LED Red | GPIO16 | OUT | Mode indicator |
| LED Green | GPIO17 | OUT | Mode indicator |
| LED Blue | GPIO18 | OUT | Mode indicator |
| Battery ADC | GPIO4 (ADC2_CH0) | IN | Battery voltage sense (only read when BLE idle). |
| T1–T6 | GPIO36,39,34,35,32,33 | IN | Thermistor ADC (ADC1 only) |

---

## POWER BUDGET (ESTIMATED)

| Subsystem | Typical Draw | Peak | Source |
|-----------|-------------|------|--------|
| TEC1-12706 | 3–4A @ 9–12V | 6A | Battery direct via IBT-2 |
| ESP32 (BLE active) | 80–130mA @ 3.3V | 240mA (TX burst) | 5V buck → onboard 3.3V reg |
| IBT-2 logic | ~10mA @ 5V | ~20mA | 5V buck |
| INA219 | ~1mA @ 3.3V | ~1mA | 3.3V rail |
| Thermistors (6×) | ~2mA total | ~2mA | 3.3V rail |
| LED | ~20mA | ~60mA (white) | 3.3V via resistor |
| **Total (TEC active)** | **~35–50W** | **~72W** | **3S1P LiPo** |

**Runtime estimate:** 2200mAh × 11.1V ≈ 24.4Wh. At 40W avg TEC draw → ~36 min continuous. BackPal Clinical profile (4×4min heat + 1min cool ≈ 20min active) is within budget.

---

## NOTES

- DPS3005 and KORAD KA3005P are **lab instruments only** — not part of the device.
- VLT-100kJ is an **industrial reference design** — BackPal uses v1B vault exclusively.
- Battery voltage divider uses ADC2 (GPIO4) — read during BLE connection interval gaps or use INA219 bus voltage as proxy.
- Constant optimized DC is the validated TEC drive method. PWM duty-cycle modulation is NOT used for thermal control.
