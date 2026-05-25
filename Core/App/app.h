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
//   ANY STATE → STATE_FAULT   (fault detected)
//   STATE_FAULT = immediate safe state + assessment
//   STATE_SURFACE = committed recovery/surfacing mode
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

// ── FAULT CLASSIFICATION ─────────────────────────────────────
// STATE_FAULT means something abnormal has been detected.
// fault_reason explains what happened.
// fault_severity controls whether the system attempts recovery or commits to surfacing.
typedef enum
{
    FAULT_NONE = 0,

    // Non-recoverable / mission-abort faults
    FAULT_LEAK,
    FAULT_BATT_CRIT,
    FAULT_HULL_PRESSURE,

    // Potentially recoverable faults
    FAULT_BATT_WARN,
    FAULT_I2C_LOCKUP,
    FAULT_SENSOR_TIMEOUT,
    FAULT_UART_TIMEOUT,
    FAULT_WATCHDOG
} fault_reason_t;

// ── FAULT BITMASK HELPER ─────────────────────────────────────
// Allows multiple active faults simultaneously.
// Example:
//     fault_active_mask |= FAULT_BIT(FAULT_LEAK);
//
// Bit positions match fault_reason_t enum values.
#define FAULT_BIT(reason)   (1UL << (uint32_t)(reason))

typedef enum
{
    SEVERITY_NONE = 0,
    SEVERITY_WARN,
    SEVERITY_RECOVERABLE,
    SEVERITY_CRITICAL
} fault_severity_t;

// ── SHARED FAULT FLAGS ──────────────────────────────────────────
// Defined in safety.c — written by ISR, read by app.c
// volatile — ISR writes, main loop reads
// latched  — only app_reset() clears them
// bitmask  — fault_active_mask can store multiple simultaneous faults
extern volatile auv_state_t       auv_state;
extern volatile fault_reason_t    fault_reason;
extern volatile fault_severity_t  fault_severity;
extern volatile uint32_t          fault_active_mask;

extern volatile uint8_t leak_fault;
extern volatile uint8_t battery_fault;
extern volatile uint8_t watchdog_fault;

// ── PUBLIC FUNCTIONS ────────────────────────────────────────────
void app_init(void);     // BOOT → SAFE after all hardware init
void app_update(void);   // call every main loop tick — 100Hz
void app_fault(fault_reason_t reason,
               fault_severity_t severity); // classify + handle fault
void app_reset(void);    // operator reset — surface only

#endif // APP_H
