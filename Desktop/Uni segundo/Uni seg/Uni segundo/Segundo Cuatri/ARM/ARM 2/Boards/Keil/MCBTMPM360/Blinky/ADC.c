/*----------------------------------------------------------------------------
 * Name:    ADC.c
 * Purpose: Toshiba TMPM364 ADC functions
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


/*----------------------------------------------------------------------------
 *  Initialize ADC Pins
 *----------------------------------------------------------------------------*/
void ADC_Init (void) {
  /* Setup analog input pin.                                                  */
  TSB_PJ->PUP  &= ~0x01;                /* Disable Pull-up for PJ0            */
  TSB_PJ->IE   &= ~0x01;                /* Set PJ0 as Analog input pin        */

  /* Setup A/D converter                                                      */
  TSB_AD->CLK   =  0x81;                /* ADCLK = fc/2, 8 clocks for conv    */
  TSB_AD->MOD1  =  0x80;                /* Input Channel: AIN0, VREF=ON       */
}


/*----------------------------------------------------------------------------
 *  Get AD Conversion
 *----------------------------------------------------------------------------*/
unsigned int ADC_Get (void) {
  TSB_AD->MOD0 =  0x01;                 /* Start ADC conversion               */
  while (!(TSB_AD->MOD0 & 0x80));       /* Wait until finished                */
  return(TSB_AD->REG08 >> 6);           /* Return converted value             */
}
