/******************************************************************************/
/* Blinky: LED Flasher                                                      */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2009 Keil Software. All rights reserved.                     */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/


#include <stdio.h>



/* NMI Handler  */
void NMI_Handler(void)
{
  printf("\nNMI occured...");
}




/* Bluescreen funcs 
 * useful for debugging
 */
 
void SystemStop(void)
{
  printf("\n\t\t System Halted...");
  while(1);
}

void HardFault_Handler(void)
{
  printf("\n\n\t\t!!! Hard Fault !!!");
  SystemStop();
}

void BusFault_Handler(void)
{
  printf("\n\n\t\t Bus Fault !!!");
  SystemStop();
}

void UsageFault_Handler(void)
{
  printf("\n\n\t\t Usage Fault !!!");
  SystemStop();
}

