/***********************************************************************/
/*  This file is part of the ARM Toolchain package                     */
/*  Copyright KEIL ELEKTRONIK GmbH 2003 - 2007                         */
/***********************************************************************/
/*                                                                     */
/*  FlashDev.C:  NAND Flash Programming Functions                      */
/*               adapted for the Samsung S3C2440 Controller            */
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
unsigned char bus_width;                // Bus width
unsigned char adr_cycles;               // Address word count
unsigned char page_type;                // Page type (small or large)
unsigned int  block_size;               // Block size
unsigned int  block_size_shift;         // Block size in bit shifts
unsigned int  page_size;                // Size of Complete Page
unsigned int  page_usr_size;            // Size of User Page Data
unsigned char data_buf[528];            // Data buffer

// Module's local functions prototypes
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
  static unsigned char first_run_f = TRUE;

  if (first_run_f) {
    first_run_f = 0;

    base_adr = adr;
  
    InitFlashController_HW (&bus_width, &adr_cycles, &page_type, clk);
  
    // Remember block size information
    block_size       = BLOCK_SIZE[page_type];
     
    // Remember block size information in bit shifts
    block_size_shift = BLOCK_SIZE_SHIFT[page_type];
  
    // Remember page size information
    page_size        = PAGE_SIZE[page_type];
  
    // Remember user page size information
    page_usr_size    = PAGE_USR_SIZE[page_type];

    // Restart block searching from beginning
    FindBlock(0, 1);
  }

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
    if (EraseBlock_HW(0) != NAND_OK) return (1);
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

  if (adr < block_size) {               // Programming 2-nd level bootloader data
    adr_dest = adr;
  } else {                              // Programming program data
    FindBlock(adr, 0);
    adr_dest  = nand_block_offset + ((adr-base_adr) & (block_size-1));
  }

  if (ProgramPage_HW(adr_dest, sz, buf) != NAND_OK) return (1);

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
    adr_dest  = adr;
  } else {                              // Verifying program data
    FindBlock(adr, 0);
    adr_dest  = nand_block_offset + ((adr-base_adr) & (block_size-1));
  }
  in_page_ofs = adr_dest & (page_usr_size-1);

  if (ReadPage_HW(adr_dest, page_usr_size, data_buf) != NAND_OK) return (1);

  // Verify if written bytes are same as what was written, if not mark block bad
  for (i=0; i<sz; i++)
    if (buf[i] != data_buf[i+in_page_ofs]) {
      if (adr_dest >= block_size)       // Do not mark block 0 as bad
        MarkBlockBad_HW (nand_block_offset);
      break;
    }

  return (adr+i);
}


/**************************** Auxiliary Functions ******************************/

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
