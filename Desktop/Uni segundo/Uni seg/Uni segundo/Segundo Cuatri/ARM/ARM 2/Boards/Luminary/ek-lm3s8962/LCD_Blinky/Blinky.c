/******************************************************************************/
/* BLINKY.C: LED Flasher                                                      */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2007 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/

#include <stdio.h>
#include <LM3Sxxxx.H>           // LM3Sxxxx library definitions
#include "..\rit128x96x4.h"     // OLED functions


/* Status LED and Push Buttons pin definitions */
#define LED             GPIO_PIN_0 /* PF0 */
#define SELECT          GPIO_PIN_1 /* PF1 */
#define UP              GPIO_PIN_0 /* PE0 */
#define DOWN            GPIO_PIN_1 /* PE1 */
#define LEFT            GPIO_PIN_2 /* PE2 */
#define RIGHT           GPIO_PIN_3 /* PE3 */
#define BUTTON         (UP | DOWN | LEFT | RIGHT)


/* Global variables */
volatile unsigned char Tick;    // Tick Counter (0..99)
volatile unsigned long Time;    // Time Counter (10ms)
volatile unsigned char Buttons; // Button States
volatile unsigned char TraceB;  // Trace Buttons


/* SysTick interrupt happens every 10 ms */
void SysTick_Handler (void) {

  if (GPIOPinRead(GPIO_PORTF_BASE, SELECT)) {
    Time++;
  }

  if (Tick == 50) {
    GPIOPinWrite(GPIO_PORTF_BASE, LED, LED);    
  }
  Tick++;
  if (Tick == 100) {
    Tick = 0;
    GPIOPinWrite(GPIO_PORTF_BASE, LED, 0);    
  }

  Buttons = GPIOPinRead(GPIO_PORTE_BASE, BUTTON);
  TraceB  = Buttons;
}


/* Main program */
int main (void) {
  unsigned long time;
  unsigned char buttons, b;
           char buf[21+1];

  /* Set the clocking to run from PLL at 50 MHz */
  SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_8MHZ);

  /* Enable peripherals */
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

  /* Configure Status LED as output */
  GPIOPadConfigSet(GPIO_PORTF_BASE, LED,    GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD);
  GPIODirModeSet  (GPIO_PORTF_BASE, LED,    GPIO_DIR_MODE_OUT);

  /* Configure push buttons as inputs */
  GPIOPadConfigSet(GPIO_PORTE_BASE, BUTTON, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
  GPIODirModeSet  (GPIO_PORTE_BASE, BUTTON, GPIO_DIR_MODE_IN);
  GPIOPadConfigSet(GPIO_PORTF_BASE, SELECT, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
  GPIODirModeSet  (GPIO_PORTF_BASE, SELECT, GPIO_DIR_MODE_IN);

  /* Initialize LCD */
  RIT128x96x4Init(1000000);
  RIT128x96x4Clear();

  /* Display initial message */
  sprintf(buf, "      Keil Demo      ");
  RIT128x96x4StringDraw(buf, 0,      6, 11);
  sprintf(buf, "     EK-LM3S8962     ");
  RIT128x96x4StringDraw(buf, 0, 10*1+6, 11);
  sprintf(buf, "   Blinky Example    ");
  RIT128x96x4StringDraw(buf, 0, 10*2+6, 11);
  sprintf(buf, "  Time =             ");
  RIT128x96x4StringDraw(buf, 0, 10*4+6, 11);
  sprintf(buf, "  Button Up:         ");
  RIT128x96x4StringDraw(buf, 0, 10*5+8, 11);
  sprintf(buf, "  Button Down:       ");
  RIT128x96x4StringDraw(buf, 0, 10*6+8, 11);
  sprintf(buf, "  Button Left:       ");
  RIT128x96x4StringDraw(buf, 0, 10*7+8, 11);
  sprintf(buf, "  Button Right:      ");
  RIT128x96x4StringDraw(buf, 0, 10*8+8, 11);

  /* Setup and enable SysTick with interrupt (100Hz) */
  SysTickPeriodSet(SysCtlClockGet() / 100);
  SysTickEnable();
  SysTickIntEnable();

  time    = ~Time;
  buttons = ~Buttons;

  /* Endless Loop */
  while (1) {
    if (time != Time) {
      time = Time;
      sprintf(buf, "%1.2f s", (float)time / 100);
      RIT128x96x4StringDraw(buf,                          9*6, 10*4+6, 11);
    }
    b = Buttons;
    if ((buttons ^ b) & UP) {
      RIT128x96x4StringDraw((b & UP)    ? "Off" : "On ", 16*6, 10*5+8, 11);
    }
    if ((buttons ^ b) & DOWN) {
      RIT128x96x4StringDraw((b & DOWN)  ? "Off" : "On ", 16*6, 10*6+8, 11);
    }
    if ((buttons ^ b) & LEFT) {
      RIT128x96x4StringDraw((b & LEFT)  ? "Off" : "On ", 16*6, 10*7+8, 11);
    }
    if ((buttons ^ b) & RIGHT) {
      RIT128x96x4StringDraw((b & RIGHT) ? "Off" : "On ", 16*6, 10*8+8, 11);
    }
    buttons = b;
  }
}
