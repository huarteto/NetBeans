/******************************************************************************/
/*  This file is part of the ARM Toolchain package                            */
/*  Copyright KEIL ELEKTRONIK GmbH 2003 - 2010                                */
/******************************************************************************/
/*                                                                            */
/*  FlashDev.C:  Flash Programming Functions for TMS570LS Device Series       */
/*                                                                            */
/******************************************************************************/

#include "..\FlashOS.H"                 // FlashOS Structures
#include "FlashAPI\f035.h"
#include "FlashAPI\flash470.h"

/*- Device specific configurations -------------------------------------------*/


/*--- TMS570LS10XX configuration ----------------------------------------------*/

#ifdef  TMS570LS10XX

#define SEC_NUM               14        // Number of all sectors in Flash
#define BANKS_NUM             2         // Number of all banks   in Flash
//#define PROT_KEYS_START       0xFFE0    // Start of Protect keys in Flash
//#define PROT_KEYS_SIZE        32        // Number of bytes for protection keys

const UINT32 sec_addr[SEC_NUM] = {      // Sector start addresses
  0x000000,0x08000,0x10000,0x18000,0x1A000,0x1C000,0x20000,0x30000,0x40000,0x60000,
  0x80000,0xA0000,0xC0000,0xE0000
};
                                        // Ending addresses of banks
const UINT32 bank_end[BANKS_NUM] = { 0x7FFFF, 0xFFFFF};
#endif

/*--- TMS570LS20XX configuration ----------------------------------------------*/

#ifdef  TMS570LS20XX

#define SEC_NUM               22        // Number of all sectors in Flash
#define BANKS_NUM             4         // Number of all banks   in Flash
//#define PROT_KEYS_START       0xFFE0    // Start of Protect keys in Flash
//#define PROT_KEYS_SIZE        32        // Number of bytes for protection keys

const UINT32 sec_addr[SEC_NUM] = {      // Sector start addresses
  0x000000,0x08000,0x10000,0x18000,0x1A000,0x1C000,0x20000,0x30000,0x40000,0x60000,
  0x80000,0xA0000,0xC0000,0xE0000,0x100000,0x120000,0x140000,0x160000,0x180000,0x1A0000,
  0x1C0000, 0x1E0000

};
                                        // Ending addresses of banks
const UINT32 bank_end[BANKS_NUM] = { 0x7FFFF, 0xFFFFF, 0x17FFFF, 0x1FFFFF };
#endif

/*--- TMS570LS10XXECC configuration ----------------------------------------------*/
#ifdef TMS570LS10XXECC
#define BANKS_NUM             2         // Number of all banks   in Flash
                                        // Ending addresses of banks
const UINT32 bank_end[BANKS_NUM] = { 0x43FFFF, 0x47FFFF};
#endif
/*--- TMS570LS20XXECC configuration ----------------------------------------------*/
#ifdef TMS570LS20XXECC
#define BANKS_NUM             4         // Number of all banks   in Flash
                                        // Ending addresses of banks
const UINT32 bank_end[BANKS_NUM] = { 0x43FFFF, 0x47FFFF, 0x4BFFFF, 0x4FFFFF};
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
#if defined (TMS570LS20XX) || defined (TMS570LS10XX)
  int i;
#endif
  Delay = (537*(clk >> 10)) >> 20;      // clk / 2MHz 
  
  switch (fnc)  {
    case 1:                             // Erase   Init
#if defined (TMS570LS20XX) || defined (TMS570LS10XX)
      for (i = 0; i < SEC_NUM; i++) {
        if (Flash_Compact_B ( (UINT32 *) sec_addr[i],
                              (FLASH_CORE) GetBank(sec_addr[i]),
                              (FLASH_SECT) 0,
                              Delay,
                              (FLASH_ARRAY_ST) 0xFFF87000,
                              (FLASH_STATUS_ST *) &flash_status) == 0 )
          return (1);                   // Compaction Failed
      }
      break;
#else
      return(0);
#endif
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
                      (FLASH_ARRAY_ST) 0xFFF87000,
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
#if defined (TMS570LS20XX) || defined (TMS570LS10XX)

  if (Flash_Erase_B ( (UINT32 *) adr,
                      0,
                      (FLASH_CORE) GetBank(adr),
                      (FLASH_SECT) 0,
                      Delay,
                      (FLASH_ARRAY_ST) 0xFFF87000,
                      (FLASH_STATUS_ST *) &flash_status) == 0 )
    return (1);                         // Sector Erase Failed
#endif
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
//#ifndef WRITE_PROT_KEYS
//  if ((adr < (PROT_KEYS_START+PROT_KEYS_SIZE) && ((adr+sz) > PROT_KEYS_START)))
//    sz -= (adr+sz-PROT_KEYS_START);
//
//  if (sz == 0) return (1);              // Writing of keys attempted
//#endif

  if (Flash_Prog_B  ( (UINT32 *) adr,
                      (UINT32 *) buf,
                      sz>>2,            // Size in 32-bit words
                      (FLASH_CORE) GetBank(adr),
                      Delay,
                      (FLASH_ARRAY_ST) 0xFFF87000,
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
                      (FLASH_ARRAY_ST) 0xFFF87000,
                      (FLASH_STATUS_ST *) &flash_status) == 0 )
    return (flash_status.stat1);        // Verification Failed (return address)

  return (adr+sz);                      // Done successfully
}
