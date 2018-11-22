/** @file spi.c 
*   @brief SPI Driver Inmplmentation File
*   @date 11.August.2009
*   @version 1.00.000
*
*   (c) Texas Instruments 2009, All rights reserved.
*/
//#define DEBUGOUTPUT

#include <TMS570.h>
#include "ConfidenceTest.h"

#ifdef DEBUGOUTPUT
#include "GLCD.h"
#include <stdio.h>
#endif

/** @fn void spi3Init(void)
*   @brief Initializes the SPI Driver
*
*   This function initializes the SPI module.
*/
void spi3Init(void)
{

    /** @b intalise SPI3 @b */

    /** bring SPI out of reset */
    spiREG3->GCR0 = 1U;

    /** SPI3 master mode and clock configuration */
    spiREG3->GCR1 = (0 << 24)  /* SPIEN must be enabled after all other configurations */
                  | (0 << 16)  /* LOOPBACK */
                  | (0 <<  8)  /* PWERDOWN */
                  | (1 <<  1)  /* CLKMOD = output */
                  | (1 <<  0); /* MASTER mode */

    /** default value for SPI chip selects */
    spiREG3->CSDEF = 0x0F;

      
    /** disable SPI3 multibuffered mode */
    spiREG3->MIBSPIE = 0U;


    /** SPI3 enable pin configuration */
    /** - set interrupt levels */
    spiREG3->ILVL = (0 << 9)  /* TXINT */
                  | (0 << 8)  /* RXINT */
                  | (0 << 6)  /* OVRNINT */
                  | (0 << 4)  /* BITERR */
                  | (0 << 3)  /* DESYNC */
                  | (0 << 2)  /* PARERR */
                  | (0 << 1)  /* TIMEOUT */
                  | (0 << 0); /* DLENERR */

    /** - clear any pending interrupts */
//  spiREG3->FLG = 0xFFFFFFFFU;

    /** - enable (no) interrupts */
    spiREG3->INT0 = (0 << 9)  /* TXINT */
                  | (0 << 8)  /* RXINT */
                  | (0 << 6)  /* OVRNINT */
                  | (0 << 4)  /* BITERR */
                  | (0 << 3)  /* DESYNC */
                  | (0 << 2)  /* PARERR */
                  | (0 << 1)  /* TIMEOUT */
                  | (0 << 0); /* DLENERR */

    /** no Delays */
    spiREG3->DELAY = (50 << 24) /* C2TDELAY */
                   | (50 << 16) /* T2CDELAY */
                   | (0 <<  8)  /* T2EDELAY */
                   | (0 <<  0); /* C2EDELAY */

    /** - Data Format 0 */
    spiREG3->FMT0 = (0 << 24)   /* wdelay */
                  | (0 << 23)   /* parity Polarity */
                  | (0 << 22)   /* parity enable */
                  | (0 << 21)   /* wait on enable */
                  | (0 << 20)   /* shift direction. 0=MSB fist */
                  | (0 << 18)   /* dis CS timers */
                  | (0 << 17)   /* clock polarity */
                  | (1 << 16)   /* clock phase */
                  | (0xC7 << 8) /* baudrate prescale */ // check if the SPI clock is below 1 MHz)
                  | 8;          /* data word length */


    /** - SPI3 Port output values */
    // These values are only needed when pin is configued as GPIO */
    spiREG3->PCDOUT =  0         /* SCS[0] */ 
                    | (1 << 1)   /* SCS[1] */  // CS1 = high
                    | (0 << 2)   /* SCS[2] */
                    | (0 << 3)   /* SCS[3] */
                    | (0 << 8)   /* ENA */
                    | (0 << 9)   /* CLK */
                    | (0 << 10)  /* SIMO */
                    | (0 << 11); /* SOMI */

    /** - SPI3 Port direction */
    spiREG3->PCDIR  =  0         /* SCS[0] */
                    | (1 << 1)   /* SCS[1] */   // CS1 is an output
                    | (0 << 2)   /* SCS[2] */
                    | (0 << 3)   /* SCS[3] */
                    | (0 << 8)   /* ENA */
                    | (0 << 9)   /* CLK */
                    | (0 << 10)  /* SIMO */
                    | (0 << 11); /* SOMI */

    /** - SPI3 Port open drain enable */
    spiREG3->PCPDR  =  0        /* SCS[0] */
                    | (0 << 1)  /* SCS[1] */
                    | (0 << 2)  /* SCS[2] */
                    | (0 << 3)  /* SCS[3] */
                    | (0 << 8)  /* ENA */
                    | (0 << 9)  /* CLK */
                    | (0 << 10)  /* SIMO */
                    | (0 << 11); /* SOMI */

    /** - SPI3 Port pullup / pulldown selection */
    spiREG3->PCPSL  =  1        /* SCS[0] */
                    | (1 << 1)  /* SCS[1] */
                    | (1 << 2)  /* SCS[2] */
                    | (1 << 3)  /* SCS[3] */
                    | (1 << 8)  /* ENA */
                    | (1 << 9)  /* CLK */
                    | (1 << 10)  /* SIMO */
                    | (1 << 11); /* SOMI */

    /** - SPI3 Port pullup / pulldown enable*/
    spiREG3->PCDIS  =  0         /* SCS[0] */
                    | (0 << 1)   /* SCS[1] */
                    | (0 << 2)   /* SCS[2] */
                    | (0 << 3)   /* SCS[3] */
                    | (0 << 8)   /* ENA */
                    | (0 << 9)   /* CLK */
                    | (0 << 10)  /* SIMO */
                    | (0 << 11); /* SOMI */

    /* SPI3 set all pins to functional */
    spiREG3->PCFUN  =  0         /* SCS[0] */
                    | (0 << 1)   /* SCS[1] */  // SD card CS is handled as IO
                    | (1 << 2)   /* SCS[2] */  // CS of touch screen controller
                    | (0 << 3)   /* SCS[3] */
                    | (0 << 8)   /* ENA */
                    | (1 << 9)   /* CLK */
                    | (1 << 10)  /* SIMO */
                    | (1 << 11); /* SOMI */

    /** SPI3 master mode and clock configuration */
    spiREG3->MIBSPIE = 0; // make sure that SPI is in compatibility mode

    /** SPI3 master mode and clock configuration */
    spiREG3->GCR1 |= (1 << 24);  /* Finaly start SPI3 */

}


/*------ SPI functions for touchscreen controller connected to SPI3/CS2  ----*/

unsigned int Read16BitTSRegister (unsigned char RegAdr) {
   volatile unsigned int dummy;
   unsigned int retval;
   
   // DAT1 must be used (not DAT0) with bit 28 set to collect all transfers to one CS sequence
   RegAdr &= 0x7F;                       // limit to 7 bit address
   while ((spiREG3->FLG & 0x0200) == 0); // Wait until TXINTFLG is set for previous transmission
   spiREG3->DAT1 = RegAdr | 0x100B0080;  // transmit register address

   while ((spiREG3->FLG & 0x0100) == 0); // Wait until RXINTFLG is set when new value is received
   dummy = spiREG3->BUF;                 // dummy read
   
   while ((spiREG3->FLG & 0x0200) == 0); // Wait until TXINTFLG is set for previous transmission
   spiREG3->DAT1 = RegAdr+1 | 0x100B0080;// transmit next register address

   while ((spiREG3->FLG & 0x0100) == 0); // Wait until RXINTFLG is set when new value is received
   retval = (spiREG3->BUF & 0x00FF) << 8;// read low byte of register content
   
   while ((spiREG3->FLG & 0x0200) == 0); // Wait until TXINTFLG is set for previous transmission
   spiREG3->DAT1 = 0x00 | 0x000B0000;    // send 0 address to terminate the sequence

   while ((spiREG3->FLG & 0x0100) == 0); // Wait until RXINTFLG is set when new value is received
   retval |= spiREG3->BUF & 0x00FF;      // read high byte of register content

   return (retval);
}

unsigned int Read8BitTSRegister (unsigned char RegAdr) {
   volatile unsigned int dummy;
   unsigned int retval;
   
   // DAT1 must be used (not DAT0) with bit 28 set to collect all transfers to one CS sequence
   RegAdr &= 0x7F;                       // limit to 7 bit address
   while ((spiREG3->FLG & 0x0200) == 0); // Wait until TXINTFLG is set for previous transmission
   spiREG3->DAT1 = RegAdr | 0x100B0080;  // transmit register address

   while ((spiREG3->FLG & 0x0100) == 0); // Wait until RXINTFLG is set when new value is received
   dummy = spiREG3->BUF;                 // dummy read
   
   while ((spiREG3->FLG & 0x0200) == 0); // Wait until TXINTFLG is set for previous transmission
   spiREG3->DAT1 = 0x00 | 0x000B0000;    // send 0 address to terminate the sequence

   while ((spiREG3->FLG & 0x0100) == 0); // Wait until RXINTFLG is set when new value is received
   retval = spiREG3->BUF & 0x00FF;       // read high byte of register content

   return (retval);
}

void Write8BitTSRegister (unsigned char RegAdr, unsigned char Content) {
   volatile unsigned int dummy;
   
   // DAT1 must be used (not DAT0) with bit 28 set to collect all transfers to one CS sequence
   RegAdr &= 0x7F;                       // limit to 7 bit address
   while ((spiREG3->FLG & 0x0200) == 0); // Wait until TXINTFLG is set for previous transmission
   spiREG3->DAT1 = RegAdr | 0x100B0000;  // transmit register address

   while ((spiREG3->FLG & 0x0100) == 0); // Wait until RXINTFLG is set when new value is received
   dummy = spiREG3->BUF;                 // dummy read
   
   while ((spiREG3->FLG & 0x0200) == 0); // Wait until TXINTFLG is set for previous transmission
   spiREG3->DAT1 = Content | 0x000B0000; // send content of register

   while ((spiREG3->FLG & 0x0100) == 0); // Wait until RXINTFLG is set when new value is received
   dummy = spiREG3->BUF & 0x00FF;        // read high byte of register content

   return;
}

void Touch_Init (void) {
  unsigned int i;
  
  Write8BitTSRegister (0x40, 0x03);       /* TSC_CTRL register X, Y only mode   */
  Write8BitTSRegister (0x03, 0x02);       /* Reset Touch-screen controller      */
  for (i = 0; i < 180000; i++);   /* Wait minimum of 10ms               */
  Write8BitTSRegister (0x04, 0x0C);       /* Enable TSC and ADC                 */
  Write8BitTSRegister (0x0A, 0x07);       /* Enable Touch detect, FIFO          */
  Write8BitTSRegister (0x20, 0x69);       /* Set sample time , 12-bit mode      */
  for (i = 0; i < 36000; i++);    /* Wait minimum of 2ms                */
  Write8BitTSRegister (0x21, 0x01);       /* ADC frequency 3.25 MHz             */
  Write8BitTSRegister (0x41, 0xF5);       /* Set TSC_CFG register               */
  Write8BitTSRegister (0x4A, 0x01);       /* Threshold for FIFO                 */
  Write8BitTSRegister (0x4B, 0x01);       /* FIFO reset                         */
  Write8BitTSRegister (0x4B, 0x00);       /* FIFO not reset                     */
  Write8BitTSRegister (0x56, 0x07);       /* Fraction z                         */
  Write8BitTSRegister (0x58, 0x01);       /* Drive 50 mA typical                */
  Write8BitTSRegister (0x40, 0x01);       /* Enable TSC                         */
  Write8BitTSRegister (0x0B, 0xFF);       /* Clear interrupt status             */
  Write8BitTSRegister (0x09, 0x01);       /* Enable global interrupt            */
}

// this is a preliminary implementation. The calculation factors are measured on one diaplay
// The right way would be that the user calibrates the touchscreen
void Touch2Screen(int * x, int *y) {
  int x1, y1;
  x1 = ((*x) - 250) / 15;  // factors are an approximation
  y1 = ((*y) - 213) / 11;
// the origin of the screen coordinates is the upper left corner, 
// the origin of the touch screen the upper right corner with x and y exchanged
  *x = 320 - y1;
  *y = x1;
}

unsigned int CheckForTouch(TouchTable * TouchButtonTable, unsigned int wait) {
  int tch_x    =  0, tch_y    =  0, tch_z   = 0;
  int tch_int  =  0;
  int tch_smpl =  0;
  static int tch_on   = 0;  // Pen down status
  static int tch_on_p = 0;  // Pen down status
  TouchTable * pTT;
    
  while(1) {
    tch_z = 0;
    tch_int = Read8BitTSRegister (0x0B);    /* Read Touch-screen interrupt status */
    if (tch_int) {                          /* If Touch-screen interrupt active   */
      if (tch_int & 1) {                    /* If touch detected                  */
        if ((Read8BitTSRegister (0x40)) & 0x80) { /* Read Touch-screen status */
          tch_on = 1;
        } else {
          tch_on = 0;
          tch_on_p = 0;
        }
      }      	
      if (tch_int & 2) {                    /* If FIFO is above threshold         */
        tch_smpl = Read8BitTSRegister (0x4C);
        while ((tch_smpl--) > 0) {          // read out fifo and keep last value
          tch_z  = Read8BitTSRegister(0x57) << 24; // Read coordinates 
          tch_z |= Read8BitTSRegister(0x57) << 16;
          tch_z |= Read8BitTSRegister(0x57) <<  8;
          tch_z |= Read8BitTSRegister(0x57);
        }
      }
      Write8BitTSRegister (0x0B, tch_int);      // clear interrupt
      if ((tch_on == 1) && (tch_z != 0) && (tch_on > tch_on_p)) {
        tch_on_p = tch_on; 
        tch_x = (tch_z >> 20) & 0x00000FFF;
        tch_y = (tch_z >>  8) & 0x00000FFF;
        tch_z = (tch_z >>  0) & 0x0000000F;
        Touch2Screen(&tch_x, &tch_y);
        for (pTT = TouchButtonTable; pTT->x_max; pTT++) {
          if ((tch_x >= pTT->x_min) && (tch_x < pTT->x_max) &&
              (tch_y >= pTT->y_min) && (tch_y < pTT->y_max)  ) {
            return(pTT->action);
          }
        }
      }
    } else {
      if (!wait) return (0xffffffff);
    }      
  }
}

/*------------------------------------------------------------------------------
  Set a SPI clock to low/high speed for SD/MMC
 *------------------------------------------------------------------------------*/
void SPI_hiSpeed (int on) {

  spiREG3->FMT0 &= 0xFFFF00FF;   // mask out baudrate prescaler
  if (on == 1) {
                                 // Max. 20 MBit used for Data Transfer.
    spiREG3->FMT0 |= 3 << 8;     // baudrate prescale 80MHz / (3+1) = 20MBit
  } else {
                                 // Max. 400 kBit used in Card Initialization.
    spiREG3->FMT0 |= 199 << 8;   // baudrate prescale 80MHz / (199+1) = 400kBit
  }
}

/*------------------------------------------------------------------------------
  Enable/Disable SPI Chip Select
 *------------------------------------------------------------------------------*/
void SPI_ssel (unsigned long ssel) {
  if (ssel) {
    spiREG3->PCDOUT |=  0x02;    // SCS[1] = high
  } else {
    spiREG3->PCDOUT &= ~0x02;    // SCS[1] = low
  }
}


/*------------------------------------------------------------------------------
  Write and Read a byte on SPI interface
 *------------------------------------------------------------------------------*/
unsigned char SPI_send (unsigned char outb) {

  while ((spiREG3->FLG & 0x0200) == 0); // Wait until TXINTFLG is set for previous transmission
  spiREG3->DAT1 = outb | 0x100D0000;    // transmit register address

  while ((spiREG3->FLG & 0x0100) == 0); // Wait until RXINTFLG is set when new value is received
  return((unsigned char)spiREG3->BUF);  // Return received value
}


