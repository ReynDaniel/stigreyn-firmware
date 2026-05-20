#ifndef ESC_PWM_H
#define ESC_PWM_H

#include "main.h"
#include "pin_config.h"

/* --------------------------------------------------------------------------
 * ESC PWM pulse widths (microseconds)
 *
 * Standard RC/ESC timing:
 * 1100us = full reverse
 * 1500us = neutral / stop
 * 1900us = full forward
 * -------------------------------------------------------------------------- */
#define ESC_PWM_MIN_US      1100
#define ESC_PWM_NEUTRAL_US  1500
#define ESC_PWM_MAX_US      1900

/* --------------------------------------------------------------------------
 * ESC PWM public function prototypes
 * -------------------------------------------------------------------------- */

/* Initialize PWM outputs and start timers */
void ESC_PWM_Init(void);
void ESC_PWM_Set_Port(int16_t speed);   // -100 to +100 port is leftside ;)
void ESC_PWM_Set_Stbd(int16_t speed);   // -100 to +100 starboard is rightside ;)
void ESC_PWM_Set_Both(int16_t speed);   // -100 to +100
void ESC_PWM_Failsafe(void);            // both → 1500µs instantly

#endif /* ESC_PWM_H */