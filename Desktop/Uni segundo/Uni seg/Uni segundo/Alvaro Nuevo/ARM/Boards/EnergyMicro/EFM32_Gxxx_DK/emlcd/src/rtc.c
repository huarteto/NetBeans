/**************************************************************************//**
 * @file
 * @brief Real Time Counter, implements simple trigger functionality
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

#include <stddef.h>
#include "efm32.h"
#include "rtc.h"

#define RTC_FREQ    32768
void (*cb)(void) = NULL;

/**************************************************************************//**
 * @brief RTC Interrupt Handler, does nothing but clear the flag.
 *        The interrupt table is in assembly startup file startup_efm32.s
 *****************************************************************************/
void RTC_IRQHandler(void)
{
  /* Clear interrupt source */
  RTC->IFC = RTC_IFC_COMP0;

  /* Trigger callback */
  if (cb != NULL)
  {
    cb();
  }

  /* Disable interrupt */
  RTC->IEN = 0;

  /* Disable RTC */
  RTC->CTRL &= ~(RTC_CTRL_EN);
}

/**************************************************************************//**
 * @brief RTC trigger enable
 * @param msec Enable trigger in msec
 * @param fp Call this function if not NULL
 *****************************************************************************/
void RTC_trigger(uint32_t msec, void (*funcPointer)(void))
{
  /* Clear pending interrupts */
  RTC->IEN = 0;
  RTC->IFC = RTC_IFC_COMP0;
  NVIC_DisableIRQ(RTC_IRQn);
  NVIC_ClearPendingIRQ(RTC_IRQn);
  /* Register callback */
  cb = funcPointer;

  /* Calculate trigger value in ticks based on 32768Hz clock */
  RTC->COMP0 = (RTC_FREQ * msec) / 1000;
  /* Restart counter */
  RTC->CTRL = ~(RTC_CTRL_EN);

  /* Wait until all registers are updated */
  while (RTC->SYNCBUSY) ;

  /* Enable interrupt */
  NVIC_EnableIRQ(RTC_IRQn);
  RTC->IEN = RTC_IEN_COMP0;

  /* Start Counter */
  RTC->CTRL = RTC_CTRL_EN |
              RTC_CTRL_DEBUGRUN |
              RTC_CTRL_COMP0TOP;
  while (RTC->SYNCBUSY) ;
}

/**************************************************************************//**
 * @brief Enables LFACLK and selects LFXO as clock source for RTC
 *****************************************************************************/
void RTC_init(void)
{
  /* Enable LFXO as LFACLK in CMU */
  CMU->HFCORECLKEN0 |= CMU_HFCORECLKEN0_LE;
  CMU->OSCENCMD      = CMU_OSCENCMD_LFXOEN;
  while (!(CMU->STATUS & CMU_STATUS_LFXORDY)) ;

  CMU->LFCLKSEL &= ~(_CMU_LFCLKSEL_LFA_MASK);
  CMU->LFCLKSEL |= CMU_LFCLKSEL_LFA_LFXO;

  /* Enable RTC clock */
  CMU->LFACLKEN0 |= CMU_LFACLKEN0_RTC;
}
