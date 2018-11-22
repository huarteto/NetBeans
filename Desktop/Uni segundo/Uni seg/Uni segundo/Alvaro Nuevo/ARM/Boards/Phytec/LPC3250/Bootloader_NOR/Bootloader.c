/***********************************************************************/
/*  This file is part of the ARM Toolchain package                     */
/*  Copyright KEIL ELEKTRONIK GmbH 2003 - 2008                         */
/***********************************************************************/
/*                                                                     */
/*  Bootloader.C 2-nd level bootloader for Philips LPC32x0             */
/*                                                                     */
/***********************************************************************/

#include <string.h>

#define STACK_SIZE       64             // Stack Size
#define USER_PROG_OFS    0x8000         // Offset of user program in NOR Flash
#define MAX_USER_PROG_SZ (0x1FC000)     // Maximal size of user program
#define EXT_NOR          (0xE0000000)   // Start address of external NOR Flash
#define EXT_SDRAM        (0x80000000)   // Start address of external SDRAM

// Pointer to start of the External SDRAM where Application is copied
typedef void (*fnc)(void);
const fnc AppEntry = (fnc) EXT_SDRAM;

/*************************** 2-nd Level Bootloader *****************************/

/*- main () --------------------------------------------------------------------
 *
 *  C routine for NOR Flash Initialization and copying code from NOR Flash
 *  to External SDRAM
 *    Parameter:
 *    Return Value:   0 - OK
 */
           
int main (void) {
  unsigned long *src, *dest;
           long size;

  size = (*((         long *) (EXT_NOR + USER_PROG_OFS + 0x14)));
  src  =   ((unsigned long *) (EXT_NOR + USER_PROG_OFS));
  dest =   ((unsigned long *) (EXT_SDRAM));

  if (size == 0xFFFFFFFF) while (1);    // If no user program size information
  if (size == 0xE1A00000)               // Unknown size (NOP instruction)
    size = MAX_USER_PROG_SZ;

  memcpy (dest, src, size);             // Copy user program to Extarnal SDRAM
  AppEntry();                           // Start the user Application
  return (0);
}

/*******************************************************************************/
