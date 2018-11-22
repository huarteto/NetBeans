/*----------------------------------------------------------------------------
 * Name:    Test.c
 * Purpose: usage of SVC function calls for TMPM330
 * Version: V1.00
 *----------------------------------------------------------------------------
 * This file is part of the uVision/ARM development tools.
 * This software may only be used under the terms of a valid, current,
 * end user licence from KEIL for a compatible version of KEIL software
 * development tools. Nothing else gives you the right to use this software.
 *
 * Copyright (c) 2005-2009 Keil Software. All rights reserved.
 *----------------------------------------------------------------------------*/

#include "TMPM330.h"


/*----------------------------------------------------------------------------
  Note:
  Software Interrupt Function accept up to four parameters
  and run in Supervisor Mode (Interrupt protected)
 *----------------------------------------------------------------------------*/

int __svc(0) add (int i1, int i2);
int __SVC_0      (int i1, int i2) {
  return (i1 + i2);
}

int __svc(1) mul4(int i);
int __SVC_1      (int i) {
  return (i << 2);
}

int __svc(2) div (int i1, int i2);
int __SVC_2      (int i1, int i2) {
  return (i1 / i2);
}

int __svc(3) mod (int i1, int i2);
int __SVC_3      (int i1, int i2) {
  return (i1 % i2);
}

int res;


/*----------------------------------------------------------------------------
  Test Function 
 *----------------------------------------------------------------------------*/
void test_t (void) {
  res  = div (res, 10);                           // Call SWI Functions
  res  = mod (res,  3);
}


/*----------------------------------------------------------------------------
  Test Function 
 *----------------------------------------------------------------------------*/
void test_a (void) {
  res  = add (74,  27);                           // Call SWI Functions
  res += mul4(res);
}


/*----------------------------------------------------------------------------
  MAIN function
 *----------------------------------------------------------------------------*/
int main (void) {

  test_a();
  test_t();

  while (1) {                                     // Loop forever
    ;
  }
}

