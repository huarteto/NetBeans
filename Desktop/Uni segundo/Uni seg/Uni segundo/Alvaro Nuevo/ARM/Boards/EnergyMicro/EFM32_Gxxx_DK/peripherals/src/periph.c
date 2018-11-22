/**************************************************************************//**
 * @file
 * @brief Peripheral Toggle Example, enables peripherals. Verify by checking
 *        board LEDs
 * @author Energy Micro AS
 * @version 1.2.0
 ******************************************************************************
 * @section License
 * <b>(C) Copyright 2010 Energy Micro AS, http://www.energymicro.com</b>
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
#include "chip.h"
#include "efm32.h"
#include "efm32_cmu.h"
#include "dvk.h"

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
  /* Chip errata */
  CHIP_init();

  /* Initialize DVK board register access */
  DVK_init();

  /* Setup SysTick Timer for 1 msec interrupts  */
  if (SysTick_Config(CMU_ClockFreqGet(cmuClock_CORE) / 1000))
  {
    while (1) ;
  }

  /* "Silly" loop that just enables peripheral access to the EFM32, and then
   * disables them again. Verify that DVK LEDs light up when access is enabled */
  while (1)
  {
    DVK_enablePeripheral(DVK_POTMETER); Delay(500);
    DVK_enablePeripheral(DVK_AMBIENT); Delay(500);
    DVK_enablePeripheral(DVK_IRDA); Delay(500);
    DVK_enablePeripheral(DVK_AUDIO_OUT); Delay(500);
    DVK_enablePeripheral(DVK_AUDIO_IN); Delay(500);
    DVK_enablePeripheral(DVK_ANALOG_SE); Delay(500);
    DVK_enablePeripheral(DVK_ANALOG_DIFF); Delay(500);
    DVK_enablePeripheral(DVK_RS232B); Delay(500);
    DVK_enablePeripheral(DVK_RS232A); Delay(500);
    DVK_enablePeripheral(DVK_ACCEL); Delay(500);
    DVK_enablePeripheral(DVK_SPI); Delay(500);
    DVK_enablePeripheral(DVK_I2C); Delay(500);

    DVK_disablePeripheral(DVK_POTMETER); Delay(500);
    DVK_disablePeripheral(DVK_AMBIENT); Delay(500);
    DVK_disablePeripheral(DVK_IRDA); Delay(500);
    DVK_disablePeripheral(DVK_AUDIO_OUT); Delay(500);
    DVK_disablePeripheral(DVK_AUDIO_IN); Delay(500);
    DVK_disablePeripheral(DVK_ANALOG_SE); Delay(500);
    DVK_disablePeripheral(DVK_ANALOG_DIFF); Delay(500);
    DVK_disablePeripheral(DVK_RS232B); Delay(500);
    DVK_disablePeripheral(DVK_RS232A); Delay(500);
    DVK_disablePeripheral(DVK_ACCEL); Delay(500);
    DVK_disablePeripheral(DVK_SPI); Delay(500);
    DVK_disablePeripheral(DVK_I2C); Delay(500);
  }
}
