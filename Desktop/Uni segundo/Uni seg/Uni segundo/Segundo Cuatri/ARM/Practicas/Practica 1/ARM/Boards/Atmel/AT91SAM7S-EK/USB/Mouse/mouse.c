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

#include <AT91SAM7S64.H>                    /* AT91SAM7S64 definitions */

#include "..\..\Board.h"

#include "type.h"

#include "usb.h"
#include "usbcfg.h"
#include "usbhw.h"
#include "usbcore.h"

#include "mouse.h"


AT91S_PIO * pPIO = AT91C_BASE_PIOA;         /* Global Pointer to PIO */


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

  key = pPIO->PIO_PDSR;                     /* Read Pin Data */
  if ((key & SW1) != 0) {                   /* Check if SW1 is not pressed */
    if ((key & SW4) == 0) {                 /* Check if SW4 is pressed */
      MouseInReport[0] = 0x01;              /* Left Click */
    } else {
      MouseInReport[0] = 0x00;              /* No Click */
    }
    switch (key & (SW2 | SW3)) {
      case SW3:                             /* SW2 pressed & SW3 not pressed */
        MouseInReport[1] = (BYTE)(-4);      /* -X: Left Movement */
        break;
      case SW2:                             /* SW3 pressed & SW2 not pressed */
        MouseInReport[1] = +4;              /* +X: Right Movement */
        break;
      default:
        MouseInReport[1] =  0;              /* No X Movement */
        break;
    }
    MouseInReport[2] = 0;                   /* No Y Movement */
  } else {                                  /* SW1 is pressed */
    if ((key & SW4) == 0) {                 /* Check if SW4 is pressed */
      MouseInReport[0] = 0x02;              /* Right Click */
    } else {
      MouseInReport[0] = 0x00;              /* No Click */
    }
    MouseInReport[1] = 0;                   /* No X Movement */
    switch (key & (SW2 | SW3)) {
      case SW3:                             /* SW2 pressed & SW3 not pressed */
        MouseInReport[2] = (BYTE)(-4);      /* -Y: Up Movement */
        break;
      case SW2:                             /* SW3 pressed & SW2 not pressed */
        MouseInReport[2] = +4;              /* +Y: Down Movement */
        break;
      default:
        MouseInReport[2] =  0;              /* No Y Movement */
        break;
    }
  }
  MouseInReport[3] = 0;                     /* Wheel not used */
}


/* Main Program */

int main (void) {
  DWORD n;

  /* Enable Clock for PIO */
  AT91C_BASE_PMC->PMC_PCER = (1 << AT91C_ID_PIOA);

  pPIO->PIO_PER  = LED_MASK;                /* Enable PIO for LED1..4  */
  pPIO->PIO_OER  = LED_MASK;                /* LED1..4 are Outputs     */
  pPIO->PIO_SODR = LED_MASK;                /* Turn off LED's ("1")    */

  USB_Init();                               /* USB Initialization */
  USB_Connect(TRUE);                        /* USB Connect */

  while (1) {                               /* Loop forever */
    n = pPIO->PIO_PDSR;                     /* Read Pin Data */
    if ((n & SW1) == 0) {                   /* Check if SW1 is pressed */
      pPIO->PIO_CODR = LED1;                /* Turn On LED1 */
    } else {
      pPIO->PIO_SODR = LED1;                /* Turn Off LED1 */
    }
    for (n = 0; n < 10000; n++);            /* Delay */
  }
}
