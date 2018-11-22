/**************************************************************************//**
 * @file
 * @brief TFT example for EFM32_G2xx_DK using EBI access
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
#include "cmu.h"
#include "tftdraw.h"

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
  uint16_t aemState  = 0;
  int      firstRun  = 1;
  int      toggleLED = 0;

  /* Chip errata */
  CHIP_init();

  /* Use 32MHZ HFXO as core clock frequency */
  CMU_setHFCLK(CMU_CLK_HFXO);

  /* Initialize DVK board register access */
  DVK_init();

  /* Setup SysTick Timer for 10 msec interrupts  */
  if (SysTick_Config(CMU_ClockFreqGet(cmuClock_CORE) / 100))
  {
    while (1) ;
  }

  /* Update TFT display forever */
  while (1)
  {
    /* Check if we should control TFT display instead of
     * AEM/board control application. Read state of AEM pushbutton */
    aemState = DVK_readRegister(BC_AEMSTATE);
    if (aemState == 1)
    {
      /* Make sure TFT init is run at least once */
      if ((DVK_readRegister(BC_BUS_CFG) != BC_BUS_CFG_EBI) || firstRun)
      {
        DVK_writeRegister(BC_BUS_CFG, BC_BUS_CFG_EBI);
        TFT_init();
        firstRun = 0;
      }   
      /* Update display */
      TFT_displayUpdate();
    }

    /* Toggle led after each TFT_displayUpdate iteration */
    if (toggleLED)
    {
      DVK_setLEDs(0x0000);
      toggleLED = 0;
    }
    else
    {
      DVK_setLEDs(0x000f);
      toggleLED = 1;
    }
  }
}
