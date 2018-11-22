/***********************************************************************/
/*  This file is part of the ARM Toolchain package                     */
/*  Copyright KEIL ELEKTRONIK GmbH 2003 - 2008                         */
/***********************************************************************/
/*                                                                     */
/*  Bootloader.C: 2-nd level NAND Flash bootloader for                 */
/*                NXP LPC3000 Device Series                            */
/*                                                                     */
/***********************************************************************/

#include "FlashHW.h"
#include "NAND_Error.h"                 // Error definitions

#define STACK_SIZE       64             // Stack Size
#define MAX_USER_PROG_SZ (0x1FFC000)    // Maximal size of user program
#define EXT_SDRAM        (0x80000000)   // Start address of external SDRAM

// Enumerated type names definitions
enum PAGE_TYPE {SMALL_PAGE = 0, LARGE_PAGE};

// Constants values for Small and Large Page
const unsigned int BLOCK_SIZE[2]      = {16384, 131072};  // Block size
const unsigned int BLOCK_SIZE_SHIFT[2]= {   14,     17};  // Block size in bit shifts
const unsigned int PAGE_USR_SIZE[2]   = {  512,   2048};  // Size of page user data

// Global variables used throughout this module
unsigned long nand_block_offset;        // Offset of valid block
unsigned char ICR;                      // Interface Configuration Data
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
  unsigned char byte;

  if (InitFlashController_HW(8, 4, SMALL_PAGE, 0)) {
    InitFlashController_HW(8, 3, SMALL_PAGE, 0);
  }
  // Read Interface Configuration Data (ICR) and setup interface according to it
  ReadPage_HW(0, 16, data_buf);

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
      InitFlashController_HW(bus_width, adr_cycles, page_type, 0);
      break;
    }
  }

  // Remember block size information
  block_size    = BLOCK_SIZE[page_type];

  // Remember block size information in bit shifts
  block_size_shift = BLOCK_SIZE_SHIFT[page_type];

  // Remember user page size information
  page_usr_size = PAGE_USR_SIZE[page_type];

  // Restart block searching from beginning
  FindBlock(0, 1);

  // Read size of application in bytes to be copied (position 0x14 in first correct block after BLOCK 0)
  FindBlock(0, 0);
  ReadPage_HW(nand_block_offset, 32, data_buf);

  // Copy code from NAND Flash to External SDRAM
  i = *(unsigned long *)(&data_buf[0x14]);  // Number of bytes to copy
  if (i == 0xFFFFFFFF) while (1);       // If no user program size information
  if (i == 0xE1A00000)                  // Unknown size (NOP instruction)
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
