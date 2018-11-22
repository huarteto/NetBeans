/***********************************************************************
 * $Id:: sdk7a404_strataflash.c 14 2007-08-28 16:08:18Z kevinw         $
 *
 * Project: Basic SDK7A400/SDK7A404 StrataFlash driver
 *
 * Description:
 *     This file contains functions used to program 2 StrataFlash
 *     devices configured in x16 mode (with a 32-bit width) on the
 *     SDK7A400 and SDK7A404 EVBs.
 *
 * Notes:
 *     This driver does not attempt to detect and work with every
 *     configuration of StrataFlash connected in a system. It is
 *     targeted to support 2 devices in x16 mode only as configured on
 *     the SDK7A400 and SDK7A404 EVBs.
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
 **********************************************************************/

#include "sdk7a404_strataflash.h"

/***********************************************************************
 * Private types and data
 **********************************************************************/

/* Maximum number of block regions supported */
#define MAX_CFI_REGIONS 8

/* Maximum number of blocks supported */
#define MAX_CFI_BLOCKS 128

/* Structure used to hold device geometry */
typedef struct
{
    UNS_32 baseaddr;    /* Device base address */
    UNS_32 devsize;     /* Device size, 2^n bytes * 2 devices */
    UNS_32 wb_size;     /* Write buffer size in 32-bit words */
    UNS_32 erblocks;    /* Number of erase block regions in device */
    /* Number of blocks in erase region */
    UNS_16 blks[MAX_CFI_REGIONS];
    /* Size of each block in erase region */
    UNS_32 bsize[MAX_CFI_REGIONS];
    UNS_32 blocks;      /* Number of blocks in the device */
    /* List of block pointers */
    UNS_32 blk_addr[MAX_CFI_BLOCKS];
    /* List of block sizes */
    UNS_32 blk_size[MAX_CFI_BLOCKS];
} CFI_DEV_GEOM_T;

/* CFI device geometry data */
CFI_DEV_GEOM_T cfi_dg;

/* CFI standard defines (32-bit) */
#define CFI32_TAG_Q 0x00510051      /* First tag string =  '_Q_Q' */
#define CFI32_TAG_R 0x00520052      /* Second tag string = '_R_R' */
#define CFI32_TAG_Y 0x00590059      /* Third tag string =  '_Y_Y' */

/* CFI query command (32-bit) */
#define CFI32_QUERY_COMMAND 0x00980098

/* CFI exit programming mode command */
#define CFI32_EXITPROG_COMMAND 0xFFFFFFFF

/* CFI read status command */
#define CFI32_READ_STATUS 0x00700070

/* CFI clear status command */
#define CFI32_CLEAR_STATUS_COMMAND 0x00500050

/* CFI set block command */
#define CFI32_SET_BLOCK_COMMAND 0x00600060

/* CFI block confirm clear (lock) command */
#define CFI32_CONFIRM_CLEAR_BLOCK_COMMAND 0x00D000D0

/* CFI block confirm buffer write command */
#define CFI32_CONFIRM_BUFWRITE_COMMAND 0x00D000D0

/* CFI block confirm command */
#define CFI32_CONFIRM_SET_BLOCK_COMMAND 0x00010001

/* CFI block erase command */
#define CFI32_BLOCKERASE_COMMAND 0x00200020

/* CFI byte (word) write command */
#define CFI32_BYTEWRITE_COMMAND 0x00400040

/* CFI write exit command */
#define CFI32_WRITE_EXIT_COMMAND 0x00FF00FF

/* CFI block write command */
#define CFI32_BLOCK_WRITE_COMMAND 0x00E800E8

/* CFI tag (QRY) register offset (32-bit) */
#define CFI32_TAG_OFFSET 0x00000010

/* CFI device geometry device size offset */
#define CFI32_DEVSIZE_OFFSET 0x00000027

/* CFI write buffer size offset */
#define CFI32_WBSIZE_OFFSET 0x0000002A

/* CFI erase block regions number offset */
#define CFI_ERBLK_OFFSET 0x0000002C

/* CFI erase block regions information offset */
#define CFI_REINFO_OFFSET 0x0000002D

/***********************************************************************
 * Private functions
 **********************************************************************/

/***********************************************************************
 *
 * Function: flash_write
 *
 * Purpose: Write a value to FLASH
 *
 * Processing:
 *     See function.
 *
 * Parameters: 
 *     addr: Address to write to
 *     data: Data to write to FLASH
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void flash_write(volatile UNS_32 *addr, UNS_32 data)
{
    *addr = data;
}

/***********************************************************************
 *
 * Function: flash_read
 *
 * Purpose: Read a value from FLASH
 *
 * Processing:
 *     See function.
 *
 * Parameters: 
 *     addr: Address to read
 *
 * Outputs: None
 *
 * Returns: Data read from the address
 *
 * Notes:
 *     A dummy read of another chip select is needed on consecutive
 *     reads to force the FLASH chip select to deassert. Be careful
 *     when calling this function consecutively.
 *
 **********************************************************************/
UNS_32 flash_read(volatile UNS_32 *addr)
{
    return *addr;
}

/***********************************************************************
 *
 * Function: power
 *
 * Purpose: Compute val to the power of pow.
 *
 * Processing:
 *     See function.
 *
 * Parameters: 
 *     val: Base value
 *     pow: Power value for val
 *
 * Outputs: None
 *
 * Returns: The computer power value of val.
 *
 * Notes: None
 *
 **********************************************************************/
UNS_32 power(UNS_32 val,
             UNS_32 pow)
{
    UNS_32 ret = 1;

    while (pow > 0)
    {
        ret = ret * val;
        pow--;
    }

    return ret;
}

/***********************************************************************
 *
 * Function: cfi_config_check
 *
 * Purpose:
 *    Check the configuration of a StrataFlash interface for the
 *    LPD SDK7A400 EVB.
 *
 * Processing:
 *     See function.
 *
 * Parameters: 
 *     addr: Base address of StrataFlash device(s)
 *
 * Outputs: None
 *
 * Returns: 0 if the FLASH device was detected, otherwise (-1)
 *
 * Notes:
 *     This sequence only works for 2x16 devices connected on a 32-bit
 *     bus in x16 mode.
 *
 **********************************************************************/
INT_32 cfi_config_check(volatile UNS_32 *addr)
{
    UNS_32 bbase;
    INT_32 idx, ebidx, bidx, status = -1;

    /* Save base address of device */
    cfi_dg.baseaddr = (UNS_32) addr;

    /* Issue CFI query command to each device */
    flash_write(addr, CFI32_QUERY_COMMAND);

    /* Verify that QRY tags are correct */
    if ((flash_read(addr + CFI32_TAG_OFFSET) == CFI32_TAG_Q) &&
        (flash_read(addr + CFI32_TAG_OFFSET + 1) == CFI32_TAG_R) &&
        (flash_read(addr + CFI32_TAG_OFFSET + 2) == CFI32_TAG_Y))
    {
        /* Valid StrataFlash part */
        /* Get device(s) size in bytes */
        cfi_dg.devsize = (flash_read(addr + CFI32_DEVSIZE_OFFSET) &
            0xFF);
        cfi_dg.devsize = 2 * power(2, cfi_dg.devsize);

        /* Get write buffer size - the FLASH device returns this size
           in bytes, but since we need to program the interface in
           32-bit increments, this number is converted to 32-bit
           words (2 devices * buffer size per dev / 4 bytes per word) */
        cfi_dg.wb_size = ((flash_read(addr + CFI32_WBSIZE_OFFSET) &
            0xFF) + ((flash_read(addr + CFI32_WBSIZE_OFFSET + 1) &
            0xFF) << 8));
        cfi_dg.wb_size = ((2 * power(2, cfi_dg.wb_size)) / 4);

        /* Get number of erase block regions */
        cfi_dg.erblocks = (flash_read(addr + CFI_ERBLK_OFFSET) & 0xFF);

        /* Get regional block information */
        for (idx = 0; idx < cfi_dg.erblocks; idx++)
        {
            /* Get low and high region sizes */
            cfi_dg.blks[idx] = (UNS_16)
                (flash_read(addr + CFI_REINFO_OFFSET + (idx * 4)) &
                0xFF);
            cfi_dg.blks[idx] += (UNS_16)
                ((flash_read(addr + CFI_REINFO_OFFSET + 1 + (idx * 4)) &
                0xFF) << 8);

            /* Get low and high region sizes */
            cfi_dg.bsize[idx] = (UNS_16)
                (flash_read(addr + CFI_REINFO_OFFSET + 2 + (idx * 4)) &
                0xFF);
            cfi_dg.bsize[idx] += (UNS_16)
                ((flash_read(addr + CFI_REINFO_OFFSET + 3 + (idx * 4)) &
                0xFF) << 8);
        }

        /* Compute block count, addresses, and sizes */
        idx = 0;
        cfi_dg.blocks = 0;
        bbase = (UNS_32) addr;
        ebidx = 0;
        while (ebidx < cfi_dg.erblocks)
        {
            /* Compute block info for this erase region */
            bidx = 0;
            while (bidx <= cfi_dg.blks[ebidx])
            {
                /* Save address to this block and size of block */
                cfi_dg.blk_addr[cfi_dg.blocks] = bbase;
                cfi_dg.blk_size[cfi_dg.blocks] =
                    (2 * 0x100 * cfi_dg.bsize[ebidx]);

                /* Increment address to next block w/ 2 devices */
                bbase = bbase + cfi_dg.blk_size[cfi_dg.blocks];

                /* Increment block count */
                cfi_dg.blocks++;

                /* Next block in erase region */
                bidx++;
            }

            /* Next erase region */
            ebidx++;
        }

        status = 0;
    }

    return status;
}

/***********************************************************************
 *
 * Function: cfi_read_status
 *
 * Purpose: Return the device statuses
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     addr: Base address of StrataFlash device(s)
 *
 * Outputs: None
 *
 * Returns: The statuses of the devices
 *
 * Notes: None
 *
 **********************************************************************/
UNS_32 cfi_read_status(volatile UNS_32 *addr)
{
    flash_write(addr, CFI32_READ_STATUS);

    return flash_read(addr);
}

/***********************************************************************
 *
 * Function: cfi_clear_status
 *
 * Purpose: Clear device statuses
 *
 * Processing:
 *     See function.
 *
 * Parameters: 
 *     addr: Base address of StrataFlash device(s)
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void cfi_clear_status(volatile UNS_32 *addr)
{
    flash_write(addr, CFI32_CLEAR_STATUS_COMMAND);
}

/***********************************************************************
 *
 * Function: cfi_wait_read
 *
 * Purpose: Wait for device to become ready
 *
 * Processing:
 *     See function.
 *
 * Parameters: 
 *     addr: Base address of StrataFlash device(s)
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void cfi_wait_read(volatile UNS_32 *addr)
{
    /* Wait until device is ready on high and low devies */
    while ((cfi_read_status(addr) & 0x00000080) == 0);
    while ((cfi_read_status(addr) & 0x00800000) == 0);
}

/***********************************************************************
 * Public functions
 **********************************************************************/

/***********************************************************************
 *
 * Function: cfi_detect
 *
 * Purpose: Detect the number of CFI devices and the configuration
 *
 * Processing:
 *     See function.
 *
 * Parameters: 
 *     addr: Base address of StrataFlash device(s)
 *
 * Outputs: None
 *
 * Returns: 0 if the FLASH device was detected, otherwise (-1)
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 cfi_detect(volatile UNS_32 *addr)
{
    /* Attempt to check device */
    return cfi_config_check(addr);
}

/***********************************************************************
 *
 * Function: cfi_getdevsize
 *
 * Purpose: Return the (combined) device size in bytes
 *
 * Processing:
 *     See function.
 *
 * Parameters: None
 *
 * Outputs: None
 *
 * Returns: The total number of available bytes for all devices in FLASH
 *
 * Notes: None
 *
 **********************************************************************/
UNS_32 cfi_getdevsize(void)
{
    return cfi_dg.devsize;
}

/***********************************************************************
 *
 * Function: cfi_clear_block_lock
 *
 * Purpose: Clear a block lock
 *
 * Processing:
 *     See function.
 *
 * Parameters: 
 *     block: Block to unlock
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void cfi_clear_block_lock(UNS_32 block)
{
    UNS_32 *addr = (UNS_32 *) cfi_get_block_address(block);

    /* Clear block lock */
    flash_write(addr, CFI32_SET_BLOCK_COMMAND);
    flash_write(addr, CFI32_CONFIRM_CLEAR_BLOCK_COMMAND);

    /* Wait until operation is complete */
    cfi_wait_read(addr);
}

/***********************************************************************
 *
 * Function: cfi_set_block_lock
 *
 * Purpose: Set a block lock
 *
 * Processing:
 *     See function.
 *
 * Parameters: 
 *     block: Block to lock
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void cfi_set_block_lock(UNS_32 block)
{
    UNS_32 *addr = (UNS_32 *) cfi_get_block_address(block);
    
    /* Set block lock */
    flash_write(addr, CFI32_SET_BLOCK_COMMAND);
    flash_write(addr, CFI32_CONFIRM_SET_BLOCK_COMMAND);

    /* Wait until operation is complete */
    cfi_wait_read(addr);
}

/***********************************************************************
 *
 * Function: cfi_get_block_count
 *
 * Purpose: Returns the total number of blocks in FLASH
 *
 * Processing:
 *     See function.
 *
 * Parameters: None
 *
 * Outputs: None
 *
 * Returns: Returns the total number of blocks in FLASH
 *
 * Notes: None
 *
 **********************************************************************/
UNS_32 cfi_get_block_count(void)
{
    return cfi_dg.blocks;
}

/***********************************************************************
 *
 * Function: cfi_get_block_address
 *
 * Purpose: Returns the starting address of the block
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     block: Block to return starting address to
 *
 * Outputs: None
 *
 * Returns: Returns the starting address of the block
 *
 * Notes: None
 *
 **********************************************************************/
UNS_32 cfi_get_block_address(UNS_32 block)
{
    return cfi_dg.blk_addr[block];
}

/***********************************************************************
 *
 * Function: cfi_get_block_size
 *
 * Purpose: Returns the size of the block
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     block: Block to return size of
 *
 * Outputs: None
 *
 * Returns: Returns the size of the block
 *
 * Notes: None
 *
 **********************************************************************/
UNS_32 cfi_get_block_size(UNS_32 block)
{
    return cfi_dg.blk_size[block];
}

/***********************************************************************
 *
 * Function: cfi_get_wb_size
 *
 * Purpose: Returns the size of the write buffer
 *
 * Processing:
 *     See function.
 *
 * Parameters: None
 *
 * Outputs: None
 *
 * Returns: Returns the size of the write buffer (in 32-bit words)
 *
 * Notes: None
 *
 **********************************************************************/
UNS_32 cfi_get_wb_size(void)
{
    return cfi_dg.wb_size;
}

/***********************************************************************
 *
 * Function: cfi_get_block_from_address
 *
 * Purpose: Returns the block number for the passed address
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     addr: Address to return block number for
 *
 * Outputs: None
 *
 * Returns:
 *     Returns the block number for the passed address, or 0xFFFFFFFF
 *     if the block was not found for the address
 *
 * Notes: None
 *
 **********************************************************************/
UNS_32 cfi_get_block_from_address(UNS_32 *addr)
{
    UNS_32 found_block = 0xFFFFFFFF, block, saddr;

    /* Search all block address ranges until found */
    block = 0;
    saddr = (UNS_32) addr;
    while ((block < cfi_dg.blocks) && (found_block == 0xFFFFFFFF))
    {
        if ((saddr >= cfi_dg.blk_addr[block]) &&
            (saddr < (cfi_dg.blk_addr[block]) + cfi_dg.blk_size[block]))
        {
            /* Block found */
            found_block = block;
        }

        block++;
    }

    return found_block;
}

/***********************************************************************
 *
 * Function: cfi_erase_block
 *
 * Purpose: Erases a block of FLASH
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     block: Block to erase
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void cfi_erase_block(UNS_32 block)
{
    volatile UNS_32 *addr = (UNS_32 *) cfi_get_block_address(block);
    
    /* Erase lock */
    flash_write(addr, CFI32_BLOCKERASE_COMMAND);
    flash_write(addr, CFI32_CONFIRM_CLEAR_BLOCK_COMMAND);

    /* Wait until operation is complete */
    cfi_wait_read(addr);
}

/***********************************************************************
 *
 * Function: cfi_erase_device
 *
 * Purpose: Erases the entire FLASH device
 *
 * Processing:
 *     See function.
 *
 * Parameters: None
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void cfi_erase_device(void)
{
    UNS_32 block;

    /* Erase all blocks */
    for (block = 0; block <= cfi_dg.blocks; block++)
    {
        cfi_clear_block_lock(block);
        cfi_erase_block(block);
        cfi_set_block_lock(block);
    }
}

/***********************************************************************
 *
 * Function: cfi_write
 *
 * Purpose: Write a single value (32-bits) to the FLASH devices
 *
 * Processing:
 *     See function.
 *
 * Parameters: 
 *     addr: Address to program
 *     val:  32-bit value to program at address
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: The FLASH block must have been previously unlocked
 *
 **********************************************************************/
void cfi_writeword(volatile UNS_32 *addr,
                   UNS_32 val)
{
    flash_write(addr, CFI32_BYTEWRITE_COMMAND);
    flash_write(addr, val);

    /* Wait until device is read */
    cfi_wait_read(addr);
}

/***********************************************************************
 *
 * Function: cfi_exitprog_mode
 *
 * Purpose: Return FLASH devices to normal mode
 *
 * Processing:
 *     See function.
 *
 * Parameters: 
 *     addr: Any address of StrataFlash device
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void cfi_exitprog_mode(volatile UNS_32 *addr)
{
    /* Exit programming mode */
    flash_write(addr, CFI32_WRITE_EXIT_COMMAND);
}

/***********************************************************************
 *
 * Function: cfi_write_to_buffer
 *
 * Purpose: Write data to FLASH (using buffer)
 *
 * Processing:
 *     See function.
 *
 * Parameters: 
 *     src:   Address of source data
 *     dest:  Address of FLASH to write data to
 *     words: Number of 32-bit words to write
 *
 * Outputs: None
 *
 * Returns: The number of actual 32-bit words written
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 cfi_write_to_buffer(UNS_32 *src,
                           volatile UNS_32 *dest,
                           INT_32 words)
{
    UNS_32 count, *bsave;

    /* Get block number from this address */
    count = cfi_get_block_from_address((UNS_32 *) dest);

    /* Get starting block address of this block */
    bsave = (UNS_32 *) cfi_get_block_address(count);

    if (words > 0)
    {
        /* Block write command */
        flash_write(bsave, CFI32_BLOCK_WRITE_COMMAND);

        /* Wait until device is ready */
        cfi_wait_read(bsave);

        /* Write count (in bytes) limited to buffer size */
        if (words > cfi_dg.wb_size)
        {
            words = cfi_dg.wb_size;
        }

        /* Since the write buffer size is expected in bytes and we
           store the value in 32-bit words, convert the write count to
           a byte size first */
        flash_write(bsave, (words | (words << 16)));

        /* Write all data or until the write buffer is full,
           whichever comes first */
        count = 0;
        while ((words > 0) && (count < cfi_dg.wb_size))
        {
            /* Start programming */
            flash_write(dest, src[count]);
            dest++;
            count++;
            words--;
        }

        /* Confirm buffer write */
        flash_write(bsave, CFI32_CONFIRM_BUFWRITE_COMMAND);

        /* Wait until device is ready */
        cfi_wait_read(bsave);
    }

    return count;
}
