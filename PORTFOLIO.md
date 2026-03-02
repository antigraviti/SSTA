# PORTFOLIO.md — SSTA Patent Portfolio
**Last verified:** March 2026
**Counsel:** Greenberg Traurig LLP
**Total families:** ~50+  |  **Total claims:** 2,000+

---

## FILING STATUS OVERVIEW

| Tier | Series | Count | Status |
|------|--------|-------|--------|
| Foundational | F-01 to F-15 | 15 | F-01–F-12 provisional filed; F-13 filed Dec 23 2025; F-14/F-15 draft ready |
| Control | E-01, E-02 | 2 | E-01 past original deadline — needs immediate action; E-02 drafted |
| Medical | MED-A to MED-E | 5 | Disclosures drafted, high value |
| Application | A-01 to A-10+ | 10+ | A-01 (BackPal) priority — no complete spec yet, only analysis docs |
| Manufacturing | M-01 to M-10 | 10 | Concepts defined, Phase 4 |
| Cryogenic | FSC | 1 | Hardened, draft ready |
| Sleep/Bed | SLP-01 to SLP-06 | 6 | New additions Dec 2025 |

---

## TIER 1 — FOUNDATIONAL (F-Series)

### F-01: Core SSTA Platform Architecture
- Scope: Fundamental thermal primitive network architecture; "treats heat as electricity"
- Status: Provisional filed
- Priority: ★★★★★ | Survivability: 85%
- Action: Convert to non-provisional

### F-02: SSTC — Solid-State Thermal Capacitor
- Scope: PCM-based latent heat storage device with conductive scaffold; layered/graded PCM configurations for storage capacity (NOT opposing k(T))
- Status: Provisional filed
- Priority: ★★★★★ | Survivability: 75–90%
- Key distinction: Multi-PCM = latent heat storage. NEVER "opposing temperature coefficients." See NREL firewall below.
- Action: Convert to non-provisional; vault renormalization data now available to strengthen

### F-03: Thermal Gates / Conditional Routing
- Scope: Hysteresis-based, phase-switch, and contact-pressure gate mechanisms
- Status: Provisional filed
- Priority: ★★★★★ | Survivability: 85–95%

### F-04: Boundary Interfaces / Thermal Governance
- Status: Provisional filed | Survivability: 90–95%

### F-05: Thermal Logic (Latching, Bistable States)
- Scope: AND/OR thermal logic, bistable latching, network oscillation
- Status: Provisional filed | Survivability: 80–90%

### F-06: Thermal Firewalls / Safety
- Status: Provisional filed | Survivability: 90–95%

### F-07: Serviceability / Hot-Swap
- Status: Provisional filed | Survivability: 95%

### F-08: Unified SSTC / High-Flux Scaffolds
- Status: Provisional filed | Survivability: 85–90%

### F-09: Liquid Metal Interfaces
- Status: Provisional filed | Survivability: 80–90%

### F-10: Carnot Battery Integration
- Status: Provisional filed | Survivability: 70–85%

### F-11: Sand Battery / Bulk Storage
- Status: Provisional filed | Survivability: 70–80%

### F-12: Grid Integration
- Status: Provisional filed | Survivability: 80–90%

### F-13: Thermal Diodes / Rectifiers ★ FLAGSHIP ★
- Scope: Asymmetric chamber geometry + porous scaffold + PCM, NO vapor space
- Filed: Dec 23, 2025 (provisional)
- Non-provisional deadline: Dec 23, 2026
- Priority: ★★★★★ | Survivability: 95%
- Validated R: 3.6 (experimental, ONYX Bethlehem Labs)
- Performance ranges: R = 2.5–4.0 basic; R = 7–12 combined enhancements; R^N cascade
- Addendum: Self-sensing claims (R is temperature-dependent → infer T without separate sensors) — add to non-provisional
- Housing: "interfaces comprise thermally conductive material" (allows polymer housing with metal inserts)
- Key language: "phase-front evolution," "asymmetric chamber," "porous scaffold" — always use these
- Prior art cleared: Duke US8716689 (vapor/droplet, no scaffold); NREL (PTC/NTC opposition, different mechanism); nanoscale diodes (wrong size regime, R only 1.02–1.07)

### F-14: Radiative Gates / Variable Emissivity
- Status: Draft ready | Priority: ★★★★

### F-15: Mechanical-Thermal Coupling
- Status: Draft ready | Priority: ★★★★★ | Survivability: 95%
- Experimental validation roadmap defined (pressure transducer, spring force, 10-cycle test)
- Phase 1 experiments: Feb–Apr 2026 at ONYX Bethlehem Labs

### F-16: Thermal Sensing Mesh
- Scope: Multi-point thermistor array with spatial interpolation
- Status: Disclosure drafted

### F-18: Capacitive PCM Phase-State Detection
- Scope: Capacitance change detects solid/liquid PCM state without thermal sensors
- Addendum v2.0 filed Dec 27, 2025 — adds claims 13A–13H, 60–63
- Next action: Procure PEG 1000, execute experiment F18-CAP-001

### F-19: Thermal Flux Density / Leverage (NEW — not yet filed)
- Scope: Thermal leverage ratio λ = A_output/A_source as design primitive; thermal spreader, concentrator, aperture, flux limiter primitives
- Origin: BackPal silicone interface problem — 0.9mm silicone creates 22°C penalty
- Key result (v2.0 corrected): 90×90mm copper spreader reduces penalty from 22°C to 4.4°C
- Status: Technical memo exists (v2.0 corrected after red-team). HYPOTHESIS — not yet experimentally validated. Do not present as established fact.
- Action: File provisional. Validate λ scaling experimentally before non-provisional.

---

## TIER 2 — CONTROL (E-Series)

### E-01: Electrical Control Architecture ⚠️ CRITICAL GAP
- Scope: Thermal-state-aware power control; PCM melt fraction throttling; multi-domain state estimation; energy recovery; fault detection; grid integration
- Original deadline: Jan 15, 2026 — **PAST DUE, needs counsel discussion**
- Priority: ★★★★★ | Survivability: 90%
- **CRITICAL:** Current BackPal controller COMPUTES thermal state variables (melt fraction, vault SOC) but does NOT use them for control decisions. This makes it functionally identical to prior art US 11,419,753 (PID-controlled TEC biomedical). The patent only holds once thermal-state-aware control is actually implemented on hardware. This is an active engineering TODO.
- Required implementation: PCM melt fraction throttling, scaffold saturation detection, runtime prediction with user alerts, multi-mode gain scheduling based on thermal state

### E-02: Multi-Zone Coordination
- Scope: Zone priority arbitration, load balancing, cascade control
- Status: Drafted | Survivability: 90%

---

## TIER 3 — MEDICAL (MED-Series)

| ID | Name | Priority | Notes |
|----|------|----------|-------|
| MED-A | Implantable Power Harvesting | ★★★★★ | High value |
| MED-B | Neural Thermal Biasing | ★★★★★ | High value |
| MED-C | Implant Microclimate Control | ★★★★ | |
| MED-D | Vascular Thermal Coupling | ★★★★★ | File first in MED series |
| MED-E | Inflammation Flare Suppressor | ★★★★ | |

All MED filings: use omnibus provisional with A+E families first, then divisionals/CIPs for B/C/D.

---

## TIER 4 — APPLICATION (A-Series)

### A-01: BackPal Wearable Contrast Therapy ★ HIGH COMMERCIAL VALUE
- Scope: Bidirectional contrast therapy (hot/cold), sealed vault, thermally conductive scaffold, silent operation, PCM pre-conditioning via TEC, lumbar/body-conformable
- Status: Analysis docs exist. **No complete patent specification drafted yet.** Do not represent as "ready to file."
- Survivability: 80–90%
- Prior art threats: Li et al. 2014 (wearable TEC+PCM, moderate risk — distinguished by bidirectional contrast + sealed vault + scaffold); various wearable cooler patents (distinguished by contrast therapy protocol, flex-zoned plate, Venturi duct, modular cartridge)
- Key differentiators: Bidirectional (heat AND cool), therapeutic protocol with defined temperature swings, no fans/pumps, PCM thermal storage, body-conformable flex plate

### A-02 to A-10: Additional Applications
- Sleep thermoregulation, medical implant thermal, data center, EV battery, aerospace — all concept stage

---

## TIER 5 — MANUFACTURING (M-Series)
M-01 through M-10 cover automated flex-zoned plate forming, robotic TEC assembly, PCM filling/degassing, validation stations, scaffold fabrication, leak testing, service cartridge assembly. All concept-defined, Phase 4 filing.

---

## CRYOGENIC — FSC
- Scope: Superconducting thermal gates using T_c quench switching; multi-node cryogenic networks (<100K); ADR integration
- Status: Hardened, draft ready | Survivability: 85%

---

## PRIOR ART — CLEARED THREATS

| Reference | What It Claims | Our Distinction | Status |
|-----------|---------------|-----------------|--------|
| NREL US 12,001,228 | PTC/NTC opposing k(T) for rectification | Our multi-PCM = latent heat storage, NOT opposing conductivity. Different purpose, different mechanism. | ✅ Cleared |
| Duke US 8,716,689 | Jumping-droplet vapor chamber, wickless | Our F-13: phase-front evolution in conductive scaffold, no vapor space | ✅ Cleared |
| Toyota US2017/0125866 | Vapor chamber thermal diodes | Our F-13: no vapor space; limitation explicit in claims | ✅ Cleared |
| Nanoscale diodes (Berkeley/UCLA) | Ballistic phonon, sub-100nm, R=1.02–1.07 | Our F-13: macro-scale (≥1cm), continuum regime, R≥2.0 | ✅ Cleared |
| Li et al. 2014 | Wearable TEC+PCM cooling, medical | A-01 distinguished by bidirectional contrast + sealed vault + scaffold | ⚠️ Moderate risk, monitor |
| US 11,419,753 | PID-controlled TEC biomedical | E-01 HIGH RISK until thermal-state-aware control is implemented on hardware | 🔴 Active risk |
| NREL contact switch | Wax-motor piston, binary ON/OFF, R≈17.5 | F-13 is continuous directional diode, not binary switch. Claim 57 explicit. | ✅ Cleared |

---

## CRITICAL LANGUAGE RULES

**ALWAYS use:**
- "latent heat storage through solid-liquid phase transitions" (multi-PCM)
- "phase-front evolution" (F-13 rectification mechanism)
- "asymmetric chamber geometry" (F-13 structure)
- "porous scaffold" (F-13 required element)
- "complementary melting temperatures" / "staged thermal absorption" (multi-PCM)
- "without requiring an externally commanded ON/OFF conductance state transition" (diode vs. switch)

**NEVER use:**
- "positive temperature coefficient" / "negative temperature coefficient"
- "opposing conductivity trends" / "junction conductivity ratio"
- "thermal switch" when describing diodes

---

## CLAIM ARCHITECTURE RULES

1. **Structural claims in independents, performance thresholds in dependents.** If examiner finds prior art matching R values, only dependents fall — structural independent survives.
2. **Network claims explicitly reference "device according to claim 1"** — prevents examiner treating networks as abstract concepts.
3. **Multi-PCM claims always framed as latent heat storage** — never opposing k(T).
4. **F-13 housing:** "interfaces comprise thermally conductive material" — allows polymer housings with metal interface inserts.

---

## WHAT GOES WHERE — QUICK REFERENCE

| Claiming... | File in... |
|-------------|------------|
| SSTC as integrated device | F-02 |
| Gate physical switching mechanism | F-03 |
| Bistable latch / thermal logic AND/OR | F-05 |
| Thermal diode (directional asymmetry) | F-13 |
| Cascade rectification | F-13 |
| Network oscillation | F-05 or F-01 |
| Battery thermal protection | F-06 |
| Control algorithms / thermal-state-aware | E-01 |
| BackPal wearable device | A-01 |
| Flux density / thermal leverage ratio | F-19 |
| Cryogenic switching | FSC |
| Capacitive phase detection | F-18 |
| Spatial thermal sensing array | F-16 |
