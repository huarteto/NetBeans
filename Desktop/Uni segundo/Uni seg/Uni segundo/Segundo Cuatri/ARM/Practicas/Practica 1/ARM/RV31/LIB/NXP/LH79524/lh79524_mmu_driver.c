/***********************************************************************
 * $Id:: lh79524_mmu_driver.c 27 2007-08-31 19:35:06Z kevinw           $
 *
 * Project: LH79524 MMU driver
 *
 * Description:
 *     This file contains driver support for the MMU module on the
 *     LH79524
 *
 ***********************************************************************
 * Software that is described herein is for illustrative purposes only  
 * which provides customers with programming information regarding the  
 * products. This software is supplied "AS IS" without any warranties.  
 * NXP Semiconductors assumes no responsibility or liability for the 
 * use of the software, conveys no license or title under any patent, 
 * copyright, or mask work right to the product. NXP Semiconductors 
 * reserves the right to make changes in the software without 
 * notification. NXP Semiconductors also make no representation or 
 * warranty that such application will be suitable for the specified 
 * use without further testing or modification. 
 *
 ***********************************************************************/

#include "lh79524_mmu_driver.h"

/***********************************************************************
 * MMU driver private data
 **********************************************************************/

/* MMU device configuration structure */
STATIC MMU_CFG_T mmucfg;

#ifndef NULL
#define NULL 0
#endif

#define MMU_ALL_CONTROL (MMU_CONTROL_M || \
                         MMU_CONTROL_A || \
                         MMU_CONTROL_C || \
                         MMU_CONTROL_W || \
                         MMU_CONTROL_S || \
                         MMU_CONTROL_R || \
                         MMU_CONTROL_V)

static UNS_32 decode_level2(UNS_32 level2, UNS_32 addr);
static void mmu_write_control_reg(UNS_32 value);
static UNS_32 mmu_read_control_reg(void);
static void mmu_write_cache_reg(UNS_32 value);
static void mmu_write_tlb_reg(UNS_32 value);
static void mmu_write_ttb_reg(UNS_32 value);
                         
/***********************************************************************
 * MMU driver private function for Green Hill tool only
 **********************************************************************/
#if defined (__ghs__)
/* Assembly macro expansions for GHS compilers */
asm UNS_32 getstatus (UNS_32 status)
{
%reg status
    MRC p15, 0, status, c1, c0, 0;
    MOV r0, status
}

asm UNS_32 get_ctrl_val (UNS_32 val)
{
%reg val
    MRC p15, 0, val, c2, c0, 0
    MOV r0, val
}

asm void set_ctrl_val (UNS_32 val)
{
%reg val
    MCR p15, 0, val, c1, c0, 0
    NOP;
    NOP;
    NOP;
    
}

asm void set_cache (UNS_32 rval, UNS_32 cval)
{
%reg rval, cval
    MCR p15,0,rval,c7,c7,0;
    NOP;
    NOP;
    MCR p15,0,cval,c1,c0,0;
    NOP;
    NOP;
}

asm void set_tlb_ops (UNS_32 rval)
{
%reg rval
    MCR p15, 0, rval, MMU_REG_TLB_OPS, c7, 0;
    NOP;
    NOP;
    NOP;
}

asm void set_cache_ops (UNS_32 rval)
{
%reg rval
    MCR p15, 0, rval, c7, c7, 0;
    NOP;
    NOP;
    NOP;
}

asm void set_ttb (UNS_32 rval)
{
%reg rval
    /* set up translation table register */
    MOV r0,rval;
    MCR p15, 0, r0, MMU_REG_TTB, c0, 0;
    NOP;
    NOP;
    NOP;
}

asm UNS_32 enable_mmu (UNS_32 tt_save, UNS_32 cval)
{
%reg tt_save, cval
   /* set domain access control for all privs. */
    MVN r1,#0;
    MCR p15, 0, r1, MMU_REG_DAC, c0, 0;
    /* set up translation table register */
    MOV r0,tt_save;
    MRC p15, 0, r0, MMU_REG_TTB, c0, 0;
    /* get the MMU control register value */
    MRC p15, 0, cval, c1, c0, 0;
    MOV r0, cval 
}
#endif

/**********************************************************************
*
* Function: mmu_map_virtual_to_physical
*
* Purpose:
*  Given a virtual address, return the corresponding
*  physical address
*
* Processing:
*  This function does the same thing as the ARM720T MMU.
*
* Parameters:
*  addr: address of a virtual memory location.
*
* Outputs: None
*
* Returns:
*  The virtual address if the MMU is not on
*  NULL if the virtual address is not mapped.
*  The corresponding physical address if the virtual address is mapped
*
* Notes: 
*  This function counts on the physical address and the 
*  virtual address of the translation table being the same. If
*  they are not the same, then the step that reads the translation
*  table base register won't initialize the tlb variable properly.
*
**********************************************************************/
UNS_32 mmu_map_virtual_to_physical(UNS_32 addr)
{
   register UNS_32 * tlb, tlb_val;
   register UNS_32 status;
   UNS_32 tlb_entry, index, vaddress;
   UNS_32 * page_table, level2;
    
   vaddress = (UNS_32)addr;
   /* read the control register and verify that MMU is on */
   
#if defined (__GNUC__)
    asm ("MRC p15, 0, %0, c1, c0, 0" : "=r" (status));
#else
#if defined (__ghs__)
      status = getstatus (status);
#else
   __asm
   {
      MRC p15,0,status,c1,c0,0;
   }
#endif
#endif

   if ((status & 1) == 0) /* bit 0 is MMU enable bit */
   {
      /* then MMU is off; virtual is physical */
      return vaddress;
   }

   /* read the translation table base register */
#if defined (__GNUC__)
   asm ("MRC p15, 0, %0, c2, c0, 0" : "=r" (tlb_val));
#else
#if defined (__ghs__)
   tlb_val = get_ctrl_val (tlb_val);
#else
   __asm
   {
      MRC p15,0,tlb_val,c2,c0,0;
   }
#endif
#endif

   /* mask off unpredictable bits */
   tlb = (UNS_32 *)(tlb_val & _SBF(14,_BITMASK(18)) );
   /* 
   get the level 1 translation table entry 
   indexed by bits 31:20 of the address
   */
   index = vaddress;
   index >>= 20;
   tlb_entry = tlb[index];
   /* figure out how to use the tlb entry based on the lower two bits */
   switch (tlb_entry & _BITMASK(2))
   {
      case 0:
      /* invalid */
         return 0;
         break;
      case 1:
         /* coarse page tables */
         index = (vaddress >> 12) & _BITMASK(8);
         page_table = (UNS_32 *)(tlb_entry & _SBF(10, _BITMASK(22)) );
         level2 = page_table[index];
         break;
      case 2:
         /* 
         section base -- upper 12 bits of entry is physical memory base
         lower 20 bits of virtual address is offset from that base
         */
         return (tlb_entry & _SBF(20,_BITMASK(12)) ) | 
                (vaddress & _BITMASK(20) );
         break;
      case 3:
         /* fine page tables */
         index = (vaddress >> 10) & _BITMASK(10);
         page_table = (UNS_32 *)(tlb_entry & _SBF(12, _BITMASK(20)) );
         level2 = page_table[index];
         break;
   }
   
   switch (level2 & _BITMASK(2) )
   {
      case 0:
         /* invalid */
         break;
      case 1:
         /* large page table */
         return (level2 & _SBF(16, _BITMASK(16)) ) | 
                (vaddress & _BITMASK(16) );
      case 2:
         /* small page table */
         return (level2 & _SBF(12, _BITMASK(20)) ) | 
                (vaddress & _BITMASK(12) );
      case 3:
         /* reserved */
         break;
   }

   /* this line should not execute */
   return 0;    
}

/**********************************************************************
*
* Function: decode_level2
*
* Purpose:
*  given a level2 descriptor and a physical address, return 
*  the virtual address if the level2 descriptor can map the 
*  physical address. Otherwise, 
*
* Processing:
*
* Parameters: 
*  level2:  level 2 descriptor
*  addr:    the physical address to be converted
*
* Outputs: None
*
* Returns:
*  the virtual address or NULL if the address does not translate.
*
* Notes:
*  CAUTION: This routine counts on the physical address and the 
*  virtual address of the translation table being the same. If
*  they are not the same, then the step that reads the translation
*  table base register won't initialize the tlb variable properly.
*  If page tables are used, then the virtual addresses of the page
*  tables must be the same as the physical addresses or else the page
*  table read will return an invalid value.
*
**********************************************************************/
static UNS_32 decode_level2(UNS_32 level2, UNS_32 addr)
{
   switch (level2 & _BITMASK(2) )
   {
      case 0:
         /* invalid */
         break;
      case 1:
         /* large page table */
         if ( (level2 & _SBF(16, _BITMASK(16))) ==
              (addr & _SBF(16, _BITMASK(16))) )
            return (level2 & _SBF(16, _BITMASK(16)) ) | 
                   (addr & _BITMASK(16) );
      case 2:
         /* small page table */
         if ( (level2 & _SBF(12, _BITMASK(20))) ==
              (addr & _SBF(12, _BITMASK(20))) )
            return (level2 & _SBF(12, _BITMASK(20)) ) | 
                (addr & _BITMASK(12) );
      case 3:
         /* reserved */
         break;
   }
   return 0;
}


/**********************************************************************
*
* Function: mmu_map_physical_to_virtual
*
* Purpose:
*  Given a physical address, return the first corresponding
*  virtual address
*
* Processing:
*
* Parameters:
*  addr:  The physical address
*
* Outputs: None
*
* Returns:
*  The physical address if the MMU is not on
*  NULL if the physical memory is not mapped
*  The first virtual address found in the translation tables that
*  matches the physical address. 
*
* Notes: 
*  This function counts on the physical address and the 
*  virtual address of the translation table being the same. If
*  they are not the same, then the step that reads the translation
*  table base register won't initialize the tlb variable properly.
*
**********************************************************************/
void * mmu_map_physical_to_virtual(UNS_32 addr)
{
   register UNS_32 * tlb, tlb_val;
   register UNS_32 status;
   UNS_32 tlb_entry, index;
   UNS_32 * page_table, level2, index2;
   void * vaddress;
   
   /* 
   do a linear search of the translation tables until the
   physical address is found 
   */
   /* read the control register and verify that MMU is on */
#if defined (__GNUC__)
    asm ("MRC p15, 0, %0, c1, c0, 0" : "=r" (status));
#else
#if defined (__ghs__)
    status = getstatus (status);
#else
   __asm
   {
      mrc p15,0,status,c1,c0,0;
   }
#endif
#endif

   if ((status & 1) == 0) /* bit 0 is MMU enable bit */
   {
      /* then MMU is off; virtual is physical */
      return (void *)addr;
   }
   /* read the translation table base register */
#if defined (__GNUC__)
   asm ("MRC p15, 0, %0, c2, c0, 0" : "=r" (tlb_val));
#else
#if defined (__ghs__)
   tlb_val = get_ctrl_val (tlb_val);
#else
   __asm
   {
      mrc p15,0,tlb_val,c2,c0,0;
   }
#endif
#endif

   /* mask off unpredictable bits */
   tlb = (UNS_32 *)(tlb_val & _SBF(14,_BITMASK(18)) );
   
   /* 
   search until found or all 4096 translation 
   table entries are examined.
   */
   for (index = 0; index < 4096; index ++)
   {
      tlb_entry = tlb[index];
      switch (tlb_entry & _BITMASK(2))
      {
         case 0:
         /* invalid */
            break;
         case 1:
            /* course page tables */
            for (index2 = 0; index2 < 256; index2++)
            {
               page_table = (UNS_32 *)(tlb_entry & _SBF(10, _BITMASK(22)) );
               level2 = page_table[index2];
               if (level2)
               {
                  vaddress = (void *)decode_level2(level2, addr);

                  if (vaddress)
                     return vaddress;
               }
            }
            break;
         case 2:
            /* 
            section base -- upper 12 bits of entry is physical memory base
            lower 20 bits of virtual address is offset from that base
            */
            if ( (tlb_entry & _SBF(20,_BITMASK(12))) 
                  == (addr & _SBF(20,_BITMASK(12))) )
            {
               return (void *)( _SBF(20,index) | (addr & _BITMASK(20)) );
            }
            break;
         case 3:
            /* fine page tables */
            for (index2 = 0; index2 < 256; index2++)
            {
               page_table = (UNS_32 *)(tlb_entry & _SBF(12, _BITMASK(20)) );
               level2 = page_table[index2];
               if (level2)
               {
                  vaddress = (void *)decode_level2(level2, addr);

                  if (vaddress)
                     return vaddress;
               }
            }
            break;
      }
   }
   /* virtual address not found */
   return NULL;
}

/**********************************************************************
*
* Function: mmu_write_control_reg
*
* Purpose:
*  Write a value to MMU control register 
*
* Processing:
*
* Parameters:
*  value:  Value to be written
*
* Outputs: None
*
* Returns: None
*
* Notes: None
*
**********************************************************************/
static void mmu_write_control_reg(UNS_32 value)
{
    #if defined (__GNUC__)
        asm ("MCR p15, 0, %0, c1, c0, 0" : "=r" (value));
    #else
        #if defined (__ghs__)
            set_ctrl_val (value);
        #else
            __asm
            {
                mcr p15, 0, value, c1, c0, 0;
            }
        #endif
    #endif
}

/**********************************************************************
*
* Function: mmu_read_control_reg
*
* Purpose:
*  Read out the value of MMU control register 
*
* Processing:
*
* Parameters: None
*
* Outputs: None
*
* Returns: control registe value 
*
* Notes: None
*
**********************************************************************/
static UNS_32 mmu_read_control_reg(void)
{
    register mmu_control;
    #if defined (__GNUC__)
        asm ("MRC p15, 0, %0, c2, c0, 0" : "=r" (mmu_control));
    #else
        #if defined (__ghs__)
            mmu_control = get_ctrl_val (mmu_control);
        #else
            __asm
            {
                MRC p15, 0, mmu_control, c2, c0, 0;
            }
        #endif
    #endif
    return mmu_control;
}

/**********************************************************************
*
* Function: mmu_write_cache_reg
*
* Purpose:
*  Write a value to MMU cache register
*
* Processing:
*
* Parameters: None
*
* Outputs: None
*
* Returns: None
*
* Notes: Write 0x0 to flush cache
*
**********************************************************************/
static void mmu_write_cache_reg(UNS_32 value)
{
    #if defined (__GNUC__)
        asm ("MCR p15, 0, %0, c7, c7, 0" : "=r" (value));
    #else
        #if defined (__ghs__)
            set_cache_ops (value);
        #else
            __asm
            {
                MCR p15, 0, value, c7, c7, 0;
            }
        #endif
    #endif
}

/**********************************************************************
*
* Function: mmu_write_tlb_reg
*
* Purpose:
*  Write a value to MMU TLB register
*
* Processing:
*
* Parameters: None
*
* Outputs: None
*
* Returns: None
*
* Notes: Write 0x0 to flush TLB - required whenever virtual to physical
*   mapping changed
*
**********************************************************************/
static void mmu_write_tlb_reg(UNS_32 value)
{
    #if defined (__GNUC__)
        asm ("MCR p15, 0, %0, c8, c7, 0" : "=r" (value));
    #else
        #if defined (__ghs__)
            set_tlb_ops (value);
        #else
            __asm
            {
                MCR p15, 0, value, c8, c7, 0;
            }
        #endif
    #endif
}

/**********************************************************************
*
* Function: mmu_write_ttb_reg
*
* Purpose:
*  Write a value to MMU TTB translation table base register to indicate
*   the translation table starting address in the physical memory
*
* Processing:
*
* Parameters: None
*
* Outputs: None
*
* Returns: None
*
* Notes: 
*
**********************************************************************/
static void mmu_write_ttb_reg(UNS_32 value)
{
    #if defined (__GNUC__)
        asm ("MCR p15, 0, %0, c2, c0, 0" : "=r" (value));
    #else
        #if defined (__ghs__)
            set_ttb (value);
        #else
            __asm
            {
                MCR p15, 0, value, c2, c0, 0;
            }
        #endif
    #endif
}

/***********************************************************************
 * MMU driver public functions
 **********************************************************************/
/***********************************************************************
 *
 * Function: mmu_open
 *
 * Purpose: open MMU for further operation
 *
 * Processing:
 *     If init is not FALSE, return 0x00000000 to the caller. Otherwise,
 *     set init to TRUE, return a pointer to the MMU config structure 
 *     to the caller.
 *
 * Parameters:
 *     ipbase: don't care, pass NULL to ipbase
 *     arg   : Not used
 *
 * Outputs: None
 *
 * Returns: 0 to the caller
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 mmu_open(void *ipbase, INT_32 arg)
{
    INT_32 status = 0;

    if (mmucfg.init == FALSE)
    {
        /* Device is valid and not previously initialized */
        mmucfg.init = TRUE;

        /* Return pointer to WDT configuration structure */
        status = (INT_32) &mmucfg;
    }

    return status;
}

/***********************************************************************
 *
 * Function: mmu_close
 *
 * Purpose: close MMU operation
 *
 * Processing:
 *     If init is not TRUE, then return _ERROR to the caller as the
 *     device was not previously opened. Otherwise, set init to
 *     FALSE, and return _NO_ERROR to the caller.
 *
 * Parameters:
 *     devid: Pointer to MMU config structure
 *
 * Outputs: None
 *
 * Returns: The status of the close operation
 *
 * Notes: None
 *
 **********************************************************************/
STATUS mmu_close(INT_32 devid)
{
    MMU_CFG_T *mmucfgptr = (MMU_CFG_T *) devid;
    STATUS status = _ERROR;

    if (mmucfgptr->init == TRUE)
    {
        status = _NO_ERROR;
        mmucfgptr->init = FALSE;
    }

    return status;
}

/***********************************************************************
 *
 * Function: mmu_ioctl
 *
 * Purpose: Watchdog timer configuration block
 *
 * Processing:
 *     This function is a large case block. Based on the passed function
 *     and option values, set or get the appropriate watchdog timer
 *     parameter.
 *
 * Parameters:
 *     devid: Pointer to MMU config structure
 *     cmd:   ioctl command
 *     arg:   ioctl argument
 *
 * Outputs: None
 *
 * Returns: The status of the ioctl operation
 *
 * Notes: None
 *
 **********************************************************************/
STATUS mmu_ioctl(INT_32 devid,
                 INT_32 cmd,
                 INT_32 arg)
{
    MMU_CFG_T *mmucfgptr = (MMU_CFG_T *) devid;
    STATUS status = _ERROR;
    register mmu_control;

    /* Get the MMU control register value */    
    mmu_control = mmu_read_control_reg();

    if (mmucfgptr->init == TRUE)
    {
        status = _NO_ERROR;

        switch (cmd)
        {
            /* Command to enable the MMU, arg = 1 enable MMU, 
                arg = 0 disable MMU */
            case MMU_SET_ENABLE:
                if (arg == 1)
                {
                    mmu_control |= MMU_CONTROL_M;
                    mmu_write_control_reg(mmu_control);
                }
                else if (arg == 0)
                {
                    /* Disable MMU, 1.invalidate cache 2.flush Translation
                    table 3.disable write buffer, 4.disable cache. 5.clear bit 0 in
                    control register */

                    /* Invalidate the cache */
                    mmu_write_cache_reg(0);
                    
                    /* flush the translation table */
                    mmu_write_tlb_reg(0);
                    
                    /* Disable cache, write buffer, disable mmu */
                    mmu_control &= ~(MMU_CONTROL_C | MMU_CONTROL_W | MMU_CONTROL_M);
                    
                    mmu_write_control_reg(mmu_control);
                }
                else
                {
                    status = _ERROR;
                }
                break;
                                
            /* Command to turn on Cache bit in MMU, arg = 1 turn
                on cache, arg = 0 turn off cache */ 
            case MMU_SET_CACHE_ON:
                if (arg == 1)
                {
                    mmu_control |= MMU_CONTROL_C;
                    mmu_write_control_reg(mmu_control);
                }
                else if (arg == 0)
                {
                    mmu_control &= ~MMU_CONTROL_C;
                    mmu_write_control_reg(mmu_control);
                }
                else
                {
                    status = _ERROR;
                }
                break;
                
            /* Command to turn on Write buffer bit in MMY, arg = 1
                turn on write buffer, arg = 0 turn off write buffer */
            case MMU_SET_WRITE_BUFFER_ON:
                if (arg == 1)
                {
                    mmu_control |= MMU_CONTROL_W;
                    mmu_write_control_reg(mmu_control);
                }
                else if (arg == 0)
                {
                    mmu_control &= ~MMU_CONTROL_W;
                    mmu_write_control_reg(mmu_control);
                }
                else
                {
                    status = _ERROR;
                }
                break;
                
            /* Refresh the cache, no arg */
            case MMU_INVALIDATE_CACHE:
                /* invalidate the cache */
                mmu_write_cache_reg(0);
                break;
                
            /* Refresh the MMU translation table, no arg */
            case MMU_FLUSH_TLB:
                /* Flush the TLB register */
                mmu_write_tlb_reg(0);
                break;
            
            /* Set the translation table address in physical memory */
            case MMU_SET_TTB_ADDR:
                /* arg holds the translation table physical address */
                mmu_write_ttb_reg(arg);
                break;
            
            /* MMU map virtual memory to physical memory, arg is the 
                virtural memory, status return physical memory */
            case MMU_MAP_VIRT_PHY:            
                status = mmu_map_virtual_to_physical(arg);
                break;
                
            /* MMU map physical memory to virtual memory, arg is the 
                physical memory, status return virtual memory */
            case MMU_MAP_PHY_VIRT:            
                status = (STATUS)mmu_map_physical_to_virtual(arg);
                break;

            case MMU_GET_STATUS:
                switch (arg)
                {
                    /* Is mmu physical to virtual control turned on */
                    case MMU_GET_ENABLE:
                        if ((mmu_control & MMU_CONTROL_M) != 0)
                        {
                            status = TRUE;
                        }
                        else
                        {
                            status = FALSE;
                        }
                        break;
                    /* Is global cache on */
                    case MMU_GET_CACHE_ON:
                        if ((mmu_control & MMU_CONTROL_C) != 0)
                        {
                            status = TRUE;
                        }
                        else
                        {
                            status = FALSE;
                        }
                        break;
                    /* Is global write buffer turned on */
                    case MMU_GET_WRITE_BUFFER_ON:
                        if ((mmu_control & MMU_CONTROL_W) != 0)
                        {
                            status = TRUE;
                        }
                        else
                        {
                            status = FALSE;
                        }
                        break;
                        
                    default:
                        /* Unsupported parameter */
                        status = _ERROR;
                        break;
                }
                break;

            default:
                /* Unsupported parameter */
                status = _ERROR;
        }
    }

    return status;
}

/***********************************************************************
 *
 * Function: mmu_read
 *
 * Purpose: MMU read function (stub only)
 *
 * Processing:
 *     Return 0 to the caller.
 *
 * Parameters:
 *     devid:     Pointer to watchdog timer config structure
 *     buffer:    Pointer to data buffer to copy to
 *     max_bytes: Number of bytes to read
 *
 * Outputs: None
 *
 * Returns: Number of bytes actually read (always 0)
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 mmu_read(INT_32 devid,
                void *buffer,
                INT_32 max_bytes)
{
    return 0;
}

/***********************************************************************
 *
 * Function: mmu_write
 *
 * Purpose: MMU write function (stub only)
 *
 * Processing:
 *     Return 0 to the caller.
 *
 * Parameters:
 *     devid: Pointer to watchdog timer config structure
 *     buffer:  Pointer to data buffer to copy from
 *     n_bytes: Number of bytes to write
 *
 * Outputs: None
 *
 * Returns: Number of bytes actually written (always 0)
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 mmu_write(INT_32 devid,
                 void *buffer,
                 INT_32 n_bytes)
{
    return 0;
}
