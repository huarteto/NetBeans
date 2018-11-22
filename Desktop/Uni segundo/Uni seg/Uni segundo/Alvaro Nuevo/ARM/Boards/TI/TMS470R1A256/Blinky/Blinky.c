/******************************************************************************/
/* Blinky.c: LED Flasher                                                      */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2008 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/
                  
#include <TMS470R1.h>                   /* TMS470R1 definitions               */
#ifdef RT_AGENT
#include <stdio.h>                      /* Standard I/O definitions           */
#include <RT_Agent.h>                   /* RT Agent definitions               */
#endif

const int LED_TABLE[] = {0x00000001, 0x00000004, 0x00000010, 0x00000040,
                         0x00000080, 0x00000100, 0x00000400, 0x00000800,
                         0x00001000, 0x00002000, 0x00040000, 0x00080000,
                         0x00100000, 0x00200000, 0x01000000, 0x80000000 };


 /* LED_reroute --------------------------------------------------------------\
|                                                                              |
|  Make a output value according to input value of LEDs states (LEDs are not   |
|  sequential on the port pins                                                 |
 \---------------------------------------------------------------------------*/

unsigned int LED_reroute (unsigned int LED_states)  {
  unsigned int i, j;
  unsigned int temp = 0;

  j = 0;
  for (i = 0x0001; i <= 0x8000; i <<= 1)  {
    if (LED_states & i)
      temp |= LED_TABLE[j];
    j++;
  } 

  return (~temp);
}


/* Delay Function ------------------------------------------------------------*/

void delay (int cnt)  {
  while (cnt--);
}


 /* Setup --------------------------------------------------------------------\
|                                                                              |
|  This function setups the HETs as outputs for driving LEDs and setups RTI    |
|  interrupt, it is implemented through SWI call because it is required that   |
|  setting be done in Supervisory Mode of ARM                                  |
|  Parameters:                                                                 |
|  Return value:                                                               |
|                                                                              |
 \---------------------------------------------------------------------------*/

void __swi(8) Setup (void);
void __SWI_8      (void) {

  pHET->HETDIR = 0xFFFFFFFF;            /* Setup HETs as outputs (for LED)    */

#ifdef RT_AGENT
  /* Setup RTI timer interrupt, every 250us                                   */
  pSM->RTIPCTL       = (7 << 11) | (1000 - 1);
  pSM->RTICMP2       = 12;
  pSM->RTICINT       = 0;
  pSM->RTICINT       = (1 << 4);
  pSM->RTICNTR       = 0;
  pSM->REQMASK      |= (1 << 1);
#endif
}


 /* HET_LED ------------------------------------------------------------------\
|                                                                              |
|  Set values for LEDs                                                         |
|  Parameters:      val  - LEDs states requested                               |
|  Return value:                                                               |
|                                                                              |
 \---------------------------------------------------------------------------*/

void __swi(9) HET_LED (unsigned int val);
void __SWI_9      (unsigned int val) {

  pHET->HETDOUT = LED_reroute(val);
}

 /* IRQ_Handler --------------------------------------------------------------\
|                                                                              |
|  This function is a common interrupt handler which handles all interrupt     |
|  requests                                                                    |
|  Parameters:                                                                 |
|  Return value:                                                               |
|                                                                              |
 \---------------------------------------------------------------------------*/

__irq void IRQ_Handler (void) {

  switch ((pSM->IRQIVEC & 0xFF) - 1)    /* Identify IRQ request               */
  {
    case 1:                             /* RTI timer interrupt request        */
#ifdef RT_AGENT
      RTA_rx_word_ext();
      RTA_tx_word_ext();
#endif
      pSM->RTICNTR  = 0;
      pSM->RTICINT &= ~(1 << 6);
      pSM->INTREQ  &= ~(1 << 1);
      break;
  }
}


/* Function for Real-Time Agent                                               */
void RTA_irq_enable_ext (void) {
  pSM->REQMASK |=  (1 << 1);            /* Enable RTI Interrupt               */  
}


/* Function for Real-Time Agent                                               */
void RTA_irq_disable_ext (void) {
  pSM->REQMASK &= ~(1 << 1);            /* Disable RTI Interrupt              */  
}


/******************************************************************************
* Main Program                                                                *
*******************************************************************************/

int main (void) {
  unsigned int n, k;
#ifdef RT_AGENT
  unsigned cnt = 0;
  RTA_Init();
#endif

  Setup ();                             /* Setup HETs and interrupts          */
 
  while (1) {                           /* Loop forever                       */
    /* Shift LEDs and turn them ON according to state                         */
    for (n = 0x0011; n <= 0x00088000; n <<= 1)  {
      k = (n >> 16) | n;
      HET_LED (k);                      /* Activate LEDs                      */
      delay(1000000);                   /* Delay                              */
    }
#ifdef RT_AGENT
    printf("Completed %d blinkies...\n", ++cnt);
#endif
  }
}
