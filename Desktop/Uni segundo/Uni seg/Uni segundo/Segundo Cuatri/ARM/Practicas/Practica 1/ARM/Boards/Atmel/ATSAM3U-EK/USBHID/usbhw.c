/*----------------------------------------------------------------------------
 * U S B  -  K e r n e l
 *----------------------------------------------------------------------------
 * Name:    usbhw.c
 * Purpose: USB Hardware Layer Module for Atmel AT91SAM7X
 * Version: V1.21
 *----------------------------------------------------------------------------
 * This file is part of the uVision/ARM development tools.
 * This software may only be used under the terms of a valid, current,
 * end user licence from KEIL for a compatible version of KEIL software
 * development tools. Nothing else gives you the right to use this software.
 *
 * This software is supplied "AS IS" without warranties of any kind.
 *
 * Copyright (c) 2008 Keil - An ARM Company. All rights reserved.
 *----------------------------------------------------------------------------
 * History:
 *          V1.21 Added HighSpeed Handling
 *          V1.20 Added USB_ClearEPBuf
 *          V1.11 changed optimizaton level
 *          V1.10 Double buffer handling corrected
 *          V1.00 Initial Version
 *----------------------------------------------------------------------------*/

#include <SAM3U.H>                                    /* ATSAM3U definitions  */
#include "type.h"

#include "usb.h"
#include "usbcfg.h"
#include "usbhw.h"
#include "usbcore.h"
#include "usbuser.h"


//#pragma diag_suppress 1293

#define UDPHS_EPTFIFO_BASE (0x20180000)               /* (UDPHS_EPTFIFO) Base Address */

volatile BOOL USB_HighSpeed = __FALSE;                /* used USB speed       */


/*
 *  Calulate EP size code Function
 *   Called during EndPoint configuration
 *    Return Value:    EP size code for given EP size
 */

static int USB_CalcSizeEP (U32 size) {
  U32 EPSize;

  if      (size <=    8) {
    EPSize = 0;
  } 
  else if (size <=   16) {
    EPSize = 1;
  }
  else if (size <=   32) {
    EPSize = 2;
  }
  else if (size <=   64) {
    EPSize = 3;
  }
  else if (size <=  128) {
    EPSize = 4;
  }
  else if (size <=  256) {
    EPSize = 5;
  }
  else if (size <=  512)  {
    EPSize = 6;
  }
  else if (size <= 1024) {
    EPSize = 7;
  }

  return (EPSize);
}


/*
 *  Retrieve bank number Function
 *   Called during EndPoint configuration
 *    Return Value:    Bank number for given EP
 */

static int USB_GetBankEP (U32 EPNum) {
  U32 EPBank;

  EPNum &= 0x0F;                       /* set EP number */
  switch (EPNum) {
    case 0:
      EPBank = 1;                      /* one bank (bank0) */
      break;
    case 1:
    case 2:
      EPBank = 2;                      /* Double bank (Ping-Pong: bank0 / bank1) */
      break;
    case 3:
    case 4:
    case 5:
    case 6:
      EPBank = 3;                      /* Triple bank (bank0 / bank1 / bank2) */
      break;
    default:
      EPBank = 0;                      /* Zero bank, the enpoint is not mapped in memory */
      break;
  }

  return (EPBank);
}


/*
 *  Retrieve maximum EP size Function
 *   Called during EndPoint configuration
 *    Return Value:    maximum size for given EP
 */

static int USB_GetSizeEP (U32 EPNum) {
  U32 EPSize;

  EPNum &= 0x0F;                       /* set EP number */
  switch (EPNum) {
    case 0:
      EPSize =    64;                  /* maximum size   64 Byte */
      break;
    case 1:
    case 2:
      EPSize =   512;                  /* maximum size  512 Byte */
      break;
    case 3:
    case 4:
      EPSize =    64;                  /* maximum size   64 Byte */
      break;
    case 5:
    case 6:
      EPSize =  1024;                  /* maximum size 1024 Byte */
      break;
    default:
      EPSize =     0;                  /* maximum size    0 Byte */
      break;
  }

  return (EPSize);
}


/*
 *  USB Initialize Function
 *   Called by the User to initialize USB
 *    Return Value:    None
 */

void USB_Init (void) {
  U32 n;

  /* Enables the 48MHz USB Clock UDPCK and System Peripheral USB Clock */
  PMC->PMC_PCER   = (1 << ID_UDPHS);                       /* enable clock for UPDHS */
  PMC->CKGR_UCKR |= (CKGR_UCKR_UPLLCOUNT | CKGR_UCKR_UPLLEN);
  while (!(PMC->PMC_SR & PMC_SR_LOCKU));                   /* wait until PLL is locked */

  /* Configure the pull-up on D+ and disconnect it */
  UDPHS->UDPHS_CTRL |= UDPHS_CTRL_DETACH;                  /* detach */
  UDPHS->UDPHS_CTRL |= UDPHS_CTRL_PULLD_DIS;              /* Disable Pull Down */

  /* Reset IP UDPHS */
  UDPHS->UDPHS_CTRL &= ~UDPHS_CTRL_EN_UDPHS;
  UDPHS->UDPHS_CTRL |=  UDPHS_CTRL_EN_UDPHS;

  /* disable DMA for UDPHS */
  for (n = 1; n <= ((UDPHS->UDPHS_IPFEATURES & UDPHS_IPFEATURES_DMA_CHANNEL_NBR)>>4); n++) {
    /* RESET endpoint canal DMA: */
    UDPHS->UDPHS_DMA[n].UDPHS_DMACONTROL = 0;                     /* STOP command */
    /* Disable endpoint */
    UDPHS->UDPHS_EPT[n].UDPHS_EPTCTLDIS  = 0xFFFFFFFF;
    /* Clear status endpoint */
    UDPHS->UDPHS_EPT[n].UDPHS_EPTCLRSTA  = 0xFFFFFFFF;
    /* Reset endpoint config */
    UDPHS->UDPHS_EPT[n].UDPHS_EPTCTLENB  = 0;
    /* Reset DMA channel (Buff count and Control field) */
    UDPHS->UDPHS_DMA[n].UDPHS_DMACONTROL = (0x1 << 1);            /* NON STOP command */
    /* Reset DMA channel 0 (STOP) */
    UDPHS->UDPHS_DMA[n].UDPHS_DMACONTROL = 0;                     /* STOP command */
    /* Clear DMA channel status (read the register for clear it) */
    UDPHS->UDPHS_DMA[n].UDPHS_DMASTATUS =  UDPHS->UDPHS_DMA[n].UDPHS_DMASTATUS;
  }

    UDPHS->UDPHS_IEN     = 0;
    UDPHS->UDPHS_CLRINT  = UDPHS_CLRINT_UPSTR_RES |
                           UDPHS_CLRINT_ENDOFRSM  |
                           UDPHS_CLRINT_WAKE_UP   |
                           UDPHS_CLRINT_ENDRESET  |
                           UDPHS_CLRINT_INT_SOF   |
                           UDPHS_CLRINT_MICRO_SOF |
                           UDPHS_CLRINT_DET_SUSPD;

  NVIC_EnableIRQ(UDPHS_IRQn);               /* enable USB interrupt */

}


/*
 *  USB Connect Function
 *   Called by the User to Connect/Disconnect USB
 *    Parameters:      con:   Connect/Disconnect
 *    Return Value:    None
 */

void USB_Connect (BOOL con) {

  if (con) {
    UDPHS->UDPHS_CTRL &= ~UDPHS_CTRL_DETACH;             /* Pull Up on DP     */
    UDPHS->UDPHS_CTRL |=  UDPHS_CTRL_PULLD_DIS;          /* Disable Pull Down */
  } else {
    UDPHS->UDPHS_CTRL |=  UDPHS_CTRL_DETACH;             /* detach            */
    UDPHS->UDPHS_CTRL &= ~UDPHS_CTRL_PULLD_DIS;          /* Enable Pull Down  */
  }
}


/*
 *  USB Reset Function
 *   Called automatically on USB Reset
 *    Return Value:    None
 */


void USB_Reset (void) {
  U32 n, EPMask, EPSize;

  EPMask = ((1 << USB_EP_NUM) - 1);

  /* Reset & Disable USB Endpoints */
  for (n = 0; n < USB_EP_NUM; n++) {
    UDPHS->UDPHS_EPT[n].UDPHS_EPTCFG    =  0;
    UDPHS->UDPHS_EPT[n].UDPHS_EPTCTLDIS = (0x1 <<  0);           /* disable EP  */
  }
  UDPHS->UDPHS_EPTRST = EPMask;                                  /* reset   EPs */
  UDPHS->UDPHS_EPTRST = 0;

  /* Setup USB Interrupts */               /* note: Micro_SOF not yet handled */
  UDPHS->UDPHS_IEN = UDPHS_IEN_DET_SUSPD |
                     UDPHS_IEN_ENDOFRSM  |
                     UDPHS_IEN_WAKE_UP   |
                     UDPHS_IEN_INT_SOF   |
                     (EPMask << 8);

  /* Setup Control Endpoint 0 */
  EPSize = USB_CalcSizeEP(USB_MAX_PACKET0);
  UDPHS->UDPHS_EPT[0].UDPHS_EPTCFG     = UDPHS_EPTCFG0_BK_NUMBER_1      |
                                         UDPHS_EPTCFG0_EPT_TYPE_CTL_EPT |
                                         EPSize;
  UDPHS->UDPHS_EPT[0].UDPHS_EPTCTLENB  = UDPHS_EPTCTLENB0_RX_BK_RDY            |
                                         UDPHS_EPTCTLENB0_TX_COMPLT            |
                                         UDPHS_EPTCTLENB0_RX_SETUP__ERR_FL_ISO |
                                         UDPHS_EPTCTLENB0_NYET_DIS             |
                                         UDPHS_EPTCTLENB0_EPT_ENABL;
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
    UDPHS->UDPHS_CTRL |=  UDPHS_CTRL_REWAKEUP;
  } else {
    UDPHS->UDPHS_CTRL &= ~UDPHS_CTRL_REWAKEUP;
  }
}


/*
 *  USB Set Address Function
 *    Parameters:      adr:   USB Address
 *    Return Value:    None
 */

void USB_SetAddress (U32 adr) {

  if (adr) {
    UDPHS->UDPHS_CTRL |=  (UDPHS_CTRL_FADDR_EN | adr);
  } else {
    UDPHS->UDPHS_CTRL &= ~(UDPHS_CTRL_FADDR_EN | UDPHS_CTRL_DEV_ADDR);
  }
}


/*
 *  USB Configure Function
 *    Parameters:      cfg:   Configure/Deconfigure
 *    Return Value:    None
 */

void USB_Configure (BOOL cfg) {
  /* Performed by Hardware */
}


/*
 *  Configure USB Endpoint according to Descriptor
 *    Parameters:      pEPD:  Pointer to Endpoint Descriptor
 *    Return Value:    None
 */

void USB_ConfigEP (USB_ENDPOINT_DESCRIPTOR *pEPD) {
  U32 num, type, dir, size, bank, interval;

  num      = pEPD->bEndpointAddress & 0x0F;
  type     = pEPD->bmAttributes & USB_ENDPOINT_TYPE_MASK;
  dir      = pEPD->bEndpointAddress >> 7;
  interval = pEPD->bInterval;
  size     = USB_CalcSizeEP(pEPD->wMaxPacketSize);
  bank     = USB_GetBankEP(num);

  /* check if MaxPacketSize fits for EndPoint */
  if (pEPD->wMaxPacketSize <= USB_GetSizeEP(num)) {
    UDPHS->UDPHS_EPT[num].UDPHS_EPTCFG    = (interval << 8) |
                                            (bank     << 6) |
                                            (type     << 4) |
                                            (dir      << 3) |
                                            (size     << 0);

    UDPHS->UDPHS_EPT[num].UDPHS_EPTCTLENB = (0x1 <<  9) |  /* Received OUT Data Interrupt Enable */
                                            (0x1 << 10) |  /* Transmitted IN Data Complete Interrupt Enable */
                                            (0x1 <<  4) |  /* NYET Disable (Only for High Speed Bulk OUT endpoints) */
                                            (0x1 << 13) ;  /* Stall Sent /ISO CRC Error/Number of Transaction Error Interrupt Enable */
  }

}


/*
 *  Set Direction for USB Control Endpoint
 *    Parameters:      dir:   Out (dir == 0), In (dir <> 0)
 *    Return Value:    None
 */

void USB_DirCtrlEP (U32 dir) {
  /* Performed by Hardware */
}


/*
 *  Enable USB Endpoint
 *    Parameters:      EPNum: Endpoint Number
 *                       EPNum.0..3: Address
 *                       EPNum.7:    Dir
 *    Return Value:    None
 */

void USB_EnableEP (U32 EPNum) {

  EPNum &= 0x0F;
  UDPHS->UDPHS_EPT[EPNum].UDPHS_EPTCTLENB = (0x1 << 0); /* Endpoint Enable */
}


/*
 *  Disable USB Endpoint
 *    Parameters:      EPNum: Endpoint Number
 *                       EPNum.0..3: Address
 *                       EPNum.7:    Dir
 *    Return Value:    None
 */

void USB_DisableEP (U32 EPNum) {

  EPNum &= 0x0F;
  UDPHS->UDPHS_EPT[EPNum].UDPHS_EPTCTLDIS = (0x1 << 0); /* Endpoint Disable */
}


/*
 *  Reset USB Endpoint
 *    Parameters:      EPNum: Endpoint Number
 *                       EPNum.0..3: Address
 *                       EPNum.7:    Dir
 *    Return Value:    None
 */

void USB_ResetEP (U32 EPNum) {

  EPNum &= 0x0F;
  UDPHS->UDPHS_EPT[EPNum].UDPHS_EPTCLRSTA = (0x1 << 6) | /* Data Toggle Clear */
                                            (0x1 << 5);  /* Stall Handshake Request Clear */

  UDPHS->UDPHS_EPTRST |=   1 << EPNum;                   /* reset endpoint */
  UDPHS->UDPHS_EPTRST &= ~(1 << EPNum);

}


/*
 *  Set Stall for USB Endpoint
 *    Parameters:      EPNum: Endpoint Number
 *                       EPNum.0..3: Address
 *                       EPNum.7:    Dir
 *    Return Value:    None
 */

void USB_SetStallEP (U32 EPNum) {

  EPNum &= 0x0F;
  UDPHS->UDPHS_EPT[EPNum].UDPHS_EPTSETSTA = (0x1 << 5); /* Stall Handshake Request Set */
}


/*
 *  Clear Stall for USB Endpoint
 *    Parameters:      EPNum: Endpoint Number
 *                       EPNum.0..3: Address
 *                       EPNum.7:    Dir
 *    Return Value:    None
 */

void USB_ClrStallEP (U32 EPNum) {

  EPNum &= 0x0F;
  UDPHS->UDPHS_EPT[EPNum].UDPHS_EPTCLRSTA = (0x1 << 6) | /* Data Toggle Clear */
                                            (0x1 << 5);  /* Stall Handshake Request Clear */
}


/*
 *  Clear USB Endpoint Buffer
 *    Parameters:      EPNum: Endpoint Number
 *                       EPNum.0..3: Address
 *                       EPNum.7:    Dir
 *    Return Value:    None
 */

void USB_ClearEPBuf (U32 EPNum) {

  EPNum &= 0x0F;
  UDPHS->UDPHS_EPT[EPNum].UDPHS_EPTSETSTA = (0x1 << 9); /* KILL Bank Set (for IN Endpoint) */
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
  U8 *pEPFifo;                                                      /* pointer to the endpoint FIFO */

  EPNum &= 0x0F;                                                    /* set EP number */
  pEPFifo = (U8*)((U32*)UDPHS_EPTFIFO_BASE + (0x4000*EPNum)); /* set pointer to EP FIFO */ 
  cnt     = (UDPHS->UDPHS_EPT[EPNum].UDPHS_EPTSTA >> 20) & 0x07FF;  /* get numbor of received bytes */
  for (n = 0; n < cnt; n++) {
    *pData++ = *pEPFifo++;
  }
  UDPHS->UDPHS_EPT[EPNum].UDPHS_EPTCLRSTA = (0x1 << 9);             /* Received OUT Data Clear */

  /* RX_Setup must be cleared after Setup packet is read */
  UDPHS->UDPHS_EPT[EPNum].UDPHS_EPTCLRSTA = (0x1 << 12);            /* Received SETUP/Error Flow Clear */

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
  U8 *pEPFifo;                                                        /* pointer to the endpoint FIFO */

  EPNum &= 0x0F;                                                      /* set EP number */

  if (UDPHS->UDPHS_EPT[EPNum].UDPHS_EPTSTA & (0x1 << 5)) {            /* do not send if EP is stalled */
    return (cnt);
  }

  if (UDPHS->UDPHS_EPT[EPNum].UDPHS_EPTSTA & (0x1 << 11)) {           /* bank not ready */
    return (0);
  }

  pEPFifo = (U8*)((U32*)UDPHS_EPTFIFO_BASE + (16384*EPNum));    /* set pointer to EP FIFO */ 
  for (n = 0; n < cnt; n++) {
    *pEPFifo++ = *pData++;                                            /* write data to fifo */
  }

  UDPHS->UDPHS_EPT[EPNum].UDPHS_EPTSETSTA = (0x1 << 11);              /* set packet ready */

  return (cnt);
}


/*
 *  Get USB Last Frame Number
 *    Parameters:      None
 *    Return Value:    Frame Number
 */

U32 USB_GetFrame (void) {
  U32 val;

  if ((UDPHS->UDPHS_FNUM & (1UL << 31)) == 0) {
    val = (UDPHS->UDPHS_FNUM & UDPHS_FNUM_FRAME_NUMBER) >> 3;
  } else {
    val = 0xFFFFFFFF;
  }

  return (val);
}



/*
 *  USB Interrupt Service Routine
 */
void UDPHS_IRQHandler (void) {
  U32 intsta, eptsta, n;

  intsta = UDPHS->UDPHS_INTSTA;


  /* End of Bus Reset Interrupt */
  if (intsta & UDPHS_INTSTA_ENDRESET) {
    /* get used speed (HighSpeed or FullSpeed) */
    USB_HighSpeed = (intsta & UDPHS_INTSTA_SPEED) ? __TRUE : __FALSE;

    USB_Reset();
#if USB_RESET_EVENT
    USB_Reset_Event();
#endif
    UDPHS->UDPHS_CLRINT = UDPHS_CLRINT_ENDRESET;
  }

  /* USB Suspend Interrupt */
  if (intsta == UDPHS_INTSTA_DET_SUSPD) {
    UDPHS->UDPHS_CLRINT = UDPHS_CLRINT_DET_SUSPD;
    USB_Suspend();
#if USB_SUSPEND_EVENT
    USB_Suspend_Event();
#endif
  }

  /* USB Resume Interrupt */
  if (intsta & UDPHS_INTSTA_ENDOFRSM) {
    USB_Resume();
#if USB_RESUME_EVENT
    USB_Resume_Event();
#endif
    UDPHS->UDPHS_CLRINT = UDPHS_CLRINT_ENDOFRSM;
  }

  /* USB Wakeup Interrupt */
  if (intsta & UDPHS_INTSTA_WAKE_UP) {
    USB_WakeUp();
#if USB_WAKEUP_EVENT
    USB_WakeUp_Event();
#endif
    UDPHS->UDPHS_CLRINT = UDPHS_CLRINT_WAKE_UP;
  }

  /* Start of Frame Interrupt */
  if (intsta & UDPHS_INTSTA_INT_SOF) {
#if USB_SOF_EVENT
    USB_SOF_Event();
#endif
    UDPHS->UDPHS_CLRINT = UDPHS_CLRINT_INT_SOF;
  }

  /* Endpoint Interrupts */
  for (n = 0; n < USB_EP_NUM; n++) {
    if (intsta & (1 << (n + 8))) {

      eptsta = UDPHS->UDPHS_EPT[n].UDPHS_EPTSTA;             /* read EP status */

      /* Data Packet Sent Interrupt */
      if (eptsta & (0x1 << 10)) {                            /* Transmitted IN Data Complete Interrupt */
        UDPHS->UDPHS_EPT[n].UDPHS_EPTCLRSTA = (0x1 << 10);   /* Transmitted IN Data Complete Clear */
        if (USB_P_EP[n]) {
          USB_P_EP[n](USB_EVT_IN);
        }
      }

      /* Data Packet Received Interrupt */
      if (eptsta & (0x1 << 9)) {                             /* Received OUT Data Interrupt */
        if (USB_P_EP[n]) {
          USB_P_EP[n](USB_EVT_OUT);
        }
//        pUDP->UDPHS_EPT[n].UDPHS_EPTCLRSTA = AT91C_UDPHS_RX_BK_RDY; // is already cleared in USB_ReadEP()
      }

      /* Setup Packet Received Interrupt */
      if (eptsta & (0x1 << 12)) {                            /* Received SETUP Interrupt */
        if (USB_P_EP[n]) {
          USB_P_EP[n](USB_EVT_SETUP);
        }
//        pUDP->UDPHS_EPT[n].UDPHS_EPTCLRSTA = AT91C_UDPHS_RX_SETUP; // is already cleared in USB_ReadEP()
      }

      /* STALL Packet Sent Interrupt */
      if (eptsta & (0x1 << 13)) {                           /* Stall Sent/CRC ISO Error/Number of Transaction Error */
        if ((UDPHS->UDPHS_EPT[n].UDPHS_EPTCFG & UDPHS_EPTCFG0_EPT_TYPE) == UDPHS_EPTCFG0_EPT_TYPE_CTL_EPT) {
          if (USB_P_EP[n]) {
            USB_P_EP[n](USB_EVT_IN_STALL);
/*          USB_P_EP[n](USB_EVT_OUT_STALL); */
          }
        }
        UDPHS->UDPHS_EPT[n].UDPHS_EPTCLRSTA = UDPHS_EPTCLRSTA0_STALL_SNT__ERR_NBTRA;
      }

    }
  }


}
