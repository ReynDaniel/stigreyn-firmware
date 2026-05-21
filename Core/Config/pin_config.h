// ═══════════════════════════════════════════════════════════════
// pin_config.h
// STIGREYN AUV — PIN ASSIGNMENT MAP
// Board:  STM32F446RE Nucleo-F446RE
// Clock:  16MHz HSI (Stage 1-2) → 84MHz PLL (Stage 3+)
// Author: Daniel Reynolds — REYN Consultancy
// ═══════════════════════════════════════════════════════════════
// POSITION RULES:
// ODR/IDR  bit position = pin x 1  (1 bit  per pin)
// MODER    bit position = pin x 2  (2 bits per pin)
// PUPDR    bit position = pin x 2  (2 bits per pin)
// AFR      bit position = pin x 4  (4 bits per pin)
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
// CUBEMX USER LABELS — USE THESE EXACT SHORT NAMES
// ───────────────────────────────────────────────────────────────
// These labels are intentionally short and pin-specific.
// Format: FUNCTION_SIDE_PORTPIN where useful.
// Example: MOT_STBD_PA1 = motor, starboard side, physical pin PA1.
//
// PA0  → BATT_ADC_PA0      battery voltage ADC input
// PA1  → MOT_STBD_PA1      starboard motor / thruster, TIM2 CH2
// PA2  → UART_TX_PA2       UART2 TX to Raspberry Pi
// PA3  → UART_RX_PA3       UART2 RX from Raspberry Pi
// PA5  → MOT_PORT_PA5      port motor / thruster, TIM2 CH1
// PB0  → LED_STATUS_PB0    external status LED
// PB1  → LED_ARMED_PB1     ESC armed indicator LED
// PB4  → PI_SLEEP_PB4      Pi sleep/wake control
// PB8  → IMU_SCL_PB8       I2C1 SCL for IMU + Bar30
// PB9  → IMU_SDA_PB9       I2C1 SDA for IMU + Bar30
// PB10 → UART3_TX_PB10     spare UART3 TX
// PB11 → UART3_RX_PB11     spare UART3 RX
// PC0  → REED_SW_PC0       reed switch input
// PC1  → ESP32_TRIG_PC1    ESP32-CAM anomaly trigger
// PC13 → LEAK_PC13         leak detector input
// ═══════════════════════════════════════════════════════════════

// ═══════════════════════════════════════════════════════════════
// PORT A — REGISTER BIT MAP
// bit: 31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
// pin: 15 15 14 14 13 13 12 12 11 11 10 10 09 09 08 08 07 07 06 06 05 05 04 04 03 03 02 02 01 01 00 00
//
// PIN   ODR    MODER     ASSIGNMENT
// ───────────────────────────────────────────────────────────────
//  0    bit0   1:0       battery voltage            ADC1 CH0
//  1    bit1   3:2       TIM2 CH2 stbd thruster     AF1
//  2    bit2   5:4       UART2 TX → Pi              AF7
//  3    bit3   7:6       UART2 RX ← Pi              AF7
//  4    bit4   9:8       — available
//  5    bit5   11:10     TIM2 CH1 port thruster      AF1
//  6    bit6   13:12     — available (was wrong TIM2 assignment)
//  7    bit7   15:14     — available
//  8    bit8   17:16     — available
//  9    bit9   19:18     — available
//  10   bit10  21:20     — available
//  11   bit11  23:22     — available
//  12   bit12  25:24     — available
//  13   bit13  27:26     — reserved ST-LINK
//  14   bit14  29:28     — reserved ST-LINK
//  15   bit15  31:30     — reserved ST-LINK
// ═══════════════════════════════════════════════════════════════

#define BATT_ADC_PA0    0   // 1:0    ADC1 CH0  battery voltage monitor
//  IDR READ:  0000 0000 0000 0000 0000 0000 0000 0001  ← PA0 analog
// MODER CLR:  1111 1111 1111 1111 1111 1111 1111 1100  ← bits 1:0 clear
// MODER SET:  0000 0000 0000 0000 0000 0000 0000 0011  ← bits 1:0 = ANALOG(11)

#define MOT_STBD_PA1    1   // 3:2    AF1  TIM2 CH2 starboard thruster
//  ODR SET:   0000 0000 0000 0000 0000 0000 0000 0010  ← PA1 HIGH
// MODER CLR:  1111 1111 1111 1111 1111 1111 1111 0011  ← bits 3:2 clear
// MODER SET:  0000 0000 0000 0000 0000 0000 0000 1000  ← bits 3:2 = AF(10)

#define UART_TX_PA2     2   // 5:4    AF7  UART2 TX → Pi
//  ODR SET:   0000 0000 0000 0000 0000 0000 0000 0100  ← PA2 HIGH
// MODER CLR:  1111 1111 1111 1111 1111 1111 1100 1111  ← bits 5:4 clear
// MODER SET:  0000 0000 0000 0000 0000 0000 0010 0000  ← bits 5:4 = AF(10)

#define UART_RX_PA3     3   // 7:6    AF7  UART2 RX ← Pi
//  ODR SET:   0000 0000 0000 0000 0000 0000 0000 1000  ← PA3 HIGH
// MODER CLR:  1111 1111 1111 1111 1111 1111 0011 1111  ← bits 7:6 clear
// MODER SET:  0000 0000 0000 0000 0000 0000 1000 0000  ← bits 7:6 = AF(10)

                            // 9:8    PA4  — available

#define MOT_PORT_PA5    5   // 11:10  AF1  TIM2 CH1 port thruster
//  ODR SET:   0000 0000 0000 0000 0000 0000 0010 0000  ← PA5 HIGH
// MODER CLR:  1111 1111 1111 1111 1111 0011 1111 1111  ← bits 11:10 clear
// MODER SET:  0000 0000 0000 0000 0000 1000 0000 0000  ← bits 11:10 = AF(10)

                            // PA6    — available (NOT TIM2 capable)
                            // PA7-PA12   — available
                            // PA13-PA15  — reserved ST-LINK

// ═══════════════════════════════════════════════════════════════
// PORT B — REGISTER BIT MAP
// bit: 31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
// pin: 15 15 14 14 13 13 12 12 11 11 10 10 09 09 08 08 07 07 06 06 05 05 04 04 03 03 02 02 01 01 00 00
//
// PIN   ODR    MODER     ASSIGNMENT
// ───────────────────────────────────────────────────────────────
//  0    bit0   1:0       status LED                 digital out
//  1    bit1   3:2       ESC armed LED              digital out
//  2    bit2   5:4       — available
//  3    bit3   7:6       — reserved ST-LINK SWO
//  4    bit4   9:8       Pi sleep/wake control      digital out
//  5    bit5   11:10     — available
//  6    bit6   13:12     — available
//  7    bit7   15:14     — available
//  8    bit8   17:16     I2C1 SCL IMU + Bar30       AF4
//  9    bit9   19:18     I2C1 SDA IMU + Bar30       AF4
//  10   bit10  21:20     UART3 TX spare/expansion   AF7
//  11   bit11  23:22     UART3 RX spare/expansion   AF7
//  12   bit12  25:24     — available
//  13   bit13  27:26     — available
//  14   bit14  29:28     — available
//  15   bit15  31:30     — available
// ═══════════════════════════════════════════════════════════════

#define LED_STATUS_PB0  0   // 1:0        status LED output
//  ODR SET:   0000 0000 0000 0000 0000 0000 0000 0001  ← PB0 HIGH = on
// MODER CLR:  1111 1111 1111 1111 1111 1111 1111 1100  ← bits 1:0 clear
// MODER SET:  0000 0000 0000 0000 0000 0000 0000 0001  ← bits 1:0 = OUTPUT(01)

#define LED_ARMED_PB1   1   // 3:2        ESC armed LED output
//  ODR SET:   0000 0000 0000 0000 0000 0000 0000 0010  ← PB1 HIGH = armed
// MODER CLR:  1111 1111 1111 1111 1111 1111 1111 0011  ← bits 3:2 clear
// MODER SET:  0000 0000 0000 0000 0000 0000 0000 0100  ← bits 3:2 = OUTPUT(01)

                            // PB2        — available
                            // PB3        — reserved ST-LINK SWO

#define PI_SLEEP_PB4    4   // 9:8        Pi sleep/wake control output
//  ODR SET:   0000 0000 0000 0000 0000 0000 0001 0000  ← PB4 HIGH = Pi wake
//  ODR CLR:   0000 0000 0000 0000 0000 0000 0000 0000  ← PB4 LOW  = Pi sleep
// MODER CLR:  1111 1111 1111 1111 1111 1111 0000 1111  ← bits 9:8 clear
// MODER SET:  0000 0000 0000 0000 0000 0000 0001 0000  ← bits 9:8 = OUTPUT(01)

                            // PB5-PB7    — available

#define IMU_SCL_PB8     8   // 17:16  AF4  I2C1 SCL — IMU + Bar30
//  ODR SET:   0000 0000 0000 0000 0000 0001 0000 0000  ← PB8 HIGH
// MODER CLR:  1111 1111 1111 1111 0000 1111 1111 1111  ← bits 17:16 clear
// MODER SET:  0000 0000 0000 0000 0010 0000 0000 0000  ← bits 17:16 = AF(10)

#define IMU_SDA_PB9     9   // 19:18  AF4  I2C1 SDA — IMU + Bar30
//  ODR SET:   0000 0000 0000 0000 0000 0010 0000 0000  ← PB9 HIGH
// MODER CLR:  1111 1111 1111 1100 1111 1111 1111 1111  ← bits 19:18 clear
// MODER SET:  0000 0000 0000 0010 0000 0000 0000 0000  ← bits 19:18 = AF(10)

#define UART3_TX_PB10  10   // 21:20  AF7  UART3 TX — spare/expansion
//  ODR SET:   0000 0000 0000 0000 0000 0100 0000 0000  ← PB10 HIGH
// MODER CLR:  1111 1111 1111 1111 0011 1111 1111 1111  ← bits 21:20 clear
// MODER SET:  0000 0000 0000 0000 1000 0000 0000 0000  ← bits 21:20 = AF(10)

#define UART3_RX_PB11  11   // 23:22  AF7  UART3 RX — spare/expansion
//  ODR SET:   0000 0000 0000 0000 0000 1000 0000 0000  ← PB11 HIGH
// MODER CLR:  1111 1111 1111 0011 1111 1111 1111 1111  ← bits 23:22 clear
// MODER SET:  0000 0000 0000 1000 0000 0000 0000 0000  ← bits 23:22 = AF(10)

                            // PB12-PB15  — available

// ═══════════════════════════════════════════════════════════════
// PORT C — REGISTER BIT MAP
// bit: 31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
// pin: 15 15 14 14 13 13 12 12 11 11 10 10 09 09 08 08 07 07 06 06 05 05 04 04 03 03 02 02 01 01 00 00
//
// PIN   ODR    MODER     ASSIGNMENT
// ───────────────────────────────────────────────────────────────
//  0    bit0   1:0       reed switch input           digital in
//  1    bit1   3:2       ESP32-CAM anomaly trigger   digital in
//  2    bit2   5:4       — available
//  3    bit3   7:6       — available
//  4    bit4   9:8       — available
//  5    bit5   11:10     — available
//  6    bit6   13:12     — available
//  7    bit7   15:14     — available
//  8    bit8   17:16     — available
//  9    bit9   19:18     — available
//  10   bit10  21:20     — available
//  11   bit11  23:22     — available
//  12   bit12  25:24     — available
//  13   bit13  27:26     leak detector / USER button digital in
//  14   bit14  29:28     — reserved oscillator
//  15   bit15  31:30     — reserved oscillator
// ═══════════════════════════════════════════════════════════════

#define REED_SW_PC0     0   // 1:0        reed switch — wake + mode cycle
//  IDR READ:  0000 0000 0000 0000 0000 0000 0000 0001  ← PC0 HIGH = triggered
// MODER CLR:  1111 1111 1111 1111 1111 1111 1111 1100  ← bits 1:0 clear
// MODER SET:  0000 0000 0000 0000 0000 0000 0000 0000  ← bits 1:0 = INPUT(00)
// PUPDR SET:  0000 0000 0000 0000 0000 0000 0000 0100  ← bits 1:0 = pullup(01)

#define ESP32_TRIG_PC1  1   // 3:2        ESP32-CAM anomaly trigger
//  IDR READ:  0000 0000 0000 0000 0000 0000 0000 0010  ← PC1 HIGH = anomaly
// MODER CLR:  1111 1111 1111 1111 1111 1111 1111 0011  ← bits 3:2 clear
// MODER SET:  0000 0000 0000 0000 0000 0000 0000 0000  ← bits 3:2 = INPUT(00)
// PUPDR SET:  0000 0000 0000 0000 0000 0000 0000 0100  ← bits 3:2 = pullup(01)

                            // PC2-PC12   — available

#define LEAK_PC13      13   // 27:26       leak detector input
//  IDR READ:  0000 0000 0000 0010 0000 0000 0000 0000  ← PC13 HIGH = leak
// MODER CLR:  1111 1111 0011 1111 1111 1111 1111 1111  ← bits 27:26 clear
// MODER SET:  0000 0000 0000 0000 0000 0000 0000 0000  ← bits 27:26 = INPUT(00)
// PUPDR SET:  0000 0000 0100 0000 0000 0000 0000 0000  ← bits 27:26 = pullup(01)

                            // PC14-PC15  — reserved oscillator

// ═══════════════════════════════════════════════════════════════
// AVAILABLE PINS — FREE FOR FUTURE EXPANSION
// ───────────────────────────────────────────────────────────────
// PORT A:  PA4, PA6, PA7, PA8, PA9, PA10, PA11, PA12   (8 pins)
// PORT B:  PB2, PB5, PB6, PB7, PB12, PB13, PB14, PB15 (8 pins)
// PORT C:  PC2-PC12                                    (11 pins)
// TOTAL:   27 pins available
// ═══════════════════════════════════════════════════════════════

// ═══════════════════════════════════════════════════════════════
// ALTERNATE FUNCTION QUICK REFERENCE
// ───────────────────────────────────────────────────────────────
// AF1  TIM2    MOT_PORT_PA5   MOT_STBD_PA1
// AF4  I2C1    IMU_SCL_PB8    IMU_SDA_PB9
// AF7  UART2   UART_TX_PA2    UART_RX_PA3
// AF7  UART3   UART3_TX_PB10  UART3_RX_PB11
//
// TIM2 CHANNEL MAP — CONFIRMED BY HARDWARE TEST:
// TIM2_CH1 = PA0, PA5, PA15  (AF1) — using PA5
// TIM2_CH2 = PA1, PB3        (AF1) — using PA1
// NOTE: PA6 does NOT support TIM2 — common misconception
// ═══════════════════════════════════════════════════════════════

// ═══════════════════════════════════════════════════════════════
// T200 PWM OPERATING REGIONS — 4S BATTERY (12.0-16.8V)
// ───────────────────────────────────────────────────────────────
#define PWM_NEUTRAL      1500   // stop      — failsafe default
#define PWM_DEADBAND_LO  1460   // deadband low
#define PWM_DEADBAND_HI  1540   // deadband high
#define PWM_CRUISE_LO    1560   // cruise start
#define PWM_CRUISE_HI    1620   // cruise end
#define PWM_DIVE_LO      1620   // dive start
#define PWM_DIVE_HI      1680   // dive end
#define PWM_HIGH_PWR_HI  1800   // high power limit
#define PWM_MAX_FWD      1900   // absolute maximum forward
#define PWM_MAX_REV      1100   // absolute maximum reverse
// ═══════════════════════════════════════════════════════════════

// ═══════════════════════════════════════════════════════════════
// TIMER CONFIGURATION — TIM2 PWM 50Hz
// ───────────────────────────────────────────────────────────────
// Stage 1-2: HSI 16MHz clock
//   APB1 prescaler = /1 → NO doubling
//   TIM2 clock = 16MHz
//   PSC = 15  → 16MHz / (15+1) = 1MHz
//   ARR = 19999 → 20000 ticks = 20ms = 50Hz
//
// Stage 3+: PLL 84MHz clock
//   APB1 prescaler = /2 → doubling applies
//   TIM2 clock = 84MHz
//   PSC = 83  → 84MHz / (83+1) = 1MHz
//   ARR = 19999 → unchanged
//
// CCR value = pulse width in microseconds directly
// ═══════════════════════════════════════════════════════════════
#define TIM_PRESCALER_HSI   15      // 16MHz HSI — Stage 1-2
#define TIM_PRESCALER_PLL   83      // 84MHz PLL — Stage 3+
#define TIM_PERIOD          19999   // 20ms = 50Hz (both clock configs)
// ═══════════════════════════════════════════════════════════════

// ═══════════════════════════════════════════════════════════════
// SYSTEM WAKE / SLEEP SEQUENCE
// ───────────────────────────────────────────────────────────────
// WAKE:
// 1. Reed switch  → PC0 interrupt → STM32 wakes
// 2. STM32        → PB4 HIGH      → Pi wakes
// 3. Pi           → WiFi/BLE on   → config available
// 4. Pi           → pre-launch check via UART2
// 5. Pi           → GO/NO-GO      → STM32 begins mission
//
// DURING MISSION:
// 6. ESP32-CAM    → PC1 interrupt → STM32 wakes Pi
// 7. Pi cam       → AI detection  → result via UART2
// 8. Pi           → PB4 LOW       → Pi sleeps
//
// FAILSAFE:
// 9. Leak/battery → PWM neutral 1500µs → passive ascent
// ═══════════════════════════════════════════════════════════════

#endif // PIN_CONFIG_H
