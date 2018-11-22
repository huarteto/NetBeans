/**************************************************************************//**
 * @file
 * @brief Core clocking example for EFM32_G8xx_DK
 * @details
 *   Allows user to configure CPU Core Clock oscillator and Frequency
 *   with joystick with LCD status, as well as EM0-EM2.
 *
 * @par Usage
 * @li Joystick Up and Down selects core clock oscillator and HFRCO band,
 *   (LFRCO/LFXO/HFRCO/HFXO)
 * @li Joystick Left and Right selects the core clock divisor, in 2^n steps
 * @li Joystick Push selects energy mode
 *
 * @note
 *   This example requires BSP version 1.0.6 or later.
 *
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

#include <string.h>
#include <stdbool.h>
#include "chip.h"
#include "efm32.h"
#include "efm32_emu.h"
#include "efm32_cmu.h"
#include "efm32_dbg.h"
#include "efm32_gpio.h"
#include "dvk.h"
#include "lcdcontroller.h"

/** Interrupt pin used to detect joystick activity */
#define GPIO_INT_PIN 14

/** Minimum allowed core clock in Hz to inhibit becoming totally unresponsive */
#define CORE_CLOCK_MIN 8000

/** HFClock oscillator selection table */
static const cmuSelect_TypeDef oscSelectorTable[] =
{
  cmuSelect_LFRCO,
  cmuSelect_LFXO,
  cmuSelect_HFRCO,
  cmuSelect_HFXO
};

/** HFClock oscillator selection name table (must match oscSelectorTable) */
static const char *oscSelectorNameTable[] =
{
  "LFRC",
  " LFX",
  "HFRC",
  " HFX",
};

/** HFRCO band selection table */
static const cmuHFRCOBand_TypeDef bandSelectorTable[] =
{
  cmuHFRCOBand_1MHz,
  cmuHFRCOBand_7MHz,
  cmuHFRCOBand_11MHz,
  cmuHFRCOBand_14MHz,
  cmuHFRCOBand_21MHz,
  cmuHFRCOBand_28MHz
};

/** Core clock DIV factor table */
static const cmuClkDiv_TypeDef clockDivTable[] =
{
  cmuClkDiv_1,
  cmuClkDiv_2,
  cmuClkDiv_4,
  cmuClkDiv_8,
  cmuClkDiv_16,
  cmuClkDiv_32,
  cmuClkDiv_64,
  cmuClkDiv_128,
  cmuClkDiv_256,
  cmuClkDiv_512
};

/* Current config variables */
static int oscIndex = 2; /* Set to HW reset default in oscSelectorTable */
static int bandIndex = 3; /* Set to HW reset default in bandSelectorTable */
static int divIndex = 0; /* Set to HW reset default in clockDivTable */
static int eMode = 0;

/**************************************************************************//**
 * @brief GPIO Interrupt handler
 * This interrupt handler is not an example of good design, as it will do
 * a lot of operations inside the interrupt handler.
 *****************************************************************************/
void GPIO_EVEN_IRQHandler(void)
{
  uint16_t joystick;
  uint32_t freq;

  /* Switch to HFRCO high frequency in order to finish interrupt as quickly */
  /* as possible. HFRCO takes almost no time to start if disabled. This will */
  /* cause energy consumption to go up during interrupt, but it improves */
  /* joystick usage. */ 
  CMU_ClockSelectSet(cmuClock_HF, cmuSelect_HFRCO);
  CMU_HFRCOBandSet(cmuHFRCOBand_28MHz);
  /* Important: Divisor must be 1 for handling towards end of this code */
  CMU_ClockDivSet(cmuClock_CORE, cmuClkDiv_1);

  /* Clear interrupt */
  DVK_clearInterruptFlags(BC_INTEN_JOYSTICK);
  GPIO_IntClear(1 << GPIO_INT_PIN);

  /* LEDs on to indicate joystick used */
  DVK_setLEDs(0xffff);

  /* Read and store joystick activity - wait for key release */
  joystick = DVK_getJoystick();
  while (DVK_getJoystick());

  /* LEDs off to indicate joystick release */
  DVK_setLEDs(0x0000);

  /* LEFT/RIGHT selects core clock divisor */
  if (joystick & BC_JOYSTICK_LEFT)
  {
    divIndex--;

    if (divIndex < 0)
    {
      divIndex = (sizeof(clockDivTable) / sizeof(cmuClkDiv_TypeDef)) - 1;
    }
  }

  if (joystick & BC_JOYSTICK_RIGHT)
  {
    divIndex++;

    if (divIndex >= (sizeof(clockDivTable) / sizeof(cmuClkDiv_TypeDef)))
    {
      divIndex = 0;
    }
  }

  /* UP/DOWN selects core oscillator/HFRCO band */
  if (joystick & BC_JOYSTICK_UP)
  {
    /* If HFRCO selected, toggle band */
    if (oscSelectorTable[oscIndex] == cmuSelect_HFRCO)
    {
      bandIndex++;

      if (bandIndex >= (sizeof(bandSelectorTable) / sizeof(cmuHFRCOBand_TypeDef)))
      {
        bandIndex = 0;
        oscIndex++;
      }
    }
    else
    {
      bandIndex = 0;
      oscIndex++;
    }

    if (oscIndex >= (sizeof(oscSelectorTable) / sizeof(cmuSelect_TypeDef)))
    {
      oscIndex = 0;
    }
  }

  if (joystick & BC_JOYSTICK_DOWN)
  {
    /* If HFRCO selected, toggle band */
    if (oscSelectorTable[oscIndex] == cmuSelect_HFRCO)
    {
      bandIndex--;

      if (bandIndex < 0)
      {
        bandIndex = 5;
        oscIndex--;
      }
    }
    else
    {
      bandIndex = 5;
      oscIndex--;
    }

    if (oscIndex < 0)
    {
      oscIndex = (sizeof(oscSelectorTable) / sizeof(cmuSelect_TypeDef)) - 1;
    }
  }

  /* Push selects EM */
  if (joystick & BC_JOYSTICK_CENTER)
  {
    eMode++;

    if (eMode > 2)
    {
      eMode = 0;
    }
  }

  /* Set to selected core clock */
  CMU_ClockSelectSet(cmuClock_HF, oscSelectorTable[oscIndex]);
  CMU_HFRCOBandSet(bandSelectorTable[bandIndex]);

  /* Divisor must be 1 in HW for below check to work; */
  /* has already been set to 1 at start of this function. */

  /* Make sure not going below min defined core clock in this app. */
  freq = CMU_ClockFreqGet(cmuClock_CORE);
  while (((freq >> divIndex) < CORE_CLOCK_MIN) && divIndex)
  {
    divIndex--;
  }

  /* Disable unused oscillators (not LFXO - used by LCD) */
  if (oscSelectorTable[oscIndex] != cmuSelect_HFXO)
    CMU_OscillatorEnable(cmuOsc_HFXO, false, false);
  if (oscSelectorTable[oscIndex] != cmuSelect_HFRCO)
    CMU_OscillatorEnable(cmuOsc_HFRCO, false, false);
  if (oscSelectorTable[oscIndex] != cmuSelect_LFRCO)
    CMU_OscillatorEnable(cmuOsc_LFRCO, false, false);

  /* Set divisor, which may have been auto-adjusted to avoid too low core clock */
  CMU_ClockDivSet(cmuClock_CORE, clockDivTable[divIndex]);
}


/**************************************************************************//**
 * @brief Initialize GPIO interrupt for joystick (ie FPGA signal)
 *****************************************************************************/
void clocksIRQInit(void)
{
  /* Configure interrupt pin as input with pull-up */
  GPIO_PinModeSet(GPIO_PortC, GPIO_INT_PIN, GPIO_ModeInputPull, 1);

  /* Set falling edge interrupt and clear/enable it */
  GPIO_IntConfig(GPIO_PortC, GPIO_INT_PIN, false, true, true);

  NVIC_ClearPendingIRQ(GPIO_EVEN_IRQn);
  NVIC_EnableIRQ(GPIO_EVEN_IRQn);
}

/**************************************************************************//**
 * @brief Update LCD with selected config
 *****************************************************************************/
void clocksUpdateLCD(void)
{
  char lcdtext[8];
  int i;
  uint32_t freq;
  uint32_t div;

  /* Show Gecko if debugger is attached. Energy modes do not behave as */
  /* expected when using the debugger. */
  if (DBG_Connected())
  {
    LCD_Symbol(LCD_SYMBOL_GECKO, 1);
  }
  else
  {
    LCD_Symbol(LCD_SYMBOL_GECKO, 0);
  }

  freq = CMU_ClockFreqGet(cmuClock_CORE);

  /* Selected clock source */
  strcpy(lcdtext, oscSelectorNameTable[oscIndex]);

  /* Choose to display Hz/Khz or Mhz according to speed */
  if (freq < 10000)
  {
    strcat(lcdtext, " Hz");
  }
  else if ((freq >= 10000) && (freq < 10000000))
  {
    freq = freq / 1000;
    strcat(lcdtext, "kHz");
  }
  else if (freq >= 10000000)
  {
    freq = freq / 1000000;
    strcat(lcdtext, "MHz");
  }
  LCD_Write(lcdtext);

  /* Output clock source and frequency on LCD */
  LCD_Number(freq);

  /* Light "Ring" according to div factor */
  div = CMU_ClockDivGet(cmuClock_CORE);
  for (i = 0; i < 8; i++)
  {
    if (div > i)
    {
      LCD_ARing(i, 1);
    }
    else
    {
      LCD_ARing(i, 0);
    }
  }

  /* Show energy mode selected when idle */
  if (eMode < 1)
  {
    LCD_EnergyMode(0, 1);
  }
  else
  {
    LCD_EnergyMode(0, 0);
  }

  if (eMode < 2)
  {
    LCD_EnergyMode(1, 1);
  }
  else
  {
    LCD_EnergyMode(1, 0);
  }

  if (eMode < 3)
  {
    LCD_EnergyMode(2, 1);
  }
  else
  {
    LCD_EnergyMode(2, 0);
  }

  LCD_EnergyMode(3, 1);
  LCD_EnergyMode(4, 1);
}

/**************************************************************************//**
 * @brief  Main function
 *****************************************************************************/
int main(void)
{
  /* Chip errata */
  CHIP_init();

  /* Rev A errata handling for EM2. Must enable DMA clock in order for EM2 */
  /* to work. This will be fixed in Rev B chip, and not needed for Rev B an later. */
  CMU_ClockEnable(cmuClock_DMA, true);

  /* Initialize DVK board register access */
  DVK_init();
  DVK_setLEDs(0xaaaa);

  /* Initialize LCD controller without boost */
  LCD_Init(false);

  /* When clock rate changes, the interrupt rate will go up and down */
  /* since we don't adjust after the new settings */  
  LCD_AllOff();

  LCD_Symbol(LCD_SYMBOL_COL5, 1);

  /* Enable board control interrupts */
  DVK_disableInterrupt(0xffff);
  DVK_clearInterruptFlags(0xffff);
  DVK_enableInterrupt(BC_INTEN_JOYSTICK);
  clocksIRQInit();
  DVK_setLEDs(0xc003);

  /* Main loop - just update LCD */
  while (1)
  {
    /* Update LCD display with current settings */
    clocksUpdateLCD();

    /* Enter EM as selected */
    switch (eMode)
    {
    case 1:
      EMU_EnterEM1();
      break;

    case 2:
      EMU_EnterEM2(true);
      break;

    default: /* EM0 */
      break;
    }
  }
}
