/*----------------------------------------------------------------------------
 * Name:    vcomdemo.c
 * Purpose: USB virtual COM port Demo
 * Version: V1.20
 *----------------------------------------------------------------------------
 * This file is part of the uVision/ARM development tools.
 * This software may only be used under the terms of a valid, current,
 * end user licence from KEIL for a compatible version of KEIL software
 * development tools. Nothing else gives you the right to use this software.
 *
 * This software is supplied "AS IS" without warranties of any kind.
 *
 * Copyright (c) 2008 Keil - An ARM Company. All rights reserved.
 *----------------------------------------------------------------------------*/

#include <LPC23xx.H>                        /* LPC23xx definitions */

#include "type.h"

#include "usb.h"
#include "usbcfg.h"
#include "usbhw.h"
#include "usbcore.h"
#include "cdc.h"
#include "cdcuser.h"

#include "serial.h"
#include "vcomdemo.h"
#include "LCD.h"


/*----------------------------------------------------------------------------
  Initialises the VCOM port.
  Call this function before using VCOM_putchar or VCOM_getchar
 *---------------------------------------------------------------------------*/
void VCOM_Init(void) {

  CDC_Init ();
}


/*----------------------------------------------------------------------------
  Reads character from serial port buffer and writes to USB buffer
 *---------------------------------------------------------------------------*/
void VCOM_Serial2Usb(void) {
  static char serBuf [USB_CDC_BUFSIZE];
         int  numBytesRead, numAvailByte;
	
  ser_AvailChar (&numAvailByte);
  if (numAvailByte > 0) {
    if (CDC_DepInEmpty) {
      numBytesRead = ser_Read (&serBuf[0], &numAvailByte);

      CDC_DepInEmpty = 0;
	  USB_WriteEP (CDC_DEP_IN, (unsigned char *)&serBuf[0], numBytesRead);
    }
  }

}

/*----------------------------------------------------------------------------
  Reads character from USB buffer and writes to serial port buffer
 *---------------------------------------------------------------------------*/
void VCOM_Usb2Serial(void) {
  static char serBuf [32];
         int  numBytesToRead, numBytesRead, numAvailByte;

  CDC_OutBufAvailChar (&numAvailByte);
  if (numAvailByte > 0) {
      numBytesToRead = numAvailByte > 32 ? 32 : numAvailByte; 
      numBytesRead = CDC_RdOutBuf (&serBuf[0], &numBytesToRead);
      ser_Write (&serBuf[0], &numBytesRead);    
  }

}


/*----------------------------------------------------------------------------
  checks the serial state and initiates notification
 *---------------------------------------------------------------------------*/
void VCOM_CheckSerialState (void) {
         unsigned short temp;
  static unsigned short serialState;

  temp = CDC_GetSerialState();
  if (serialState != temp) {
     serialState = temp;
     CDC_NotificationIn();                  /* send SERIAL_STATE notification */
  }
}


/*----------------------------------------------------------------------------
  Main Program
 *---------------------------------------------------------------------------*/
int main (void) {

  PINSEL10 = 0;                             /* Disable ETM interface */
  FIO2DIR |= LED_MSK;                       /* LEDs, port 2, bit 0~7 output only */

  lcd_init();
  lcd_clear();
  lcd_print ("MCB2300 USB VCOM");
  set_cursor (0, 1);
  lcd_print ("  www.keil.com  ");

  VCOM_Init();                              /* VCOM Initialization */

  USB_Init();                               /* USB Initialization */
  USB_Connect(__TRUE);                      /* USB Connect */

  while (!USB_Configuration) ;              /* wait until USB is configured */

  while (1) {                               /* Loop forever */
    VCOM_Serial2Usb();                      /* read serial port and initiate USB event */
    VCOM_CheckSerialState();
	VCOM_Usb2Serial();
  }											   
} 

