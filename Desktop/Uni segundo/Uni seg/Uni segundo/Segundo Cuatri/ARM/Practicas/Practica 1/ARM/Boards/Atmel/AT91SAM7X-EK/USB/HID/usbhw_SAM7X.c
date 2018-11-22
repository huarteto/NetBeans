/*----------------------------------------------------------------------------
 * U S B  -  K e r n e l
 *----------------------------------------------------------------------------
 * Name:    usbhw.c
 * Purpose: USB Hardware Layer Module for Atmel AT91SAM7X
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

#include <AT91SAM7X256.H>                         /* AT91SAM7X256 definitions */
#include "type.h"

#include "usb.h"
#include "usbcfg.h"
#include "usbhw.h"
#include "usbcore.h"
#include "usbuser.h"


const U8  DualBankEP = 0x06;                      /* Dual Bank Endpoint Bit Mask */

const U32 RX_DATA_BK[2] = {
  AT91C_UDP_RX_DATA_BK0,
  AT91C_UDP_RX_DATA_BK1
};


AT91PS_UDP UDP = AT91C_BASE_UDP;                  /* Global UDP Pointer */

U8  RxDataBank[USB_EP_NUM];
U8  TxDataBank[USB_EP_NUM];


/*
 *  Set / Clear functions to modify UDP_CSR register
 *   
 *  see UserManual ATSAM3S
 */

/* Bitmap for all status bits in CSR that are not effected by a value 1. */
#define REG_NO_EFFECT_1_ALL    AT91C_UDP_RX_DATA_BK0\
                             | AT91C_UDP_RX_DATA_BK1\
                             | AT91C_UDP_STALLSENT\
                             | AT91C_UDP_RXSETUP\
                             | AT91C_UDP_TXCOMP

static void USB_SetCSR(U32 EPNum, U32 flags) {
  volatile U32 csr, timeOutCnt = 0;
           U32 ep;

  ep = EPNum & 0x0F;                              /* get endpoint number    */

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

  ep = EPNum & 0x0F;                              /* get endpoint number    */

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

  ep = EPNum & 0x0F;                              /* get endpoint number    */
  switch (ep) {
    case 0:
      epSize =     8;                             /* maximum size    8 Byte */
      break;
    case 1:
    case 2:
    case 3:
      epSize =    64;                             /* maximum size   64 Byte */
      break;
    case 4:
    case 5:
      epSize =   256;                             /* maximum size  256 Byte */
      break;
    default:
      epSize =     0;                             /* maximum size    0 Byte */
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
  AT91C_BASE_PMC->PMC_SCER = AT91C_PMC_UDP;
  AT91C_BASE_PMC->PMC_PCER = (1 << AT91C_ID_UDP);

  /* Global USB Interrupt: Mode and Vector with Highest Priority and Enable */
  AT91C_BASE_AIC->AIC_SMR[AT91C_ID_UDP] = AT91C_AIC_SRCTYPE_INT_POSITIVE_EDGE |
                                          AT91C_AIC_PRIOR_HIGHEST;
  AT91C_BASE_AIC->AIC_SVR[AT91C_ID_UDP] = (unsigned long) USB_ISR;
  AT91C_BASE_AIC->AIC_IECR = (1 << AT91C_ID_UDP);

}


/*
 *  USB Connect Function
 *   Called by the User to Connect/Disconnect USB
 *    Parameters:      con:   Connect/Disconnect
 *    Return Value:    None
 */

void USB_Connect (BOOL con) {

  if (con) {
    UDP->UDP_TXVC &= ~AT91C_UDP_TXVDIS;           /*  enable Transciever */
  } else {
    UDP->UDP_TXVC |=  AT91C_UDP_TXVDIS;           /* disable Transciever */
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
  UDP->UDP_GLBSTATE  =   0;                       /* reset global status to 0     */
  UDP->UDP_FADDR     =   AT91C_UDP_FEN;           /* set address to 0             */
  UDP->UDP_ICR       =   0xFFFFFFFF;              /* clear all pending interrupts */

  /* Reset & Disable USB Endpoints */
  for (ep = 0; ep < USB_EP_NUM; ep++) {
    UDP->UDP_CSR[ep] = 0;
    RxDataBank[ep] = 0;
    TxDataBank[ep] = 0;
  }
  UDP->UDP_RSTEP = 0xFFFFFFFF;
  UDP->UDP_RSTEP = 0;

  /* Setup USB Interrupts */
  UDP->UDP_IER = AT91C_UDP_RXSUSP | 
                 AT91C_UDP_RXRSM  | 
                 AT91C_UDP_EXTRSM |
                 AT91C_UDP_SOFINT ;

  /* Setup Control Endpoint 0 */
  UDP->UDP_IER = (1 << 0);                        /* enable EP0 interrupts */ 
  USB_SetCSR(0, (AT91C_UDP_EPEDS | AT91C_UDP_EPTYPE_CTRL));
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
    UDP->UDP_GLBSTATE |=  AT91C_UDP_RMWUPE;
  } else {
    UDP->UDP_GLBSTATE &= ~AT91C_UDP_RMWUPE;
  }
}


/*
 *  USB Set Address Function
 *    Parameters:      adr:   USB Address
 *    Return Value:    None
 */

void USB_SetAddress (U32 adr) {

  UDP->UDP_FADDR = AT91C_UDP_FEN | (adr & AT91C_UDP_FADD); /* set the device address  */
  if (adr) {                                      /* if address is non-zero      */
    UDP->UDP_GLBSTATE |=  AT91C_UDP_FADDEN;       /* device enters address state */
  } else {
    UDP->UDP_GLBSTATE &= ~AT91C_UDP_FADDEN;       /* device enters default state */
  }
}


/*
 *  USB Configure Function
 *    Parameters:      cfg:   Configure/Deconfigure
 *    Return Value:    None
 */

void USB_Configure (BOOL cfg) {

  if (cfg) {                                      /* if configuration is non-zero   */
    UDP->UDP_GLBSTATE |=  AT91C_UDP_CONFG;        /* device enters configured state */
  } else {
    UDP->UDP_GLBSTATE &= ~AT91C_UDP_CONFG;        /* device clears configured state */
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
    USB_SetCSR(ep, csr);                          /* configure the EP */
  }
}


/*
 *  Set Direction for USB Control Endpoint
 *    Parameters:      dir:   Out (dir == 0), In (dir <> 0)
 *    Return Value:    None
 */

void USB_DirCtrlEP (U32 dir) {

  if (dir ) {
    USB_SetCSR(0, AT91C_UDP_DIR);
  } else {
    USB_ClrCSR(0, AT91C_UDP_DIR);
  }
  USB_ClrCSR(0, AT91C_UDP_RXSETUP);
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

  ep = EPNum & 0x0F;                              /* get endpoint number    */
  UDP->UDP_IER = (1 << ep);                       /* enable EP interrupts */ 
  USB_SetCSR(ep, AT91C_UDP_EPEDS);
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

  ep = EPNum & 0x0F;                              /* get endpoint number    */
  USB_ClrCSR(ep, AT91C_UDP_EPEDS);
  UDP->UDP_IDR = (1 << ep);                       /* disable EP0 interrupts */ 
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

  ep = EPNum & 0x0F;                              /* get endpoint number    */
  USB_ClrCSR(ep, (AT91C_UDP_TXCOMP      | AT91C_UDP_RXSETUP      |
                  AT91C_UDP_RX_DATA_BK0 | AT91C_UDP_RX_DATA_BK1  |
                                          AT91C_UDP_FORCESTALL   |
                  AT91C_UDP_STALLSENT                             ));

  if (UDP->UDP_CSR[ep] & AT91C_UDP_TXPKTRDY) {
    USB_ClrCSR(ep, AT91C_UDP_TXPKTRDY);

    if (DualBankEP & (1 << ep)) {
      USB_SetCSR(ep, AT91C_UDP_TXPKTRDY);
      USB_ClrCSR(ep, AT91C_UDP_TXPKTRDY);
    }
}

  UDP->UDP_RSTEP  |=  (1 << ep);
  UDP->UDP_RSTEP  &= ~(1 << ep);
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

  ep = EPNum & 0x0F;                              /* get endpoint number    */
  USB_SetCSR(ep, AT91C_UDP_FORCESTALL);
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

  ep = EPNum & 0x0F;                              /* get endpoint number    */
  USB_ClrCSR(ep, AT91C_UDP_FORCESTALL);

  UDP->UDP_RSTEP  |=  (1 << ep);
  UDP->UDP_RSTEP  &= ~(1 << ep);
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

  ep = EPNum & 0x0F;                              /* get endpoint number    */

  UDP->UDP_RSTEP  |=  (1 << ep);
  UDP->UDP_RSTEP  &= ~(1 << ep);
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

  ep = EPNum & 0x0F;                              /* get endpoint number    */

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

  ep = EPNum & 0x0F;                              /* get endpoint number    */

  if (UDP->UDP_CSR[ep] & AT91C_UDP_FORCESTALL) {  /* do not send if EP is stalled */
    return (cnt);
  }

  if (TxDataBank[ep] != 0) {                      /* bank0 or bank1 ready ? */
    return (0);                                   /* no bank ready */
  }

  if (UDP->UDP_CSR[ep] & AT91C_UDP_TXPKTRDY) {    /* bank 0 not ready ? */ 
    if (DualBankEP & (1 << ep)) {                 /* dualbank available ? */
      TxDataBank[ep] = 1;                         /* select bank1 */
    }
    else {
      return (0);                                 /* no bank ready */
    }
  }

  for (n = 0; n < cnt; n++) {
    UDP->UDP_FDR[ep] = *pData++;                  /* write data to fifo */
  }

  if (TxDataBank[ep] == 0) {
    USB_SetCSR(ep, AT91C_UDP_TXPKTRDY);
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

  while ((UDP->UDP_NUM & (AT91C_UDP_FRM_OK | AT91C_UDP_FRM_ERR)) == 0);
  if (UDP->UDP_NUM & AT91C_UDP_FRM_OK) {
    val = UDP->UDP_NUM & AT91C_UDP_FRM_NUM;
  } else {
    val = 0xFFFFFFFF;
  }

  return (val);
}


/*
 *  USB Interrupt Service Routine
 */

void USB_ISR (void) __irq {
  U32 isr, csr, bkm, n;

  isr = UDP->UDP_ISR;                             /* read UDP Interrupt Status Register */

  /* End of Bus Reset Interrupt */
  if (isr & AT91C_UDP_ENDBUSRES) {
    USB_Reset();
#if USB_RESET_EVENT
    USB_Reset_Event();
#endif
    UDP->UDP_ICR = AT91C_UDP_ENDBUSRES;
  }

  /* USB Suspend Interrupt */
  if (isr == AT91C_UDP_RXSUSP) {                  /* acc. ATMEL documentation */ 
    UDP->UDP_ICR = AT91C_UDP_RXSUSP;              /* has to handled at last  */
    USB_Suspend();
#if USB_SUSPEND_EVENT
    USB_Suspend_Event();
#endif
  }

  /* USB Resume Interrupt */
  if (isr & AT91C_UDP_RXRSM) {
    USB_Resume();
#if USB_RESUME_EVENT
    USB_Resume_Event();
#endif
    UDP->UDP_ICR = AT91C_UDP_RXRSM;
  }

  /* External Resume Interrupt */
  if (isr & AT91C_UDP_EXTRSM) {
    USB_WakeUp();
#if USB_WAKEUP_EVENT
    USB_WakeUp_Event();
#endif
    UDP->UDP_ICR = AT91C_UDP_EXTRSM;
  }

  /* Start of Frame Interrupt */
  if (isr & AT91C_UDP_SOFINT) {
#if USB_SOF_EVENT
    USB_SOF_Event();
#endif
    UDP->UDP_ICR = AT91C_UDP_SOFINT;
  }

  /* Endpoint Interrupts */
  for (n = 0; n < USB_EP_NUM; n++) {
    if (isr & (1 << n)) {

      csr = UDP->UDP_CSR[n];

      /* Setup Packet Received Interrupt */
      if (csr & AT91C_UDP_RXSETUP) {
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
      if (csr & AT91C_UDP_TXCOMP) {
        if (TxDataBank[n]) {
          USB_SetCSR(n, AT91C_UDP_TXPKTRDY);
          TxDataBank[n] = 0;
        }
        if (USB_P_EP[n]) {
          USB_P_EP[n](USB_EVT_IN);
        }
        USB_ClrCSR(n, AT91C_UDP_TXCOMP);
      }

      /* STALL Packet Sent Interrupt */
      if (csr & AT91C_UDP_STALLSENT) {
        if ((csr & AT91C_UDP_EPTYPE) == AT91C_UDP_EPTYPE_CTRL) {
          if (USB_P_EP[n]) {
            USB_P_EP[n](USB_EVT_IN_STALL);
/*          USB_P_EP[n](USB_EVT_OUT_STALL); */
          }
        }
        USB_ClrCSR(n, AT91C_UDP_STALLSENT);
      }

    }
  }

  *AT91C_AIC_EOICR = 0;                           /* End of Interrupt */
}
