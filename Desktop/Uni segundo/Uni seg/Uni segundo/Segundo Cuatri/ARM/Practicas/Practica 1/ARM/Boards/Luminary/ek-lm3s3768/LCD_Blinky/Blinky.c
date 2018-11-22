/******************************************************************************/
/* BLINKY.C: LED Flasher using also LCD                                       */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2008 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/

#include <stdio.h>
#include <LM3Sxxxx.H>                   /* LM3Sxxxx library definitions       */
#include "grlib.h"
#include "..\formike128x128x16.h"


/* Status LED and Navigator pin definitions */
#define LED             GPIO_PIN_0      /* PF0 */
#define NAV_UP          GPIO_PIN_3      /* PB3 */
#define NAV_DN          GPIO_PIN_4      /* PB4 */
#define NAV_L           GPIO_PIN_5      /* PB5 */
#define NAV_R           GPIO_PIN_6      /* PB6 */
#define NAV_SEL         GPIO_PIN_7      /* PB7 */
#define NAV_MSK        (NAV_UP | NAV_DN | NAV_L | NAV_R | NAV_SEL)
#define AR_MSK         (NAV_UP | NAV_DN | NAV_L | NAV_R)    /* Autorepeat mask*/
#define DEB_TIME        10              /* Debounce time (10*10ms=100ms)      */
#define AR_TIME         20              /* Autorepeat time (20*10ms=100ms)    */


/* Global variables */
volatile unsigned char Tick;            /* Tick Counter (0..99)               */
volatile unsigned long Time;            /* Time Counter (10ms)                */
volatile unsigned char Nav;             /* Navigator States                   */
volatile unsigned char TraceB;          /* Trace Navigator                    */


/* SysTick interrupt happens every 10 ms */
void SysTick_Handler (void) {
  static unsigned int cur, old;
  static unsigned int deb_cnt = DEB_TIME + 1;
  static unsigned int aut_cnt = AR_TIME;
  static unsigned int led;

  /* Handle debouncing and autorepeat for navigator */
  cur = ~GPIOPinRead(GPIO_PORTB_BASE, NAV_MSK) & NAV_MSK;
  
  if (cur ^ old) {
    old     = cur;
    deb_cnt = DEB_TIME + 1;
    aut_cnt = AR_TIME;
  } else {
    if (deb_cnt) {
      deb_cnt --;
      aut_cnt ++;
    }
    aut_cnt --;
  }

  if (deb_cnt == 1) {
    Nav = cur;
  }
  if (aut_cnt == 0) {
    aut_cnt = AR_TIME;
    Nav |= cur & AR_MSK;
  }

  /* If navigator select pressed, time is stopped */
  if (!(Nav & NAV_SEL)) {
    Time++;
  }
  
  /* Toggle LED every 1 second */
  if (Tick ++ > 99) {
    Tick  = 0;
    led  ^= LED;
    GPIOPinWrite(GPIO_PORTF_BASE, LED, led);
  }
}


/* Main program */
int main (void) {
  tContext    sContext;
  tRectangle  sRect;
  unsigned    int  w, h; 
  unsigned    int  time, nav, n;
              char buf[13+1];

  /* Set the clocking to run from XTAL */
  SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN | SYSCTL_XTAL_8MHZ);

  /* Enable peripherals */
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

  /* Configure Status LED as output */
  GPIOPadConfigSet(GPIO_PORTF_BASE, LED,     GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD);
  GPIODirModeSet  (GPIO_PORTF_BASE, LED,     GPIO_DIR_MODE_OUT);
  GPIOPinWrite    (GPIO_PORTF_BASE, LED, 0);

  /* Configure navigator pins as inputs */
  GPIOPadConfigSet(GPIO_PORTB_BASE, NAV_MSK, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
  GPIODirModeSet  (GPIO_PORTB_BASE, NAV_MSK, GPIO_DIR_MODE_IN);

  /* Initialize LCD */
  Formike128x128x16Init();
  Formike128x128x16BacklightOn();
  GrContextInit(&sContext, &g_sFormike128x128x16);
  w = GrContextDpyWidthGet(&sContext);
  h = GrContextDpyHeightGet(&sContext);

  /* Display initial message */
  sRect.sXMin = 0; sRect.sYMin = 0; sRect.sXMax = w - 1; sRect.sYMax = 30;
  GrContextForegroundSet(&sContext, ClrDarkBlue);
  GrRectFill(&sContext, &sRect);
  GrContextForegroundSet(&sContext, ClrWhite);
  GrRectDraw(&sContext, &sRect);
  sRect.sXMax = w - 1; sRect.sYMax = h - 1;
  GrRectDraw(&sContext, &sRect);
  sRect.sYMin = sRect.sYMax - 15;
  GrRectDraw(&sContext, &sRect);
  GrContextFontSet(&sContext, &g_sFontCm14b);
  GrStringDrawCentered(&sContext, "KEIL Demo",    -1, w / 2  , 10    , 0);
  GrContextFontSet(&sContext, &g_sFontFixed6x8);
  GrStringDrawCentered(&sContext, "for LM3S3768", -1, w / 2  , 22    , 0);
  GrStringDraw(&sContext, "Use navigator ...",    -1, 4      , h - 26, 0);
  GrStringDraw(&sContext, "          .   s",      -1, w - 100, h - 11, 0);
  sprintf(buf, "%1.2f s", (float)time / 100);

  /* Setup and enable SysTick with interrupt (100Hz) */
  SysTickPeriodSet(SysCtlClockGet() / 100);
  SysTickEnable();
  SysTickIntEnable();

  time = ~Time;
  n    =  0;

  /* Endless Loop */
  while (1) {
    /* Display time */
    if (time ^ Time) {
      /* Clear old time value */
      GrContextForegroundSet(&sContext, ClrBlack);
      sprintf(buf, "%13.2f", (float)time / 100); 
      buf[10] =' ';
      GrStringDraw(&sContext, buf,    -1, w - 100, h - 11, 0);
      time = Time;

      /* Display new time value */
      GrContextForegroundSet(&sContext, ClrWhite);
      sprintf(buf, "%13.2f", (float)time / 100);
      GrStringDraw(&sContext, buf,    -1, w - 100, h - 11, 0);
    }

    /* Display Navigator state */
    nav  = Nav;
    n   ^= nav;
    if (n & NAV_UP) {
      if (nav & NAV_UP) {
        /* UP pressed */
        GrContextForegroundSet(&sContext, ClrWhite);
      } else {
        /* UP released */
        GrContextForegroundSet(&sContext, ClrBlack);
      }
      GrStringDrawCentered(&sContext, "UP",    -1, w / 2, h / 2 - 16, 0);
    }
    if (n & NAV_DN) {
      if (nav & NAV_DN) {
        /* DOWN pressed */
        GrContextForegroundSet(&sContext, ClrWhite);
      } else {
        /* DOWN released */
        GrContextForegroundSet(&sContext, ClrBlack);
      }
      GrStringDrawCentered(&sContext, "DOWN",  -1, w / 2, h / 2 + 16, 0);
    }
    if (n & NAV_L) {
      if (nav & NAV_L) {
        /* LEFT pressed */
        GrContextForegroundSet(&sContext, ClrWhite);
      } else {
        /* LEFT released */
        GrContextForegroundSet(&sContext, ClrBlack);
      }
      GrStringDrawCentered(&sContext, "LEFT",  -1, w / 2 - 40, h / 2 , 0);
    }
    if (n & NAV_R) {
      if (nav & NAV_R) {
        /* RIGHT pressed */
        GrContextForegroundSet(&sContext, ClrWhite);
      } else {
        /* RIGHT released */
        GrContextForegroundSet(&sContext, ClrBlack);
      }
      GrStringDrawCentered(&sContext, "RIGHT",    -1, w / 2 + 40, h / 2, 0);
    }
    if (n & NAV_SEL) {
      if (nav & NAV_SEL) {
        /* SELECT pressed */
        GrContextForegroundSet(&sContext, ClrWhite);
      } else {
        /* SELECT released */
        GrContextForegroundSet(&sContext, ClrBlack);
      }
      GrStringDrawCentered(&sContext, "Time",     -1, w / 2, h / 2 - 4, 0);
      GrStringDrawCentered(&sContext, "stopped",  -1, w / 2, h / 2 + 4, 0);
    }
    n = nav;
  }
}
