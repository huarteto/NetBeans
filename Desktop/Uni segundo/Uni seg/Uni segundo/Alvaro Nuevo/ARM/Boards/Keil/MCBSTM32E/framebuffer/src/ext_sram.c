/******************************************************************************
 * @file:    ext_ram.c
 * @purpose: Driver for External SRAM
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

#include <stm32f10x_lib.h>
#include "main.h"
#include "gpio.h"
#include "ext_sram.h"


#define FSMC_EN         8           // Clk enable bit for FSMC



int ExtSRAM_Init (void) 
{

  /*-- Clocks Configuration ------------------------------------------------------*/
  bit_on(RCC->APB2ENR, IOPD_EN);      // enable PORT D
  bit_on(RCC->APB2ENR, IOPG_EN);      // enable PORT G
  bit_on(RCC->APB2ENR, IOPE_EN);      // enable PORT E
  bit_on(RCC->APB2ENR, IOPF_EN);      // enable PORT F  
  bit_on(RCC->AHBENR,  FSMC_EN);      // enable FSMC Clock
    
  
  /*-- GPIO Configuration ------------------------------------------------------*/
  
  /* SRAM Data lines configuration */
  /* NOE and NWE configuration */
  GPIOD->CRL &= ~0x00FF00FF;
  GPIOD->CRL |= 
  ((GPIO_AF_PUSHPULL | GPIO_SPEED_50MHZ) << GPIO_CONF_BIT(0))  | 
  ((GPIO_AF_PUSHPULL | GPIO_SPEED_50MHZ) << GPIO_CONF_BIT(1))  |
  ((GPIO_AF_PUSHPULL | GPIO_SPEED_50MHZ) << GPIO_CONF_BIT(4))  |
  ((GPIO_AF_PUSHPULL | GPIO_SPEED_50MHZ) << GPIO_CONF_BIT(5))  ;
  
  GPIOD->CRH &= ~0xFFFFFFFF;
  GPIOD->CRH |= 
  ((GPIO_AF_PUSHPULL | GPIO_SPEED_50MHZ) << GPIO_CONF_BIT(8))  | 
  ((GPIO_AF_PUSHPULL | GPIO_SPEED_50MHZ) << GPIO_CONF_BIT(9))  | 
  ((GPIO_AF_PUSHPULL | GPIO_SPEED_50MHZ) << GPIO_CONF_BIT(10)) |
  ((GPIO_AF_PUSHPULL | GPIO_SPEED_50MHZ) << GPIO_CONF_BIT(11)) |
  ((GPIO_AF_PUSHPULL | GPIO_SPEED_50MHZ) << GPIO_CONF_BIT(12)) |
  ((GPIO_AF_PUSHPULL | GPIO_SPEED_50MHZ) << GPIO_CONF_BIT(13)) |
  ((GPIO_AF_PUSHPULL | GPIO_SPEED_50MHZ) << GPIO_CONF_BIT(14)) |
  ((GPIO_AF_PUSHPULL | GPIO_SPEED_50MHZ) << GPIO_CONF_BIT(15)) ;
  
  
  /* NBL0, NBL1 configuration */
  GPIOE->CRL &= ~0xF00000FF;
  GPIOE->CRL |=
  ((GPIO_AF_PUSHPULL | GPIO_SPEED_50MHZ) << GPIO_CONF_BIT(0))  |
  ((GPIO_AF_PUSHPULL | GPIO_SPEED_50MHZ) << GPIO_CONF_BIT(1))  |
  ((GPIO_AF_PUSHPULL | GPIO_SPEED_50MHZ) << GPIO_CONF_BIT(7))  ;
  
  GPIOE->CRH &= ~0xFFFFFFFF;
  GPIOE->CRH |= 
  ((GPIO_AF_PUSHPULL | GPIO_SPEED_50MHZ) << GPIO_CONF_BIT(8))  | 
  ((GPIO_AF_PUSHPULL | GPIO_SPEED_50MHZ) << GPIO_CONF_BIT(9))  | 
  ((GPIO_AF_PUSHPULL | GPIO_SPEED_50MHZ) << GPIO_CONF_BIT(10)) |
  ((GPIO_AF_PUSHPULL | GPIO_SPEED_50MHZ) << GPIO_CONF_BIT(11)) |
  ((GPIO_AF_PUSHPULL | GPIO_SPEED_50MHZ) << GPIO_CONF_BIT(12)) |
  ((GPIO_AF_PUSHPULL | GPIO_SPEED_50MHZ) << GPIO_CONF_BIT(13)) |
  ((GPIO_AF_PUSHPULL | GPIO_SPEED_50MHZ) << GPIO_CONF_BIT(14)) |
  ((GPIO_AF_PUSHPULL | GPIO_SPEED_50MHZ) << GPIO_CONF_BIT(15)) ;
  
  /* SRAM Address / data lines configuration */
  GPIOF->CRL &= ~0x00FFFFFF;
  GPIOF->CRL |= 
  ((GPIO_AF_PUSHPULL | GPIO_SPEED_50MHZ) << GPIO_CONF_BIT(0))  |
  ((GPIO_AF_PUSHPULL | GPIO_SPEED_50MHZ) << GPIO_CONF_BIT(1))  |
  ((GPIO_AF_PUSHPULL | GPIO_SPEED_50MHZ) << GPIO_CONF_BIT(2))  |
  ((GPIO_AF_PUSHPULL | GPIO_SPEED_50MHZ) << GPIO_CONF_BIT(3))  |
  ((GPIO_AF_PUSHPULL | GPIO_SPEED_50MHZ) << GPIO_CONF_BIT(4))  |
  ((GPIO_AF_PUSHPULL | GPIO_SPEED_50MHZ) << GPIO_CONF_BIT(5))  ;
  
  GPIOF->CRH &= ~0xFFFF0000;
  GPIOF->CRH |= 
  ((GPIO_AF_PUSHPULL | GPIO_SPEED_50MHZ) << GPIO_CONF_BIT(12)) |
  ((GPIO_AF_PUSHPULL | GPIO_SPEED_50MHZ) << GPIO_CONF_BIT(13)) |
  ((GPIO_AF_PUSHPULL | GPIO_SPEED_50MHZ) << GPIO_CONF_BIT(14)) |
  ((GPIO_AF_PUSHPULL | GPIO_SPEED_50MHZ) << GPIO_CONF_BIT(15)) ;
  
  GPIOG->CRL &= ~0x00FFFFFF;
  GPIOG->CRL |= 
  ((GPIO_AF_PUSHPULL | GPIO_SPEED_50MHZ) << GPIO_CONF_BIT(0))  |
  ((GPIO_AF_PUSHPULL | GPIO_SPEED_50MHZ) << GPIO_CONF_BIT(1))  |
  ((GPIO_AF_PUSHPULL | GPIO_SPEED_50MHZ) << GPIO_CONF_BIT(2))  |
  ((GPIO_AF_PUSHPULL | GPIO_SPEED_50MHZ) << GPIO_CONF_BIT(3))  |
  ((GPIO_AF_PUSHPULL | GPIO_SPEED_50MHZ) << GPIO_CONF_BIT(4))  |
  ((GPIO_AF_PUSHPULL | GPIO_SPEED_50MHZ) << GPIO_CONF_BIT(5))  ;
  
  
  /* NE3 configuration */
  GPIOG->CRH &= ~0x000F0F00;
  GPIOG->CRH |= 
  ((GPIO_AF_PUSHPULL | GPIO_SPEED_50MHZ) << GPIO_CONF_BIT(10)) |
  ((GPIO_AF_PUSHPULL | GPIO_SPEED_50MHZ) << GPIO_CONF_BIT(12)) ;
    
  
  /*-- FSMC Configuration ------------------------------------------------------*/
  /*----------------------- SRAM Bank 3 ----------------------------------------*/
  
  FSMC_Bank1->BTCR[FSMC_Bank1_NORSRAM3+1] =           /* Bank1 NOR/SRAM timing register configuration */
                                         (0 << 28) |  /* FSMC AccessMode A */
                                         (0 << 24) |  /* Data Latency */
                                         (0 << 20) |  /* CLK Division */
                                         (0 << 16) |  /* Bus Turnaround Duration */
                                         (1 <<  8) |  /* Data SetUp Time */
                                         (0 <<  4) |  /* Address Hold Time */
                                         (0 <<  0);   /* Address SetUp Time */
                                         
  FSMC_Bank1->BTCR[FSMC_Bank1_NORSRAM3  ] =           /* Control register */
                                         (0 << 19) |  /* Write burst disabled */
                                         (0 << 15) |  /* Async wait disabled */
                                         (0 << 14) |  /* Extended mode disabled */
                                         (0 << 13) |  /* NWAIT signal is disabled */ 
                                         (1 << 12) |  /* Write operation enabled */
                                         (0 << 11) |  /* NWAIT signal is active one data cycle before wait state */
                                         (0 << 10) |  /* Wrapped burst mode disabled */
                                         (1 <<  9) |  /* Wait signal polarity active low */
                                         (0 <<  8) |  /* Burst access mode disabled */
                                         (1 <<  4) |  /* Memory data  bus width is 16 bits */
                                         (0 <<  2) |  /* Memory type is SRAM */
                                         (0 <<  1) |  /* Address/Data Multiplexing disable */
                                         (1 <<  0);   /* Memory Bank enable */  
  // Test ExtSRAM
  return(ExtSRAM_Test());
}




int ExtSRAM_Test(void)
{
  unsigned short *extRAM;
  
  int i, val, ramTestOk=1;  
  
  extRAM = (unsigned short *) EXT_SRAM_ADDR;
  

  for(i=0; i<EXT_SRAM_SIZE/2; i++)
  {
    extRAM[i] = i & 0xffff;
  }
  
  for(i=0; i<EXT_SRAM_SIZE/2; i++)
  {
    val = extRAM[i];
    
    if(val != (i&0xffff))
    {
      ramTestOk=0;
      break;
    }
  }

  if(ramTestOk) return(0);
  else          return(1);
}




