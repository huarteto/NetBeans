/******************************************************************************/
/* IRQ.C: IRQ Module                                                          */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2010 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/

#include "IRQ.h"


/*----------------------------------------------------------------------------
  Default IRQ Handler 
 *----------------------------------------------------------------------------*/
void Default_IRQ_Handler (void) {
    return;
}


/*----------------------------------------------------------------------------
  Table for IRQ Handler 
 *----------------------------------------------------------------------------*/
void (*IRQ_ISR[IRQSIZE])(void) = {             /* IRQ Interrupt Service Routines */
  &Default_IRQ_Handler,                        /* channel  0: reserved           */
  &Default_IRQ_Handler,                        /* channel  1: WatchDog Timer     */
  &Default_IRQ_Handler,                        /* channel  2: reserved           */
  &Default_IRQ_Handler,                        /* channel  3: GPIO Interrupt0    */
  &Default_IRQ_Handler,                        /* channel  4: GPIO Interrupt1    */
  &Default_IRQ_Handler,                        /* channel  5: GPIO Interrupt2    */
  &Default_IRQ_Handler,                        /* channel  6: GPIO Interrupt3    */
  &Default_IRQ_Handler,                        /* channel  7: Audio Proc. Unit   */
  &Default_IRQ_Handler,                        /* channel  8: reserved           */
  &Default_IRQ_Handler,                        /* channel  9: reserved           */
  &Default_IRQ_Handler,                        /* channel 10: AD Converter       */
  &Default_IRQ_Handler,                        /* channel 11: Real Time Clock    */
  &Default_IRQ_Handler,                        /* channel 12: UART-0             */
  &Default_IRQ_Handler,                        /* channel 13: UART-1             */
  &Default_IRQ_Handler,                        /* channel 14: Timer-1            */
  &Default_IRQ_Handler,                        /* channel 15: Timer-0            */
  &Default_IRQ_Handler,                        /* channel 16: reserved           */
  &Default_IRQ_Handler,                        /* channel 17: reserved           */
  &Default_IRQ_Handler,                        /* channel 18: reserved           */
  &Default_IRQ_Handler,                        /* channel 19: USB Device         */
  &Default_IRQ_Handler,                        /* channel 20: reserved           */
  &Default_IRQ_Handler,                        /* channel 21: reserved           */
  &Default_IRQ_Handler,                        /* channel 22: PWM Interrupt0     */
  &Default_IRQ_Handler,                        /* channel 23: PWM Interrupt1     */
  &Default_IRQ_Handler,                        /* channel 24: PWM Interrupt2     */
  &Default_IRQ_Handler,                        /* channel 25: PWM Interrupt3     */
  &Default_IRQ_Handler,                        /* channel 26: I2C Interface      */
  &Default_IRQ_Handler,                        /* channel 27: SPI (Master/Slave) */
  &Default_IRQ_Handler,                        /* channel 28: reserved           */
  &Default_IRQ_Handler,                        /* channel 29: System Wake-Up     */
  &Default_IRQ_Handler,                        /* channel 30: SPIM0/1 Interrupt  */
  &Default_IRQ_Handler                         /* channel 31: reserved           */
};
