// ═══════════════════════════════════════════════════════════════
// safety.c
// STIGREYN AUV — Hardware Safety System
// Leak detection ISR, fault flags, watchdog stub
// ═══════════════════════════════════════════════════════════════
// FOCUSED RESPONSIBILITY:
// safety_init()          — EXTI interrupt setup for leak detector
// EXTI9_5_IRQHandler()   — leak ISR — hardware first, flag second
// safety_update()        — main loop fault management
// safety_watchdog_kick() — watchdog stub (disabled in dev)
//
// State machine lives in app.c — not here
// ═══════════════════════════════════════════════════════════════

#include "feature_flags.h"
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
// Purpose: configure leak detector EXTI interrupt on PA8 / D7
// Bare metal — SYSCFG, EXTI, NVIC direct register access
// Call:    after rcc_init() and gpio_init() in main.c
// ═══════════════════════════════════════════════════════════════
void safety_init(void)
{
#if FEATURE_LEAK_ISR
    // ── Step 1: SYSCFG clock on ───────────────────────────────
    // SYSCFG routes GPIO port to EXTI line.
    // Must enable before touching SYSCFG registers.
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

    // ── Step 2: connect PA8 → EXTI line 8 ────────────────────
    // EXTICR[2] controls EXTI lines 8-11.
    // EXTI8 = bits 3:0.
    // 0=PA 1=PB 2=PC 3=PD.
    // Port A = 0000, so clearing the bits selects PA8.
    SYSCFG->EXTICR[2] &= ~(0xF << 0);  // EXTI8 = PA8

    // ── Step 3: unmask EXTI8 ─────────────────────────────────
    EXTI->IMR |= (1 << LEAK_PA8);

    // ── Step 4: falling edge trigger ─────────────────────────
    // PA8/D7 pull-up → normally HIGH.
    // Water contact → pulled LOW → falling edge = leak detected.
    EXTI->FTSR |=  (1 << LEAK_PA8);  // falling edge on line 8
    EXTI->RTSR &= ~(1 << LEAK_PA8);  // no rising edge

    // ── Step 5: clear pending before enabling ─────────────────
    // Write-1-to-clear — STM32 specific register design.
    EXTI->PR = (1 << LEAK_PA8);

    // ── Step 6: NVIC — enable and set priority ────────────────
    // PA8 → EXTI line 8 → EXTI9_5 interrupt group.
    // Priority 2 — high but not highest.
    // Priority 0 reserved for future timing-critical interrupts.
    NVIC_SetPriority(EXTI9_5_IRQn, 2);
    NVIC_EnableIRQ(EXTI9_5_IRQn);

    printf("Safety: leak detector armed PA8/D7 EXTI8\r\n");
#else
    printf("WARNING: leak ISR disabled - feature_flags.h\r\n");
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
    // ISR already neutralised motors and latched leak_fault.
    // Main loop classifies and logs the fault.
    // Stage 4b validation target:
    // leak fault -> STATE_FAULT -> motors neutral -> fault latched.
    if (leak_fault && !(fault_active_mask & FAULT_BIT(FAULT_LEAK)))
    {
        printf("FAULT: leak detected - classified critical\r\n");

        // visual indicators
        GPIOB->ODR &= ~(1 << LED_ARMED_PB1);  // armed LED off
        GPIOB->ODR |=  (1 << LED_STATUS_PB0); // status LED on

        // Keep hardware safe. app_fault() also calls ESC_PWM_Failsafe().
        app_fault(FAULT_LEAK, SEVERITY_CRITICAL);

        // ── TODO: wake Pi ─────────────────────────────────────
        // Stage 7: GPIOB->ODR |= (1 << PI_SLEEP_PB4);
        // Ignore for now — continue
        // ─────────────────────────────────────────────────────
    }

    // ── Battery fault ─────────────────────────────────────────
    // Placeholder until PA0 battery ADC thresholds are implemented.
    // Low battery warning may be recoverable; critical battery may require surfacing.
    if (battery_fault && !(fault_active_mask & FAULT_BIT(FAULT_BATT_WARN)))
    {
        printf("FAULT: battery warning placeholder - assessing\r\n");
        app_fault(FAULT_BATT_WARN, SEVERITY_WARN);

        // ── TODO: staged battery response ─────────────────────
        // Stage 4b: WARN/CRIT/DEAD voltage thresholds
        // WARN  -> reduce power / notify Pi / monitor
        // CRIT  -> classify FAULT_BATT_CRIT + SEVERITY_CRITICAL
        // DEAD  -> immediate failsafe + recovery behaviour
        // ─────────────────────────────────────────────────────
    }

    // ── Fault state — keep motors neutral ─────────────────────
    if (auv_state == STATE_FAULT || auv_state == STATE_SURFACE)
    {
        TIM2->CCR1 = PWM_NEUTRAL_US;
        TIM2->CCR2 = PWM_NEUTRAL_US;
    }

    // ── Surface state — future recovery behaviour ─────────────
    if (auv_state == STATE_SURFACE)
    {
        // ── TODO: pinger ──────────────────────────────────────
        // Stage 8: pulse acoustic pinger at 1Hz
        // Only enter STATE_SURFACE after recovery policy commits to surfacing.
        // ─────────────────────────────────────────────────────
    }
}

// ═══════════════════════════════════════════════════════════════
// safety_watchdog_kick
// ───────────────────────────────────────────────────────────────
// Disabled in development — enable via FEATURE_WATCHDOG in feature_flags.h
// ═══════════════════════════════════════════════════════════════
void safety_watchdog_kick(void)
{
#if FEATURE_WATCHDOG
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
    // Set FEATURE_WATCHDOG = 1 in feature_flags.h to activate
    // ─────────────────────────────────────────────────────────
}

// ═══════════════════════════════════════════════════════════════
// EXTI9_5_IRQHandler — LEAK DETECTOR ISR
// ───────────────────────────────────────────────────────────────
// Fires: PA8 / D7 goes LOW — water contact on leak detector
// ISR rule: hardware safe → flags → clear interrupt → exit
// Nothing else — no printf, no HAL, no delays, no logic
// ═══════════════════════════════════════════════════════════════
void EXTI9_5_IRQHandler(void)
{
    if (EXTI->PR & (1 << LEAK_PA8))
    {
        // ── Hardware safe FIRST ───────────────────────────────
        // Direct register write — no function call overhead
        // Fastest possible path — microseconds to neutral
        TIM2->CCR1 = PWM_NEUTRAL_US;    // port → stop
        TIM2->CCR2 = PWM_NEUTRAL_US;    // stbd → stop

        // ── Latch fault — no recovery without operator reset ──
        leak_fault = 1;
        // Immediate state only. Classification happens in safety_update().
        auv_state  = STATE_FAULT;

        // ── Clear interrupt pending flag ──────────────────────
        // Write-1-to-clear — must clear or ISR fires again
        EXTI->PR = (1 << LEAK_PA8);
        // clear EXTI8 pending bit

        // ── EXIT ──────────────────────────────────────────────
        // main loop handles everything else
        // printf, LED, Pi wake — all in safety_update()
    }
}
