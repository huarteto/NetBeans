/******************************************************************************/
/* BLINKY.C: LED Flasher                                                      */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2007 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/
                  
#include <stdio.h>
#include <LPC23xx.H>                    /* LPC23xx/LPC24xx definitions        */

#define I2C_AA      0x00000004
#define I2C_SI      0x00000008
#define I2C_STO     0x00000010
#define I2C_STA     0x00000020
#define I2C_I2EN    0x00000040


/* Import external IRQ handlers from IRQ.c file                               */
extern __irq void T0_IRQHandler  (void);
extern __irq void ADC_IRQHandler (void);

/* Import external functions from Serial.c file                               */
extern       void init_serial    (void);

/* Import external variables from IRQ.c file                                  */
extern unsigned int leds;
extern short AD_last;
extern unsigned char clock_1s;


/* Function that initializes LEDs                                             */
void LED_Init (void) {
  PCONP    |=  (1 << 7);                /* Enable clock for I2C0              */

  /* Initialize pins for SDA (P0.27) and SCL (P0.28) functions                */
  PINSEL1  &= ~0x03C00000;
  PINSEL1  |=  0x01400000;

  /* Clear I2C state machine                                                  */
  I20CONCLR =  I2C_AA | I2C_SI | I2C_STA | I2C_I2EN;

  /* Setup I2C clock speed                                                    */
  I20SCLL   =  0x80;
  I20SCLH   =  0x80;

  I20CONSET =  I2C_I2EN;
  I20CONSET =  I2C_STO;
}


/* Function that outputs value to LEDs                                        */
void LED_Out (unsigned int value) {
  unsigned int val = value;

  I20CONCLR =  I2C_AA | I2C_SI | I2C_STA | I2C_STO;
  I20CONSET =  I2C_STA;
  while (!(I20CONSET & I2C_SI));        /* Wait for START                     */
  I20DAT    =  0xC0;
  I20CONCLR =  I2C_SI | I2C_STA;
  while (!(I20CONSET & I2C_SI));        /* Wait for ADDRESS send              */
  I20DAT    =  0x18;
  I20CONCLR =  I2C_SI;
  while (!(I20CONSET & I2C_SI));        /* Wait for DATA send                 */
  I20DAT    =  (0x01 * ((val >> 0) & 1)) |
               (0x04 * ((val >> 1) & 1)) |
               (0x10 * ((val >> 2) & 1)) |
               (0x40 * ((val >> 3) & 1)) ;
  I20CONCLR =  I2C_SI;
  while (!(I20CONSET & I2C_SI));        /* Wait for DATA send                 */
  I20CONSET =  I2C_STO;
  I20CONCLR =  I2C_SI;
  while (I20CONSET & I2C_STO);          /* Wait for STOP                      */
}


/* Delay dependant upon AD conversion                                         */
void delay (void) {                     /* Delay function */
  unsigned int cnt;
  unsigned int val = AD_last;

  val = ((val >> 2) << 12) + 100000;    /* Adjust Delay Value */

  for (cnt = 0; cnt < val; cnt++);      /* Delay */
}


int main (void) {
  unsigned int n;
  short AD_print;

  LED_Init();                                  /* LED Initialization          */

  /* Enable and setup timer interrupt, start timer                            */
  PCONP        |= (1 << 1);                    /* Enable power to TIM block   */
  T0MR0         = 18000-1;                     /* 1msec = 18000-1 at 18 MHz   */
  T0MCR         = 3;                           /* Interrupt and Reset on MR0  */
  T0TCR         = 1;                           /* Timer0 Enable               */
  VICVectAddr4  = (unsigned long)T0_IRQHandler;/* Set Interrupt Vector        */
  VICVectCntl4  = 15;                          /* use it for Timer0 Interrupt */
  VICIntEnable  = (1  << 4);                   /* Enable Timer0 Interrupt     */

  /* Power enable, Setup pin, enable and setup AD converter interrupt         */
  PCONP        |= (1 << 12);                   /* Enable power to AD block    */
  PINSEL1      &=~0xC000;                      /* AD0.0 pin function select   */
  PINSEL1      |= 0x4000;
  AD0INTEN      = (1 <<  0);                   /* CH0 enable interrupt        */
  AD0CR         = 0x00200301;                  /* Power up, PCLK/4, sel AD0.0 */
  VICVectAddr18 = (unsigned long)ADC_IRQHandler;/* Set Interrupt Vector       */
  VICVectCntl18 = 14;                          /* use it for ADC Interrupt    */
  VICIntEnable  = (1  << 18);                  /* Enable ADC Interrupt        */

  init_serial();                               /* Init UART                   */

  while (1) {                                  /* Loop forever                */
    for (n = 0x00000001; n <= 0x00000010; n <<= 1) {
      /* Blink LED 0, 1, 2, 3                                                 */
      LED_Out(n);
      delay();                                 /* Delay                       */
    }
    if (clock_1s) {
      clock_1s = 0;
      AD_print = AD_last;
      printf ("AD value = 0x%03X\n\r", AD_print);
    }
  }
}
