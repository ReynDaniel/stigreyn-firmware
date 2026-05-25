// ═══════════════════════════════════════════════════════════════
// safety.h
// STIGREYN AUV — Safety System Public Interface
// Leak detection, fault management, watchdog (future)
// ═══════════════════════════════════════════════════════════════
// SAFETY PHILOSOPHY:
// Fault detected  → make hardware safe immediately
// Fault reason    → classify what happened
// Fault severity  → decide monitor / recover / surface later
// Leak            → immediate hard stop, latch fault, classify critical
// Battery         → staged response: warn / critical / dead
// Watchdog        → last-resort reset if firmware stops running
//
// ISR rule: make safe → set flag → set STATE_FAULT → clear interrupt → exit
// Main loop: classify → report → log → decide recovery policy
// ═══════════════════════════════════════════════════════════════

#ifndef SAFETY_H
#define SAFETY_H

#include <stdint.h>
#include "feature_flags.h"
#include "app.h"
#include "pin_config.h"

// ── BATTERY THRESHOLDS ──────────────────────────────────────────
// 4S lithium (12.0V–16.8V)
// ADC is 12-bit (0–4095) on 3.3V reference
// Battery connected via voltage divider — scale factor TBD Stage 4
#define BATT_WARN_MV     13000   // 13.0V — warn Pi, start logging
#define BATT_CRIT_MV     12000   // 12.0V — stop motors, ascend
#define BATT_DEAD_MV     11000   // 11.0V — minimum, BMS will cut

// ── WATCHDOG ────────────────────────────────────────────────────
// Watchdog enable is controlled centrally in feature_flags.h:
//   FEATURE_WATCHDOG = 0  development/debug mode
//   FEATURE_WATCHDOG = 1  field-test mode after validation
//
// Important:
// Once the STM32 independent watchdog is enabled, it cannot be
// disabled again until reset. Keep disabled during active debugging.
#define WATCHDOG_TIMEOUT_MS  1000   // 1 second — if main loop hangs

// ── PUBLIC FUNCTIONS ────────────────────────────────────────────

// Initialise safety system
// Sets up leak detector EXTI interrupt on PC13
// Call after gpio_init() and rcc_init() in main
void safety_init(void);

// Call every main loop tick
// Checks fault flags, manages state transitions
// Handles battery monitoring when ADC ready
void safety_update(void);

// Kick watchdog — call regularly in main loop
// Does nothing until WATCHDOG_ENABLED is defined
void safety_watchdog_kick(void);

// ── TODO: WATCHDOG INIT ─────────────────────────────────────────
// Enable only when FEATURE_WATCHDOG = 1 in feature_flags.h
// void safety_watchdog_init(void);
// Stage 4b/5: after leak ISR tested, UART logging confirmed,
// and main-loop health aggregation is implemented.
// ─────────────────────────────────────────────────────────────────

// ── TODO: BATTERY ADC ───────────────────────────────────────────
// Stage 4b: uint16_t safety_read_battery_mv(void);
// Reads PA0 ADC, scales to millivolts via divider ratio.
// BATT_WARN_MV  -> FAULT_BATT_WARN + SEVERITY_WARN
// BATT_CRIT_MV  -> FAULT_BATT_CRIT + SEVERITY_CRITICAL
// BATT_DEAD_MV  -> immediate failsafe + recovery policy
// ─────────────────────────────────────────────────────────────────

#endif // SAFETY_H
