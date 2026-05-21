// ═══════════════════════════════════════════════════════════════
// esc_pwm.h
// STIGREYN AUV — Layer 0: ESC / Thruster PWM Driver
// Board:  STM32F446RE Nucleo-F446RE
// Author: Daniel Reynolds — REYN Consultancy
// v1.0.0 2026-06-21 — initial implementation
// ═══════════════════════════════════════════════════════════════
// LAYER OVERVIEW:
// Layer 0 (this file) — motor_cmd (-100 to +100) → PWM µs
// Layer 1 (control.c) — throttle + turn → motor_cmd
// Layer 2 (nav.c)     — heading/depth → throttle + turn  [TODO Stage 5]
// Layer 3 (mission.c) — waypoints → heading/depth        [TODO Stage 7]
// ═══════════════════════════════════════════════════════════════
// PHYSICAL MOTOR LAYOUT:
// Port  — TIM2 CH1 — PA5 (D13) — CW  viewed from rear — normal mapping
// Stbd  — TIM2 CH2 — PA1 (A1)  — CCW viewed from rear — inverted mapping
// Counter-rotating INWARD — tops rotate toward centreline
// ═══════════════════════════════════════════════════════════════
// USAGE:
// 1. Call ESC_PWM_Init() once at startup after MX_TIM2_Init()
// 2. Call ESC_PWM_Failsafe() on ANY safety event
// 3. Use Drive_Set() in control.c for normal operation
//    or ESC_PWM_Set_Port/Stbd directly for bench testing
// ═══════════════════════════════════════════════════════════════

#ifndef ESC_PWM_H
#define ESC_PWM_H

#include "main.h"
#include "pin_config.h"

// ── MOTOR IDENTITY ──────────────────────────────────────────────
// Use these names — never pass raw channel numbers
// Note: C enums are integer-compatible so not full type-safe
//       defensive default case in ESC_PWM_Set() handles invalid values
typedef enum {
    MOTOR_PORT = 0,   // port  thruster — TIM2 CH1 — PA5 — CW
    MOTOR_STBD = 1    // stbd  thruster — TIM2 CH2 — PA1 — CCW
} motor_id_t;

// ── PWM PULSE WIDTH DEFINES ─────────────────────────────────────
// All values in microseconds — CCR value = µs directly at 1MHz timer
#define PWM_NEUTRAL_US      1500    // stop — safe default, ESC arms here
#define PWM_MIN_US          1100    // full reverse
#define PWM_MAX_US          1900    // full forward
#define PWM_DEADBAND_LO_US  1460    // deadband low  — avoid, motor twitches
#define PWM_DEADBAND_HI_US  1540    // deadband high — avoid, motor twitches
#define PWM_CRUISE_LO_US    1560    // cruise start  — most efficient region
#define PWM_CRUISE_HI_US    1620    // cruise end
#define PWM_DIVE_LO_US      1620    // dive start
#define PWM_DIVE_HI_US      1680    // dive end
#define PWM_HIGH_PWR_US     1800    // high power limit — short duration only
#define PWM_MAX_FWD_US      1900    // absolute maximum forward
#define PWM_MAX_REV_US      1100    // absolute maximum reverse

// ── MOTOR COMMAND LIMITS ────────────────────────────────────────
// Normalised motor commands — Layer 0 input range
#define MOTOR_CMD_MAX    100    // full forward
#define MOTOR_CMD_MIN   -100    // full reverse
#define MOTOR_CMD_STOP     0    // neutral

// ── PUBLIC FUNCTIONS ────────────────────────────────────────────

// Initialise PWM output — call once at startup
// Starts TIM2 CH1 + CH2, immediately sets neutral
void ESC_PWM_Init(void);

// Safety stop — both thrusters → PWM_NEUTRAL_US instantly
// Call on: leak detected, watchdog timeout, low battery,
//          invalid motor command, any safety event
void ESC_PWM_Failsafe(void);

// Set individual thruster motor command
// motor_cmd: MOTOR_CMD_MIN (-100) to MOTOR_CMD_MAX (+100)
// Inversion applied internally — Layer 1 unaware of prop direction
void ESC_PWM_Set_Port(int16_t motor_cmd);
void ESC_PWM_Set_Stbd(int16_t motor_cmd);

// Set both thrusters same command — straight line
// Each applies own inversion internally
void ESC_PWM_Set_Both(int16_t motor_cmd);

// ── TODO: FUTURE PUBLIC FUNCTIONS ───────────────────────────────
// Stage 3: void ESC_PWM_Arm(void)
//          arming sequence — 1500µs for 2s → ESC beeps → armed
// Stage 4: uint8_t ESC_PWM_IsArmed(void)
//          returns 1 if ESC armed and ready
// Stage 4: voltage compensation — scale PWM for battery level
// Stage 6: void ESC_PWM_SetRampRate(uint8_t rate_per_tick)
//          smooth acceleration profile — glider dynamics
// ─────────────────────────────────────────────────────────────────

#endif // ESC_PWM_H
