/*----------------------------------------------------------------------------
 * Name:    ADC.c
 * Purpose: MCB1700 low level ADC functions
 * Version: V1.00
 * Note(s):
 *----------------------------------------------------------------------------
 * This file is part of the uVision/ARM development tools.
 * This software may only be used under the terms of a valid, current,
 * end user licence from KEIL for a compatible version of KEIL software
 * development tools. Nothing else gives you the right to use this software.
 *
 * This software is supplied "AS IS" without warranties of any kind.
 *
 * Copyright (c) 2009 Keil - An ARM Company. All rights reserved.
 *----------------------------------------------------------------------------
 * History:
 *          V1.00 Initial Version
 *----------------------------------------------------------------------------*/

#include <stdint.h>                           /* Include standard types */
#include <LPC23xx.H>                             /* LPC23xx definitions */
#include "ADC.h"

/*----------------------------------------------------------------------------
  initialize ADC Pins
 *----------------------------------------------------------------------------*/
void ADC_init (void) {

  PCONP        |= (1 << 12);                     /* Enable power to AD block */
  PINSEL1       = 0x00004000;                    /* AD0.0 pin function select */
  AD0CR         = 0x002E0301;                    /* Power up, PCLK/4, sel AD0.0 */
}


/*----------------------------------------------------------------------------
  start ADC Conversion
 *----------------------------------------------------------------------------*/
void ADC_startCnv (void) {
  AD0CR &= ~0x01000000;                          /* stop conversion  */
  AD0CR |=  0x01000000;                          /* start conversion */
}


/*----------------------------------------------------------------------------
  stop ADC Conversion
 *----------------------------------------------------------------------------*/
void ADC_stopCnv (void) {
  AD0CR &= ~0x01000000;                          /* stop conversion  */
}


/*----------------------------------------------------------------------------
  get converted ADC value
 *----------------------------------------------------------------------------*/
uint32_t ADC_getCnv (void) {
  uint32_t adGdr;

  while (!(AD0GDR & (1UL<<31)));                 /* Wait for Conversion end */
  adGdr = AD0GDR;

  return((adGdr >> 8) & ADC_VALUE_MAX);          /* read converted value */
}
