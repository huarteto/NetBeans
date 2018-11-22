/******************************************************************************/
/* ExtMem.c: Memory Test for Star9 Board                                      */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2007 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/

#include <91x_lib.h>
#include "memtest.h" 

#define LED_RED   0x04
#define LED_YEL   0x40

#define LED_On(led)         GPIO2->DR[led*4] = ~led
#define LED_Off(led)        GPIO2->DR[led*4] =  led
#define LED_out(mask, val)  GPIO2->DR[mask*4] = ~(val & mask)

#define EXTERNAL_RAM  0x38000000          /* bank 1 */
#define RAM_SIZE      (2 * 512 * 1024)    /* 2 chips a 512k * 8 */


void enable_LEDs (void) {
  /* init LEDs */
  SCU->GPIOOUT[2]   = 0x1010;             /* P2.2, P2.6 output - mode 1       */
  GPIO2->DDR        = 0x44;               /* P2.2, P2.6 Outputs (LED Data)    */
  GPIO2->DR[0x44*4] = 0x44;	              /* clear LEDs                       */
}

void enable_ExtMem (void) {
  /* Mux mode with 16-bit data, 24-bit address */
  /* GPIO P5.4..P5.7   EMI_CS[3:0]   */
  /* GPIO P7.0..7.7    EMI_A [23:16] */
  /* GPIO P9.0..9.7    EMI_AD[15:8]  */
  /* GPIO P8.0..8.7    EMI_AD[7:0]   */

  /* Enable P8, P9 for EMI function */
  SCU->GPIOEMI = 0x01;

  /* Enable GPIO 7 EMI_A (Alternate Output2)*/
  SCU->GPIOOUT[7]    =  0xAAAA;           /* P7.0..7 alternate output2 */
//  SCU->GPIOTYPE[7]   =  0x00;             /* P7.0..7 push pull */

  /* Enable GPIO5 CS Pins P5.7,P5.5, P5.6, P5.4  EMI ChipSelect (Alternate Output3)*/
  SCU->GPIOOUT[5]    =  0xFF00;           /* P5.7, P5.5, P5.6, P5.4 alternate output3 */
//  SCU->GPIOIN[5]    &= ~0xF0;             /* P5.7, P5.5, P5.6, P5.4 no input */
//  SCU->GPIOTYPE[5]  &= ~0xF0;             /* P5.7, P5.5, P5.6, P5.4 push pull */

  /* 16bit memory width bank0, bank1, bank2 */
// do not configure Bank0 if you are running out of ext flash !
//  EMI_Bank0->BCR  &= ~0x0030;             /* keep other bits */
//  EMI_Bank0->BCR  |=  0x0010;             /* set 16 bit memory width */
  EMI_Bank1->BCR  &= ~0x0030;             /* keep other bits */
  EMI_Bank1->BCR  |=  0x0010;             /* set 16 bit memory width */

}

int memTest(datum * baseAddress, unsigned long numBytes) {

  if ((memTestDataBus(baseAddress)              != 0)    ||
      (memTestAddressBus(baseAddress, numBytes) != NULL) ||
      (memTestDevice(baseAddress, numBytes)     != NULL)   ) {
    return (-1);
  }
  else {
    return (0);
  }
		
}   /* memTest() */


int main (void) {

  enable_LEDs ();
  enable_ExtMem();

  if (memTest ((datum *)EXTERNAL_RAM, RAM_SIZE) != 0) {
    LED_On(LED_RED);                      /* a RAM test failed */
  }
  else {
    LED_On(LED_YEL);                      /* all RAM tests successful */
  }

  while (1) {                             /* Loop forever */
    ;
  }

}
