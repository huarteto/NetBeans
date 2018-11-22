/******************************************************************************/
/* IRQ.C: IRQ Module                                                          */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2010 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/

#include "IRQ.h"

void (*IRQ_ISR[IRQSIZE])(void);             /* IRQ Interrupt Service Routines */

