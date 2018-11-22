/******************************************************************************/
/* MCOMMAND.C: Time Set Commands for the Remote Measurement Recorder          */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2006 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/

//#include <stdio.h>                       /* standard ANSI I/O .h-file         */
#include <91x_lib.h> 					 /* STR91x definitions                */
#include <string.h>

#include "measure.h"                     /* global project definition file    */
#include "RT_Agent.H"


void prints (const char *text)   {
  while (*text)  {
    RTA_TermPutChar (*text++);
  }
}


static void numout (unsigned char v)  {
  v &= 0xF;                            // convert to ASCII
  v += '0';
  if (v > '9')  {
    v += 'A'-'9'-1;
  }
  RTA_TermPutChar (v);
}


static void printv (unsigned int num, unsigned int base, unsigned int len)  {
  unsigned int tmp;
  unsigned int i = 0;
  char buf[16];

  memset (buf, 0, sizeof (buf));

  do  {                     // calculate number
    tmp = num % base;
    num = num / base;
    buf[i++] = tmp;
  } while (num);

  while (len)  {
    numout (buf[--len]);
  }
}

/******************************************************************************/
/*                           Display measurements                             */
/******************************************************************************/
void measure_display (struct mrec display)  {
  prints ("\rTime: ");
  printv (display.time.hour, 10, 2);
  prints (":");
  printv (display.time.min, 10, 2);
  prints (":");
  printv (display.time.sec, 10, 2);
  prints (":");
  printv (display.time.msec, 10, 3);
  prints (" GPIO7:");
  printv (display.gpio, 16, 2);
}

