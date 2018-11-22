/******************************************************************************/
/* RETARGET.C: 'Retarget' layer for target-dependent low level functions      */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2006 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/

#include <stdio.h>
#include "leuart.h"

/* #pragma import(__use_no_semihosting_swi) */

struct __FILE
{
  int handle;
};

FILE __stdout;

int fputc(int ch, FILE *f)
{
  return(LEUART1_txByte(ch));
}


int fgetc(FILE *f)
{
  return(LEUART1_rxByte());
}

int ferror(FILE *f)
{
  /* Your implementation of ferror */
  return EOF;
}


void _ttywrch(int ch)
{
  LEUART1_txByte(ch);
}


void _sys_exit(int return_code)
{
 label:  goto label; /* endless loop */
}
