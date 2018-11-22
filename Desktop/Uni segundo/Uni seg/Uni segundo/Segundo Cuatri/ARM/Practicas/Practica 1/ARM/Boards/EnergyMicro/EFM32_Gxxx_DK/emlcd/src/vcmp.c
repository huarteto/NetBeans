/**************************************************************************//**
 * @file
 * @brief VCMP - Voltage Comparator
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
#include "efm32.h"
#include "vcmp.h"

/**************************************************************************//**
 * @brief VCMP initialization routine
 *****************************************************************************/
void VCMP_init(void)
{
  /* Enable LE peripherals */
  CMU->HFCORECLKEN0 |= CMU_HFCORECLKEN0_LE;
  /* Enable VCMP clock */
  CMU->HFPERCLKEN0 |= CMU_HFPERCLKEN0_VCMP;
}

/**************************************************************************//**
 * @brief VCMP deinitialization routine
 *****************************************************************************/
void VCMP_disable(void)
{
  /* Disable VCMP */
  VCMP->CTRL &= ~(VCMP_CTRL_EN);

  /* Disable clock to VCMP */
  CMU->HFPERCLKEN0 &= ~CMU_HFPERCLKEN0_VCMP;
}

/**************************************************************************//**
 * @brief Acquire voltage trigger level
 *
 * @param level
 *        The input level to compare against.
 * @return true if voltage is higher.
 *         false otherwise
 *****************************************************************************/
bool VCMP_compare(uint32_t level)
{
  /* Disable VCMP */
  VCMP->CTRL &= ~(VCMP_CTRL_EN);

  VCMP->INPUTSEL &= ~(_VCMP_INPUTSEL_MASK);
  VCMP->INPUTSEL |= (level << (_VCMP_INPUTSEL_TRIGLEVEL_SHIFT));

  /* Enable comparator */
  VCMP->CTRL &= ~(_VCMP_CTRL_WARMTIME_MASK);
  VCMP->CTRL |= 7 << (_VCMP_CTRL_WARMTIME_SHIFT);
  VCMP->CTRL |= VCMP_CTRL_EN;

  /* Delay until warm up ready */
  while (!(VCMP->STATUS & VCMP_STATUS_VCMPACT)) ;

  /* If zero result, voltage is higher */
  if ((VCMP->STATUS & VCMP_STATUS_VCMPOUT) == 0) return true;

  /* Otherwise return false */
  return false;
}

