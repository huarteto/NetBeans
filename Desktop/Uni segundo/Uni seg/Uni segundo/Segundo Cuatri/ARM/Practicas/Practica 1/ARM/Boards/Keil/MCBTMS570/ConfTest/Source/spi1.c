/** @file spi.c 
*   @brief SPI Driver Inmplmentation File
*   @date 11.August.2009
*   @version 1.00.000
*
*   (c) Texas Instruments 2009, All rights reserved.
*/

#include <TMS570.h>
#include <string.h>

/** @fn void spi1Init(void)
*   @brief Initializes the SPI Driver
*
*   This function initializes the SPI module.
*/
void spi1Init(void)
{

    /** intalise SPI1 */

    /** bring SPI out of reset */
    spiREG1->GCR0 = 1U;

    /** SPI3 master mode and clock configuration */
    spiREG1->GCR1 = (0 << 24)  /* SPIEN must be enabled after all other configurations */
                  | (0 << 16)  /* LOOPBACK */
                  | (0 <<  8)  /* PWERDOWN */
                  | (1 <<  1)  /* CLKMOD = output */
                  | (1 <<  0); /* MASTER mode */

    /** default value for SPI chip selects */
    spiREG1->CSDEF = 0x0f;     // set all 4 possible CS lines to high

    /** disable SPI1 multibuffered mode */
    spiREG1->MIBSPIE = 0U;

#if 0  // interrupts are not used at this time
    /** - set interrupt levels */
    spiREG1->ILVL = (0 << 9)  /* TXINT */
                  | (0 << 8)  /* RXINT */
                  | (0 << 6)  /* OVRNINT */
                  | (0 << 4)  /* BITERR */
                  | (0 << 3)  /* DESYNC */
                  | (0 << 2)  /* PARERR */
                  | (0 << 1)  /* TIMEOUT */
                  | (0 << 0); /* DLENERR */

    /** - clear any pending interrupts */
//???    spiREG1->FLG = 0xFFFFFFFFU;

    /** - enable (no) interrupts */
    spiREG1->INT0 = (0 << 9)  /* TXINT */
                  | (0 << 8)  /* RXINT */
                  | (0 << 6)  /* OVRNINT */
                  | (0 << 4)  /* BITERR */
                  | (0 << 3)  /* DESYNC */
                  | (0 << 2)  /* PARERR */
                  | (0 << 1)  /* TIMEOUT */
                  | (0 << 0); /* DLENERR */
#endif

    /** SPI1 enable pin configuration */
    /** no Delays */
    spiREG1->DELAY = (50 << 24) /* C2TDELAY */
                   | (50 << 16) /* T2CDELAY */
                   | (0 <<  8)  /* T2EDELAY */
                   | (0 <<  0); /* C2EDELAY */

    /** - Data Format 0 */
    spiREG1->FMT0 = (0 << 24)  /* wdelay */
                  | (0 << 23)  /* parity Polarity */
                  | (0 << 22)  /* parity enable */
                  | (0 << 21)  /* wait on enable */
                  | (0 << 20)  /* shift direction. 0=MSB fist */
                  | (0 << 18)  /* dis CS timers */
                  | (0 << 17)  /* clock polarity */
                  | (1 << 16)  /* clock phase */
                  | (0xC7 << 8)/* baudrate prescale */ // check if the SPI clock is below 1 MHz)
                  | 8;         /* data word length */


    /** - SPI1 Port output values */
#if 0    // These initializations are only needed when pins is configued as GPIO (which is not used now)
    spiREG1->PCDOUT =  0         /* SCS[0] */ 
                    | (0 << 1)   /* SCS[1] */
                    | (0 << 2)   /* SCS[2] */
                    | (0 << 3)   /* SCS[3] */
                    | (0 << 8)   /* ENA */
                    | (0 << 9)   /* CLK */
                    | (0 << 10)  /* SIMO */
                    | (0 << 11); /* SOMI */

    /** - SPI1 Port direction */
    spiREG1->PCDIR  =  0        /* SCS[0] */
                    | (0 << 1)  /* SCS[1] */
                    | (0 << 2)  /* SCS[2] */
                    | (0 << 3)  /* SCS[3] */
                    | (0 << 8)  /* ENA */
                    | (0 << 9)  /* CLK */
                    | (0 << 10)  /* SIMO */
                    | (0 << 11); /* SOMI */

    /** - SPI1 Port open drain enable */
    spiREG1->PCPDR  =  0        /* SCS[0] */
                    | (0 << 1)  /* SCS[1] */
                    | (0 << 2)  /* SCS[2] */
                    | (0 << 3)  /* SCS[3] */
                    | (0 << 8)  /* ENA */
                    | (0 << 9)  /* CLK */
                    | (0 << 10)  /* SIMO */
                    | (0 << 11); /* SOMI */

    /** - SPI1 Port pullup / pulldown selection */
    spiREG1->PCPSL  =  1        /* SCS[0] */
                    | (1 << 1)  /* SCS[1] */
                    | (1 << 2)  /* SCS[2] */
                    | (1 << 3)  /* SCS[3] */
                    | (1 << 8)  /* ENA */
                    | (1 << 9)  /* CLK */
                    | (1 << 10)  /* SIMO */
                    | (1 << 11); /* SOMI */

    /** - SPI1 Port pullup / pulldown enable*/
    spiREG1->PCDIS  =  0         /* SCS[0] */
                    | (0 << 1)   /* SCS[1] */
                    | (0 << 2)   /* SCS[2] */
                    | (0 << 3)   /* SCS[3] */
                    | (0 << 8)   /* ENA    */
                    | (0 << 9)   /* CLK    */
                    | (0 << 10)  /* SIMO   */
                    | (0 << 11); /* SOMI   */
#endif

    /* SPI1 set all pins to functional */
    spiREG1->PCFUN  = (1 << 0)   /* SCS[0] */
                    | (1 << 1)   /* SCS[1] */
                    | (0 << 2)   /* SCS[2] */
                    | (0 << 3)   /* SCS[3] */
                    | (0 << 8)   /* ENA    */
                    | (1 << 9)   /* CLK    */
                    | (1 << 10)  /* SIMO   */
                    | (1 << 11); /* SOMI   */

    /** SPI1 mode */
    spiREG1->MIBSPIE = 0; // make sure that SPI is in compatibility mode

    /** SPI1 master mode and clock configuration */
    spiREG1->GCR1 |= (1 << 24);  /* Finaly start SPI3 */
}




/*------ SPI functions for FRAM connected to SPI1/CS1  ----*/

void ReadFRAMMemory (unsigned int Address, unsigned int Count, unsigned char *Content) {
   volatile unsigned int dummy;
   
   // DAT1 must be used (not DAT0) with bit 28 set to collect all transfers to one CS sequence
   Address &= 0x1FFF;                    // limit to 13 bit address
   while ((spiREG1->FLG & 0x0200) == 0); // Wait until TXINTFLG is set for previous transmission
   spiREG1->DAT1 = 0x03 | 0x100D0000;    // memory read command hold CS

   while ((spiREG1->FLG & 0x0100) == 0); // Wait until RXINTFLG is set when new value is received
   dummy = spiREG1->BUF;                 // dummy read
   
   while ((spiREG1->FLG & 0x0200) == 0); // Wait until TXINTFLG is set for previous transmission
   spiREG1->DAT1 = ((Address >> 8) & 0x00FF) | 0x100D0000;// transmit high byte of address

   while ((spiREG1->FLG & 0x0100) == 0); // Wait until RXINTFLG is set when new value is received
   dummy = spiREG1->BUF;                 // dummy read
   
   while ((spiREG1->FLG & 0x0200) == 0); // Wait until TXINTFLG is set for previous transmission
   spiREG1->DAT1 = (Address & 0x00FF) | 0x100D0000;// transmit low byte of address

   while ((spiREG1->FLG & 0x0100) == 0); // Wait until RXINTFLG is set when new value is received
   dummy = spiREG1->BUF;                 // dummy read
   
   while (Count--) {
     while ((spiREG1->FLG & 0x0200) == 0); // Wait until TXINTFLG is set for previous transmission
     spiREG1->DAT1 = 0x00 | 0x100D0000;  // send dummy bytes to receive bytes

     while ((spiREG1->FLG & 0x0100) == 0); // Wait until RXINTFLG is set when new value is received
     *Content++ = spiREG1->BUF & 0x00FF;   // store read bytes
   }
   while ((spiREG1->FLG & 0x0200) == 0); // Wait until TXINTFLG is set for previous transmission
   spiREG1->DAT1 = 0x00 | 0x000D0000;                 // send dummy bytes to deactivate CS

   while ((spiREG1->FLG & 0x0100) == 0); // Wait until RXINTFLG is set when new value is received
   dummy = spiREG1->BUF;                 // store read bytes
}

void WriteFRAMMemory (unsigned int Address, unsigned int Count, unsigned char *Content) {
   volatile unsigned int dummy;
   
   // DAT1 must be used (not DAT0) with bit 28 set to collect all transfers to one CS sequence
   Address &= 0x1FFF;                    // limit to 13 bit address
   while ((spiREG1->FLG & 0x0200) == 0); // Wait until TXINTFLG is set for previous transmission
   spiREG1->DAT1 = 0x06 | 0x000D0000;                 // write enable

   while ((spiREG1->FLG & 0x0100) == 0); // Wait until RXINTFLG is set when new value is received
   dummy = spiREG1->BUF;                 // dummy read
   
   while ((spiREG1->FLG & 0x0200) == 0); // Wait until TXINTFLG is set for previous transmission
   spiREG1->DAT1 = 0x02 | 0x100D0000;    // memory write command hold CS

   while ((spiREG1->FLG & 0x0100) == 0); // Wait until RXINTFLG is set when new value is received
   dummy = spiREG1->BUF;                 // dummy read
   
   while ((spiREG1->FLG & 0x0200) == 0); // Wait until TXINTFLG is set for previous transmission
   spiREG1->DAT1 = ((Address >> 8) & 0x00FF) | 0x100D0000;// transmit high byte of address

   while ((spiREG1->FLG & 0x0100) == 0); // Wait until RXINTFLG is set when new value is received
   dummy = spiREG1->BUF;                 // dummy read
   
   while ((spiREG1->FLG & 0x0200) == 0); // Wait until TXINTFLG is set for previous transmission
   spiREG1->DAT1 = (Address & 0x00FF) | 0x100D0000;// transmit low byte of address

   while ((spiREG1->FLG & 0x0100) == 0); // Wait until RXINTFLG is set when new value is received
   dummy = spiREG1->BUF;                 // dummy read
   
   while (Count--) {
     while ((spiREG1->FLG & 0x0200) == 0); // Wait until TXINTFLG is set for previous transmission
     if (Count == 0) {
       spiREG1->DAT1 = (*Content++) | 0x000D0000;         // send dummy bytes to receive bytes
     } else {
       spiREG1->DAT1 = (*Content++) | 0x100D0000;  // send dummy bytes to receive bytes
     }
     while ((spiREG1->FLG & 0x0100) == 0); // Wait until RXINTFLG is set when new value is received
     dummy = spiREG1->BUF;               // dummy read
   }
}

// The FM25CL64 device has a memory size of 8K. It is connectd to SPI1/CS1
unsigned int Test_FRAM( void ) {
  unsigned char wbuffer[0x400];
  unsigned char rbuffer[0x400];
  int i;
  
  for (i = 0; i < sizeof(wbuffer); i++) { // fill write buffer with address pattern
  	wbuffer[i] = i;
  }
  for(i = 0; i < 0x2000; i += sizeof(wbuffer)) {
    WriteFRAMMemory (i, sizeof(wbuffer), &wbuffer[0]);
    memset(rbuffer, 0x00, sizeof(rbuffer));               // clear read buffer
    ReadFRAMMemory  (i, sizeof(rbuffer), &rbuffer[0]);
    if (memcmp(wbuffer, rbuffer, sizeof(rbuffer)) != 0) { // is read and write buffer identical?
      return(1);                                          // return error
    }
  }
  return (0);
}


/*------ SPI functions for EEPROM connected to SPI1/CS0  ----*/

void ReadEEMemory (unsigned int Address, unsigned int Count, unsigned char *Content) {
   volatile unsigned int dummy;
   
   // DAT1 must be used (not DAT0) with bit 28 set to collect all transfers to one CS sequence
   Address &= 0x07FF;                    // limit to 13 bit address
   while ((spiREG1->FLG & 0x0200) == 0); // Wait until TXINTFLG is set for previous transmission
   spiREG1->DAT1 = 0x03 | 0x100E0000;    // memory read command hold CS

   while ((spiREG1->FLG & 0x0100) == 0); // Wait until RXINTFLG is set when new value is received
   dummy = spiREG1->BUF;                 // dummy read
   
   while ((spiREG1->FLG & 0x0200) == 0); // Wait until TXINTFLG is set for previous transmission
   spiREG1->DAT1 = ((Address >> 8) & 0x00FF) | 0x100E0000;// transmit high byte of address

   while ((spiREG1->FLG & 0x0100) == 0); // Wait until RXINTFLG is set when new value is received
   dummy = spiREG1->BUF;                 // dummy read
   
   while ((spiREG1->FLG & 0x0200) == 0); // Wait until TXINTFLG is set for previous transmission
   spiREG1->DAT1 = (Address & 0x00FF) | 0x100E0000;// transmit low byte of address

   while ((spiREG1->FLG & 0x0100) == 0); // Wait until RXINTFLG is set when new value is received
   dummy = spiREG1->BUF;                 // dummy read
   
   while (Count--) {
     while ((spiREG1->FLG & 0x0200) == 0); // Wait until TXINTFLG is set for previous transmission
     spiREG1->DAT1 = 0x00 | 0x100E0000;  // send dummy bytes to receive bytes

     while ((spiREG1->FLG & 0x0100) == 0); // Wait until RXINTFLG is set when new value is received
     *Content++ = spiREG1->BUF & 0x00FF;   // store read bytes
   }
   while ((spiREG1->FLG & 0x0200) == 0); // Wait until TXINTFLG is set for previous transmission
   spiREG1->DAT1 = 0x00 | 0x000E0000;                 // send dummy bytes to deactivate CS

   while ((spiREG1->FLG & 0x0100) == 0); // Wait until RXINTFLG is set when new value is received
   dummy = spiREG1->BUF;                 // store read bytes
}

int WriteEEMemory (unsigned int Address, unsigned int Count, unsigned char *Content) {
   volatile unsigned int dummy;
   unsigned int MaxWait;
      
   // DAT1 must be used (not DAT0) with bit 28 set to collect all transfers to one CS sequence
   Address &= 0x07FF;                    // limit to 11 bit address
   while ((spiREG1->FLG & 0x0200) == 0); // Wait until TXINTFLG is set for previous transmission
   spiREG1->DAT1 = 0x06 | 0x000E0000;    // write enable

   while ((spiREG1->FLG & 0x0100) == 0); // Wait until RXINTFLG is set when new value is received
   dummy = spiREG1->BUF;                 // dummy read

   while ((spiREG1->FLG & 0x0200) == 0); // Wait until TXINTFLG is set for previous transmission
   spiREG1->DAT1 = 0x02 | 0x100E0000;    // memory write command hold CS

   while ((spiREG1->FLG & 0x0100) == 0); // Wait until RXINTFLG is set when new value is received
   dummy = spiREG1->BUF;                 // dummy read
   
   while ((spiREG1->FLG & 0x0200) == 0); // Wait until TXINTFLG is set for previous transmission
   spiREG1->DAT1 = ((Address >> 8) & 0x00FF) | 0x100E0000;// transmit high byte of address

   while ((spiREG1->FLG & 0x0100) == 0); // Wait until RXINTFLG is set when new value is received
   dummy = spiREG1->BUF;                 // dummy read
   
   while ((spiREG1->FLG & 0x0200) == 0); // Wait until TXINTFLG is set for previous transmission
   spiREG1->DAT1 = (Address & 0x00FF) | 0x100E0000;// transmit low byte of address

   while ((spiREG1->FLG & 0x0100) == 0); // Wait until RXINTFLG is set when new value is received
   dummy = spiREG1->BUF;                 // dummy read
   
   while (Count--) {
     while ((spiREG1->FLG & 0x0200) == 0); // Wait until TXINTFLG is set for previous transmission
     if (Count == 0) {
       spiREG1->DAT1 = (*Content++) | 0x000E0000; // send byte and deactivate CS
     } else {
       spiREG1->DAT1 = (*Content++) | 0x100E0000; // send byte and keep CS active
     }
     while ((spiREG1->FLG & 0x0100) == 0); // Wait until RXINTFLG is set when new value is received
     dummy = spiREG1->BUF;               // dummy read
   }
   MaxWait = 0x10000;
   do {
     while ((spiREG1->FLG & 0x0200) == 0); // Wait until TXINTFLG is set for previous transmission
     spiREG1->DAT1 = 0x05 | 0x100E0000;    // read status register command

     while ((spiREG1->FLG & 0x0100) == 0); // Wait until RXINTFLG is set when new value is received
     dummy = spiREG1->BUF;                 // dummy read
   
     while ((spiREG1->FLG & 0x0200) == 0); // Wait until TXINTFLG is set for previous transmission
     spiREG1->DAT1 = 0x00 | 0x000E0000;    // dummy byte

     while ((spiREG1->FLG & 0x0100) == 0); // Wait until RXINTFLG is set when new value is received
     dummy = spiREG1->BUF;                 // read status register
   } while (((dummy & 0x0001) == 1) && (MaxWait-- >0));
   return(MaxWait);
}

// The CAT25160 device has a memory size of 2K. It is connectd to SPI1/CS0
unsigned int Test_EEPROM( void ) {
  unsigned char wbuffer[0x20];
  unsigned char rbuffer[0x20];
  int i;
  
  for (i = 0; i < sizeof(wbuffer); i++) { // fill write buffer with address pattern
  	wbuffer[i] = i+0x10;
  }
  for(i = 0; i < 0x0800; i += sizeof(wbuffer)) {
    WriteEEMemory (i, sizeof(wbuffer), &wbuffer[0]);
    memset(rbuffer, 0x00, sizeof(rbuffer));               // clear read buffer
    ReadEEMemory  (i, sizeof(rbuffer), &rbuffer[0]);
    if (memcmp(wbuffer, rbuffer, sizeof(rbuffer)) != 0) { // is read and write buffer identical?
      return(1);                                          // return error
    }
  }
  return (0);
}

