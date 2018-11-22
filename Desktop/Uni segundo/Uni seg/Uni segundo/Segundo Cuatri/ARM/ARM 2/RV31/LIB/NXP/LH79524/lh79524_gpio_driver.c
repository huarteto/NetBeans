/***********************************************************************
 * $Id:: lh79524_gpio_driver.c 27 2007-08-31 19:35:06Z kevinw          $
 *
 * Project: LH79524 GPIO driver
 *
 * Description:
 *     This file contains driver support for the GPIO module on the
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

#include "lh79524_iocon.h"
#include "lh79524_rcpc.h"
#include "lh79524_gpio_driver.h"



/***********************************************************************
 *
 * Function: gpio_set_data_dir
 *
 * Purpose: Set a GPIO data port's bits direction as inputs or outputs
 *
 * Processing:
 *     For the selected value of port, update the data direction
 *     register with the passed value of dir.
 *
 * Parameters:
 *     gpio_port    Must be a GPIO port of type GPIO_REGS_T
 *     pins         Selected pins for the 'dir' operation
 *     dir          Direction to set. GPIO_INPUT or GPIO_OUTPUT
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void gpio_set_data_dir(GPIO_REGS_T * gpio_port,
                       UNS_8 pins,
                       GPIO_DIR_T dir)
{
	switch((INT_32)gpio_port)
	{
	    /* Bi-directional ports */
		case (INT_32)GPIOA:
		case (INT_32)GPIOB:
		case (INT_32)GPIOC:
		case (INT_32)GPIOD:
		case (INT_32)GPIOE:
		case (INT_32)GPIOF:
		case (INT_32)GPIOG:
		case (INT_32)GPIOH:
		case (INT_32)GPIOI:
		case (INT_32)GPIOK:
		case (INT_32)GPIOL:
		case (INT_32)GPION:
		    if (dir == GPIO_INPUT)
		        gpio_port->ddr &= (~(UNS_32) pins);
		    else
		        gpio_port->ddr |= ((UNS_32) pins);   
		    break;
		/* Input only port */    
		case (INT_32)GPIOJ:
		    if (dir == GPIO_INPUT)
		        gpio_port->ddr &= (~(UNS_32) pins);
		    break;
		/* Output only port */    
		case (INT_32)GPIOM:
		    if (dir == GPIO_OUTPUT)
		        gpio_port->ddr |= ((UNS_32) pins);
        /* Port not defined */
		default:
		    break;
	}
}	
		                    
		            
/***********************************************************************
 *
 * Function: gpio_get_data_dir
 *
 * Purpose: get a GPIO data port's direction setting
 *
 * Processing:
 *     For the selected value of port, update the data direction
 *     register with the passed value of dir.
 *
 * Parameters:
 *     gpio_port    Must be an GPIO port of type GPIO_REGS_T
 *     pins         Selected pins for the 'dir' operation
 *     dir          Direction to set. GPIO_INPUT or GPIO_OUTPUT
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
UNS_8 gpio_get_data_dir(GPIO_REGS_T * gpio_port)
{
	return (gpio_port->ddr);
}	
		                    

/***********************************************************************
 *
 * Function: gpio_write_byte
 *
 * Purpose: Write one byte data to GPIO port
 *
 * Processing:
 *     For the selected value of port, update the data
 *     register with the passed value of data.
 *
 * Parameters:
 *     gpio_port    Must be a GPIO port of type GPIO_REGS_T
 *     data         Data for output
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void gpio_write_byte(GPIO_REGS_T * gpio_port, UNS_8 data)
{
	switch((INT_32)gpio_port)
	{
		case (INT_32)GPIOA:
		case (INT_32)GPIOB:
		case (INT_32)GPIOC:
		case (INT_32)GPIOD:
		case (INT_32)GPIOE:
		case (INT_32)GPIOF:
		case (INT_32)GPIOG:
		case (INT_32)GPIOH:
		case (INT_32)GPIOI:
		case (INT_32)GPIOK:
		case (INT_32)GPIOL:
		case (INT_32)GPION:
        case (INT_32)GPIOM:		
		    gpio_port->dr = data;
		    break;
		default:
		    break;
	}
}
	
		                    
/***********************************************************************
 *
 * Function: gpio_read_byte
 *
 * Purpose: Read one byte data from GPIO port
 *
 * Processing:
 *     For the selected port, read the data register.
 *
 * Parameters:
 *     gpio_port    Must be a GPIO port of type GPIO_REGS_T
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
UNS_8 gpio_read_byte(GPIO_REGS_T * gpio_port)
{
	UNS_8 temp;

	switch((INT_32)gpio_port)
	{
		case (INT_32)GPIOA:
		case (INT_32)GPIOB:
		case (INT_32)GPIOC:
		case (INT_32)GPIOD:
		case (INT_32)GPIOE:
		case (INT_32)GPIOF:
		case (INT_32)GPIOG:
		case (INT_32)GPIOH:
		case (INT_32)GPIOI:
		case (INT_32)GPIOK:
		case (INT_32)GPIOL:
		case (INT_32)GPION:
        case (INT_32)GPIOJ:		
		    temp = gpio_port->dr;
		    break;
		default:
		    temp = 0;
		    break;
	}

    return temp;
}	


/***********************************************************************
 *
 * Function: gpio_ext_int_init
 *
 * Purpose: external int pin init function
 *
 * Processing:
 *
 * Parameters:
 *     source:  EXT_INT0 to EXT_INT7
 *     trigger: Level low  = 0
 *              Level high = 1
 *              Falling edge = 2
 *              Rising edge = 3
 *
 * Outputs: None
 *
 * Returns: None
 *
 * Notes: None
 *
 **********************************************************************/
void gpio_ext_int_init(INT_32 source, INT_32 trigger)
{
	switch (source)
    {
    	case EXT_INT0:
        	RCPC->intconfig &= ~(_BIT(0) | _BIT(1));
            RCPC->intconfig |= _SBF(0,trigger);
            IOCON->mux_ctl_5 &= ~(_BIT(8) | _BIT(9));   
            IOCON->mux_ctl_5 |= IOCON_MUX5_INT0;
            break;
        case EXT_INT1:
            RCPC->intconfig &= ~(_BIT(2) | _BIT(3));
            RCPC->intconfig |= _SBF(2,trigger);
            IOCON->mux_ctl_5 &= ~(_BIT(10) | _BIT(11));   
            IOCON->mux_ctl_5 |= IOCON_MUX5_INT1;
            break;
        case EXT_INT2:
            RCPC->intconfig &= ~(_BIT(4) | _BIT(5));
            RCPC->intconfig |= _SBF(4,trigger);   
            IOCON->mux_ctl_5 &= ~(_BIT(12) | _BIT(13));   
            IOCON->mux_ctl_5 |= IOCON_MUX5_INT2;
            break;
        case EXT_INT3:
            RCPC->intconfig &= ~(_BIT(6) | _BIT(7));
            RCPC->intconfig |= _SBF(6,trigger);
            IOCON->mux_ctl_5 &= ~(_BIT(14) | _BIT(15));   
            IOCON->mux_ctl_5 |= IOCON_MUX5_INT3;
            break;
        case EXT_INT4:
            RCPC->intconfig &= ~(_BIT(8) | _BIT(9));
            RCPC->intconfig |= _SBF(8,trigger);
            IOCON->mux_ctl_3 &= ~(_BIT(0) | _BIT(1));   
            IOCON->mux_ctl_3 |= IOCON_MUX3_INT4;
            break;
        case EXT_INT5:
            RCPC->intconfig &= ~(_BIT(10) | _BIT(11));
            RCPC->intconfig |= _SBF(10,trigger);   
            IOCON->mux_ctl_25 &= ~(_BIT(10) | _BIT(11));   
            IOCON->mux_ctl_25 |= IOCON_MUX25_INT5;
            break;
        case EXT_INT6:
            RCPC->intconfig &= ~(_BIT(12) | _BIT(13));
            RCPC->intconfig |= _SBF(12,trigger);   
            IOCON->mux_ctl_25 &= ~(_BIT(12) | _BIT(13));   
            IOCON->mux_ctl_25 |= IOCON_MUX25_INT6;
            break;
        case EXT_INT7:
            RCPC->intconfig &= ~(_BIT(14) | _BIT(15));
            RCPC->intconfig |= _SBF(14,trigger);   
            IOCON->mux_ctl_25 &= ~(_BIT(14) | _BIT(15));   
            IOCON->mux_ctl_25 |= IOCON_MUX25_INT7;
            break;
        default:
            break;
    }       
	
}

/***********************************************************************
 *
 * Function: gpio_ext_int_clear
 *
 * Purpose: Clear int status bit caused by external int pin
 *
 * Processing:
 *
 * Parameters:
 *     source:  EXT_INT0 to EXT_INT7
 *
 * Outputs: None
 *
 * Returns: None
 *
 * Notes: Clears edge-triggered interrupt status only.
 *
 **********************************************************************/
void gpio_ext_int_clear(INT_32 source)
{
	switch (source)
	{
		case EXT_INT0:
			RCPC->intclear |= _BIT(0);
			break;
		case EXT_INT1:
			RCPC->intclear |= _BIT(1);
			break;
		case EXT_INT2:
			RCPC->intclear |= _BIT(2);
			break;
		case EXT_INT3:
			RCPC->intclear |= _BIT(3);
			break;
		case EXT_INT4:
			RCPC->intclear |= _BIT(4);
			break;
		case EXT_INT5:
			RCPC->intclear |= _BIT(5);
			break;
		case EXT_INT6:
			RCPC->intclear |= _BIT(6);
			break;
		case EXT_INT7:
			RCPC->intclear |= _BIT(7);
			break;
		default:
			break;
	}       
	
}
