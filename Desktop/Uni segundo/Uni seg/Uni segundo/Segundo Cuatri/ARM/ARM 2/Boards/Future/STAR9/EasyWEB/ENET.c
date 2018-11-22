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
 *****  Keil: Module modified for use with STmicroelectronics *****
 *****        STR912F ENET Ethernet controller                *****
 *****                                                        *****
 ******************************************************************/

#include "ENET.h"
#include "tcpip.h"
#include <91x_lib.h>

static unsigned short *rptr;
static unsigned short *tptr;
static unsigned int TxBufIndex;
static unsigned int RxBufIndex;
static DMA_Desc Rx_Desc[NUM_RX_BUF];
static DMA_Desc Tx_Desc[NUM_TX_BUF];


// Keil: function added to write PHY
void write_PHY (int PhyReg, int Value)
{
  unsigned int tout;

  ENET_MAC->MIID = Value;
  ENET_MAC->MIIA = STE100P_DEF_ADR | (PhyReg << 6) | MIIA_WR | MIIA_BUSY;

  /* Wait utill operation completed */
  tout = 0;
  for (tout = 0; tout < MII_WR_TOUT; tout++) {
    if ((ENET_MAC->MIIA & MIIA_BUSY) == 0) {
      break;
    }
  }
}


// Keil: function added to read PHY
unsigned short read_PHY (unsigned char PhyReg) 
{
  unsigned int tout;

  ENET_MAC->MIIA = STE100P_DEF_ADR | (PhyReg << 6) | MIIA_BUSY;

  /* Wait until operation completed */
  tout = 0;
  for (tout = 0; tout < MII_RD_TOUT; tout++) {
    if ((ENET_MAC->MIIA & MIIA_BUSY) == 0) {
      break;
    }
  }
  return (ENET_MAC->MIID);
}


// Keil: function added to initialize Rx Descriptors
void rx_descr_init (void)
{
  unsigned int i,next;

  RxBufIndex = 0;
  for (i = 0, next = 0; i < NUM_RX_BUF; i++) {
    if (++next == NUM_RX_BUF) next = 0;
    Rx_Desc[i].Ctrl = ETH_BUF_SIZE | DMA_CTRL_NEXT_EN;
    Rx_Desc[i].Addr = RX_BUF(i);
    Rx_Desc[i].Next = (unsigned int)&Rx_Desc[next] | DMA_NEXT_POL_EN;
    Rx_Desc[i].Stat = DMA_RX_VALID;
  }
  ENET_DMA->RXNDAR = (unsigned int)&Rx_Desc[0] | RXNDAR_NPOL_EN;
}


// Keil: function added to initialize Tx Descriptors
void tx_descr_init (void) {
  unsigned int i,next;

  TxBufIndex = 0;
  for (i = 0, next = 0; i < NUM_TX_BUF; i++) {
    if (++next == NUM_TX_BUF) next = 0;
    Tx_Desc[i].Ctrl = 0;
    Tx_Desc[i].Addr = TX_BUF(i);
    Tx_Desc[i].Next = (unsigned int)&Tx_Desc[next] | DMA_NEXT_POL_EN;
    Tx_Desc[i].Stat = 0;
  }
  ENET_DMA->TXNDAR = (unsigned int)&Tx_Desc[0] | TXNDAR_NPOL_EN;
}


// configure port-pins for use with LAN-controller,
// reset it and send the configuration-sequence

void Init_ENET(void)
{
// Keil: function modified to access the EMAC
// Initializes the EMAC ethernet controller
  unsigned int regv,tout,id1,id2;

  /* Enable GPIO1 Ethernet Pins. */
  GPIO1->DDR         |=  0x9E;
  GPIO1->DR[0x9E<<2]  =  0x00;
  SCU->GPIOTYPE[1]   &= ~0x9E;
  SCU->GPIOIN[1]     &= ~0x9E;
  SCU->GPIOOUT[1]    &= ~0xC3FC;
  SCU->GPIOOUT[1]    |=  0x82A8;

  /* Enable GPIO5 Ethernet Pins, drive MII clock 25MHz. */
  GPIO5->DDR         |=  0x0C;
  GPIO5->DR[0x0C<<2]  =  0x00;
  SCU->GPIOTYPE[5]   &= ~0x0C;
  SCU->GPIOIN[5]     &= ~0x0C;
  SCU->GPIOOUT[5]    &= ~0x00F0;
  SCU->GPIOOUT[5]    |=  0x00A0;

  /* Remove reset for ENET and MAC devices */
  ENET_DMA->SCR &= ~SCR_SRESET;

  /* Initialize MAC control register. */
  ENET_MAC->MCR = MCR_RVFF | MCR_APR | MCR_DCE;

  /* If HCLK > 50MHz enable the following line. */
  //ENET_MAC->MCR |= MAC_PS_DEF;

  /* Setup Tx & Rx burst size. */
  regv = ENET_DMA->SCR & ~(SCR_TX_BURST_SIZE | SCR_RX_BURST_SIZE);
  ENET_DMA->SCR = regv |  (SCR_TX_BURST_DEF  | SCR_RX_BURST_DEF);

  /* Set clock to PCLK */
  ENET_DMA->CCR = (ENET_DMA->CCR & CCR_SEL_CLK) | CCR_SEL_CLK_DEF;

  /* Put the STE100P in reset mode */
  write_PHY (PHY_REG_XCR, 0x8000);

  /* Wait for hardware reset to end. */
  for (tout = 0; tout < 0x100000; tout++) {
    regv = read_PHY (PHY_REG_XCR);
    if (!(regv & 0x8000)) {
      /* Reset complete */
      break;
    }
  }
  /* Check if this is a STE100P PHY. */
  id1 = read_PHY (PHY_REG_PID1);
  id2 = read_PHY (PHY_REG_PID2);

  if (((id1 << 16) | (id2 & 0xFFF0)) == STE100P_ID) {
    /* Configure the PHY device */
    /* Use autonegotiation about the link speed. */
    write_PHY (PHY_REG_XCR, PHY_AUTO_NEG);
    /* Wait to complete Auto_Negotiation. */
    for (tout = 0; tout < 0x100000; tout++) {
      regv = read_PHY (PHY_REG_XSR);
      if (regv & 0x0020) {
        /* ANEG_ACK set, autonegotiation finished. */
        break;
      }
    }
  }

  /* Check the link status. */
  for (tout = 0; tout < 0x10000; tout++) {
    regv = read_PHY (PHY_REG_XCIIS);
    if (!(regv & 0x0010)) {
      /* Link is on. */
      break;
    }
  }
  if (regv & 0x0100) {
    /* Full duplex is enabled. */
    ENET_MAC->MCR |= MCR_FDM;
  }
  else {
    /* Half duplex mode. */
    ENET_MAC->MCR |= MCR_DRO;
  }

  /* Set the Ethernet MAC Address registers */
  ENET_MAC->MAH = (MYMAC_6 << 8)  |  MYMAC_5;
  ENET_MAC->MAL = (MYMAC_4 << 24) | (MYMAC_3 << 16) |
                  (MYMAC_2 << 8)  |  MYMAC_1;

  /* Initialize Tx and Rx DMA Descriptors */
  rx_descr_init ();
  tx_descr_init ();

  /* Force a ENET abort by software */
  ENET_DMA->RXSTR |= RXSTR_DMA_EN;
  ENET_DMA->TXSTR |= TXSTR_DMA_EN;

  /* Setup Descriptor Fetch Delay */
  ENET_DMA->RXSTR = (ENET_DMA->RXSTR & RXSTR_DFETCH_DLY) | RXSTR_DFETCH_DEF |
                     RXSTR_COLL_SEEN | RXSTR_RUNT_FRAME  | RXSTR_FILTER_FAIL;
  ENET_DMA->TXSTR = (ENET_DMA->TXSTR & TXSTR_DFETCH_DLY) | TXSTR_DFETCH_DEF |
                     TXSTR_UNDER_RUN;

  /* Enable ENET interrupts. */
  ENET_DMA->IER   = INT_RX_CURR_DONE | INT_TX_CURR_DONE | INT_RX_DONE;

  /* Reset all interrupts */
  ENET_DMA->ISR   = 0xFFFFFFFF;

  /* Enable receive and transmit mode of MAC Ethernet core */
  ENET_MAC->MCR  |= (MCR_TE | MCR_RE);

  /* Start the receive operation */
  ENET_DMA->RXSTR |= RXSTR_START_FETCH;
}


// reads a word in little-endian byte order from RX_BUFFER

unsigned short ReadFrame_ENET(void)
{
  return (*rptr++);
}

// reads a word in big-endian byte order from RX_FRAME_PORT
// (useful to avoid permanent byte-swapping while reading
// TCP/IP-data)

unsigned short ReadFrameBE_ENET(void)
{
  unsigned short ReturnValue;

  ReturnValue = SwapBytes (*rptr++);
  return (ReturnValue);
}


// copies bytes from frame port to MCU-memory
// NOTES: * an odd number of byte may only be transfered
//          if the frame is read to the end!
//        * MCU-memory MUST start at word-boundary

void CopyFromFrame_ENET(void *Dest, unsigned short Size)
{
  unsigned short * piDest;                       // Keil: Pointer added to correct expression

  piDest = Dest;                                 // Keil: Line added
  while (Size > 1) {
    *piDest++ = ReadFrame_ENET();
    Size -= 2;
  }
  
  if (Size) {                                         // check for leftover byte...
    *(unsigned char *)piDest = (char)ReadFrame_ENET();// the LAN-Controller will return 0
  }                                                   // for the highbyte
}

// does a dummy read on frame-I/O-port
// NOTE: only an even number of bytes is read!

void DummyReadFrame_ENET(unsigned short Size)    // discards an EVEN number of bytes
{                                                // from RX-fifo
  while (Size > 1) {
    ReadFrame_ENET();
    Size -= 2;
  }
}

// Reads the length of the received ethernet frame and checks if the 
// destination address is a broadcast message or not
// returns the frame length
unsigned short StartReadFrame(void) {
  unsigned short RxLen;
  unsigned int idx;

  idx   = RxBufIndex;
  RxLen = Rx_Desc[idx].Stat & DMA_RX_FLEN;
  rptr  = (unsigned short *)(Rx_Desc[idx].Addr & ~3);
  return(RxLen);
}

void EndReadFrame(void) {
  unsigned int idx;

  /* DMA free packet. */
  idx = RxBufIndex;
  Rx_Desc[idx].Stat = DMA_RX_VALID;
  if (++idx == NUM_RX_BUF) idx = 0;
  RxBufIndex = idx;

  /* Check for possible ISR RX event loss. */
  if (Rx_Desc[idx].Stat & DMA_RX_VALID) {
    /* No more packets, acknowledge */
    ENET_DMA->ISR = INT_RX_CURR_DONE;
  }
}

unsigned int CheckFrameReceived(void) {             // Packet received ?

  if (ENET_DMA->ISR & INT_RX_CURR_DONE)             // more packets received ?
    return(1);
  else 
    return(0);
}

// requests space in EMAC memory for storing an outgoing frame

void RequestSend(unsigned short FrameSize)
{
  unsigned int idx;

  idx  = TxBufIndex;
  tptr = (unsigned short *)(Tx_Desc[idx].Addr & ~3);
  Tx_Desc[idx].Ctrl = FrameSize | DMA_CTRL_NEXT_EN;
}

// check if ethernet controller is ready to accept the
// frame we want to send

unsigned int Rdy4Tx(void)
{
  return (1);   // the ethernet controller transmits much faster
}               // than the CPU can load its buffers


// writes a word in little-endian byte order to TX_BUFFER
void WriteFrame_ENET(unsigned short Data)
{
  *tptr++ = Data;
}

// copies bytes from MCU-memory to frame port
// NOTES: * an odd number of byte may only be transfered
//          if the frame is written to the end!
//        * MCU-memory MUST start at word-boundary

void CopyToFrame_ENET(void *Source, unsigned int Size)
{
  unsigned short * piSource;
  unsigned int idx;

  piSource = Source;
  Size = (Size + 1) & 0xFFFE;    // round Size up to next even number
  while (Size > 0) {
    WriteFrame_ENET(*piSource++);
    Size -= 2;
  }

  idx = TxBufIndex;
  Tx_Desc[idx].Stat = DMA_TX_VALID;
  if (++idx == NUM_TX_BUF) idx = 0;
  TxBufIndex = idx;
  /* Start frame transmission. */
  ENET_DMA->TXSTR |= TXSTR_START_FETCH;
}

