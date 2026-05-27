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
#include "feature_flags.h"
#include "main.h"
#include <stdio.h>

// ── STATE MACHINE VARIABLES ─────────────────────────────────────
// Defined here — declared extern in app.h
volatile auv_state_t auv_state = STATE_BOOT;
volatile fault_reason_t   fault_reason   = FAULT_NONE;
volatile fault_severity_t fault_severity = SEVERITY_NONE;
volatile uint32_t fault_active_mask = 0;
// Future black-box logging:
// fault_active_mask is intended for UART telemetry, mission logs,
// post-recovery diagnostics, and possible future flash/EEPROM persistence.
// It allows multiple simultaneous faults to be retained without losing
// the primary/latest fault_reason used for immediate response.

// ── CONTROL VARIABLES ───────────────────────────────────────────
// Back pocket — populated when MANUAL/AUTO states active

static int16_t throttle_cmd = 0;   // -100 to +100
static int16_t steering_cmd = 0;   // -100 to +100

// ── FAULT STRING HELPERS ──────────────────────────────────────
// Convert enums into readable UART debug text.
// Numeric codes remain stable for logging and future Pi parsing.
static const char* fault_reason_string(fault_reason_t reason)
{
    switch (reason)
    {
        case FAULT_NONE:            return "NONE";
        case FAULT_LEAK:            return "LEAK";
        case FAULT_BATT_CRIT:       return "BATT_CRIT";
        case FAULT_BATT_WARN:       return "BATT_WARN";
        case FAULT_I2C_LOCKUP:      return "I2C_LOCKUP";
        case FAULT_SENSOR_TIMEOUT:  return "SENSOR_TIMEOUT";
        case FAULT_UART_TIMEOUT:    return "UART_TIMEOUT";
        case FAULT_WATCHDOG:        return "WATCHDOG";
        default:                    return "UNKNOWN";
    }
}

static const char* fault_severity_string(fault_severity_t severity)
{
    switch (severity)
    {
        case SEVERITY_NONE:         return "NONE";
        case SEVERITY_WARN:         return "WARN";
        case SEVERITY_RECOVERABLE:  return "RECOVERABLE";
        case SEVERITY_CRITICAL:     return "CRITICAL";
        default:                    return "UNKNOWN";
    }
}

static const char* auv_state_string(auv_state_t state)
{
    switch (state)
    {
        case STATE_BOOT:     return "BOOT";
        case STATE_SAFE:     return "SAFE";
        case STATE_ARMED:    return "ARMED";
        case STATE_MANUAL:   return "MANUAL";
        case STATE_AUTO:     return "AUTO";
        case STATE_FAULT:    return "FAULT";
        case STATE_SURFACE:  return "SURFACE";
        default:             return "UNKNOWN";
    }
}

// ═══════════════════════════════════════════════════════════════
// app_init
// ───────────────────────────────────────────────────────────────
// Purpose: transition from BOOT to SAFE
// Call:    once at startup after all hardware init
// ═══════════════════════════════════════════════════════════════
void app_init(void)
{
    throttle_cmd   = 0;
    steering_cmd   = 0;
    fault_reason   = FAULT_NONE;
    fault_severity = SEVERITY_NONE;
    fault_active_mask = 0;
    auv_state      = STATE_SAFE;

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
    static auv_state_t previous_state = STATE_BOOT;

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

#if FEATURE_ESC_ARMING
            // Stage 5a: first arming trigger uses Nucleo USER button PC13.
            // Button pressed = logic LOW on most Nucleo boards.
            // Later this can also be triggered by UART command "arm".
            if (!(GPIOC->IDR & (1 << 13)))
            {
                ESC_PWM_Arm();
                auv_state = STATE_ARMED;
                printf("App: STATE_SAFE -> STATE_ARMED\r\n");
            }
#else
            // ESC arming disabled in feature_flags.h
#endif
            break;

        // ── ARMED — ESC armed, motors neutral, ready ─────────
        case STATE_ARMED:
            ESC_PWM_Set_Both(0);
            // Stage 5b/5c: drive_update() watches UART commands.
            // First W/A/S/D command will transition to STATE_MANUAL.
            break;

        // ── MANUAL — throttle/steering commands ──────────────
        case STATE_MANUAL:
            // Stage 5c: manual control is owned by drive_update().
            // Do not call ESC_PWM_Failsafe() here or it will overwrite
            // valid W/A/S/D drive commands each loop tick.
            // Future manual inputs may include UART terminal control,
            // RC override, or tethered debug driving.
            // Suggested priority: RC override > UART terminal > autonomy.
            break;

        // ── AUTO — Pi sends mission commands ─────────────────
        case STATE_AUTO:
            // ── TODO: AUTONOMOUS ─────────────────────────────
            // Stage 7: receive speed/heading from Pi via UART2
            // Future autonomy rule:
            // STM32 remains authoritative for safety, watchdog,
            // leak response, and motor neutral failsafe.
            // Pi provides mission intent only and must not bypass safety.
            // Drive_Set(throttle_cmd, steering_cmd);
            // Ignore for now — continue
            // ─────────────────────────────────────────────────
            ESC_PWM_Failsafe();  // safe until implemented
            break;

        // ── SURFACE — fault recovery, passive ascent ─────────
        case STATE_SURFACE:
            // Motors stay neutral — handled in safety_update()
            // ── TODO: SURFACE / RECOVERY BEHAVIOUR ───────────
            // Stage 8: pulse acoustic pinger GPIO at 1Hz
            // Stage 8: keep reduced-power UART telemetry active
            // Stage 8: wake Pi long enough to save remaining data
            // Stage 8: consider GPS/acoustic/radio beacon only when surfaced
            // Enter this state only after recovery policy commits to surfacing.
            // ─────────────────────────────────────────────────
            break;

        // ── FAULT / DEFAULT — always safe ────────────────────
        case STATE_FAULT:
        default:
            ESC_PWM_Failsafe();
            break;
    }

#if FEATURE_STATE_PRINT
    if (auv_state != previous_state)
    {
        printf("STATE: %s -> %s | faults=0x%08lX primary=%d:%s severity=%d:%s\r\n",
               auv_state_string(previous_state),
               auv_state_string(auv_state),
               (unsigned long)fault_active_mask,
               fault_reason,
               fault_reason_string(fault_reason),
               fault_severity,
               fault_severity_string(fault_severity));

        previous_state = auv_state;
    }
#endif
}

// ═══════════════════════════════════════════════════════════════
// app_fault
// ───────────────────────────────────────────────────────────────
// Purpose: classify a fault and force the vehicle into a safe state
// Hardware made safe first — then state and fault record are updated
// ═══════════════════════════════════════════════════════════════
void app_fault(fault_reason_t reason, fault_severity_t severity)
{
    // Hardware safe first — always.
    ESC_PWM_Failsafe();

    fault_reason   = reason;
    fault_severity = severity;
    if (reason != FAULT_NONE)
    {
        fault_active_mask |= FAULT_BIT(reason);
    }
    auv_state      = STATE_FAULT;

    // Do not automatically surface for every fault.
    // STATE_SURFACE is only entered after recovery policy commits to surfacing.
    if (severity == SEVERITY_CRITICAL)
    {
        printf("FAULT [%d:%s] severity [%d:%s] mask=0x%08lX - surface decision required\r\n",
               reason,
               fault_reason_string(reason),
               severity,
               fault_severity_string(severity),
               (unsigned long)fault_active_mask);

        // TODO Stage 4b/8:
        // Decide whether to transition to STATE_SURFACE after recovery policy exists.
        // For now, remain in STATE_FAULT so fault latch and neutral PWM can be validated.
        // auv_state = STATE_SURFACE;
        //
        // Future pressure fault note:
        // Add internal hull pressure monitoring as a relative fault source.
        // Because the printed/resin hull is flexible, internal pressure may shift as
        // the hull compresses under external depth pressure. Compare internal pressure
        // against the sealed startup baseline, not only against absolute atmosphere.
        // Also account for pressure locked in during assembly when fitting the rear plug.
        // Potential future fault: FAULT_INTERNAL_PRESSURE_DELTA.
        // Future pressure policy:
        // classify slow pressure drift separately from sudden pressure jump.
        // Slow drift may be WARN/RECOVERABLE; sudden jump may be CRITICAL.
    }
    else
    {
        printf("FAULT [%d:%s] severity [%d:%s] mask=0x%08lX - assessing\r\n",
               reason,
               fault_reason_string(reason),
               severity,
               fault_severity_string(severity),
               (unsigned long)fault_active_mask);

        // TODO Stage 5/8:
        // Recoverable/warning faults may include internal pressure drift.
        // Treat pressure as delta from startup sealed baseline before deciding whether
        // to continue, pause, recheck, or commit to surfacing.
    }
}

// ═══════════════════════════════════════════════════════════════
// app_reset
// ───────────────────────────────────────────────────────────────
// Purpose: operator initiated fault clear
// ONLY call when vehicle is safely recovered at surface
// Never call mid-dive
// Future operator reset policy:
// require explicit recovered/surfaced confirmation before clearing faults.
void app_reset(void)
{
    leak_fault     = 0;
    battery_fault  = 0;
    watchdog_fault = 0;
    fault_reason   = FAULT_NONE;
    fault_severity = SEVERITY_NONE;
    fault_active_mask = 0;
    throttle_cmd   = 0;
    steering_cmd   = 0;
    auv_state      = STATE_SAFE;
    printf("App: fault cleared by operator - STATE_SAFE\r\n");
}
