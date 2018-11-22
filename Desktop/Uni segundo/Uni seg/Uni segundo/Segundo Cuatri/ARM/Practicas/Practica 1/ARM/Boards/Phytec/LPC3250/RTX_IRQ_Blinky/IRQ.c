/******************************************************************************/
/* IRQ.c: IRQ Handlers                                                        */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2007-2009 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/

#include <RTL.h>
#include <LPC325x.H>


extern OS_TID t_taskB;

/* Milisecond Timer Interrupt Handler                                         */
void interrupt_mstim (void) __irq {
  MSTIM_INT = (1 <<  1);                /* Clear interrupt                    */
  MIC_RSR   = (1 << 27);
  isr_evt_set (0x0001, t_taskB);        /* Send event to taskB                */
}
