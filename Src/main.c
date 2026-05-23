/* USER CODE BEGIN Header */
// ═══════════════════════════════════════════════════════════════
// main.c
// STIGREYN AUV — System Entry Point
// Board:  STM32F446RE Nucleo-F446RE
// Author: Daniel Reynolds — REYN Consultancy
// ═══════════════════════════════════════════════════════════════
// VERSION HISTORY:
// v1.0.0 2026-06-01 — blink LED, toolchain verified
// v1.0.1 2026-06-20 — TIM2 PWM neutral on PA5 + PA1
// v1.0.2 2026-06-21 — bare metal GPIO, ESC driver integrated
// v1.0.3 2026-06-22 — PLL 84MHz, UART debug, bare metal init
// ═══════════════════════════════════════════════════════════════
// SYSTEM STARTUP SEQUENCE:
// 1. HAL_Init()           — SysTick only (replaced Stage 3)
// 2. SystemClock_Config() — PLL 84MHz — keep HAL (complex)
// 3. rcc_init()           — bare metal peripheral clock enables
// 4. gpio_init()          — bare metal pin MODER + PUPDR
// 5. uart2_init()         — bare metal UART2 115200 8N1
// 6. tim2_pwm_gpio_init() — bare metal PA1/PA5 AF1 TIM2
// 7. tim2_pwm_mode_init() — bare metal TIM2 PWM CH1+CH2
// 8. ESC_PWM_Init()       — enable outputs, set neutral
// 9. while(1)             — main loop, heartbeat LED
// ═══════════════════════════════════════════════════════════════
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "pin_config.h"
#include "feature_flags.h"
#include "esc_pwm.h"
#include "app.h"
#include "safety.h"
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);

/* USER CODE BEGIN PFP */
static void rcc_init(void);
static void gpio_init(void);
static void uart2_init(void);
static void tim2_pwm_gpio_init(void);
static void tim2_pwm_mode_init(void);
static void uart2_write(uint8_t *data, uint16_t len);
int _write(int file, char *ptr, int len);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/* USER CODE END 0 */

// ═══════════════════════════════════════════════════════════════
// main
// ═══════════════════════════════════════════════════════════════
int main(void)
{
    /* USER CODE BEGIN 1 */
    /* USER CODE END 1 */

    // ── Keep HAL_Init — sets up SysTick for HAL_Delay ────────
    // ── TODO Stage 3: replace HAL_Delay with timer tick ──────
    HAL_Init();

    // ── Keep SystemClock_Config — PLL is complex ─────────────
    // 20+ registers, CubeMX generates correctly
    // PLLM=8 PLLN=168 PLLP=4 → 84MHz SYSCLK
    SystemClock_Config();

    /* USER CODE BEGIN 2 */

    // ── Bare metal from here ──────────────────────────────────
    rcc_init();           // enable all peripheral clocks
    gpio_init();          // configure all GPIO pins
    uart2_init();         // UART2 115200 8N1 bare metal
    tim2_pwm_gpio_init(); // PA1/PA5 → AF1 TIM2
    tim2_pwm_mode_init(); // TIM2 PWM mode CH1+CH2
    ESC_PWM_Init();       // enable outputs, set 1500µs neutral

#if FEATURE_LEAK_ISR
    safety_init();
#else
    printf("WARNING: leak ISR disabled - feature_flags.h\r\n");
#endif

    app_init();

    // ── Startup log ───────────────────────────────────────────
    printf("StigReyn AUV v1.0 starting...\r\n");
    printf("Clock: PLL 84MHz\r\n");
    printf("ESC PWM init complete - PA5 + PA1 neutral\r\n");
    printf("System ready - STATE_SAFE\r\n");

    /* USER CODE END 2 */

    /* USER CODE BEGIN WHILE */
    while (1)
    {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

        app_update();
        HAL_Delay(10);   // 100Hz tick

#if FEATURE_HEARTBEAT_LED
        static uint32_t heartbeat_tick = 0;
        heartbeat_tick++;

        if (heartbeat_tick >= 50)   // 50 x 10ms = 500ms
        {
            heartbeat_tick = 0;
            GPIOB->ODR ^= (1 << LED_STATUS_PB0);
        }
#endif

        // ── TODO: MAIN CONTROL LOOP ───────────────────────────
        // Stage 4: safety checks — leak, battery, watchdog
        // Stage 5: sensor reads — IMU, Bar30
        // Stage 6: Drive_Set() — throttle + turn
        // Stage 7: UART RX — commands from Pi
        // ─────────────────────────────────────────────────────
    }
    /* USER CODE END 3 */
}

/* USER CODE BEGIN 4 */

// ═══════════════════════════════════════════════════════════════
// rcc_init
// ───────────────────────────────────────────────────────────────
// Purpose: enable clocks for all peripherals we use
// Must run before any peripheral register is touched
// ═══════════════════════════════════════════════════════════════
static void rcc_init(void)
{
    // ── AHB1 — GPIO clocks ───────────────────────────────────
    RCC->AHB1ENR |= (1 << 0);   // 0000 0001 — GPIOA clock on
    RCC->AHB1ENR |= (1 << 1);   // 0000 0010 — GPIOB clock on
    RCC->AHB1ENR |= (1 << 2);   // 0000 0100 — GPIOC clock on

    // ── APB1 — TIM2, UART2 clocks ────────────────────────────
    RCC->APB1ENR |= (1 << 0);   // 0000 0001 — TIM2  clock on
    RCC->APB1ENR |= (1 << 17);  // 0002 0000 — UART2 clock on

    // ── APB2 — ADC1 clock ────────────────────────────────────
    RCC->APB2ENR |= (1 << 8);   // 0000 0001 0000 0000 — ADC1 clock on

    // ── TODO: APB1 — I2C1 clock ──────────────────────────────
    // Stage 5: RCC->APB1ENR |= (1 << 21); // I2C1 clock on
    // Ignore for now — continue
    // ─────────────────────────────────────────────────────────
}

// ═══════════════════════════════════════════════════════════════
// gpio_init
// ───────────────────────────────────────────────────────────────
// Purpose: configure all GPIO pins bare metal
// RCC clocks must be enabled first via rcc_init()
// ═══════════════════════════════════════════════════════════════
static void gpio_init(void)
{
    // ── PB0 — LED_STATUS — output ────────────────────────────
    // pin 0 x 2 = bits 1:0 in MODER
    GPIOB->MODER &= ~(MODER_MASK   << MODER_POS(LED_STATUS_PB0));
    //               1111 1100  ← bits 1:0 clear
    GPIOB->MODER |=  (MODER_OUTPUT << MODER_POS(LED_STATUS_PB0));
    //               0000 0001  ← bits 1:0 = OUTPUT(01)
    GPIOB->ODR   &= ~(1 << LED_STATUS_PB0);
    //               1111 1110  ← PB0 LOW = LED off at startup

    // ── PB1 — LED_ARMED — output ─────────────────────────────
    // pin 1 x 2 = bits 3:2 in MODER
    GPIOB->MODER &= ~(MODER_MASK   << MODER_POS(LED_ARMED_PB1));
    //               1111 0011  ← bits 3:2 clear
    GPIOB->MODER |=  (MODER_OUTPUT << MODER_POS(LED_ARMED_PB1));
    //               0000 0100  ← bits 3:2 = OUTPUT(01)
    GPIOB->ODR   &= ~(1 << LED_ARMED_PB1);
    //               1111 1101  ← PB1 LOW = LED off at startup

    // ── PB4 — PI_SLEEP — output ──────────────────────────────
    // pin 4 x 2 = bits 9:8 in MODER
    GPIOB->MODER &= ~(MODER_MASK   << MODER_POS(PI_SLEEP_PB4));
    //               1111 1111 0000 1111  ← bits 9:8 clear
    GPIOB->MODER |=  (MODER_OUTPUT << MODER_POS(PI_SLEEP_PB4));
    //               0000 0000 0001 0000  ← bits 9:8 = OUTPUT(01)
    GPIOB->ODR   &= ~(1 << PI_SLEEP_PB4);
    //               0000 0000 1110 1111  ← PB4 LOW = Pi sleep

    // ── PB8 — IMU_SCL — AF4 I2C1 open drain ─────────────────
    // pin 8 x 2 = bits 17:16 in MODER
    // pin 8 x 4 = bit 32 → uses AFR[1] bit 0
    GPIOB->MODER  &= ~(MODER_MASK << MODER_POS(IMU_SCL_PB8));
    //  1111 1111 1111 1111 0000 1111 1111 1111  ← bits 17:16 clear
    GPIOB->MODER  |=  (MODER_AF   << MODER_POS(IMU_SCL_PB8));
    //  0000 0000 0000 0000 0010 0000 0000 0000  ← bits 17:16 = AF(10)
    GPIOB->OTYPER |=  (1 << IMU_SCL_PB8);
    //  0000 0001 0000 0000  ← PB8 open drain — I2C requires this
    GPIOB->AFR[1] &= ~(0xF << 0);   // clear bits 3:0 in AFR[1]
    GPIOB->AFR[1] |=  (4   << 0);   // AF4 = I2C1
    //  0000 0100  ← bits 3:0 = AF4

    // ── PB9 — IMU_SDA — AF4 I2C1 open drain ─────────────────
    // pin 9 x 2 = bits 19:18 in MODER
    // pin 9 in AFR[1] = bits 7:4
    GPIOB->MODER  &= ~(MODER_MASK << MODER_POS(IMU_SDA_PB9));
    //  1111 1111 1111 1100 1111 1111 1111 1111  ← bits 19:18 clear
    GPIOB->MODER  |=  (MODER_AF   << MODER_POS(IMU_SDA_PB9));
    //  0000 0000 0000 0010 0000 0000 0000 0000  ← bits 19:18 = AF(10)
    GPIOB->OTYPER |=  (1 << IMU_SDA_PB9);
    //  0000 0010 0000 0000  ← PB9 open drain — I2C requires this
    GPIOB->AFR[1] &= ~(0xF << 4);   // clear bits 7:4 in AFR[1]
    GPIOB->AFR[1] |=  (4   << 4);   // AF4 = I2C1
    //  0100 0000  ← bits 7:4 = AF4

    // ── PC0 — REED_SW — input with pull-up ───────────────────
    // pin 0 x 2 = bits 1:0 in MODER and PUPDR
    GPIOC->MODER &= ~(MODER_MASK << MODER_POS(REED_SW_PC0));
    //               1111 1100  ← bits 1:0 clear = INPUT(00)
    GPIOC->PUPDR &= ~(PUPDR_MASK << PUPDR_POS(REED_SW_PC0));
    GPIOC->PUPDR |=  (PUPDR_UP   << PUPDR_POS(REED_SW_PC0));
    //               0000 0001  ← bits 1:0 = PULLUP(01)

    // ── PC1 — ESP32_TRIG — input with pull-up ────────────────
    // pin 1 x 2 = bits 3:2 in MODER and PUPDR
    GPIOC->MODER &= ~(MODER_MASK << MODER_POS(ESP32_TRIG_PC1));
    //               1111 0011  ← bits 3:2 clear = INPUT(00)
    GPIOC->PUPDR &= ~(PUPDR_MASK << PUPDR_POS(ESP32_TRIG_PC1));
    GPIOC->PUPDR |=  (PUPDR_UP   << PUPDR_POS(ESP32_TRIG_PC1));
    //               0000 0100  ← bits 3:2 = PULLUP(01)

    // ── PC13 — LEAK_PC13 — input with pull-up ────────────────
    // pin 13 x 2 = bits 27:26 in MODER and PUPDR
    GPIOC->MODER &= ~(MODER_MASK << MODER_POS(LEAK_PC13));
    //  1111 1111 0011 1111 1111 1111 1111 1111  ← bits 27:26 clear
    GPIOC->PUPDR &= ~(PUPDR_MASK << PUPDR_POS(LEAK_PC13));
    GPIOC->PUPDR |=  (PUPDR_UP   << PUPDR_POS(LEAK_PC13));
    //  0000 0000 0100 0000 0000 0000 0000 0000  ← bits 27:26 = PULLUP(01)

    // ── TODO: PA0 — BATT_ADC — analog ────────────────────────
    // Stage 4: MODER = ANALOG(11), no pull resistors
    // Ignore for now — continue
    // ─────────────────────────────────────────────────────────
}

// ═══════════════════════════════════════════════════════════════
// uart2_init
// ───────────────────────────────────────────────────────────────
// Purpose: configure UART2 bare metal — 115200 8N1
// PA2 = TX AF7, PA3 = RX AF7
// ═══════════════════════════════════════════════════════════════
static void uart2_init(void)
{
    // ── PA2 — UART2 TX — AF7 ─────────────────────────────────
    // pin 2 x 2 = bits 5:4 in MODER
    // pin 2 x 4 = bits 11:8 in AFR[0]
    GPIOA->MODER  &= ~(MODER_MASK << MODER_POS(UART_TX_PA2));
    //               1111 1111 1100 1111  ← bits 5:4 clear
    GPIOA->MODER  |=  (MODER_AF   << MODER_POS(UART_TX_PA2));
    //               0000 0000 0010 0000  ← bits 5:4 = AF(10)
    GPIOA->AFR[0] &= ~(0xF << AFR_POS(UART_TX_PA2));
    GPIOA->AFR[0] |=  (7   << AFR_POS(UART_TX_PA2));
    //               0111 0000 0000 0000  ← bits 11:8 = AF7 UART2

    // ── PA3 — UART2 RX — AF7 ─────────────────────────────────
    // pin 3 x 2 = bits 7:6 in MODER
    // pin 3 x 4 = bits 15:12 in AFR[0]
    GPIOA->MODER  &= ~(MODER_MASK << MODER_POS(UART_RX_PA3));
    //               1111 1111 0011 1111  ← bits 7:6 clear
    GPIOA->MODER  |=  (MODER_AF   << MODER_POS(UART_RX_PA3));
    //               0000 0000 1000 0000  ← bits 7:6 = AF(10)
    GPIOA->AFR[0] &= ~(0xF << AFR_POS(UART_RX_PA3));
    GPIOA->AFR[0] |=  (7   << AFR_POS(UART_RX_PA3));
    //               0111 0000 0000 0000 0000  ← bits 15:12 = AF7 UART2

    // ── UART2 register configuration ─────────────────────────
    // STM32F4 USART BRR uses mantissa/fraction encoding when oversampling by 16.
    // APB1 = 42MHz, baud = 115200
    // USARTDIV = 42,000,000 / (16 x 115200) = 22.786
    // Mantissa = 22 = 0x16
    // Fraction = round(0.786 x 16) = 13 = 0xD
    // BRR = (0x16 << 4) | 0xD = 0x016D
    USART2->BRR = 0x016D;       // USART2 @ 42MHz APB1, 115200 baud, 8N1

    // CR1: 8 bit word, no parity, enable TX + RX + UART
    USART2->CR1 = (1 << 3) |    // 0000 1000 — TE: transmit enable
                  (1 << 2) |    // 0000 0100 — RE: receive enable
                  (1 << 13);    // 0010 0000 0000 0000 — UE: UART enable
}

// ═══════════════════════════════════════════════════════════════
// tim2_pwm_gpio_init
// ───────────────────────────────────────────────────────────────
// Purpose: configure PA5 and PA1 as TIM2 AF1 outputs
// Must run before tim2_pwm_mode_init
// ═══════════════════════════════════════════════════════════════
static void tim2_pwm_gpio_init(void)
{
    // ── PA5 — TIM2 CH1 — AF1 ─────────────────────────────────
    // pin 5 x 2 = bits 11:10 in MODER
    // pin 5 x 4 = bits 23:20 in AFR[0]
    GPIOA->MODER  &= ~(MODER_MASK << MODER_POS(MOT_PORT_PA5));
    //               1111 0011 1111 1111  ← bits 11:10 clear
    GPIOA->MODER  |=  (MODER_AF   << MODER_POS(MOT_PORT_PA5));
    //               0000 1000 0000 0000  ← bits 11:10 = AF(10)
    GPIOA->OTYPER &= ~(1 << MOT_PORT_PA5);
    //               1101 1111  ← push-pull output
    GPIOA->PUPDR  &= ~(PUPDR_MASK << PUPDR_POS(MOT_PORT_PA5));
    //               no pull resistors on PWM output
    GPIOA->AFR[0] &= ~(0xF << AFR_POS(MOT_PORT_PA5));
    GPIOA->AFR[0] |=  (1   << AFR_POS(MOT_PORT_PA5));
    //               0001 0000 0000 0000 0000 0000  ← bits 23:20 = AF1 TIM2

    // ── PA1 — TIM2 CH2 — AF1 ─────────────────────────────────
    // pin 1 x 2 = bits 3:2 in MODER
    // pin 1 x 4 = bits 7:4 in AFR[0]
    GPIOA->MODER  &= ~(MODER_MASK << MODER_POS(MOT_STBD_PA1));
    //               1111 1111 1111 0011  ← bits 3:2 clear
    GPIOA->MODER  |=  (MODER_AF   << MODER_POS(MOT_STBD_PA1));
    //               0000 0000 0000 1000  ← bits 3:2 = AF(10)
    GPIOA->OTYPER &= ~(1 << MOT_STBD_PA1);
    //               1111 1101  ← push-pull output
    GPIOA->PUPDR  &= ~(PUPDR_MASK << PUPDR_POS(MOT_STBD_PA1));
    //               no pull resistors on PWM output
    GPIOA->AFR[0] &= ~(0xF << AFR_POS(MOT_STBD_PA1));
    GPIOA->AFR[0] |=  (1   << AFR_POS(MOT_STBD_PA1));
    //               0001 0000  ← bits 7:4 = AF1 TIM2
}

// ═══════════════════════════════════════════════════════════════
// tim2_pwm_mode_init
// ───────────────────────────────────────────────────────────────
// Purpose: configure TIM2 for 50Hz PWM on CH1 and CH2
// PSC=83 ARR=19999 at 84MHz APB1 timer clock
// ═══════════════════════════════════════════════════════════════
static void tim2_pwm_mode_init(void)
{
    // ── Timer clock and period ────────────────────────────────
    // APB1 prescaler /2 → doubling → TIM2 clock = 84MHz
    // PSC=83 → 84MHz/(83+1) = 1MHz → 1 tick = 1µs
    // ARR=19999 → 20000 ticks = 20ms = 50Hz
    TIM2->PSC  = 83;            // prescaler
    TIM2->ARR  = 19999;         // period — 20ms = 50Hz

    // ── Neutral pulse before enabling outputs ─────────────────
    TIM2->CCR1 = PWM_NEUTRAL_US;    // port  → 1500µs
    TIM2->CCR2 = PWM_NEUTRAL_US;    // stbd  → 1500µs

    // ── CH1 — PWM mode 1 + output compare preload ────────────
    // OC1M bits 6:4 = 110 = PWM mode 1
    // pin HIGH while counter < CCR1, LOW when counter > CCR1
    TIM2->CCMR1 &= ~(TIM_CCMR1_OC1M | TIM_CCMR1_OC1PE);
    TIM2->CCMR1 |=  (TIM_CCMR1_OC1M_1 |    // 0010 0000 — OC1M bit 5
                     TIM_CCMR1_OC1M_2 |    // 0100 0000 — OC1M bit 6
                     TIM_CCMR1_OC1PE);     // 0000 1000 — preload enable

    // ── CH2 — PWM mode 1 + output compare preload ────────────
    TIM2->CCMR1 &= ~(TIM_CCMR1_OC2M | TIM_CCMR1_OC2PE);
    TIM2->CCMR1 |=  (TIM_CCMR1_OC2M_1 |    // OC2M bit 13
                     TIM_CCMR1_OC2M_2 |    // OC2M bit 14
                     TIM_CCMR1_OC2PE);     // preload enable

    // ── Active high polarity — clear polarity bits ────────────
    TIM2->CCER &= ~(TIM_CCER_CC1P | TIM_CCER_CC2P);

    // ── Enable CH1 + CH2 outputs ──────────────────────────────
    TIM2->CCER |= (TIM_CCER_CC1E | TIM_CCER_CC2E);
    //             0001 0001  ← bit0=CC1E, bit4=CC2E

    // ── Auto-reload preload enable ────────────────────────────
    TIM2->CR1  |= TIM_CR1_ARPE;    // 0000 0000 1000 0000

    // ── Force register update ─────────────────────────────────
    TIM2->EGR  |= TIM_EGR_UG;      // 0000 0001 — update generation

    // ── Start counter ─────────────────────────────────────────
    TIM2->CR1  |= TIM_CR1_CEN;     // 0000 0001 — counter enable
}

// ═══════════════════════════════════════════════════════════════
// uart2_write  [PRIVATE]
// ───────────────────────────────────────────────────────────────
// Purpose: bare metal UART2 byte transmit
// Polls TX empty flag before each byte — blocking but simple
// ═══════════════════════════════════════════════════════════════
static void uart2_write(uint8_t *data, uint16_t len)
{
    for (uint16_t i = 0; i < len; i++)
    {
        while (!(USART2->SR & (1 << 7)));   // wait TXE — TX empty
        //                      1000 0000  ← bit 7 = TXE flag
        USART2->DR = data[i];               // write byte to data register
    }
}

// ── printf redirect to UART2 ─────────────────────────────────
// Retargets printf() to UART2 bare metal transmit
// Debug infrastructure only — not in control path
int _write(int file, char *ptr, int len)
{
    (void)file;
    uart2_write((uint8_t*)ptr, (uint16_t)len);
    return len;
}

/* USER CODE END 4 */

// ═══════════════════════════════════════════════════════════════
// SystemClock_Config — CubeMX generated — keep HAL here
// ───────────────────────────────────────────────────────────────
// PLL: HSI 16MHz → PLLM/8 → PLLN x168 → PLLP/4 → 84MHz
// APB1 /2 = 42MHz peripheral, 84MHz timer (doubling applies)
// APB2 /1 = 84MHz
// ═══════════════════════════════════════════════════════════════
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

    RCC_OscInitStruct.OscillatorType      = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState            = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState        = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource       = RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL.PLLM            = 8;
    RCC_OscInitStruct.PLL.PLLN            = 168;
    RCC_OscInitStruct.PLL.PLLP            = RCC_PLLP_DIV4;
    RCC_OscInitStruct.PLL.PLLQ            = 2;
    RCC_OscInitStruct.PLL.PLLR            = 2;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) Error_Handler();

    RCC_ClkInitStruct.ClockType      = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                     | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) Error_Handler();
}

// ═══════════════════════════════════════════════════════════════
// Error_Handler
// ═══════════════════════════════════════════════════════════════
void Error_Handler(void)
{
    /* USER CODE BEGIN Error_Handler_Debug */
    __disable_irq();
    while (1) {}
    /* USER CODE END Error_Handler_Debug */
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
    /* USER CODE BEGIN 6 */
    /* USER CODE END 6 */
}
#endif
