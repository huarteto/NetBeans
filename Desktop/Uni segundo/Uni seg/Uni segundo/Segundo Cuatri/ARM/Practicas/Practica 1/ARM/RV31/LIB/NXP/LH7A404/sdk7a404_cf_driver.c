/***********************************************************************
 * $Id:: sdk7a404_cf_driver.c 14 2007-08-28 16:08:18Z kevinw           $
 *
 * Project: SDK7A404 CompactFlash driver
 *
 * Description:
 *     This file contains driver support for the IDE mode CompactFlash
 *     interface on the SDK7A404 EVB.
 *
 * Notes:
 *     This driver provides very basic functions only such as sector
 *     read and write, and data transfer. No error detection is provided
 *     with this driver. A device does not have to be installed in the
 *     EVB to 'open' this driver. BE CAREFUL WRITING TO A CF CARD - YOU
 *     CAN EASILY OVERWRITE IMPORTANT SECTORS AND LOSE INFORMATION ON
 *     THE CARD.
 *
 *     When using the cf_read() and cf_write() operations, the number
 *     of bytes to read or write MUST be at least 512! If it isn't, the
 *     operation will not occur. This is needed because this driver does
 *     not provide buffering to the CF device and data read and write
 *     sector sizes are set at 512 bytes. Note that read and write sizes
 *     should always be a factor of 512, based on the number of sectors
 *     read or written. There is no error checking for this requirement.
 *
 * Important note about accessing CF registers:
 *     The CF interface to the SOC has been configured in such as way
 *     as reads and writes are always 16-bits. Unfortunately, some
 *     registers *must* be written as 8-bit value to the device. This
 *     is not possible in this design - trying to do an 8-bit write to
 *     an 8-bit register will corrupt the other half of the 16-bit
 *     operation. (For example, writing an 8-bit value to address
 *     0x60201000 will corrupt address 0x60201001.) To get around this
 *     minor problem, all write operations must occur as 16-bit
 *     operations, so some CF register values may need to be saved
 *     and written out with another desired register value. When
 *     performing consecutive writes to the CF interface, a read to
 *     another chip select (besides the CF chip select) needs to be
 *     performed in between each write - this is needed to work
 *     around ISA timing issues with the CF interface and the memory
 *     interface timing.
 *
 * Important note 2 about accessing CF:
 *     This driver relies on LOLO setting up GPIO and the CF static emory bank (bank6)
 *     BCR6 = 100003e2 and BCR7 = 10000102 (note RBLE=0 for CF to work due to CPLD)
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

#include "sdk7a404_cf_driver.h"

/***********************************************************************
 * CompactFlash driver private data and types
 **********************************************************************/

/* CF interface (memory mode) access map */
/* Address    Access     R/W     Notes */
/* 0x60200000 16-bits    R only  Card Information Structure (CIS) */
/* 0x60200200 16-bits    R/W     Attribute memory */
/* 0x60201000 16-bits    R/W     Common memory */

/* Offset to CIS */
#define CF_OFFSET_CSI 0x0

/* Offset to attribute memory */
#define CF_OFFSET_ATTB 0x200

/* Offset to common memory */
#define CF_OFFSET_CM 0x1000

/* CF dummy read address used to force a chip select toggle */
/* On the SDK, the CF's nCE and nWE/nRD lines go through the CPLD - which seems to be terminating the bus cycle because */
/* The CF still works even if the CF_DUMMY_RD address is set to the same bank as the CF card (Bank6) */
/* or the LH7A404's scratchpad register (address: 0x80000440). To be safe, use the 0x70000000 dummy address */
#define CF_DUMMY_RD 0x70000000

/* Organization of attribute memory on the CF device */
typedef struct
{
    volatile UNS_16 config_option;
    volatile UNS_16 card_config_sts;
    volatile UNS_16 pin_replacement;
    volatile UNS_16 socket_copy;
} CF_ATTB_T;

/* CompactFlash Memory mapped mode interface register structure for
   the SDK7A404 (for read accesses only) */
typedef struct
{
    volatile UNS_16 data;           /* CF read data */
    volatile UNS_16 sec_count_no;   /* CF sector count and low 8 bits
                                       of number (LBA mode) */
    volatile UNS_16 sec_mid;        /* CF sector middle 16 bits */
    volatile UNS_16 sec_high_sts;   /* Sector high 8 bits and status
                                       value */
    volatile UNS_16 dup_data;       /* Duplicated data */
    volatile UNS_16 rsvd1;
    volatile UNS_16 dup_error;      /* Duplicated error value */
    volatile UNS_16 dup_sts_da;     /* Duplicated status and drive
                                       address  */
} CF_STATUS_READ_REG_T;

/* CompactFlash Memory mapped mode interface register structure for
   the SDK7A404 (for write accesses) */
typedef struct
{
    volatile UNS_16 data;           /* CF write data */
    volatile UNS_16 sec_count_no;   /* CF sector count and low 8 bits
                                       of number (LBA mode) */
    volatile UNS_16 sec_mid;        /* CF sector middle 16 bits */
    volatile UNS_16 sec_high_cmd;   /* Sector high 8 bits and command
                                       value */
    volatile UNS_16 dup_data;       /* Duplicated data */
    volatile UNS_16 rsvd1;
    volatile UNS_16 dup_features;   /* Duplicated features value */
    volatile UNS_16 device_ctl;     /* Device control */
} CF_STATUS_WRITE_REG_T;

/* CompactFlash status register busy bit */
#define CF_BUSY    _BIT(7)
/* CompactFlash status register command ready bit */
#define CF_RDY     _BIT(6)
/* CompactFlash status register Write fault bit */
#define CF_DWF     _BIT(5)
/* CompactFlash status register card ready bit */
#define CF_DSC     _BIT(4)
/* CompactFlash status register data request bit */
#define CF_DRQ     _BIT(3)
/* CompactFlash status register correctable error bit */
#define CF_CORR    _BIT(2)
/* CompactFlash status register reserved bit, alwats 0 */
#define CF_IDX     _BIT(1)
/* CompactFlash status register noncorrectable error bit */
#define CF_ERR     _BIT(0)

/* CompactFlash error register bad block bit */
#define CF_BBK     _BIT(7)
/* CompactFlash error register bauncorrectable error bit */
#define CF_UNC     _BIT(6)
/* CompactFlash error register bad sector bit */
#define CF_IDNF    _BIT(4)
/* CompactFlash error register abort error bit */
#define CF_ABORT   _BIT(2)
/* CompactFlash error register general error bit */
#define CF_AMNF    _BIT(1)

/* CompactFlash head register LBA mode select bit */
#define CF_LBA     _BIT(4)
/* CompactFlash Device control register reset bit */
#define CF_RST     _BIT(2)
/* CompactFlash Device control register Interrupt enable bit */
#define CF_IEN     _BIT(1)

/* CompactFlash card identification command */
#define CFC_IDENT  0xEC
/* CompactFlash Read sector command */
#define CFC_R_SECT 0x20
/* CompactFlash Read multiple sectors command */
#define CFC_R_MULT 0xC4
/* CompactFlash Write sector command */
#define CFC_W_SECT 0x30
/* CompactFlash Erase sector command */
#define CFC_E_SECT 0xC0
/* CompactFlash Set Multiple command */
#define CFC_S_MULT 0xC6

/* CompactFlash LBA mode enable in the sector register */
#define CF_LBA_MODE 0x40

/* Status word check mask used in detection scheme */
#define CF_DETECT_MASK (CF_BUSY | CF_RDY | CF_DSC | CF_IDX)

/* Expected value from masked status in CF detection scheme */
#define CF_DETECT_VALUE (CF_RDY | CF_DSC)

/* CF device sector size */
#define CF_SECT_SIZE 512

/* CompactFlash configuration structure type */
typedef struct
{
    BOOL_32 init;     /* Driver initialized/open flag */
    CF_STATUS_READ_REG_T *rregptr; /* Read register base pointer */
    CF_STATUS_WRITE_REG_T *wregptr; /* Write register base pointer */
} CF_CFG_T;

/* CompactFlash configuration driver data */
STATIC CF_CFG_T cfcfg;

/***********************************************************************
 * CompactFlash driver private functions
 **********************************************************************/

/***********************************************************************
 *
 * Function: cf_set_seccmd
 *
 * Purpose: Issue the command for the next sector operation
 *
 * Processing:
 *     Set the Logical Block address (LBA) sector value to the passed
 *     sector value and the sector count to 1. Issue the passed
 *     command to the CompactFlash device.
 *
 * Parameters:
 *     sector:  Sector (LBA) to perform operation on
 *     command: CompactFlash command to use with sector
 *     wptr:    Pointer to CompactFlash memory mode register set
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void cf_set_seccmd(UNS_32 sector,
                   UNS_8 command,
                   CF_STATUS_WRITE_REG_T *wptr)
{
    volatile UNS_16 dummy;

    wptr->sec_count_no = (0x01 | (UNS_16) ((sector & 0xFF) << 8));
    dummy = * (volatile UNS_16 *) CF_DUMMY_RD;
    wptr->sec_mid = (UNS_16) (((sector & 0xFF0000) >> 8) |
        (sector & 0xFF00) >> 8);
    dummy = * (volatile UNS_16 *) CF_DUMMY_RD;
    wptr->sec_high_cmd = (UNS_16) ((((sector & 0xFF000000) >> 24) |
        CF_LBA_MODE) | (command << 8));
    dummy = * (volatile UNS_16 *) CF_DUMMY_RD;
}

/***********************************************************************
 *
 * Function: cf_start_read
 *
 * Purpose: Start a block read or single sector read operation
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     sectorinfo: Pointer to the sector number and block size
 *     wptr:       Pointer to CompactFlash memory mode register set
 *
 * Outputs: None
 *
 * Returns: Always returns _NO_ERROR.
 *
 * Notes: None
 *
 **********************************************************************/
STATUS cf_start_read(CF_BLOCKS_T *sectorinfo,
                     CF_STATUS_WRITE_REG_T *wptr)
{
    volatile UNS_16 dummy;

    /* Issue command to read a single sector */
    if (sectorinfo->num_blocks == 1)
    {
        cf_set_seccmd((UNS_32) sectorinfo->sector, CFC_R_SECT, wptr);
    }
    else
    {
        wptr->sec_count_no = (sectorinfo->num_blocks & 0xFF);
        dummy = * (volatile UNS_16 *) CF_DUMMY_RD;
        wptr->sec_count_no = (sectorinfo->num_blocks |
            (UNS_16) ((sectorinfo->sector & 0xFF) << 8));
        dummy = * (volatile UNS_16 *) CF_DUMMY_RD;
        wptr->sec_mid = (UNS_16)
            (((sectorinfo->sector & 0xFF0000) >> 8) |
            (sectorinfo->sector & 0xFF00) >> 8);
        dummy = * (volatile UNS_16 *) CF_DUMMY_RD;
        wptr->sec_high_cmd = (UNS_16)
            ((((sectorinfo->sector & 0xFF000000) >> 24) | CF_LBA_MODE) |
            (CFC_R_SECT << 8));
        dummy = * (volatile UNS_16 *) CF_DUMMY_RD;
    }

    return _NO_ERROR;
}

/***********************************************************************
 *
 * Function: cf_start_write
 *
 * Purpose: Start a single sector write operation
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     sectorinfo: Pointer to the sector number and block size
 *     wptr:       Pointer to CompactFlash memory mode register set
 *
 * Outputs: None
 *
 * Returns: Always returns _NO_ERROR.
 *
 * Notes: None
 *
 **********************************************************************/
STATUS cf_start_write(CF_BLOCKS_T *sectorinfo,
                      CF_STATUS_WRITE_REG_T *wptr)
{
    /* Issue command to write a single sector */
    cf_set_seccmd((UNS_32) sectorinfo->sector, CFC_W_SECT, wptr);

    return _NO_ERROR;
}

/***********************************************************************
 * CompactFlash driver public functions
 **********************************************************************/

/***********************************************************************
 *
 * Function: cf_open
 *
 * Purpose: Open the CompactFlash interface
 *
 * Processing:
 *     If init is not FALSE and the passed ipbase does not point to the
 *     CompactFlash register base address, return NULL to the caller.
 *     Otherwise, set init to TRUE and save the CF device register set
 *     addresses. Return a pointer to the CF config structure to the
 *     caller.
 *
 * Parameters:
 *     ipbase: CompactFlash device address
 *     arg   : Not used
 *
 * Outputs: None
 *
 * Returns: The pointer to a CompactFlash config structure or NULL
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 cf_open(void *ipbase,
               INT_32 arg)
{
    INT_32 status = 0;

    if ((cfcfg.init == FALSE) && ((INT_32) ipbase == CF_BASE))
    {
        /* Device is valid and not previously initialized */
        cfcfg.init = TRUE;

        /* Save address of registers and interface */
        cfcfg.rregptr =
            (CF_STATUS_READ_REG_T *) ((INT_32) ipbase + CF_OFFSET_CM);
        cfcfg.wregptr =
            (CF_STATUS_WRITE_REG_T *) ((INT_32) ipbase + CF_OFFSET_CM);

        /* Return pointer to CF configuration structure */
        status = (INT_32) &cfcfg;
    }

    return status;
}

/***********************************************************************
 *
 * Function: cf_close
 *
 * Purpose: Close the CompactFlash interface
 *
 * Processing:
 *     If init is not TRUE, then return _ERROR to the caller as the
 *     device was not previously opened. Otherwise, set init to FALSE,
 *     and return _NO_ERROR to the caller.
 *
 * Parameters:
 *     devid: Pointer to CF config structure
 *
 * Outputs: None
 *
 * Returns: The status of the close operation
 *
 * Notes: None
 *
 **********************************************************************/
STATUS cf_close(INT_32 devid)
{
    CF_CFG_T *cfcfgptr = (CF_CFG_T *) devid;
    STATUS status = _ERROR;

    if (cfcfgptr->init == TRUE)
    {
        status = _NO_ERROR;
        cfcfgptr->init = FALSE;
    }

    return status;
}

/***********************************************************************
 *
 * Function: cf_ioctl
 *
 * Purpose: CF configuration block
 *
 * Processing:
 *     This function is a large case block. Based on the passed function
 *     and option values, set or get the appropriate CF parameter.
 *
 * Parameters:
 *     devid: Pointer to CF config structure
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
STATUS cf_ioctl(INT_32 devid,
                INT_32 cmd,
                INT_32 arg)
{
    UNS_32 tmp;
    CF_STATUS_READ_REG_T *rptr;
    CF_STATUS_WRITE_REG_T *wptr;
    CF_CFG_T *cfcfgptr = (CF_CFG_T *) devid;
    STATUS status = _ERROR;
    volatile UNS_16 dummy;

    if (cfcfgptr->init == TRUE)
    {
        status = _NO_ERROR;
        rptr = cfcfgptr->rregptr;
        wptr = cfcfgptr->wregptr;

        switch (cmd)
        {
            case CF_READ_BLOCKS:
                /* Issue command to read blocks */
                status = cf_start_read((CF_BLOCKS_T *) arg, wptr);
                break;

            case CF_WRITE_BLOCKS:
                /* Issue command to write blocks */
                status = cf_start_write((CF_BLOCKS_T *) arg, wptr);
                break;

            case CF_ERASE_SECTOR:
                /* Erase a single sector */
                cf_set_seccmd((UNS_32) arg, CFC_E_SECT, wptr);
                break;

            case CF_GET_STATUS:
                /* Return a CF interface status */
                switch (arg)
                {
                    case CF_CARD_DETECT:
                        /* Returns 1 if a CF card is inserted in the
                           EVB, otherwise 0 */
                        /* Read the CF status and mask off some bits */
                        tmp = (UNS_32) rptr->dup_sts_da & CF_DETECT_MASK;
                        dummy = * (volatile UNS_16 *) CF_DUMMY_RD;
                        if (tmp == CF_DETECT_VALUE)
                        {
                            /* Card has been detected */
                            status = 1;
                        }
                        else
                        {
                            /* No card detected */
                            status = 0;
                        }
                        break;

                    case CF_CARD_READY:
                        /* Returns 1 if the card is ready for a new
                           command, otherwise 0 */
                        if ((rptr->dup_sts_da & CF_RDY) != 0)
                        {
                            status = 1;
                        }
                        else
                        {
                            status = 0;
                        }
                        dummy = * (volatile UNS_16 *) CF_DUMMY_RD;
                        break;

                    case CF_CARD_BUSY:
                        /* Returns 1 if the card is busy, otherwise 0 */
                        if ((rptr->dup_sts_da & CF_BUSY) != 0)
                        {
                            status = 1;
                        }
                        else
                        {
                            status = 0;
                        }
                        dummy = * (volatile UNS_16 *) CF_DUMMY_RD;
                        break;

                    default:
                        /* Unsupported parameter */
                        status = SMA_BAD_PARAMS;
                        break;
                }
                break;

            default:
                /* Unsupported parameter */
                status = SMA_BAD_PARAMS;
        }
    }

    return status;
}

/***********************************************************************
 *
 * Function: cf_read
 *
 * Purpose: CompactFlash read function
 *
 * Processing:
 *     If the init flag for the CF structure is FALSE or the number of
 *     bytes to read is not 512, return 0 to the caller. Otherwise, loop
 *     until max_bytes equals 0. Inside the loop, read the data from
 *     the CF card into the user buffer, increment the user buffer
 *     pointer, decrement max_bytes by two, and increment bytes. When
 *     max_bytes expires to 0, return bytes to the caller.
 *
 * Parameters:
 *     devid:     Pointer to CF config structure
 *     buffer:    Pointer to data buffer to copy to
 *     max_bytes: Number of bytes to read
 *
 * Outputs: None
 *
 * Returns: Number of bytes actually read, 0 on error
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 cf_read(INT_32 devid,
               void *buffer,
               INT_32 max_bytes)
{
    CF_STATUS_READ_REG_T *rptr;
    CF_CFG_T *cfcfgptr = (CF_CFG_T *) devid;
    UNS_8 *data = (UNS_8 *) buffer;
    UNS_16 tmpdata;
    INT_32 secbytes, bytes = 0;
    volatile UNS_16 dummy;

    if ((cfcfgptr->init == TRUE) && (max_bytes >= CF_SECT_SIZE))
    {
        /* Limit read size to size of sector */
        rptr = cfcfgptr->rregptr;

        /* If an error occured, then quit */
        if ((rptr->dup_sts_da & CF_ERR) != 0)
        {
            max_bytes = 0;
        }

        /* Loop until n_bytes expires to 0 */
        secbytes = 0;
        while (max_bytes > 0)
        {
            tmpdata = rptr->data;
            dummy = * (volatile UNS_16 *) CF_DUMMY_RD;
            *data = (UNS_8) (tmpdata & 0xFF);
            data++;
            *data = (UNS_8) (tmpdata >> 8);
            data++;
            max_bytes = max_bytes - 2;
            bytes = bytes + 2;
            secbytes = secbytes + 2;
            if ((secbytes >= CF_SECT_SIZE) && (max_bytes > 0))
            {
                /* Small delay to allow next sector to load */
//                for (secbytes = 0; secbytes < 100; secbytes++);
                secbytes = 0;
                while (cf_ioctl(devid, CF_GET_STATUS,
                        CF_CARD_BUSY) == 1);
            }
        }
    }

    return bytes;
}

/***********************************************************************
 *
 * Function: cf_write
 *
 * Purpose: CompactFlash write function
 *
 * Processing:
 *     If the init flag for the CF structure is FALSE or the number of
 *     bytes to write is not 512, return 0 to the caller. Otherwise,
 *     loop until n_bytes equals 0. Inside the loop, write the data from
 *     the user buffer into the CF card, increment the user buffer
 *     pointer, decrementn_bytes by two, and increment bytes. When
 *     n_bytes expires to 0, return bytes to the caller.
 *
 * Parameters:
 *     devid:   Pointer to CF config structure
 *     buffer:  Pointer to data buffer to copy from
 *     n_bytes: Number of bytes to write
 *
 * Outputs: None
 *
 * Returns: Number of bytes actually written
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 cf_write(INT_32 devid,
                void *buffer,
                INT_32 n_bytes)
{
    CF_STATUS_WRITE_REG_T *wptr;
    CF_CFG_T *cfcfgptr = (CF_CFG_T *) devid;
    UNS_8 *data = (UNS_8 *) buffer;
    UNS_16 tmpdata;
    volatile UNS_16 dummy;
    INT_32 bytes = 0;

    if ((cfcfgptr->init == TRUE) && (n_bytes >= CF_SECT_SIZE))
    {
        wptr = cfcfgptr->wregptr;

        /* Loop until n_bytes expires to 0 */
        while (n_bytes > 0)
        {
            tmpdata = ((UNS_16) data[bytes] |
                (UNS_16) (data[bytes + 1] << 8));
            wptr->data = tmpdata;
            dummy = * (volatile UNS_16 *) CF_DUMMY_RD;
            n_bytes = n_bytes - 2;
            bytes = bytes + 2;
        }
    }

    return bytes;
}
