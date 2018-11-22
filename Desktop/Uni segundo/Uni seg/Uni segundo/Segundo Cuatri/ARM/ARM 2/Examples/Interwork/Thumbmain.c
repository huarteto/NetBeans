/******************************************************************************/
/* THUMBMAIN.C:  ARM/Thumb Interworking Example                               */
/*               Thumb main() calling ARM function                            */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2006 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/

#include <stdio.h>                /* prototype declarations for I/O functions */

extern void init_serial (void);
extern void arm_function (void);

int main (void) {
  init_serial();
  printf("Hello from Thumb\n"); 
  arm_function(); 
  printf("And goodbye from Thumb\n"); 

  while (1) {                   /* An embedded program does not stop and       */
    ;  /* ... */                /* never returns. We use an endless loop.      */
  }                             /* Replace the dots (...) with your own code.  */

}
