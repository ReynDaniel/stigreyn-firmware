# StigReyn AUV — Embedded Firmware
### STM32F446RE Real-Time Control System

> Low-cost, modular, open-source Autonomous Underwater Vehicle  
> Embedded firmware for the STM32 Nucleo-F446RE  
> Part of the StigReyn capstone project — Daniel Reynolds, REYN Consultancy

---

## Project Status

| Stage | Task | Status |
|-------|------|--------|
| 1 | GPIO — blink LED, toolchain verified | ✅ Complete |
| 2 | TIM2 PWM — 50Hz ESC signal on PA5 + PA1 | ✅ Complete |
| 3 | UART — debug output, switch to PLL 84MHz | 🔲 Planned |
| 4 | Safety — watchdog, leak detector, failsafe | 🔲 Planned |
| 5 | Sensors — IMU, Bar30 over I2C | 🔲 Planned |
| 6 | Control modes — safe, test, RC, auto | 🔲 Planned |
| 7 | Pi integration — UART protocol, sleep/wake | 🔲 Planned |
| 8 | ESP32-CAM — anomaly trigger, burst imaging | 🔲 Planned |
| 9 | Integration — full system bucket test | 🔲 Planned |

---

## Hardware Verified

```
Stage 2 — PWM signal confirmed with FNiRSi 2C23T oscilloscope:
PA5 (D13) — TIM2 CH1 — port thruster
  Frequency:   50Hz     ✓
  Pulse width: 1500µs   ✓ neutral
  Duty cycle:  7.5%     ✓
  Voltage:     3.13V    ✓ 3.3V logic

PA1 (A1)  — TIM2 CH2 — starboard thruster
  Frequency:   50Hz     ✓
  Pulse width: 1500µs   ✓ neutral
  Duty cycle:  7.5%     ✓
  Voltage:     3.13V    ✓ 3.3V logic
```

---

## Project Overview

StigReyn is a bio-inspired modular AUV platform intended for scalable low-cost ocean exploration.
The embedded firmware runs on an STM32F446RE and is responsible for all
real-time low-level control and safety-critical supervision, sensor management, and system orchestration.

A Raspberry Pi 4 companion computer handles high-level autonomy, AI-based
detection, and mission logging — and remains in low-power sleep for the majority of each mission
to conserve power.

---

## Hardware Architecture

```
┌─────────────────────────────────────────────────────┐
│                   STIGREYN AUV                      │
│                                                     │
│  ┌──────────────┐         ┌──────────────────────┐  │
│  │  STM32F446RE │◄─UART──►│   Raspberry Pi 4     │  │
│  │  (always on) │         │   (sleeps most dive) │  │
│  └──────┬───────┘         └──────────┬───────────┘  │
│         │                            │              │
│    ┌────┴────┐                  ┌────┴────┐         │
│    │ 2x T200 │                  │ Pi Cam  │         │
│    │ via ESC │                  │ Module3 │         │
│    └─────────┘                  └─────────┘         │
│                                                     │
│  ┌──────────────┐                                   │
│  │  ESP32-CAM   │──interrupt──► STM32               │
│  │  (proximity) │                                   │
│  └──────────────┘                                   │
│                                                     │
│  Sensors: IMU, Bar30 depth, leak detector, ADC      │
│  Power:   4S lithium 20Ah, external BMS/charger     │
└─────────────────────────────────────────────────────┘
```

---

## STM32 Responsibilities

| Task | Method | Priority |
|------|--------|----------|
| Thruster PWM control | TIM2 hardware PWM | Critical |
| Leak detection | GPIO interrupt | Critical |
| Watchdog / failsafe | IWDG hardware watchdog | Critical |
| Battery monitoring | ADC1 CH0 | High |
| IMU reading | I2C1 | High |
| Depth / pressure | I2C1 (shared Bar30) | High |
| UART to Pi | UART2 | High |
| Pi sleep/wake control | GPIO PB4 | Medium |
| ESP32 anomaly trigger | GPIO interrupt PC1 | Medium |
| Reed switch wake | GPIO interrupt PC0 | Medium |
| Status LEDs | GPIO PB0, PB1 | Low |

---

## Passive Fail-Safe Design

StigReyn is intentionally buoyancy-biased nose-up.
On any failure — power loss, leak, watchdog timeout, low battery —
thrusters return to neutral (1500µs) and the vehicle passively ascends.

```
Failure event detected
        ↓
PWM → 1500µs neutral (both thrusters)
        ↓
Passive buoyant ascent
        ↓
Surface recovery
```

No active control required for basic recovery.

---

## T200 Thruster PWM Regions

| Region | PWM (µs) | Use |
|--------|----------|-----|
| Neutral / failsafe | 1500 | Stop, passive ascent |
| Deadband | 1460–1540 | Avoid — no intentional thrust |
| Cruise forward | 1560–1620 | Normal autonomous operation |
| Dive / strong | 1620–1680 | Intentional dive |
| High power | 1680–1800 | Short duration only |
| Maximum forward | 1800–1900 | Emergency only |
| Gentle reverse | 1440–1460 | Docking / bench test |
| Moderate reverse | 1380–1440 | Manoeuvre |
| Strong reverse | 1100–1380 | Emergency braking |

Battery: 4S lithium 20Ah (12.0–16.8V)
ESC signal: 50Hz, 1100–1900µs pulse width
Counter-rotating props: starboard PWM inverted in firmware

---

## Pin Assignment — Confirmed

### Port A
| Pin | Board Label | Assignment | Type | Verified |
|-----|------------|-----------|------|----------|
| PA0 | A0 | Battery voltage | ADC1 CH0 | 🔲 |
| PA1 | A1 | Starboard thruster PWM | TIM2 CH2 AF1 | ✅ |
| PA2 | D1 | UART2 TX → Pi | AF7 | 🔲 |
| PA3 | D0 | UART2 RX ← Pi | AF7 | 🔲 |
| PA4 | A2 | Available | — | — |
| PA5 | D13 | Port thruster PWM | TIM2 CH1 AF1 | ✅ |
| PA6–PA12 | — | Available | — | — |
| PA13–PA15 | — | Reserved ST-LINK | — | — |

### Port B
| Pin | Board Label | Assignment | Type | Verified |
|-----|------------|-----------|------|----------|
| PB0 | A3 | Status LED | Digital out | 🔲 |
| PB1 | — | ESC armed LED | Digital out | 🔲 |
| PB3 | — | Reserved ST-LINK SWO | — | — |
| PB4 | D5 | Pi sleep/wake | Digital out | 🔲 |
| PB8 | D15 | I2C1 SCL (IMU + Bar30) | AF4 | 🔲 |
| PB9 | D14 | I2C1 SDA (IMU + Bar30) | AF4 | 🔲 |
| PB10 | D6 | UART3 TX spare | AF7 | 🔲 |
| PB11 | — | UART3 RX spare | AF7 | 🔲 |

### Port C
| Pin | Board Label | Assignment | Type | Verified |
|-----|------------|-----------|------|----------|
| PC0 | — | Reed switch input | Digital in | 🔲 |
| PC1 | A4 | ESP32-CAM trigger | Digital in | 🔲 |
| PC13 | — | Leak detector / USER button | Digital in | 🔲 |
| PC14–PC15 | — | Reserved oscillator | — | — |

**27 pins available for future expansion**

---

## Important Hardware Notes

```
TIM2 channel mapping — confirmed by hardware test:
TIM2_CH1 = PA0, PA5, PA15  — using PA5 (D13)
TIM2_CH2 = PA1, PB3        — using PA1 (A1)
PA6 does not support TIM2_CH2 on STM32F446RE; verified against STM32 alternate-function tables and oscilloscope hardware testing.

Timer doubling rule:
APB1 prescaler = /1  → timer clock = APB clock (no doubling)
APB1 prescaler > /1  → timer clock = 2 x APB clock

Current HSI clock (Stage 1-2):
PSC = 15, ARR = 19999, TIM2 = 16MHz

PLL clock (Stage 3+):
PSC = 83, ARR = 19999, TIM2 = 84MHz
```

---

## Engineering Notes

```
Hardware verification was used to confirm actual STM32 timer pin mappings.

PA6 was initially assumed to support TIM2_CH2; STM32 alternate-function
verification and oscilloscope testing confirmed this was incorrect.

Final verified PWM mapping:
PA5 = TIM2_CH1
PA1 = TIM2_CH2

Hardware-first validation was prioritised before ESC integration.
```

---

## Wake / Sleep Sequence

```
SURFACE (pre-launch):
Reed switch (PC0) → STM32 wakes → Pi wakes (PB4 HIGH)
Pi runs pre-launch checks via UART2
Pi sends GO/NO-GO → Pi sleeps → mission begins

DURING DIVE:
ESP32-CAM detects anomaly → PC1 interrupt
STM32 wakes Pi (PB4 HIGH)
Pi cam captures + AI detection
Result sent to STM32 via UART2
Pi sleeps (PB4 LOW)

LOW BATTERY:
STM32 ADC reads voltage drop
STM32 warns Pi → Pi saves data
STM32 sets PWM neutral → passive ascent

LEAK DETECTED:
PC13 interrupt fires immediately
PWM → 1500µs both thrusters
Passive ascent
```

---

## Firmware Module Structure

```
stigreyn_firmware/
├── Core/
│   ├── App/                — application logic
│   ├── Config/
│   │   └── pin_config.h    — all pin assignments
│   ├── Control/
│   │   ├── esc_pwm.c       — T200 thruster PWM driver
│   │   └── esc_pwm.h
│   ├── Sensors/            — IMU, Bar30, battery ADC
│   ├── Comms/              — UART to Pi protocol
│   └── Safety/             — watchdog, leak, failsafe
├── Drivers/                — ST HAL/CMSIS (do not modify)
├── Inc/                    — CubeMX generated headers
├── Src/
│   └── main.c              — system init, main loop
└── README.md
```

---

## Development Environment

- IDE: STM32CubeIDE 2.1.1 (Mac)
- Config: STM32CubeMX (separate from IDE in 2.x)
- Board: STM32 Nucleo-F446RE
- MCU: STM32F446RET6 — ARM Cortex-M4, 180MHz capable
- Current clock: HSI 16MHz (PLL upgrade at Stage 3)
- Approach: CubeMX infrastructure + HAL peripheral initialisation + selective bare-metal register access
- Language: Embedded C (C99)

---

## Verification Equipment

```
FNiRSi 2C23T — oscilloscope / logic analyser
Used to verify PWM signal on PA5 and PA1
Confirmed: 50Hz, 1500µs neutral, 3.13V logic level
```

---

## Related Repositories

- `stigreyn-mechanical` — SolidWorks CAD, FEA, hydrodynamic analysis
- `stigreyn-firmware` — this repository
- `stigreyn-pi` — Raspberry Pi companion software (planned)

---

*StigReyn is an open-source platform intended to lower the barrier
to deep-ocean exploration for students, researchers, and small teams.*
