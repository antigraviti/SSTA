# DECISIONS.md — Key Technical Decisions & Rationale
**Last verified:** March 2026
**Purpose:** Prevent re-litigating resolved decisions. Before proposing a change to anything listed here, read the rationale. If you believe a decision should be revisited, flag it explicitly — do not silently reverse it.

---

## HARDWARE DECISIONS

### [D-HW-01] MCU: Arduino Nano, not STM32 Blue Pill
- **Decision:** Current BackPal controller uses Arduino Nano + IBT-2 H-bridge
- **Rationale:** STM32 Blue Pill was evaluated but Arduino Nano was adopted for development velocity and toolchain simplicity. IBT-2 provides adequate bidirectional TEC drive at the current power budget.
- **Implication:** All firmware generation must target Arduino Nano. Do not generate STM32 code unless explicitly asked.

### [D-HW-02] TEC Drive: Constant Optimized DC, not PWM
- **Decision:** Constant DC delivered via KORAD KA3005P is the validated TEC drive method
- **Rationale:** Feb 28, 2026 experiment showed 4.5× better skin-side temperature stability (±0.09°C vs ±0.41°C). Two mechanisms: (1) PWM current ripple increases Joule heating without proportional heat pumping benefit; (2) TEC resistance drift causes fixed-duty PWM to deliver declining power as device warms.
- **Validated:** Yes — lab report Feb 28, 2026, SHA-256 verified
- **Implication:** Do not recommend PWM as primary TEC drive. PWM may still be appropriate for other actuators or legacy contexts — but not TEC thermal control.

### [D-HW-03] TEC Orientation: Cold side UP, hot side DOWN
- **Decision:** TEC cold side faces toward copper spreader (skin contact). Hot side faces down toward SSTC vault.
- **Rationale:** Correct orientation for cooling mode — cold side extracts heat from skin, hot side rejects into vault. Reversed orientation would heat the skin in cooling mode.
- **Note:** This was caught and corrected during build documentation review. Any wiring diagram or assembly doc must reflect this orientation.

### [D-HW-04] BackPal Vault: v1B (51×51×31mm), not VLT-100kJ
- **Decision:** BackPal device uses SSTC Vault v1B (51×51×31mm, 36.15g paraffin, ~762g total device)
- **Rationale:** VLT-100kJ (120×120mm) is an industrial-scale reference design — 3,100g total, not wearable. This was a critical BOM error corrected in Feb 2026.
- **Implication:** Any document referencing BackPal device mass, dimensions, or vault spec must use v1B values. VLT-100kJ is a design study only.

### [D-HW-05] DPS3005 is Lab Instrument Only
- **Decision:** The DPS3005 programmable power supply is not part of the BackPal device
- **Rationale:** It is a bench instrument used for development and characterization. The device uses direct battery drive through PWM/DC control.
- **Implication:** Do not include DPS3005 in device BOMs, enclosure layouts, or product specifications.

---

## FIRMWARE DECISIONS

### [D-FW-01] Therapeutic Timer: TEMP_THEN_HOLD, not wall clock
- **Decision:** BackPal controller counts therapeutic time only when T1 is within tolerance of target temperature
- **Rationale:** Clinical validation requires measurement of actual therapeutic exposure time, not total session duration. A timer that counts from phase start regardless of whether target is reached is clinically meaningless and would fail FDA documentation requirements.
- **Implemented:** v2.6+
- **Implication:** Never revert to wall-clock timing for therapeutic phases.

### [D-FW-02] Control Loop at 10Hz, not 1Hz
- **Decision:** Thermal control loop runs at 10Hz
- **Rationale:** At 1Hz the thermal system responds in 2–3 seconds between corrections, causing overshoot. 10Hz allows responsive proportional control without instability.
- **Implemented:** v2.5+

### [D-FW-03] Use React Refs for Serial Data, not State Closures
- **Decision:** Web Serial API data handling uses React refs, not useState closures
- **Rationale:** useState closures capture stale values at render time, causing CSV logging to record incorrect profile data when phases transition. Refs provide stable references to current values across async callbacks.
- **Implemented:** v2.6+

### [D-FW-04] T1 is Primary Safety Limit Sensor
- **Decision:** T1 (skin-side spreader) is the primary sensor for safety cutoff (>45°C limit)
- **Rationale:** T1 is closest to the user's skin — it is the most clinically relevant temperature for safety enforcement. T3 is unreliable (multiple failures). T2 reflects TEC hot side, not skin exposure.
- **Open question:** Whether T1 or T2 is the primary feedback sensor for the PI control loop is not yet decided (see ACTIVE_WORK Q1).

---

## PATENT STRATEGY DECISIONS

### [D-IP-01] Structural Claims in Independents, Performance in Dependents
- **Decision:** F-13 (and all thermal primitive patents) — independent claims describe structure only. Performance thresholds (R ≥ 2.0 etc.) are in dependent claims.
- **Rationale:** If examiner finds prior art matching a performance value, only the dependent claim falls. The structural independent survives. This is the correct prosecution strategy for a new class of device.

### [D-IP-02] Multi-PCM = Latent Heat Storage, Never Opposing Conductivity
- **Decision:** All multi-PCM configurations in SSTA patents are described as "latent heat storage through solid-liquid phase transitions" with "complementary melting temperatures" for "staged thermal absorption"
- **Rationale:** NREL US 12,001,228 covers PTC/NTC materials with opposing temperature coefficients of conductivity. SSTA's multi-PCM uses layered configurations for storage capacity — fundamentally different purpose and mechanism. Using NREL's language would walk into their claim scope.
- **Hard rule:** Never use "positive temperature coefficient," "negative temperature coefficient," "opposing conductivity trends," or "junction conductivity ratio" in any SSTA patent document.

### [D-IP-03] F-13 Mechanism: Phase-Front Evolution, Not Vapor Transport
- **Decision:** F-13 thermal diode rectification is described as "phase-front evolution in asymmetric chamber geometry with conductive scaffold" — explicitly "no vapor space"
- **Rationale:** Duke US 8,716,689 (jumping-droplet) and Toyota US2017/0125866 (vapor chambers) both rely on vapor-phase transport. The "no vapor space" limitation and scaffold-enhanced solid conduction create distinct, cleared patent territory.
- **Hard rule:** Never describe F-13 operation using vapor, droplet, or wick terminology.

### [D-IP-04] F-13 Housing: Interface Material Flexibility
- **Decision:** F-13 Claim 1 uses "housing wherein at least the first and second thermal interface regions comprise thermally conductive material" — not "thermally conductive body"
- **Rationale:** Allows polymer housings (polycarbonate, PEEK) with metal interface inserts. The original "thermally conductive body" language would have excluded non-metal housings, limiting manufacturing flexibility and claim scope.
- **Version:** Changed in v3.5 → v3.6 of the provisional.

### [D-IP-05] Network Claims Bound to Device Claims
- **Decision:** F-13 network/system claims explicitly reference "device according to claim 1"
- **Rationale:** Prevents examiner from treating network configurations as abstract concepts. Forces prior art search to find the specific PCM + scaffold + asymmetric chamber combination in the claimed topology — a much higher bar.

### [D-IP-06] A-01 and E-01 Are Not Ready to File
- **Decision:** Neither A-01 (BackPal) nor E-01 (Electrical Control) should be represented as "ready to file" in their current state
- **Rationale:** A-01 has analysis documents and claim structure but no complete provisional specification. E-01 has a drafted spec but its core differentiator (thermal-state-aware control) has no working hardware embodiment — making it functionally identical to prior art US 11,419,753 until FW-03 is implemented.
- **Implication:** Do not tell counsel these are ready until spec (A-01) and hardware embodiment (E-01) are complete.

### [D-IP-07] Self-Sensing Claims Belong in F-13, Not a Separate Patent
- **Decision:** Self-sensing capability (inferring temperature from R(T) variation) is added to F-13 non-provisional, not filed as a separate F-17 patent
- **Rationale:** The physics is intrinsic to F-13's structure — R varies predictably with temperature due to PCM phase state. Separating it would weaken both patents. Filing together preserves the Dec 23, 2025 priority date for the sensing claims.

---

## DOCUMENTATION DECISIONS

### [D-DOC-01] Signing Authority for All Technical Documents
- **Decision:** All lab reports, memos, and technical documents: **ONYX Bethlehem Labs + SSTA IP Holdings**
- **Rationale:** IP protection requires consistent entity attribution. ThermalCare Medical is a commercialization entity — mixing it into R&D documentation creates ambiguity about IP ownership.
- **Hard rule:** Never sign as ThermalCare Medical on any technical or IP document. Never sign as either entity alone.

### [D-DOC-02] No Fabricated Data, Hashes, or Measurements
- **Decision:** If experimental data is missing, say so explicitly. Never generate placeholder SHA-256 hashes, measurement values, or test results.
- **Rationale:** Patent prosecution documents submitted to USPTO must reflect actual experimental results. Fabricated data in prosecution history is a serious legal liability.
- **Implication:** If a lab report requires data not yet collected, create a placeholder section labeled "PENDING: [experiment name]" — never fill it with invented values.

### [D-DOC-03] Validated Results vs. Hypotheses Must Be Distinguished
- **Decision:** Any claim that has not been experimentally confirmed must be labeled as hypothesis, design target, or analytical result — never presented as validated fact.
- **Key examples:**
  - Thermal leverage ratio λ scaling: analytically validated (v2.0 corrected memo), experimentally pending → label as analytical result, hypothesis for λ^0.5 rectification scaling
  - R(T) self-sensing: physically plausible, not yet measured → hypothesis
  - Vault renormalization mitigations A/B/C: under test → not yet validated

### [D-DOC-04] Red-Team Critiques Are Not Performative
- **Decision:** When asked to red-team a document, provide only genuine issues — not a long list of invented concerns to satisfy the format.
- **Rationale:** Established after Feb 2026 session where a 12-point critique was found to contain only 2–3 legitimate issues. Extensive performative criticism wastes time and erodes trust in the review process.
- **Standard:** If fewer than 3 real issues exist, say so. Fix the real ones. Do not invent problems.
