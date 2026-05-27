// ═══════════════════════════════════════════════════════════════
// drive.c
// STIGREYN AUV — Layer 1: Drive / Throttle Control
// ═══════════════════════════════════════════════════════════════
// UART command interface:
//   W = throttle up    S = throttle down
//   A = steer left     D = steer right
//   SPACE = neutral stop
//   X = disarm / failsafe
//   ? = print status
//
// Telemetry log on command/status:
//   THR=+30 STR=0 PORT=1620 STBD=1620 STATE=MANUAL
// ═══════════════════════════════════════════════════════════════

#include "drive.h"
#include "esc_pwm.h"
#include "app.h"
#include "safety.h"
#include "main.h"
#include <stdio.h>

// ── DRIVE STATE ─────────────────────────────────────────────────

static int16_t throttle = 0;   // -100 to +100
static int16_t steering = 0;   // -100 to +100

// Stage 5 test clamp in command space, not direct PWM space.
// Full command range remains -100..+100, but early tests are limited.
#ifndef DRIVE_TEST_CMD_MAX
#define DRIVE_TEST_CMD_MAX   30
#endif

#ifndef DRIVE_TEST_CMD_MIN
#define DRIVE_TEST_CMD_MIN  -30
#endif

// ── UART RX — bare metal single byte read ───────────────────────
static int8_t uart2_read_char(void)
{
    if (USART2->SR & (1 << 5))     // RXNE — receive not empty
    {
        return (int8_t)(USART2->DR & 0xFF);
    }
    return -1;  // no data available
}

// ═══════════════════════════════════════════════════════════════
// drive_init
// ═══════════════════════════════════════════════════════════════
void drive_init(void)
{
    throttle = 0;
    steering = 0;
    printf("Drive: init complete\r\n");
    printf("Drive: W/S=throttle A/D=steer SPACE=stop X=disarm ?=status\r\n");
}

// ═══════════════════════════════════════════════════════════════
// drive_update
// ───────────────────────────────────────────────────────────────
// Call every main loop tick
// Reads UART command, updates throttle/steering, sends to ESC
// ═══════════════════════════════════════════════════════════════
void drive_update(void)
{
    // ── Only active in ARMED or MANUAL state ─────────────────
    if (auv_state != STATE_ARMED && auv_state != STATE_MANUAL)
        return;

    // ── Read UART command ────────────────────────────────────
    int8_t cmd = uart2_read_char();
    uint8_t command_changed = 0;

    if (cmd != -1)
    {
        switch (cmd)
        {
            case 'w': case 'W':
                throttle += DRIVE_THROTTLE_STEP;
                if (throttle > 100) throttle = 100;
                auv_state = STATE_MANUAL;
                command_changed = 1;
                break;

            case 's': case 'S':
                throttle -= DRIVE_THROTTLE_STEP;
                if (throttle < -100) throttle = -100;
                auv_state = STATE_MANUAL;
                command_changed = 1;
                break;

            case 'a': case 'A':
                steering -= DRIVE_STEERING_STEP;
                if (steering < -100) steering = -100;
                auv_state = STATE_MANUAL;
                command_changed = 1;
                break;

            case 'd': case 'D':
                steering += DRIVE_STEERING_STEP;
                if (steering > 100) steering = 100;
                auv_state = STATE_MANUAL;
                command_changed = 1;
                break;

            case ' ':   // SPACE — neutral stop
                throttle = 0;
                steering = 0;
                ESC_PWM_Failsafe();
                printf("Drive: STOP - neutral\r\n");
                return;

            case 'x': case 'X':   // disarm / failsafe
                throttle = 0;
                steering = 0;
                ESC_PWM_Failsafe();
                auv_state = STATE_SAFE;
                printf("Drive: DISARMED - STATE_SAFE\r\n");
                return;

            case '?':   // status
                drive_print_status();
                return;

            default:
                break;  // ignore unknown chars
        }
    }

    // ── Apply drive command only when a new command arrives ───
    if (command_changed && auv_state == STATE_MANUAL)
    {
        Drive_Set(throttle, steering);
    }
}

// ═══════════════════════════════════════════════════════════════
// Drive_Set
// ───────────────────────────────────────────────────────────────
// throttle: -100 to +100  forward/reverse
// steering: -100 to +100  left/right
// Port  = throttle - steering
// Stbd  = throttle + steering
// Both clamped and passed to ESC layer
// ═══════════════════════════════════════════════════════════════
void Drive_Set(int16_t thr, int16_t str)
{
    int16_t port_cmd = thr - str;
    int16_t stbd_cmd = thr + str;

    // clamp to safe range
    if (port_cmd >  100) port_cmd =  100;
    if (port_cmd < -100) port_cmd = -100;
    if (stbd_cmd >  100) stbd_cmd =  100;
    if (stbd_cmd < -100) stbd_cmd = -100;

#if FEATURE_TEST_LIMITS
    // Stage 5 bench/bucket test limits.
    // Clamp command values before sending to the ESC layer.
    // This keeps all PWM conversion and motor inversion inside esc_pwm.c.
    if (port_cmd > DRIVE_TEST_CMD_MAX) port_cmd = DRIVE_TEST_CMD_MAX;
    if (port_cmd < DRIVE_TEST_CMD_MIN) port_cmd = DRIVE_TEST_CMD_MIN;
    if (stbd_cmd > DRIVE_TEST_CMD_MAX) stbd_cmd = DRIVE_TEST_CMD_MAX;
    if (stbd_cmd < DRIVE_TEST_CMD_MIN) stbd_cmd = DRIVE_TEST_CMD_MIN;
#endif

    // Layer 0 owns PWM conversion, direct register writes, and motor inversion.
    ESC_PWM_Set_Port(port_cmd);
    ESC_PWM_Set_Stbd(stbd_cmd);

    printf("THR=%+4d STR=%+4d PORT_CMD=%+4d STBD_CMD=%+4d PORT=%4d STBD=%4d STATE=%d\r\n",
           thr,
           str,
           port_cmd,
           stbd_cmd,
           (int)TIM2->CCR1,
           (int)TIM2->CCR2,
           (int)auv_state);
}

// ═══════════════════════════════════════════════════════════════
// drive_print_status
// ═══════════════════════════════════════════════════════════════
void drive_print_status(void)
{
    printf("--- STATUS ---\r\n");
    printf("State:    %d\r\n",    (int)auv_state);
    printf("Throttle: %+d%%\r\n", (int)throttle);
    printf("Steering: %+d%%\r\n", (int)steering);
    printf("PORT PWM: %d us\r\n", (int)TIM2->CCR1);
    printf("STBD PWM: %d us\r\n", (int)TIM2->CCR2);
    printf("Leak:     %d\r\n",    (int)leak_fault);
    printf("Faults:   0x%08lX\r\n", (unsigned long)fault_active_mask);
    #if FEATURE_TEST_LIMITS
    printf("Limits:   ON cmd_min=%d cmd_max=%d\r\n",
           DRIVE_TEST_CMD_MIN, DRIVE_TEST_CMD_MAX);
    #else
    printf("Limits:   OFF - full command range\r\n");
    #endif
    printf("--------------\r\n");
}
