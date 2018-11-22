/**************************************************************************//**
 * @file
 * @brief Joystick interrupt example for EFM32-Gxxx-DK
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
#include "chip.h"
#include "efm32.h"
#include "efm32_cmu.h"
#include "dvk.h"

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
 * @brief Initialize GPIO interrupt on PC14
 *****************************************************************************/
void GPIO_EVEN_IRQHandler(void)
{
  uint16_t data, joystick;

  /* Clear interrupt */
  data = DVK_getInterruptFlags();
  DVK_clearInterruptFlags(data);

  /* Clear GPIO interrupt */
  GPIO->IFC = (1<<14);

  /* Read joystick status */
  joystick = DVK_getJoystick();

  /* Light up LEDs according to joystick status */
  DVK_setLEDs(joystick);
}

/**************************************************************************//**
 * @brief Initialize GPIO interrupt on PC14
 *****************************************************************************/
void GPIO_IRQInit(void)
{
  GPIO_TypeDef *gpio = GPIO;

  /* Configure PC14 as input with pull up */
  gpio->P[2].MODEH &= ~(_GPIO_P_MODEH_MODE14_MASK);
  gpio->P[2].MODEH |= GPIO_P_MODEH_MODE14_INPUTPULL;
  gpio->P[2].DOUT |= (1 << 14);

  /* Set falling edge interrupt */
  gpio->EXTIFALL |= (1 << 14);

  /* Enable interrupt for PC 14 */
  gpio->EXTIPSELH &= ~(_GPIO_EXTIPSELH_EXTIPSEL14_MASK);
  gpio->EXTIPSELH |= (GPIO_EXTIPSELH_EXTIPSEL14_PORTC);
  gpio->IFC        = (1 << 14);
  gpio->IEN       |= (1 << 14);

  NVIC_EnableIRQ(GPIO_EVEN_IRQn);
}

/**************************************************************************//**
 * @brief Enable DVK FPGA to generate GPIO PC14 trigger on control updates
 *****************************************************************************/
void DVK_IRQInit(void)
{
  /* Enable interrupts on joystick events only */
  DVK_disableInterrupt(0xffff);
  DVK_clearInterruptFlags(0xffff);
  DVK_enableInterrupt(BC_INTEN_JOYSTICK);
}

/**************************************************************************//**
 * @brief  Main function
 *****************************************************************************/
int main(void)
{
  /* Chip errata */
  CHIP_init();

  /* Initialize DVK board register access */
  DVK_init();

  /* Setup SysTick Timer for 1 msec interrupts  */
  if (SysTick_Config(CMU_ClockFreqGet(cmuClock_CORE) / 1000))
  {
    while (1) ;
  }

  /* Initialize DVK interrupt enable */
  DVK_IRQInit();
  /* Initialize GPIO interrupt */
  GPIO_IRQInit();

  /* Turn off LEDs */
  DVK_setLEDs(0x0000);

  while (1)
  {
    /* Wait 5 seconds */
    Delay(5000);
    /* Quick flash to show we're alive */
    DVK_setLEDs(0xffff);
    Delay(20);
    DVK_setLEDs(0x0000);
  }
}
