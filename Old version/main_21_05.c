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
// ═══════════════════════════════════════════════════════════════
// SYSTEM STARTUP SEQUENCE:
// 1. HAL_Init()          — SysTick, interrupt priority grouping
// 2. SystemClock_Config() — HSI 16MHz (PLL upgrade at Stage 3)
// 3. MX_GPIO_Init()      — RCC clocks, pin modes (bare metal below)
// 4. MX_TIM2_Init()      — TIM2 PSC=15 ARR=19999 PWM mode
// 5. gpio_init()         — our bare metal GPIO setup
// 6. ESC_PWM_Init()      — start PWM, set neutral on both thrusters
// 7. while(1)            — main loop, heartbeat LED
// ═══════════════════════════════════════════════════════════════
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "pin_config.h"
#include "esc_pwm.h"
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
TIM_HandleTypeDef htim2;

/* USER CODE BEGIN PV */
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM2_Init(void);
void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* USER CODE BEGIN PFP */
static void gpio_init(void);
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

    // CubeMX generated init — keep HAL here for infrastructure
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_TIM2_Init();

    /* USER CODE BEGIN 2 */

    // Our bare metal GPIO setup — pin modes, pull resistors
    gpio_init();

    // Start PWM output — sets both thrusters to 1500µs neutral
    // ESC will arm when it receives neutral signal
    ESC_PWM_Init();

    /* USER CODE END 2 */

    /* USER CODE BEGIN WHILE */
    while (1)
    {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

        // ── Heartbeat LED ────────────────────────────────────
        // PB0 blinks 500ms — confirms firmware is running
        // bare metal toggle — no HAL
        GPIOB->ODR ^= (1 << LED_STATUS_PB0);   // 0000 0001 — toggle PB0
        HAL_Delay(500);

        // ── TODO: MAIN CONTROL LOOP ──────────────────────────
        // Stage 3: replace HAL_Delay with timer tick flag
        // Stage 4: add safety checks — leak, battery, watchdog
        // Stage 5: add sensor reads — IMU, Bar30
        // Stage 6: add Drive_Set() — throttle + turn commands
        // Stage 7: add UART comms — receive commands from Pi
        // Ignore for now — continue
        // ─────────────────────────────────────────────────────
    }
    /* USER CODE END 3 */
}

// ═══════════════════════════════════════════════════════════════
// gpio_init  [PRIVATE]
// ───────────────────────────────────────────────────────────────
// Purpose: configure our GPIO pins bare metal
//          RCC clocks enabled by MX_GPIO_Init() above
//          we just set MODER and PUPDR here
// ═══════════════════════════════════════════════════════════════
/* USER CODE BEGIN 4 */
static void gpio_init(void)
{
    // ── PB0 — LED_STATUS — output ────────────────────────────
    // pin 0 x 2 = bit 0 in MODER
    GPIOB->MODER &= ~(MODER_MASK   << MODER_POS(LED_STATUS_PB0));
    //               1111 1100  ← bits 1:0 clear
    GPIOB->MODER |=  (MODER_OUTPUT << MODER_POS(LED_STATUS_PB0));
    //               0000 0001  ← bits 1:0 = OUTPUT(01)
    GPIOB->ODR   &= ~(1 << LED_STATUS_PB0);
    //               1111 1110  ← PB0 LOW = LED off at startup

    // ── PB1 — LED_ARMED — output ─────────────────────────────
    // pin 1 x 2 = bit 2 in MODER
    GPIOB->MODER &= ~(MODER_MASK   << MODER_POS(LED_ARMED_PB1));
    //               1111 0011  ← bits 3:2 clear
    GPIOB->MODER |=  (MODER_OUTPUT << MODER_POS(LED_ARMED_PB1));
    //               0000 0100  ← bits 3:2 = OUTPUT(01)
    GPIOB->ODR   &= ~(1 << LED_ARMED_PB1);
    //               1111 1101  ← PB1 LOW = LED off at startup

    // ── PB4 — PI_SLEEP — output ──────────────────────────────
    // pin 4 x 2 = bit 8 in MODER
    GPIOB->MODER &= ~(MODER_MASK   << MODER_POS(PI_SLEEP_PB4));
    //               1111 1111 0000 1111  ← bits 9:8 clear
    GPIOB->MODER |=  (MODER_OUTPUT << MODER_POS(PI_SLEEP_PB4));
    //               0000 0000 0001 0000  ← bits 9:8 = OUTPUT(01)
    GPIOB->ODR   &= ~(1 << PI_SLEEP_PB4);
    //               0000 0000 1110 1111  ← PB4 LOW = Pi sleep at startup

    // ── PC0 — REED_SW — input with pull-up ───────────────────
    // pin 0 x 2 = bit 0 in MODER and PUPDR
    GPIOC->MODER &= ~(MODER_MASK << MODER_POS(REED_SW_PC0));
    //               1111 1100  ← bits 1:0 clear = INPUT(00)
    GPIOC->PUPDR &= ~(PUPDR_MASK << PUPDR_POS(REED_SW_PC0));
    GPIOC->PUPDR |=  (PUPDR_UP   << PUPDR_POS(REED_SW_PC0));
    //               0000 0100  ← bits 1:0 = PULLUP(01)

    // ── PC1 — ESP32_TRIG — input with pull-up ────────────────
    // pin 1 x 2 = bit 2 in MODER and PUPDR
    GPIOC->MODER &= ~(MODER_MASK << MODER_POS(ESP32_TRIG_PC1));
    //               1111 0011  ← bits 3:2 clear = INPUT(00)
    GPIOC->PUPDR &= ~(PUPDR_MASK << PUPDR_POS(ESP32_TRIG_PC1));
    GPIOC->PUPDR |=  (PUPDR_UP   << PUPDR_POS(ESP32_TRIG_PC1));
    //               0000 0100  ← bits 3:2 = PULLUP(01) wait — 
    //               0000 1000  ← bits 3:2 = PULLUP(01) correct

    // ── PC13 — LEAK_PC13 — input with pull-up ────────────────
    // pin 13 x 2 = bit 26 in MODER and PUPDR
    GPIOC->MODER &= ~(MODER_MASK << MODER_POS(LEAK_PC13));
    //  1111 1111 0011 1111 1111 1111 1111 1111  ← bits 27:26 clear
    GPIOC->PUPDR &= ~(PUPDR_MASK << PUPDR_POS(LEAK_PC13));
    GPIOC->PUPDR |=  (PUPDR_UP   << PUPDR_POS(LEAK_PC13));
    //  0000 0000 0100 0000 0000 0000 0000 0000  ← bits 27:26 = PULLUP(01)

    // ── TODO: PA0 — BATT_ADC — analog input ─────────────────
    // Stage 4: configure ADC1 CH0 for battery voltage reading
    // MODER = ANALOG(11) — no pull resistors for ADC
    // Ignore for now — continue
    // ─────────────────────────────────────────────────────────
}

// ── HAL_TIM_MspPostInit ──────────────────────────────────────
// Required by MX_TIM2_Init — connects TIM2 to physical pins
// PA5 = TIM2 CH1 AF1 — port  thruster
// PA1 = TIM2 CH2 AF1 — stbd  thruster
// Keep HAL here — called once at init, not in control path
void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    if (htim->Instance == TIM2)
    {
        __HAL_RCC_GPIOA_CLK_ENABLE();

        // PA5 — TIM2 CH1 — port thruster — AF1
        GPIO_InitStruct.Pin       = GPIO_PIN_5;
        GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull      = GPIO_NOPULL;
        GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF1_TIM2;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        // PA1 — TIM2 CH2 — stbd thruster — AF1
        GPIO_InitStruct.Pin       = GPIO_PIN_1;
        GPIO_InitStruct.Alternate = GPIO_AF1_TIM2;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    }
}
/* USER CODE END 4 */

// ═══════════════════════════════════════════════════════════════
// SystemClock_Config — CubeMX generated — do not modify
// HSI 16MHz — no PLL
// Stage 3: switch to PLL 84MHz — update PSC to 83 in CubeMX
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
    RCC_OscInitStruct.PLL.PLLState        = RCC_PLL_NONE;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) Error_Handler();

    RCC_ClkInitStruct.ClockType      = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                     | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_HSI;
    RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK) Error_Handler();
}

// ═══════════════════════════════════════════════════════════════
// MX_TIM2_Init — CubeMX generated — do not modify
// PSC=15 ARR=19999 — 50Hz at 16MHz HSI
// CH1 pulse=1500 CH2 pulse=1500 — neutral on startup
// ═══════════════════════════════════════════════════════════════
static void MX_TIM2_Init(void)
{
    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig     = {0};
    TIM_OC_InitTypeDef sConfigOC              = {0};

    htim2.Instance               = TIM2;
    htim2.Init.Prescaler         = 15;
    htim2.Init.CounterMode       = TIM_COUNTERMODE_UP;
    htim2.Init.Period            = 19999;
    htim2.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
    htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    if (HAL_TIM_Base_Init(&htim2) != HAL_OK) Error_Handler();

    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK) Error_Handler();
    if (HAL_TIM_PWM_Init(&htim2) != HAL_OK) Error_Handler();

    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode     = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK) Error_Handler();

    sConfigOC.OCMode     = TIM_OCMODE_PWM1;
    sConfigOC.Pulse      = 1500;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1) != HAL_OK) Error_Handler();
    if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_2) != HAL_OK) Error_Handler();

    HAL_TIM_MspPostInit(&htim2);
}

// ═══════════════════════════════════════════════════════════════
// MX_GPIO_Init — CubeMX generated — enables RCC clocks
// Bare metal pin configuration done in gpio_init() above
// ═══════════════════════════════════════════════════════════════
static void MX_GPIO_Init(void)
{
    // Enable peripheral clocks — bare metal equivalent:
    // RCC->AHB1ENR |= (1 << 0);  // 0000 0001 — GPIOA clock
    // RCC->AHB1ENR |= (1 << 1);  // 0000 0010 — GPIOB clock
    // RCC->AHB1ENR |= (1 << 2);  // 0000 0100 — GPIOC clock
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
}

// ═══════════════════════════════════════════════════════════════
// Error_Handler — system fault — disable interrupts, halt
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