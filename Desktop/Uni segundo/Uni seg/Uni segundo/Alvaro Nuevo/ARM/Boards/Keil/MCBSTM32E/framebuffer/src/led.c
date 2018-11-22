/******************************************************************************
 * @file:    led.c
 * @purpose: Driver for LEDs
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
#include "gpio.h"
#include "led.h"




int led_init(void)
{ 
  bit_on(RCC->APB2ENR, IOPB_EN);      // enable PORT B
  
  GPIOB->CRH = 
  ((GPIO_OUT_PUSH_PULL | GPIO_SPEED_2MHZ) << GPIO_CONF_BIT(8)) | \
  ((GPIO_OUT_PUSH_PULL | GPIO_SPEED_2MHZ) << GPIO_CONF_BIT(9)) | \
  ((GPIO_OUT_PUSH_PULL | GPIO_SPEED_2MHZ) << GPIO_CONF_BIT(10)) | \
  ((GPIO_OUT_PUSH_PULL | GPIO_SPEED_2MHZ) << GPIO_CONF_BIT(11)) | \
  ((GPIO_OUT_PUSH_PULL | GPIO_SPEED_2MHZ) << GPIO_CONF_BIT(12)) | \
  ((GPIO_OUT_PUSH_PULL | GPIO_SPEED_2MHZ) << GPIO_CONF_BIT(13)) | \
  ((GPIO_OUT_PUSH_PULL | GPIO_SPEED_2MHZ) << GPIO_CONF_BIT(14)) | \
  ((GPIO_OUT_PUSH_PULL | GPIO_SPEED_2MHZ) << GPIO_CONF_BIT(15));

  return(0);
}


void led_out(char out)
{
    LED_PORT = (LED_PORT & LED_PORT_MASK) | (out << LED_PORT_SHIFT);
}



void running_light(void)
{
  static int i=0, count=0;
  
  count++;

  if(count > 0xffff)
  {
    count=0;
    led_out(1<<(i=++i<8?i:0));          // feed the running light
  } 
}

