/*----------------------------------------------------------------------------
 * Name:    BTN.c
 * Purpose: Toshiba TMPM395 Button functions
 *----------------------------------------------------------------------------
 * This file is part of the uVision/ARM development tools.
 * This software may only be used under the terms of a valid, current,
 * end user licence from KEIL for a compatible version of KEIL software
 * development tools. Nothing else gives you the right to use this software.
 *
 * This software is supplied "AS IS" without warranties of any kind.
 *
 * Copyright (c) 2010 Keil - An ARM Company. All rights reserved.
 *----------------------------------------------------------------------------*/

#include "TMPM395.h"                    /* TMPM395 definitions                */
#include "BTN.h"                        /* Buttons definitions                */

#define BTN_PB5    (1UL << 5)           /* Button PB.5                        */
#define BTN_PB7    (1UL << 7)           /* Button PB.7                        */
#define BTN_ALL    (BTN_PB7 | BTN_PB5) 
const unsigned long btn_mask[] = { BTN_PB7, BTN_PB5 };


/*----------------------------------------------------------------------------
 *  Initialize Buttons Pins
 *----------------------------------------------------------------------------*/
void BTN_Init (void) {
//  TSB_PB->FR1 &= ~BTN_ALL;              /* Set BTN pins function as "Port"    */
//  TSB_PB->FR2 &= ~BTN_ALL;              /* Set BTN pins function as "Port"    */
  TSB_PB->PUP &= ~BTN_ALL;              /* Disable BTN pins pull ups          */
  TSB_PB->IE  |=  BTN_ALL;              /* Enable  BTN pins as inputs         */
}


/*----------------------------------------------------------------------------
 *  Get status of Buttons
 *----------------------------------------------------------------------------*/
uint32_t BTN_Get (void) {
  int i;
  uint32_t btn     =  0;
  uint32_t btnData = (~(TSB_PB->DATA) & BTN_ALL);

  for (i = 0; i < BTN_NUM; i++) {
    if (btnData & (btn_mask[i])) {
      btn |= (1<<i);
    }
  }

  return (btn);
}
