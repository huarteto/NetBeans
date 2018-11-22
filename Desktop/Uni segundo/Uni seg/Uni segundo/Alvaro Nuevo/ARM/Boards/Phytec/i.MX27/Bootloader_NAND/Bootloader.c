/******************************************************************************/
/*  This file is part of the ARM Toolchain package                            */
/*  Copyright KEIL ELEKTRONIK GmbH 2003 - 2008                                */
/******************************************************************************/
/*                                                                            */
/*  Bootloader.C: 2-nd level NAND Flash bootloader for                        */
/*                Freescale i.MX27 Device Series                              */
/*                                                                            */
/******************************************************************************/

#include "FlashHW.h"
#include "NAND_Error.h"                 // Error definitions

#define STACK_SIZE       64             // Stack Size
#define MAX_USER_PROG_SZ (0x07FFC000)   // Maximal size of user program
#define EXT_SDRAM        (0xA0000000)   // Start address of external SDRAM

// Enumerated type names definitions
enum PAGE_TYPE    {SMALL_PAGE = 0, LARGE_PAGE};
enum SEARCH_BLOCK {SEARCH = 0, RESTART_SEARCH};

// Constants values for Small and Large Page
const unsigned int BLOCK_SIZE[2]      = {16384, 131072};  // Block size
const unsigned int BLOCK_SIZE_SHIFT[2]= {   14,     17};  // Blk sz in bit shift
const unsigned int PAGE_USR_SIZE[2]   = {  512,   2048};  // Sz of page usr data

// Global variables used throughout this module
unsigned long nand_block_offset;        // Offset of valid block
unsigned char bus_width;                // Bus width
unsigned char adr_cycles;               // Address word count
unsigned char page_type;                // Page type (small or large)
unsigned int  block_size;               // Block size
unsigned int  block_size_shift;         // Block size in bit shifts
unsigned int  page_usr_size;            // Size of User Page Data
unsigned char data_buf[32];             // Data buffer

// Module's local functions prototypes
static int FindBlock     (unsigned long adr, unsigned int restart);

// Pointer to start of the External SDRAM where Application is copied
typedef void (*fnc)(void);
const fnc AppEntry = (fnc) EXT_SDRAM;


/*************************** 2-nd Level Bootloader *****************************/

/*- main () --------------------------------------------------------------------
 *
 *  C routine for NAND Flash Initialization and copying code from NAND Flash
 *  to External SDRAM
 *    Parameter:
 *    Return Value:   0 - OK,  1 - Failed
 */
           
int main (void) {

  unsigned long i,j;

  // Setup flash access parameters
  bus_width        = 8;
  adr_cycles       = 4;
  page_type        = SMALL_PAGE;
  block_size       = BLOCK_SIZE[page_type];
  block_size_shift = BLOCK_SIZE_SHIFT[page_type];
  page_usr_size    = PAGE_USR_SIZE[page_type];

  // Initialize NAND Flash Controller to work in 8-bit mode, 4 address cycles, 
  // Small page
  InitFlashController_HW(8, 4, SMALL_PAGE, 0);

  // Restart block searching from beginning
  FindBlock(0, RESTART_SEARCH);

  // Read size of application in bytes to be copied 
  // (position 0x14 in first correct block after BLOCK 0)
  FindBlock(0, SEARCH);
  ReadPage_HW(nand_block_offset, 32, data_buf);

  // Copy code from NAND Flash to External SDRAM
  i = *(unsigned long *)(&data_buf[0x14]);  // Number of bytes to copy
  if (i == 0xFFFFFFFF) while (1);       // If no user program size information
  if (i >= MAX_USER_PROG_SZ)            // Size larger then Flash size
    i = MAX_USER_PROG_SZ;

  j = 0;
  while (i) {
    FindBlock(j, 0);
    if (i > page_usr_size) {            // Copy whole page
      ReadPage_HW(nand_block_offset+(j & (block_size-1)), page_usr_size, (unsigned char *)(EXT_SDRAM+j));
      j += page_usr_size;
      i -= page_usr_size;
    } else {                            // Copy less then whole page
      ReadPage_HW(nand_block_offset+(j & (block_size-1)),             i, (unsigned char *)(EXT_SDRAM+j));
      j += i;
      i  = 0;
    }
  }

  AppEntry();                           // Start the copied Application

  return (0);
}


/**************************** Auxiliary Functions ******************************/

/*- FindBlock (...) ------------------------------------------------------------
 *
 *  Find appropriate good block for requested address.
 *    Parameter:     adr:       Address Requested for Setting
 *                   restart:   0 - find operation, 1 - restart search
 *    Return Value:  0 - OK,  1 - Failed
 */

static int FindBlock (unsigned long adr, unsigned int restart) {
  static int last_src_index = -1;
         int src_index;

  if (restart == RESTART_SEARCH) {      // Restart block search
    last_src_index    = -1;
    nand_block_offset =  0;
    return (0);
  }

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
