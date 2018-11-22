/******************************************************************************
 * @file:    taskman.c
 * @purpose: RTOS config and start
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
#include <RTL.h>                    /* RTX kernel functions & defines      */
#include "main.h"
#include "taskman.h"
#include "led.h"
#include "framebuf.h"
#include "keyboard.h"
#include "application.h"



FILE *fp_glcd0=0;


OS_TID idFrameBuf_WriteDisp={0};    // Frame Buffer Update
OS_TID idMoveBackImage={0};         // Moves Backg Image
OS_TID idTypeTextOnGLCD0={0};       // For typing text on GLCD0



// the taskman is controlling all about the running tasks, inserting and removing them
__task void taskman(void)
{
  // Insert some Tasks
  idFrameBuf_WriteDisp = os_tsk_create(FrameBuf_WriteDisp, 100);
  idMoveBackImage      = os_tsk_create(MoveBackImage,       50);
  idTypeTextOnGLCD0    = os_tsk_create(TypeTextOnGLCD0,     5);
  
  // Initialize System
  BootSystem();

  // Start the tasks
  os_evt_set(FB_START, idFrameBuf_WriteDisp);
  os_evt_set(FB_START, idMoveBackImage);
  os_evt_set(FB_START, idTypeTextOnGLCD0);
  
  os_tsk_delete_self();
 
  while(1)
  {
    ;     // should never reach this ...
  }
}



void BootSystem(void)
{
  led_init();
  InitKeyboard();
 
  fp_glcd0 = freopen("glcd0", "w", stdout);
}









