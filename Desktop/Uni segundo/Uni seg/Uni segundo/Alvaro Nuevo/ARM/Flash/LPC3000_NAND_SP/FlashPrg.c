/***********************************************************************/
/*  This file is part of the ARM Toolchain package                     */
/*  Copyright KEIL ELEKTRONIK GmbH 2003 - 2008                         */
/***********************************************************************/
/*                                                                     */
/*  FlashDev.C:  Flash Programming Functions adapted for NAND Flash    */
/*               programmed with NXP LPC3000 Device Series             */
/*                                                                     */
/***********************************************************************/

#include "..\FlashOS.H"                 // FlashOS Structures
#include "FlashHW.h"                    // Low level NAND flash declarations
#include "NAND_Error.h"                 // Error definitions

#define STACK_SIZE   64                 // Stack Size

// Enumerated type names definitions
enum PAGE_TYPE {SMALL_PAGE = 0, LARGE_PAGE};
enum BOOL_TYPE {     FALSE = 0,       TRUE};

// Constants values for Small and Large Page
const unsigned int BLOCK_SIZE[2]      = {16384, 131072};  // Block size
const unsigned int BLOCK_SIZE_SHIFT[2]= {   14,     17};  // Block size in bit shifts
const unsigned int PAGE_SIZE[2]       = {  528,   2112};  // Size of complete page
const unsigned int PAGE_USR_SIZE[2]   = {  512,   2048};  // Size of page user data

// Global variables used throughout this module
unsigned long base_adr;                 // Base Address
unsigned long nand_block_offset;        // Offset of valid block
unsigned char ICR;                      // Interface Configuration Data
unsigned char SZ_INFO = 0x1F ;          // Size info for bootloader (1-st level from ROM) (copy 31 pages)
unsigned char bus_width;                // Bus width
unsigned char adr_cycles;               // Address word count
unsigned char page_type;                // Page type (small or large)
unsigned int  block_size;               // Block size
unsigned int  block_size_shift;         // Block size in bit shifts
unsigned int  page_size;                // Size of Complete Page
unsigned int  page_usr_size;            // Size of User Page Data
unsigned char data_buf[2112];           // Data buffer

// Module's local functions prototypes
static int WriteBootInfo (void);
static int TryInit       (unsigned char bus_wdth, unsigned char adr_cyc, unsigned char page_typ, unsigned long clk);
static int FindBlock     (unsigned long adr, unsigned int restart);


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
  int i;
  unsigned char byte, temp;
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

    // Check if Interface Configuration Data (ICR) are valid and setup interface accordingly
    for (i=0; i<16; i=i+4) {
      byte = data_buf[i];

      // Check if ICR valid
      if (((byte & 0x0F) ^ (byte >> 4)) == 0x0F) {

        ICR = byte;
        if (ICR & 0x01) bus_width = 16;
        else            bus_width =  8;
  
        if (ICR & 0x04) {
          page_type = LARGE_PAGE;
          if (ICR & 0x02) adr_cycles = 5;
          else            adr_cycles = 4;
        } else {
          page_type = SMALL_PAGE;
          if (ICR & 0x02) adr_cycles = 4;
          else            adr_cycles = 3;
        }
        if (TryInit(bus_width, adr_cycles, page_type, clk) == 0) {
          // Restart block searching from beginning
          FindBlock(0, 1);

          return (0);
        }
      }
    }

    // If no ICRs were valid erase first block detect bus width 
    // and setup new ICRs and write them to page 0 of bank 0
    EraseBlock_HW(0);

    // Check if 8-bit bus width not already autodetected
    if (bus_width != 8) {
      if (InitFlashController_HW(16, adr_cycles, page_type, clk) == NAND_OK) {
        for (i=0; i<page_size; i++) data_buf[i]=0xFF;
        data_buf[0] = 0x55;
        data_buf[1] = 0xAA;
        ProgramPage_HW (0, 2, data_buf);
        ReadPage_HW    (0, 2, data_buf);
        if ((data_buf[0] != 0x55) || (data_buf[1] != 0xAA)) {
          // 8-bit bus width detected
          InitFlashController_HW(8, adr_cycles, page_type, clk);
        } else {
          // 16-bit bus width detected, it is already used
        }
      }
    }

    // Setup Interface Configuration Data
    temp = 0;
    if (bus_width == 16) temp = 1;
    if (page_type == SMALL_PAGE && adr_cycles == 4) 
      temp |= 0x02;
      
    if (page_type == LARGE_PAGE) {
      temp |= 0x04;
      if (adr_cycles == 5) temp |= 0x02;
    }           
    
    // Generate ICR
    ICR = (temp | temp << 4) ^ 0xF0;

    // Clear data buffer
    for (i=0; i<page_size; i++) data_buf[i]=0xFF;

    // Setup ICRs to data buffer to be written to page 0 of bank 0
    for (i=0; i<16; i=i+4) data_buf[i] = ICR;

    // Write ICRs to flash to page 0 of bank 0
    ProgramPage_HW (0, 16, data_buf);
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


/*- EraseSector (...) ----------------------------------------------------------
 *
 *  Erase Sector in Flash Memory
 *    Parameter:      adr:  Sector Address
 *    Return Value:   0 - OK,  1 - Failed
 */

int EraseSector (unsigned long adr) {

  if (adr == 0) {                       // Erasing block for 2-nd level bootloader
    EraseBlock_HW(0);
    if (WriteBootInfo () == 0) return (0);
  } else {                              // Erasing block for program
    FindBlock(adr, 0);
    if (EraseBlock_HW(nand_block_offset) == NAND_OK) return (0);
  }

  return (1);
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

  if (adr < block_size) {               // Programming 2-nd level bootloader data
    adr_dest = adr + page_usr_size;     // skip page 0 of block 0
  } else {                              // Programming program data
    FindBlock(adr, 0);
    adr_dest  = nand_block_offset + (adr & (block_size-1));
  }

  if (ProgramPage_HW(adr_dest, sz, buf) != NAND_OK) return (1);
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

  if (adr < block_size) {               // Verifying 2-nd level bootloader data
    adr_dest = adr + page_usr_size;     // skip page 0 of block 0
  } else {                              // Verifying program data
    FindBlock(adr, 0);
    adr_dest  = nand_block_offset + (adr & (block_size-1));
  }
  in_page_ofs = adr_dest & (page_usr_size-1);

  if (ReadPage_HW(adr_dest, page_usr_size, data_buf)) return (1);
  for (i=0; i<sz; i++)
    if (buf[i] != data_buf[i+in_page_ofs])
      break;

  return (adr+i);
}


/**************************** Auxiliary Functions ******************************/

/*- WriteBootInfo () -----------------------------------------------------------
 *
 *  Try initializing controller and reading Interface Configuration Data (ICR)
 *    Parameter:   
 *    Return Value:  0 - Ok, 1 - Fail
 */

static int WriteBootInfo (void) {
  int i;

  for (i=0;  i<page_size; i++) data_buf[i]=0xFF;   // Clear data buffer
  for (i=0;  i<16; i=i+4) data_buf[i] =  ICR;      // d0, d1, d2, d3  - Setup ICRs
  for (i=16; i<48; i=i+8) data_buf[i] =  SZ_INFO;  // d4, d6, d8, d10 - Setup Size info for bootloader
  for (i=20; i<48; i=i+8) data_buf[i] = ~SZ_INFO;  // d5, d7, d9, d11 - Setup inv Size info for bootloader
  data_buf[48] = 0xaa;                             // d12
  data_buf[49] = 0x00;
  data_buf[50] = 0x00;
  data_buf[51] = 0x00;

  ProgramPage_HW (0, 64, data_buf);

  return (0);
}


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
  if ((ReadPage_HW(0, 16, data_buf) != NAND_OK) && (ReadPage_HW(0, 16, data_buf) != NAND_ECC_ERROR))
    return (1);

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
    last_src_index = -1;
    nand_block_offset = 0;
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
