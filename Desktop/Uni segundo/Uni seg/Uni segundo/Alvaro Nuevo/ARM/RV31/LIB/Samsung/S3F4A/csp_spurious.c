/******************************************************************************/
/* CSP_SPURIOUS.C: Spurious Interrupt Handler                                 */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2007 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/

#include "csp.h"


__irq void CSP_GICSpuriousIntHandler (void){

  CSP_GIC_SET_EOICR(GIC, 0);            /* End Interrupt                      */
}
