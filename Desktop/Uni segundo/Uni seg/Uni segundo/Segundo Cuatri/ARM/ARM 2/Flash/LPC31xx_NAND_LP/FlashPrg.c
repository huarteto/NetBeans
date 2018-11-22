/******************************************************************************/
/*  This file is part of the ARM Toolchain package                            */
/*  Copyright KEIL ELEKTRONIK GmbH 2003 - 2009                                */
/******************************************************************************/
/*                                                                            */
/*  FlashDev.C: Flash Programming Functions adapted for NAND Flash programmed */
/*              with NXP LPC313x, LPC314x and LPC315x Device Series           */
/*                                                                            */
/******************************************************************************/

#include "..\FlashOS.H"                 // FlashOS Structures
#include "FlashHW.h"                    // Low level NAND flash declarations
#include "NAND_Error.h"                 // Error definitions

#define M8(adr)  (*((volatile unsigned char  *) (adr)))
#define M16(adr) (*((volatile unsigned short *) (adr)))
#define M32(adr) (*((volatile unsigned long  *) (adr)))

#define STACK_SIZE   64                 // Stack Size

#define IRAM_ADR     0x11029000         // Internal RAM (where 2-nd level bootloader runs)
#define BLOCK_NR     2048               // Number of blocks

// Enumerated type names definitions
enum PAGE_TYPE {SMALL_PAGE = 0, LARGE_PAGE};
enum BOOL_TYPE {     FALSE = 0,       TRUE};

// Constants values for Small and Large Page
const unsigned int   BLOCK_SIZE[2]      = {16384, 131072};  // Block size
const unsigned int   BLOCK_SIZE_SHIFT[2]= {   14,     17};  // Block size in bit shifts
const unsigned int   PAGE_SIZE[2]       = {  528,   2112};  // Size of complete page
const unsigned int   PAGE_USR_SIZE[2]   = {  512,   2048};  // Size of page user data
const unsigned long *CRC32TABLE         = (const unsigned long *)0x12015CBC;

// Global variables used throughout this module
unsigned long base_adr;                 // Base Address
unsigned long nand_block_offset;        // Offset of valid block
unsigned char bus_width;                // Bus width
unsigned char adr_cycles;               // Address word count
unsigned char page_type;                // Page type (small or large)
unsigned int  block_size;               // Block size
unsigned int  block_size_shift;         // Block size in bit shifts
unsigned int  page_size;                // Size of Complete Page
unsigned int  page_usr_size;            // Size of User Page Data
unsigned char data_buf[2112];           // Data buffer

// Module's local functions prototypes
static int           TryInit       (unsigned char bus_wdth, unsigned char adr_cyc, unsigned char page_typ, unsigned long clk);
static unsigned long CalcCRC32     (unsigned char *buf, unsigned long sz);
static int           WriteNANDPara (void);
static int           FindBlock     (unsigned long adr, unsigned int restart);


/************************* Flash Algorithm Functions ***************************/

/*- Init (...) -----------------------------------------------------------------
 *
 *  Initialize Flash Programming Functions
 *    Parameter:      adr:  Device Base Address
 *                    clk:  Clock Frequency (Hz)
 *                    fnc:  Function Code (1 - Erase, 2 - Program, 3 - Verify)
 *    Return Value:   0 - OK,  1 - Failed
 */
           
int Init (unsigned long adr, unsigned long clk, unsigned long fnc) {
  static unsigned char first_run_f = TRUE;

  base_adr = adr;

  if (first_run_f == TRUE) {
    first_run_f = FALSE;

    // Autodetect address cycles and page type
    if (TryInit(16, 5, LARGE_PAGE, clk) == NAND_OK) goto init_ok;
    if (TryInit(16, 5, SMALL_PAGE, clk) == NAND_OK) goto init_ok;
    if (TryInit(16, 4, LARGE_PAGE, clk) == NAND_OK) goto init_ok;
    if (TryInit(16, 4, SMALL_PAGE, clk) == NAND_OK) goto init_ok;
    if (TryInit(16, 3, LARGE_PAGE, clk) == NAND_OK) goto init_ok;
    if (TryInit(16, 3, SMALL_PAGE, clk) == NAND_OK) goto init_ok;
    if (TryInit( 8, 5, LARGE_PAGE, clk) == NAND_OK) goto init_ok;
    if (TryInit( 8, 5, SMALL_PAGE, clk) == NAND_OK) goto init_ok;
    if (TryInit( 8, 4, LARGE_PAGE, clk) == NAND_OK) goto init_ok;
    if (TryInit( 8, 4, SMALL_PAGE, clk) == NAND_OK) goto init_ok;
    if (TryInit( 8, 3, LARGE_PAGE, clk) == NAND_OK) goto init_ok;
    if (TryInit( 8, 3, SMALL_PAGE, clk) == NAND_OK) goto init_ok;

init_ok:

    // Remember block size information
    block_size       = BLOCK_SIZE[page_type];
   
    // Remember block size information in bit shifts
    block_size_shift = BLOCK_SIZE_SHIFT[page_type];

    // Remember page size information
    page_size        = PAGE_SIZE[page_type];

    // Remember user page size information
    page_usr_size    = PAGE_USR_SIZE[page_type];
  }

  // Restart block searching from beginning
  FindBlock(0, 1);

  return (0);
}


/*
 *  De-Initialize Flash Programming Functions
 *    Parameter:      fnc:  Function Code (1 - Erase, 2 - Program, 3 - Verify)
 *    Return Value:   0 - OK,  1 - Failed
 */

int UnInit (unsigned long fnc) {
  return (0);
}


/*- BlankCheck (...) -----------------------------------------------------------
 *
 *  Blank Check Checks if Memory is Blank
 *    Parameter:      adr:  Block Start Address
 *                    sz:   Block Size (in bytes)
 *                    pat:  Block Pattern
 *    Return Value:   0 - OK,  1 - Failed
 */

int BlankCheck (unsigned long adr, unsigned long sz, unsigned char pat) {

  return (1);                           // Always Force Erase
}


/*- EraseChip (...) ------------------------------------------------------------
 *
 *  Erase whole Flash Memory (erases flash and leaves sectors marked as bad)
 *    Parameter:
 *    Return Value:   0 - OK,  1 - Failed
 */

int EraseChip (void) {
  int i;

  for (i = 0; i < BLOCK_NR; i++) {
    if (CheckBlock_HW(i<<block_size_shift) == NAND_OK) {
      if (EraseBlock_HW(i<<block_size_shift) != NAND_OK) {
        return (1);
      }
    }
  }

  return (0);
}



/*- EraseSector (...) ----------------------------------------------------------
 *
 *  Erase Sector in Flash Memory
 *    Parameter:      adr:  Sector Address
 *    Return Value:   0 - OK,  1 - Failed
 */

int EraseSector (unsigned long adr) {

  if (adr == IRAM_ADR) {                // Erasing block for 2-nd level bootloader
    EraseBlock_HW(0);                   // Erase block for NAND parameters
    EraseBlock_HW(block_size);          // Erase block for 2-nd level bootloader
    WriteNANDPara();                    // Write NAND parameters
  } else {                              // Erasing block for program
    FindBlock(adr, 0);
    if (EraseBlock_HW(nand_block_offset) != NAND_OK) return (1);
  }

  return (0);
}


/*- ProgramPage (...) ----------------------------------------------------------
 *
 *  Program Page in Flash Memory
 *    Parameter:      adr:  Page Start Address
 *                    sz:   Page Size
 *                    buf:  Page Data
 *    Return Value:   0 - OK,  1 - Failed
 */

int ProgramPage (unsigned long adr, unsigned long sz, unsigned char *buf) {
  unsigned long adr_dest;
  int stat;

  if (adr < (IRAM_ADR + block_size)) {  // Programming 2-nd level bootloader data
    adr_dest = adr + block_size - IRAM_ADR;
  } else {                              // Programming program data
    FindBlock(adr, 0);
    adr_dest = nand_block_offset + (adr & (block_size-1));
  }

  if (ProgramPage_HW(adr_dest, sz, buf) != NAND_OK) 
    return (1);
  stat = ReadPage_HW(adr_dest, sz, data_buf);
  if (stat != NAND_OK) {
    if (stat == NAND_ECC_ERROR) {
      MarkBlockBad_HW(adr_dest);
    }
    return (1);
  }

  return (0);
}

/*- Verify (...) ---------------------------------------------------------------
 *
 *  Verify Flash Contents
 *    Parameter:      adr:  Start Address
 *                    sz:   Size (in bytes)
 *                    buf:  Data
 *    Return Value:   (adr+sz) - OK, Failed Address
 */

unsigned long Verify (unsigned long adr, unsigned long sz, unsigned char *buf) {
  unsigned long i, adr_dest, in_page_ofs;

  if (adr < (IRAM_ADR + block_size)) {  // Verifying 2-nd level bootloader data
    adr_dest = adr + block_size - IRAM_ADR;
  } else {                              // Verifying program data
    FindBlock(adr, 0);
    adr_dest = nand_block_offset + (adr & (block_size-1));
  }
  in_page_ofs = adr_dest & (page_usr_size-1);

  if (ReadPage_HW(adr_dest, page_usr_size, data_buf)) return (1);
  for (i=0; i<sz; i++)
    if (buf[i] != data_buf[i+in_page_ofs])
      break;

  return (adr+i);
}


/**************************** Auxiliary Functions ******************************/


/*- TryInit (...) --------------------------------------------------------------
 *
 *  Try initializing controller and reading Interface Configuration Data (ICR)
 *    Parameter:     bus_wdth:  Bus Width (8, 16 bit)
 *                   adr_cyc:   Addressing Cycles (3, 4, 5)
 *                   page_typ:  Page Type (0 -Small Page, 1 - Large Page)
 *                   clk:       Clock Frequency (Hz)
 *    Return Value:  0 - Ok, 1 - Fail
 */

static int TryInit (unsigned char bus_wdth, unsigned char adr_cyc, unsigned char page_typ, unsigned long clk) {
  bus_width  = bus_wdth; 
  adr_cycles = adr_cyc; 
  page_type  = page_typ;

  if (InitFlashController_HW(bus_wdth, adr_cyc, page_typ, clk) != NAND_OK)
    return (1);
  if (ReadPage_HW(0, 16, data_buf) != NAND_OK)
    return (1);

  return (0);
}


/*- CalcCRC32 (...) ------------------------------------------------------------
 *
 *  Calculate CRC32 on requested data
 *    Parameter:     buf:       Data
 *                   sz:        Size (in bytes)
 *    Return Value:  Calculated CRC32
 */

static unsigned long CalcCRC32 (unsigned char *buf, unsigned long sz) {
  unsigned long crc32 = 0xFFFFFFFF;

  while (sz--) {
    crc32 = CRC32TABLE[(crc32 ^ *buf++) & 0xFF] ^ (crc32 >> 8);
  }
  return (crc32 ^ 0xFFFFFFFF);
}


/*- WriteNANDPara (...) --------------------------------------------------------
 *
 *  Write NAND Parameters
 *    Parameter:
 *    Return Value:  0 - Ok, 1 - Fail
 */

static int WriteNANDPara (void) {
  int i, stat;

  /* Prepare NAND parameters that will be written to start of the NAND flash */
  for (i = 0; i < 512;  i++)
    data_buf[i]         = 0;

  data_buf[0x00]        = 'N';
  data_buf[0x01]        = 'A';
  data_buf[0x02]        = 'N';
  data_buf[0x03]        = 'D';
  data_buf[0x04]        = 'f';
  data_buf[0x05]        = 'l';
  data_buf[0x06]        = 's';
  data_buf[0x07]        = 'h';
  data_buf[0x08]        = 8;
  M16(&data_buf[0x0A])  = 2048;
  M16(&data_buf[0x0C])  = 512;
  M16(&data_buf[0x0E])  = 64;
  M32(&data_buf[0x10])  = 2048;
  data_buf[0x14]        = 5;
  data_buf[0x15]        = 3;
  data_buf[0x16]        = 1;
  data_buf[0x17]        = 2;
  data_buf[0x18]        = 0x2C;
  data_buf[0x19]        = 0xAA;
  data_buf[0x1A]        = 0xFF;
  data_buf[0x1B]        = 0x15;
  data_buf[0x1C]        = 0x20;
  data_buf[0x1D]        = 'M';
  data_buf[0x1E]        = 'T';
  data_buf[0x1F]        = '2';
  data_buf[0x20]        = '9';
  data_buf[0x21]        = 'F';
  data_buf[0x22]        = '2';
  data_buf[0x23]        = 'G';
  data_buf[0x24]        = '0';
  data_buf[0x25]        = '8';
  M32(&data_buf[0x40])  = 0x00331031;
  M32(&data_buf[0x44])  = 0x31142423;
  data_buf[0x48]        = 5;
  data_buf[0x49]        = 6;
  M32(&data_buf[0xFC])  = CalcCRC32(data_buf, 252);

  if (ProgramPage_HW(0, 256, data_buf) != NAND_OK) 
    return (1);
  stat = ReadPage_HW(0, 256, data_buf);
  if (stat != NAND_OK) {
    return (1);
  }

  for (i = 0; i < 2112;  i++)
    data_buf[i]         = 0xFF;

  M32(&data_buf[0x00]) = 0x00000000;    // Size of bad block list = 0
  data_buf[0x04] = 'B';                 // ASCII "BAD"
  data_buf[0x05] = 'A';
  data_buf[0x06] = 'D';
  data_buf[0x07] =  1 ;                 // Page number = 1
  M32(&data_buf[0x08]) = CalcCRC32(&data_buf[0x00], 8);

  if (ProgramPage_HW(0x800, 2048, data_buf) != NAND_OK) 
    return (1);
  stat = ReadPage_HW(0x800, 2048, data_buf);
  if (stat != NAND_OK) {
    return (1);
  }
  return (0);
}


/*- FindBlock (...) ------------------------------------------------------------
 *
 *  Find appropriate valid block for requested address
 *    Parameter:     adr:       Address Requested for Setting
 *                   restart:   0 - find operation, 1 - restart search from beginning
 *    Return Value:  0 - OK,  1 - Failed
 */

static int FindBlock (unsigned long adr, unsigned int restart) {
  static int last_src_index  = -1;
         int src_index;

  if (restart == 1) {                   // Just restart for block finding starting from beginning
    last_src_index    = -1;
    nand_block_offset =  block_size;    // Skip Blocks 0 and 1 as they are used by 2-nd level bootloader
    return (0);
  }

  adr      -= base_adr;                 // Address to relative
  src_index = adr >> block_size_shift;  // Get requested block index for source

  if (src_index == last_src_index)      // Same block as previously requested
    return (0);

  while (last_src_index < src_index) {  // Find appropriate valid block
    nand_block_offset += block_size;
    if (CheckBlock_HW(nand_block_offset) == NAND_OK) 
      last_src_index++;
  }

  return (0);
}


/*******************************************************************************/
