// ═══════════════════════════════════════════════════════════════
// app.c
// STIGREYN AUV — Application State Machine
// ═══════════════════════════════════════════════════════════════
// Owns the AUV state machine and high-level control flow
// Safety events come from safety.c via shared fault flags
// Hardware commands go down to esc_pwm.c
// ═══════════════════════════════════════════════════════════════

#include "app.h"
#include "safety.h"
#include "esc_pwm.h"
#include "main.h"
#include <stdio.h>

// ── STATE MACHINE VARIABLES ─────────────────────────────────────
// Defined here — declared extern in app.h
volatile auv_state_t auv_state = STATE_BOOT;

// ── CONTROL VARIABLES ───────────────────────────────────────────
// Back pocket — populated when MANUAL/AUTO states active
static int16_t throttle_cmd = 0;   // -100 to +100
static int16_t steering_cmd = 0;   // -100 to +100

// ═══════════════════════════════════════════════════════════════
// app_init
// ───────────────────────────────────────────────────────────────
// Purpose: transition from BOOT to SAFE
// Call:    once at startup after all hardware init
// ═══════════════════════════════════════════════════════════════
void app_init(void)
{
    throttle_cmd = 0;
    steering_cmd = 0;
    auv_state    = STATE_SAFE;

    printf("App: state machine init - STATE_SAFE\r\n");
}

// ═══════════════════════════════════════════════════════════════
// app_update
// ───────────────────────────────────────────────────────────────
// Purpose: run one tick of the state machine
// Call:    every main loop tick — 100Hz (10ms)
// ═══════════════════════════════════════════════════════════════
void app_update(void)
{
    // ── Safety checks first — every tick ─────────────────────
    // safety_update checks fault flags and manages
    // STATE_FAULT and STATE_SURFACE transitions
    safety_update();
    safety_watchdog_kick();

    // ── State machine ─────────────────────────────────────────
    switch (auv_state)
    {
        // ── BOOT — startup complete, go to SAFE ──────────────
        case STATE_BOOT:
            ESC_PWM_Failsafe();
            auv_state = STATE_SAFE;
            printf("App: STATE_BOOT complete\r\n");
            break;

        // ── SAFE — thrusters neutral, waiting ────────────────
        case STATE_SAFE:
            ESC_PWM_Failsafe();
            // ── TODO: wait for arm command via UART ──────────
            // Stage 6: if UART receives "ARM" → STATE_ARMED
            break;

        // ── ARMED — ESC armed, motors neutral, ready ─────────
        case STATE_ARMED:
            ESC_PWM_Set_Both(0);
            // ── TODO: wait for drive command via UART ─────────
            // Stage 6: if UART receives "MANUAL" → STATE_MANUAL
            break;

        // ── MANUAL — throttle/steering commands ──────────────
        case STATE_MANUAL:
            // ── TODO: MANUAL CONTROL ─────────────────────────
            // Stage 6: parse UART commands into throttle/steering
            // Drive_Set(throttle_cmd, steering_cmd);
            // Ignore for now — continue
            // ─────────────────────────────────────────────────
            ESC_PWM_Failsafe();  // safe until implemented
            break;

        // ── AUTO — Pi sends mission commands ─────────────────
        case STATE_AUTO:
            // ── TODO: AUTONOMOUS ─────────────────────────────
            // Stage 7: receive speed/heading from Pi via UART2
            // Drive_Set(throttle_cmd, steering_cmd);
            // Ignore for now — continue
            // ─────────────────────────────────────────────────
            ESC_PWM_Failsafe();  // safe until implemented
            break;

        // ── SURFACE — fault recovery, passive ascent ─────────
        case STATE_SURFACE:
            // Motors stay neutral — handled in safety_update()
            // ── TODO: SURFACE BEHAVIOUR ──────────────────────
            // Stage 8: pulse pinger GPIO at 1Hz
            // Stage 8: keep UART logging active
            // Stage 8: Pi saves remaining data
            // Ignore for now — continue
            // ─────────────────────────────────────────────────
            break;

        // ── FAULT / DEFAULT — always safe ────────────────────
        case STATE_FAULT:
        default:
            ESC_PWM_Failsafe();
            break;
    }
}

// ═══════════════════════════════════════════════════════════════
// app_fault
// ───────────────────────────────────────────────────────────────
// Purpose: force fault from any context
// Hardware made safe first — then state updated
// ═══════════════════════════════════════════════════════════════
void app_fault(void)
{
    // hardware first — fastest path
    TIM2->CCR1 = PWM_NEUTRAL_US;
    TIM2->CCR2 = PWM_NEUTRAL_US;
    auv_state  = STATE_FAULT;
    printf("App: FAULT forced - motors neutral\r\n");
}

// ═══════════════════════════════════════════════════════════════
// app_reset
// ───────────────────────────────────────────────────────────────
// Purpose: operator initiated fault clear
// ONLY call when vehicle is safely recovered at surface
// Never call mid-dive
// ═══════════════════════════════════════════════════════════════
void app_reset(void)
{
    leak_fault     = 0;
    battery_fault  = 0;
    watchdog_fault = 0;
    throttle_cmd   = 0;
    steering_cmd   = 0;
    auv_state      = STATE_SAFE;
    printf("App: fault cleared by operator - STATE_SAFE\r\n");
}
