// ═══════════════════════════════════════════════════════════════
// app.h
// STIGREYN AUV — Application State Machine Public Interface
// ═══════════════════════════════════════════════════════════════
// STATE TRANSITIONS:
//
//   STATE_BOOT
//       ↓ startup checks pass
//   STATE_SAFE
//       ↓ "ARM" command received via UART     [Stage 6]
//   STATE_ARMED
//       ↓ "MANUAL" command received
//   STATE_MANUAL ←──────────────────┐
//       ↓ "AUTO" command             │ "MANUAL" override
//   STATE_AUTO  ────────────────────┘
//
//   ANY STATE → STATE_FAULT   (leak / battery / watchdog)
//   STATE_FAULT → STATE_SURFACE (main loop transitions)
//   STATE_SURFACE → STATE_SAFE  (operator app_reset() only)
//
// ═══════════════════════════════════════════════════════════════

#ifndef APP_H
#define APP_H

#include <stdint.h>

// ── SYSTEM STATE ────────────────────────────────────────────────
typedef enum
{
    STATE_BOOT    = 0,  // startup — running init checks
    STATE_SAFE    = 1,  // motors neutral — not armed
    STATE_ARMED   = 2,  // ESC armed — neutral, ready for commands
    STATE_MANUAL  = 3,  // accepting throttle/turn commands
    STATE_AUTO    = 4,  // Pi sends autonomous commands  [Stage 7]
    STATE_FAULT   = 5,  // safety event — motors neutral immediately
    STATE_SURFACE = 6   // recovery — ascending, pinger, logging [Stage 8]
} auv_state_t;

// ── SHARED FAULT FLAGS ──────────────────────────────────────────
// Defined in safety.c — written by ISR, read by app.c
// volatile — ISR writes, main loop reads
// latched  — only app_reset() clears them
extern volatile auv_state_t auv_state;
extern volatile uint8_t     leak_fault;
extern volatile uint8_t     battery_fault;
extern volatile uint8_t     watchdog_fault;

// ── PUBLIC FUNCTIONS ────────────────────────────────────────────
void app_init(void);     // BOOT → SAFE after all hardware init
void app_update(void);   // call every main loop tick — 100Hz
void app_fault(void);    // force fault from any context
void app_reset(void);    // operator reset — surface only

#endif // APP_H
