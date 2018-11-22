/******************************************************************************/
/* Blinky.C: LED Flasher                                                      */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2007 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/
                  
#include <stdio.h>                         /* standard I/O .h-file */
#include <LPC213x.H>                       /* LPC213x definitions  */

#define BUT1 (!(IOPIN0 & (1<<15)))
#define BUT2 (!(IOPIN0 & (1<<14)))


extern int scrl_dir;                       /* Scrolling direction (1 = right, -1 = left) */
extern char intensity;                     /* Intensity 0..8, 8 - highest intensity */

extern void MatrixInit(void);              /* Initialize LED matrix */
extern void SerialInit (void);             /* Initialize Serial Interface */
extern int  SendStr (char *p);             /* Write string to Serial Port */


int main (void) {
  int i;
  unsigned int but1_ex = 0, but2_ex = 0;

  MatrixInit();                            /* Initialize LED Matrix */
  SerialInit();                            /* Initialize Serial Interface */
  SendStr("\nKEIL Demo for SILICA-LPC2103");  /* Output text on UART1 */

  while (1) {                              /* Loop forever */
    if (BUT1 & !but1_ex) {                 /* If BUT1 was pressed */
      but1_ex = BUT1;                      /* Remember last state of button */
      if (scrl_dir == 1) scrl_dir = -1;
      else               scrl_dir = 1;
      SendStr ("\nLeft button pressed -> change scroll direction");  /* Output text on UART1 */
      for (i=0; i<5000; i++);              /* Debounce */
    }
    if (!BUT1 & but1_ex) {                 /* If BUT1 was released */
      but1_ex = BUT1;                      /* Remember last state of button */
      for (i=0; i<5000; i++);              /* Debounce */
    }

    if (BUT2 & !but2_ex) {                 /* If BUT2 was pressed */
      but2_ex = BUT2;                      /* Remember last state of button */
      if (intensity == 0) intensity = 9;
      intensity--;
      SendStr ("\nRight button pressed -> change intensity");  /* Output text on UART1 */
      for (i=0; i<5000; i++);              /* Debounce */
    }
    if (!BUT2 & but2_ex) {                 /* If BUT2 was released */
      but2_ex = BUT2;                      /* Remember last state of button */
      for (i=0; i<5000; i++);              /* Debounce */
    }    
  }
}
