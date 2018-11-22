/**************************************************************************//**
 * @file
 * @brief LCD Controller test and demo
 * @author Energy Micro AS
 * @version 1.2.0
 ******************************************************************************
 * @section License
 * <b>(C) Copyright 2009 Energy Micro AS, http://www.energymicro.com</b>
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
#include <stddef.h>
#include "efm32.h"
#include "lcdcontroller.h"
#include "lcdtest.h"
#include "energymodes.h"
#include "rtc.h"
#include "dvk.h"

/** SysTick polled Delay routine */
extern void Delay(uint32_t dlyTicks);

/** Demo scroll text */
static char *stext = "Energy Micro        ";

/* Joystick press */
#define JOY_MODE_EM3       BC_JOYSTICK_UP
#define JOY_MODE_EM4       BC_JOYSTICK_DOWN
#define JOY_MODE_BUTTON    BC_JOYSTICK_CENTER
#define JOY_MODE_NONE      0x0000

static uint16_t emMode = JOY_MODE_NONE;

/**************************************************************************//**
 * @brief GPIO Interrupt handler
 *****************************************************************************/
void GPIO_EVEN_IRQHandler(void)
{
  GPIO_TypeDef *gpio = GPIO;
  uint16_t     joystick;

  /* Clear interrupt */
  DVK_clearInterruptFlags(BC_INTEN_JOYSTICK);
  gpio->IFC = (1 << 14);

  /* Read and store joystick activity */
  joystick = DVK_getJoystick();

  /* One bit for each direction left/right/up/down + button */
  switch (joystick)
  {
  case JOY_MODE_BUTTON:
    emMode = JOY_MODE_NONE;
    DVK_setLEDs(0x0000);
    LCD_Symbol(LCD, LCD_SYMBOL_PAD0, 0);
    LCD_Symbol(LCD, LCD_SYMBOL_PAD1, 0);
    break;
  case JOY_MODE_EM3:
  case JOY_MODE_EM4:
    emMode = joystick;
    DVK_setLEDs(0xffff);
    LCD_Symbol(LCD, LCD_SYMBOL_PAD0, 1);
    LCD_Symbol(LCD, LCD_SYMBOL_PAD1, 1);
    break;
  default:
    break;
  }
}

/**************************************************************************//**
 * @brief Initialize GPIO interrupt on PC14
 *****************************************************************************/
void GPIO_IRQInit(void)
{
  GPIO_TypeDef *gpio = GPIO;

  /* Configure PC14 as input */
  gpio->P[2].MODEH &= ~(_GPIO_P_MODEH_MODE14_MASK);
  gpio->P[2].MODEH |= GPIO_P_MODEH_MODE14_INPUTPULL;
  gpio->P[2].DOUT  |= (1 << 14);

  /* Set rising edge interrupt */
  gpio->EXTIFALL |= (1 << 14);

  /* Enable interrupt for PC 14 */
  gpio->EXTIPSELH &= ~(_GPIO_EXTIPSELH_EXTIPSEL14_MASK);
  gpio->EXTIPSELH |= (GPIO_EXTIPSELH_EXTIPSEL14_PORTC);
  gpio->IFC        = (1 << 14);
  gpio->IEN       |= (1 << 14);

  NVIC_ClearPendingIRQ(GPIO_EVEN_IRQn);
  NVIC_EnableIRQ(GPIO_EVEN_IRQn);
}


/**************************************************************************//**
 * @brief Callback function lighting the "Antenna symbol"
 *****************************************************************************/
void LCD_trigger(void)
{
  /* Just a few No-OPerations to have a place to put a breakpoint */
  LCD_Symbol(LCD, LCD_SYMBOL_EFM32, 1);
}

/**************************************************************************//**
 * @brief Sleeps in EM1 in given time unless some other IRQ sources has been
 *        enabled
 * @param msec Time in milliseconds
 *****************************************************************************/
void LCD_EM1Sleep(uint32_t msec)
{
  /* Wake us up after msec (or joystick pressed) */
  NVIC_DisableIRQ(LCD_IRQn);
  /* Tell AEM we're in EM1 */
  DVK_setEnergyMode(1);
  RTC_trigger(msec, LCD_trigger);
  EM1_Enter();
  DVK_setEnergyMode(0);
  NVIC_EnableIRQ(LCD_IRQn);
}

/**************************************************************************//**
 * @brief Sleeps in EM2 in given time unless some other IRQ sources has been
 *        enabled
 * @param msec Time in milliseconds
 *****************************************************************************/
void LCD_EM2Sleep(uint32_t msec)
{
  /* Wake us up after msec (or joystick pressed) */
  NVIC_DisableIRQ(LCD_IRQn);
  /* Tell AEM we're in EM2 */
  DVK_setEnergyMode(2);
  RTC_trigger(msec, NULL);
  EM2_Enter();
  DVK_setEnergyMode(0);
  NVIC_EnableIRQ(LCD_IRQn);
}


/**************************************************************************//**
 * @brief Sleeps in EM3 until GPIO interrupt is triggered
 *****************************************************************************/
void LCD_EM3Sleep(void)
{
  /* Tell AEM we're in EM3  */
  DVK_setEnergyMode(3);
  EM3_Enter();
  DVK_setEnergyMode(0);
}

/**************************************************************************//**
 * @brief Sleeps in EM4 until reset
 *****************************************************************************/
void LCD_EM4Sleep(void)
{
  /* Tell AEM we're in EM4 */
  DVK_setEnergyMode(4);
  EM4_Enter();
  /* we will never wake up again here - reset required */
}

/**************************************************************************//**
 * @brief LCD scrolls a text over the display, sort of "polled printf"
 * @param lcd Pointer to LCD register block
 *****************************************************************************/
void LCD_ScrollText(LCD_TypeDef *lcd, char *scrolltext)
{
  int  i, len;
  char buffer[8];

  buffer[7] = 0x00;
  len       = strlen(scrolltext);
  if (len < 7) return;
  for (i = 0; i < (len - 7); i++)
  {
    memcpy(buffer, scrolltext + i, 7);
    LCD_Write(lcd, buffer);
    LCD_EM2Sleep(125);
  }
}

/**************************************************************************//**
 * @brief LCD Blink Test
 * @param lcd Pointer to LCD register block
 *****************************************************************************/
void LCD_BlinkTest(LCD_TypeDef *lcd)
{
  LCD_EnergyMode(lcd, 0, 1);
  LCD_EnergyMode(lcd, 1, 1);
  LCD_EnergyMode(lcd, 2, 1);
  LCD_EnergyMode(lcd, 3, 1);
  LCD_EnergyMode(lcd, 4, 1);

  /* 2 minutes to midnight */
  LCD_Number(lcd, 2358);
  LCD_Symbol(lcd, LCD_SYMBOL_COL10, 1);
  LCD_Symbol(lcd, LCD_SYMBOL_GECKO, 1);
  LCD_Symbol(lcd, LCD_SYMBOL_EFM32, 1);
  LCD_Write(lcd, " EFM32 ");
  lcd->BACTRL |= LCD_BACTRL_BLINKEN;
  while (lcd->SYNCBUSY) ;
  LCD_EM2Sleep(2000);
  LCD_EnergyMode(lcd, 4, 0);
  LCD_EM2Sleep(62);
  LCD_EnergyMode(lcd, 3, 0);
  LCD_EM2Sleep(62);
  LCD_EnergyMode(lcd, 2, 0);
  LCD_EM2Sleep(62);
  LCD_EnergyMode(lcd, 1, 0);
  LCD_EM2Sleep(62);
  LCD_EnergyMode(lcd, 0, 0);
  lcd->BACTRL &= ~LCD_BACTRL_BLINKEN;
  while (lcd->SYNCBUSY) ;
}

/**************************************************************************//**
 * @brief LCD Animate
 * @param lcd Pointer to LCD register block
 *****************************************************************************/
void LCD_AnimateTest(LCD_TypeDef *lcd)
{
  LCD_Write(lcd, "ANIMATE");
  lcd->AREGB   = 0x03;
  lcd->BACTRL  = lcd->BACTRL & ~_LCD_BACTRL_FCTOP_MASK | 1 << _LCD_BACTRL_FCTOP_SHIFT;
  lcd->BACTRL |= LCD_BACTRL_AREGASC_SHIFTLEFT | LCD_BACTRL_AREGBSC_SHIFTLEFT | LCD_BACTRL_ALOGSEL_OR | LCD_BACTRL_AEN | LCD_BACTRL_FCEN;

  LCD_EM2Sleep(5000);

  lcd->AREGA  = 0x00;
  lcd->AREGB  = 0xAA;
  lcd->BACTRL = lcd->BACTRL & ~_LCD_BACTRL_FCTOP_MASK | 5 << _LCD_BACTRL_FCTOP_SHIFT | LCD_BACTRL_BLINKEN;

  LCD_EM2Sleep(5000);

  lcd->BACTRL &= ~LCD_BACTRL_BLINKEN;
}

/**************************************************************************//**
 * @brief LCD Test Routine, shows various text and patterns
 *****************************************************************************/
void LCD_Test(LCD_TypeDef *lcd)
{
  int i, numberOfIterations = 0;

  /* Only enable LE core clock */
  CMU->HFCORECLKEN0 = CMU_HFCORECLKEN0_LE;

  /* Disable all high freq peripherals - except SPI, including USART2/GPIO */
  CMU->HFPERCLKEN0 |= CMU_HFPERCLKEN0_USART2 | CMU_HFPERCLKEN0_GPIO;

  /* Enable interrupts on pushbuttons */
  DVK_clearInterruptFlags(0xffff);
  DVK_enableInterrupt(BC_INTEN_JOYSTICK);
  DVK_setLEDs(0x0000);

  /* Initialize GPIO */
  GPIO_IRQInit();

  /* Initialize RTC module */
  RTC_init();

  /* Loop through funny pattern */
  while (1)
  {
    LCD_AllOff(lcd);
    if (emMode != JOY_MODE_NONE)
    {
      LCD_Symbol(lcd, LCD_SYMBOL_PAD0, 1);
      LCD_Symbol(lcd, LCD_SYMBOL_PAD1, 1);
    }
    else
    {
      for (i = 100; i > 0; i--)
      {
        LCD_Number(lcd, i);
        LCD_EM2Sleep(10);
      }
      LCD_NumberOff(lcd);

      LCD_Symbol(lcd, LCD_SYMBOL_GECKO, 1);
      LCD_Symbol(lcd, LCD_SYMBOL_EFM32, 1);
      LCD_Write(lcd, " Gecko ");
      LCD_EM2Sleep(1000);

      LCD_AllOn(lcd);
      LCD_EM2Sleep(1000);

      LCD_AllOff(lcd);
    }
    if (emMode != JOY_MODE_NONE)
    {
      LCD_Symbol(lcd, LCD_SYMBOL_PAD0, 1);
      LCD_Symbol(lcd, LCD_SYMBOL_PAD1, 1);
    }
    else
    {
      LCD_Write(lcd, "OOOOOOO");
      LCD_EM2Sleep(62);
      LCD_Write(lcd, "XXXXXXX");
      LCD_EM2Sleep(62);
      LCD_Write(lcd, "+++++++");
      LCD_EM2Sleep(62);
      LCD_Write(lcd, "@@@@@@@");
      LCD_EM2Sleep(62);
      LCD_Write(lcd, "ENERGY ");
      LCD_EM2Sleep(250);
      LCD_Write(lcd, "@@ERGY ");
      LCD_EM2Sleep(62);
      LCD_Write(lcd, " @@RGY ");
      LCD_EM2Sleep(62);
      LCD_Write(lcd, " M@@GY ");
      LCD_EM2Sleep(62);
      LCD_Write(lcd, " MI@@Y ");
      LCD_EM2Sleep(62);
      LCD_Write(lcd, " MIC@@ ");
      LCD_EM2Sleep(62);
      LCD_Write(lcd, " MICR@@");
      LCD_EM2Sleep(62);
      LCD_Write(lcd, " MICRO@");
      LCD_EM2Sleep(62);
      LCD_Write(lcd, " MICRO ");
      LCD_EM2Sleep(250);
      LCD_Write(lcd, "-EFM32-");
      LCD_EM2Sleep(250);

      /* Various eye candy */
      LCD_AllOff(lcd);
      if (emMode != JOY_MODE_NONE)
      {
        LCD_Symbol(lcd, LCD_SYMBOL_PAD0, 1);
        LCD_Symbol(lcd, LCD_SYMBOL_PAD1, 1);
      }
      for (i = 0; i < 8; i++)
      {
        LCD_Number(lcd, numberOfIterations + i);
        LCD_ARing(lcd, i, 1);
        LCD_EM2Sleep(20);
      }
      for (i = 0; i < 8; i++)
      {
        LCD_Number(lcd, numberOfIterations + i);
        LCD_ARing(lcd, i, 0);
        LCD_EM2Sleep(100);
      }

      for (i = 0; i < 5; i++)
      {
        LCD_Number(lcd, numberOfIterations + i);
        LCD_Battery(lcd, i);
        LCD_EnergyMode(lcd, i, 1);
        LCD_EM2Sleep(100);
        LCD_EnergyMode(lcd, i, 0);
        LCD_EM2Sleep(100);
      }
      LCD_Symbol(lcd, LCD_SYMBOL_ANT, 1);
      for (i = 0; i < 4; i++)
      {
        LCD_EnergyMode(lcd, i, 1);
        LCD_EM2Sleep(100);
      }
      LCD_Symbol(lcd, LCD_SYMBOL_ANT, 0);
    }
    /* Energy Modes */
    LCD_NumberOff(lcd);
    LCD_Symbol(lcd, LCD_SYMBOL_GECKO, 1);
    LCD_Symbol(lcd, LCD_SYMBOL_EFM32, 1);
    if ((emMode != JOY_MODE_EM3) && (emMode != JOY_MODE_EM4))
    {
      LCD_ScrollText(lcd, "Energy Mode demo, joystick up or down for EM3 and EM4       ");
    }
    LCD_Write(lcd, "  EM0  ");
    LCD_Number(lcd, 0);
    LCD_EnergyMode(lcd, 0, 1);
    LCD_EnergyMode(lcd, 1, 1);
    LCD_EnergyMode(lcd, 2, 1);
    LCD_EnergyMode(lcd, 3, 1);
    LCD_EnergyMode(lcd, 4, 1);
    Delay(4000);
    LCD_Write(lcd, "  EM1  ");
    LCD_Number(lcd, 1111);
    LCD_EnergyMode(lcd, 0, 0);
    LCD_EM1Sleep(4000);
    LCD_Write(lcd, "  EM2  ");
    LCD_Number(lcd, 2222);
    LCD_EnergyMode(lcd, 1, 0);
    LCD_EM2Sleep(4000);

    /* Check if somebody's pressed joystick up */
    if (emMode == JOY_MODE_EM3)
    {
      LCD_ScrollText(lcd, "Going down to EM3, press joystick to wake up    ");
      LCD_Write(lcd, "  EM3  ");
      LCD_Number(lcd, 3333);
      Delay(1000);

      /* Wake up on GPIO interrupt */
      LCD_EM3Sleep();
      LCD_Number(lcd, 0000);
      LCD_Write(lcd, "--EM0--");
      Delay(500);
      LCD_Symbol(lcd, LCD_SYMBOL_PAD0, 0);
      LCD_Symbol(lcd, LCD_SYMBOL_PAD1, 0);
      DVK_setLEDs(0x0000);
      emMode = JOY_MODE_NONE;
    }
    /* Check if somebody's joystick down */
    if (emMode == JOY_MODE_EM4)
    {
      LCD_ScrollText(lcd, "Going down to EM4, press reset to restart    ");
      LCD_Write(lcd, "  EM4  ");
      LCD_Number(lcd, 4444);
      Delay(1000);

      /* Wake up on reset */
      LCD_EM4Sleep();
    }
    LCD_EnergyMode(lcd, 0, 0);
    LCD_EnergyMode(lcd, 1, 0);
    LCD_EnergyMode(lcd, 2, 0);
    LCD_EnergyMode(lcd, 3, 0);
    LCD_EnergyMode(lcd, 4, 0);

    /* Scrolltext */
    LCD_ScrollText(lcd, stext);

    /* Blink and animation featurs */
    LCD_BlinkTest(lcd);
    LCD_AnimateTest(lcd);

    numberOfIterations++;
  }
}

