// CubeMX generates this — MX_TIM2_Init()
// Let's read it like engineers

TIM_HandleTypeDef htim2;  // struct that holds all TIM2 settings
                           // HAL's way of grouping timer config
                           // same concept as your MSP430 structs

htim2.Instance = TIM2;    // which timer — TIM2 base address
                           // under the hood this is just a pointer
                           // to TIM2 registers in memory

htim2.Init.Prescaler = 83;  // divide 84MHz by (83+1) = 84
                              // result = 1MHz timer clock
                              // 1 tick = 1 microsecond
                              // THIS is why CCR value = microseconds directly

htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
                              // counter counts 0 → ARR then resets
                              // up mode — simplest, most common

htim2.Init.Period = 20000;   // ARR register = 20000
                              // counter resets every 20000 ticks
                              // 20000 x 1µs = 20ms = 50Hz
                              // THIS sets your PWM frequency

htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
                              // no extra division — use full 1MHz


Then the PWM channel config:

TIM_OC_InitTypeDef sConfigOC;  // output compare struct
                                 // configures what happens when
                                 // counter matches CCR value

sConfigOC.OCMode = TIM_OCMODE_PWM1;
// PWM mode 1:
// pin HIGH when counter < CCR
// pin LOW  when counter > CCR
// THIS is what creates your pulse

sConfigOC.Pulse = 1500;   // CCR register = 1500
                           // pin HIGH for 1500 ticks = 1500µs
                           // neutral signal for ESC
                           // change this number = change thrust

sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
// active high — pin goes HIGH during pulse
// not inverted
// standard for ESC signal

The pin connection — alternate function:

// CubeMX also configures GPIO
// this is what connects PA5 to TIM2

GPIO_InitStruct.Pin = GPIO_PIN_5;
GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
// AF_PP = Alternate Function Push Pull
// tells PA5 to listen to TIM2 not GPIO ODR
// this is MODER = 10 that we learned
// PP = push pull output — can drive high and low

GPIO_InitStruct.Alternate = GPIO_AF1_TIM2;
// AF1 = alternate function 1
// this is AFR register = 1
// tells PA5 specifically: use TIM2
// not UART, not SPI, not I2C — TIM2

Why HAL_TIM_SET_COMPARE is not magic:

// what we write:
__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, 1500);

// what it actually does under the hood:
TIM2->CCR1 = 1500;
// that's it — one register write
// HAL macro expands to exactly this
// you already knew this from our sessions

The full picture — what happens when ESC receives 1500µs:

STM32 timer counting 0→20000
At count 0    — PA5 goes HIGH
At count 1500 — PA5 goes LOW  (1500µs pulse)
At count 20000 — resets to 0, repeats

ESC reads pulse width
1500µs = neutral command
ESC holds motor at stop

Change CCR1 to 1900:
At count 1900 — PA5 goes LOW  (1900µs pulse)
ESC reads 1900µs = full forward
Motor spins forward

Why 50Hz specifically:

ESC expects update every 20ms minimum
Too slow = ESC thinks signal lost = failsafe
Too fast = some ESCs get confused

50Hz = 20ms = industry standard
       same as RC receivers
       same as servo controllers
       Blue Robotics ESC expects this