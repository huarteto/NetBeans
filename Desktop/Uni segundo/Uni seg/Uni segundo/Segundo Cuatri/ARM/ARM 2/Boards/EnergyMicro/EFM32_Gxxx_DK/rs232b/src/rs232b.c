/**************************************************************************//**
 * @file
 * @brief RS232B example for EFM32_G2xx_DK and EFM32_G8xx_DK
 *     Connect a terminal application with baudrate 9600-8-N-1
 *     on serial port B of kit to
 * @author Energy Micro AS
 * @version 1.2.0
 ******************************************************************************
 * @section License
 * <b>(C) Copyright 2010 Energy Micro AS, http://www.energymicro.com</b>
 ******************************************************************************
 *
 * This source code is the property of Energy Micro AS. The source and compiled
 * code may only be used on Energy Micro "EFM32" microcontrollers.
 *
 * This copyright notice may not be removed from the source code nor changed.
 *
 * DISCLAIMER OF WARRANTY/LIMITATION OF REMEDIES: Energy Micro AS has no
 * obligation to support this Software. Energy Micro AS is providing the
 * Software "AS IS", with no express or implied warranties of any kind,
 * including, but not limited to, any implied warranties of merchantability
 * or fitness for any particular purpose or warranties against infringement
 * of any proprietary rights of a third party.
 *
 * Energy Micro AS will not be liable for any consequential, incidental, or
 * special damages, or any other relief, or for any claim by any third party,
 * arising from your use of this Software.
 *
 *****************************************************************************/
#include <stdio.h>
#include "chip.h"
#include "efm32.h"
#include "efm32_cmu.h"
#include "dvk.h"
#include "leuart.h"

volatile uint32_t msTicks; /* counts 1ms timeTicks */
/**************************************************************************//**
 * @brief SysTick_Handler
 * Interrupt Service Routine for system tick counter
 *****************************************************************************/
void SysTick_Handler(void)
{
  msTicks++;       /* increment counter necessary in Delay()*/
}

/**************************************************************************//**
 * @brief Delays number of msTick Systicks (typically 1 ms)
 * @param dlyTicks Number of ticks to delay
 *****************************************************************************/
void Delay(uint32_t dlyTicks)
{
  uint32_t curTicks;

  curTicks = msTicks;
  while ((msTicks - curTicks) < dlyTicks) ;
}

/**************************************************************************//**
 * @brief  Main function
 *****************************************************************************/
int main(void)
{
  int c;

  /* Chip errata */
  CHIP_init();

  /* Initialize DVK board register access */
  DVK_init();

  /* Setup SysTick Timer for 1 msec interrupts  */
  if (SysTick_Config(CMU_ClockFreqGet(cmuClock_CORE) / 1000))
  {
    while (1) ;
  }

  /* Enable EFM32 to access peripheral Serial Port B */
  DVK_enablePeripheral(DVK_RS232B);

  /* 9600 baud, 8 data bits, no parity, 1 stop bit */
  LEUART1_init(9600, 8, 0, 1);

  /* Verify that IAR i/o redirection is working */
  printf("hello, EFM Gecko world\r\n");

  /* Exercise for the reader: Enter Energy Mode 2 here (see emlcd example) */
  /* Be sure to disable SysTick if you don't want it to wake us up */
  while (1)
  {
    /* Wait for interrupt (either LEUART or SysTick ) */
    __WFI();
    /* Echo output back to user */

    /* Retrieve new character */
    c = getchar();
    if (c > 0)
    {
      /* Output character - most terimnals use CRLF */
      if (c == '\r') putchar('\n');
      putchar(c);
    }
  }
}
