/**************************************************************************//**
 * @file
 * @brief Blink example for EFM32_G2xx_DK and EFM32_G8xx_DK
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
#include "dvk.h"
#include "usart.h"

/** USART1 Input buffer size */
#define ECHOBUFSIZE    80
/** USART1 Input buffer */
char echoBuffer[ECHOBUFSIZE];

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
  int c, index;

  /* Chip errata */
  CHIP_init();

  /* Initialize DVK board register access */
  DVK_init();
  DVK_enablePeripheral(DVK_RS232A);

  /* Initialize USART */
  USART1_init();

  /* Verify that IAR i/o redirection is working */
  printf("\r\nEFM32 USART1 example\r\n");

  /* Exercise for the reader: Enter Energy Mode 2 here (see emlcd example) */
  /* Be sure to disable SysTick if you don't want it to wake us up */
  index = 0;
  while (1)
  {
    /* Retrieve new character */
    c = getchar();
    if (c > 0)
    {
      /* Output character - most terimnals use CRLF */
      if (c == '\r')
      {
        echoBuffer[index] = '\0';
        /* Output entire line
         *
         * For now - just run prints
         * TBD: DMA it back while in EM1 or EM2
         */
        printf("\r\n%s\r\n", echoBuffer);
        index = 0;
      }
      else
      {
        /* Filter non-printable characters */
        if (c < ' ' || c > '~') continue;

        /* Enter into buffer */
        echoBuffer[index] = c & 0xff;
        index++;
        if (index == ECHOBUFSIZE)
        {
          /* Flush buffer */
          index = 0;
        }
        /* Local echo */
        putchar(c);
      }
    }
    else
    {
      /* Clear SLEEPDEEP for EM1 - wait for interrupt */
      SCB->SCR &= ~(1 << SCB_SCR_SLEEPDEEP_Pos);
      __WFI();
    }
  }
}
