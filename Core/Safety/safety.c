// ═══════════════════════════════════════════════════════════════
// safety.c
// STIGREYN AUV — Hardware Safety System
// Leak detection ISR, fault flags, watchdog stub
// ═══════════════════════════════════════════════════════════════
// FOCUSED RESPONSIBILITY:
// safety_init()          — EXTI interrupt setup for leak detector
// EXTI15_10_IRQHandler() — leak ISR — hardware first, flag second
// safety_update()        — main loop fault management
// safety_watchdog_kick() — watchdog stub (disabled in dev)
//
// State machine lives in app.c — not here
// ═══════════════════════════════════════════════════════════════

#include "safety.h"
#include "app.h"
#include "esc_pwm.h"
#include "main.h"
#include <stdio.h>

// ── FAULT FLAG DEFINITIONS ──────────────────────────────────────
// Declared extern in app.h — defined once here
// volatile — written by ISR, read by main loop
// latched  — once set, only app_reset() clears
volatile uint8_t leak_fault    = 0;
volatile uint8_t battery_fault = 0;
volatile uint8_t watchdog_fault = 0;

// ═══════════════════════════════════════════════════════════════
// safety_init
// ───────────────────────────────────────────────────────────────
// Purpose: configure leak detector EXTI interrupt on PC13
// Bare metal — SYSCFG, EXTI, NVIC direct register access
// Call:    after rcc_init() and gpio_init() in main.c
// ═══════════════════════════════════════════════════════════════
void safety_init(void)
{
    // ── Step 1: SYSCFG clock on ───────────────────────────────
    // SYSCFG routes GPIO port to EXTI line
    // Must enable before touching SYSCFG registers
    RCC->APB2ENR |= (1 << 14);
    //               0100 0000 0000 0000 — SYSCFG clock on

    // ── Step 2: connect PC13 → EXTI line 13 ──────────────────
    // EXTICR[3] controls lines 12-15
    // EXTI13 = bits 7:4
    // 0=PA 1=PB 2=PC 3=PD
    SYSCFG->EXTICR[3] &= ~(0xF << 4);  // clear bits 7:4
    SYSCFG->EXTICR[3] |=  (2   << 4);  // 0010 = Port C
    //                      ↑
    //                  PC13 selected

    // ── Step 3: unmask EXTI13 ────────────────────────────────
    EXTI->IMR |= (1 << 13);
    //            0010 0000 0000 0000 — line 13 unmasked

    // ── Step 4: falling edge trigger ─────────────────────────
    // PC13 pull-up → normally HIGH
    // Water contact → pulled LOW → falling edge = leak detected
    EXTI->FTSR |= (1 << 13);   // falling edge on line 13
    EXTI->RTSR &= ~(1 << 13);  // no rising edge

    // ── Step 5: clear pending before enabling ─────────────────
    // Write-1-to-clear — STM32 specific register design
    EXTI->PR = (1 << 13);
    //          0010 0000 0000 0000 — clear pending bit 13

    // ── Step 6: NVIC — enable and set priority ────────────────
    // PC13 → EXTI line 13 → EXTI15_10 interrupt group
    // Priority 2 — high but not highest
    // Priority 0 reserved for future timing-critical interrupts
    NVIC_SetPriority(EXTI15_10_IRQn, 2);
    NVIC_EnableIRQ(EXTI15_10_IRQn);

    printf("Safety: leak detector armed PC13 EXTI13\r\n");

    // ── WATCHDOG STATUS MESSAGE ───────────────────────────────
#ifdef WATCHDOG_ENABLED
    printf("Safety: watchdog ENABLED - %dms timeout\r\n",
           WATCHDOG_TIMEOUT_MS);
#else
    printf("Safety: watchdog DISABLED - development mode\r\n");
    printf("Safety: define WATCHDOG_ENABLED to activate\r\n");
#endif
}

// ═══════════════════════════════════════════════════════════════
// safety_update
// ───────────────────────────────────────────────────────────────
// Purpose: check fault flags, manage state transitions
// Call:    every main loop tick via app_update()
// ═══════════════════════════════════════════════════════════════
void safety_update(void)
{
    // ── Leak fault ────────────────────────────────────────────
    // Motors already neutralised by ISR
    // Here: log, update LEDs, transition to SURFACE
    if (leak_fault && auv_state != STATE_SURFACE)
    {
        printf("FAULT: leak detected - passive ascent\r\n");

        // visual indicators
        GPIOB->ODR &= ~(1 << LED_ARMED_PB1);  // armed LED off
        GPIOB->ODR |=  (1 << LED_STATUS_PB0); // status LED on

        // ── TODO: wake Pi ─────────────────────────────────────
        // Stage 7: GPIOB->ODR |= (1 << PI_SLEEP_PB4);
        // Ignore for now — continue
        // ─────────────────────────────────────────────────────

        auv_state = STATE_SURFACE;
    }

    // ── Battery fault ─────────────────────────────────────────
    if (battery_fault && auv_state != STATE_FAULT
                      && auv_state != STATE_SURFACE)
    {
        printf("FAULT: low battery - safe ascent\r\n");
        ESC_PWM_Failsafe();
        auv_state = STATE_SURFACE;

        // ── TODO: staged battery response ─────────────────────
        // Stage 4: WARN/CRIT/DEAD voltage thresholds
        // Ignore for now — continue
        // ─────────────────────────────────────────────────────
    }

    // ── Surface state — keep motors neutral ───────────────────
    if (auv_state == STATE_SURFACE)
    {
        TIM2->CCR1 = PWM_NEUTRAL_US;
        TIM2->CCR2 = PWM_NEUTRAL_US;

        // ── TODO: pinger ──────────────────────────────────────
        // Stage 8: pulse acoustic pinger at 1Hz
        // Ignore for now — continue
        // ─────────────────────────────────────────────────────
    }
}

// ═══════════════════════════════════════════════════════════════
// safety_watchdog_kick
// ───────────────────────────────────────────────────────────────
// Disabled in development — enable via #define WATCHDOG_ENABLED
// ═══════════════════════════════════════════════════════════════
void safety_watchdog_kick(void)
{
#ifdef WATCHDOG_ENABLED
    IWDG->KR = 0xAAAA;  // kick — reset watchdog counter
#endif

    // ── TODO: WATCHDOG INIT (when ready to enable) ────────────
    // In safety_init() add:
    // IWDG->KR  = 0x5555;  // unlock
    // IWDG->PR  = 4;       // prescaler /64
    // IWDG->RLR = 500;     // 1 second timeout at 32kHz/64
    // IWDG->KR  = 0xCCCC;  // start — cannot be stopped after this
    //
    // WARNING: once IWDG started it cannot be stopped without reset
    // Only enable when main loop is stable and tested
    // Define WATCHDOG_ENABLED in safety.h to activate
    // ─────────────────────────────────────────────────────────
}

// ═══════════════════════════════════════════════════════════════
// EXTI15_10_IRQHandler — LEAK DETECTOR ISR
// ───────────────────────────────────────────────────────────────
// Fires: PC13 goes LOW — water contact on leak detector
// ISR rule: hardware safe → flags → clear interrupt → exit
// Nothing else — no printf, no HAL, no delays, no logic
// ═══════════════════════════════════════════════════════════════
void EXTI15_10_IRQHandler(void)
{
    if (EXTI->PR & (1 << 13))
    {
        // ── Hardware safe FIRST ───────────────────────────────
        // Direct register write — no function call overhead
        // Fastest possible path — microseconds to neutral
        TIM2->CCR1 = PWM_NEUTRAL_US;    // port → stop
        TIM2->CCR2 = PWM_NEUTRAL_US;    // stbd → stop

        // ── Latch fault — no recovery without operator reset ──
        leak_fault = 1;
        auv_state  = STATE_FAULT;

        // ── Clear interrupt pending flag ──────────────────────
        // Write-1-to-clear — must clear or ISR fires again
        EXTI->PR = (1 << 13);
        //          0010 0000 0000 0000 — clear bit 13

        // ── EXIT ──────────────────────────────────────────────
        // main loop handles everything else
        // printf, LED, Pi wake — all in safety_update()
    }
}
