# StigReyn AUV — Embedded Firmware
### STM32F446RE Real-Time Control System

> Low-cost, modular, open-source Autonomous Underwater Vehicle  
> Embedded firmware for the STM32 Nucleo-F446RE  
> Part of the StigReyn capstone project — Daniel Reynolds, REYN Consultancy

---

## Project Status

| Stage | Task                                                                  | Status |
|-------|-----------------------------------------------------------------------|--------|
| 1 | GPIO — blink LED, toolchain verified                                      | ✅ Complete |
| 2 | TIM2 PWM — 50Hz ESC signal on PA5 + PA1                                   | ✅ Complete |
| 3 | UART — debug output, PLL 84MHz, 115200 baud                               | ✅ Complete |
| 4a | Architecture — state machine, feature flags, safety framework            | ✅ Complete |
| 4b | Safety validation — PA8/D7 leak ISR, fault latch, multi-fault logging    | ✅ Complete |
| 5a | ESC arming — neutral hold, STATE_SAFE → STATE_ARMED transition           | ✅ Complete |
| 5b | Drive layer — differential thrust mixing, UART WASD control, PWM clamps  | ✅ Complete |
| 5c | Bench thruster validation — ESC + T200 test with external power          | 🔄 In Progress |
| 6 | Sensors — battery ADC, IMU, Bar30 over I2C                                | 🔲 Planned |
| 7 | Control modes — safe, test, RC, auto                                      | 🔲 Planned |
| 7 | Pi integration — UART protocol, sleep/wake                                | 🔲 Planned |
| 8 | ESP32-CAM — anomaly trigger, burst imaging                                | 🔲 Planned |
| 9 | Integration — full system bucket test                                     | 🔲 Planned |

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
| ESC arming sequence | Timed neutral PWM state machine | Critical |
| Differential thrust mixing / limits | drive.c control layer | Critical |
| Leak detection | EXTI8 interrupt (PA8 / D7) | Critical |
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
On any critical failure — leak detection, watchdog timeout, critical battery,
or future hull-pressure fault — thrusters return to neutral (1500µs)
and the vehicle passively ascends.

```
Failure event detected
        ↓
PWM → 1500µs neutral (both thrusters)
        ↓
Passive buoyant ascent
        ↓
Surface recovery
```
Recoverable and warning faults are intentionally separated from automatic
surface decisions. The firmware now distinguishes between:

- `STATE_FAULT`   → stop, assess, classify, remain safe
- `STATE_SURFACE` → committed passive recovery ascent

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

Stage 5 firmware now supports:
- ESC neutral-hold arming sequence
- UART W/A/S/D manual thrust control
- Differential thrust mixing
- Command-space PWM safety clamps
- Manual safe-stop and disarm control

Planned future additions:
- PWM ramp/rate limiting
- Closed-loop thrust validation
- Autonomous control integration
- Pi command protocol integration

---

## Pin Assignment — Confirmed

### Port A
| Pin | Board Label | Assignment | Type | Verified |
|-----|------------|-----------|------|----------|
| PA0 | A0 | Battery voltage | ADC1 CH0 | 🔲 |
| PA1 | A1 | Starboard thruster PWM | TIM2 CH2 AF1 | ✅ |
| PA2 | D1 | UART2 TX → Pi | AF7 | 🔲 |
| PA3 | D0 | UART2 RX ← Pi | AF7 | 🔲 |
| PA4 | A2 | Reserved manual throttle input | Future ADC/manual control | 🔲 |
| PA5 | D13 | Port thruster PWM | TIM2 CH1 AF1 | ✅ |
| PA6 | D12 | Reserved SPI MISO | Spare / future SPI | 🔲 |
| PA7 | D11 | Reserved SPI MOSI | Spare / future SPI | 🔲 |
| PA8 | D7 | Leak detector input | EXTI8 active LOW | ✅ |
| PA9–PA12 | — | Available | — | — |
| PA13–PA15 | — | Reserved ST-LINK | — | — |

### Port B
| Pin | Board Label | Assignment | Type | Verified |
|-----|------------|-----------|------|----------|
| PB0 | A3* | Status LED | Digital out | ✅ |
| PB1 | A4* | ESC armed LED | Digital out | 🔲 |
| PB3 | — | Reserved ST-LINK SWO | — | — |
| PB4 | D5 | Pi sleep/wake | Digital out | 🔲 |
| PB8 | D15 | I2C1 SCL (IMU + Bar30) | AF4 | 🔲 |
| PB9 | D14 | I2C1 SDA (IMU + Bar30) | AF4 | 🔲 |
| PB10 | D6 | UART3 TX spare | AF7 | 🔲 |
| PB11 | — | UART3 RX spare | AF7 | 🔲 |

### Port C
| Pin | Board Label | Assignment | Type | Verified |
|-----|------------|-----------|------|----------|
| PC0 | A5 | Reed switch input | Digital in | 🔲 |
| PC1 | A4 | ESP32-CAM trigger | Digital in/out | 🔲 |
| PC13 | USER BTN | Nucleo user button (unused by leak ISR) | Digital in | 🔲 |
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

Stage 5 UART propulsion validation:
- USER button triggers ESC neutral-hold arming sequence
- W/A/S/D commands provide differential thrust control
- SPACE returns both thrusters to 1500µs neutral
- X forces immediate disarm/failsafe transition
- PWM outputs validated live through UART telemetry logging

---

## Engineering Notes

```
Hardware-first validation was used throughout development to confirm
actual STM32 timer mappings, EXTI routing, UART timing, and fault behaviour.

PA6 was initially assumed to support TIM2_CH2; STM32 alternate-function
verification and oscilloscope testing confirmed this was incorrect.

Final verified PWM mapping:
PA5 = TIM2_CH1
PA1 = TIM2_CH2

Leak detection was later migrated from PC13 to PA8/D7 to better suit
Arduino proto-shield integration and modular connector layout.

The firmware architecture now separates:
- STATE_FAULT   → immediate safe stop and fault assessment
- STATE_SURFACE → committed passive recovery ascent

Multi-fault tracking is supported through a bitmask-based fault system
allowing multiple simultaneous faults while preserving a primary fault reason
for logging and immediate response.

Stage 5 introduced a modular drive layer supporting UART-based W/A/S/D
manual control from a host terminal. The firmware now supports:
- ESC neutral-hold arming
- Differential thrust mixing
- Command-space safety clamps
- Multi-state propulsion control
- Manual safe-stop and disarm commands

Initial propulsion validation was completed through live UART command
parsing and real-time PWM verification prior to ESC wet testing.

The next development milestone is physical thruster validation using
externally powered ESCs and Blue Robotics T200 thrusters before sealed
integration into the pressure hull.

Internal hull pressure monitoring is planned as a future relative-pressure
fault source using startup-sealed baseline comparison rather than only
absolute atmospheric reference.
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

PRE-DIVE ARMING:
STATE_SAFE → timed neutral PWM hold
ESCs complete startup beeps
STATE_ARMED entered
UART W/A/S/D manual bench-test mode permitted

LOW BATTERY:
STM32 ADC reads voltage drop
STM32 warns Pi → Pi saves data
STM32 sets PWM neutral → passive ascent

LEAK DETECTED:
PA8 / D7 EXTI8 interrupt fires immediately
PWM → 1500µs both thrusters
STATE_FAULT entered
Critical fault classified
Passive ascent / recovery policy
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
│   │   ├── esc_pwm.c       — low-level PWM output driver
│   │   ├── esc_pwm.h
│   │   ├── drive.c         — UART manual drive + differential thrust mix
│   │   └── drive.h
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
- Current clock: PLL 84MHz system clock
- Approach: CubeMX infrastructure + CMSIS/HAL startup + selective bare-metal register programming
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
