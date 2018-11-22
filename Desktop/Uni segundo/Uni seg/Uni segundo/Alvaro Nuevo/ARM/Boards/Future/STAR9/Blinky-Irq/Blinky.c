/******************************************************************************/
/* BLINKY.C: LED Flasher for Star9 Board                                      */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2007 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/

#include <91x_lib.h> 

#define LED_RED   0x04
#define LED_YEL   0x40

#define LED_On(led)         GPIO2->DR[led*4]  = ~led
#define LED_Off(led)        GPIO2->DR[led*4]  =  led
#define LED_out(mask, val)  GPIO2->DR[mask*4] = ~(val & mask)

 /* Blink the LEDs on Star9 board */
 const unsigned char led_val[4] = { 0x04,0x44,0x40,0x00 };
 static unsigned int led_cnt = 0;

__irq void TIM3_IRQ_Handler (void) {    /* TIM3 timer interrupt routine       */


  LED_out (0x44, led_val[led_cnt]);
  if (++led_cnt >= sizeof(led_val)) {
    led_cnt = 0;
  }

  TIM3->SR &= ~0x2000;                  /* Clear Timer Overflow interrupt flag*/

  VIC0->VAR = 0;                        /* Acknowledge Interrupt              */  
  VIC1->VAR = 0;
}



int main (void) {

  /* init LEDs */
  SCU->GPIOOUT[2]   = 0x1010;             /* P2.2, P2.6 output - mode 1       */
  GPIO2->DDR        = 0x44;               /* P2.2, P2.6 Outputs (LED Data)    */
  GPIO2->DR[0x44*4] = 0x44;	              /* clear LEDs                       */

  /* Configure and enable IRQ for Timer (TIM3)                                */
  VIC0->VAiR[7]   = (unsigned int)TIM3_IRQ_Handler;/* Setup TIM3 IRQ Hndl addr*/
  VIC0->VCiR[7]  |= 0x20;                 /* Enable the vector interrupt      */
  VIC0->VCiR[7]  |= 7;                    /* Specify the interrupt number     */
  VIC0->INTER    |= (1<<7);               /* Enable TIM3 interrupt            */

  /* Timer 3 Configuration (TIM3)                                             */
  TIM3->CNTR      = 0x0000;               /* Setup TIM3 counter register      */
  TIM3->CR2      &= 0xFF00;               /* Clear prescaler value            */
  TIM3->CR2      |= 0x00FF;               /* Setup TIM3 prescaler             */
  TIM3->CR2      |= 0x2000;               /* TIM3 timer overflow intrupt en   */
  TIM3->CR1      |= 0x8000;               /* TIM3 counter enable              */

  while (1) {                             /* Loop forever                     */
    ;
  }
}
