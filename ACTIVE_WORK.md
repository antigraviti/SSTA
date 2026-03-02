# ACTIVE_WORK.md — Open TODOs, Current Experiments & Known Issues
**Last verified:** March 2026
**Update this file whenever a task completes or a new issue is discovered.**

---

## 🔴 BLOCKING — Must resolve before next dependent work

### [HW-01] IBT-2 Enable Pin Fault
- Enable pins D7/D8 outputting 1.7–1.9V instead of 5V
- Effect: HEAT mode produces cooling behavior; cooling works correctly
- Last attempted fix: Pin swap (fw v2.6), direct 5V bypass test → T1 sensor fault appeared
- **Action:** Physical hardware inspection before next test run. Verify wiring integrity on T1 after bypass modification. Determine if IBT-2 unit needs replacement.

### [FW-01] Firmware Heating Mode Validation
- Blocked by HW-01 above
- Heating mode has not been validated end-to-end since degradation appeared
- Cannot run BackPal Clinical profile (requires both HEAT and COOL) until resolved

### [IP-01] E-01 Filing — Past Deadline
- Original deadline: Jan 15, 2026 — **PAST DUE**
- **Action:** Contact Greenberg Traurig LLP immediately to discuss options
- **Additional blocker:** Thermal-state-aware control not yet implemented on hardware — the core patent differentiator has no working embodiment until this is built and validated (see FW-03)

---

## 🟡 HIGH PRIORITY — Active engineering work

### [FW-02] Constant DC Control Integration
- PWM superseded by constant optimized DC as of Feb 28, 2026 (4.5× better stability)
- Current firmware still uses open-loop PWM architecture in places
- **Action:** Fully migrate BackPal controller to constant optimized DC drive via KORAD KA3005P protocol or onboard regulation
- **Supports:** E-01 patent claims; BackPal product

### [FW-03] PI Closed-Loop TEC Control (E-01 scope)
- **Critical for E-01 patent:** Must implement thermal-state-aware control using T1 feedback
- Required features to distinguish from prior art US 11,419,753:
  1. PCM melt fraction throttling — reduce TEC power as vault approaches saturation
  2. Scaffold saturation detection — thermal protection
  3. Runtime prediction with user alerts — based on vault SOC
  4. Multi-mode gain scheduling — PID gains change based on thermal state (solid/transition/liquid)
- Current state: Melt fraction and vault SOC are computed but NOT fed back into control decisions
- **Action:** Implement all four features; validate on hardware; document with lab report

### [EXP-01] Vault Renormalization Test Series (H1–H6)
- Root problem: After deep cooling (T2 → 1.6°C), 30K gradient (T2 vs T4 = 32.8°C) prevents HEAT mode from resetting vault
- **Three mitigation strategies under test:**
  - Strategy A: Stop cooling at T2 ≈ 10°C to reduce gradient before reversal
  - Strategy B: Higher reset voltage (4V vs 2.5V)
  - Strategy C: Passive equalization pause (2–3 min) before HEAT mode
- **Test series:**
  - H1: Passive equalization baseline — measure gradient decay with no TEC power
  - H2: Minimum temperature threshold sweep — find T2 floor that allows effective reversal
  - H3: Voltage sweep — characterize reset effectiveness at 2.5V, 3.5V, 4V, 5V
  - H4: Combined strategy — best threshold + best voltage + equalization pause
  - H5: Multi-cycle stress test — 3+ contrast cycles with winning strategy
  - H6: Asymmetric phase timing — shorter cooling phase to prevent deep soak
- Include v1F and v1G vault variants in H5/H6
- **Supports:** BackPal clinical validation; E-01 continuation claims; multi-cycle contrast therapy design

### [EXP-02] Oscilloscope TEC Current Waveform Capture
- Purpose: Anchor PWM ripple analysis with real measurements (not estimates)
- Equipment: Rigol DS1054Z via USB-B, PyVISA script (already written)
- Metrics: I_rms, I_avg, joule ratio, duty cycle, switching frequency
- Output: SSTA canonical CSV format
- **Action:** Execute as one-time calibration; append data to Feb 28 lab report

### [IP-02] F-13 Non-Provisional Preparation
- Provisional filed Dec 23, 2025 — non-provisional deadline: **Dec 23, 2026**
- Add self-sensing claims addendum (R is T-dependent → infer temperature without separate sensors)
- Isolate geometry vs. PCM contributions to rectification ratio through control testing
- R(T) calibration curve needed: measure R at temperatures spanning below and above PCM Tm
- **Action:** Execute R(T) test protocol; generate curve; incorporate into non-provisional spec

---

## 🟢 ACTIVE — In progress or next up

### [EXP-03] F-15 Phase 1 Validation (Feb–Apr 2026)
- Experiment 1: Vault pressure during melt — install pressure transducer on v1B, log pressure vs. time (need to purchase transducer)
- Experiment 2: Spring force vs. coupling conductance characterization
- Experiment 3: 10 vault cycles — mechanical durability baseline
- Experiment 4: TIM datasheet confirmation
- **Supports:** F-15 claims (P_max/P_min ≤ 2.0, pressure < 50 kPa, G variation < 20%, 10,000 cycles)
- **Procurement needed:** Pressure transducer compatible with LabJack U3-LV

### [EXP-04] F-18 Capacitive Phase-State Detection
- Build single-cell test article with PEG 1000 (Tm ≈ 37°C) between parallel-plate electrodes
- Measure capacitance vs. temperature through melt/solidify cycle
- Validate: capacitance drop correlates with solid→liquid transition
- Next action: Procure PEG 1000; execute experiment F18-CAP-001
- Follow-on: Multi-cell array (PEG 600/1000/4000), interdigitated electrodes, CaCl₂·6H₂O comparison

### [DOC-01] BackPal BOM & Architecture — Final Correction
- Corrected device mass: ~762g (v1B vault, not VLT-100kJ industrial)
- Corrected enclosure: 110×175×55mm external
- Corrected battery: 3S1P
- **Action:** Ensure all downstream documents (A-01 patent spec, manufacturing BOM, build guide) reflect corrected v1B architecture — not the old 3,100g industrial version

### [IP-03] A-01 BackPal Patent — Spec Drafting
- Analysis documents and claim structure exist, but NO complete provisional specification drafted
- Do not represent as "ready to file" until spec is complete
- **Action:** Draft complete provisional spec incorporating: bidirectional contrast therapy, sealed PCM vault, thermally conductive scaffold, thermal leverage ratio (λ from F-19), silent operation, body-conformable flex plate, therapeutic protocol with defined temperature swings

### [IP-04] F-19 Thermal Flux Density — File Provisional
- Technical memo v2.0 exists (corrected after red-team found calculation errors in v1.0)
- λ = A_output/A_source validated analytically; experimental validation pending
- **Action:** Draft and file provisional; do not present λ^0.5 rectification scaling as established fact until experimentally confirmed

---

## 🔵 RESEARCH / EXPLORATORY — Not yet scheduled

### [RES-01] Advanced PCM Candidates
- Sodium acetate trihydrate (SAT): ~2× volumetric energy density vs. paraffin; Tm ≈ 58°C
- Solar salt (NaNO3/KNO3), HITEC: high-temp PCMs — SAFE, not covered by NREL
- Molten salts: high-temp applications
- PEG series: PEG 600 (Tm≈20°C), PEG 1000 (Tm≈37°C), PEG 4000 (Tm≈55°C)
- CaCl₂·6H₂O (Tm≈29°C): higher capacitance contrast for F-18

### [RES-02] SSTA Network Simulator
- JSON/YAML graph-based simulator for thermal primitive networks
- PCM-capable with nonlinear edges and hysteresis events
- Parameter fitting from test logs
- Architecture defined in SSTA Instructions (see project file)
- **Ideal Claude Code project** — well-defined data model, iterative development

### [RES-03] Solid-State Battery Thermal Management
- SSTA F-13 diodes (R=3.6) and SSTC capacitors applicable to SSB thermal challenges
- Key SSB problems: interfacial thermal expansion, temperature uniformity, fast-charge spikes
- **Gate:** Customer discovery first — verify willingness to license before investing in patents
- Recommendation: 3-phase gated approach, not immediate patent filing

### [RES-04] Oscilloscope Data Integration Pipeline
- PyVISA script written and ready
- Goal: Auto-calculate TEC waveform metrics and output in SSTA canonical CSV format
- Completes the Feb 28 lab report to B+ or better (currently B− due to missing waveform data)

---

## FIRMWARE VERSION HISTORY (BackPal Controller)

| Version | Key Change |
|---------|-----------|
| v2.1 | Smart app-controlled power, T3 hot-side monitoring |
| v2.2–v2.5 | Bug fixes: timer calculation, bang-bang oscillation, 10Hz control loop |
| v2.6 | TEMP_THEN_HOLD therapeutic timer; pin swap attempt for HEAT mode |
| v2.7 | Continued debugging; Web Serial API CSV logging fix |
| **Current** | **v2.7 — HEAT mode unvalidated pending HW-01 resolution** |

---

## OPEN QUESTIONS — Need answers before proceeding

| # | Question | Blocks |
|---|----------|--------|
| Q1 | Is T1 definitively the primary control sensor, or still being decided between T1 and T2? | FW-03 PI controller implementation |
| Q2 | Has E-01 deadline miss been communicated to Greenberg Traurig? | IP-01 |
| Q3 | What are the measured dimensions for v1G vault? | H5/H6 test series |
| Q4 | Has the IBT-2 unit been physically inspected after the bypass modification? | HW-01, FW-01 |
| Q5 | Is the oscilloscope PyVISA script confirmed working with the Rigol DS1054Z on current machine? | EXP-02 |
