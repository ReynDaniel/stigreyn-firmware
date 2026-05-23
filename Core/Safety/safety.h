// ═══════════════════════════════════════════════════════════════
// safety.h
// STIGREYN AUV — Safety System Public Interface
// Leak detection, fault management, watchdog (future)
// ═══════════════════════════════════════════════════════════════
// SAFETY PHILOSOPHY:
// Leak    → immediate hard stop, no negotiation, latch fault
// Battery → staged graceful response, save data first
// Watchdog→ hardware reset, firmware hung = unknown state
//
// ISR rule: make safe → set flag → clear interrupt → exit
// Main loop: report → log → decide → recover
// ═══════════════════════════════════════════════════════════════

#ifndef SAFETY_H
#define SAFETY_H

#include <stdint.h>
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
// DISABLED during development — enable after system is stable
// Uncomment to enable:
// #define WATCHDOG_ENABLED
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
// Enable after system stable — define WATCHDOG_ENABLED above
// void safety_watchdog_init(void);
// Stage 4: after leak ISR tested and UART logging confirmed
// ─────────────────────────────────────────────────────────────────

// ── TODO: BATTERY ADC ───────────────────────────────────────────
// Stage 4: uint16_t safety_read_battery_mv(void);
// Reads PA0 ADC, scales to millivolts via divider ratio
// ─────────────────────────────────────────────────────────────────

#endif // SAFETY_H
