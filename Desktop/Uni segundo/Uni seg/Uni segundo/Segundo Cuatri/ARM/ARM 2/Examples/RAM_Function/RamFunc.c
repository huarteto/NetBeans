/******************************************************************************/
/* RamFunc.c: External RAM_Function: RO,RW & ZI sections                      */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2006 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/

extern int count;

/*  Assignment of Code Memory for Module to RAM (under Options for file 'RamFunc.c') */
/*   RO - Code/Const:       RAM1 (0x80010000-0x8001FFFF)                             */

void ram_function (void) {
  count++;
}

