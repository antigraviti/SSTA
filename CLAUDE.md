# CLAUDE.md — SSTA Project Root
**Inventor:** William Janczewski (legal: William Janczewski)
**Entities:** SSTA IP Holdings LLC (IP) | ONYX Bethlehem Labs (R&D) | ThermalCare Medical LLC (commercialization, Co-Founder Lina Maini)
**Counsel:** Greenberg Traurig LLP
**Last verified:** March 2026

---

## CRITICAL RULES — READ EVERY SESSION

1. **All lab reports, memos, and technical documents sign as ONYX Bethlehem Labs + SSTA IP Holdings.** Never ThermalCare Medical. Never either entity alone. This is a firm IP protection rule.
2. **Legal name on all patent filings: William Janczewski**
3. **Never fabricate experimental data, hashes, or measurements.** If data is missing, say so explicitly and flag it as needed.
4. **Distinguish validated results from hypotheses.** If a claim hasn't been experimentally confirmed, label it as a hypothesis or design target.
5. **PWM is superseded.** As of Feb 28, 2026, constant optimized DC is the validated TEC drive method (4.5× better stability). Do not recommend PWM for TEC control.
6. **Current MCU is Arduino Nano + IBT-2 H-bridge.** STM32 Blue Pill was an earlier design. Do not generate firmware for STM32 unless explicitly asked.
7. **BackPal device mass is ~762g.** The VLT-100kJ industrial vault (3,100g) was a documentation error — corrected to v1B vault (51×51×31mm, 36.15g paraffin).

---

## CONTEXT FILES — load when relevant

| File | When to load |
|------|-------------|
| `.claude/PORTFOLIO.md` | Patent strategy, claim structure, prior art, filing status |
| `.claude/HARDWARE.md` | Lab equipment, prototype specs, validated experimental results |
| `.claude/ACTIVE_WORK.md` | Open TODOs, current experiments, known issues, firmware state |
| `.claude/DECISIONS.md` | Key technical decisions and rationale — check before re-litigating |

---

## PROJECT SCOPE

SSTA (Solid-State Thermal Architecture) treats heat as a controllable, routable resource via engineered thermal primitives analogous to electrical components:

- **SSTC** — Solid-State Thermal Capacitor (PCM-based latent heat storage)
- **Thermal Diode (F-13)** — asymmetric geometry + porous scaffold + PCM, no vapor space
- **Thermal Gate** — controllable thermal coupling (hysteresis, phase-switch)
- **Thermal State Machine** — network-level emergent behavior from primitive composition

**Addressable market:** $1.7T+ across EV/automotive, data centers, medical devices, defense/aerospace, consumer electronics.

**Active commercialization:** BackPal wearable contrast therapy device (ThermalCare Medical).

---

## STANDARD LAB EQUIPMENT

- LabJack U3-LV (DAQ)
- KORAD KA3005P (programmable power supply)
- MF55-103F-B3950 thermistors
- TEC1-12706 Peltier modules
- TASi TA612C thermocouple thermometer
- Rigol DS1054Z oscilloscope
- Arduino Nano (current MCU)
- IBT-2 H-bridge motor driver

---

## DATA INTEGRITY STANDARD

All experimental CSV output must conform to the Standard Logging CSV Schema (see `Standard_Logging_CSV_Schema__SSTA_`). SHA-256 hash verification is required on all files submitted for patent prosecution.

---

*For full portfolio details → `.claude/PORTFOLIO.md`*
*For hardware specs and validated results → `.claude/HARDWARE.md`*
*For active work and open issues → `.claude/ACTIVE_WORK.md`*
*For decision history → `.claude/DECISIONS.md`*
