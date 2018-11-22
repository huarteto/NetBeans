/******************************************************************************/
/* PWM.c: Timer PWM generation                                                */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2008 Keil Software. All rights reserved.                     */
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


// Port H function Register 1
#define TB0IN0      0

// TMRBn enable register (channels 0 through 9)
#define TBnEN       7

// TMRB RUN register (channels 0 through 9)
#define TBnRUN      0
#define TBnPRUN     2

// TMRB control register (channels 0 through 9)
#define I2TBn       3
#define TBnSYNC     5
#define TBnWBF      7

// TMRB mode register (channels 0 thorough 9)
#define TBnCLK      0
  #define TBnCLK_IN0  0x00
  #define TBnCLK_T1   0x01
  #define TBnCLK_T4   0x02
  #define TBnCLK_T16  0x03
  
#define TBnCLE      2
#define TBnCPM      3
  #define TBnCPM_DIS          0x00
  #define TBnCPM_IN0U_IN1U    0x01
  #define TBnCPM_IN0UD        0x02
  #define TBnCPM_CAPTRUD      0x03
  
#define TBnCP0      5

// TMRB flip-flop control register (channels 0 through 9)
#define TBnFF0C     0
    #define TBnFF0C_INV     0x00
    #define TBnFF0C_SET     0x01
    #define TBnFF0C_CLR     0x02
    #define TBnFF0C_DC      0x03
    
#define TBnE0T1     2
#define TBnE1T1     3
#define TBnC0T1     4
#define TBnC1T1     5



const int LED[] = { 0x01, 0x02, 0x04 };     /* LED pin mask                       */


volatile unsigned short AD_Val;             /* AD Value                           */
volatile unsigned int   cycle;
volatile unsigned long  SysTickCnt;         /* SysTick Counter                    */

void SysTick_Handler (void) {               /* SysTick Interrupt Handler (1ms)    */
  SysTickCnt++;

  TSB_AD->MOD0 = 0x01;                      /* Start ADC Conversion               */
  while (!(TSB_AD->MOD0 & 0x80));           /* Wait until finished                */

  AD_Val = TSB_AD->REG4C >> 6;              /* Store ADC Value                    */
  
  // Set PWM on AD Value (0..1023)
  if(AD_Val < cycle) TSB_TB0->RG0 = AD_Val+1;   /* Specifies a duty. (16 bits *32-bits for register) */
  
  // print AD Value to ITM
  printf("\rDuty: %4i   Cycle: %4i",  AD_Val, cycle);
}


void Delay (unsigned long tick) {           /* Delay Function                     */
  unsigned long systickcnt;

  systickcnt = SysTickCnt;
  while ((SysTickCnt - systickcnt) < tick);
}



// Initialize Timer 0 
void InitTB0(void)
{
  TSB_TB0->EN   =   (1<<TBnEN);
  TSB_TB0->RUN &= ~((1<<TBnRUN) | (1<<TBnPRUN));
      
  TSB_TB0->MOD  =   (TBnCLK_T4 << TBnCLK) | (1<<TBnCP0) | (TBnCPM_IN0UD<<TBnCPM);
  TSB_TB0->CR   = 0x80;
  TSB_TB0->FFCR = 0x0e;
  TSB_TB0->MOD  = 0x25;
  
  TSB_PI->CR    = 0x01;
  TSB_PI->FR1   = 0x01;
  
  TSB_TB0->RG0  = 512;
  TSB_TB0->RG1  = 1023;
  cycle         = 1023;
  
  TSB_TB0->RUN |= (1<<TBnRUN) | (1<<TBnPRUN);
}






int main (void) {                           /* Main Program                       */

  SysTick_Config(SystemCoreClock/100); /* Generate interrupt each 1 ms       */
  InitTB0();
  
   /* Setup ADC */
  TSB_AD->MOD1 = 0x84;                      /* Input Channel: AIN4, VREF=ON       */

  /* Setup I/O */
  TSB_PH->PUP  = 0x02;                      /* PH.1 with enabled Pull-up          */
  TSB_PH->IE   = 0x02;                      /* PH.1 defined as Input (Push Button)*/

  BTN_ENABLE                                /* Enable BTN                         */
  LED_ENABLE                                /* Enable LEDs and turn off all LEDs  */
  
  printf("\nHello, World\n");  
  
  for (;;) {                                /* Loop forever                       */
   if (BTN) {                               /* If BTN Button not pressed          */
      Delay(10);                            /* Delay for debouncing button        */
      cycle = AD_Val;
      TSB_TB0->RG1 = cycle+1;               /* Specifies a cycle.                 */
    } 
  }
}




void NMI_Handler(void)
{
  printf("\nWhoops... NMI occured!\n");
}


