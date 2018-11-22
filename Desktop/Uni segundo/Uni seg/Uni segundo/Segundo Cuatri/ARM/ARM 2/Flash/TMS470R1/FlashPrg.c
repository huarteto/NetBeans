/******************************************************************************/
/*  This file is part of the ARM Toolchain package                            */
/*  Copyright KEIL ELEKTRONIK GmbH 2003 - 2006                                */
/******************************************************************************/
/*                                                                            */
/*  FlashDev.C:  Flash Programming Functions for TMS470R1 Device Series       */
/*                                                                            */
/******************************************************************************/

#include "..\FlashOS.H"                 // FlashOS Structures
#include "FlashAPI\f05.h"
#include "FlashAPI\flash470.h"

/*- Device specific configurations -------------------------------------------*/

/*--- TMS470R1A64 configuration ----------------------------------------------*/

#ifdef  A64

#define SEC_NUM               5         // Number of all sectors in Flash
#define BANKS_NUM             1         // Number of all banks   in Flash
#define PROT_KEYS_START       0x1FE0    // Start of Protect keys in Flash
#define PROT_KEYS_SIZE        32        // Number of bytes for protection keys

const UINT32 sec_addr[SEC_NUM] = {      // Sector start addresses
  0x000000,0x002000,0x004000,0x008000,0x00C000
};
                                        // Ending addresses of banks
const UINT32 bank_end[BANKS_NUM] = { 0xFFFF };
#endif

/*--- TMS470R1A128 configuration ---------------------------------------------*/

#ifdef  A128

#define SEC_NUM               10        // Number of all sectors in Flash
#define BANKS_NUM             1         // Number of all banks   in Flash
#define PROT_KEYS_START       0x1FE0    // Start of Protect keys in Flash
#define PROT_KEYS_SIZE        32        // Number of bytes for protection keys

const UINT32 sec_addr[SEC_NUM] = {      // Sector start addresses
  0x000000,0x002000,0x004000,0x008000,0x00C000,0x010000,0x014000,0x018000,
  0x01C000,0x01E000
};
                                        // Ending addresses of banks
const UINT32 bank_end[BANKS_NUM] = { 0x1FFFF };
#endif

/*--- TMS470R1A256 configuration ---------------------------------------------*/

#ifdef  A256

#define SEC_NUM               14        // Number of all sectors in Flash
#define BANKS_NUM             1         // Number of all banks   in Flash
#define PROT_KEYS_START       0x1FE0    // Start of Protect keys in Flash
#define PROT_KEYS_SIZE        32        // Number of bytes for protection keys

const UINT32 sec_addr[SEC_NUM] = {      // Sector start addresses
  0x000000,0x002000,0x004000,0x006000,0x008000,0x010000,0x018000,0x020000,
  0x028000,0x030000,0x038000,0x03A000,0x03C000,0x03E000
};
                                        // Ending addresses of banks
const UINT32 bank_end[BANKS_NUM] = { 0x3FFFF };
#endif

/*--- TMS470R1A288 configuration ---------------------------------------------*/

#ifdef  A288_BANK0

#define SEC_NUM               4         // Number of all sectors in Flash
#define BANKS_NUM             1         // Number of all banks   in Flash
#define PROT_KEYS_START       0x1FE0    // Start of Protect keys in Flash
#define PROT_KEYS_SIZE        32        // Number of bytes for protection keys

const UINT32 sec_addr[SEC_NUM] = {      // Sector start addresses
  0x000000,0x002000,0x004000,0x006000
};
                                        // Ending addresses of banks
const UINT32 bank_end[BANKS_NUM] = { 0x7FFF };
#endif

#ifdef  A288_BANK1

#define SEC_NUM               4         // Number of all sectors in Flash
#define BANKS_NUM             1         // Number of all banks   in Flash
#define PROT_KEYS_START       0x1FE0    // Start of Protect keys in Flash
#define PROT_KEYS_SIZE        32        // Number of bytes for protection keys

const UINT32 sec_addr[SEC_NUM] = {      // Sector start addresses
  0x000000,0x010000,0x020000,0x030000
};
                                        // Ending addresses of banks
const UINT32 bank_end[BANKS_NUM] = { 0x7FFFF };
#endif

/*--- TMS470R1A384 configuration ---------------------------------------------*/

#ifdef  A384

#define SEC_NUM               18        // Number of all sectors in Flash
#define BANKS_NUM             3         // Number of all banks   in Flash
#define PROT_KEYS_START       0x1FE0    // Start of Protect keys in Flash
#define PROT_KEYS_SIZE        32        // Number of bytes for protection keys

const UINT32 sec_addr[SEC_NUM] = {      // Sector start addresses
  0x000000,0x002000,0x004000,0x008000,0x00C000,0x010000,0x014000,0x018000,
  0x01C000,0x01E000,0x020000,0x028000,0x030000,0x038000,0x040000,0x048000,
  0x050000,0x058000
};
                                        // Ending addresses of banks
const UINT32 bank_end[BANKS_NUM] = { 0x1FFFF, 0x3FFFF, 0x5FFFF };
#endif

/*--- TMS470R1B512 configuration ---------------------------------------------*/

#ifdef  B512

#define SEC_NUM               14        // Number of all sectors in Flash
#define BANKS_NUM             2         // Number of all banks   in Flash
#define PROT_KEYS_START       0x3FE0    // Start of Protect keys in Flash
#define PROT_KEYS_SIZE        32        // Number of bytes for protection keys

const UINT32 sec_addr[SEC_NUM] = {      // Sector start addresses
  0x000000,0x004000,0x008000,0x010000,0x018000,0x020000,0x028000,0x030000,
  0x038000,0x03C000,0x040000,0x050000,0x060000,0x070000
};
                                        // Ending addresses of banks
const UINT32 bank_end[BANKS_NUM] = { 0x3FFFF, 0x7FFFF };
#endif

/*--- TMS470R1B768 configuration ---------------------------------------------*/

#ifdef  B768

#define SEC_NUM               18        // Number of all sectors in Flash
#define BANKS_NUM             3         // Number of all banks   in Flash
#define PROT_KEYS_START       0x3FE0    // Start of Protect keys in Flash
#define PROT_KEYS_SIZE        32        // Number of bytes for protection keys

const UINT32 sec_addr[SEC_NUM] = {      // Sector start addresses
  0x000000,0x004000,0x008000,0x010000,0x018000,0x020000,0x028000,0x030000,
  0x038000,0x03C000,0x040000,0x050000,0x060000,0x070000,0x080000,0x090000,
  0x0A0000,0x0B0000
};
                                        // Ending addresses of banks
const UINT32 bank_end[BANKS_NUM] =  { 0x3FFFF, 0x7FFFF, 0xBFFFF };
#endif

/*--- TMS470R1B1M configuration ----------------------------------------------*/

#ifdef  B1M

#define SEC_NUM               16        // Number of all sectors in Flash
#define BANKS_NUM             2         // Number of all banks   in Flash
#define PROT_KEYS_START       0xFFE0    // Start of Protect keys in Flash
#define PROT_KEYS_SIZE        32        // Number of bytes for protection keys

const UINT32 sec_addr[SEC_NUM] = {      // Sector start addresses
  0x000000,0x010000,0x020000,0x030000,0x040000,0x050000,0x060000,0x070000,
  0x080000,0x090000,0x0A0000,0x0B0000,0x0C0000,0x0D0000,0x0E0000,0x0F0000
};
                                        // Ending addresses of banks
const UINT32 bank_end[BANKS_NUM] = { 0x7FFFF, 0xFFFFF };
#endif

/*----------------------------------------------------------------------------*/


// Flash status structure
FLASH_STATUS_ST flash_status;

// Flash Delay parameter
UINT32 Delay;


/** Auxiliary functions *******************************************************/

/*- GetBank (...) ------------------------------------------
 *
 *  Get Bank number in which the requested address is
 *    Parameter:      adr:  Data Address
 *    Return Value:   0..7 - Bank Index, 0xFF - Unexisting Bank
 */

int GetBank (unsigned long adr)  {
  int i;

  // Find out in which bank is given address
  for (i = 0; i < BANKS_NUM; i++) {
    if (adr < bank_end[i]) return (i);
  }

  return (0xFF);
}


/** Exported functions ********************************************************/

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

  Delay = (537*(clk >> 10)) >> 20;      // clk / 2MHz 

  switch (fnc)  {
    case 1:                             // Erase   Init
      for (i = 0; i < SEC_NUM; i++) {
        if (Flash_Compact_B ( (UINT32 *) sec_addr[i],
                              (FLASH_CORE) GetBank(sec_addr[i]),
                              (FLASH_SECT) 0,
                              Delay,
                              (FLASH_ARRAY_ST) 0xFFE88000,
                              (FLASH_STATUS_ST *) &flash_status) == 0 )
          return (1);                   // Compaction Failed
      }
      break;
    case 2:                             // Program Init
      break;
    case 3:                             // Verify  Init
      break;
  }

  return (0);
}


/*- UnInit (...) ---------------------------------------------------------------
 *
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

  if (Flash_Blank_B ( (UINT32 *) adr,
                      sz>>2,            // Size in 32-bit words
                      (FLASH_CORE) GetBank(adr),
                      (FLASH_ARRAY_ST) 0xFFE88000,
                      (FLASH_STATUS_ST *) &flash_status) == 0 )
    flash_status.stat1 = 0x00000000;
  else
    flash_status.stat1 = 0x12345678;

  return (1);                           // Always Force Erase
}


/*- EraseSector (...) ----------------------------------------------------------
 *
 *  Erase Sector in Flash Memory
 *    Parameter:      adr:  Sector Address
 *    Return Value:   0 - OK,  1 - Failed
 */

int EraseSector (unsigned long adr) {

  if (Flash_Erase_B ( (UINT32 *) adr,
                      0,
                      (FLASH_CORE) GetBank(adr),
                      (FLASH_SECT) 0,
                      Delay,
                      (FLASH_ARRAY_ST) 0xFFE88000,
                      (FLASH_STATUS_ST *) &flash_status) == 0 )
    return (1);                         // Sector Erase Failed

  return (0);                           // Done successfully
}


/*- ProgramPage (...) ----------------------------------------------------------
 *
 *  Program Page in Flash Memory
 *    Parameter:      adr:  Page Start Address
 *                    sz:   Page Size (in bytes)
 *                    buf:  Page Data
 *    Return Value:   0 - OK,  1 - Failed
 */

int ProgramPage (unsigned long adr, unsigned long sz, unsigned char *buf) {

  // Disable writing of the protection keys
#ifndef WRITE_PROT_KEYS
  if ((adr < (PROT_KEYS_START+PROT_KEYS_SIZE) && ((adr+sz) > PROT_KEYS_START)))
    sz -= (adr+sz-PROT_KEYS_START);

  if (sz == 0) return (1);              // Writing of keys attempted
#endif

  if (Flash_Prog_B  ( (UINT32 *) adr,
                      (UINT32 *) buf,
                      sz>>2,            // Size in 32-bit words
                      (FLASH_CORE) GetBank(adr),
                      Delay,
                      (FLASH_ARRAY_ST) 0xFFE88000,
                      (FLASH_STATUS_ST *) &flash_status) == 0 )
    return (1);                         // Programming Page Failed

  return (0);                           // Done successfully
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

  if (Flash_Verify_B ((UINT32 *) adr,
                      (UINT32 *) buf,
                      sz>>2,            // Size in 32-bit words
                      (FLASH_CORE) GetBank(adr),
                      (FLASH_ARRAY_ST) 0xFFE88000,
                      (FLASH_STATUS_ST *) &flash_status) == 0 )
    return (flash_status.stat1);        // Verification Failed (return address)

  return (adr+sz);                      // Done successfully
}
