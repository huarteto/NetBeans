/*----------------------------------------------------------------------------
 * U S B  -  K e r n e l
 *----------------------------------------------------------------------------
 * Name:    usbhw.c
 * Purpose: USB Hardware Layer Module for Atmel AT91SAM3S
 * Version: V1.20
 *----------------------------------------------------------------------------
 * This file is part of the uVision/ARM development tools.
 * This software may only be used under the terms of a valid, current,
 * end user licence from KEIL for a compatible version of KEIL software
 * development tools. Nothing else gives you the right to use this software.
 *
 * This software is supplied "AS IS" without warranties of any kind.
 *
 * Copyright (c) 2008 - 2010 Keil - An ARM Company. All rights reserved.
 *----------------------------------------------------------------------------
 * History:
 *          V1.20 Added USB_ClearEPBuf
 *          V1.11 changed optimizaton level
 *          V1.10 Double buffer handling corrected
 *          V1.00 Initial Version
 *----------------------------------------------------------------------------*/

#include <SAM3S.H>                                     /* ATSAM3S definitions */
#include "type.h"

#include "usb.h"
#include "usbcfg.h"
#include "usbhw.h"
#include "usbcore.h"
#include "usbuser.h"


const U8  DualBankEP = 0xF6;                           /* Dual Bank Endpoint Bit Mask */

const U32 RX_DATA_BK[2] = {
  UDP_CSR_RX_DATA_BK0,
  UDP_CSR_RX_DATA_BK1
};


U8  RxDataBank[USB_EP_NUM];
U8  TxDataBank[USB_EP_NUM];


/*
 *  Set / Clear functions to modify UDP_CSR register
 *   
 *  see UserManual ATSAM3S
 */

/* Bitmap for all status bits in CSR that are not effected by a value 1. */
#define REG_NO_EFFECT_1_ALL    UDP_CSR_RX_DATA_BK0\
                             | UDP_CSR_RX_DATA_BK1\
                             | UDP_CSR_STALLSENTISOERROR\
                             | UDP_CSR_RXSETUP\
                             | UDP_CSR_TXCOMP

static void USB_SetCSR(U32 EPNum, U32 flags) {
  volatile U32 csr, timeOutCnt = 0;
           U32 ep;

  ep = EPNum & 0x0F;                   /* get endpoint number    */

  csr  = UDP->UDP_CSR[ep] ;
  csr |= REG_NO_EFFECT_1_ALL;
  csr |= (flags);

  UDP->UDP_CSR[ep] = csr;
  while ( (UDP->UDP_CSR[ep] & (flags)) != (flags))
  {
    if (timeOutCnt++ > 100) break;
  }
}

static void USB_ClrCSR(U32 EPNum, U32 flags) {
  volatile U32 csr, timeOutCnt = 0;
           U32 ep;

  ep = EPNum & 0x0F;                   /* get endpoint number    */

  csr  = UDP->UDP_CSR[ep];
  csr |= REG_NO_EFFECT_1_ALL;
  csr &= ~(flags);

  UDP->UDP_CSR[ep] = csr;
  while ( (UDP->UDP_CSR[ep] & (flags)) == (flags))
  {
    if (timeOutCnt++ > 100) break;
  }
}


/*
 *  Retrieve maximum EP size Function
 *   Called during EndPoint configuration
 *    Return Value:    maximum size for given EP
 */

static int USB_GetSizeEP (U32 EPNum) {
  U32 ep, epSize;

  ep = EPNum & 0x0F;                   /* get endpoint number    */
  switch (ep) {
    case 0:
    case 1:
    case 2:
    case 3:
      epSize =    64;                  /* maximum size   64 Byte */
      break;
    case 4:
    case 5:
      epSize =   512;                  /* maximum size  512 Byte */
      break;
    case 6:
    case 7:
      epSize =   256;                  /* maximum size  256 Byte */
      break;
    default:
      epSize =     0;                  /* maximum size    0 Byte */
      break;
  }

  return (epSize);
}


/*
 *  USB Initialize Function
 *   Called by the User to initialize USB
 *    Return Value:    None
 */

void USB_Init (void) {

  /* Enables the 48MHz USB Clock UDPCK and System Peripheral USB Clock */
                                                       /* PLLB is configured for 96MHz     */
  PMC->PMC_PCER1 = PMC_PCER1_PID34;                    /* enable master clock for UPD      */
  PMC->PMC_USB   = ((0x01 << 8) |                      /* USBDIV = 1  (96MHz / (USBDIV+1)) */ 
                    (0x01 << 0)  );                    /* USB Clock Input is PLLB          */

  UDP->UDP_TXVC &= ~UDP_TXVC_PUON;                     /* disconnect pullup on DP          */

  PMC->PMC_SCER  =  PMC_SCER_UDP;                      /* enable 48MHz clock for UDP       */

  NVIC_EnableIRQ(UDP_IRQn);                            /* enable USB interrupt             */

}


/*
 *  USB Connect Function
 *   Called by the User to Connect/Disconnect USB
 *    Parameters:      con:   Connect/Disconnect
 *    Return Value:    None
 */

void USB_Connect (BOOL con) {

  if (con) {
    UDP->UDP_TXVC &= ~UDP_TXVC_TXVDIS;                 /*  enable Transciever              */
    UDP->UDP_TXVC |=  UDP_TXVC_PUON;                   /*    connect pullup on DP          */
  } else {
    UDP->UDP_TXVC &= ~UDP_TXVC_PUON;                   /* disconnect pullup on DP          */
    UDP->UDP_TXVC |=  UDP_TXVC_TXVDIS;                 /* disable Transciever              */
  }
}


/*
 *  USB Reset Function
 *   Called automatically on USB Reset
 *    Return Value:    None
 */

void USB_Reset (void) {
  U32 ep;

  /* Global USB Reset */
  UDP->UDP_GLB_STAT  =   0;                            /* reset global status to 0         */
  UDP->UDP_FADDR     =   UDP_FADDR_FEN;                /* set address to 0                 */
  UDP->UDP_ICR       =   0xFFFFFFFF;                   /* clear all pending interrupts     */

  /* Reset & Disable USB Endpoints */
  for (ep = 0; ep < USB_EP_NUM; ep++) {
    UDP->UDP_CSR[ep] = 0;
    RxDataBank[ep] = 0;
    TxDataBank[ep] = 0;
  }
  UDP->UDP_RST_EP = 0xFFFFFFFF;
  UDP->UDP_RST_EP = 0;

  /* Setup USB Interrupts */
  UDP->UDP_IER = UDP_IER_RXSUSP | 
                 UDP_IER_RXRSM  | 
                 UDP_IER_EXTRSM |
                 UDP_IER_SOFINT ;

  /* Setup Control Endpoint 0 */
  UDP->UDP_IER = (1 << 0);                             /* enable EP0 interrupts */ 
  USB_SetCSR(0, (UDP_CSR_EPEDS | UDP_CSR_EPTYPE_CTRL));
}


/*
 *  USB Suspend Function
 *   Called automatically on USB Suspend
 *    Return Value:    None
 */

void USB_Suspend (void) {
  /* Performed by Hardware */
}


/*
 *  USB Resume Function
 *   Called automatically on USB Resume
 *    Return Value:    None
 */

void USB_Resume (void) {
  /* Performed by Hardware */
}


/*
 *  USB Remote Wakeup Function
 *   Called automatically on USB Remote Wakeup
 *    Return Value:    None
 */

void USB_WakeUp (void) {
  /* Performed by Hardware */
}


/*
 *  USB Remote Wakeup Configuration Function
 *    Parameters:      cfg:   Enable/Disable
 *    Return Value:    None
 */

void USB_WakeUpCfg (BOOL cfg) {

  if (cfg) {
    UDP->UDP_GLB_STAT |=  UDP_GLB_STAT_RMWUPE;
  } else {
    UDP->UDP_GLB_STAT &= ~UDP_GLB_STAT_RMWUPE;
  }
}


/*
 *  USB Set Address Function
 *    Parameters:      adr:   USB Address
 *    Return Value:    None
 */

void USB_SetAddress (U32 adr) {

  UDP->UDP_FADDR = UDP_FADDR_FEN | (adr & UDP_FADDR_FADD); /* set the device address  */
  if (adr) {                                           /* if address is non-zero      */
    UDP->UDP_GLB_STAT |=  UDP_GLB_STAT_FADDEN;         /* device enters address state */
  } else {
    UDP->UDP_GLB_STAT &= ~UDP_GLB_STAT_FADDEN;         /* device enters default state */
  }
}


/*
 *  USB Configure Function
 *    Parameters:      cfg:   Configure/Deconfigure
 *    Return Value:    None
 */

void USB_Configure (BOOL cfg) {

  if (cfg) {                                           /* if configuration is non-zero   */
    UDP->UDP_GLB_STAT |=  UDP_GLB_STAT_CONFG;          /* device enters configured state */
  } else {
    UDP->UDP_GLB_STAT &= ~UDP_GLB_STAT_CONFG;          /* device clears configured state */
  }
}


/*
 *  Configure USB Endpoint according to Descriptor
 *    Parameters:      pEPD:  Pointer to Endpoint Descriptor
 *    Return Value:    None
 */

void USB_ConfigEP (USB_ENDPOINT_DESCRIPTOR *pEPD) {
  U32 ep, dir, type, csr;

  ep       = pEPD->bEndpointAddress & 0x0F;
  type     = pEPD->bmAttributes     & USB_ENDPOINT_TYPE_MASK;
  dir      = pEPD->bEndpointAddress >> 7;
  csr      = ((type | (dir << 2)) << 8);

  /* check if MaxPacketSize fits for EndPoint */
  if (pEPD->wMaxPacketSize <= USB_GetSizeEP(ep)) {
    USB_SetCSR(ep, csr);                               /* configure the EP */
  }
}


/*
 *  Set Direction for USB Control Endpoint
 *    Parameters:      dir:   Out (dir == 0), In (dir <> 0)
 *    Return Value:    None
 */

void USB_DirCtrlEP (U32 dir) {

  if (dir ) {
    USB_SetCSR(0, UDP_CSR_DIR);
  } else {
    USB_ClrCSR(0, UDP_CSR_DIR);
  }
  USB_ClrCSR(0, UDP_CSR_RXSETUP);
}

/*
 *  Enable USB Endpoint
 *    Parameters:      EPNum: Endpoint Number
 *                       EPNum.0..3: Address
 *                       EPNum.7:    Dir
 *    Return Value:    None
 */

void USB_EnableEP (U32 EPNum) {
  U32 ep;

  ep = EPNum & 0x0F;                                   /* get endpoint number    */
  UDP->UDP_IER = (1 << ep);                            /* enable EP interrupts   */ 
  USB_SetCSR(ep, UDP_CSR_EPEDS);
}


/*
 *  Disable USB Endpoint
 *    Parameters:      EPNum: Endpoint Number
 *                       EPNum.0..3: Address
 *                       EPNum.7:    Dir
 *    Return Value:    None
 */

void USB_DisableEP (U32 EPNum) {
  U32 ep;

  ep = EPNum & 0x0F;                                   /* get endpoint number    */
  USB_ClrCSR(ep, UDP_CSR_EPEDS);
  UDP->UDP_IDR = (1 << ep);                            /* disable EP interrupts  */ 
}


/*
 *  Reset USB Endpoint
 *    Parameters:      EPNum: Endpoint Number
 *                       EPNum.0..3: Address
 *                       EPNum.7:    Dir
 *    Return Value:    None
 */

void USB_ResetEP (U32 EPNum) {
  U32 ep;

  ep = EPNum & 0x0F;                                   /* get endpoint number    */
  USB_ClrCSR(ep, (UDP_CSR_TXCOMP      | UDP_CSR_RXSETUP      |
                  UDP_CSR_RX_DATA_BK0 | UDP_CSR_RX_DATA_BK1  |
                                        UDP_CSR_FORCESTALL   |
                  UDP_CSR_STALLSENTISOERROR                   ));

  if (UDP->UDP_CSR[ep] & UDP_CSR_TXPKTRDY) {
    USB_ClrCSR(ep, UDP_CSR_TXPKTRDY);

    if (DualBankEP & (1 << ep)) {
      USB_SetCSR(ep, UDP_CSR_TXPKTRDY);
      USB_ClrCSR(ep, UDP_CSR_TXPKTRDY);
    }
}

  UDP->UDP_RST_EP  |=  (1 << ep);
  UDP->UDP_RST_EP  &= ~(1 << ep);
//  RxDataBank[ep] =   0;              /* reset does not force that Bank0 is the next to receive */
//  TxDataBank[ep] =   0;              /* therefore we use the the bank which is next to receive */
}


/*
 *  Set Stall for USB Endpoint
 *    Parameters:      EPNum: Endpoint Number
 *                       EPNum.0..3: Address
 *                       EPNum.7:    Dir
 *    Return Value:    None
 */

void USB_SetStallEP (U32 EPNum) {
  U32 ep;

  ep = EPNum & 0x0F;                                   /* get endpoint number    */
  USB_SetCSR(ep, UDP_CSR_FORCESTALL);
}


/*
 *  Clear Stall for USB Endpoint
 *    Parameters:      EPNum: Endpoint Number
 *                       EPNum.0..3: Address
 *                       EPNum.7:    Dir
 *    Return Value:    None
 */

void USB_ClrStallEP (U32 EPNum) {
  U32 ep;

  ep = EPNum & 0x0F;                                   /* get endpoint number    */
  USB_ClrCSR(ep, UDP_CSR_FORCESTALL);

  UDP->UDP_RST_EP  |=  (1 << ep);
  UDP->UDP_RST_EP  &= ~(1 << ep);
}


/*
 *  Clear USB Endpoint Buffer
 *    Parameters:      EPNum: Endpoint Number
 *                       EPNum.0..3: Address
 *                       EPNum.7:    Dir
 *    Return Value:    None
 */

void USB_ClearEPBuf (U32 EPNum) {
  U32 ep;

  ep = EPNum & 0x0F;                                   /* get endpoint number    */

  UDP->UDP_RST_EP  |=  (1 << ep);
  UDP->UDP_RST_EP  &= ~(1 << ep);
}


/*
 *  Read USB Endpoint Data
 *    Parameters:      EPNum: Endpoint Number
 *                       EPNum.0..3: Address
 *                       EPNum.7:    Dir
 *                     pData: Pointer to Data Buffer
 *    Return Value:    Number of bytes read
 */

U32 USB_ReadEP (U32 EPNum, U8 *pData) {
  U32 cnt, n;
  U32 ep;

  ep = EPNum & 0x0F;                                   /* get endpoint number    */

  cnt = (UDP->UDP_CSR[ep] >> 16) & 0x07FF;
  for (n = 0; n < cnt; n++) {
    *pData++ = (U8)UDP->UDP_FDR[ep];
  }

  return (cnt);
}


/*
 *  Write USB Endpoint Data
 *    Parameters:      EPNum: Endpoint Number
 *                       EPNum.0..3: Address
 *                       EPNum.7:    Dir
 *                     pData: Pointer to Data Buffer
 *                     cnt:   Number of bytes to write
 *    Return Value:    Number of bytes written
 */

U32 USB_WriteEP (U32 EPNum, U8 *pData, U32 cnt) {
  U32 n;
  U32 ep;

  ep = EPNum & 0x0F;                                   /* get endpoint number    */

  if (UDP->UDP_CSR[ep] & UDP_CSR_FORCESTALL) {         /* do not send if EP is stalled */
    return (cnt);
  }

  if (TxDataBank[ep] != 0) {                           /* bank0 or bank1 ready ? */
    return (0);                                        /* no bank ready */
  }

  if (UDP->UDP_CSR[ep] & UDP_CSR_TXPKTRDY) {           /* bank 0 not ready ? */ 
    if (DualBankEP & (1 << ep)) {                      /* dualbank available ? */
      TxDataBank[ep] = 1;                              /* select bank1 */
    }
    else {
      return (0);                                      /* no bank ready */
    }
  }

  for (n = 0; n < cnt; n++) {
    UDP->UDP_FDR[ep] = *pData++;                       /* write data to fifo */
  }

  if (TxDataBank[ep] == 0) {
    USB_SetCSR(ep, UDP_CSR_TXPKTRDY);
  }

  return (cnt);
}


/*
 *  Get USB Last Frame Number
 *    Parameters:      None
 *    Return Value:    Frame Number
 */

U32 USB_GetFrame (void) {
  U32 val;

  while ((UDP->UDP_FRM_NUM & (UDP_FRM_NUM_FRM_OK | UDP_FRM_NUM_FRM_ERR)) == 0);
  if (UDP->UDP_FRM_NUM & UDP_FRM_NUM_FRM_OK) {
    val = UDP->UDP_FRM_NUM & UDP_FRM_NUM_FRM_NUM;
  } else {
    val = 0xFFFFFFFF;
  }

  return (val);
}


/*
 *  USB Interrupt Service Routine
 */

void UDP_IRQHandler (void) {
  U32 isr, csr, bkm, n;

  isr = UDP->UDP_ISR;                                  /* read UDP Interrupt Status Register */

  /* End of Bus Reset Interrupt */
  if (isr & UDP_ISR_ENDBUSRES) {
    USB_Reset();
#if USB_RESET_EVENT
    USB_Reset_Event();
#endif
    UDP->UDP_ICR = UDP_ISR_ENDBUSRES;
  }

  /* USB Suspend Interrupt */
  if (isr == UDP_ISR_RXSUSP) {                         /* acc. ATMEL documentation */ 
    UDP->UDP_ICR = UDP_ISR_RXSUSP;                     /* has to handled at last  */
    USB_Suspend();
#if USB_SUSPEND_EVENT
    USB_Suspend_Event();
#endif
  }

  /* USB Resume Interrupt */
  if (isr & UDP_ISR_RXRSM) {
    USB_Resume();
#if USB_RESUME_EVENT
    USB_Resume_Event();
#endif
    UDP->UDP_ICR = UDP_ISR_RXRSM;
  }

  /* External Resume Interrupt */
  if (isr & UDP_ISR_EXTRSM) {
    USB_WakeUp();
#if USB_WAKEUP_EVENT
    USB_WakeUp_Event();
#endif
    UDP->UDP_ICR = UDP_ISR_EXTRSM;
  }

  /* Start of Frame Interrupt */
  if (isr & UDP_ISR_SOFINT) {
#if USB_SOF_EVENT
    USB_SOF_Event();
#endif
    UDP->UDP_ICR = UDP_ISR_SOFINT;
  }

  /* Endpoint Interrupts */
  for (n = 0; n < USB_EP_NUM; n++) {
    if (isr & (1 << n)) {

      csr = UDP->UDP_CSR[n];

      /* Setup Packet Received Interrupt */
      if (csr & UDP_CSR_RXSETUP) {
        if (USB_P_EP[n]) {
          USB_P_EP[n](USB_EVT_SETUP);
        }
        /* Setup Flag is already cleared in USB_DirCtrlEP */
        /* UDP->UDP_CSR[n] &= ~AT91C_UDP_RXSETUP; */
      }

      /* Data Packet Received Interrupt */
      bkm = RX_DATA_BK[RxDataBank[n]];
      if (csr & bkm) {
        if (USB_P_EP[n]) {
          USB_P_EP[n](USB_EVT_OUT);
        }
        USB_ClrCSR(n, bkm);
        if (DualBankEP & (1 << n)) {
          RxDataBank[n] ^= 1;
        }
      }

      /* Data Packet Sent Interrupt */
      if (csr & UDP_CSR_TXCOMP) {
        if (TxDataBank[n]) {
          USB_SetCSR(n, UDP_CSR_TXPKTRDY);
          TxDataBank[n] = 0;
        }
        if (USB_P_EP[n]) {
          USB_P_EP[n](USB_EVT_IN);
        }
        USB_ClrCSR(n, UDP_CSR_TXCOMP);
      }

      /* STALL Packet Sent Interrupt */
      if (csr & UDP_CSR_STALLSENTISOERROR) {
        if ((csr & UDP_CSR_EPTYPE) == UDP_CSR_EPTYPE_CTRL) {
          if (USB_P_EP[n]) {
            USB_P_EP[n](USB_EVT_IN_STALL);
/*          USB_P_EP[n](USB_EVT_OUT_STALL); */
          }
        }
        USB_ClrCSR(n, UDP_CSR_STALLSENTISOERROR);
      }

    }
  }

}
