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
ADC_HandleTypeDef hadc1;

TIM_HandleTypeDef htim2;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM2_Init(void);
static void MX_ADC1_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */
static void gpio_init(void);
static void tim2_pwm_gpio_init(void);
static void tim2_pwm_mode_init(void);
int _write(int file, char *ptr, int len);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM2_Init();
  MX_ADC1_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */

      gpio_init();
      tim2_pwm_gpio_init();
      tim2_pwm_mode_init();
      ESC_PWM_Init();

      // First UART startup log — confirms system is alive
      // Keep UART text plain ASCII for reliable terminal output.
            
      // printf() is redirected to USART2 by _write() in USER CODE 4.
      printf("StigReyn AUV v1.0 starting...\r\n");
      printf("ESC PWM init complete - PA5 + PA1 neutral\r\n");
      printf("System ready - waiting for commands\r\n");
      // char msg[] = "StigReyn AUV v1.0 starting...\r\n";
      // HAL_UART_Transmit(&huart2, (uint8_t*)msg, sizeof(msg)-1, 100);

      // char pwm_msg[] = "ESC PWM init complete - PA5 + PA1 neutral\r\n";
      // HAL_UART_Transmit(&huart2, (uint8_t*)pwm_msg, sizeof(pwm_msg)-1, 100);

      // char ready_msg[] = "System ready - waiting for commands\r\n";
      // HAL_UART_Transmit(&huart2, (uint8_t*)ready_msg, sizeof(ready_msg)-1, 100);

  /* USER CODE END 2 */

  /* Infinite loop */
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
        // Stage 4: add safety checks - leak, battery, watchdog
        // Stage 5: add sensor reads - IMU, Bar30
        // Stage 6: add Drive_Set() - throttle + turn commands
        // Stage 7: add UART RX commands from Pi
        // Stage 7b: add periodic telemetry at low rate, not every loop
        // Ignore for now - continue
        // ─────────────────────────────────────────────────────
    }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 83;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 19999;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LED_STATUS_PB0_Pin|LED_ARMED_PB1_Pin|PI_SLEEP_PB4_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : LEAK_PC13_Pin REED_SW_PC0_Pin */
  GPIO_InitStruct.Pin = LEAK_PC13_Pin|REED_SW_PC0_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : LED_STATUS_PB0_Pin LED_ARMED_PB1_Pin PI_SLEEP_PB4_Pin */
  GPIO_InitStruct.Pin = LED_STATUS_PB0_Pin|LED_ARMED_PB1_Pin|PI_SLEEP_PB4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : IMU_SCL_PB8_Pin IMU_SDA_PB9_Pin */
  GPIO_InitStruct.Pin = IMU_SCL_PB8_Pin|IMU_SDA_PB9_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

static void tim2_pwm_gpio_init(void)
{
    // ── PA5 — TIM2 CH1 AF1 ─────────────────────────────
    // ── PA1 — TIM2 CH2 AF1 ─────────────────────────────

    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;

    // Set PA1 + PA5 to Alternate Function mode (10)
    GPIOA->MODER &= ~((MODER_MASK << MODER_POS(MOT_STBD_PA1)) |
                      (MODER_MASK << MODER_POS(MOT_PORT_PA5)));

    GPIOA->MODER |=  ((MODER_AF << MODER_POS(MOT_STBD_PA1)) |
                      (MODER_AF << MODER_POS(MOT_PORT_PA5)));

    // Push-pull outputs
    GPIOA->OTYPER &= ~((1 << MOT_STBD_PA1) |
                       (1 << MOT_PORT_PA5));

    // No pull resistors
    GPIOA->PUPDR &= ~((PUPDR_MASK << PUPDR_POS(MOT_STBD_PA1)) |
                      (PUPDR_MASK << PUPDR_POS(MOT_PORT_PA5)));

    // AF1 = TIM2
    GPIOA->AFR[0] &= ~((0xF << AFR_POS(MOT_STBD_PA1)) |
                       (0xF << AFR_POS(MOT_PORT_PA5)));

    GPIOA->AFR[0] |=  ((1 << AFR_POS(MOT_STBD_PA1)) |
                       (1 << AFR_POS(MOT_PORT_PA5)));
}

static void tim2_pwm_mode_init(void)
{
    // Neutral pulse before enabling outputs
    TIM2->CCR1 = PWM_NEUTRAL_US;
    TIM2->CCR2 = PWM_NEUTRAL_US;

    // CH1 PWM mode 1 + preload
    TIM2->CCMR1 &= ~(TIM_CCMR1_OC1M | TIM_CCMR1_OC1PE);
    TIM2->CCMR1 |=  (TIM_CCMR1_OC1M_1 |
                     TIM_CCMR1_OC1M_2 |
                     TIM_CCMR1_OC1PE);

    // CH2 PWM mode 1 + preload
    TIM2->CCMR1 &= ~(TIM_CCMR1_OC2M | TIM_CCMR1_OC2PE);
    TIM2->CCMR1 |=  (TIM_CCMR1_OC2M_1 |
                     TIM_CCMR1_OC2M_2 |
                     TIM_CCMR1_OC2PE);

    // Active high polarity
    TIM2->CCER &= ~(TIM_CCER_CC1P | TIM_CCER_CC2P);

    // Enable CH1 + CH2 outputs
    TIM2->CCER |= (TIM_CCER_CC1E | TIM_CCER_CC2E);

    // Force register update
    TIM2->EGR |= TIM_EGR_UG;

    // Enable counter
    TIM2->CR1 |= TIM_CR1_CEN;
}

// ── printf UART redirect ─────────────────────────────────────
// Retargets printf() to USART2 so debug messages appear on Mac terminal.
// This is debug infrastructure only; control-path code remains bare metal.
int _write(int file, char *ptr, int len)
{
    (void)file;
    HAL_UART_Transmit(&huart2, (uint8_t*)ptr, len, 100);
    return len;
}

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

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
    __disable_irq();
    while (1) {}
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
