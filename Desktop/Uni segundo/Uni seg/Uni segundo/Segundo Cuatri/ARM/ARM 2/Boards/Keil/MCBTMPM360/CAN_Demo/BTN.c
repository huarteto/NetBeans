/*----------------------------------------------------------------------------
 * Name:    BTN.c
 * Purpose: Toshiba TMPM364 Button functions
 * Note(s):
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

#include <TMPM364.h>                    /* Toshiba TMPM364 definitions        */

#define  BTN_ALL   0xF0


/*----------------------------------------------------------------------------
 *  Initialize Buttons Pins
 *----------------------------------------------------------------------------*/
void BTN_Init (void) {
  TSB_PJ->FR2 &= ~BTN_ALL;              /* Set BTN pins function as "Port"    */
  TSB_PJ->PUP &= ~BTN_ALL;              /* Disable BTN pins pull ups          */
  TSB_PJ->IE  |=  BTN_ALL;              /* Enable  BTN pins as inputs         */
}


/*----------------------------------------------------------------------------
 *  Get status of Buttons
 *----------------------------------------------------------------------------*/
unsigned int BTN_Get (void) {
  return (~(TSB_PJ->DATA >> 4) & 0x0000000F);
}
