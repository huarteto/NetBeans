/**************************************************************************//**
 * @file
 * @brief Energy Mode functionality, enter Energy Mode 0-4 and pend
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

#include "efm32.h"
#include "energymodes.h"

/* If this flag is set we will disconnect the debug interface to allow
 * going down when debugger is connected. However this will disable debug
 * access, and thus can be a potential hazard if we're locked out from
 * debug completly */
#undef DISCONNECT_DEBUG_INTERFACE

/**************************************************************************//**
 * @brief Enter EM4
 *****************************************************************************/
void EM4_Enter(void)
{
  int i;

  /* Write "magic pattern" (see EFM32 reference manual) to enable EM4 */
  /* Sequence is 2,3,2,3,2,3,2,3 */
  for (i = 0; i < 4; i++)
  {
    EMU->CTRL = 2 << _EMU_CTRL_EM4CTRL_SHIFT;
    EMU->CTRL = 3 << _EMU_CTRL_EM4CTRL_SHIFT;
  }
  EMU->CTRL = 2 << 2;
  /* Reset device to get back.... */
}

/**************************************************************************//**
 * @brief Enter EM3
 * Note! If this code is a basis for going into EM3 when HFXO is used, HFXO
 * must be restored after wake up (not done in this example)
 *****************************************************************************/
void EM3_Enter(void)
{
  uint32_t lfclk;
  int      systick, auxclk, dmaclk;

  /* Disable AUXHFRCO (debug clock prevents E Modes) */
  if (CMU->STATUS & CMU_STATUS_AUXHFRCOENS)
  {
    auxclk        = 1;
    CMU->OSCENCMD = CMU_OSCENCMD_AUXHFRCODIS;
  }

  /* Make sure DMA clock is running to enter EM2 (see chip errata) */
  if (CMU->HFCORECLKEN0 & CMU_HFCORECLKEN0_DMA)
  {
    dmaclk = 1;
  }
  else
  {
    CMU->HFCORECLKEN0 |= CMU_HFCORECLKEN0_DMA;
    dmaclk             = 0;
  }

  /* Disable LFA, LFB */
  lfclk         = CMU->LFCLKSEL;
  CMU->LFCLKSEL = 0;

  /* Disconnect debug interface (disable pull-up/down) */
#if defined(DISCONNECT_DEBUG_INTERFACE)
  GPIO->ROUTE = 0;
#endif

  /* If systick is enabled, disable it */
  if (SysTick->CTRL & SysTick_CTRL_ENABLE_Msk)
  {
    systick        = 1;
    SysTick->CTRL &= ~(1 << SysTick_CTRL_ENABLE_Pos);
  }

  /* EM3 */
  SCB->SCR = (1 << SCB_SCR_SLEEPDEEP_Pos);
  __WFI();

  /* Renenable AUXHFRCO */
  if (auxclk == 1)
  {
    CMU->OSCENCMD = CMU_OSCENCMD_AUXHFRCOEN;
  }

  /* Reenable systick */
  if (systick)
  {
    SysTick->CTRL |= (1 << SysTick_CTRL_ENABLE_Pos);
  }

  /* Restore DMA clock (see chip errata) */
  if (!dmaclk)
  {
    CMU->HFCORECLKEN0 &= ~CMU_HFCORECLKEN0_DMA;
  }

  /* Reenable LFCLKs */
  CMU->LFCLKSEL = lfclk;
}


/**************************************************************************//**
 * @brief Enter EM2
 * Note! If this code is a basis for going into EM3 when HFXO is used, HFXO
 * must be restored after wake up (not done in this example)
 *****************************************************************************/
void EM2_Enter(void)
{
  int dmaclk, auxclk, systick;

  /* Disable AUXHFRCO (debug clock prevents E Modes) */
  if (CMU->STATUS & CMU_STATUS_AUXHFRCOENS)
  {
    auxclk        = 1;
    CMU->OSCENCMD = CMU_OSCENCMD_AUXHFRCODIS;
  }

  /* Make sure DMA clock is running to enter EM2 (see chip errata) */
  if (CMU->HFCORECLKEN0 & CMU_HFCORECLKEN0_DMA)
  {
    dmaclk = 1;
  }
  else
  {
    CMU->HFCORECLKEN0 |= CMU_HFCORECLKEN0_DMA;
    dmaclk             = 0;
  }

  /* Disconnect debug interface (disable pull-up/down) */
#if defined(DISCONNECT_DEBUG_INTERFACE)
  GPIO->ROUTE = 0;
#endif

  /* If systick is enabled, disable it */
  if (SysTick->CTRL & SysTick_CTRL_ENABLE_Msk)
  {
    systick        = 1;
    SysTick->CTRL &= ~(1 << SysTick_CTRL_ENABLE_Pos);
  }

  /* EM2 */
  SCB->SCR = (1 << SCB_SCR_SLEEPDEEP_Pos);
  __WFI();

  /* Renenable AUXHFRCO */
  if (auxclk == 1)
  {
    CMU->OSCENCMD = CMU_OSCENCMD_AUXHFRCOEN;
  }

  /* Reenable systick */
  if (systick)
  {
    SysTick->CTRL |= (1 << SysTick_CTRL_ENABLE_Pos);
  }

  /* Restore DMA clock (see chip errata) */
  if (!dmaclk)
  {
    CMU->HFCORECLKEN0 &= ~CMU_HFCORECLKEN0_DMA;
  }
}


/**************************************************************************//**
 * @brief Enter EM1
 *****************************************************************************/
void EM1_Enter(void)
{
  uint32_t systick, auxclk;

  /* Disable AUXHFRCO (debug clock prevents E Modes) */
  if (CMU->STATUS & CMU_STATUS_AUXHFRCOENS)
  {
    auxclk        = 1;
    CMU->OSCENCMD = CMU_OSCENCMD_AUXHFRCODIS;
  }

  /* Disconnect debug interface (disable pull-up/down) */
#if defined(DISCONNECT_DEBUG_INTERFACE)
  GPIO->ROUTE = 0;
#endif

  /* If systick is enabled, disable it */
  if (SysTick->CTRL & SysTick_CTRL_ENABLE_Msk)
  {
    systick        = 1;
    SysTick->CTRL &= ~(1 << SysTick_CTRL_ENABLE_Pos);
  }

  /* Clear SLEEPDEEP for EM1 */
  SCB->SCR &= ~(1 << SCB_SCR_SLEEPDEEP_Pos);
  /* Go down */
  __WFI();

  /* Renenable AUXHFRCO */
  if (auxclk == 1)
  {
    CMU->OSCENCMD = CMU_OSCENCMD_AUXHFRCOEN;
  }

  /* Reenable systick */
  if (systick)
  {
    SysTick->CTRL |= (1 << SysTick_CTRL_ENABLE_Pos);
  }
}
