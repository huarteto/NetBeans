/*----------------------------------------------------------------------------
 * Name:    memory.c
 * Purpose: USB Memory Storage Demo
 * Version: V1.20
 *----------------------------------------------------------------------------
 * This file is part of the uVision/ARM development tools.
 * This software may only be used under the terms of a valid, current,
 * end user licence from KEIL for a compatible version of KEIL software
 * development tools. Nothing else gives you the right to use this software.
 *
 * This software is supplied "AS IS" without warranties of any kind.
 *
 * Copyright (c) 2008 Keil - An ARM Company. All rights reserved.
 *----------------------------------------------------------------------------*/

#include <LPC23xx.H>                        /* LPC23xx/LPC24xx definitions */

#include "type.h"

#include "usb.h"
#include "usbcfg.h"
#include "usbhw.h"
#include "mscuser.h"

#include "memory.h"


/* I2C definitions */
#define I2C_AA      0x00000004              /* I2C Assert acknowledge flag */
#define I2C_SI      0x00000008              /* I2C interrupt flag */
#define I2C_STO     0x00000010              /* I2C STOP flag */
#define I2C_STA     0x00000020              /* I2C START flag */
#define I2C_I2EN    0x00000040              /* I2C enable */

unsigned int LED_State = 0;                 /* reflect the LEDs */

extern U8 Memory[MSC_MemorySize];           /* MSC Memory in RAM */


/*------------------------------------------------------------------------------
  Initialize the I2C interface
 *------------------------------------------------------------------------------*/
void I2C_Init (void) {
  PCONP    |=  (1 << 7);                    /* Enable clock for I2C0 */

  PINSEL1  &= ~0x03C00000;                  /* P0.27 SDA, P0.28 SCL */
  PINSEL1  |=  0x01400000;                  /* PINSEL1 22.23, 24.25 */
  
  I20CONCLR =  I2C_AA  | I2C_SI | 
               I2C_STA | I2C_I2EN;          /* Clear I2C state machine */

  I20SCLL   =  0x80;                        /* Setup I2C clock speed */
  I20SCLH   =  0x80;

  I20CONSET =  I2C_I2EN;
  I20CONSET =  I2C_STO;
}


/*------------------------------------------------------------------------------
  Set value to LEDs
 *------------------------------------------------------------------------------*/
void LED_Set (unsigned int value) {
  unsigned int val = (value & LED_MSK);     /* only 4 bits are used */

  I20CONCLR =  I2C_AA | I2C_SI | I2C_STA | I2C_STO;
  I20CONSET =  I2C_STA;
  while (!(I20CONSET & I2C_SI));            /* Wait for START */
  I20DAT    =  0xC0;                        /* write to device 0xC0 */
  I20CONCLR =  I2C_SI | I2C_STA;
  while (!(I20CONSET & I2C_SI));            /* Wait for ADDRESS send */
  I20DAT    =  0x18;                        /* select LS2 register */
  I20CONCLR =  I2C_SI;
  while (!(I20CONSET & I2C_SI));            /* Wait for DATA send */
  I20DAT    =  (0x01 * ((val >> 0) & 1)) |	/* set data */
               (0x04 * ((val >> 1) & 1)) |
               (0x10 * ((val >> 2) & 1)) |
               (0x40 * ((val >> 3) & 1)) ;
	I20CONCLR =  I2C_SI;
  while (!(I20CONSET & I2C_SI));            /* Wait for DATA send */
  I20CONSET =  I2C_STO;
  I20CONCLR =  I2C_SI;
  while (I20CONSET & I2C_STO);              /* Wait for STOP */
}


/*------------------------------------------------------------------------------
  Switch on LEDs
 *------------------------------------------------------------------------------*/
void LED_On (unsigned int value) {

  LED_State |= value;
  LED_Set(LED_State);
}


/*------------------------------------------------------------------------------
  Switch off LEDs
 *------------------------------------------------------------------------------*/
void LED_Off (unsigned int value) {

  LED_State &= ~value;
  LED_Set(LED_State);
}


/*------------------------------------------------------------------------------
  Main Program
 *------------------------------------------------------------------------------*/
int main (void) {
  U32 n;

  for (n = 0; n < MSC_ImageSize; n++) {     /* Copy Initial Disk Image */
    Memory[n] = DiskImage[n];               /*   from Flash to RAM     */
  }

  I2C_Init();                               /* LED */

  USB_Init();                               /* USB Initialization */
  USB_Connect(__TRUE);                      /* USB Connect */

  while (1);                                /* Loop forever */
}
