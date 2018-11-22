/******************************************************************************/
/*  This file is part of the ARM Toolchain package                            */
/*  Copyright KEIL ELEKTRONIK GmbH 2005 - 2010                                */
/******************************************************************************/
/*                                                                            */
/*  Bootloader.C 1-st level bootloader for AT91SAM9XE Devices                 */
/*                                                                            */
/******************************************************************************/

#include <AT91SAM9XE512.H>              // AT91SAM9XE512 definitions
#include "FlashHW.h"
#include "Error.h"                      // Error definitions

#define STACK_SIZE   64                 // Stack Size

#define DF_OFFSET    0x4200             // User Program offset in DataFlash
#define EXT_SDRAM    (0x20000000)       // Start address of external SDRAM

// Pointer to start of the External SDRAM where Application is copied
typedef void (*fnc)(void);
const fnc AppEntry = (fnc) EXT_SDRAM;

unsigned char buf[32];                  // Buffer for reading data

/**************************** Auxiliary Functions ******************************/

/*- GetProgramSize () ----------------------------------------------------------
 *
 *  Get size of user program to be copied
 *    Parameter:
 *    Return Value:  size in bytes
 */

static unsigned int GetProgramSize (void) {

  ReadPage_HW(DF_OFFSET, 32, buf);

  return (*(unsigned long *)&buf[0x14]);
}


/*************************** 1-st Level Bootloader *****************************/

/*- main () --------------------------------------------------------------------
 *
 *  C routine for DataFlash Initialization and copying code from External Flash
 *  to External SDRAM
 *    Parameter:
 *    Return Value:   0 - OK,  1 - Failed
 */
           
int main (void) {

  unsigned long prog_size, page_size;
  unsigned long i;

  *AT91C_MATRIX_MRCR = 0;               // Disable Remap

  InitFlashController_HW(0, 0, 0, 18432000);

  // Copy code from External Flash to External SDRAM
  prog_size  = GetProgramSize ();       // Number of bytes to copy
  if (prog_size == 0xFFFFFFFF)          // If no program exists in Ext Flash
    while (1);                          // then wait forever
  if (prog_size == 0xE1A00000)          // Unknown size (NOP instruction)
    prog_size = 0x83BE00;               // Use maximum Dataflsh size

  // Get page size for copying
  page_size  = (unsigned long) GetPageSize_HW();

  // Copy code from External Flash to External SDRAM
  for (i = 0; i < prog_size; i += page_size)
    ReadPage_HW(DF_OFFSET+i, page_size, (unsigned char *)(EXT_SDRAM+i));

  // Uninitalize hardware
  UnInit_HW();

  AppEntry();                           // Start the copied Application

  return (0);
}

/*******************************************************************************/
