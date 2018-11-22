/******************************************************************************/
/* BLINKY.C: LED Flasher                                                      */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2007 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/
                  
#include <AS352x.H>                     /* AustriaMiroSystems AS352x registers*/

#define SPEED       64000000            /* Processor speed                    */
#define LED_NUM     2                   /* Number of user LEDs                */
#define LED_D1      (1 << 5)            /* LED D1 is connected to GPIO2 pin 5 */
#define LED_D2      (1 << 4)            /* LED D2 is connected to GPIO2 pin 4 */
#define LED_MSK     (LED_D1 | LED_D2)   /* Mask of all LEDs                   */

const int led_mask[] = {LED_D1, LED_D2};/* Status LEDs D1 and D2              */
unsigned int LEDCycles = 0.005 * SPEED; /* Cycles for LED blinking            */

void wait (void) {
  unsigned int n;

  n = LEDCycles;
  while (n--);
}


int main (void) {                       /* Main Function                      */

  int num      = -1; 
  int dir      =  1;

  Reg32(CGU_REG_PERI) |= (1 << 16);     /* Enable GPIO clock                  */

  Reg32(GPIO2_DIR) |= (0x03 << 4);      /* Setup LED pins as outputs          */
  Reg32(GPIO2_DATA + (LED_MSK << 2))  = ~(0x03 << 4); /* Turn off all LEDs    */

  for (;;) {                            /* Loop forever                       */

    /* Calculate 'num': 0, 1, ... , LED_NUM-1, LED_NUM-1, ... , 1, 0, 0, ...  */
    num += dir;
    if (num == LED_NUM) { dir = -1; num =  LED_NUM-1; } 
    else if   (num < 0) { dir =  1; num =  0;         }

    /* Turn  on LED with index 'num'                                          */
    Reg32(GPIO2_DATA + (LED_MSK << 2)) |= led_mask[num];
    wait();
    /* Turn off all LEDs                                                      */
    Reg32(GPIO2_DATA + (LED_MSK << 2))  = ~(0x03 << 4);
    wait();
  }
}
