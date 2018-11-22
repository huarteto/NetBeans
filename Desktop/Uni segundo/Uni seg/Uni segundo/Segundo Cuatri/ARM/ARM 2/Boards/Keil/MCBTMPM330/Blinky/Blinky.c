/******************************************************************************/
/* Blinky.c: LED Flasher                                                      */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2009 Keil Software. All rights reserved.                     */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/
                  
#include <stdio.h>
#include "TMPM330.H"                    /* TMPM330 definitions                */



#define LED_NUM     3                           /* Number of user LEDs        */
#define LED_ENABLE  TSB_PI->CR   |=  0x07;      /* Enable LED pins as outputs */\
                    TSB_PI->DATA |=  0x07;      /* Turn all LEDs off          */
#define LED_ON(n)   TSB_PI->DATA &= ~LED[n];    /* Turn LED on                */
#define LED_OFF(n)  TSB_PI->DATA |=  LED[n];    /* Turn LED off               */
#define BTN_ENABLE  TSB_PH->PUP  |=  0x02;      /* Button PH1 enable          */
#define BTN         (!(TSB_PH->DATA & (1 << 1)))/* Button PH1 state           */


const int LED[] = { 0x01, 0x02, 0x04 };         /* LED pin mask               */


volatile unsigned short AD_Val;         /* AD Value                           */
volatile unsigned long  SysTickCnt;     /* SysTick Counter                    */



void SysTick_Handler (void) {           /* SysTick Interrupt Handler (1ms)    */
  SysTickCnt++;

  TSB_AD->MOD0 = 0x01;                  /* Start ADC Conversion               */
  while (!(TSB_AD->MOD0 & 0x80));       /* Wait until finished                */

  AD_Val = TSB_AD->REG4C >> 6;          /* Store ADC Value                    */
}

void Delay (unsigned long tick) {       /* Delay Function                     */
  unsigned long systickcnt;

  systickcnt = SysTickCnt;
  while ((SysTickCnt - systickcnt) < tick);
}





int main (void) {                       /* Main Program                       */
  int idx = -1, dir = 1;

  SysTick_Config(SystemCoreClock/1000); /* Generate interrupt each 1 ms   */

  /* Setup ADC */
  TSB_AD->MOD1 = 0x84;                  /* Input Channel: AIN4, VREF=ON       */

  /* Setup I/O */
  TSB_PH->PUP  = 0x02;                  /* PH.1 with enabled Pull-up          */
  TSB_PH->IE   = 0x02;                  /* PH.1 defined as Input (Push Button)*/

  BTN_ENABLE                            /* Enable BTN                         */
  LED_ENABLE                            /* Enable LEDs and turn off all LEDs  */
  
  
  printf("\n\nHello, World");
  
  
  for (;;) {                            /* Loop forever                       */
    if (!BTN) {                         /* If BTN Button not pressed          */
      Delay(10);                        /* Delay                              */
      
      /* Calculate 'idx': 0,1,...,LED_NUM-1,LED_NUM-1,...,1,0,0,...           */
      idx += dir;
      if (idx == LED_NUM) { dir = -1; idx =  LED_NUM-1; } 
      else if   (idx < 0) { dir =  1; idx =  0;         }

      LED_ON(idx)                       /* Turn on LED 'idx'                  */
      Delay(AD_Val+1);                  /* Delay                              */

      LED_OFF(idx)                      /* Turn off LED 'idx'                 */
      Delay(AD_Val+1);                  /* Delay                              */
    }
    else {
      printf("\n\nButton pressed...");  /* print message                      */
      while(BTN);                       /* wait for releasing button          */
      printf("\nButton released...");   /* print message                      */      
    }
  }
}




