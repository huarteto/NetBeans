/***********************************************************************/
/*  This file is part of the ARM Toolchain package                     */
/*  Copyright KEIL ELEKTRONIK GmbH 2003 - 2007                         */
/***********************************************************************/
/*                                                                     */
/*  Bootloader.C 2-nd level bootloader for Samsung S3C2440             */
/*                                                                     */
/***********************************************************************/

#include "FlashHW.h"
#include "NAND_Error.h"                 // Error definitions

#define STACK_SIZE   64                 // Stack Size

#define EXT_SDRAM    (0x30000000)       // Start address of external SDSRAM

// Enumerated type names definitions
enum PAGE_TYPE {SMALL_PAGE = 0, LARGE_PAGE};

// Constants values for Small and Large Page
const unsigned int BLOCK_SIZE[2]      = {16384, 131072};  // Block size
const unsigned int BLOCK_SIZE_SHIFT[2]= {   14,     17};  // Block size in bit shifts
const unsigned int PAGE_USR_SIZE[2]   = {  512,   2048};  // Size of page user data

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

  InitFlashController_HW (&bus_width, &adr_cycles, &page_type, 0);

  // Remember block size information
  block_size    = BLOCK_SIZE[page_type];

  // Remember block size information in bit shifts
  block_size_shift = BLOCK_SIZE_SHIFT[page_type];

  // Remember user page size information
  page_usr_size = PAGE_USR_SIZE[page_type];

  // Restart block searching from beginning
  FindBlock(0, 1);

  // Position to first correct block after BLOCK 0
  FindBlock(0, 0);

  // Read size of bytes to be copied (position 0x14 in BLOCK 1)
  ReadPage_HW(nand_block_offset, 32, data_buf);

  // Copy code from NAND Flash to External SDRAM
  i = *(unsigned long *)(&data_buf[0x14]);  // Number of bytes to copy
  if (i == 0xFFFFFFFF) {                // If no user program exists wait in endless loop
    while (1);
  }

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
 *  Find appropriate good block for requested address
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
