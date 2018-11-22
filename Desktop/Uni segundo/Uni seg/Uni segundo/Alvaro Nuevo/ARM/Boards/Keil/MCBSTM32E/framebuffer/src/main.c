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
#include <RTL.h>                      /* RTX kernel functions & defines      */
#include <stm32f10x.h>

#include "main.h"
#include "term_ctrl.h"
#include "ext_sram.h"
#include "framebuf.h"
#include "taskman.h"



unsigned int  flags  = 0;


int main(void)
{
  FILE *fp_itm = 0;
  
  SystemInit();
  
  // Open STDIN / STDOUT for ITM Viewer and CMSIS back channel
  fp_itm = freopen("itm", "w", stdout);
  if(fp_itm) fp_itm = freopen("itm", "r", stdin);
   
  printf("\nSTDOUT is now redirected to ITM output channel.\nYou may print some debug messages via printf to ITM.");
  
  
	// Start the System
  os_sys_init(taskman);
    
	while(1)
	{
    ;
  }
}






