# HARDWARE.md — Lab Equipment, Prototypes & Validated Results
**Location:** ONYX Bethlehem Labs, Bethlehem PA
**Last verified:** March 2026

---

## LAB EQUIPMENT — CANONICAL LIST

| Device | Model | Role |
|--------|-------|------|
| Data acquisition | LabJack U3-LV | Primary DAQ, thermistor reading, event logging |
| Power supply | KORAD KA3005P | Programmable bench supply; TEC drive (constant DC) |
| Thermistors | MF55-103F-B3950 | All temperature sensing (NTC, B=3950K) |
| TEC modules | TEC1-12706 | 40×40mm Peltier, primary thermal actuator |
| Thermocouple meter | TASi TA612C | Secondary / spot-check temperature measurement |
| Oscilloscope | Rigol DS1054Z | Waveform capture; TEC current ripple analysis (USB-B) |
| Microcontroller | Arduino Nano | Current MCU for BackPal controller (replaces STM32) |
| H-bridge | IBT-2 (BTS7960) | Bidirectional TEC drive for heating and cooling modes |
| CNC / Laser | Genmitsu 4040 Pro Max | 20W laser; enclosure and fixture fabrication |
| 3D printer | On-site | Prototype enclosures and mechanical fixtures |

**NOTE:** STM32 Blue Pill was an earlier design. All current firmware targets Arduino Nano. Do not generate STM32 code unless explicitly asked.

---

## PROTOTYPE INVENTORY

### SSTC Vault v1B — PRIMARY TEST ARTICLE
| Parameter | Value |
|-----------|-------|
| Housing | Aluminum, 51×51×31mm |
| PCM | Food-grade paraffin (TooGet), 36.15g |
| PCM melt point | ~55°C nominal |
| Scaffold | Aluminum wool |
| TEC interface | 40×40mm TEC module |
| Latent capacity | ~6.5 kJ (36.15g × 180 J/g) |
| Sensible capacity | ~2.5 kJ (24→55°C range) |
| Total thermal budget | ~9 kJ |
| PCM saturation time | ~10 min at ~15W effective vault drive |

**Standard sensor placement (v1B):**

| Channel | Location | Calibration Offset | Purpose |
|---------|----------|--------------------|---------|
| T1 | Heat sink surface / skin-side spreader | −0.03°C | Skin temp proxy |
| T2 | TEC hot face / vault contact | +0.19°C | TEC control reference |
| T3 | TEC cold face / heatsink contact | +0.20°C | TEC ΔT (failed T2–T3 in some runs) |
| T4 | Vault center, embedded in PCM | −0.02°C | PCM core temp |
| T5 | Vault edge, ~0.5–1mm from wall | −0.60°C | PCM edge / gradient |
| T6 | Ambient, free air, 10cm from apparatus | 0.00°C (ref) | Baseline |

*Offsets from initial ambient equilibrium. No separate calibration procedure performed. T6 = reference.*

---

### SSTC Vault v1F — FINNED ALUMINUM VAULT
| Parameter | Value |
|-----------|-------|
| Base plate | 100×69×4.75mm aluminum |
| Fins | 16 fins, 36.35mm height |
| Mass | 234g (measured) |
| Void volume | 160mL |
| Housing | ABS enclosure |
| Status | Fabricated Feb 13, 2026 |

---

### SSTC Vault v1G — LARGE FINNED ALUMINUM VAULT
- Large aluminum finned vault, ABS housing
- Variant of v1F geometry — specific dimensions TBD in test protocol
- Include in vault renormalization test series (H1–H6)

---

### VLT-100kJ — INDUSTRIAL REFERENCE DESIGN (NOT WEARABLE)
| Parameter | Value |
|-----------|-------|
| Cavity | 90×90×45mm internal |
| Scaffold | Copper comb, 1.0mm sheet, 4–8mm pitch |
| Seal | O-ring or gasket |
| Fasteners | M5 bolts |
| Status | Reference/design study only — NOT in BackPal device |

⚠️ **This was incorrectly included in early BackPal BOM documentation. Corrected to v1B.**

---

### BackPal Device — CORRECTED ARCHITECTURE (as of Feb 2026)
| Parameter | Value |
|-----------|-------|
| Vault | SSTC v1B (51×51×31mm, 36.15g paraffin) |
| Total device mass | ~762g |
| Enclosure | 110×175×55mm external |
| MCU | Arduino Nano |
| H-bridge | IBT-2 |
| Battery | 3S1P (corrected from 3S2P) |
| Power architecture | Direct battery drive through PWM/DC control |
| Current sensing | INA219 |
| User input | Pushbutton |
| TEC orientation | Cold side UP (toward copper spreader/skin), hot side DOWN (toward vault) |

⚠️ **DPS3005 programmable power supply is a LAB INSTRUMENT ONLY — not part of the device.**

---

## VALIDATED EXPERIMENTAL RESULTS

### SSTC Thermal Buffering (Jan 26–27, 2026 — Lab Report 20260128_v4)
- **Temperature rise suppression during melt plateau: 7.0×** (5–8× range across trials)
- **Solidification suppression: 8.2×**
- **Plateau duration: 67 minutes**
- **Supports:** F-02 patent claims for isothermal thermal buffering
- **Uncertainty:** T5 offset (−0.60°C) cause unknown; T3 failed in Trials 2–3
- **Data integrity:** SHA-256 verified

### PWM vs. Constant Optimized DC (Feb 28, 2026)
- **Constant DC skin-side stability: ±0.09°C**
- **Fixed-duty PWM stability: ±0.41°C**
- **Improvement factor: 4.5× better with constant DC**
- **Mechanism:** PWM current ripple increases Joule heating without proportional heat pumping; TEC resistance drift causes fixed-duty PWM to deliver declining power as device warms
- **Status: VALIDATED. PWM is superseded for TEC control.**
- **Supporting data needed:** Oscilloscope TEC current waveform (Rigol DS1054Z via PyVISA) to anchor ripple analysis with measured data rather than estimates
- **Supports:** E-01 patent claims; BackPal controller architecture

### Thermal Rectification — F-13 Diode (Dec 2025)
- **Measured R: 3.6** (exceeds minimum threshold R > 1.0; exceeds target R > 3.6 boundary)
- **Mechanism:** Asymmetric chamber geometry + porous scaffold + PCM phase-front evolution
- **No vapor space** — explicit distinction from Duke/Toyota prior art
- **Interface area asymmetry:** 11.3:1 ratio (primary geometric contributor)
- **Note:** Relative contributions of geometry vs. PCM not yet isolated through control testing
- **Supports:** F-13 claims; rectification ratio R = G_easy / G_hard

---

## KNOWN HARDWARE ISSUES & ANOMALIES

### Vault Renormalization Problem (confirmed Mar 2026)
- **Symptom:** After deep cooling (T2 → 1.6°C), switching to HEAT mode fails to efficiently reset vault. TEC waste heat heats T2 locally but vault stays thermally loaded (T4 → 32.8°C).
- **Root cause:** 30K gradient between T2 and T4 overwhelms TEC pumping capacity during polarity reversal
- **Three mitigation strategies under test:**
  - A) Stop cooling at T2 ≈ 10°C to reduce gradient before reversal
  - B) Higher reset voltage (4V vs 2.5V)
  - C) Passive equalization pause (2–3 min) before HEAT mode
- **Test series:** H1–H6 (passive baseline, min temp threshold, voltage sweep, combined strategies, multi-cycle stress, asymmetric phase timing)
- **Critical design constraint for multi-cycle contrast therapy**
- **Supports:** BackPal controller design; potential E-01 continuation claims

### IBT-2 Enable Pin Degradation (Mar 2026)
- **Symptom:** Enable pins D7/D8 outputting 1.7–1.9V instead of 5V, preventing HEAT mode
- **Effect:** Heating mode produces cooling behavior; cooling works correctly
- **Resolution attempted:** Pin swap in firmware (v2.6), direct 5V bypass test
- **Status:** T1 sensor fault observed after bypass modification — may be wiring issue or component failure
- **Action needed:** Hardware inspection; verify pin integrity before next test run

### T3 Sensor Reliability
- T3 (TEC cold face) has failed in multiple test runs
- Do not rely on T3 as primary control reference — use T2 or T1

---

## CONTROLLER FIRMWARE STATE

| Item | Detail |
|------|--------|
| MCU | Arduino Nano |
| Current version | v2.6 / v2.7 (active development) |
| H-bridge | IBT-2 |
| Control mode | Open-loop PWM → **transitioning to constant optimized DC** |
| Web interface | React app, Web Serial API, npm local server |
| Profiles | BackPal Clinical (4×4min@40°C + 1min@9°C), characterization, diode test, power sweep |
| Data logging | 1Hz CSV, long-format, SHA-256 hash, SSTA canonical schema |
| Timer logic | TEMP_THEN_HOLD — counts therapeutic time only when T1 within tolerance of target |
| Control loop | 10Hz for responsive thermal management |

**Open TODO (E-01 scope):** Implement PI closed-loop TEC control using T1 feedback for stable skin temperature regulation. Currently transitioning from open-loop fixed PWM to constant optimized DC. True thermal-state-aware control (melt fraction throttling, vault SOC feedback) not yet implemented.

---

## THERMAL INTERFACE — SILICONE PENALTY

- **0.9mm silicone skin interface creates ~22°C temperature penalty** at current device geometry
- **Solution (F-19 / validated analytically):** 90×90mm copper spreader reduces penalty to ~4.4°C
- **Thermal leverage ratio:** λ = A_output / A_source = 90×90 / 40×40 ≈ 5.06
- **Status:** Analytically validated (v2.0 corrected memo). Experimental validation pending.
- **Only silicone and air gaps require significant spreading (λ ≥ 4).** Standard TIMs perform adequately at λ = 1.

---

## MANUFACTURING CAPABILITIES (ON-SITE)

- Genmitsu 4040 Pro Max CNC, 20W laser
- 3D printing
- Aluminum fabrication
- PCM sourcing and filling
- Electronic component integration
