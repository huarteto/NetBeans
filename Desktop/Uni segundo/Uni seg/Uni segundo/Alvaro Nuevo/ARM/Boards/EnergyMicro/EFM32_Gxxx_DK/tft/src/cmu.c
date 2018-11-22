/**************************************************************************//**
 * @file
 * @brief Clock Management Unit control and status
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
#include "cmu.h"

/**************************************************************************//**
 * @brief Get Frequency of HFCLK
 * @return Frequency in number of ticks per second
 *****************************************************************************/
uint32_t CMU_getHFCLKFreq(void)
{
  CMU_TypeDef *cmu = CMU;
  uint32_t    ticksPerSecond;

  /* Check source for core clock */
  switch (cmu->STATUS &
          (CMU_STATUS_HFRCOSEL |
           CMU_STATUS_HFXOSEL |
           CMU_STATUS_LFRCOSEL |
           CMU_STATUS_LFXOSEL))
  {
  case CMU_STATUS_HFXOSEL:
    ticksPerSecond = EFM32_HFXO_FREQ;
    break;
  case CMU_STATUS_LFRCOSEL:
    ticksPerSecond = EFM32_LFRCO_FREQ;
    break;
  case CMU_STATUS_LFXOSEL:
    ticksPerSecond = EFM32_LFXO_FREQ;
    break;
  case CMU_STATUS_HFRCOSEL:
  default:
    /* Find selected HFRCO band */
    switch ((cmu->HFRCOCTRL & _CMU_HFRCOCTRL_BAND_MASK) >> _CMU_HFRCOCTRL_BAND_SHIFT)
    {
    case _CMU_HFRCOCTRL_BAND_28MHZ:
      ticksPerSecond = 28000000;
      break;
    case _CMU_HFRCOCTRL_BAND_21MHZ:
      ticksPerSecond = 21000000;
      break;
    case _CMU_HFRCOCTRL_BAND_14MHZ:
      ticksPerSecond = 14000000;
      break;
    case _CMU_HFRCOCTRL_BAND_11MHZ:
      ticksPerSecond = 11000000;
      break;
    case _CMU_HFRCOCTRL_BAND_7MHZ:
      ticksPerSecond = 7000000;
      break;
    case _CMU_HFRCOCTRL_BAND_1MHZ:
      ticksPerSecond = 1500000;
      break;
    default:
      ticksPerSecond = 0;
      break;
    }
    break;
  }

  return ticksPerSecond;
}


/**************************************************************************//**
 * @brief Select HFCLK source oscillator
 * @param clk Source
 *****************************************************************************/
void CMU_setHFCLK(CMU_ClockSource clk)
{
  /* Enforce wait state until we know what new core clock is */
  MSC->READCTRL = MSC_READCTRL_MODE_WS1;

  switch (clk)
  {
  case CMU_CLK_HFXO:
    CMU->OSCENCMD = CMU_OSCENCMD_HFXOEN;
    while (!(CMU->STATUS & CMU_STATUS_HFXORDY)) ;
    CMU->CMD = CMU_CMD_HFCLKSEL_HFXO;
    break;
  case CMU_CLK_HFRCO:
    CMU->OSCENCMD = CMU_OSCENCMD_HFRCOEN;
    while (!(CMU->STATUS & CMU_STATUS_HFRCORDY)) ;
    CMU->CMD = CMU_CMD_HFCLKSEL_HFRCO;
    break;
  case CMU_CLK_LFXO:
    CMU->OSCENCMD = CMU_OSCENCMD_LFXOEN;
    while (!(CMU->STATUS & CMU_STATUS_LFXORDY)) ;
    CMU->CMD = CMU_CMD_HFCLKSEL_LFXO;
    break;
  case CMU_CLK_LFRCO:
    CMU->OSCENCMD = CMU_OSCENCMD_LFRCOEN;
    while (!(CMU->STATUS & CMU_STATUS_LFRCORDY)) ;
    CMU->CMD = CMU_CMD_HFCLKSEL_LFRCO;
    break;
  }

  if (CMU_getHFCORECLKFreq() < 16000000)
  {
    /* Reduce wait state if below 16Mhz */
    MSC->READCTRL = MSC_READCTRL_MODE_WS0;
  }
}


/**************************************************************************//**
 * @brief Returns the active High Frequency clock source
 * @return Clock source
 *****************************************************************************/
CMU_ClockSource CMU_getHFCLK(void)
{
  /* Check source for core clock */
  switch (CMU->STATUS &
          (CMU_STATUS_HFRCOSEL |
           CMU_STATUS_HFXOSEL |
           CMU_STATUS_LFRCOSEL |
           CMU_STATUS_LFXOSEL))
  {
  case CMU_STATUS_HFXOSEL:
    return CMU_CLK_HFXO;
  case CMU_STATUS_LFRCOSEL:
    return CMU_CLK_LFRCO;
  case CMU_STATUS_LFXOSEL:
    return CMU_CLK_LFXO;
  case CMU_STATUS_HFRCOSEL:
  default:
    return CMU_CLK_HFRCO;
  }
}


/**************************************************************************//**
 * @brief Returns HF core clock frequency
 * @return Ticks per second
 *****************************************************************************/
uint32_t CMU_getHFCORECLKFreq(void)
{
  uint32_t ticksPerSecond = CMU_getHFCLKFreq();

  /* Adjust according to clock divisor */
  return(ticksPerSecond / (1 << ((CMU->HFCORECLKDIV & _CMU_HFCORECLKDIV_MASK)
                                 >> _CMU_HFCORECLKDIV_HFCORECLKDIV_SHIFT)));
}


/**************************************************************************//**
 * @brief Returns peripheral HF peripheral clock frequency
 * @return Ticks per second
 *****************************************************************************/
uint32_t CMU_getHFPERCLKFreq(void)
{
  uint32_t ticksPerSecond = CMU_getHFCLKFreq();

  /* Adjust according to clock divisor */
  return(ticksPerSecond / (1 << ((CMU->HFPERCLKDIV & _CMU_HFPERCLKDIV_MASK)
                                 >> _CMU_HFPERCLKDIV_HFPERCLKDIV_SHIFT)));
}


/**************************************************************************//**
 * @brief Set HF core clock divisor
 * @param CMU_HFCORECLKDIV_ constant
 *****************************************************************************/
void CMU_setHFCORECLKDiv(uint32_t div)
{
  CMU->HFCORECLKDIV = (div << _CMU_HFCORECLKDIV_HFCORECLKDIV_SHIFT);
}


/**************************************************************************//**
 * @brief Returns HF core clock divisor
 * @return CMU_HFCORECLKDIV_HFCLK<0-512>
 *****************************************************************************/
uint32_t CMU_getHFCORECLKDiv(void)
{
  return(CMU->HFCORECLKDIV >> _CMU_HFCORECLKDIV_HFCORECLKDIV_SHIFT);
}


/**************************************************************************//**
 * @brief Set HF peripheral clock divisor
 * @param CMU_HFPERCLKDIV_ constant
 *****************************************************************************/
void CMU_setHFPERCLKDiv(uint32_t div)
{
  CMU->HFPERCLKDIV = (div << _CMU_HFPERCLKDIV_HFPERCLKDIV_SHIFT);
}


/**************************************************************************//**
 * @brief Returns HF peripheral clock divisor
 * @return CMU_HFPERCLKDIV_HFCLK<0-512>
 *****************************************************************************/
uint32_t CMU_getHFPERCLKDiv(void)
{
  return(CMU->HFPERCLKDIV >> _CMU_HFPERCLKDIV_HFPERCLKDIV_SHIFT);
}
