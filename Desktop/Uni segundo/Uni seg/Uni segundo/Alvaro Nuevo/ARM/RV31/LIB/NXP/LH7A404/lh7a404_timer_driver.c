/***********************************************************************
 * $Id:: lh7a404_timer_driver.c 14 2007-08-28 16:08:18Z kevinw         $
 *
 * Project: LH7A404 timer driver
 *
 * Description:
 *     This file contains driver support for the timer modules on the
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

#include "lh7a404_timer_driver.h"

/***********************************************************************
 * Timer clock frequencies
 **********************************************************************/

/* Timer 3 clock source */
#define TIMER3_CLK      (CLOCK_MAINOSC / 4)
/* Timers 1 and 2 2KHz clock source */
#define TIMER12_CLK2K   (CLOCK_MAINOSC / 7395)
/* Timers 1 and 2 508KHz clock source */
#define TIMER12_CLK508K (CLOCK_MAINOSC / 29)

/* Number of microSeconds in a second */
#define USEC_PER_SEC 1000000
#define MSEC_PER_USEC 1000

/***********************************************************************
 * Timer driver private data and types
 **********************************************************************/

/* Timer device configuration structure type */
typedef struct
{
    BOOL_32 init;         /* Device initialized flag */
    TIMER_REGS_T *regptr; /* Pointer to Timer registers */
} TIMER_CFG_T;

/* Timer device configuration structure */
STATIC TIMER_CFG_T timer1cfg, timer2cfg, timer3cfg;

/***********************************************************************
 * Timer driver private functions
 **********************************************************************/

/***********************************************************************
 *
 * Function: timer_set_delay
 *
 * Purpose: Set timer delay (undeflow) value in microSeconds.
 *
 * Processing:
 *     TIMER3 is clocked differently from TIMER1 and TIMER2, so treat
 *     each separately. For TIMER1 or TIMER2, first convert the time in
 *     microseconds to counts per 508 kHz clock tick. If the time
 *     interval is too long to be obtained using the 508 kHz clock,
 *     convert the time in microseconds to counts per 2 kHz clock tick.
 *     If the time interval is too long to be obtained using the 2 KHz
 *     clock, set the count interval to the maximum possible count.
 *
 *     For TIMER3, convert the time in microseconds to counts per
 *     (roughly) 7.4MHz clock tick. If the time interval is too long to
 *     be obtained using TIMER3, set the count to the maximum possible
 *     count.
 *
 * Parameters: 
 *     timer      : Must be TIMER1, TIMER2 or TIMER3
 *     time_in_us : The desired delay in microseconds
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
STATIC void timer_set_delay(TIMER_REGS_T *timer,
                            UNS_32 time_in_us)
{
    INT_64 count;
    UNS_32 tmp;
   
    if ((timer == TIMER1) || (timer == TIMER2))
    {
        /* Check if timing is possible with a 508KHz clock first */
        tmp = timer->control & ~(TIMER_CTRL_2K | TIMER_CTRL_508K);
        count = ((((INT_64)TIMER12_CLK508K * (INT_64)time_in_us) /
            (INT_64)USEC_PER_SEC)) - 1;

        /* Is the count value to large for the timer load word? */
        if (count > (INT_64)TIMER_MAXCOUNT)
        {
            /* Use the 2KHz timer instead */
            count = ((((INT_64)TIMER12_CLK2K * (INT_64)time_in_us) /
                (INT_64)USEC_PER_SEC)) - 1;
            timer->control = tmp | (TIMER_CTRL_2K |
                TIMER_CTRL_PERIODIC);
        }
        else
        {
            timer->control = tmp | (TIMER_CTRL_508K |
                TIMER_CTRL_PERIODIC);
        }

        timer->load = (UNS_32)(TIMER_LOAD(count));
    }
    else if (timer == TIMER3)
    {
        count = ((((INT_64)TIMER3_CLK * (INT_64)time_in_us) /
            (INT_64)USEC_PER_SEC)) - 1;
        timer->load = (UNS_32)(TIMER_LOAD(count));
    }
}

/***********************************************************************
 * Timer driver public functions
 **********************************************************************/

/***********************************************************************
 *
 * Function: timer_open
 *
 * Purpose: Open the timer
 *
 * Processing:
 *     Determine the timer configuration structure to use based on the
 *     passed arg value. If the arg value doesn't match an available
 *     timer, return NULL to the caller. Otherwise, check the status of
 *     the init flag. If it is TRUE, return NULL to the caller.
 *     Otherwise, set init to TRUE and save the pointer to the timer
 *     registers. Disable the timer and set the timer base clock to
 *     2KHz (Timer 3 only supports 7.4MHz) and freerunning mode. Clear
 *     any pending timer interrupts. If this is timer 1, set the BZTOG
 *     output low. Return the pointer to the UART config structure to
 *     the caller.
 *
 * Parameters:
 *     ipbase: Pointer to a timer peripheral block
 *     arg   : Not used
 *
 * Outputs: None
 *
 * Returns: The pointer to a timer config structure or NULL
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 timer_open(void *ipbase, INT_32 arg)
{
    TIMER_CFG_T *timercfg;

    if ((TIMER_REGS_T *) ipbase == TIMER1)
    {
        /* Timer 1 selected */
        timercfg = &timer1cfg;
    }
    else if ((TIMER_REGS_T *) ipbase == TIMER2)
    {
        /* Timer 2 selected */
        timercfg = &timer2cfg;
    }
    else if ((TIMER_REGS_T *) ipbase == TIMER3)
    {
        /* Timer 3 selected */
        timercfg = &timer3cfg;
    }
    else
    {
        /* Invalid timer */
        timercfg = (TIMER_CFG_T *) NULL;
    }

    if (timercfg != (TIMER_CFG_T *) NULL)
    {
        /* Valid timer selected, has it been previously initialized? */
        if (timercfg->init == FALSE)
        {
            /* Device not initialized and it usable, so set it to
               used */
            timercfg->init = TRUE;

            /* Save address of register block */
            timercfg->regptr = (TIMER_REGS_T *) ipbase;

            /* Set default timer state to disabled, 2KHz clock - timer
               3 only supports 7.37MHz and the extra bits do nothing */
            timercfg->regptr->control = (TIMER_CTRL_DISABLE |
                TIMER_CTRL_FREERUN | TIMER_CTRL_2K);

            /* Clear pending interrupt */
            timercfg->regptr->clear = 0x00000000;

            /* Disable buzzer on timer 1 only (TBUZ driven low) */
            if ((TIMER_REGS_T *) ipbase == TIMER1)
            {
                BZCONT = 0x00000000;
            }
        }
    }

    return (INT_32) timercfg;
}

/***********************************************************************
 *
 * Function: timer_close
 *
 * Purpose: Close the timer
 *
 * Processing:
 *     If init is not TRUE, then return _ERROR to the caller as the
 *     device was not previously opened. Otherwise, disable the timer,
 *     set init to FALSE, and return _NO_ERROR to the caller.
 *
 * Parameters:
 *     devid: Pointer to timer config structure
 *
 * Outputs: None
 *
 * Returns: The status of the close operation
 *
 * Notes: None
 *
 **********************************************************************/
STATUS timer_close(INT_32 devid)
{
    TIMER_CFG_T *timercfgptr = (TIMER_CFG_T *) devid;
    STATUS status = _ERROR;

    if (timercfgptr->init == TRUE)
    {
        /* Disable timer */
        timercfgptr->regptr->control = TIMER_CTRL_DISABLE;

        /* Set timer as uninitialized */
        timercfgptr->init = FALSE;

        /* Successful operation */
        status = _NO_ERROR;
    }

    return status;
}

/***********************************************************************
 *
 * Function: timer_ioctl
 *
 * Purpose: Timer configuration block
 *
 * Processing:
 *     This function is a large case block. Based on the passed function
 *     and option values, set or get the appropriate timer parameter.
 *
 * Parameters:
 *     devid: Pointer to timer config structure
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
STATUS timer_ioctl(INT_32 devid,
                   INT_32 cmd,
                   INT_32 arg)
{
    TIMER_REGS_T *timerregs;
    TIMER_CFG_T *timercfgptr = (TIMER_CFG_T *) devid;
    STATUS status = _ERROR;

    if (timercfgptr->init == TRUE)
    {
        status = _NO_ERROR;
        timerregs = timercfgptr->regptr;

        switch (cmd)
        {
            case TIMER_ENABLE:
                 if ((INT_32) arg == 1)
                 {
                     /* Enable timer */
                     timerregs->control |= TIMER_CTRL_ENABLE;
                 }
                 else
                 {
                     /* Disable timer */
                     timerregs->control &= ~TIMER_CTRL_ENABLE;
                 }
                 break;

            case TIMER_MODE:
                if ((INT_32) arg == TIMER_FREERUN)
                {
                    /* Set timer to freerun mode */
                    timerregs->control &= ~TIMER_CTRL_PERIODIC;
                }
                else if ((INT_32) arg == TIMER_PERIODIC)
                {
                    /* Set timer to periodic mode */
                    timerregs->control |= TIMER_CTRL_PERIODIC;
                }
                break;

            case TIMER_CLOCK:
                /* This command is not valid on timer 3 */
                if (timerregs == TIMER3)
                {
                    status = SMA_BAD_CLK;
                }
                else
                {
                    if ((INT_32) arg == TIMER_CLOCK_2K)
                    {
                        /* Set timer clock to 2KHz */
                        timerregs->control &= ~TIMER_CTRL_508K;
                    }
                    else if ((INT_32) arg == TIMER_CLOCK_508K)
                    {
                        /* Set timer clock to 508KHz */
                        timerregs->control |= TIMER_CTRL_508K;
                    }
                    else
                    {
                        /* Not a valid clock configuration */
                        status = SMA_BAD_CLK;
                    }
                }
                break;

            case TIMER_SET_COUNT:
                /* Set timer count value */
                timerregs->load = TIMER_LOAD((INT_32) arg);
                break;

            case TIMER_SET_USECS:
                /* Set the timer to the passed time in uSecs */
                timer_set_delay(timerregs, (UNS_32) arg);
                break;

            case TIMER_INT_CLEAR:
                /* Clear the timer interrupt */
                timerregs->clear = 0;
                break;

            case TIMER_SET_BZCONT:
                /* Set the state of the BZTOG output */
                switch ((INT_32) arg)
                {
                    case TIMER_BZTOG_LOW:
                        /* Drive BZTOG low */
                        BZCONT = 0;
                        break;

                    case TIMER_BZTOG_HIGH:
                        /* Drive BZTOG high */
                        BZCONT = BZCONT_BZTOG;
                        break;

                    case TIMER_BZTOG_TOGGLE:
                        /* Drive BZTOG with timer 1 underflow */
                        BZCONT = BZCONT_BZMOD;
                        break;

                    default:
                        /* Invalid argument */
                        status = SMA_BAD_PARAMS;
                        break;
                }
                break;

            case TIMER_GET_STATUS:
                /* Return a timer status */
                switch (arg)
                {
                    case TIMER_ENABLE_ST:
                        /* Return timer enabled status */
                        if ((timerregs->control &
                            TIMER_CTRL_ENABLE) != 0)
                        {
                            /* Timer is enabled */
                            status = 1;
                        }
                        else
                        {
                            /* Timer is disabled */
                            status = 0;
                        }
                        break;

                    case TIMER_MODE_ST:
                        /* Return timer mode */
                        if ((timerregs->control &
                            TIMER_CTRL_PERIODIC) != 0)
                        {
                            /* Periodic timer mode */
                            status = TIMER_PERIODIC;
                        }
                        else
                        {
                            /* Freerunning timer mode */
                            status = TIMER_FREERUN;
                        }
                        break;

                    case TIMER_CLOCK_ST:
                        /* If this is timer 3, just return with an
                           error status, as timer 3 does not have a
                           programmable clock */
                        if (timerregs == TIMER3)
                        {
                            status = _ERROR;
                        }
                        else if ((timerregs->control &
                            TIMER_CTRL_508K) != 0)
                        {
                            /* 508KHz timer clock */
                            status = TIMER_CLOCK_508K;
                        }
                        else
                        {
                            /* 2KHz timer clock */
                            status = TIMER_CLOCK_2K;
                        }
                        break;

                    case TIMER_COUNT_ST:
                        /* Return the timer load value */
                        status = TIMER_LOAD(timerregs->load);
                        break;

                    case TIMER_VALUE_ST:
                        /* Return the current timer value */
                        status = TIMER_LOAD(timerregs->value);
                        break;

                    case TIMER_BZCONT_ST:
                        /* Return BZTOG output state */
                        if ((BZCONT & BZCONT_BZMOD) != 0)
                        {
                            /* Timer 1 underflow controls state */
                            status = TIMER_BZTOG_TOGGLE;
                        }
                        else if ((BZCONT & BZCONT_BZTOG) != 0)
                        {
                            /* State is driven high */
                            status = TIMER_BZTOG_HIGH;
                        }
                        else
                        {
                            /* State is driven low */
                            status = TIMER_BZTOG_LOW;
                        }
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
 * Function: timer_read
 *
 * Purpose: Timer read function (stub only)
 *
 * Processing:
 *     Return 0 to the caller.
 *
 * Parameters:
 *     devid:     Pointer to timer descriptor
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
INT_32 timer_read(INT_32 devid,
                  void *buffer,
                  INT_32 max_bytes)
{
    return 0;
}

/***********************************************************************
 *
 * Function: timer_write
 *
 * Purpose: Timer write function (stub only)
 *
 * Processing:
 *     Return 0 to the caller.
 *
 * Parameters:
 *     devid:   Pointer to timer descriptor
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
INT_32 timer_write(INT_32 devid,
                   void *buffer,
                   INT_32 n_bytes)
{
    return 0;
}

/*********************************************************************** 
 * 
 * Function: timer_wait_us
 * 
 * Purpose: Delay for usec microSeconds 
 * 
 * Processing:
 *     If the timer argument is a valid timer, stop the timer and use
 *     the timer_set_delay function to program the timer underflow to
 *     the needed delay and then restart the timer. Monitor the raw
 *     timer count value until the value underflows. Stop the timer and
 *     then clear the pending timer interrupt.
 * 
 * Parameters: 
 *     timer : Must be TIMER1, TIMER2 or TIMER3
 *     usec  : the delay time in microseconds
 * 
 * Outputs: None
 * 
 * Returns: Nothing
 * 
 * Notes: 
 *     This function is intended for simple non-interruptable delays
 *     with accurate timing. Using this function will change the
 *     settings of the timer the function is used with. For very small
 *     times with a low CPU and bus clock speed, the timing may be
 *     inaccurate. Use with care.
 * 
 **********************************************************************/
void timer_wait_us(TIMER_REGS_T *timer,
                   UNS_32 usec)
{
    UNS_16 count, count2;

    if ((timer == TIMER1) || (timer == TIMER2) || (timer == TIMER3))
    {
        timer->control = TIMER_CTRL_DISABLE;
        timer_set_delay(timer, usec);

        /* Start timer */
        timer->control |= TIMER_CTRL_ENABLE;

        /* Wait for timer to underflow */
        count2 = (UNS_16) timer->value;
        count = count2;
        while (count == count2)
        {
            count2 = (UNS_16) timer->value;
            if (count > count2)
            {
                count = count2;
            }
        }

        timer->control = TIMER_CTRL_DISABLE;
        timer->clear = 0;
    }
}

/***********************************************************************
 *
 * Function: timer_wait_ms
 *
 * Purpose:
 *     Wait for the specified number of milliseconds
 *
 * Processing:
 *     Call timer_wait_us() with a value of 1000 usec for the number of
 *     times specified by the msec argument.
 *
 * Parameters:
 *     timer: Must be one of TIMER1, TIMER2 or TIMER3
 *     msec:   the delay time in milliseconds
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void timer_wait_ms(TIMER_REGS_T *timer, UNS_32 msec)
{
    UNS_32 elapsed_ms;

    for (elapsed_ms = 0; elapsed_ms < msec; elapsed_ms++)
    {
        timer_wait_us(timer, MSEC_PER_USEC);
    }
}
