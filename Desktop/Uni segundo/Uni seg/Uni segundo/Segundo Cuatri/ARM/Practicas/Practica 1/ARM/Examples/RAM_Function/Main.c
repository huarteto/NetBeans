/******************************************************************************/
/* RAM_Function.C: Execute Functions in RAM                                   */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2006 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/

#include <stdlib.h>

unsigned int count = 0;

extern void ram_function (void);

/* Mapped to Read-Only Code Region (RO-CODE same as CODE)
   which is a part of Read-Only Region (RO) */

int main (void)  {

  while (1) {
    ram_function ();
  }


}
