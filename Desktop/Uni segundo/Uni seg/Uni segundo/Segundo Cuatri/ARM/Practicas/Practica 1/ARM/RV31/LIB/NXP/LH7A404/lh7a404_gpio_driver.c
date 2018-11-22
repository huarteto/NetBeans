/***********************************************************************
 * $Id:: lh7a404_gpio_driver.c 14 2007-08-28 16:08:18Z kevinw          $
 *
 * Project: LH7A404 GPIO driver
 *
 * Description:
 *     This file contains driver support for the GPIO module on the
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

#include "lh7a404_gpio_driver.h"

/***********************************************************************
 * GPIO driver public functions
 **********************************************************************/

/***********************************************************************
 *
 * Function: gpio_set_data_dir
 *
 * Purpose: Set a GPIO data port's bits as inputs or outputs
 *
 * Processing:
 *     For the selected value of port, update the data direction
 *     register with the passed value of dir.
 *
 * Parameters:
 *     port : Must be an enueration of type GPIO_PORT_T
 *     pins : Selected pins for the 'dir' operation 
 *     dir  : Direction to set the selected 'pins'
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void gpio_set_data_dir(GPIO_PORT_T port,
                       UNS_8 pins,
                       GPIO_DIR_T dir)
{
    if (dir == GPIO_INPUT)
    {
        /* GPIO inputs have the corresponding data direction bit set
           to 0 */
        switch (port)
        {
            case GPIO_PORT_A:
                GPIO->paddr &= (~(UNS_32) pins);
                break;

            case GPIO_PORT_B:
                GPIO->pbddr &= (~(UNS_32) pins);
                break;

            case GPIO_PORT_C:
                GPIO->pcddr |= (UNS_32) pins;
                break;

            case GPIO_PORT_D:
                GPIO->pdddr |= (UNS_32) pins;
                break;

            case GPIO_PORT_E:
                GPIO->peddr &= (~(UNS_32) pins);
                break;

            case GPIO_PORT_F:
                GPIO->pfddr &= (~(UNS_32) pins);
                break;

            case GPIO_PORT_G:
                GPIO->pgddr |= (UNS_32) pins;
                break;

            case GPIO_PORT_H:
                GPIO->phddr &= (~(UNS_32) pins);
                break;

            default:
                break;
        }
    }
    else
    {
        /* GPIO outputs have the corresponding data direction bit set
           to 1 */
        switch (port)
        {
            case GPIO_PORT_A:
                GPIO->paddr |= (UNS_32) pins;
                break;

            case GPIO_PORT_B:
                GPIO->pbddr |= (UNS_32) pins;
                break;

            case GPIO_PORT_C:
                GPIO->pcddr &= (~(UNS_32) pins);
                break;

            case GPIO_PORT_D:
                GPIO->pdddr &= (~(UNS_32) pins);
                break;

            case GPIO_PORT_E:
                GPIO->peddr |= (UNS_32) pins;
                break;

            case GPIO_PORT_F:
                GPIO->pfddr |= (UNS_32) pins;
                break;

            case GPIO_PORT_G:
                GPIO->pgddr &= (~(UNS_32) pins);
                break;

            case GPIO_PORT_H:
                GPIO->phddr |= (UNS_32) pins;
                break;

            default:
                break;
        }
    }
}

/***********************************************************************
 *
 * Function: gpio_get_data_dir
 *
 * Purpose: Get a GPIO data port's data direction status
 *
 * Processing:
 *     For the selected value of port, read the data direction
 *     register and return the value to the caller.
 *
 * Parameters:
 *     port : Must be an enueration of type GPIO_PORT_T
 *
 * Outputs: None
 *
 * Returns: The raw value of the data direction register for port
 *
 * Notes:
 *     For ports A, B, E, F, and H, a '1' specifies an output, while
 *     ports C, D, and G use a '1' for an input.
 *
 **********************************************************************/
UNS_8 gpio_get_data_dir(GPIO_PORT_T port)

{
    UNS_32 dir;

    switch (port)
    {
        case GPIO_PORT_A:
            dir = GPIO->paddr;
            break;

        case GPIO_PORT_B:
            dir = GPIO->pbddr;
            break;

        case GPIO_PORT_C:
            dir = GPIO->pcddr;
            break;

        case GPIO_PORT_D:
            dir = GPIO->pdddr;
            break;

        case GPIO_PORT_E:
            dir = GPIO->peddr;
            break;

        case GPIO_PORT_F:
            dir = GPIO->pfddr;
            break;

        case GPIO_PORT_G:
            dir = GPIO->pgddr;
            break;

        case GPIO_PORT_H:
            dir = GPIO->phddr;
            break;

        default:
        	dir = 0;
            break;
    }

    return (UNS_32) dir;
}

/***********************************************************************
 *
 * Function: gpio_data_write
 *
 * Purpose: Write to a GPIO data register (output)
 *
 * Processing:
 *     For the selected value of port, update the data register with
 *     the passed value of data.
 *
 * Parameters:
 *     port : Must be an enueration of type GPIO_PORT_T
 *     data : Value to write to the data register (1 = high)
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void gpio_data_write(GPIO_PORT_T port,
                     UNS_8 data)
{
    switch (port)
    {
        case GPIO_PORT_A:
            GPIO->padr = (UNS_32) data;
            break;

        case GPIO_PORT_B:
            GPIO->pbdr = (UNS_32) data;
            break;

        case GPIO_PORT_C:
            GPIO->pcdr = (UNS_32) data;
            break;

        case GPIO_PORT_D:
            GPIO->pddr = (UNS_32) data;
            break;

        case GPIO_PORT_E:
            GPIO->pedr = (UNS_32) data;
            break;

        case GPIO_PORT_F:
            GPIO->pfdr = (UNS_32) data;
            break;

        case GPIO_PORT_G:
            GPIO->pgdr = (UNS_32) data;
            break;

        case GPIO_PORT_H:
            GPIO->phdr = (UNS_32) data;
            break;

        default:
            break;
    }
}

/***********************************************************************
 *
 * Function: gpio_data_read
 *
 * Purpose: Read a GPIO data register (input)
 *
 * Processing:
 *     For the selected value of port, read the data register and
 *     return the value to the caller.
 *
 * Parameters:
 *     port : Must be an enueration of type GPIO_PORT_T
 *
 * Outputs: None
 *
 * Returns: The data value for the data register 'port'
 *
 * Notes:
 *     The states of the GPIO inputs are read from the feedback input
 *     signals. In some conditions, the programmed state of an output
 *     may not match a read performed with this function.
 *
 **********************************************************************/
UNS_8 gpio_data_read(GPIO_PORT_T port)
{
    UNS_32 data;

    switch (port)
    {
        case GPIO_PORT_A:
            data = GPIO->papindr;
            break;

        case GPIO_PORT_B:
            data = GPIO->pbpindr;
            break;

        case GPIO_PORT_C:
            data = GPIO->pcpindr;
            break;

        case GPIO_PORT_D:
            data = GPIO->pdpindr;
            break;

        case GPIO_PORT_E:
            data = GPIO->pepindr;
            break;

        case GPIO_PORT_F:
            data = GPIO->pfpindr;
            break;

        case GPIO_PORT_G:
            data = GPIO->pgpindr;
            break;

        case GPIO_PORT_H:
            data = GPIO->phpindr;
            break;

        default:
        	data = 0;
            break;
    }

    return (UNS_8) data;
}

/***********************************************************************
 *
 * Function: gpio_kbcol_drive
 *
 * Purpose: Drive a keyboard column high, low, or hi-z
 *
 * Processing:
 *     For the passed keyboard column value (col), set the drive line
 *     for the column to high, low, or high impediance based on the
 *     value of kbstate.
 *
 * Parameters:
 *     col     : Must be an enumeration of type GPIO_KB_COL_T
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void gpio_kbcol_drive(GPIO_KB_COL_T col)
{
    GPIO->kscan = (UNS_32) col;
}

/***********************************************************************
 *
 * Function: gpio_uart3_select
 *
 * Purpose: Select UART3 (TRUE) or GPIO signals
 *
 * Processing:
 *     If select is TRUE, select UART3 operation on GPIO signals B1,
 *     B2, B3, B4, and B5. Otherwise, these signals are placed in a
 *     GPIO mode.
 *
 * Parameters:
 *     select : TRUE for UART3 mode, FALSE for GPIO mode
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void gpio_uart3_select(BOOL_32 select)
{
    if (select == TRUE)
    {
        GPIO->pinmux |= GPIO_PINMUX_UART3ON;
    }
    else
    {
        GPIO->pinmux &= ~GPIO_PINMUX_UART3ON;
    }
}

/***********************************************************************
 *
 * Function: gpio_lcd_signal_select
 *
 * Purpose: Select LCD data signal multiplexing
 *
 * Processing:
 *     Read the GPIO pinmix register and mask off the LCD data muxing
 *     bits. 'OR' in the new selected bits and update the GPIO pinmux
 *     register.
 *
 * Parameters:
 *     select : Must be an enumeration of type GPIO_LCD_MP_T
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void gpio_lcd_signal_select(GPIO_LCD_MP_T lcdm)
{
    UNS_32 pinmux;

    /* Read pinmux register and mask off LCD data muxing bits */
    pinmux = GPIO->pinmux & ~(GPIO_PINMUX_PEOCON | GPIO_PINMUX_PDOCON);

    /* Write selected bits into the pinmux register */
    GPIO->pinmux = (pinmux | (UNS_32) lcdm);
}

/***********************************************************************
 *
 * Function: gpio_codec_ac97_select
 *
 * Purpose: Select AC97 CODEC (TRUE) or ACI CODEC interface
 *
 * Processing:
 *     If select is TRUE, select AC97 (AAC) CODEC operation. Otherwise,
 *     select ACI CODEC operation.
 *
 * Parameters:
 *     select : TRUE for AC97 CODEC mode, FALSE for ACI CODEC mode
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void gpio_codec_ac97_select(BOOL_32 select)
{
    if (select == TRUE)
    {
        GPIO->pinmux |= GPIO_PINMUX_CODECON;
    }
    else
    {
        GPIO->pinmux &= GPIO_PINMUX_CODECON;
    }
}

/***********************************************************************
 *
 * Function: gpio_sclke_select
 *
 * Purpose: Select states of synchronous clocks enables 0/1/2 and
 *          cs6/cs7
 *
 * Processing:
 *     Read the GPIO pinmix register and mask off the synchronous clock
 *     muxing bits. 'OR' in the new selected bits and update the GPIO
 *     pinmux register.
 *
 * Parameters:
 *     sclke :  Must be an enumeration of type GPIO_SCLKE_T
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void gpio_sclke_select(GPIO_SCLKE_T sclke)
{
    UNS_32 pinmux;

    /* Read pinmux register and mask off clock muxing bits */
    pinmux = GPIO->pinmux & ~(GPIO_PINMUX_CLK0EN | GPIO_PINMUX_CLK12EN);

    /* Write selected bits into the pinmux register */
    GPIO->pinmux = (pinmux | (UNS_32) sclke);
}

/***********************************************************************
 *
 * Function: gpio_ext_int_config
 *
 * Purpose: Set an external interrupt's level/edge and high/low active
 *          mode
 *
 * Processing:
 *     If edge is TRUE, configure the appropriate interrupt designated
 *     by the passed value of intr to an edge triggered interrupt mode.
 *     Otherwise, configure it for level sensitive mode. If high is
 *     TRUE, configure the appropriate interrupt for active high mode.
 *     Otherwise, configure it for level active low mode
 *
 * Parameters:
 *     intr : Must be an enumeration of type GPIO_INT_T
 *     edge : If TRUE, use edge mode, else use level mode
 *     high : If TRUE, use active high mode, else ude active low mode
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void gpio_ext_int_config(GPIO_INT_T intr,
                         BOOL_32 edge,
                         BOOL_32 high)
{
    /* Setup interrupt signal as an input */
    gpio_set_data_dir(GPIO_PORT_F, (UNS_8) intr, GPIO_INPUT);
    
    if (edge == TRUE)
    {
        /* Set interrupt as active on edge */
        GPIO->inttype1 |= (UNS_32) intr;
    }
    else
    {
        /* Set interrupt as active on level */
        GPIO->inttype1 &= ~(UNS_32) intr;
    }

    if (high == TRUE)
    {
        /* Set interrupt as active high */
        GPIO->inttype2 |= (UNS_32) intr;
    }
    else
    {
        /* Set interrupt as active low */
        GPIO->inttype2 &= ~(UNS_32) intr;
    }
}

/***********************************************************************
 *
 * Function: gpio_clear_int
 *
 * Purpose: Clear a pending external (edge) interrupt
 *
 * Processing:
 *     Clear the approproate edge triggered interrupt by setting the
 *     end-of-interrupt bit for that interrupt.
 *
 * Parameters:
 *     intr : Must be an enumeration of type GPIO_INT_T
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void gpio_clear_int(GPIO_INT_T intr)
{
    GPIO->gpiofeoi = (UNS_32) intr;
}

/***********************************************************************
 *
 * Function: gpio_ext_int_enable
 *
 * Purpose: Enable or disable an external interrupt
 *
 * Processing:
 *     If enable is TRUE, enable the port F external interrupt for the
 *     passed value of intr by setting the appropriate bit in the
 *     interrupt enable register. If enable is FALSE, disable the
 *     interrupt and return functionality of the port to a GPIO.
 *
 * Parameters:
 *     intr : Must be an enumeration of type GPIO_INT_T
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void gpio_ext_int_enable(GPIO_INT_T intr,
                         BOOL_32 enable)
{
    if (enable == TRUE)
    {
        GPIO->gpiointen |= (UNS_32)intr;
    }
    else
    {
        GPIO->gpiointen &= ~(UNS_32)intr;
    }
}

/***********************************************************************
 *
 * Function: gpio_interrupt_pending
 *
 * Purpose: Return pending state of an external interrupt
 *
 * Processing:
 *     Check the interrupt status bit for the specified interrupt based
 *     on intr. If the bit is set, return TRUE to the caller. Otherwise,
 *     return FALSE to the caller.
 *
 * Parameters:
 *     intr : Must be an enumeration of type GPIO_INT_T
 *
 * Outputs: None
 *
 * Returns: TRUE if the specified interrupt is pending, otherwise FALSE
 *
 * Notes: None
 *
 **********************************************************************/
BOOL_32 gpio_interrupt_pending(GPIO_INT_T intr)
{
    BOOL_32 pending = FALSE;

    if ((GPIO->intstatus & ~(UNS_32) intr) != 0)
    {
        pending = TRUE;
    }

    return pending;
}

/***********************************************************************
 *
 * Function: gpio_enable_int_debounce
 *
 * Purpose: Enable or disable signal debouncing for an external
 *          interrupt
 *
 * Processing:
 *     If enable is TRUE, enable interrupt debouncing for the interrupt
 *     specified by intr by setting the debounce bit for the interrupt
 *     in the debounce register. Otherwise, disable the bit to disable
 *     interrupt signal debouncing.
 *
 * Parameters:
 *     intr   : Must be an enumeration of type GPIO_INT_T
 *     enable : If TRUE, enable debouncing for the specific interrupt
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void gpio_enable_int_debounce(GPIO_INT_T intr,
                              BOOL_32 enable)
{
    if (enable == TRUE)
    {
        GPIO->gpiodb |= (UNS_32)intr;
    }
    else
    {
        GPIO->gpiodb &= ~(UNS_32)intr;
    }
}
