/*----------------------------------------------------------------------------
 *      Name:    MOUSE.C
 *      Purpose: USB Mouse Demo
 *      Version: V1.10
 *----------------------------------------------------------------------------
 *      This file is part of the uVision/ARM development tools.
 *      Copyright (c) 2005-2006 Keil Software. All rights reserved.
 *      This software may only be used under the terms of a valid, current,
 *      end user licence from KEIL for a compatible version of KEIL software
 *      development tools. Nothing else gives you the right to use it.
 *---------------------------------------------------------------------------*/

#include <AT91SAM7A3.H>                     /* AT91SAM7A3 definitions */

#include "..\..\Board.h"

#include "type.h"

#include "usb.h"
#include "usbcfg.h"
#include "usbhw.h"
#include "usbcore.h"

#include "mouse.h"


AT91S_PIO * pPIOA = AT91C_BASE_PIOA;        /* Global Pointer to PIOA */
AT91S_PIO * pPIOB = AT91C_BASE_PIOB;        /* Global Pointer to PIOB */


BYTE MouseInReport[4];                      /* Mouse Input Report */
                                            /*   Byte0.0: 1st Button (Left)  */
                                            /*   Byte0.1: 2nd Button (Right) */
                                            /*   Byte0.2: 3rd Button         */
                                            /*   Byte1: Relative X Pos       */
                                            /*   Byte2: Relative Y Pos       */
                                            /*   Byte3: Relative Wheel Pos   */

/*
 *  Get Mouse Input Report -> MouseInReport
 *    Parameters:      None
 *    Return Value:    None
 */

void GetMouseInReport (void) {
  DWORD key;

  MouseInReport[0] = 0x00;                  /* Default: No Click */
  MouseInReport[1] = 0;                     /* Default: No X Movement */
  MouseInReport[2] = 0;                     /* Default: No Y Movement */
  MouseInReport[3] = 0;                     /* Wheel not used */

  key = pPIOB->PIO_PDSR;                    /* Read Pin Data */

  if ((key & JS_LEFT) == 0) {               /* Check if Left is pressed */
    MouseInReport[1] = (BYTE)(-4);          /* -X: Left Movement */
  }
  if ((key & JS_RIGHT) == 0) {              /* Check if Left is pressed */
    MouseInReport[1] = +4;                  /* +X: Right Movement */
  }
  if ((key & JS_UP) == 0) {                 /* Check if Up is pressed */
    MouseInReport[2] = (BYTE)(-4);          /* -Y: Up Movement */
  }
  if ((key & JS_DOWN) == 0) {               /* Check if Down is pressed */
    MouseInReport[2] = +4;                  /* +Y: Down Movement */
  }
  if ((key & JS_PUSH) == 0) {               /* Check if Push is pressed */
    MouseInReport[0] = 0x01;                /* Left Click */
  }
}


/* Main Program */

int main (void) {

  *AT91C_PMC_PCER = (1 << AT91C_ID_PIOA) |  /* Enable Clock for PIOA */
                    (1 << AT91C_ID_PIOB);   /* Enable Clock for PIOB */

  pPIOA->PIO_PER  = LED_MASK;               /* Enable PIO for LED's  */
  pPIOA->PIO_OER  = LED_MASK;               /* LED's are Outputs     */
  pPIOA->PIO_SODR = LED_MASK;               /* Turn Off LED's ("1")  */

  USB_Init();                               /* USB Initialization */
  USB_Connect(TRUE);                        /* USB Connect */

  while (1);                                /* Loop forever */
}
