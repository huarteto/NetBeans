/**************************************************************************//**
 * @file
 * @brief LCD controller and Energy Mode/RTC demo for  EFM32_G8xx_DK
 *  NOTE! This uses onbard segment LCD controller, and will not work on
 *        EFM32_G2xx_DK
 * @author Energy Micro AS
 * @version 1.2.0
 ******************************************************************************
 * @section License
 * <b>(C) Copyright 2009 Energy Micro AS, http://www.energymicro.com</b>
 ******************************************************************************
 *
 * This source code is the property of Energy Micro AS. The source and compiled
 * code may only be used on Energy Micro "EFM32" microcontrollers.
 *
 * This copyright notice may not be removed from the source code nor changed.
 *
 * DISCLAIMER OF WARRANTY/LIMITATION OF REMEDIES: Energy Micro AS has no
 * obligation to support this Software. Energy Micro AS is providing the
 * Software "AS IS", with no express or implied warranties of any kind,
 * including, but not limited to, any implied warranties of merchantability
 * or fitness for any particular purpose or warranties against infringement
 * of any proprietary rights of a third party.
 *
 * Energy Micro AS will not be liable for any consequential, incidental, or
 * special damages, or any other relief, or for any claim by any third party,
 * arising from your use of this Software.
 *
 *****************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include "chip.h"
#include "efm32.h"
#include "efm32_cmu.h"
#include "dvk.h"
#include "vcmp.h"
#include "lcdcontroller.h"
#include "lcdtest.h"

volatile uint32_t msTicks; /* counts 1ms timeTicks */

/**************************************************************************//**
 * @brief SysTick_Handler
 * Interrupt Service Routine for system tick counter
 *****************************************************************************/
void SysTick_Handler(void)
{
  msTicks++;       /* increment counter necessary in Delay()*/
}

/**************************************************************************//**
 * @brief Delays number of msTick Systicks (typically 1 ms)
 * @param dlyTicks Number of ticks to delay
 *****************************************************************************/
void Delay(uint32_t dlyTicks)
{
  uint32_t curTicks;

  curTicks = msTicks;
  while ((msTicks - curTicks) < dlyTicks) ;
}

/**************************************************************************//**
 * @brief  Main function
 *****************************************************************************/
int main(void)
{
  bool vboost = false;

  /* Chip errata */
  CHIP_init();

  /* Initialize DVK board register access */
  DVK_init();

  /* Setup SysTick Timer for 1 msec interrupts  */
  if (SysTick_Config(CMU_ClockFreqGet(cmuClock_CORE) / 1000))
  {
    while (1) ;
  }

  /* Initialize board specific registers */
  VCMP_init();
  /* Check that voltage is above 3V, if not use voltage boost */
  /* For EFM32G890F128 - voltage = 1.67 + trigger * 0.034 */
  if (VCMP_compare(38)) vboost = true;

  VCMP_disable();

  /* Run Energy Mode with LCD demo, see lcdtest.c */
  LCD_Init(LCD, vboost);
  LCD_Test(LCD);
}
