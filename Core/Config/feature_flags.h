// ═══════════════════════════════════════════════════════════════
// feature_flags.h
// STIGREYN AUV — Centralized Feature Control
// ═══════════════════════════════════════════════════════════════
// PURPOSE:
// Single source of truth for all subsystem enable/disable
// Staged bring-up — enable one subsystem at a time
// Preserved architectural intent — nothing gets forgotten
// Safe debugging — isolate modules without touching their code
//
// RULES:
// 1 = enabled   0 = disabled
// Never scatter feature defines across module files
// All enables/disables happen here and only here
// Each flag has a TODO note explaining what's needed to enable
// ═══════════════════════════════════════════════════════════════
// BUILD STAGES:
// Stage 1 ✅  GPIO, toolchain
// Stage 2 ✅  TIM2 PWM — PA5 + PA1 confirmed on scope
// Stage 3 ✅  UART debug — 115200 baud, PLL 84MHz
// Stage 4 →   Safety — leak ISR, watchdog, battery ADC
// Stage 5 →   Sensors — IMU, Bar30 I2C
// Stage 6 →   Control modes — MANUAL command interface
// Stage 7 →   Pi integration — UART2 protocol, sleep/wake
// Stage 8 →   ESP32-CAM, pinger, surface recovery
// Stage 9 →   Full integration, bucket test, wet test
// ═══════════════════════════════════════════════════════════════

#ifndef FEATURE_FLAGS_H
#define FEATURE_FLAGS_H

// ── STAGE 3 — CONFIRMED WORKING ─────────────────────────────────
#define FEATURE_UART_DEBUG          1   // UART2 printf to terminal
#define FEATURE_PLL_84MHZ           1   // PLL clock 84MHz
#define FEATURE_PWM_ESC             1   // TIM2 PWM — PA5 + PA1
#define FEATURE_HEARTBEAT_LED       1   // PB0 blink 500ms

// ── STAGE 4 — SAFETY SYSTEM ─────────────────────────────────────
#define FEATURE_LEAK_ISR            1   // leak detector EXTI PC13
//  ✓ enable now — hardware configured, ISR written
//  ✓ pull-up on PC13, falling edge trigger
//  ✓ latched fault, operator reset only

#define FEATURE_WATCHDOG            0   // IWDG hardware watchdog
//  ✗ NOT YET — enable only after:
//    ✓ main loop stable and tested
//    ✓ all init functions completing reliably
//    ✓ UART startup log printing cleanly every boot
//    ✓ leak ISR tested and confirmed
//  WARNING: once IWDG started it CANNOT be stopped without reset
//  Enable by changing 0 → 1 AND adding safety_watchdog_init()

#define FEATURE_BATTERY_ADC         0   // ADC1 CH0 — PA0 voltage
//  ✗ NOT YET — enable after:
//    ✓ voltage divider circuit designed and built
//    ✓ ADC calibration against known voltage
//    ✓ BATT_WARN_MV / BATT_CRIT_MV thresholds validated
//    Stage 4b

// ── STAGE 5 — SENSORS ───────────────────────────────────────────
#define FEATURE_IMU                 0   // I2C1 — IMU on PB8/PB9
//  ✗ NOT YET — enable after:
//    ✓ I2C1 bare metal init complete
//    ✓ I2C recovery (timeout + bus reset) implemented
//    ✓ IMU I2C address confirmed on bus scanner
//    ✓ raw register reads verified before driver written

#define FEATURE_BAR30               0   // I2C1 — Bar30 depth sensor
//  ✗ NOT YET — enable after:
//    ✓ FEATURE_IMU working — shares same I2C bus
//    ✓ Bar30 I2C address confirmed (0x76)
//    ✓ pressure to depth conversion formula validated

// ── STAGE 6 — CONTROL MODES ─────────────────────────────────────
#define FEATURE_MANUAL_CONTROL      0   // UART command interface
//  ✗ NOT YET — enable after:
//    ✓ UART RX interrupt or polling implemented
//    ✓ command parser written (throttle, steering, arm, disarm)
//    ✓ STATE_MANUAL tested with bench ESC (no thrusters)
//    ✓ Drive_Set() mixing function validated on scope

#define FEATURE_ESC_ARMING          0   // 2s neutral arming sequence
//  ✗ NOT YET — enable before first ESC connection
//    ✓ ESC_PWM_Arm() function written
//    ✓ arming beep sequence confirmed
//    ✓ STATE_ARMED transition tested
//    Add before any ESC or thruster is connected

// ── STAGE 7 — PI INTEGRATION ────────────────────────────────────
#define FEATURE_PI_COMMS            0   // UART2 protocol to Pi
//  ✗ NOT YET — enable after:
//    ✓ Pi UART script written and tested
//    ✓ message format defined (JSON or CSV)
//    ✓ Pi sleep/wake via PB4 tested
//    ✓ pre-launch checklist protocol agreed

#define FEATURE_PI_SLEEP_WAKE       0   // PB4 Pi power control
//  ✗ NOT YET — enable with FEATURE_PI_COMMS
//    ✓ Pi boot time measured — know how long to wait
//    ✓ Pi ready signal defined (UART acknowledgement)

// ── STAGE 8 — SURFACE RECOVERY ──────────────────────────────────
#define FEATURE_PINGER              0   // acoustic pinger GPIO
//  ✗ NOT YET — enable after:
//    ✓ pinger hardware selected and wired
//    ✓ GPIO pin assigned in pin_config.h
//    ✓ pulse timing (1Hz) implemented via timer tick
//    ✓ STATE_SURFACE behaviour validated in tank

#define FEATURE_ESP32_CAM_TRIGGER   0   // PC1 anomaly trigger input
//  ✗ NOT YET — enable after:
//    ✓ ESP32-CAM firmware written
//    ✓ trigger signal tested (active HIGH on detect)
//    ✓ Pi wake sequence on trigger validated
//    ✓ burst imaging pipeline tested

// ── STAGE 9 — AUTONOMY ──────────────────────────────────────────
#define FEATURE_AUTONOMY            0   // Pi autonomous commands
//  ✗ NOT YET — enable after:
//    ✓ ALL previous stages validated
//    ✓ heading hold working with IMU
//    ✓ depth hold working with Bar30
//    ✓ tethered RC testing complete
//    ✓ shallow water tank test complete
//    ✓ watchdog enabled and tested
//    ✓ full fault recovery tested

// ── DEVELOPMENT AIDS ────────────────────────────────────────────
#define FEATURE_REGISTER_DUMP       0   // print TIM2/UART registers at boot
//  Useful for debugging peripheral init issues
//  Disable for production — verbose output

#define FEATURE_STATE_PRINT         1   // print state transitions to UART
//  Useful now — shows state machine working
//  Disable later to reduce UART noise in production

// ═══════════════════════════════════════════════════════════════
// USAGE IN CODE:
//
// #if FEATURE_LEAK_ISR
//     safety_init();
// #else
//     printf("WARNING: leak ISR disabled\r\n");
// #endif
//
// #if FEATURE_WATCHDOG
//     safety_watchdog_kick();
// #endif
// ═══════════════════════════════════════════════════════════════

#endif // FEATURE_FLAGS_H
