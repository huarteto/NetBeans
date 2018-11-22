/******************************************************************************
 * @file:    retarget.c
 * @purpose: retargeting layer for GLCD
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
#include <stdlib.h>
#include <string.h>
#include <rt_sys.h>
#include <rt_misc.h>
#include <stm32f10x.h>
#include "drv_glcd0.h"
#include "drv_itm.h"
#include "retarget.h"



#pragma import(__use_no_semihosting)

#ifdef _MICROLIB
  #pragma import(__use_full_stdio)
#endif


/* Standard IO device name defines. */
const char __stdin_name[]  = "STDIN";
const char __stdout_name[] = "STDOUT";
const char __stderr_name[] = "STDERR";
const char __glcd0_name[]  = "glcd0";
const char __itm_name[]    = "itm";






/*==============================================================================
   I/O Retargetting Code
==============================================================================*/


void _ttywrch(int c)
{
  fputc(c, stdout);   // stdout
  fflush(stdout);
}


FILEHANDLE _sys_open (const char *name, int openmode)
{  
  // Register standard Input Output devices.
  if (strcmp(name, __stdin_name) == 0) 
  {
    return (STDIN);
  }
  
  else if (strcmp(name, __stdout_name) == 0) 
  {
    return (STDOUT);
  }
  
  else if (strcmp(name, __stderr_name) == 0) 
  {      
    return (STDERR);
  } 
  
  else if (strcmp(name, __glcd0_name) == 0) 
  {
    Open_GLCD0();
    return (GLCD0_STREAM);
  }

  else if (strcmp(name, __itm_name) == 0) 
  {
    Open_ITM();
    return (ITM_STREAM);
  }
    
  else return(0);
}


int _sys_close (FILEHANDLE fh) 
{
  if(fh == GLCD0_STREAM) Close_GLCD0();
  else if(fh == ITM_STREAM) Close_ITM();
  return (0);
}


int _sys_write (FILEHANDLE fh, const unsigned char *buf, unsigned len, int mode) 
{
  switch(fh)
  {
    case STDOUT:
      break;
      
    case GLCD0_STREAM:  Output_GLCD0(buf, len);
      break;
    
    case ITM_STREAM:    Output_ITM(buf, len);
      break;

  }
  
  return (0);
}

int _sys_read (FILEHANDLE fh, unsigned char *buf, unsigned len, int mode) 
{
  if(fh == ITM_STREAM) return(Input_ITM(buf, len));
  else return (0);
}


/*--------------------------- _sys_istty ------------------------------------*/
int _sys_istty (FILEHANDLE fh) 
{
  if (fh > 0x8000) 
  {
    return (1);
  }
  return (0);
}

/*--------------------------- _sys_seek -------------------------------------*/
int _sys_seek (FILEHANDLE fh, long pos) 
{
  return (0);
}

/*--------------------------- _sys_ensure -----------------------------------*/
int _sys_ensure (FILEHANDLE fh) 
{
  return (-1);
}

/*--------------------------- _sys_flen -------------------------------------*/
long _sys_flen (FILEHANDLE fh) 
{
  return (0);
}

/*--------------------------- _sys_tmpnam -----------------------------------*/
int _sys_tmpnam (char *name, int sig, unsigned maxlen) 
{
  return (1);
}

/*--------------------------- _sys_command_string ---------------------------*/
char *_sys_command_string (char *cmd, int len) 
{
  return (cmd);
}


void _sys_exit (int return_code) 
{
  while(1);
}



/*==============================================================================
   CMSIS Code
==============================================================================*/

void SystemReset()
{
#if (defined  USE_CMSIS) && (USE_CMSIS == 1)
  NVIC_SystemReset();
#else
  while(1);
#endif
}


