/***********************************************************************
 * $Id:: LH7A404_clcdc_driver.c 14 2007-08-28 16:08:18Z kevinw         $
 *
 * Project: LH7A404 CLCDC driver
 *
 * Description:
 *     This file contains driver support for the CLCDC module on the
 *     LH7A404
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

#include "lh7a404_clcdc_driver.h"
#include "lh7a404_csc_driver.h"

/***********************************************************************
 * LCD driver private data and types
 **********************************************************************/

/* LCD device configuration structure type */
typedef struct
{
    BOOL_32 init;          /* Device initialized flag */
    CLCDC_REGS_T *regptr;  /* Pointer to LCD registers */
    LCDICP_REGS_T *hrptr;  /* Pointer to LCD HRTFT registers */
    LCD_PARAM_T *dptr;     /* Pointer to LCD controller settings */
} CLCDC_CFG_T;

/* LCD device configuration structure */
STATIC CLCDC_CFG_T lcdcfg;

/* Array that converts an LCD_INTERRUPT_T to a VCOMP interrupt
   selection bitfield for the CLCDC control word */
STATIC const UNS_32 lcd_vcmp_sel[FRONT_PORCH_START + 1] =
{
    CLCDC_LCDCTRL_VCOMP_VS,
    CLCDC_LCDCTRL_VCOMP_BP,
    CLCDC_LCDCTRL_VCOMP_AV,
    CLCDC_LCDCTRL_VCOMP_FP
};

/***********************************************************************
 * CLCDC driver private functions
 **********************************************************************/

/***********************************************************************
 *
 * Function: lcd_update_clock
 *
 * Purpose: Update the pixel clock
 *
 * Processing:
 *     The closest pixel divider to generate the desired clock frequency
 *     from the AHB clock frequency is computed with a call to
 *     csc_compute_divider. The value of the LCD timing 2 register is
 *     then loaded. If the divider is '1', the 'BCD' divider bit in the
 *     loaded timing 2 word is or'ed into the word. If the divider is 2
 *     or greater, the previous BCD bit and divider value are masked out
 *     of the loaded word and the new divider added into the word. The
 *     changed word is then put back into the LCD timing 2 register.
 *
 * Parameters:
 *     desired_clock : New desired pixel clock frequency in Hz
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
STATIC void lcd_update_clock(CLCDC_CFG_T *lcdcfgptr,
                             UNS_32 desired_clock)
{
    INT_32 pixel_div;
    UNS_32 tmp;
    CLCDC_REGS_T *lcdptr = lcdcfgptr->regptr;

    /* Check which clock is being used */
    if ((lcdptr->lcdtiming2 & CLCDC_LCDTIMING2_CLKSEL) != 0)
    {
        /* Main oscillator is used as clock base */
        pixel_div = csc_compute_divider(CLOCK_MAINOSC, desired_clock);
    }
    else
    {
        pixel_div = csc_compute_divider(csc_get_clock(AHBCLK),
            desired_clock);
    }

    tmp = lcdptr->lcdtiming2;
    if (pixel_div <= 1)
    {
        /* Pixel clock divider is 1, skip divider logic */
        tmp |= CLCDC_LCDTIMING2_BCD;
    }
    else
    {
        /* Mask off old divider and bypass bits */
        tmp &= ~(CLCDC_LCDTIMING2_BCD | CLCDC_LCDTIMING2_PCD(0x1F));

        /* Use divider logic */
        tmp |= CLCDC_LCDTIMING2_PCD(pixel_div);
    }

    lcdptr->lcdtiming2 = tmp;
}
/***********************************************************************
 *
 * Function: lcd_initialize
 *
 * Purpose: Initialize the LCD controller
 *
 * Processing:
 *     Prior to resetting LCD values, disable the LCD controller. Based
 *     on the values in the passed LCD_PARAM_T structure, generate
 *     the appropriate LCD configuration words and place them in the
 *     correct LCD registers.
 *
 * Parameters:
 *     lcdcfgptr : A pointer to an LCD configuration data structure
 *
 * Outputs: None
 *
 * Returns: TRUE if the LCD was initialized, otherwise FALSE
 *
 * Notes: None
 *
 **********************************************************************/
STATIC BOOL_32 lcd_initialize(CLCDC_CFG_T *lcdcfgptr)
{
    UNS_32 tmp;
    STATUS status = _NO_ERROR;
    CLCDC_REGS_T *lcdptr = lcdcfgptr->regptr;
    LCDICP_REGS_T *icpptr = lcdcfgptr->hrptr;
    LCD_PARAM_T *cgdatptr = lcdcfgptr->dptr;

    /* Disable the display in case it is on */
    lcdptr->lcdctrl &= ~CLCDC_LCDCTRL_ENABLE;

    /* Generate the timing 0 word */
    tmp = (CLCDC_LCDTIMING0_PPL(cgdatptr->pixels_per_line) |
        CLCDC_LCDTIMING0_HSW(cgdatptr->h_sync_pulse_width) |
        CLCDC_LCDTIMING0_HFP(cgdatptr->h_front_porch) |
        CLCDC_LCDTIMING0_HBP(cgdatptr->h_back_porch));
    lcdptr->lcdtiming0 = tmp;

    /* Generate the timing 1 word */
    if (cgdatptr->dual_panel == 1)
    {
        tmp = cgdatptr->lines_per_panel / 2;
    }
    else
    {
        tmp = cgdatptr->lines_per_panel;
    }

    tmp = (CLCDC_LCDTIMING1_LPP(tmp) |
        CLCDC_LCDTIMING1_VSW(cgdatptr->v_sync_pulse_width) |
        CLCDC_LCDTIMING1_VFP(cgdatptr->v_front_porch) |
        CLCDC_LCDTIMING1_VBP(cgdatptr->v_back_porch));
    lcdptr->lcdtiming1 = tmp;

    /* Generate the timing 2 word */
    tmp = CLCDC_LCDTIMING2_ACB(cgdatptr->ac_bias_frequency);
    if (cgdatptr->invert_output_enable != 0)
    {
        tmp |= CLCDC_LCDTIMING2_IOE;
    }
    if (cgdatptr->invert_panel_clock != 0)
    {
        tmp |= CLCDC_LCDTIMING2_IPC;
    }
    if (cgdatptr->invert_hsync != 0)
    {
        tmp |= CLCDC_LCDTIMING2_IHS;
    }
    if (cgdatptr->invert_vsync != 0)
    {
        tmp |= CLCDC_LCDTIMING2_IVS;
    }

    /* Compute clocks per line based on panel type */
    switch (cgdatptr->lcd_panel_type)
    {
        case MONO_4BIT:
            /* Clocks per line is a quarter of pixels per line */
            tmp = tmp | CLCDC_LCDTIMING2_CPL(
                (cgdatptr->pixels_per_line / 4) - 1);
            break;

        case MONO_8BIT:
            /* Clocks per line is an eighth of pixels per line */
            tmp = tmp | CLCDC_LCDTIMING2_CPL(
                (cgdatptr->pixels_per_line / 8) - 1);
            break;

        case CSTN:
            /* CSTN Clocks per line (* 3 / 8) */
            tmp = tmp | CLCDC_LCDTIMING2_CPL((
                (cgdatptr->pixels_per_line * 3) / 8) - 1);
            break;
            
        case TFT:
        case ADTFT:
        case HRTFT:
        default:
            /* Clocks per line and pixels per line are the same */
            tmp = tmp |
                CLCDC_LCDTIMING2_CPL(cgdatptr->pixels_per_line - 1);
            break;
    }

    lcdptr->lcdtiming2 = tmp;

    /* Update timing 2 word with correct clock data */
    lcd_update_clock(lcdcfgptr, cgdatptr->optimal_clock);

    /* Skip timing 3 word - just set to 0x0 */
    lcdptr->lcdtiming3 = 0x00000000;

    /* Default with all interrupts of */
    lcdptr->lcdintrenable = 0x00000000; 

    /* Default configuration is 16 bits per pixel with blue and
       green not swapped */
    tmp = (CLCDC_LCDCTRL_BPP16 | CLCDC_LCDCTRL_RGB);

    switch (cgdatptr->lcd_panel_type)
    {
        case ADTFT:
        case HRTFT:
        case TFT:
            tmp |= CLCDC_LCDCTRL_TFT;
            break;

        case MONO_4BIT:
            tmp |= CLCDC_LCDCTRL_BW_MONO;
            break;

        case MONO_8BIT:
            tmp |= (CLCDC_LCDCTRL_MONO8 | CLCDC_LCDCTRL_BW_MONO);
            break;

        case CSTN:
            ;
            break;

        default:
            /* Unsupported panel type */
            status = _ERROR;
            break;
    }

    /* Dual panel operation */
    if (cgdatptr->dual_panel == 1)
    {
        tmp |= CLCDC_LCDCTRL_DUAL;
    }    
    lcdptr->lcdctrl = tmp;

    /* Additional ADTFT and HRTFT setup */
    if ((cgdatptr->lcd_panel_type == HRTFT) ||
        (cgdatptr->lcd_panel_type == ADTFT))
    {
        /* Setup HR-TFT controller 'setup' register */
        tmp = LCDICP_SETUP_PPL(cgdatptr->pixels_per_line) |
            LCDICP_SETUP_MODE_HRTFT | LCDICP_SETUP_RESERVED |
            LCDICP_SETUP_EN;
        icpptr->lcdicp_setup = tmp;

        /* Setup HR-TFT controller 'control' register */
        tmp = 0x00000000;
        if (cgdatptr->hrtft_cls_enable != 0)
        {
            tmp |= LCDICP_CONTROL_CLSEN;
        }
        if (cgdatptr->hrtft_sps_enable != 0)
        {
            tmp |= LCDICP_CONTROL_SPSEN;
        }
        icpptr->lcdicp_control = tmp;

        /* Setup HR-TFT timing 1 register */
        tmp = (LCDICP_TIMING1_LPDEL(cgdatptr->hrtft_lp_delay) |
            LCDICP_TIMING1_REVDEL(cgdatptr->hrtft_polarity_delay) |
            LCDICP_TIMING1_PS_CLSDEL(cgdatptr->hrtft_lp_to_ps_delay));
        icpptr->lcdicp_timing1 = tmp;

        /* Setup HR-TFT timing 2 register */
        tmp =
            (LCDICP_TIMING2_PS_CLSDEL2(
            cgdatptr->hrtft_spl_to_cls_delay) |
            LCDICP_TIMING2_SPLVALUE(cgdatptr->hrtft_spl_delay));
        icpptr->lcdicp_timing2 = tmp;
    }
    else
    {
        /* Normal display type, set HRTFT bypass mode */
        tmp = LCDICP_SETUP_MODE_BYPASS;
        icpptr->lcdicp_setup = tmp;
    }

    return status;
}

/***********************************************************************
 * CLCDC driver public functions
 **********************************************************************/

/***********************************************************************
 *
 * Function: lcd_open
 *
 * Purpose: Open the LCD
 *
 * Processing:
 *     If init is not FALSE and the passed ipbase does not point to the
 *     CLCDC register base address, return NULL to the caller.
 *     Otherwise, set init to TRUE, save the LCD peripheral register set
 *     addresses for the LCD controller and HRTFT timing block. Disable
 *     the LCD controller. If arg is not 0, then call the lcd_initialize
 *     function with the address of the LCD configuration structure as
 *     its argument. Return a pointer to the LCD config structure to the
 *     caller.
 *
 * Parameters:
 *     ipbase: LCD descriptor device address
 *     arg   : Not used
 *
 * Outputs: None
 *
 * Returns: The pointer to a LCD config structure or NULL
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 lcd_open(void *ipbase, INT_32 arg)
{
    INT_32 status = 0;

    if ((lcdcfg.init == FALSE) && ((CLCDC_REGS_T *) ipbase == CLCDC))
    {
        /* Device is valid and not previously initialized */
        lcdcfg.init = TRUE;

        /* Save and return address of peripheral block */
        lcdcfg.regptr = (CLCDC_REGS_T *) ipbase;
        lcdcfg.hrptr = (LCDICP_REGS_T *) LDCICP;
        lcdcfg.dptr = (LCD_PARAM_T *) arg;

        /* Disable LCD */
        lcdcfg.regptr->lcdctrl &= ~CLCDC_LCDCTRL_ENABLE;

        /* If the passed argument was not NULL, then it was a pointer
           to a LCD device configuration structure that needs to be
           configured */
        if (arg != 0)
        {
            lcd_initialize(&lcdcfg);
        }

        /* Return pointer to LCD configuration structure */
        status = (INT_32) &lcdcfg;
    }

    return status;
}

/***********************************************************************
 *
 * Function: lcd_close
 *
 * Purpose: Close the LCD
 *
 * Processing:
 *     If init is not TRUE, then return _ERROR to the caller as the
 *     device was not previously opened. Otherwise, set init to FALSE,
 *     disable LCD controller, and return _NO_ERROR to the caller.
 *
 * Parameters:
 *     devid: Pointer to LCD config structure
 *
 * Outputs: None
 *
 * Returns: The status of the close operation
 *
 * Notes: None
 *
 **********************************************************************/
STATUS lcd_close(INT_32 devid)
{
    CLCDC_CFG_T *lcdcfgptr = (CLCDC_CFG_T *) devid;
    STATUS status = _ERROR;

    if (lcdcfgptr->init == TRUE)
    {
        /* 'Uninitialize' device */
        lcdcfgptr->init = FALSE;
        status = _NO_ERROR;

       /* Disable LCD */
        lcdcfg.regptr->lcdctrl &= ~CLCDC_LCDCTRL_ENABLE;
    }

    return status;
}

/***********************************************************************
 *
 * Function: lcd_ioctl
 *
 * Purpose: LCD configuration block
 *
 * Processing:
 *     This function is a large case block. Based on the passed function
 *     and option values, set or get the appropriate real time clock
 *     parameter.
 *
 * Parameters:
 *     devid: Pointer to LCD config structure
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
STATUS lcd_ioctl(INT_32 devid,
                 INT_32 cmd,
                 INT_32 arg)
{
    CLCDC_REGS_T *lcdregs;
    UNS_32 tmp, tmp1;
    CLCDC_CFG_T *lcdcfgptr = (CLCDC_CFG_T *) devid;
    STATUS status = _ERROR;

    if (lcdcfgptr->init == TRUE)
    {
        status = _NO_ERROR;
        lcdregs = lcdcfgptr->regptr;

        switch (cmd)
        {
            case LCD_CONFIG:
                /* Configure the LCD controller with a predefined LCD
                   configuration structure */
                if (arg != 0)
                {
                    lcdcfgptr->dptr = (LCD_PARAM_T *) arg;
                    lcd_initialize(lcdcfgptr);
                }
                else
                {
                    /* Not a valid address of structure */
                    status = _ERROR;
                }
                break;

            case LCD_ENABLE:
                /* Enable or disable LCD controller */
                if (arg == 1)
                {
                    /* Enable */
                    lcdregs->lcdctrl |= CLCDC_LCDCTRL_ENABLE;
                }
                else
                {
                    /* Disable */
                    lcdregs->lcdctrl *= ~CLCDC_LCDCTRL_ENABLE;
                }
                break;

            case LCD_PWENABLE:
                /* Enable or disable LCD power */
                if (arg == 1)
                {
                    /* Enable */
                    lcdregs->lcdctrl |= (CLCDC_LCDCTRL_ENABLE |
                        CLCDC_LCDCTRL_PWR);
                }
                else
                {
                    /* Disable */
                    lcdregs->lcdctrl &= ~(CLCDC_LCDCTRL_ENABLE |
                        CLCDC_LCDCTRL_PWR);
                }
                break;

            case LCD_SET_UP_FB:
                /* Set upper frame buffer address */
                lcdregs->lcdupbase = (UNS_32) arg;
                break;

            case LCD_SET_LW_FB:
                /* Set lower frame buffer address */
                lcdregs->lcdlpbase = (UNS_32) arg;
                break;

            case LCD_SET_OV_FB:
                /* Set overflow frame buffer address */
                lcdregs->lcdlpoverflow = (UNS_32) arg;
                break;

            case LCD_ENABLE_INTS:
                /* Enable LCD interrupts */
                tmp = lcdregs->lcdintrenable;
                tmp = tmp | ((UNS_32) arg & (CLCDC_LCDSTATUS_FUFUP |
                    CLCDC_LCDSTATUS_FUFLP |CLCDC_LCDSTATUS_LNBU |
                    CLCDC_LCDSTATUS_VCOMP | CLCDC_LCDSTATUS_MBERROR));
                lcdregs->lcdintrenable = tmp;
                break;

            case LCD_DISABLE_INTS:
                /* Disable LCD interrupts */
                tmp = lcdregs->lcdintrenable;
                tmp = tmp & ~((UNS_32) arg & (CLCDC_LCDSTATUS_FUFUP |
                    CLCDC_LCDSTATUS_FUFLP |CLCDC_LCDSTATUS_LNBU |
                    CLCDC_LCDSTATUS_VCOMP | CLCDC_LCDSTATUS_MBERROR));
                lcdregs->lcdintrenable = tmp;
                break;

            case LCD_PICK_INT:
                /* Select the interrupt condition that generates the
                   CLCDC_LCDSTATUS_VCOMP interrupt */
                if ((arg >= VSYNC_START) && (arg <= FRONT_PORCH_START))
                {
                    /* Temporarily disable CLCDC_LCDSTATUS_VCOMP
                       interrupt */
                    tmp = lcdregs->lcdintrenable;
                    lcdregs->lcdintrenable &= ~CLCDC_LCDSTATUS_VCOMP;

                    /* Mask off VCOMP selection bits in control word */
                    tmp1 = lcdregs->lcdctrl & ~CLCDC_LCDCTRL_VCOMP_IC;

                    /* 'OR' in selected interrupt */
                    tmp1 |= lcd_vcmp_sel[arg];
                    lcdregs->lcdctrl = tmp1;

                    /* Restore the LCD CLCDC_LCDSTATUS_VCOMP
                       interrupt */
                    lcdregs->lcdintrenable = tmp;
                }
                else
                {
                    /* Invalid argument */
                    status = _ERROR;
                }
                break;

            case LCD_CLEAR_INT:
                /* Clear the latched LCD interrupt condition */
                lcdregs->lcdstatus = 0xFF;
                break;

            case LCD_DMA_ON_4MT:
                /* Set DMA requests to start when 4 or 8 entries are
                   free in the CLCDC FIFOs */
                if (arg == 0)
                {
                    /* Use 8 entries */
                    lcdregs->lcdctrl &= ~CLCDC_LCDCTRL_WATERMARK;
                }
                else
                {
                    /* Use 4 entries */
                    lcdregs->lcdctrl |= CLCDC_LCDCTRL_WATERMARK;
                }
                break;

            case LCD_SWAP_RGB:
                /* Swaps the red and green colors in a 16-bit color
                   value */
                if (arg == 1)
                {
                    /* Swap RGB colors (555 mode only) */
                    lcdregs->lcdctrl &= ~CLCDC_LCDCTRL_RGB;
                }
                else
                {
                    /* Do not swap RGB colors (555 mode only) */
                    lcdregs->lcdctrl |= CLCDC_LCDCTRL_RGB;
                }
                break;

            case LCD_SET_BPP:
                /* Set bits per pixel */
                tmp = lcdregs->lcdctrl & ~CLCDC_LCDCTRL_BPP_MASK;
                switch (arg)
                {
                    case 1:
                        tmp |= CLCDC_LCDCTRL_BPP1;
                        break;

                    case 2:
                        tmp |= CLCDC_LCDCTRL_BPP2;
                        break;

                    case 4:
                        tmp |= CLCDC_LCDCTRL_BPP4;
                        break;

                    case 8:
                        tmp |= CLCDC_LCDCTRL_BPP8;
                        break;

                    case 16:
                    default:
                        tmp |= CLCDC_LCDCTRL_BPP16;
                        break;
                }
                lcdregs->lcdctrl = tmp;
                break;

            case LCD_SET_CLOCK:
                /* Sets or updates the LCD pixel clock */
                if (arg > 0)
                {
                    lcd_update_clock(lcdcfgptr, (UNS_32) arg);
                }
                break;

            case LCD_GET_STATUS:
                /* Return an LCD status */
                switch (arg)
                {
                    case LCD_ENABLE_ST:
                        /* Return LCD enabled status */
                        if ((lcdregs->lcdctrl &
                            CLCDC_LCDCTRL_ENABLE) != 0)
                        {
                            status = 1;
                        }
                        else
                        {
                            status = 0;
                        }
                        break;

                    case LCD_PWENABLE_ST:
                        /* Return LCD power enabled status */
                        if ((lcdregs->lcdctrl & CLCDC_LCDCTRL_PWR) != 0)
                        {
                            status = 1;
                        }
                        else
                        {
                            status = 0;
                        }
                        break;

                    case LCD_UP_FB:
                        /* Return upper frame buffer address */
                        status = (INT_32) lcdregs->lcdupbase;
                        break;

                    case LCD_LW_FB:
                        /* Return lower frame buffer address */
                        status = (INT_32) lcdregs->lcdlpbase;
                        break;

                    case LCD_OVR_FB:
                        /* Return overflow frame buffer address */
                        status = (INT_32) lcdregs->lcdlpoverflow;
                        break;

                    case LCD_PANEL_TYPE:
                        /* Return the LCD panel type */
                        status =
                            (INT_32) lcdcfgptr->dptr->lcd_panel_type;
                        break;

                    case LCD_CLOCK:
                        /* Returns the pixel clock in Hz */
                        /* This function is not yet implemented */
                        break;

                    case LCD_XSIZE:
                        /* Returns the display horizontal size in
                            pixels */
                        status =
                            (INT_32) lcdcfgptr->dptr->pixels_per_line;
                        break;

                    case LCD_YSIZE:
                        /* Returns the display vertical size in
                            pixels */
                        status =
                            (INT_32) lcdcfgptr->dptr->lines_per_panel;
                        break;

                    case LCD_COLOR_DEPTH:
                        /* Returns the display color depth */
                        switch (lcdregs->lcdctrl &
                            CLCDC_LCDCTRL_BPP_MASK)
                        {
                            case CLCDC_LCDCTRL_BPP1:
                                status = 1;
                                break;

                            case CLCDC_LCDCTRL_BPP2:
                                status = 2;
                                break;

                            case CLCDC_LCDCTRL_BPP4:
                                status = 4;
                                break;

                            case CLCDC_LCDCTRL_BPP8:
                                status = 8;
                                break;

                            case CLCDC_LCDCTRL_BPP16:
                                status = 16;
                                break;

                            default:
                                status = SMA_BAD_PARAMS;
                        }
                        break;

                    default:
                        /* Unsupported parameter */
                        status = SMA_BAD_PARAMS;
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
 * Function: lcd_read
 *
 * Purpose: LCD read function (stub only)
 *
 * Processing:
 *     Return 0 to the caller.
 *
 * Parameters:
 *     devid:     Pointer to LCD config structure
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
INT_32 lcd_read(INT_32 devid,
                void *buffer,
                INT_32 max_bytes)
{
    return 0;
}

/***********************************************************************
 *
 * Function: lcd_write
 *
 * Purpose: LCD write function (stub only)
 *
 * Processing:
 *     Return 0 to the caller.
 *
 * Parameters:
 *     devid:   Pointer to LCD config structure
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
INT_32 lcd_write(INT_32 devid,
                 void *buffer,
                 INT_32 n_bytes)
{
    return 0;
}

