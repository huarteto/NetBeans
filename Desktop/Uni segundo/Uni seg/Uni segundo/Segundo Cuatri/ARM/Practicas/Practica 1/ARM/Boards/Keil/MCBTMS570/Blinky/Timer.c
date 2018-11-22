/******************************************************************************/
/* Timer.c: Low Level RTI Timer Routines                                        */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2010 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/

#include <TMS570.h>

#define pRTI      ((RTI_ST     *)RTI)
#define pVIM      ((VIM_ST     *)VIM)
#define pVIM_RAM  ((VIM_RAM_ST *)VIM_RAM)

#define RTICLK 80000000UL
#define RTICMP0 2

extern void RTICMP0_IRQHandler(void);

/*----------------------------------------------------------------------------
  
 *----------------------------------------------------------------------------*/
void TIM_init (void) {


  pVIM_RAM->ISR[RTICMP0+1] = (FuncPTR)RTICMP0_IRQHandler;
  pVIM->REQMASKSET0_UL = (1UL << RTICMP0);

  pRTI->RTICPUC1_UL = (RTICLK / 100UL) - 1;                     /* 10ms tick */
  pRTI->RTICOMP0_UL = 1;
  pRTI->RTIUDCP0_UL = 1;
  pRTI->RTICOMPCTRL_UN.RTICOMPCTRL_ST.COMPSEL0_B1 = 1;
  pRTI->RTISETINT_UN.RTISETINT_ST.SETINT0_B1      = 1;

  pRTI->RTIGCTRL_UN.RTIGCTRL_ST.CNT1EN_B1         = 1;         /* enable CNT1 */
}


unsigned int TIM_rdFRC1 (void) {
  unsigned int val;

  val = pRTI->RTIFRC1_UL;

  return (val);
}


