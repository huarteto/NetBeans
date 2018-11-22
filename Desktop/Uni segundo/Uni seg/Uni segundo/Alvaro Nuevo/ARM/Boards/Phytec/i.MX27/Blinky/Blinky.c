/******************************************************************************/
/* Blinky.c: LED Flasher                                                      */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2008 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/


#include <iMX27.h>                      /* i.MX27 definitions                 */

#define LED         (1 << 18)           /* LED is connected to PE18           */

 
volatile char Clock1s;
volatile long TimeTick;


void wait (void) {                      /* Wait 1 second                      */
  while (!Clock1s);
  Clock1s = 0;
}

__irq void IRQ_Handler (void)  {        /* Global interrupt handler           */

  switch (AITC_NIPNDL) {
    case (1 <<  2):
      if (TimeTick++ >= 99) {
        Clock1s  = 1;
        TimeTick = 0;
      }
      GPT6_TSTAT = 1; 
      break;
  }
}

int main (void)  {

  /* Setup GPIO PE18 as output for driving LED, and turn LED off              */
  PLLCLK_PCCR0  |=  1 << 25;
  GPIO_PTE_DDIR |=  LED;
  GPIO_PTE_GPR  &= ~LED;
  GPIO_PTE_OCR2 |=  3 <<  4;
  GPIO_PTE_GIUS |=  LED;
  GPIO_PTE_DR   &= ~LED;

  /* Setup and enable General Purpose Timer 6 interrupt generation every 10 ms*/
  PLLCLK_PCCR1  |=  1 << 10;            /* PERCLK1 Clock Enable               */
  PLLCLK_PCDR1   =  (PLLCLK_PCDR1 & ~0xFF)|(19-1);/* Set PCLK1 divider (19)   */ 
  PLLCLK_PCCR0  |=  1 << 19;            /* GPT6 Clock Enable                  */
  GPT6_TCTL      =  (1 << 10) |         /* Clear counter                      */
                    (1 <<  4) |         /* Compare Interrupt Enable           */
                    (1 <<  1) ;         /* Clock Source is PERCLK1            */
  GPT6_TPRER     =  0;                  /* GPT6 Clock Prescaler = 1           */
  GPT6_TCMP      =  ((266000000/19)/100)-1;       /* Compare period for 10 ms */
  GPT6_TCTL     |=  1;                  /* GPT6 Enable                        */
  AITC_INTENNUM  =  2;                  /* GPT6 Interrupt Enable              */

  for (;;)  {                           /* Loop forever                       */
    GPIO_PTE_DR |=  LED;                /* Turn LED on                        */
    wait();
    GPIO_PTE_DR &= ~LED;                /* Turn LED off                       */
    wait();
  }
}
