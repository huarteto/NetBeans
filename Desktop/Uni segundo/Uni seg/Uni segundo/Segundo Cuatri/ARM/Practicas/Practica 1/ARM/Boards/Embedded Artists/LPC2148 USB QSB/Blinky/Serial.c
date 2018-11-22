/******************************************************************************/
/* Serial.c: Low Level Serial Routines                                        */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2007 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/
// -------- <<< Use Configuration Wizard in Context Menu >>> -----------------


#include <LPC21xx.H>                    /* LPC21xx definitions                */

// <o> Use UART <0-1>
#define UART 0

// <h> Setup Communication Speed
//   <o> VPB Clock <0-60000000>
#define VPB_CLOCK 15000000

//   <o> Baudrate 
//        <0=>     600 Bauds
//        <1=>    1200 Bauds
//        <2=>    2400 Bauds
//        <3=>    4800 Bauds
//        <4=>    9600 Bauds
//        <5=>   14400 Bauds
//        <6=>   19200 Bauds
//        <7=>   28800 Bauds
//        <8=>   38400 Bauds
//        <9=>   56000 Bauds
//        <10=>  57600 Bauds
//        <11=> 115200 Bauds
#define BAUD_ID 11
#define BAUD_RATE \
  ((BAUD_ID ==  0) ?    600 : \
   (BAUD_ID ==  1) ?   1200 : \
   (BAUD_ID ==  2) ?   2400 : \
   (BAUD_ID ==  3) ?   4800 : \
   (BAUD_ID ==  4) ?   9600 : \
   (BAUD_ID ==  5) ?  14400 : \
   (BAUD_ID ==  6) ?  19200 : \
   (BAUD_ID ==  7) ?  28800 : \
   (BAUD_ID ==  8) ?  38400 : \
   (BAUD_ID ==  9) ?  56000 : \
   (BAUD_ID == 10) ?  57600 : \
   (BAUD_ID >= 11) ? 115200 : 115200)

// </h>


#if (UART == 0)
  #define UxPINS  0x00000005
  #define UxRBR   U0RBR
  #define UxTHR   U0THR
  #define UxLCR   U0LCR
  #define UxLSR   U0LSR
  #define UxDLL   U0DLL
  #define UxDLM   U0DLM
#else
  #define UxPINS  0x00050000
  #define UxRBR   U1RBR
  #define UxTHR   U1THR
  #define UxLCR   U1LCR
  #define UxLSR   U1LSR
  #define UxDLL   U1DLL
  #define UxDLM   U1DLM
#endif

void init_serial (void)  {              /* Initialize Serial Interface        */
  PINSEL0 = UxPINS;                     /* Enable RxD and TxD pins            */
  UxLCR = 0x83;                         /* 8 bits, no Parity, 1 Stop bit      */
  UxDLL = (VPB_CLOCK/16/BAUD_RATE) & 0xFF;  /* Setup Baudrate                 */
  UxDLM = ((VPB_CLOCK/16/BAUD_RATE) >> 8) & 0xFF;
  UxLCR = 0x03;                         /* DLAB = 0                           */
}


/* implementation of putchar (also used by printf function to output data)    */
int sendchar (int ch)  {                /* Write character to Serial Port     */

  while (!(UxLSR & 0x20));

  return (UxTHR = ch);
}


int getkey (void)  {                    /* Read character from Serial Port    */

  while (!(UxLSR & 0x01));

  return (UxRBR);
}
