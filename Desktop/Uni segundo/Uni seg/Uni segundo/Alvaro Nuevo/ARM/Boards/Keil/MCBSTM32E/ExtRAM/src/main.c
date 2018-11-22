/******************************************************************************
 * @file:    main.c
 * @purpose: main file
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



#include <stdio.h>
#include <stm32f10x.h>

#include "main.h"
#include "ext_sram.h"


#ifdef RUN_FROM_FLASH
volatile unsigned short *frameBuf  = (volatile unsigned short *) EXT_SRAM_ADDR;   // External SRAM

#else   // This is only a guess! Modify for your programm if you need it!
volatile unsigned short *frameBuf  = (volatile unsigned short *) EXT_SRAM_ADDR + KBYTES(4);   // External SRAM

#endif


__ASM void Exit(void) 
{
  BKPT    0x02
}



int main(void)
{
  int i=0xff;
  
 
#ifdef RUN_FROM_FLASH
  int val;
#endif


  SystemInit();  
  for(; i--;) __nop();     // inertial delay, nessessary after system init
  
#ifdef RUN_FROM_FLASH  
  val = ExtSRAM_Init();
  printf("\nSimple external SRAM test: %s", val? "failed": "succeed");

#else
  printf("\nApplication is running from External SRAM now!");

#endif

  Exit();

	while(1)
	{
    ;
  }
}






