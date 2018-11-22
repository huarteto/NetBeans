/******************************************************************************/
/* I2C_STM32.c: STM32 low level I2C routines                                  */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2009 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/

#include <stm32f10x_cl.h>


/************************ Local auxiliary functions ***************************/

/*******************************************************************************
* I2C communication status                                                     *
*   Parameter:                                                                 *
*   Return:               status                                               *
*******************************************************************************/

static __inline unsigned int I2C_sr (void) {
  unsigned int sr;

  sr  = I2C1->SR1;
  sr |= I2C1->SR2 << 16;
  return (sr);
}

/************************ Exported functions **********************************/

/*******************************************************************************
* Initialize I2C interface in master mode                                      *
*   Parameter:                                                                 *
*   Return:                                                                    *
*******************************************************************************/

void I2C_Init (void) {
  unsigned int tout;

  /* Enable clock for I2C1, GPIOB and AFIO                                    */
  RCC->APB2ENR |= (1 << 4) | (1 << 0);
  RCC->APB1ENR |= (1 << 21);

  /* I2C1 pins remapped, use PB8, PB9                                         */
  AFIO->MAPR   |= 0x00000002;
  GPIOB->CRH   |= 0x000000FF;

  I2C1->CR1     = 0x8000;               /* Reset I2C peripheral               */
  for (tout = 100; tout; tout--);
  I2C1->CR1     = 0x0000;

  /* Configure I2C peripheral                                                 */
  I2C1->CR1     = 0x0001;
  I2C1->CR2     = 0x0024;
  I2C1->CR1     = 0x0000;
  I2C1->TRISE   = 0x0025;
  I2C1->CCR     = 0x005A;
  I2C1->CR1    |= 0x0401;
  I2C1->OAR1    = 0x40A0;
}


/*******************************************************************************
* Generate start condition on I2C bus                                          *
*   Parameter:                                                                 *
*   Return:                                                                    *
*******************************************************************************/

void I2C_Start (void) {

  I2C1->CR1 |= 0x0100;
  while (!(I2C_sr() & 0x0001));
}


/*******************************************************************************
* Generate stop condition on I2C bus                                           *
*   Parameter:                                                                 *
*   Return:                                                                    *
*******************************************************************************/

void I2C_Stop (void) {

  I2C1->CR1 |= 0x0200;
  while (I2C_sr() & 0x00020000);        /* Wait until BUSY bit reset          */
}


/*******************************************************************************
* Write address on I2C interface                                               *
*   Parameter:    adr:    address to be written                                *
*   Return:                                                                    *
*******************************************************************************/

void I2C_Addr (unsigned char adr) {

  I2C1->DR = adr;
  while (!(I2C_sr() & 0x0002));
}


/*******************************************************************************
* Write a byte to I2C interface                                                *
*   Parameter:    c:      data to be written                                   *
*   Return:                                                                    *
*******************************************************************************/

void I2C_Write (unsigned char c) {

  I2C1->DR = c;
  while (!(I2C_sr() & 0x00000004));     /* Wait until BTF bit set             */ 
}


/*******************************************************************************
* Read a byte from I2C interface                                               *
*   Parameter:                                                                 *
*   Return:               read data                                            *
*******************************************************************************/

unsigned char I2C_Read (int ack) {

  /* Enable/disable Master acknowledge                                        */
  if (ack) I2C1->CR1 |=  0x0400;
  else     I2C1->CR1 &= ~0x0400;

  while (!(I2C_sr() & 0x00000040));     /* Wait until RxNE bit set            */
  return (I2C1->DR);
}

/******************************************************************************/
