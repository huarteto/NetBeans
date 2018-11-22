/******************************************************************************
 * @file:    joystick.c
 * @purpose: Driver for joystick
 * @version: V1.0
 * @date:    14. Aug 2009
 *----------------------------------------------------------------------------
 *
 * Copyright (C) 2009 ARM Limited. All rights reserved.
 *
 * ARM Limited (ARM) is supplying this software for use with Cortex-Mx 
 * processor based microcontrollers.  This file can be freely distributed 
 * within development tools that are supporting such ARM based processors. 
 *
 * THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 * OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 * ARM SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR
 * CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 *
 ******************************************************************************/
 
 
#include <stm32f10x.h>
#include "main.h"
#include "keyboard.h"
#include "gpio.h"


int InitKeyboard(void)
{  
  bit_on(RCC->APB2ENR, IOPD_EN);			// enable PORT D
  bit_on(RCC->APB2ENR, IOPG_EN);			// enable PORT G
  
  /* PG07: Joystick select */
  GPIOG->CRL &= ~0xF0000000;
  GPIOG->CRL |= 
	((GPIO_IN_FLOATING | GPIO_MODE_INPUT) << GPIO_CONF_BIT(7))  ;
 
  /* PG15: Joystick up,  PG14: Joystick left,  PG13: Joystick right  */
  GPIOG->CRH &= ~0xFFF00000;
  GPIOG->CRH |= 
	((GPIO_IN_FLOATING | GPIO_MODE_INPUT) << GPIO_CONF_BIT(13)) |
  ((GPIO_IN_FLOATING | GPIO_MODE_INPUT) << GPIO_CONF_BIT(14)) |
  ((GPIO_IN_FLOATING | GPIO_MODE_INPUT) << GPIO_CONF_BIT(15)) ;
 
  /* PD03: Joystick down   */
  GPIOD->CRL &= ~0x0000F000;
  GPIOD->CRL |= 
	((GPIO_IN_FLOATING | GPIO_MODE_INPUT) << GPIO_CONF_BIT(3))  ;
  
  /* PG08: User Button   */
  GPIOG->CRH &= ~0x0000000F;
  GPIOG->CRH |= 
	((GPIO_IN_FLOATING | GPIO_MODE_INPUT) << GPIO_CONF_BIT(8))  ;
  
  return(0);
}



int getKeyboardVal(void)
{
  unsigned int val=0;
  
  if ( (GPIOG->IDR & (1<< JOYHW_UP     )) == 0) val |= JOY_UP;      /* up     pressed means 0 */
  if ( (GPIOG->IDR & (1<< JOYHW_LEFT   )) == 0) val |= JOY_LEFT;    /* left   pressed means 0 */
  if ( (GPIOG->IDR & (1<< JOYHW_RIGHT  )) == 0) val |= JOY_RIGHT;   /* right  pressed means 0 */
  if ( (GPIOG->IDR & (1<< JOYHW_SELECT )) == 0) val |= JOY_BUT;     /* select pressed means 0 */
  if ( (GPIOD->IDR & (1<< JOYHW_DOWN   )) == 0) val |= JOY_DOWN;    /* down   pressed means 0 */
  
  if ( (GPIOG->IDR & (1<< BUTHW_USER   )) == 0) val |= BUT_USER;    /* User Button pressed    */
  
  return(val);
}





