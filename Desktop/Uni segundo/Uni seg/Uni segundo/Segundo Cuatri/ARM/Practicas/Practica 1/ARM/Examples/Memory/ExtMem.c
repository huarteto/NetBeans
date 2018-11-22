/******************************************************************************/
/* ExtMem.c: External Memory: RO,RW & ZI sections                             */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2006 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/


/* External Memory Assignment (under Options for file 'ExtMem.c') */
/*   RO - Code/Const:       ROM1 (0x80000000-0x8000FFFF) */
/*   RW - Data:             RAM1 (0x80010000-0x8001FFFF) */
/*   ZI - Zero Init. Data:  RAM1 (0x80010000-0x8001FFFF) */


/* Mapped to Read/Write Region (RW) */
unsigned char FillVal = 0xff;

/* Mapped to Zero-Initialized Region (ZI) */
unsigned char Buffer[4096];


/* Mapped to Read-Only Code Region (RO-CODE same as CODE)
   which is a part of Read-Only Region (RO) */

void Setup (void) {
  FillVal ^= 0x55;
}
