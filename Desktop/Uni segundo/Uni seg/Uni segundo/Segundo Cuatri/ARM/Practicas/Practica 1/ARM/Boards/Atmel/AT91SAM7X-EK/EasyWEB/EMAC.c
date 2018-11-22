/******************************************************************
 *****                                                        *****
 *****  Name: cs8900.c                                        *****
 *****  Ver.: 1.0                                             *****
 *****  Date: 07/05/2001                                      *****
 *****  Auth: Andreas Dannenberg                              *****
 *****        HTWK Leipzig                                    *****
 *****        university of applied sciences                  *****
 *****        Germany                                         *****
 *****  Func: ethernet packet-driver for use with LAN-        *****
 *****        controller CS8900 from Crystal/Cirrus Logic     *****
 *****                                                        *****
 *****  Keil: Module modified for use with Atmel AT91SAM7X    *****
 *****        EMAC Ethernet controller                        *****
 *****                                                        *****
 ******************************************************************/

#include "EMAC.h"
#include "tcpip.h"

// Keil: local variables added
static unsigned short *rptr;
static unsigned short *tptr;
static unsigned int rcnt;
static unsigned int RxBufIndex;
static unsigned int RxStartIndex;
static Buf_Desc Rx_Desc[NUM_RX_BUF];
static Buf_Desc Tx_Desc[NUM_TX_BUF];


// Keil: function added to write PHY
void write_PHY (int PhyReg, int Value)
{
  pEMAC->EMAC_MAN = ((AT91C_EMAC_SOF & (0x01<<30)) | (2 << 16) | (1 << 28) |
                     (AT91C_PHY_ADDR << 23) | (PhyReg << 18))  | Value;

  /* Wait until IDLE bit in Network Status register is cleared */
  while (!(pEMAC->EMAC_NSR & AT91C_EMAC_IDLE));
}


// Keil: function added to read PHY
unsigned short read_PHY (unsigned char PhyReg) 
{
  pEMAC->EMAC_MAN = (AT91C_EMAC_SOF & (0x01<<30)) | (2 << 16) | (2 << 28) |
                    (AT91C_PHY_ADDR << 23) | (PhyReg << 18);

  /* Wait until IDLE bit in Network Status register is cleared */
  while (!(pEMAC->EMAC_NSR & AT91C_EMAC_IDLE));
  return (pEMAC->EMAC_MAN & 0x0000ffff);
}


// Keil: function added to initialize Rx Descriptors
void rx_descr_init (void)
{
  unsigned int i;

  RxBufIndex = 0;
  for (i = 0; i < NUM_RX_BUF; i++) {
    Rx_Desc[i].addr = RX_BUF(i);
    Rx_Desc[i].stat = 0;
  }
  /* Set the WRAP bit at the end of the list descriptor. */
  Rx_Desc[NUM_RX_BUF-1].addr |= 0x02;
  /* Set Rx Queue pointer to descriptor list. */
  pEMAC->EMAC_RBQP = (unsigned int)&Rx_Desc[0];
}


// Keil: function added to initialize Tx Descriptors
void tx_descr_init (void) {
  unsigned int i;

   for (i = 0; i < NUM_TX_BUF; i++) {
      Tx_Desc[i].addr = TX_BUF(i);
      Tx_Desc[i].stat = 0;
   }
   /* Set the WRAP bit at the end of the list descriptor. */
   Tx_Desc[NUM_TX_BUF-1].stat |= TD_TRANSMIT_WRAP;
   /* Set Tx Queue pointer to descriptor list. */
   pEMAC->EMAC_TBQP = (unsigned int)&Tx_Desc[0];
}


// configure port-pins for use with LAN-controller,
// reset it and send the configuration-sequence

void Init_EMAC(void)
{
// Keil: function modified to access the EMAC
// Initializes the EMAC ethernet controller
  unsigned int regv,tout,id1,id2;

  /* Enable Peripheral Clock for EMAC Peripherals */
  AT91C_BASE_PMC->PMC_PCER  = 1 << AT91C_ID_PIOB;
  AT91C_BASE_PMC->PMC_PCER  = 1 << AT91C_ID_EMAC;

  /* Disable pull up on RXDV => PHY normal mode (not in test mode), */
  /* and set MII mode. PHY has internal pull down.                  */
  pPIOB->PIO_PPUDR = (1<<16) | (1 << 15);

  /* Clear PB18 <=> PHY powerdown */
  AT91C_BASE_PIOB->PIO_PER  = 1<<18;
  AT91C_BASE_PIOB->PIO_OER  = 1<<18;
  AT91C_BASE_PIOB->PIO_CODR = 1<<18;

  /* After PHY power up, hardware reset the PHY */
  pRSTC->RSTC_RCR = 0xA5000000 | AT91C_RSTC_EXTRST;

  /* Wait for hardware reset to end. */
  while (!(pRSTC->RSTC_RSR & AT91C_RSTC_NRSTL));

  /* EMAC IO init for EMAC-PHY communication. */
  AT91C_BASE_PIOB->PIO_ASR  = (AT91C_PB0_ETXCK_EREFCK |
                               AT91C_PB1_ETXEN        |
                               AT91C_PB2_ETX0         |
                               AT91C_PB3_ETX1         |
                               AT91C_PB4_ECRS         |
                               AT91C_PB5_ERX0         |
                               AT91C_PB6_ERX1         |
                               AT91C_PB7_ERXER        |
                               AT91C_PB8_EMDC         |
                               AT91C_PB9_EMDIO        |
                               AT91C_PB10_ETX2        |
                               AT91C_PB11_ETX3        |
                               AT91C_PB12_ETXER       |
                               AT91C_PB13_ERX2        |
                               AT91C_PB14_ERX3        |
                               AT91C_PB15_ERXDV_ECRSDV|
                               AT91C_PB16_ECOL        |
                               AT91C_PB17_ERXCK       );
  AT91C_BASE_PIOB->PIO_PDR  = (AT91C_PB0_ETXCK_EREFCK |
                               AT91C_PB1_ETXEN        |
                               AT91C_PB2_ETX0         |
                               AT91C_PB3_ETX1         |
                               AT91C_PB4_ECRS         |
                               AT91C_PB5_ERX0         |
                               AT91C_PB6_ERX1         |
                               AT91C_PB7_ERXER        |
                               AT91C_PB8_EMDC         |
                               AT91C_PB9_EMDIO        |
                               AT91C_PB10_ETX2        |
                               AT91C_PB11_ETX3        |
                               AT91C_PB12_ETXER       |
                               AT91C_PB13_ERX2        |
                               AT91C_PB14_ERX3        |
                               AT91C_PB15_ERXDV_ECRSDV|
                               AT91C_PB16_ECOL        |
                               AT91C_PB17_ERXCK       );

  /* Enable communication between EMAC-PHY. */
  pEMAC->EMAC_NCR   |= AT91C_EMAC_MPE;
  /* MDC = MCK/32 */
  pEMAC->EMAC_NCFGR |= 2<<10;

  /* Put the DM9161 in reset mode */
  write_PHY (PHY_REG_BMCR, BMCR_RESET);

  /* Wait for hardware reset to end. */
  for (tout = 0; tout < 0x100000; tout++) {
    regv = read_PHY (PHY_REG_BMCR);
    if (!(regv & BMCR_RESET)) {
      /* Reset complete */
      break;
    }
  }

  /* Check if this is a DM9161 PHY. */
  id1 = read_PHY (PHY_REG_PHYID1);
  id2 = read_PHY (PHY_REG_PHYID2);

  if (((id1 << 16) | (id2 & 0xfff0)) == MII_DM9161_ID) {
    /* Configure the PHY device */
    /* Use autonegotiation about the link speed. */
    write_PHY (PHY_REG_BMCR, PHY_AUTO_NEG);
    /* Wait to complete Auto_Negotiation. */
    for (tout = 0; tout < 0x100000; tout++) {
      regv = read_PHY (PHY_REG_BMSR);
      if (regv & BMSR_ANEGCOMPLETE) {
        /* ANEG_ACK set, autonegotiation finished. */
        break;
      }
    }

    /* Check the link status. */
    for (tout = 0; tout < 0x10000; tout++) {
      regv = read_PHY (PHY_REG_BMSR);
      if (regv & BMSR_LINKST) {
        /* Link is on. */
        break;
      }
    }
    regv = read_PHY (PHY_REG_DSCSR);
  }

  /* Update the MAC register NCFGR. */
  pEMAC->EMAC_NCFGR &= ~(AT91C_EMAC_SPD | AT91C_EMAC_FD);

  if (regv & 0xA000) {
    /* Full duplex is enabled. */
    pEMAC->EMAC_NCFGR |= AT91C_EMAC_FD;
  }
  if (regv & 0xC000) {
    /* Speed 100Mbit is enabled. */
    pEMAC->EMAC_NCFGR |= AT91C_EMAC_SPD;
  }

  /* Disable management port in MAC control register. */
  pEMAC->EMAC_NCR &= ~AT91C_EMAC_MPE;

  /* Enable EMAC in MII mode, enable clock ERXCK and ETXCK */
  pEMAC->EMAC_USRIO= AT91C_EMAC_CLKEN;

  /* Transmit and Receive disable. */
  pEMAC->EMAC_NCR &= ~(AT91C_EMAC_RE | AT91C_EMAC_TE);

  /* Initialize Tx and Rx DMA Descriptors */
  rx_descr_init ();
  tx_descr_init ();

  /* The sequence write EMAC_SA1L and write EMAC_SA1H must be respected. */
  pEMAC->EMAC_SA1L = (MYMAC_4 << 24) | (MYMAC_3 << 16) |
                     (MYMAC_2 << 8)  |  MYMAC_1;
  pEMAC->EMAC_SA1H = (MYMAC_6 << 8)  |  MYMAC_5;

  /* Clear receive and transmit status registers. */
  pEMAC->EMAC_RSR  = (AT91C_EMAC_OVR | AT91C_EMAC_REC | AT91C_EMAC_BNA);
  pEMAC->EMAC_TSR  = (AT91C_EMAC_UND | AT91C_EMAC_COMP| AT91C_EMAC_BEX |
                      AT91C_EMAC_RLES| AT91C_EMAC_COL | AT91C_EMAC_UBR);

  /* Configure EMAC operation mode. */
  pEMAC->EMAC_NCFGR |= (AT91C_EMAC_BIG | AT91C_EMAC_DRFCS);
  pEMAC->EMAC_NCR   |= (AT91C_EMAC_TE  | AT91C_EMAC_RE | AT91C_EMAC_WESTAT);
}


// reads a word in little-endian byte order from RX_BUFFER

unsigned short ReadFrame_EMAC(void)
{
  if (rcnt == 0) {
    rptr = (unsigned short *)(Rx_Desc[RxBufIndex].addr & ~3);
    if (++RxBufIndex == NUM_RX_BUF) RxBufIndex = 0;
    rcnt = ETH_RX_BUF_SIZE;
  }
  rcnt -= 2;
  return (*rptr++);
}

// reads a word in big-endian byte order from RX_FRAME_PORT
// (useful to avoid permanent byte-swapping while reading
// TCP/IP-data)

unsigned short ReadFrameBE_EMAC(void)
{
  unsigned short ReturnValue;

  ReturnValue = SwapBytes (ReadFrame_EMAC ());
  return (ReturnValue);
}


// copies bytes from frame port to MCU-memory
// NOTES: * an odd number of byte may only be transfered
//          if the frame is read to the end!
//        * MCU-memory MUST start at word-boundary

void CopyFromFrame_EMAC(void *Dest, unsigned short Size)
{
  unsigned short * piDest;                       // Keil: Pointer added to correct expression

  piDest = Dest;                                 // Keil: Line added
  while (Size > 1) {
    *piDest++ = ReadFrame_EMAC();
    Size -= 2;
  }
  
  if (Size) {                                         // check for leftover byte...
    *(unsigned char *)piDest = (char)ReadFrame_EMAC();// the LAN-Controller will return 0
  }                                                   // for the highbyte
}

// does a dummy read on frame-I/O-port
// NOTE: only an even number of bytes is read!

void DummyReadFrame_EMAC(unsigned short Size)    // discards an EVEN number of bytes
{                                                // from RX-fifo
  while (Size > 1) {
    ReadFrame_EMAC();
    Size -= 2;
  }
}

// Reads the length of the received ethernet frame and checks if the 
// destination address is a broadcast message or not
// returns the frame length
unsigned short StartReadFrame(void) {
  unsigned short RxLen;
  unsigned int idx,i;

  rcnt = 0;
  idx = RxStartIndex = RxBufIndex;
  for (i = 0; i < NUM_RX_BUF; i++) {
    if (Rx_Desc[idx].stat & RD_EOF) {
      break;
    }
    if (++idx == NUM_RX_BUF) idx = 0;
  }
  RxLen = Rx_Desc[idx].stat & RD_LENGTH_MASK;
  return(RxLen);
}

void EndReadFrame(void) {
  unsigned int idx,j;

  /* DMA free packet. */
  for (idx = RxStartIndex; idx != RxBufIndex;  ) {
    Rx_Desc[idx].addr &= ~AT91C_OWNERSHIP_BIT;
    if (++idx == NUM_RX_BUF) idx = 0;
  }
  /* Check for possible ISR RX event loss. */
  for (j = 0; j < ETH_RX_BUF_NUM; j++) {
    if (!(Rx_Desc[idx].addr & AT91C_OWNERSHIP_BIT)) {
      break;
    }
    if (Rx_Desc[idx].stat & RD_EOF) {
      /* Another packet received, no acknowledge. */
      return;
    }
    if (++idx == NUM_RX_BUF) idx = 0;
  }
  /* No more packets, acknowledge */
  pEMAC->EMAC_RSR = AT91C_EMAC_REC;
}

unsigned int CheckFrameReceived(void) {             // Packet received ?

  // Keil: added to check Rx buffer overflow
  if (pEMAC->EMAC_RSR & AT91C_EMAC_BNA) {
    pEMAC->EMAC_NCR &= ~AT91C_EMAC_RE;              // Stop Rx
    rx_descr_init ();                               // All packet data is invalid
    pEMAC->EMAC_RSR  = AT91C_EMAC_OVR | AT91C_EMAC_REC | AT91C_EMAC_BNA;
    pEMAC->EMAC_NCR |= AT91C_EMAC_RE;               // Restart Rx
  }
  if (pEMAC->EMAC_RSR & AT91C_EMAC_REC)             // more packets received ?
    return(1);
  else 
    return(0);
}

// requests space in EMAC memory for storing an outgoing frame

void RequestSend(unsigned short FrameSize)
{
  while (pEMAC->EMAC_TSR & AT91C_EMAC_TGO);
  tptr = (unsigned short *)(Tx_Desc[0].addr & ~3);
  Tx_Desc[0].stat = (FrameSize | TD_LAST_BUF | TD_TRANSMIT_WRAP);
}

// check if ethernet controller is ready to accept the
// frame we want to send

unsigned int Rdy4Tx(void)
{
  return (1);   // the ethernet controller transmits much faster
}               // than the CPU can load its buffers


// writes a word in little-endian byte order to TX_BUFFER
void WriteFrame_EMAC(unsigned short Data)
{
  *tptr++ = Data;
}

// copies bytes from MCU-memory to frame port
// NOTES: * an odd number of byte may only be transfered
//          if the frame is written to the end!
//        * MCU-memory MUST start at word-boundary

void CopyToFrame_EMAC(void *Source, unsigned int Size)
{
  unsigned short * piSource;

  piSource = Source;
  Size = (Size + 1) & 0xFFFE;    // round Size up to next even number
  while (Size > 0) {
    WriteFrame_EMAC(*piSource++);
    Size -= 2;
  }

  /* Start frame transmission. */
  pEMAC->EMAC_NCR |= AT91C_EMAC_TSTART;
}

