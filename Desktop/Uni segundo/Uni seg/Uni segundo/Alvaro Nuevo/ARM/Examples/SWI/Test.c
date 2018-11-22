/******************************************************************************/
/* TEST.C: Demonstration of SWI                                               */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2006 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/

#include <LPC213x.H>               // LPC213x definitions 


/*
 * Software Interrupt Function accept parameters and run in
 * Supervisor Mode (Interrupt protected)
 */

int __swi(0) add (int i1, int i2);
int __SWI_0      (int i1, int i2) {
  return (i1 + i2);
}

int __swi(1) mul4(int i);
int __SWI_1      (int i) {
  return (i << 2);
}

int __swi(2) div (int i1, int i2);
int __SWI_2      (int i1, int i2) {
  return (i1 / i2);
}

int __swi(3) mod (int i1, int i2);
int __SWI_3      (int i1, int i2) {
  return (i1 % i2);
}


int res;


/*
 * Test Function in Thumb Mode
 */

#pragma thumb

void test_t (void) {
  res  = div (res, 10);            // Call SWI Functions
  res  = mod (res,  3);
}


/*
 * Test Function in ARM Mode
 */

#pragma arm

void test_a (void) {
  res  = add (74,  27);            // Call SWI Functions
  res += mul4(res);
}


/*
 * Sample 'main' Function
 */

int main (void) {

  test_a();
  test_t();

	while (1);                       // Endless Loop
}
