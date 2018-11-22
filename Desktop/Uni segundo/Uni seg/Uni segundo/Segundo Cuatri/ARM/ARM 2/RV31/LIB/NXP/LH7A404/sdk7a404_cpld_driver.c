/***********************************************************************
 * $Id:: sdk7a404_cpld_driver.c 14 2007-08-28 16:08:18Z kevinw         $
 *
 * Project: LogicPD SDK7A404 CPLD driver
 *
 * Description:
 *     This file contains driver support for the CPLD module on the
 *     SDK7A404 EVB.
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

#include "sdk7a404_cpld_driver.h"

/***********************************************************************
 * CPLD driver public functions
 **********************************************************************/

/***********************************************************************
 *
 * Function: cpld_init
 *
 * Purpose: Initializes the CPLD driver (stub function only)
 *
 * Processing:
 *     Does nothing
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
void cpld_init(void)
{
    ;
}

/***********************************************************************
 *
 * Function: cpld_enable_lan_power
 *
 * Purpose: Enable or disable LAN power
 *
 * Processing:
 *     If enable is TRUE, enable the LAN device power. Otherwise,
 *     disable it.
 *
 * Parameters:
 *     enable: TRUE to enable LAN power, FALSE to disable it
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void cpld_enable_lan_power(BOOL_32 enable)
{
    if (enable == TRUE)
    {
        /* Enable LAN chip power */
        * (volatile UNS_16 *) CARDE_REG_BASE &= ~CARDE_LAN_PWR_DISABLE;
    }
    else
    {
        /* Disable LAN chip power */
        * (volatile UNS_16 *) CARDE_REG_BASE |= CARDE_LAN_PWR_DISABLE;
    }
}

/***********************************************************************
 *
 * Function: cpld_lan_int_enable
 *
 * Purpose: Disable or enable the LAN interrupt
 *
 * Processing:
 *     If enable is TRUE, enable the LAN interrupt. Otherwise, disable
 *     it.
 *
 * Parameters:
 *     enable: TRUE to enable the LAN interrupt, FALSE to disable it
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void cpld_lan_int_enable(BOOL_32 enable)
{
    if (enable == TRUE)
    {
        /* Enable LAN interrupt */
        * (volatile UNS_16 *) INTMSK_REG_BASE &= ~INTMSK_LAN_DISABLE;
    }
    else
    {
        /* Disable LAN interrupt */
        * (volatile UNS_16 *) INTMSK_REG_BASE |= INTMSK_LAN_DISABLE;
    }
}

/***********************************************************************
 *
 * Function: cpld_lan_int_pending
 *
 * Purpose: Return LAN interrupt pending status
 *
 * Processing:
 *     If the LAN interrupt is pending (per the CPLD), return TRUE to
 *     the caller. Otherwise, return FALSE.
 *
 * Parameters: None
 *
 * Outputs: None
 *
 * Returns: TRUE of the LAN interrupt is pending, otherwise FALSE
 *
 * Notes: None
 *
 **********************************************************************/
BOOL_32 cpld_lan_int_pending(void)
{
    BOOL_32 pending = FALSE;

    if (((* (volatile UNS_16 *) INTMSK_REG_BASE) &
        INTMSK_LAN_PENDING) == 0)
    {
        pending = TRUE;
    }

    return pending;
}

/***********************************************************************
 *
 * Function: cpld_enable_flash_prog
 *
 * Purpose: Enable or disable FLASH programming
 *
 * Processing:
 *     If enable is TRUE, enable FLASH programming (by disabling FLASH
 *     write protection). Otherwise, disable it.
 *
 * Parameters:
 *     enable: TRUE to enable FLASH programming, FALSE to disable it
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void cpld_enable_flash_prog(BOOL_32 enable)
{
    if (enable == TRUE)
    {
        /* Enable FLASH programming */
        * (volatile UNS_16 *) FLASH_REG_BASE |= FLASH_PROG_ENABLE;
    }
    else
    {
        /* Disable FLASH programming */
        * (volatile UNS_16 *) FLASH_REG_BASE &= ~FLASH_PROG_ENABLE;
    }
}

/***********************************************************************
 *
 * Function: cpld_get_flash_sts
 *
 * Purpose: Read FLASH STS1 or STS2 bit
 *
 * Processing:
 *     For the passed value of flash_sts, check the status of the
 *     STS bit in the CPLD FLASH register. If the bit is set, return
 *     TRUE to the caller. Otherwise, return FALSE.
 *
 * Parameters:
 *     flash_sts: Must be CPLD_STS1 or CPLD_STS2
 *
 * Outputs: None
 *
 * Returns:
 *     TRUE of the STS signal for the selected device is high,
 *     otherwise FALSE
 *
 * Notes: None
 *
 **********************************************************************/
BOOL_32 cpld_get_flash_sts(CPLD_FLASH_STS_T flash_sts)
{
    UNS_16 mask = FLASH_STS1_MASK;
    BOOL_32 set = FALSE;

    if (flash_sts == CPLD_STS2)
    {
        mask = FLASH_STS2_MASK;
    }

    if (((* (volatile UNS_16 *) FLASH_REG_BASE) & mask) != 0)
    {
        set = TRUE;
    }

    return set;
}

/***********************************************************************
 *
 * Function: cpld_enable_gpio
 *
 * Purpose: Set or disable the extended GPIO output state
 *
 * Processing:
 *     Set the value of mask with the LED mask bit for the passed value
 *     of led. If set is TRUE, enable the LED for the value of mask.
 *     Otherwise, disable (turn off) the LED.
 *
 * Parameters:
 *     set: TRUE to turn on the LED, FALSE to turn it off
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void cpld_enable_gpio(BOOL_32 set)
{
#if 0 /* Function disabled */
    if (set == TRUE)
    {
        * (volatile UNS_16 *) EXTGPIO_REG_BASE &= ~EXTGPIO_GPIO1_BIT;
    }
    else
    {
        * (volatile UNS_16 *) EXTGPIO_REG_BASE |= EXTGPIO_GPIO1_BIT;
    }
#endif
}

/***********************************************************************
 *
 * Function: cpld_set_gpio_dir
 *
 * Purpose: Set direction of CPLD GPIO data register
 *
 * Processing:
 *     if input is TRUE, set the GPIO data direction register in the
 *     CPLD to input mode. Otherwise, set it to output mode.
 *
 * Parameters:
 *     gpio_bit_group: Group of bits to set direction on
 *     gpio_dir      : Data direction enumeration (CPLD_GPIO_DIR_T)
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void cpld_set_gpio_dir(CPLD_GPIO_BITS_T gpio_bit_group,
                       CPLD_GPIO_DIR_T gpio_dir)
{
    if (gpio_dir == CPLD_GPIO_INPUT)
    {
        * (volatile UNS_16 *) GPDIR_REG_BASE |=
            (0x0001 << (INT_32) gpio_bit_group);
    }
    else
    {
        * (volatile UNS_16 *) GPDIR_REG_BASE &=
            ~(0x0001 << (INT_32) gpio_bit_group);
    }
}

/***********************************************************************
 *
 * Function: cpld_set_gpio_data
 *
 * Purpose: Set the CPLD GPIO data register
 *
 * Processing:
 *     Set the CPLD GPIO data register with the passed value of data.
 *
 * Parameters:
 *     data: Value to set the CPLD GPIO data register with
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void cpld_set_gpio_data(UNS_8 data)
{
    * (volatile UNS_16 *) GPDAT_REG_BASE  = (UNS_16) data;
}

/***********************************************************************
 *
 * Function: cpld_get_gpio_data
 *
 * Purpose: Get the CPLD GPIO data register
 *
 * Processing:
 *     Read the CPLD GPIO data register and return the value to the
 *     caller.
 *
 * Parameters: None
 *
 * Outputs: None
 *
 * Returns: The value of the CPLD GPIO data register
 *
 * Notes: None
 *
 **********************************************************************/
UNS_8 cpld_get_gpio_data(void)
{
    return (UNS_8) (* (volatile UNS_16 *) GPDAT_REG_BASE & 0xFF);
}

/***********************************************************************
 *
 * Function: cpld_enable_led
 *
 * Purpose: Set or disable an application board LED
 *
 * Processing:
 *     Set or disavle the application LED.
 *
 * Parameters:
 *     led: Must be LED_GPIO or LED_STATUS
 *     set: TRUE to turn on the LED, FALSE to turn it off
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void cpld_enable_led(BOOL_32 set)
{
    if (set == TRUE)
    {
        * (volatile UNS_16 *) LED_REG_BASE &= LED_GPIO_DISABLE;
    }
    else
    {
        * (volatile UNS_16 *) LED_REG_BASE |= LED_GPIO_DISABLE;
    }
}
