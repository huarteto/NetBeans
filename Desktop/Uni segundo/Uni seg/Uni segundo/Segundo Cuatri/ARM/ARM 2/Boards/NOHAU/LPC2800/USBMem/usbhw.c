/*----------------------------------------------------------------------------
 *      U S B  -  K e r n e l
 *----------------------------------------------------------------------------
 *      Name:    USBHW.C
 *      Purpose: USB Hardware Layer Module for Philips LPC288x
 *      Version: V1.11
 *----------------------------------------------------------------------------
 *      This software is supplied "AS IS" without any warranties, express,
 *      implied or statutory, including but not limited to the implied
 *      warranties of fitness for purpose, satisfactory quality and
 *      noninfringement. Keil extends you a royalty-free right to reproduce
 *      and distribute executable files created using this software for use
 *      on Philips LPC microcontroller devices only. Nothing else gives you
 *      the right to use this software.
 *
 *      Copyright (c) 2005-2007 Keil Software.
 *---------------------------------------------------------------------------*/

#include <LPC288x.h>                    /* LPC288x definitions */

#include "type.h"

#include "usb.h"
#include "usbcfg.h"
#include "usbhw.h"
#include "usbcore.h"
#include "usbuser.h"


/* LPC288x USB Register Bit definitions */

#define EIX_DIR         0x01
#define EIX_SEL_EP0SET  0x20

#define ETYPE_CTRL      0x00
#define ETYPE_ISO       0x01
#define ETYPE_BULK      0x02
#define ETYPE_INT       0x03
#define ETYPE_DBLBUF    0x04
#define ETYPE_EP_ENAB   0x08
#define ETYPE_DIS_EOT   0x10

#define ECTRL_STALL     0x01
#define ECTRL_STATUS    0x02
#define ECTRL_DATA      0x04
#define ECTRL_CLRBUF    0x10
#define ECTRL_BUFFULL   0x20


/* Base address of Interrupt Vector Table
   Must be on 2048 boundry, and must not overlap with other variables or stack
*/
#define IRQ_TABLE_BASE  0x40D000

/* Interrupt Vector table looks like this:
0x40D000 --|-----------------------|-----------------------|
           | IRQ0  Handler Address |    IRQ0 Priority      |
           |-----------------------|-----------------------|
           :                       :                       :
           :                       :                       :
           |-----------------------|-----------------------|
           | IRQ31 Handler Address |    IRQ31 Priority     |
           |-----------------------|-----------------------|
*/


/* Endpoint Masks */
#define EP_MSK_CTRL     0x0001          /* Control Endpoint Mask */
#define EP_MSK_BULK     0xC924          /* Bulk Endpoint Mask */
#define EP_MSK_INT      0x4492          /* Interrupt Endpoint Mask */
#define EP_MSK_ISO      0x1248          /* Isochronous Endpoint Mask */


/* USB Hardware Variables */
volatile BOOL USB_HighSpeed = FALSE;


/*
 *  USB Initialize Function
 *   Called by the User to initialize USB
 *    Return Value:    None
 */

void USB_Init (void) {

  USBDevAdr = 0x00;
  USBIntCfg = 0xFC;

  USBMode &= ~(1 << 0);                 // SOFTCT = 0

  // Put IRQ handler address to IRQ table
  *(DWORD *)(IRQ_TABLE_BASE |  (26 << 3)     ) = (DWORD) USB_ISR;
  // Put IRQ priority to IRQ table (not used in this system)
  *(DWORD *)(IRQ_TABLE_BASE | ((26 << 3) + 4)) = 1;

  // Set interrupt request settings for USB interrupt
  INT_REQ26 = (1<<28) | (1<<27) | (1<<26) | (1<<16) | 0x1;

  // Set VECTOR0 table address to IRQ table start
  INT_VECTOR0=IRQ_TABLE_BASE & 0xFFFFF800;

  USB_Reset();
  USB_SetAddress(0);

  USBMode |= (1 << 0);                  // SOFTCT = 1

  USBMode |= ((0x1<<3)|(0x1<<7));       // Set GIE, CLKAON
}


/*
 *  USB Connect Function
 *   Called by the User to Connect/Disconnect USB
 *    Parameters:      con:   Connect/Disconnect
 *    Return Value:    None
 */

void USB_Connect (BOOL con) {

  if (con) {
    USBMode |=  0x00000001;
  } else {
    USBMode &= ~0x00000001;
  }
}


/*
 *  USB Reset Function
 *   Called automatically on USB Reset
 *    Return Value:    None
 */

void USB_Reset (void) {

  USB_HighSpeed = FALSE;
  USBLock    = 0xAA37;

  USBClkEn   = 1;                       // Documentation error in USB chapter
                                        // On master reset, this bit needs to set again.
  USBEIX = EIX_SEL_EP0SET;              // EP0 set up register
  USBIntClr  = 0x000000FF;

  USBEIntClr = 0xFFFF;
  USBEIntE  |= 0xFFFF;

#if USB_DMA
  UDMACtrl    = 0x01;                   // Enable DMA operation
  UDMASoftRes = 0x03;                   // Soft reset to both DMA channel

  UDMAIntClr  = 0x00000066;             // clear EOT and Error for CH0 and CH1
  while (UDMAStat & 0x0707);            // Wait if both channels are not idle.

  UDMAIntEn = 0x44;                     /* Due to the nature of the RAM drive,
                                           polling on the EOT make the code more
                                           readable and easy to maintain. */
//UDMAIntEn = 0x66;                     // Enable both EOT and Error
#endif

  USBIntE  = 0x000000B9;                /* no SOF, PSOF, and DMA */

#if USB_DMA
  USBIntE |= 0x40;                      /* Enable DMA bit */
#endif
}


/*
 *  USB Suspend Function
 *   Called automatically on USB Suspend
 *    Return Value:    None
 */

void USB_Suspend (void) {
  USBMode |= (1<<5);
}


/*
 *  USB Resume Function
 *   Called automatically on USB Resume
 *    Return Value:    None
 */

void USB_Resume (void) {
  USBMode |=  (1<<6);
  USBMode &= ~(1<<6);
}


/*
 *  USB Remote Wakeup Function
 *   Called automatically on USB Remote Wakeup
 *    Return Value:    None
 */

void USB_WakeUp (void) {

  if (USB_DeviceStatus & USB_GETSTATUS_REMOTE_WAKEUP) {
    USBMode |= (1<<2);
  }
}


/*
 *  USB Remote Wakeup Configuration Function
 *    Parameters:      cfg:   Enable/Disable
 *    Return Value:    None
 */

void USB_WakeUpCfg (BOOL cfg) {
  /* Not needed */
}


/*
 *  USB Set Address Function
 *    Parameters:      adr:   USB Address
 *    Return Value:    None
 */

void USB_SetAddress (DWORD adr) {
  USBDevAdr = (1<<7) | adr;
}


/*
 *  USB Configure Function
 *    Parameters:      cfg:   Configure/Deconfigure
 *    Return Value:    None
 */

void USB_Configure (BOOL cfg) {
  /* Not needed */
}


/*
 *  Configure USB Endpoint according to Descriptor
 *    Parameters:      pEPD:  Pointer to Endpoint Descriptor
 *    Return Value:    None
 */

void USB_ConfigEP (USB_ENDPOINT_DESCRIPTOR *pEPD) {
  DWORD ep_addr;

  ep_addr = pEPD->bEndpointAddress;
  if (ep_addr & 0x80) {
    ep_addr = ((ep_addr & 0x7F) << 1) | EIX_DIR;
  } else {
    ep_addr =   ep_addr << 1;
  }
  USBEIX = ep_addr;

  USBEType = (pEPD->bmAttributes & 0x001F) | 0x0010;

  USBEMaxSize = pEPD->wMaxPacketSize & 0x03FF;
}


/*
 *  Set Direction for USB Control Endpoint
 *    Parameters:      dir:   Out (dir == 0), In (dir <> 0)
 *    Return Value:    None
 */

void USB_DirCtrlEP (DWORD dir) {
  /* Not needed */
}


/*
 *  Enable USB Endpoint
 *    Parameters:      EPNum: Endpoint Number
 *                       EPNum.0..3: Address
 *                       EPNum.7:    Dir
 *    Return Value:    None
 */

void USB_EnableEP (DWORD EPNum) {
  DWORD ep_addr;

  if (EPNum & 0x80) {
    ep_addr = ((EPNum&0x7F) << 1) | EIX_DIR;    // IN
  } else {
    ep_addr =  (EPNum&0x7F) << 1;               // OUT
  }
  USBEIX = ep_addr;
  USBEType |= ETYPE_EP_ENAB;
}


/*
 *  Disable USB Endpoint
 *    Parameters:      EPNum: Endpoint Number
 *                       EPNum.0..3: Address
 *                       EPNum.7:    Dir
 *    Return Value:    None
 */

void USB_DisableEP (DWORD EPNum) {
  DWORD ep_addr;

  if (EPNum & 0x80) {
    ep_addr = ((EPNum&0x7F) << 1) | EIX_DIR;    // IN
  } else {
    ep_addr =  (EPNum&0x7F) << 1;               // OUT
  }
  USBEIX = ep_addr;
  USBEType &= ~ETYPE_EP_ENAB;
}


/*
 *  Reset USB Endpoint
 *    Parameters:      EPNum: Endpoint Number
 *                       EPNum.0..3: Address
 *                       EPNum.7:    Dir
 *    Return Value:    None
 */

void USB_ResetEP (DWORD EPNum) {
}


/*
 *  Set Stall for USB Endpoint
 *    Parameters:      EPNum: Endpoint Number
 *                       EPNum.0..3: Address
 *                       EPNum.7:    Dir
 *    Return Value:    None
 */

void USB_SetStallEP (DWORD EPNum) {
  DWORD ep_addr;

  if (EPNum & 0x80) {
    ep_addr = ((EPNum&0x7F) << 1) | EIX_DIR;    // IN
  } else {
    ep_addr =  (EPNum&0x7F) << 1;               // OUT
  }
  USBEIX = ep_addr;
  USBECtrl |= (1 << 0);
}


/*
 *  Clear Stall for USB Endpoint
 *    Parameters:      EPNum: Endpoint Number
 *                       EPNum.0..3: Address
 *                       EPNum.7:    Dir
 *    Return Value:    None
 */

void USB_ClrStallEP (DWORD EPNum) {
  DWORD ep_addr;

  if (EPNum & 0x80) {
    ep_addr = ((EPNum&0x7F) << 1) | EIX_DIR;    // IN
  } else {
    ep_addr =  (EPNum&0x7F) << 1;               // OUT
  }
  USBEIX = ep_addr;
  USBECtrl &= ~(1 << 0);
}


/*
 *  Read USB Endpoint Data
 *    Parameters:      EPNum: Endpoint Number
 *                       EPNum.0..3: Address
 *                       EPNum.7:    Dir
 *                     pData: Pointer to Data Buffer
 *    Return Value:    Number of bytes read
 */

DWORD USB_ReadEP (DWORD EPNum, BYTE *pData) {
  DWORD cnt, n;

  USBEIX    = ((EPNum & 0x0F) << 1) | ((EPNum >> 7) & 0x01);
  USBECtrl |=  ECTRL_DATA;              // set DATA bit
  cnt = USBDCnt;
  for (n = 0; n < (cnt + 3) / 4; n++) {
    *((__packed DWORD *)pData) = USBData;
    pData += 4;
  }
  USBECtrl &= ~ECTRL_DATA;              // clear DATA bit
  USBECtrl |=  ECTRL_CLRBUF;            // clear buffer
  return (cnt);
}


/*
 *  Read USB Setup Data on Endpoint 0
 *    Parameters:      pData: Pointer to Data Buffer
 *    Return Value:    Number of bytes read
 */

DWORD USB_ReadSetupEP0 (BYTE *pData) {
  DWORD cnt, n;

  USBEIX = EIX_SEL_EP0SET;              // Setup buffer allocated
  USBECtrl |=  ECTRL_DATA;              // set DATA bit
  cnt = USBDCnt;
  for (n = 0; n < (cnt + 3) / 4; n++) {
    *((__packed DWORD *)pData) = USBData;
    pData += 4;
  }
  USBECtrl &= ~ECTRL_DATA;              // clear DATA bit
  USBECtrl |=  ECTRL_CLRBUF;            // clear buffer
  return (cnt);
}


/*
 *  Read USB Status Input (Zero Data) on Endpoint 0
 *    Parameters:      None
 *    Return Value:    Number of bytes read
 */

DWORD USB_StatusInEP0 (void) {

  USBEIX = EIX_DIR;                     // direction: IN
  USBECtrl |= ECTRL_STATUS;
  while (USBECtrl & ECTRL_STATUS);
  return (0);
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

DWORD USB_WriteEP (DWORD EPNum, BYTE *pData, DWORD cnt) {
  DWORD n;

  USBEIX    = ((EPNum & 0x0F) << 1) | ((EPNum >> 7) & 0x01);
  USBECtrl |=  ECTRL_DATA;              // set DATA bit
  USBDCnt   = cnt;
  for (n = 0; n < (cnt + 3) / 4; n++) {
    while (USBECtrl & ECTRL_BUFFULL);
    USBData = *((__packed DWORD *)pData);
    pData += 4;
  }
  USBECtrl &= ~ECTRL_DATA;              // clear DATA bit
  return (cnt);
}


/*
 *  Write USB Status Output (Zero Data) on Endpoint 0
 *    Parameters:      None
 *    Return Value:    Number of bytes written
 */

DWORD USB_StatusOutEP0 (void) {

  USBEIX = 0x00;                        // direction: OUT
  USBECtrl |= ECTRL_STATUS;
  while (USBECtrl & ECTRL_STATUS);
  return (0);
}


#if USB_DMA

/*
 *  Setup USB DMA Transfer for selected Endpoint
 *    Parameters:      Endpoint Num
 *                     Src or Dest address, for EP IN, it's the src
 *                     for EP OUT, it's the Dest.
 *    Return Value:    TRUE - Success, FALSE - Error
 */

BOOL USB_DMA_Setup(DWORD EPNum, DWORD SrcDestAddr, DWORD Length) {
  DWORD ep_addr;
  DWORD regCtrl0, regCtrl1;

  ep_addr = EPNum & 0x7F;
  if (ep_addr == 0) {
    return(FALSE);
  }

  if (EPNum & 0x80) {                   // Direction IN, channel 1
    UDMA1Src = SrcDestAddr;             // Source is IRAM on LPC288x
    switch (ep_addr) {
      case 1:
        UDMA1Dest = 0x00000004;         // EP1 IN transfer
        break;
      case 2:
        UDMA1Dest = 0x00000008;         // EP2 IN transfer
        break;
    }
    UDMA1Throtl = 0x00000001;
    UDMA1Cnt = Length;

    UDMAFCP3 = 0x00000001;
    /* for Channel 1, ERR(31) and EOT(30) Enabled, DFC_PORT(23) is IN EP2(0x3),
       DFC_MODE(21) is 0x1(IN), DA_ADJ(19) is 0x0(IN), DTYPE(17) is 0x2(32 bit),
       DEST(15) is 0x1(IN), SFC_PORT(11) is N/A, SFC_MODE(9) is 0x0(IN),
       SA_ADJ(7) is 0x1(IN src adr inc), STYPE(5) is 0x2(32 bit), SOURCE(3) is
       0x0(IN), CHEN is 0x3
    */
    regCtrl1 = 0xC0000000|(3<<23)|(1<<21)|(0<<19)|(2<<17)
                         |(1<<15)|(0<< 9)|(1<< 7)|(2<< 5)|(0<< 3);
    UDMA1Ctrl = regCtrl1;
  }
  else {                                // direction OUT, channel 0
    switch ( ep_addr ) {
      case 1:
        UDMA0Src = 0x00000004;          // EP1 OUT transfer
        break;
      case 2:
        UDMA0Src = 0x00000008;          // EP2 OUT transfer
        break;
    }
    UDMA0Dest = SrcDestAddr;            // Destination is IRAM on LPC288x
    UDMA0Throtl = 0x00000001;
    UDMA0Cnt = Length;                  // For FS = 0x40, for HS = 0x200

    UDMAFCP2 = 0x00000001;
    /* for Channel 0, ERR(31) and EOT(30) Enabled, DFC_PORT(23) is N/A,
       DFC_MODE(21) is 0x0(no dest FC, OUT), DA_ADJ(19) is 0x1(OUT dest adr inc),
       DTYPE(17) is 0x2(32 bit), DEST(15) is 0x0(OUT), SFC_PORT(11) is
       0x2(OUT EP2), SFC_MODE(9) is 0x1(OUT), SA_ADJ(7) is 0x0(OUT), STYPE(5) is
       0x2(32 bit), SOURCE(3) is 0x1(OUT).
    */
    regCtrl0 = 0xC0000000|(0<<21)|(1<<19)|(2<<17)
                         |(0<<15)|(2<<11)|(1<<9)|(0<<7)|(2<<5)|(1<<3);
    UDMA0Ctrl = regCtrl0;
  }
  return (TRUE);                        /* Success */
}


/*
 *  Enable USB DMA Endpoint
 *    Parameters:      Channel Num
 *    Return Value:    None
 */

void USB_DMA_Enable (DWORD CHNum) {

  if (CHNum == 0x0) {
//  UDMAIntEn |= 0x06;
    UDMA0Ctrl |= 0x01;                  // Send DMA request on channel 0
  }
  else if (CHNum == 0x01) {
//  UDMAIntEn |= 0x60;
    UDMA1Ctrl |= 0x01;                  // Send DMA request on channel 1
  }
}


/*
 *  Disable USB DMA Endpoint
 *    Parameters:      Channel Num
 *    Return Value:    None
 */

void USB_DMA_Disable (DWORD CHNum) {

  if (CHNum == 0x00) {
    UDMAIntDis |= 0x06;
  }
  else if (CHNum == 0x01) {
    UDMAIntDis |= 0x60;
  }
}


/*
 *  Get USB DMA Endpoint Status
 *    Parameters:      Channel Num
 *
 *    Return Value:    DMA Status
 */

DWORD USB_DMA_Status (DWORD CHNum) {
  DWORD regVal, CHStat;

  if (CHNum == 1) {                     // direction IN, channel 1
    regVal = UDMA1Stat;
  } else if (CHNum == 0) {
    regVal = UDMA0Stat;
  }
  switch (regVal & 0x03) {
    case 0:
    CHStat = USB_DMA_IDLE;
    break;
  case 1:
    CHStat = USB_DMA_BUSY;
    break;
  case 2:
    CHStat = USB_DMA_SUSPEND;
    break;
  case 3:
    CHStat = USB_DMA_ERROR;
    break;
  }
  return(CHStat);
}

#endif /* USB_DMA */


/*
 *  Get USB Last Frame Number
 *    Parameters:      None
 *    Return Value:    Frame Number
 */

DWORD USB_GetFrame (void) {
  DWORD val;

  val = USBFN;
  return (val);
}


/*
 *  USB Interrupt Service Routine
 */

void USB_ISR(void)  {
  DWORD disr, n, m;

  disr = USBIntStat;                    /* Device Interrupt Status */
  if (disr != 0) {
    /* Device Status Interrupt (Reset, Connect change, Suspend/Resume) */
    if (disr & (1<<0)) {                /* Reset */
      USB_Reset();
      USB_SetAddress(0);
#if   USB_RESET_EVENT
      USB_Reset_Event();
#endif
      goto isr_end;
    }

    if (disr & (1<<3)) {                /* Suspend */
      USB_Suspend();
#if   USB_SUSPEND_EVENT
      USB_Suspend_Event();
#endif
      goto isr_end;
    }

    if (disr & (1<<4)) {                /* Resume */
      USBLock = 0xAA37;
      USB_Resume();
#if   USB_RESUME_EVENT
      USB_Resume_Event();
#endif
      goto isr_end;
    }

#if USB_SOF_EVENT
    /* Start of Frame Interrupt */
    if (disr & (1<<1)) {
      USB_SOF_Event();
    }
#endif

    if (disr & (1<<5)) {                /* High speed or full speed */
      USBLock = 0xAA37;
      USB_HighSpeed = TRUE;
      goto isr_end;
    }

    /* Setup 0 packet coming */
    if (disr & (1<<7)) {
      USB_P_EP[0](USB_EVT_SETUP);
      goto isr_end;
    }
  }

  while (USBEIntStat) {                 /* Endpoint Interrupt Status */
    for (n = 0; n < USB_EP_NUM; n++) {  /* Check All Endpoints */
      if (USBEIntStat & (1 << n)) {
        m = n >> 1;
        USBEIntClr = 1 << n;
        if ((n & 1) == 0) {             /* OUT Endpoint */
          if (USB_P_EP[m]) {
            USB_P_EP[m](USB_EVT_OUT);
          }
        } else {                        /* IN Endpoint */
          if (USB_P_EP[m]) {
            USB_P_EP[m](USB_EVT_IN);
          }
        }
      }
    }
  }

#if USB_DMA
  if (UDMAIntStat & 0x02) {             /* End of Transfer Interrupt */
    UDMAIntClr = 0x02;
  }
  if (UDMAIntStat & 0x20) {
    UDMAIntClr = 0x20;
  }

  if (UDMAIntStat & 0x04) {             /* System Error Interrupt */
    USB_DMA_Status(0);
    UDMAIntClr = 0x04;
  }
  if (UDMAIntStat & 0x40) {
    USB_DMA_Status(1);
    UDMAIntClr = 0x40;
  }
#endif /* USB_DMA */

isr_end:
  USBIntClr = disr;
}
