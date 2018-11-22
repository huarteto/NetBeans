/*----------------------------------------------------------------------------
 * Name:    Timer.c
 * Purpose: Timer usage for STM32
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
 * Copyright (c) 2008 Keil - An ARM Company. All rights reserved.
 *----------------------------------------------------------------------------
 * History:
 *          V1.00 Initial Version
 *----------------------------------------------------------------------------*/

#include <stm32f10x_lib.h>                        // STM32F10x Library Definitions
#include "STM32_Init.h"                           // STM32 Initialization


unsigned short int  secCnt   = 0;                 // counts the seconds
unsigned short int msecCnt   = 0;                 // counts the milli seconds

unsigned short int DBG_secCnt;                   // counts the seconds
unsigned short int DBG_msecCnt;                  // counts the milli seconds

/*------------------------------------------------------------------------------
  Display value (0..9) on right segment
 *------------------------------------------------------------------------------*/
void displayRight (unsigned int val) {

  GPIOC->BRR = (1<<11);                                          // switch off PC11
  GPIOD->BRR = (1<<12)|(1<<11)|(1<<8)|(1<<7)|(1<<2);             // switch off PD12,PD11,PD8,PD7,PD2
  GPIOE->BRR = (1<<14);                                          // switch off PE14 

  switch (val) {
    case 0:
      GPIOC->BSRR = (1<<11);                                     // switch on  PC11
      GPIOD->BSRR = (1<<11)|(1<<8)|(1<<7)|(1<<2);                // switch on  PD11,PD8,PD7,PD2
      GPIOE->BSRR = (1<<14);                                     // switch on  PE14 
      break;
    case 1:
      GPIOD->BSRR = (1<<7)|(1<<2);                               // switch on  PD15,PD7,PD2
      break;
    case 2:
      GPIOC->BSRR = (1<<11);                                     // switch on  PC11
      GPIOD->BSRR = (1<<12)|(1<<8)|(1<<2);                       // switch on  PD12,PD8,PD2
      GPIOE->BSRR = (1<<14);                                     // switch on  PE14 
      break;
    case 3:
      GPIOC->BSRR = (1<<11);                                     // switch on  PC11
      GPIOD->BSRR = (1<<12)|(1<<8)|(1<<7)|(1<<2);                // switch on  PD12,PD8,PD7,PD2
      break;
    case 4:
      GPIOD->BSRR = (1<<12)|(1<<11)|(1<<7)|(1<<2);               // switch on  PD12,PD11,PD7,PD2
      break;
    case 5:
      GPIOC->BSRR = (1<<11);                                     // switch on  PC11
      GPIOD->BSRR = (1<<12)|(1<<11)|(1<<8)|(1<<7);               // switch on  PD12,PD11,PD8,PD7
      break;
    case 6:
      GPIOC->BSRR = (1<<11);                                     // switch on  PC11
      GPIOD->BSRR = (1<<12)|(1<<11)|(1<<8)|(1<<7);               // switch on  PD12,PD11,PD8,PD7
      GPIOE->BSRR = (1<<14);                                     // switch on  PE14 
      break;
    case 7:
      GPIOC->BSRR = (1<<11);                                     // switch on  PC11
      GPIOD->BSRR = (1<<7)|(1<<2);                               // switch on  PD7,PD2
      break;
    case 8:
      GPIOC->BSRR = (1<<11);                                     // switch on  PC11
      GPIOD->BSRR = (1<<12)|(1<<11)|(1<<8)|(1<<7)|(1<<2);        // switch on  PD12,PD11,PD8,PD7,PD2
      GPIOE->BSRR = (1<<14);                                     // switch on  PE14 
      break;
    case 9:
      GPIOC->BSRR = (1<<11);                                     // switch on  PC11
      GPIOD->BSRR = (1<<12)|(1<<11)|(1<<8)|(1<<7)|(1<<2);        // switch on  PD12,PD11,PD8,PD7,PD2
      break;
    default:
      GPIOC->BSRR = (1<<11);                                     // switch on  PC11
      GPIOD->BSRR = (1<<12)|(1<<11)|(1<<8);                      // switch on  PD12,PD11,PD8
      GPIOE->BSRR = (1<<14);                                     // switch on  PE14 
      break;
  } // end switch

}

/*------------------------------------------------------------------------------
  Display value (0..9) on left segment
 *------------------------------------------------------------------------------*/
void displayLeft (unsigned int val) {

  GPIOB->BRR = (1<<15)|(1<<12)|(1<<7)|(1<<6)|(1<<5);             // switch off PB15,PB12,PB7,PB6,PB5
  GPIOC->BRR = (1<<8)|(1<<7);                                    // switch off PC8,PC7 

  switch (val) {
    case 0:
//      GPIOB->BSRR = (1<<15)|(1<<12)|(1<<7)|(1<<6)|(1<<5);        // switch on  PB15,PB12,PB7,PB6,PB5
//      GPIOC->BSRR = (1<<7);                                      // switch on  PC7 
      break;
    case 1:
      GPIOB->BSRR = (1<<7)|(1<<6);                               // switch on  PB7,PB6
      break;
    case 2:
      GPIOB->BSRR = (1<<15)|(1<<12)|(1<<6)|(1<<5);               // switch on  PB15,PB12,PB6,PB5
      GPIOC->BSRR = (1<<8);                                      // switch on  PC8 
      break;
    case 3:
      GPIOB->BSRR = (1<<12)|(1<<7)|(1<<6)|(1<<5);                // switch on  PB12,PB7,PB6,PB5
      GPIOC->BSRR = (1<<8);                                      // switch on  PC8 
      break;
    case 4:
      GPIOB->BSRR = (1<<7)|(1<<6);                               // switch on  PB7,PB6
      GPIOC->BSRR = (1<<8)|(1<<7);                               // switch on  PC8,PC7 
      break;
    case 5:
      GPIOB->BSRR = (1<<12)|(1<<7)|(1<<5);                       // switch on  PB12,PB7,PB5
      GPIOC->BSRR = (1<<8)|(1<<7);                               // switch on  PC8,PC7 
      break;
    case 6:
      GPIOB->BSRR = (1<<15)|(1<<12)|(1<<7)|(1<<5);               // switch on  PB15,PB12,PB7,PB5
      GPIOC->BSRR = (1<<8)|(1<<7);                               // switch on  PC8,PC7 
      break;
    case 7:
      GPIOB->BSRR = (1<<7)|(1<<6)|(1<<5);                        // switch on  PB7,PB6,PB5
      break;
    case 8:
      GPIOB->BSRR = (1<<15)|(1<<12)|(1<<7)|(1<<6)|(1<<5);        // switch on  PB15,PB12,PB71,PB6,PB5
      GPIOC->BSRR = (1<<8)|(1<<7);                               // switch on  PC8,PC7 
      break;
    case 9:
      GPIOB->BSRR = (1<<12)|(1<<7)|(1<<6)|(1<<5);                // switch on  PB12,PB71,PB6,PB5
      GPIOC->BSRR = (1<<8)|(1<<7);                               // switch on  PC8,PC7 
      break;
    default:
      GPIOB->BSRR = (1<<15)|(1<<12)|(1<<5);                      // switch on  PB15,PB12,PB5
      GPIOC->BSRR = (1<<8)|(1<<7);                               // switch on  PC8,PC7 
      break;
  } // end switch

}

/*------------------------------------------------------------------------------
  Timer1 Update Interrupt Handler       (ticks every 10 msec)
 *------------------------------------------------------------------------------*/
void TIM1_UP_IRQHandler (void) {

  if ((TIM1->SR & 0x0001) != 0) {                 // check interrupt source

    msecCnt = msecCnt + 10;                       // count the milliSeconds
    DBG_msecCnt = msecCnt; 
    if (msecCnt == 1000) {
      msecCnt = 0;
       
      secCnt = secCnt + 1;                        // count the seconds
      DBG_secCnt = secCnt; 
      if (secCnt == 60) {
        secCnt = 0;
      }

      displayLeft  ((secCnt/10));                 // display seconds
      displayRight ((secCnt%10));
    }


    TIM1->SR &= ~(1<<0);                          // clear UIF flag
 }
} // end TIM1_UP_IRQHandler


/*----------------------------------------------------------------------------
  MAIN function
 *----------------------------------------------------------------------------*/
int main (void) {

  stm32_Init ();                                  // STM32 setup

  GPIOD->BSRR = (1<<15);                          // switch on  PD15 (right dot)
  while (1) {                                     // Loop forever
    ;
  } // end while
} // end main
