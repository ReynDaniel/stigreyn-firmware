// ═══════════════════════════════════════════════════════════════
// drive.h
// STIGREYN AUV — Layer 1: Drive / Throttle Control
// ═══════════════════════════════════════════════════════════════
// Sits between UART command input and ESC PWM driver
// Handles: mixing, clamping, test limits, telemetry logging
//
// Layer 1 (this file) — throttle/steering → port/stbd commands
// Layer 0 (esc_pwm.c) — motor commands → PWM microseconds
// ═══════════════════════════════════════════════════════════════

#ifndef DRIVE_H
#define DRIVE_H

#include <stdint.h>
#include "feature_flags.h"
#include "pin_config.h"

// ── TEST MODE LIMITS ────────────────────────────────────────────
// Restrict PWM range for first bench and wet tests
// Widen after initial validation
// Controlled by FEATURE_TEST_LIMITS in feature_flags.h
#if FEATURE_TEST_LIMITS
    #define DRIVE_MAX_FWD_US    1620    // cruise end — safe first test
    #define DRIVE_MIN_REV_US    1380    // gentle reverse only
#else
    #define DRIVE_MAX_FWD_US    PWM_MAX_FWD   // 1900 full range
    #define DRIVE_MIN_REV_US    PWM_MAX_REV   // 1100 full range
#endif

// ── THROTTLE STEP SIZE ───────────────────────────────────────────
// How much each W/S keypress changes throttle
#define DRIVE_THROTTLE_STEP     10  // 10% per keypress
#define DRIVE_STEERING_STEP     10  // 10% per keypress

// ── PUBLIC FUNCTIONS ────────────────────────────────────────────
void drive_init(void);
void drive_update(void);
void Drive_Set(int16_t throttle, int16_t steering);
void drive_print_status(void);

// ── TODO: POT CONTROL ───────────────────────────────────────────
// Reserved: PA4/A2 = ADC1 CH4 for future physical controller
// POT_ADC_PA4 reserved in pin_config.h
// Enable FEATURE_POT_CONTROL when physical controller ready
// ─────────────────────────────────────────────────────────────────

#endif // DRIVE_H
