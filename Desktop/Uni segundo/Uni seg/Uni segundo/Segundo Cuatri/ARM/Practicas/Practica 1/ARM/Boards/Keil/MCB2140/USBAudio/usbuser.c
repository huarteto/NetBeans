/*----------------------------------------------------------------------------
 * U S B  -  K e r n e l
 *----------------------------------------------------------------------------
 * Name:    usbuser.c
 * Purpose: USB Custom User Module
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

#include "type.h"

#include "usb.h"
#include "usbcfg.h"
#include "usbhw.h"
#include "usbcore.h"
#include "usbuser.h"

#include "adcuser.h"
#include "demo.h"


/*
 *  USB Power Event Callback
 *   Called automatically on USB Power Event
 *    Parameter:       power: On(TRUE)/Off(FALSE)
 */

#if USB_POWER_EVENT
void USB_Power_Event (BOOL  power) {
}
#endif


/*
 *  USB Reset Event Callback
 *   Called automatically on USB Reset Event
 */

#if USB_RESET_EVENT
void USB_Reset_Event (void) {
  USB_ResetCore();
}
#endif


/*
 *  USB Suspend Event Callback
 *   Called automatically on USB Suspend Event
 */

#if USB_SUSPEND_EVENT
void USB_Suspend_Event (void) {
}
#endif


/*
 *  USB Resume Event Callback
 *   Called automatically on USB Resume Event
 */

#if USB_RESUME_EVENT
void USB_Resume_Event (void) {
}
#endif


/*
 *  USB Remote Wakeup Event Callback
 *   Called automatically on USB Remote Wakeup Event
 */

#if USB_WAKEUP_EVENT
void USB_WakeUp_Event (void) {
}
#endif


/*
 *  USB Start of Frame Event Callback
 *   Called automatically on USB Start of Frame Event
 */

#if USB_SOF_EVENT
void USB_SOF_Event (void) {
#if USB_DMA == 0
  if (USB_ReadEP(ADC_EP_OUT, (U8 *)&DataBuf[DataIn])) {
    /* Data Available */
    DataIn += P_S;                          /* Update Data In Index */
    DataIn &= B_S - 1;                      /* Adjust Data In Index */
    if (((DataIn - DataOut) & (B_S - 1)) == (B_S/2)) {
      DataRun = 1;                          /* Data Stream running */
    }
  } else {
    /* No Data */
    DataRun  = 0;                           /* Data Stream not running */
    DataOut  = DataIn;                      /* Initialize Data Indexes */
  }
#endif
}
#endif


/*
 *  USB Error Event Callback
 *   Called automatically on USB Error Event
 *    Parameter:       error: Error Code
 */

#if USB_ERROR_EVENT
void USB_Error_Event (DWORD error) {
}
#endif


/*
 *  USB Set Configuration Event Callback
 *   Called automatically on USB Set Configuration Request
 */

#if USB_CONFIGURE_EVENT
void USB_Configure_Event (void) {
}
#endif


/*
 *  USB Set Interface Event Callback
 *   Called automatically on USB Set Interface Request
 */

#if USB_INTERFACE_EVENT
void USB_Interface_Event (void) {
}
#endif


/*
 *  USB Set/Clear Feature Event Callback
 *   Called automatically on USB Set/Clear Feature Request
 */

#if USB_FEATURE_EVENT
void USB_Feature_Event (void) {
}
#endif


#define P_EP(n) ((USB_EP_EVENT & (1 << (n))) ? USB_EndPoint##n : NULL)

/* USB Endpoint Events Callback Pointers */
void (* const USB_P_EP[16]) (U32 event) = {
  P_EP(0),
  P_EP(1),
  P_EP(2),
  P_EP(3),
  P_EP(4),
  P_EP(5),
  P_EP(6),
  P_EP(7),
  P_EP(8),
  P_EP(9),
  P_EP(10),
  P_EP(11),
  P_EP(12),
  P_EP(13),
  P_EP(14),
  P_EP(15),
};


/*
 *  USB Endpoint 1 Event Callback
 *   Called automatically on USB Endpoint 1 Event
 *    Parameter:       event
 */

void USB_EndPoint1 (U32 event) {
}


/*
 *  USB Endpoint 2 Event Callback
 *   Called automatically on USB Endpoint 2 Event
 *    Parameter:       event
 */

void USB_EndPoint2 (U32 event) {
}


/*
 *  USB Endpoint 3 Event Callback
 *   Called automatically on USB Endpoint 3 Event
 *    Parameter:       event
 */

void USB_EndPoint3 (U32 event) {
#if USB_DMA
  USB_DMA_DESCRIPTOR DD;

  if (event & USB_EVT_OUT_DMA_EOT) {
    /* End of Transfer */
    if (USB_DMA_BufAdr(ADC_EP_OUT) != ((U32)DataBuf + 2*DataIn)) {
      /* Data Available */
      DataIn += P_C*P_S;                    /* Update Data In Index */
      DataIn &= B_S - 1;                    /* Adjust Data In Index */
      if (((DataIn - DataOut) & (B_S - 1)) == (B_S/2)) {
        DataRun = 1;                        /* Data Stream running */
      }
    } else {
      /* No Data */
      DataRun = 0;                          /* Data Stream not running */
      DataOut = DataIn;                     /* Initialize Data Indexes */
    }
  }
  if (event & (USB_EVT_OUT_DMA_EOT) | (USB_EVT_OUT_DMA_NDR)) {
    /* End of Transfer or New Descriptor Request */
    DD.BufAdr  = (U32)DataBuf + 2*DataIn;   /* DMA Buffer Address */
    DD.BufLen  = P_C;                       /* DMA Packet Count */
    DD.MaxSize = 0;                         /* Must be 0 for Iso Transfer */
    DD.InfoAdr = (U32)InfoBuf;              /* Packet Info Buffer Address */
    DD.Cfg.Val = 0;                         /* Initial DMA Configuration */
    DD.Cfg.Type.IsoEP = 1;                  /* Iso Endpoint */
    USB_DMA_Setup (ADC_EP_OUT, &DD);        /* Setup DMA */
    USB_DMA_Enable(ADC_EP_OUT);             /* Enable DMA */
  }
#endif
}


/*
 *  USB Endpoint 4 Event Callback
 *   Called automatically on USB Endpoint 4 Event
 *    Parameter:       event
 */

void USB_EndPoint4 (U32 event) {
}


/*
 *  USB Endpoint 5 Event Callback
 *   Called automatically on USB Endpoint 5 Event
 *    Parameter:       event
 */

void USB_EndPoint5 (U32 event) {
}


/*
 *  USB Endpoint 6 Event Callback
 *   Called automatically on USB Endpoint 6 Event
 *    Parameter:       event
 */

void USB_EndPoint6 (U32 event) {
}


/*
 *  USB Endpoint 7 Event Callback
 *   Called automatically on USB Endpoint 7 Event
 *    Parameter:       event
 */

void USB_EndPoint7 (U32 event) {
}


/*
 *  USB Endpoint 8 Event Callback
 *   Called automatically on USB Endpoint 8 Event
 *    Parameter:       event
 */

void USB_EndPoint8 (U32 event) {
}


/*
 *  USB Endpoint 9 Event Callback
 *   Called automatically on USB Endpoint 9 Event
 *    Parameter:       event
 */

void USB_EndPoint9 (U32 event) {
}


/*
 *  USB Endpoint 10 Event Callback
 *   Called automatically on USB Endpoint 10 Event
 *    Parameter:       event
 */

void USB_EndPoint10 (U32 event) {
}


/*
 *  USB Endpoint 11 Event Callback
 *   Called automatically on USB Endpoint 11 Event
 *    Parameter:       event
 */

void USB_EndPoint11 (U32 event) {
}


/*
 *  USB Endpoint 12 Event Callback
 *   Called automatically on USB Endpoint 12 Event
 *    Parameter:       event
 */

void USB_EndPoint12 (U32 event) {
}


/*
 *  USB Endpoint 13 Event Callback
 *   Called automatically on USB Endpoint 13 Event
 *    Parameter:       event
 */

void USB_EndPoint13 (U32 event) {
}


/*
 *  USB Endpoint 14 Event Callback
 *   Called automatically on USB Endpoint 14 Event
 *    Parameter:       event
 */

void USB_EndPoint14 (U32 event) {
}


/*
 *  USB Endpoint 15 Event Callback
 *   Called automatically on USB Endpoint 15 Event
 *    Parameter:       event
 */

void USB_EndPoint15 (U32 event) {
}
