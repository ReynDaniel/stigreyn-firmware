// ═══════════════════════════════════════════════════════════════
// esc_pwm.c
// STIGREYN AUV — Layer 0: ESC / Thruster PWM Driver
// Board:  STM32F446RE Nucleo-F446RE
// Author: Daniel Reynolds — REYN Consultancy
// version history:
// v1.0.0 2026-06-21 — initial implementation
// ═══════════════════════════════════════════════════════════════
// LAYER OVERVIEW:
// Layer 0 (this file) — motor_cmd (-100 to +100) → PWM µs
// Layer 1 (control.c) — throttle + turn → motor_cmd
// Layer 2 (nav.c)     — heading/depth → throttle + turn  [TODO Stage 5]
// Layer 3 (mission.c) — waypoints → heading/depth        [TODO Stage 7]
// ═══════════════════════════════════════════════════════════════
// BARE METAL — no HAL in control paths
// CubeMX/HAL used only for clock and peripheral init (main.c)
// All PWM writes are direct TIM2 register access
// ═══════════════════════════════════════════════════════════════
// MOTOR INVERSION:
// Port  prop CW  viewed from rear — normal mapping
// Stbd  prop CCW viewed from rear — inverted mapping
// Both produce forward thrust when given same positive motor_cmd
// Inversion handled here — Layer 1 sends same command to both
// ═══════════════════════════════════════════════════════════════

#include "esc_pwm.h"

// ── MOTOR INVERSION FLAGS ───────────────────────────────────────
// Compile-time constants — resolved before code runs
// Change here if props physically swapped during assembly
#define ESC_PORT_INVERT   0   // port  CW  — no inversion
#define ESC_STBD_INVERT   1   // stbd  CCW — invert to match port

// ── PRIVATE FUNCTION DECLARATION ───────────────────────────────
// static = private to this file — Layer 1 cannot call directly
static void ESC_PWM_Set(motor_id_t motor, int16_t motor_cmd);

// ═══════════════════════════════════════════════════════════════
// ESC_PWM_Init
// ───────────────────────────────────────────────────────────────
// Purpose: start bare metal PWM output on both thruster channels
// Call:    once at startup, after MX_TIM2_Init() in main.c
// Result:  TIM2 running, both outputs active, neutral signal out
// ═══════════════════════════════════════════════════════════════
void ESC_PWM_Init(void)
{
    // Enable TIM2 CH1 and CH2 output — connect timer to pins
    // CCER = Capture/Compare Enable Register
    TIM2->CCER |= (1 << 0);    // 0000 0001 — CC1E: enable CH1 output PA5
    TIM2->CCER |= (1 << 4);    // 0001 0000 — CC2E: enable CH2 output PA1

    // Start TIM2 counter
    // CR1 = Control Register 1
    TIM2->CR1  |= (1 << 0);    // 0000 0001 — CEN: counter enable

    // Immediately set neutral — safety before anything else
    // ESC must see 1500µs to arm — do this before connecting power
    ESC_PWM_Failsafe();
}

// ═══════════════════════════════════════════════════════════════
// ESC_PWM_Failsafe
// ───────────────────────────────────────────────────────────────
// Purpose: immediately stop both thrusters
// Call:    on ANY safety event:
//          leak detected, watchdog timeout, low battery,
//          invalid motor command, comms lost
// Result:  both thrusters → 1500µs → passive buoyant ascent
// ───────────────────────────────────────────────────────────────
// BARE METAL DIRECT WRITE — fastest possible path
// No function call overhead in safety critical code
// ═══════════════════════════════════════════════════════════════
void ESC_PWM_Failsafe(void)
{
    TIM2->CCR1 = PWM_NEUTRAL_US;    // port  → 1500µs → stop
    TIM2->CCR2 = PWM_NEUTRAL_US;    // stbd  → 1500µs → stop

    // ── TODO: SAFETY EVENT LOG ──────────────────────────────
    // Stage 7: send failsafe event to Pi via UART2
    // Pi logs: reason, timestamp, depth, heading, battery
    // Ignore for now — continue
    // ─────────────────────────────────────────────────────────
}

// ═══════════════════════════════════════════════════════════════
// ESC_PWM_Set_Port
// ───────────────────────────────────────────────────────────────
// Purpose: set port thruster motor command
// Input:   motor_cmd — MOTOR_CMD_MIN (-100) to MOTOR_CMD_MAX (+100)
// Layer 1 sends same signed command to port and stbd
// This function passes to internal ESC_PWM_Set()
// ═══════════════════════════════════════════════════════════════
void ESC_PWM_Set_Port(int16_t motor_cmd)
{
    ESC_PWM_Set(MOTOR_PORT, motor_cmd);
}

// ═══════════════════════════════════════════════════════════════
// ESC_PWM_Set_Stbd
// ───────────────────────────────────────────────────────────────
// Purpose: set starboard thruster motor command
// Input:   motor_cmd — same convention as port
//          inversion applied internally in ESC_PWM_Set()
// ═══════════════════════════════════════════════════════════════
void ESC_PWM_Set_Stbd(int16_t motor_cmd)
{
    ESC_PWM_Set(MOTOR_STBD, motor_cmd);
}

// ═══════════════════════════════════════════════════════════════
// ESC_PWM_Set_Both
// ───────────────────────────────────────────────────────────────
// Purpose: set both thrusters same motor command
//          straight line — no steering input
// Input:   motor_cmd — same convention as port/stbd
// ═══════════════════════════════════════════════════════════════
void ESC_PWM_Set_Both(int16_t motor_cmd)
{
    ESC_PWM_Set(MOTOR_PORT, motor_cmd);
    ESC_PWM_Set(MOTOR_STBD, motor_cmd);
}

// ═══════════════════════════════════════════════════════════════
// ESC_PWM_Set  [PRIVATE]
// ───────────────────────────────────────────────────────────────
// Purpose: core motor command → PWM conversion
//          handles inversion, clamp, calculation, register write
// Input:   motor    — MOTOR_PORT or MOTOR_STBD (motor_id_t)
//          motor_cmd — -100 to +100
// Note:    static — Layer 1 cannot call directly
//          default case triggers failsafe on invalid motor
// ═══════════════════════════════════════════════════════════════
static void ESC_PWM_Set(motor_id_t motor, int16_t motor_cmd)
{
    // ── Step 1: apply physical motor inversion ───────────────
    // Port  CW  — no inversion — forward command = forward thrust
    // Stbd  CCW — inverted    — forward command must be negated
    //             so CCW prop produces forward thrust same as CW
    if (motor == MOTOR_STBD)
    {
        #if ESC_STBD_INVERT
            motor_cmd = -motor_cmd;
        #endif
    }
    else // MOTOR_PORT
    {
        #if ESC_PORT_INVERT
            motor_cmd = -motor_cmd;
        #endif
    }

    // ── Step 2: clamp motor command to safe range ────────────
    // Prevents overcommand from Layer 1 mixing calculations
    // e.g. Drive_Set(80, 40) gives port=120 — must clamp to 100
    if (motor_cmd >  MOTOR_CMD_MAX) motor_cmd =  MOTOR_CMD_MAX;
    if (motor_cmd <  MOTOR_CMD_MIN) motor_cmd =  MOTOR_CMD_MIN;

    // ── Step 3: convert motor command to PWM pulse width ─────
    // Open loop mapping — Layer 0 responsibility
    //   -100 → 1100µs full reverse
    //      0 → 1500µs neutral
    //   +100 → 1900µs full forward
    //
    // Formula: PWM = NEUTRAL + (motor_cmd x 4)
    // Scale factor: (1900-1100) / (100-(-100)) = 800/200 = 4
    //
    // ── TODO: DEADBAND SKIP ──────────────────────────────────
    // Before wet test: skip 1460-1540µs deadband
    // Snap to 1500µs if result falls in deadband range
    // Ignore for now — continue
    // ─────────────────────────────────────────────────────────
    //
    // ── TODO: VOLTAGE COMPENSATION ──────────────────────────
    // Stage 4: scale motor_cmd based on battery ADC reading
    // Lower voltage → higher PWM to maintain consistent thrust
    // Ignore for now — continue
    // ─────────────────────────────────────────────────────────
    uint16_t pwm_us = (uint16_t)(PWM_NEUTRAL_US + (motor_cmd * 4));

    // ── Step 4: write PWM to correct timer register ──────────
    // Bare metal direct register write — no HAL overhead
    // switch default triggers failsafe on invalid motor_id_t
    // (C enums are integer-compatible — defensive check needed)
    switch (motor)
    {
        case MOTOR_PORT:
            TIM2->CCR1 = pwm_us;    // port  thruster PA5 CH1
            break;

        case MOTOR_STBD:
            TIM2->CCR2 = pwm_us;    // stbd  thruster PA1 CH2
            break;

        default:
            // Invalid motor identity — stop everything
            // Safety critical: unknown motor = failsafe
            ESC_PWM_Failsafe();
            break;
    }
}

// ═══════════════════════════════════════════════════════════════
// ── TODO: ESC ARMING SEQUENCE ─────────────────────────────────
// Stage 3: void ESC_PWM_Arm(void)
// Send PWM_NEUTRAL_US for 2000ms → ESC plays startup beeps → armed
// Must complete before thrusters will respond to commands
// Add before first ESC connection test
// ─────────────────────────────────────────────────────────────

// ── TODO: RAMP LIMITING ───────────────────────────────────────
// Stage 6: smooth acceleration profile
// Glider/aircraft dynamics — sudden thrust = pitch disturbance
// Limit rate of change: max ±N motor_cmd units per control tick
// Add at Stage 6 when control loop timer is running
// ─────────────────────────────────────────────────────────────

// ── TODO: RPM FEEDBACK ────────────────────────────────────────
// Stage 5+: read ESC telemetry if available
// Blue Robotics Basic ESC has no telemetry
// Future: upgrade to ESC with UART telemetry for closed loop
// ─────────────────────────────────────────────────────────────
