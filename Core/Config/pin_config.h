// ═══════════════════════════════════════════════════════════════
// pin_config.h
// STIGREYN AUV — PIN ASSIGNMENT MAP
// Board:  STM32F446RE Nucleo-F446RE + ElectroCookie Proto Shield v3.3
// Clock:  84MHz PLL
// Author: Daniel Reynolds — REYN Consultancy
// ═══════════════════════════════════════════════════════════════
// ALL PINS USE ARDUINO HEADER — compatible with proto shield
// POSITION RULES:
// ODR/IDR  bit position = pin x 1
// MODER    bit position = pin x 2
// PUPDR    bit position = pin x 2
// AFR      bit position = pin x 4  (AFR[0]=pins 0-7, AFR[1]=pins 8-15)
// ═══════════════════════════════════════════════════════════════

#ifndef PIN_CONFIG_H
#define PIN_CONFIG_H

// ── MODER MODE VALUES ───────────────────────────────────────────
#define MODER_MASK      3   // 0b11 — clear mask, always use first
#define MODER_INPUT     0   // 0b00 — input
#define MODER_OUTPUT    1   // 0b01 — output
#define MODER_AF        2   // 0b10 — alternate function
#define MODER_ANALOG    3   // 0b11 — analog / ADC

// ── PUPDR PULL VALUES ───────────────────────────────────────────
#define PUPDR_MASK      3   // 0b11 — clear mask
#define PUPDR_NONE      0   // 0b00 — no pull resistor
#define PUPDR_UP        1   // 0b01 — pull up
#define PUPDR_DOWN      2   // 0b10 — pull down

// ── POSITION MACROS ─────────────────────────────────────────────
#define MODER_POS(pin)  (pin * 2)   // 2 bits per pin
#define PUPDR_POS(pin)  (pin * 2)   // 2 bits per pin
#define AFR_POS(pin)    (pin * 4)   // 4 bits per pin

// ═══════════════════════════════════════════════════════════════
// PORT A — REGISTER BIT MAP
// bit: 31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
// pin: 15 15 14 14 13 13 12 12 11 11 10 10 09 09 08 08 07 07 06 06 05 05 04 04 03 03 02 02 01 01 00 00
//
// Arduino  Header  STM32   MODER     ASSIGNMENT
// ───────────────────────────────────────────────────────────────
// A0       CN8     PA0     1:0       battery voltage ADC1 CH0
// A1       CN8     PA1     3:2       TIM2 CH2 starboard ESC PWM   AF1
// D1       CN9     PA2     5:4       UART2 TX → Pi/Mac             AF7
// D0       CN9     PA3     7:6       UART2 RX ← Pi/Mac             AF7
// A2       CN8     PA4     9:8       — available
// D13      CN5     PA5     11:10     TIM2 CH1 port ESC PWM         AF1
// D12      CN5     PA6     13:12     — reserve SPI MISO / spare input
// D11      CN5     PA7     15:14     — reserve SPI MOSI / spare output
// D7       CN9     PA8     17:16     leak detector EXTI8 input     EXTI9_5
//          PA9     19:18     — available
//          PA10    21:20     — available
//          PA11    23:22     — available
//          PA12    25:24     — available
//          PA13    27:26     — reserved ST-LINK
//          PA14    29:28     — reserved ST-LINK
//          PA15    31:30     — reserved ST-LINK
// ═══════════════════════════════════════════════════════════════

#define BATT_ADC_PA0    0   // A0  — 1:0    ADC1 CH0  battery voltage
//  IDR READ:  0000 0000 0000 0000 0000 0000 0000 0001
// MODER CLR:  1111 1111 1111 1111 1111 1111 1111 1100
// MODER SET:  0000 0000 0000 0000 0000 0000 0000 0011  ← ANALOG(11)

#define MOT_STBD_PA1    1   // A1  — 3:2    AF1  TIM2 CH2 stbd ESC
//  ODR SET:   0000 0000 0000 0000 0000 0000 0000 0010
// MODER CLR:  1111 1111 1111 1111 1111 1111 1111 0011
// MODER SET:  0000 0000 0000 0000 0000 0000 0000 1000  ← AF(10)

#define UART_TX_PA2     2   // D1  — 5:4    AF7  UART2 TX → Pi/Mac
//  ODR SET:   0000 0000 0000 0000 0000 0000 0000 0100
// MODER CLR:  1111 1111 1111 1111 1111 1111 1100 1111
// MODER SET:  0000 0000 0000 0000 0000 0000 0010 0000  ← AF(10)

#define UART_RX_PA3     3   // D0  — 7:6    AF7  UART2 RX ← Pi/Mac
//  ODR SET:   0000 0000 0000 0000 0000 0000 0000 1000
// MODER CLR:  1111 1111 1111 1111 1111 1111 0011 1111
// MODER SET:  0000 0000 0000 0000 0000 0000 1000 0000  ← AF(10)

                            // A2  PA4  9:8  — available

#define MOT_PORT_PA5    5   // D13 — 11:10  AF1  TIM2 CH1 port ESC
//  ODR SET:   0000 0000 0000 0000 0000 0000 0010 0000
// MODER CLR:  1111 1111 1111 1111 1111 0011 1111 1111
// MODER SET:  0000 0000 0000 0000 0000 1000 0000 0000  ← AF(10)

                            // D12 PA6 13:12 — reserve SPI MISO / spare input
                            // D11 PA7 15:14 — reserve SPI MOSI / spare output

#define LEAK_PA8        8   // D7  — 17:16  EXTI8 leak detector input
//  IDR READ:  0000 0000 0000 0000 0000 0001 0000 0000  ← PA8 LOW = leak
// MODER CLR:  1111 1111 1111 1111 0000 1111 1111 1111
// MODER SET:  0000 0000 0000 0000 0000 0000 0000 0000  ← INPUT(00)
// PUPDR SET:  0000 0000 0000 0000 0000 0001 0000 0000  ← PULLUP(01)
// EXTI:       EXTI9_5_IRQn — ISR: EXTI9_5_IRQHandler()
// SYSCFG:     EXTICR[2] bits 3:0 = 0000 (Port A)

                            // PA9-PA12     — available
                            // PA13-PA15    — reserved ST-LINK

// ═══════════════════════════════════════════════════════════════
// PORT B — REGISTER BIT MAP
// bit: 31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
// pin: 15 15 14 14 13 13 12 12 11 11 10 10 09 09 08 08 07 07 06 06 05 05 04 04 03 03 02 02 01 01 00 00
//
// Arduino  Header  STM32   MODER     ASSIGNMENT
// ───────────────────────────────────────────────────────────────
// A3       CN8     PB0     1:0       status LED output
// —        —       PB1     3:2       ESC armed LED output
// —        —       PB2     5:4       — available
// —        —       PB3     7:6       — reserved ST-LINK SWO
// D5       CN9     PB4     9:8       Pi sleep/wake control output
// —        —       PB5     11:10     — available
// D10      CN5     PB6     13:12     — reserve SPI CS / spare output
// —        —       PB7     15:14     — available
// D15      CN5     PB8     17:16     I2C1 SCL IMU + Bar30          AF4
// D14      CN5     PB9     19:18     I2C1 SDA IMU + Bar30          AF4
// D6       CN9     PB10    21:20     — reserve spare PWM/UART3 TX  AF7
// —        —       PB11    23:22     — reserve spare UART3 RX      AF7
// —        —       PB12    25:24     — available / not Arduino SPI on this shield map
// —        —       PB13    27:26     — available / not Arduino D13 on this shield map
// —        —       PB14    29:28     — available / not Arduino D12 on this shield map
// —        —       PB15    31:30     — available / not Arduino D11 on this shield map
// ═══════════════════════════════════════════════════════════════

#define LED_STATUS_PB0  0   // A3  — 1:0    status LED output
//  ODR SET:   0000 0000 0000 0000 0000 0000 0000 0001
// MODER CLR:  1111 1111 1111 1111 1111 1111 1111 1100
// MODER SET:  0000 0000 0000 0000 0000 0000 0000 0001  ← OUTPUT(01)

#define LED_ARMED_PB1   1   // —   — 3:2    ESC armed LED output
//  ODR SET:   0000 0000 0000 0000 0000 0000 0000 0010
// MODER CLR:  1111 1111 1111 1111 1111 1111 1111 0011
// MODER SET:  0000 0000 0000 0000 0000 0000 0000 0100  ← OUTPUT(01)

                            // PB2  5:4     — available
                            // PB3  7:6     — reserved ST-LINK SWO

#define PI_SLEEP_PB4    4   // D5  — 9:8    Pi sleep/wake output
//  ODR SET:   0000 0000 0000 0000 0000 0000 0001 0000
// MODER CLR:  1111 1111 1111 1111 1111 1111 0000 1111
// MODER SET:  0000 0000 0000 0000 0000 0000 0001 0000  ← OUTPUT(01)

                            // PB5          — available
                            // D10 PB6      — reserve SPI CS / spare output
                            // PB7          — available

#define IMU_SCL_PB8     8   // D15 — 17:16  AF4  I2C1 SCL IMU+Bar30
//  ODR SET:   0000 0000 0000 0000 0000 0001 0000 0000
// MODER CLR:  1111 1111 1111 1111 0000 1111 1111 1111
// MODER SET:  0000 0000 0000 0000 0010 0000 0000 0000  ← AF(10)

#define IMU_SDA_PB9     9   // D14 — 19:18  AF4  I2C1 SDA IMU+Bar30
//  ODR SET:   0000 0000 0000 0000 0000 0010 0000 0000
// MODER CLR:  1111 1111 1111 1100 1111 1111 1111 1111
// MODER SET:  0000 0000 0000 0010 0000 0000 0000 0000  ← AF(10)

#define UART3_TX_PB10  10   // D6  — 21:20  AF7  spare UART3 TX
#define UART3_RX_PB11  11   //      — 23:22  AF7  spare UART3 RX

                            // PB12-PB15    — available / Morpho-only for this layout
                            // Arduino SPI-style reserves use:
                            // D10 PB6      — CS reserve
                            // D11 PA7      — MOSI reserve
                            // D12 PA6      — MISO reserve
                            // D13 PA5      — SCK label, currently port ESC PWM

// ═══════════════════════════════════════════════════════════════
// PORT C — REGISTER BIT MAP
// bit: 31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
// pin: 15 15 14 14 13 13 12 12 11 11 10 10 09 09 08 08 07 07 06 06 05 05 04 04 03 03 02 02 01 01 00 00
//
// Arduino  Header  STM32   MODER     ASSIGNMENT
// ───────────────────────────────────────────────────────────────
// A5       CN8     PC0     1:0       reed switch / magnetic arm input
// A4       CN8     PC1     3:2       ESP32-CAM anomaly trigger input
// —        —       PC2     5:4       — available
// —        —       PC3     7:6       — available
// —        —       PC4     9:8       — available
// —        —       PC5     11:10     — available
//          PC6-PC12          — available
//          PC13    27:26     — USER button (was leak, now moved to PA8)
//          PC14    29:28     — reserved oscillator
//          PC15    31:30     — reserved oscillator
// ═══════════════════════════════════════════════════════════════

#define REED_SW_PC0     0   // A5  — 1:0    reed switch input
//  IDR READ:  0000 0000 0000 0000 0000 0000 0000 0001
// MODER CLR:  1111 1111 1111 1111 1111 1111 1111 1100
// MODER SET:  0000 0000 0000 0000 0000 0000 0000 0000  ← INPUT(00)
// PUPDR SET:  0000 0000 0000 0000 0000 0000 0000 0100  ← PULLUP(01)

#define ESP32_TRIG_PC1  1   // A4  — 3:2    ESP32-CAM trigger input
//  IDR READ:  0000 0000 0000 0000 0000 0000 0000 0010
// MODER CLR:  1111 1111 1111 1111 1111 1111 1111 0011
// MODER SET:  0000 0000 0000 0000 0000 0000 0000 0000  ← INPUT(00)
// PUPDR SET:  0000 0000 0000 0000 0000 0000 0000 0100  ← PULLUP(01) wait
//             0000 0000 0000 0000 0000 0000 0000 1000  ← PULLUP(01) correct

                            // PC2-PC12     — available

                            // PC13         — USER button only
                            //                leak moved to PA8/D7
                            // PC14-PC15    — reserved oscillator

// ═══════════════════════════════════════════════════════════════
// ARDUINO HEADER QUICK REFERENCE — proto shield layout
// ───────────────────────────────────────────────────────────────
// D0   CN9  PA3   UART2 RX
// D1   CN9  PA2   UART2 TX
// D2   CN9  —     spare interrupt input (reserve)
// D3   CN9  —     spare interrupt / pulse (reserve)
// D4   CN9  —     spare digital output (reserve)
// D5   CN9  PB4   Pi sleep/wake
// D6   CN9  PB10  spare PWM / UART3 TX (reserve)
// D7   CN9  PA8   leak detector EXTI8
// D8   CN5  —     pinger output (reserve Stage 8)
// D9   CN5  —     spare PWM (reserve)
// D10  CN5  PB6   SPI CS reserve / spare output
// D11  CN5  PA7   SPI MOSI reserve / spare output
// D12  CN5  PA6   SPI MISO reserve / spare input
// D13  CN5  PA5   port ESC PWM TIM2 CH1 / Arduino SCK label
// D14  CN5  PB9   I2C1 SDA
// D15  CN5  PB8   I2C1 SCL
// A0   CN8  PA0   battery ADC
// A1   CN8  PA1   stbd ESC PWM TIM2 CH2
// A2   CN8  PA4   spare analog
// A3   CN8  PB0   status LED
// A4   CN8  PC1   ESP32-CAM trigger
// A5   CN8  PC0   reed switch
// ═══════════════════════════════════════════════════════════════

// NOTE:
// Arduino header group names are included for proto shield layout:
// CN8 = analog header A0-A5
// CN9 = digital header D0-D7
// CN5 = digital header D8-D15 / I2C/SPI area
// For soldering, verify the physical silkscreen on the Nucleo and proto shield.
// Some STM32 pins exist on Morpho headers only and are marked as "—" here.
// ═══════════════════════════════════════════════════════════════

// ═══════════════════════════════════════════════════════════════
// ALTERNATE FUNCTION QUICK REFERENCE
// ───────────────────────────────────────────────────────────────
// AF1  TIM2    MOT_PORT_PA5(D13)  MOT_STBD_PA1(A1)
// AF4  I2C1    IMU_SCL_PB8(D15)   IMU_SDA_PB9(D14)
// AF7  UART2   UART_TX_PA2(D1)    UART_RX_PA3(D0)
// AF7  UART3   UART3_TX_PB10(D6)  UART3_RX_PB11
//
// Arduino SPI header reserve note:
// D13 = PA5 and is currently used for port ESC PWM, despite the Arduino SCK label.
// D11/D12/D10 are reserved as PA7/PA6/PB6 for proto-shield planning only.
// Confirm final SPI peripheral choice before implementing SPI firmware.
//
// EXTI INTERRUPT REFERENCE
// LEAK_PA8(D7) → EXTI line 8 → EXTI9_5_IRQn
//   SYSCFG->EXTICR[2] bits 3:0 = 0000 (Port A)
//   ISR: void EXTI9_5_IRQHandler(void)
// ═══════════════════════════════════════════════════════════════

// ═══════════════════════════════════════════════════════════════
// T200 PWM OPERATING REGIONS — 4S BATTERY (12.0-16.8V)
// ───────────────────────────────────────────────────────────────
#define PWM_NEUTRAL_US   1500   // stop      — failsafe default
#define PWM_DEADBAND_LO  1460   // deadband low
#define PWM_DEADBAND_HI  1540   // deadband high
#define PWM_CRUISE_LO    1560   // cruise start
#define PWM_CRUISE_HI    1620   // cruise end
#define PWM_DIVE_LO      1620   // dive start
#define PWM_DIVE_HI      1680   // dive end
#define PWM_HIGH_PWR_HI  1800   // high power limit — short only
#define PWM_MAX_FWD      1900   // absolute maximum forward
#define PWM_MAX_REV      1100   // absolute maximum reverse
// ═══════════════════════════════════════════════════════════════

// ═══════════════════════════════════════════════════════════════
// TIMER CONFIGURATION — TIM2 PWM 50Hz at 84MHz PLL
// ───────────────────────────────────────────────────────────────
// APB1 /2 = 42MHz → timer doubling → TIM2 = 84MHz
// PSC=83 → 84MHz/(83+1) = 1MHz → 1 tick = 1µs
// ARR=19999 → 20000 ticks = 20ms = 50Hz
// CCR value = pulse width in microseconds directly
// ═══════════════════════════════════════════════════════════════
#define TIM_PRESCALER   83      // 84MHz PLL
#define TIM_PERIOD      19999   // 20ms = 50Hz
// ═══════════════════════════════════════════════════════════════

// ═══════════════════════════════════════════════════════════════
// MOTOR COMMAND LIMITS
// ───────────────────────────────────────────────────────────────
#define MOTOR_CMD_MAX    100    // full forward
#define MOTOR_CMD_MIN   -100    // full reverse
#define MOTOR_CMD_STOP    0     // neutral
// ═══════════════════════════════════════════════════════════════

#endif // PIN_CONFIG_H
// ═══════════════════════════════════════════════════════════════
// STIGREYN AUV — PIN QUICK REFERENCE
// STM32F446RE NUCLEO + Arduino Proto Shield
// Source of truth — update this table FIRST before firmware
// ═══════════════════════════════════════════════════════════════

/*
┌─────────┬──────┬────────┬─────────────────────────────────────┐
│ Arduino │ MCU  │ Header │ Function                            │
├─────────┼──────┼────────┼─────────────────────────────────────┤
│ D0      │ PA3  │ CN9    │ UART2_RX — debug RX / Pi RX         │
│ D1      │ PA2  │ CN9    │ UART2_TX — debug TX / Pi TX         │
│ D5      │ PB4  │ CN9    │ PI_SLEEP — Pi wake/sleep GPIO       │
│ D6      │ PB10 │ CN9    │ reserve UART3_TX / spare output     │
│ D7      │ PA8  │ CN9    │ LEAK detector — EXTI8 active LOW    │
│ D10     │ PB6  │ CN5    │ reserve SPI CS / spare output       │
│ D11     │ PA7  │ CN5    │ reserve SPI MOSI / spare output     │
│ D12     │ PA6  │ CN5    │ reserve SPI MISO / spare input      │
│ D13     │ PA5  │ CN5    │ MOT_PORT — ESC PWM TIM2_CH1         │
│         │      │        │ (Arduino SCK label reused)           │
├─────────┼──────┼────────┼─────────────────────────────────────┤
│ A0      │ PA0  │ CN8    │ BATT_ADC — battery voltage divider  │
│ A1      │ PA1  │ CN8    │ MOT_STBD — ESC PWM TIM2_CH2         │
│ A4      │ PC1  │ CN8    │ ESP32 trigger / spare GPIO          │
│ A5      │ PC0  │ CN8    │ REED switch input                   │
├─────────┼──────┼────────┼─────────────────────────────────────┤
│ D14 SDA │ PB9  │ CN5    │ I2C1 SDA — IMU / Bar30              │
│ D15 SCL │ PB8  │ CN5    │ I2C1 SCL — IMU / Bar30              │
└─────────┴──────┴────────┴─────────────────────────────────────┘
*/

// ── ACTIVE TIMERS / PERIPHERALS ───────────────────────────────
//
// TIM2 CH1 → PA5 / D13 → Port ESC PWM
// TIM2 CH2 → PA1 / A1  → Starboard ESC PWM
//
// USART2 TX → PA2 / D1
// USART2 RX → PA3 / D0
//
// EXTI8 → PA8 / D7 leak detector
//
// I2C1 SDA → PB9  / D14
// I2C1 SCL → PB8  / D15
//
// PLL SYSCLK = 84 MHz
// USART2->BRR = 0x016D for 115200 baud @ APB1 42MHz
//
// ═══════════════════════════════════════════════════════════════