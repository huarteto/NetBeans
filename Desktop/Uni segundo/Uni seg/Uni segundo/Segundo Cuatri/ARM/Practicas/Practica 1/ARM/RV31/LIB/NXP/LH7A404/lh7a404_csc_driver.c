/***********************************************************************
 * $Id:: lh7a404_csc_driver.c 14 2007-08-28 16:08:18Z kevinw           $
 *
 * Project: LH7A404 CSC driver
 *
 * Description:
 *     This file contains driver support for the CSC module on the
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

#include "lh7a404_csc_driver.h"
#include "lpc_arm922t_arch.h"

#ifdef __ICCARM__ 
#include "inarm.h"
#endif

/***********************************************************************
 * CSC driver local data
 **********************************************************************/

/* Maps field bit locations in CSC PWRSR status to csc_status_types */
const INT_32 pwrsr_bit_index[CHIPMAN + 1] = {0, 6, 7, 8, 9, 10, 11,
                                             12, 13, 14, 16, 24};
/* Maps field sizes in CSC PWRSR status to csc_status_types */
const INT_32 pwrsr_bit_size[CHIPMAN + 1] = {6, 1, 1, 1, 1, 1, 1, 1,
                                            1, 1, 8, 8};

#ifdef __ICCARM__
/* New clock mode flags for IAR compiler only */
static volatile UNS_32 new_clk_mode;
#endif

/***********************************************************************
 * CSC driver private functions and macros
 **********************************************************************/

#ifdef __ghs__
/* 5 inline assmbly NOPS used with GHS compilers */
asm void nops5(void)
{
    NOP
    NOP
    NOP
    NOP
    NOP
}

/* Bus mode change inline assembly code used with GHS compilers */
asm void change_clkbus_mode(UNS_32 clk_mode, UNS_32 bus_mode)
{
%con clk_mode %con bus_mode
    STMFD [sp]!, {r1-r2}
    MRC ARM922T_MMU_CP, 0, r1, ARM922T_MMU_REG_CONTROL, c0, 0
    MOV r2, #clk_mode
    AND r1, r1, #bus_mode
    ORR r1, r1, r2
    MCR ARM922T_MMU_CP, 0, r1, ARM922T_MMU_REG_CONTROL, c0, 0
    LDMFD [sp]!, {r1-r2}
%con clk_mode %reg bus_mode
    STMFD [sp]!, {r1-r2}
    MRC ARM922T_MMU_CP, 0, r1, ARM922T_MMU_REG_CONTROL, c0, 0
    MOV r2, #clk_mode
    AND r1, r1, bus_mode
    ORR r1, r1, r2
    MCR ARM922T_MMU_CP, 0, r1, ARM922T_MMU_REG_CONTROL, c0, 0
    LDMFD [sp]!, {r1-r2}
%reg clk_mode %con bus_mode
    STMFD [sp]!, {r1-r2}
    MRC ARM922T_MMU_CP, 0, r1, ARM922T_MMU_REG_CONTROL, c0, 0
    MOV r2, clk_mode
    AND r1, r1, #bus_mode
    ORR r1, r1, r2
    MCR ARM922T_MMU_CP, 0, r1, ARM922T_MMU_REG_CONTROL, c0, 0
    LDMFD [sp]!, {r1-r2}
%reg clk_mode %reg bus_mode
    STMFD [sp]!, {r1-r2}
    MRC ARM922T_MMU_CP, 0, r1, ARM922T_MMU_REG_CONTROL, c0, 0
    MOV r2, clk_mode
    AND r1, r1, bus_mode
    ORR r1, r1, r2
    MCR ARM922T_MMU_CP, 0, r1, ARM922T_MMU_REG_CONTROL, c0, 0
    LDMFD [sp]!, {r1-r2}
%error
}
#endif

/***********************************************************************
 *
 * Function: csc_get_cpuclk
 *
 * Purpose: Get the CPU clock speed in Hz
 *
 * Processing:
 *     Read the programmed PLL frequency settings from the CLKSET
 *     register. Mask out the main PLL components and compute the
 *     frequency from the masked components. Return the computed
 *     frequency.
 *
 * Parameters: None
 *
 * Outputs: None
 *
 * Returns: The CPU clock speed in Hz
 *
 * Notes: None
 *
 **********************************************************************/
UNS_32 csc_get_cpuclk(void)
{
    UNS_32 m1, m2, p, ps;
    register UNS_32 tmp;

    /* Get clock values from CLKSET register */
    tmp = CLKSC->clkset;

    /* Get programmed PLL values */
    m1 = (tmp >> 7) & 0x0000000F;
    m2 = (tmp >> 11) & 0x000001F;
    p = (tmp >> 2) & 0x0000001F;
    ps = (tmp >> 18) & 0x00000003;

    /* Compute CPU clock speed */
    tmp = (((((m2 + 2) * CLOCK_MAINOSC) >> ps) / (p + 2)) * (m1 + 2));

    return tmp;
}

/***********************************************************************
 *
 * Function: csc_get_hclk
 *
 * Purpose: Get the AHB bus clock speed in Hz
 *
 * Processing:
 *     Get the CPU clock speed by calling the csc_get_cpuclk function.
 *     Mask out the AHB bus divider value from the CLKSET register.
 *     Compute the AHB bus speed by dividing the CPU clock frequency
 *     by the masked AHB bus divider + 1. Return the computed AHB bus
 *     frequency.
 *
 * Parameters: None
 *
 * Outputs: None
 *
 * Returns: The AHB bus clock speed in Hz
 *
 * Notes: None
 *
 **********************************************************************/
UNS_32 csc_get_hclk(void)
{
    UNS_32 hclkdiv;

    /* Get HCLK divider value from CLKSET register */
    hclkdiv = CLKSC->clkset & 0x00000003;

    return (csc_get_cpuclk() / (hclkdiv  + 1));
}

/***********************************************************************
 *
 * Function: csc_get_pclk
 *
 * Purpose: Get the APB bus clock speed in Hz
 *
 * Processing:
 *     Get the AHB bus speed by calling the csc_get_hclk function.
 *     Mask out the APB bus divider value from the CLKSET register.
 *     Compute the APB bus speed by dividing the AHB bus frequency
 *     by 1 shifted left by the (APB bus divider + 1). Return the
 *     computed APB bus frequency.
 *
 * Parameters: None
 *
 * Outputs: None
 *
 * Returns: The APB bus clock speed in Hz
 *
 * Notes: None
 *
 **********************************************************************/
UNS_32 csc_get_pclk(void)
{
    UNS_32 pclkdiv;

    /* Get HCLK divider value from CLKSET register */
    pclkdiv = 1 << (((CLKSC->clkset & 0x30000) >> 16) + 1);

    return (csc_get_hclk() / pclkdiv);
}

/***********************************************************************
 * CSC driver public functions
 **********************************************************************/

/***********************************************************************
 *
 * Function: csc_enter_low_power_state
 *
 * Purpose: Enter the chip low power state
 *
 * Processing:
 *     If the passed argument was HALT, than check the status of the
 *     DMA system. If any DMA channels are active, then enable (or
 *     re-enable the static memory controller clock). If all of the DMA
 *     channels are inactive, then disable the static memory controller
 *     clock to save more power. Go into halt mode by reading the CSC
 *     halt register. After halt, perform five NOP commands to clear
 *     the CPU pipeline. If the passed argument was STANDBY, go into
 *     standby mode by reading the CSC standby register. After standby,
 *     perform five NOP commands to clear the CPU pipeline. Restore the
 *     original state (with or without the SMCROM bit) to the CLKSET
 *     register in the CSC.
 *
 * Parameters:
 *     state : Must be STANDBY or HALT
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes:
 *     If the LCD frame buffer is not located in SDRAM or internal
 *     memory, the HALT function may cause system lockup in the static
 *     memory controller clock is disabled. Use with care or change
 *     this function to *not* disable the static memory controller
 *     clock when placed into HALT mode.
 *
 **********************************************************************/
void csc_enter_low_power_state(CSC_POWER_T state)
{
    volatile UNS_32 tmp;
    volatile UNS_32 *tmpptr;

    if (state == HALT)
    {
        /* System will go into halt state */
        tmpptr = &CLKSC->halt;
    }
    else
    {
        /* System will go into standby state */
        tmpptr = &CLKSC->stby;
    }

    /* Go into halt or standby state by reading the register */
    tmp = *tmpptr ;

    /* NOPs to clear the pipeline after coming out of standby */
#ifdef __ICCARM__
    __no_operation();
    __no_operation();
    __no_operation();
    __no_operation();
    __no_operation();
#endif

#ifdef __GNUC__
    asm ("NOP");
    asm ("NOP");
    asm ("NOP");
    asm ("NOP");
    asm ("NOP");
#endif

#ifdef __ghs__
    nops5();
#endif

#ifdef __arm
    __asm
    {
        NOP
        NOP
        NOP
        NOP
        NOP
    }
#endif
}

/***********************************************************************
 *
 * Function: csc_clear_interrupt
 *
 * Purpose: Clear a CSC interrupt 
 *
 * Processing:
 *     Based on the passed interrupts argument, clear the battery low,
 *     media change, or 64Hz tick interrupt.
 *
 * Parameters:
 *     interrupts : Must be BATTLOWINT, MEDCHGINT, or TICKINT
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void csc_clear_interrupt(CSC_INT_CLEAR_T interrupts)
{
    switch (interrupts)
    {
        case BATTLOWINT:
            /* Clear battery low interrupt */
            CLKSC->bleoi = 0x0;
            break;

        case MEDCHGINT:
            /* Clear media change interrupt */
            CLKSC->mceoi = 0x0;
            break;

        case TICKINT:
            /* Clear 64Hz tick interrupt */
            CLKSC->teoi = 0x0;
            break;

        default:
            break;
    }
}

/***********************************************************************
 *
 * Function: csc_clear_misc_status
 *
 * Purpose: Clear misc CSC status bits
 *
 * Processing:
 *     Write to the CSC STFCLR register to clear the miscellaneous
 *     status bits.
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
void csc_clear_misc_status(void)
{
    /* Clear new battery, user reset, power fail, and cold start status
       flag */
    CLKSC->stfclr = 0x0;
}

/***********************************************************************
 *
 * Function: csc_get_status
 *
 * Purpose: Return a status value from the PWRSR register in the CSC
 *
 * Processing:
 *     Based on the passed argument, read the CSC status register and
 *     shift the value by the appropriate number of bits and then
 *     mask off the most significant bits to leave the desired bit(s).
 *
 * Parameters:
 *     status : Must be an enumeration of type CSC_STATUS_T
 *
 * Outputs: None
 *
 * Returns: The value of the bitfield for the selected status
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 csc_get_status(CSC_STATUS_T status)
{
    /* Read PWRSR status register, shift, mask, and return to caller */
    return ((CLKSC->pwrsr >> pwrsr_bit_index[status]) &
        _BITMASK(pwrsr_bit_size[status]));
}

/***********************************************************************
 *
 * Function: csc_get_clock
 *
 * Purpose: Get CPU, AHB Bus, or APB bus clock speed
 *
 * Processing:
 *     Based on the passed argument, read and return the CPU, AHB bus,
 *     or APB bus frequency to the caller.
 *
 * Parameters:
 *     clock : Must be CPUCLK, AHBCLK, or APBCLK
 *
 * Outputs: None
 *
 * Returns: The selected clock speed in Hz
 *
 * Notes: None
 *
 **********************************************************************/
UNS_32 csc_get_clock(CSC_CLOCK_T clock)
{
    UNS_32 clkinhz = 0;
    
    switch (clock)
    {
        case CPUCLK:
            clkinhz = csc_get_cpuclk();
            break;

        case AHBCLK:
            clkinhz = csc_get_hclk();
            break;

        case APBCLK:
            clkinhz = csc_get_pclk();
            break;

        default:
            break;
    }

    return clkinhz;
}

/***********************************************************************
 *
 * Function: csc_clock_set
 *
 * Purpose: Set the CSC CLKSET register
 *
 * Processing:
 *     Get the existing value of the clock setting register. Set the
 *     state of the static memory controller clock in the new clock
 *     setting word to the state from the existing word. If the existing
 *     clock settings have a bus speed that is a divider (not 1) of the
 *     CPU speed, the existing mode is set to SYNC mode and the new
 *     mode is checked to to be FASTBUS or SYNC mode. If the existing
 *     clock settings have a bus speed that is the same as the CPU
 *     speed, the existing mode is set to FASTBUS mode and the new
 *     mode is checked to to be FASTBUS or SYNC mode. If the new and
 *     existing modes are the same, the clock settings are updated
 *     and 5 'No operation' instructions are executed to clear the
 *     pipeline after the clock setting change, as the chip goes into
 *     standby for a short period of time. If the new and existing
 *     modes are not the same and the new mode is FASTBUS mode, then
 *     the set the new clock speed and then set the new bus mode. If
 *     the new and existing modes are not the same and the new mode
 *     is SYNC mode, then set the new bus mode and then set the new
 *     clock speed.
 *
 * Parameters:
 *     clkset_register_setting : Value to program into CLKSET
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes:
 *     There are 4 possible conditions for bus clocking mode management
 *     in this function. The conditions for handling of each condition
 *     are shown below. The bus clocking mode MUST be configured based
 *     on the CPU and AHB bus speeds or data loss will occur. For
 *     clock settings where the CPU and bus speed are the same, the
 *     FASTBUS clocking mode must be used. For clock settings where the
 *     CPU speed is greater then the bus speed, the SYNC clocking mode
 *     must be used.
 *      Condition 1: Present bus mode is fastbus, new mode is sync
 *          For this condition, the target system bus speed is a
 *          divided speed of the CPU speed and the present system bus
 *          speed is the same as the CPU speed. Prior 
 *      Condition 2: Present bus mode is sync, new mode is fastbus
 *      Condition 3: Present bus mode is sync, new mode is sync
 *          For this condition, no bus mode changes are needed.
 *      Condition 4: Present bus mode is fastbus, new mode is fastbus
 *          For this condition, no bus mode changes are needed.
 *
 **********************************************************************/
void csc_clock_set(UNS_32 clkset_register_setting)
{
    UNS_32 smc_clk_mode;
#ifdef __arm
	UNS_32 trx, tro;
#endif

    /* Save state of static memory controller clock in CLKSET word */
    smc_clk_mode = CLKSC->clkset & CSC_SMCROM;

    /* Switch to fastbus mode to put CPU and bus clocks at the same
       speed */
#ifdef __ICCARM__
    {
        UNS_32 new_clk_mode;

        new_clk_mode = __MRC(15, 0, 1, 0, 0);
        new_clk_mode = new_clk_mode & ARM922T_MMU_CONTROL_BUSMASK;
        new_clk_mode = new_clk_mode | ARM922T_MMU_CONTROL_FASTBUS;
        __MCR(15, 0, new_clk_mode, 1, 0, 0);
        __no_operation();
        __no_operation();
        __no_operation();
        __no_operation();
        __no_operation();
    }
#endif

#ifdef __GNUC__
    asm ("MRC p15, 0, r1, c1, c0, 0");
    asm ("MOV r2, %0" : : "r" (ARM922T_MMU_CONTROL_FASTBUS));
    asm ("AND r1, r1, %0" : : "r" (ARM922T_MMU_CONTROL_BUSMASK));
    asm ("ORR r1, r1, r2");
    asm ("MCR p15, 0, r1, c1, c0, 0");
    asm ("NOP");
    asm ("NOP");
    asm ("NOP");
    asm ("NOP");
    asm ("NOP");
#endif

#ifdef __ghs__
    change_clkbus_mode (ARM922T_MMU_CONTROL_FASTBUS,
        ARM922T_MMU_CONTROL_BUSMASK);
    nops5();
#endif

#ifdef __arm
    __asm
    {
        MRC ARM922T_MMU_CP, 0, trx, ARM922T_MMU_REG_CONTROL, c0, 0
        MOV tro, #ARM922T_MMU_CONTROL_FASTBUS
        AND trx, trx, #ARM922T_MMU_CONTROL_BUSMASK
        ORR trx, trx, tro
        MCR ARM922T_MMU_CP, 0, trx, ARM922T_MMU_REG_CONTROL, c0, 0
        NOP
        NOP
        NOP
        NOP
        NOP
    }
#endif

    /* Set the new clock mode */
    CLKSC->clkset = (clkset_register_setting | smc_clk_mode);

    /* NOPs to clear the pipeline after coming out of standby */
#ifdef __ICCARM__
    __no_operation();
    __no_operation();
    __no_operation();
    __no_operation();
    __no_operation();
#endif

#ifdef __GNUC__
    asm ("NOP");
    asm ("NOP");
    asm ("NOP");
    asm ("NOP");
    asm ("NOP");
#endif

#ifdef __ghs__
    nops5();
#endif

#ifdef __arm
    __asm
    {
        NOP
        NOP
        NOP
        NOP
        NOP
    }
#endif

    /* If the CPU and bus clocks are different, switch to SYNC mode */
    if ((clkset_register_setting & 0x00000003) != 0)
    {
#ifdef __ICCARM__
    {
        UNS_32 new_clk_mode;

        new_clk_mode = __MRC(15, 0, 1, 0, 0);
        new_clk_mode = new_clk_mode & ARM922T_MMU_CONTROL_BUSMASK;
        new_clk_mode = new_clk_mode | ARM922T_MMU_CONTROL_SYNC;
        __MCR(15, 0, new_clk_mode, 1, 0, 0);
        __no_operation();
        __no_operation();
        __no_operation();
        __no_operation();
        __no_operation();
    }
#endif

#ifdef __GNUC__
        asm ("MRC p15, 0, r1, c1, c0, 0");
        asm ("MOV r2, %0" : : "r" (ARM922T_MMU_CONTROL_SYNC));
        asm ("AND r1, r1, %0" : : "r" \
            (ARM922T_MMU_CONTROL_BUSMASK));
        asm ("ORR r1, r1, r2");
        asm ("MCR p15, 0, r1, c1, c0, 0");
        asm ("NOP");
        asm ("NOP");
        asm ("NOP");
        asm ("NOP");
        asm ("NOP");
#endif

#ifdef __ghs__
        change_clkbus_mode (ARM922T_MMU_CONTROL_SYNC,
            ARM922T_MMU_CONTROL_BUSMASK);
        nops5();
#endif

#ifdef __arm
        __asm
        {
            MRC ARM922T_MMU_CP, 0,trx, ARM922T_MMU_REG_CONTROL, c0, 0
            MOV tro, #ARM922T_MMU_CONTROL_SYNC
            AND trx, trx, #ARM922T_MMU_CONTROL_BUSMASK
            ORR trx, trx, tro
            MCR ARM922T_MMU_CP, 0, trx, ARM922T_MMU_REG_CONTROL, c0, 0
            NOP
            NOP
            NOP
            NOP
            NOP
        }
#endif
    }
}

/***********************************************************************
 *
 * Function: csc_stby_wakeup_enable
 *
 * Purpose: Enable or disable the wakeup signal
 *
 * Processing:
 *     Disable the wakeup signal by setting the wakeup disable bit in
 *     the CSC WRCNT register.
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
void csc_stby_wakeup_enable(BOOL_32 enable)
{
    if (enable == TRUE)
    {
        /* Wakeup is enabled */
        CLKSC->pwrcnt &= ~CSC_PWRCNT_WAKEDIS;
    }
    else
    {
        /* Wakeup is disabled */
        CLKSC->pwrcnt |= CSC_PWRCNT_WAKEDIS;
    }
}

/***********************************************************************
 *
 * Function: csc_set_pgmclk_divisor
 *
 * Purpose: Set the PGMCLK divisor (> 1) or disable it (= 0)
 *
 * Processing:
 *     Get the value of the CSC power count register and mask off the
 *     clock program bits. Or in the new clock program bits into the
 *     power count register.
 *
 * Parameters:
 *     div : Program clock divisor value (0 to 254)
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void csc_set_pgmclk_divisor(UNS_8 div)
{
    UNS_32 tmp;

    /* Get register and mask off clock program bits */
    tmp = ((CLKSC->pwrcnt & ~CSC_PWRCNT_PGMCLK(0xFF)) |
        CSC_PWRCNT_PGMCLK(div));
    CLKSC->pwrcnt = tmp;
}

/***********************************************************************
 *
 * Function: csc_enable_dma_clock
 *
 * Purpose: Enable or disable a DMA controller channel clock
 *
 * Processing:
 *     If enable is TRUE, enables a selected DMA channel by setting the
 *     appropriate bit in the CSC PWRCNT register. If enable is FALSE,
 *     the appropriate bit in the CSC PWRCNT register id disabled.
 *
 * Parameters:
 *     channel : Must be of enumeration type CSC_DMA_CHANNEL_T
 *     enable  : Must be TRUE or FALSE
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void csc_enable_dma_clock(CSC_DMA_CHANNEL_T channel, BOOL_32 enable)
{
    CLKSC->pwrcnt |= (CSC_PWRCNT_DMAPCH1 << (INT_32) channel);
}

/***********************************************************************
 *
 * Function: csc_usb_reset
 *
 * Purpose: Issue a USB IO or control reset
 *
 * Processing:
 *     Issues a USB control or IO reset by setting the 
 *
 * Parameters:
 *     reset : Must be USB_IO or USB_CONTROL
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void csc_usb_reset(CSC_USB_RESET_T reset)
{
    CLKSC->usbreset |= (UNS_32) reset;
}

/***********************************************************************
 *
 * Function: csc_compute_divider
 *
 * Purpose:
 *     Compute the closest divider to the passed input frequency and
 *     desired target frequency 
 *
 * Processing:
 *     Start by computing a divider value that will get the closest
 *     frequency just below the desired target, based on the input
 *     frequency. If the divider is 1, just return '1'. Otherwise,
 *     determine if the divider value or the divider value minus 1 is
 *     a closer match to get the target frequency. Return the closest
 *     match divider value.
 *
 * Parameters:
 *     input_freq   : Input frequency (such as AHB bus speed)
 *     desired_freq : Target frequency
 *
 * Outputs: None
 *
 * Returns:
 *     The closest divider value for (Input frequency) divided by
 *     (Target frequency)
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 csc_compute_divider(UNS_32 input_freq,
                           UNS_32 desired_freq)
{
    UNS_32 div_clock, clock_diff1, clock_diff2;
    UNS_32 div = 1;

    /* Strt with a divider of 1 */
    div_clock = input_freq / div;

    /* Try to get the best divider just under the desired frequency */
    while (div_clock > desired_freq)
    {
        div++;
        div_clock = input_freq / div;
    }

    if (div > 1)
    {
        /* Determine if 'div' or 'div - 1' is closer to desired
           frequency */
        if (div_clock > desired_freq)
        {
            clock_diff1 = div_clock - desired_freq;
        }
        else
        {
            clock_diff1 = desired_freq - div_clock;
        }

        div_clock = desired_freq / (div - 1);
        if (div_clock > desired_freq)
        {
            clock_diff2 = div_clock - desired_freq;
        }
        else
        {
            clock_diff2 = desired_freq - div_clock;
        }

        /* If diff1 is greater than diff2, then 'div + 1' is the
           closest divider */
        if ((clock_diff1 > clock_diff2) && (div > 1))
        {
            div--;
        }
    }

    return div;
}
