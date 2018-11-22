/***************************************************************************//**
 * @file
 * @brief Reset Management Unit (RMU) peripheral module library implementation
 *   for EFM32 devices.
 * @author Energy Micro AS
 * @version 1.1.1
 *******************************************************************************
 * @section License
 * <b>(C) Copyright 2010 Energy Micro AS, http://www.energymicro.com</b>
 *******************************************************************************
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
 ******************************************************************************/

#include "efm32_rmu.h"
#include "efm32_emu.h"
#include "efm32_bitband.h"

/***************************************************************************//**
 * @addtogroup EFM32_Library
 * @{
 ******************************************************************************/

/***************************************************************************//**
 * @addtogroup RMU
 * @brief EFM32 reset management unit utilities.
 * @{
 ******************************************************************************/

/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/

/***************************************************************************//**
 * @brief
 *   Disable/enable the lockup signal from the Cortex M-3.
 *
 * @param[in] disable
 *   @li false - Allow lockup signal to reset.
 *   @li true - Do not allow lockup signal to reset.
 ******************************************************************************/
void RMU_LockupResetDisable(bool disable)
{
  BITBAND_Peripheral(&(RMU->CTRL), _RMU_CTRL_LOCKUPRDIS_SHIFT, (unsigned int)disable);
}


/***************************************************************************//**
 * @brief
 *   Clear the reset cause register.
 ******************************************************************************/
void RMU_ResetCauseClear(void)
{
  uint32_t locked;

  RMU->CMD = RMU_CMD_RCCLR;

  /* Clear some reset causes not cleared with RMU CMD register */
  /* (If EMU registers locked, they must be unlocked first) */
  locked = EMU->LOCK & EMU_LOCK_LOCKKEY_LOCKED;
  if (locked)
  {
    EMU_Unlock();
  }

  BITBAND_Peripheral(&(EMU->AUXCTRL), 0, 1);
  BITBAND_Peripheral(&(EMU->AUXCTRL), 0, 0);

  if (locked)
  {
    EMU_Lock();
  }
}


/***************************************************************************//**
 * @brief
 *   Get the cause of the last reset.
 *
 * @details
 *   In order to be useful, the reset cause must be cleared by SW before a new
 *   reset occurs, otherwise reset causes may accumulate. See
 *   RMU_ResetCauseClear().
 *
 * @return
 *   The reset cause, a bit mask of (typically only one) of:
 *   @li RMU_RSTCAUSE_PORST - Power on reset
 *   @li RMU_RSTCAUSE_BODUNREGRST - Brown out detector, unregulated power
 *   @li RMU_RSTCAUSE_BODREGRST - Brown out detector, regulated power
 *   @li RMU_RSTCAUSE_EXTRST - External reset
 *   @li RMU_RSTCAUSE_WDOGRST - Watchdog reset
 *   @li RMU_RSTCAUSE_LOCKUPRST - Cortex-M3 lockup reset
 *   @li RMU_RSTCAUSE_SYSREQRST - Cortex-M3 system request reset
 ******************************************************************************/
uint32_t RMU_ResetCauseGet(void)
{
  uint32_t ret = RMU->RSTCAUSE;

  /* Inspect and decode bits */
  if (ret & RMU_RSTCAUSE_PORST)
  {
    ret = RMU_RSTCAUSE_PORST;
  }
  else if ((ret & 0x3) == RMU_RSTCAUSE_BODUNREGRST)
  {
    ret = RMU_RSTCAUSE_BODUNREGRST;
  }
  else if ((ret & 0x1F) == RMU_RSTCAUSE_BODREGRST)
  {
    ret = RMU_RSTCAUSE_BODREGRST;
  }
  else if ((ret & 0xB) == RMU_RSTCAUSE_EXTRST)
  {
    ret = RMU_RSTCAUSE_EXTRST;
  }
  else if ((ret & 0x1B) == RMU_RSTCAUSE_WDOGRST)
  {
    ret = RMU_RSTCAUSE_WDOGRST;
  }
  else
    ret &= 0x60;

  return ret;
}


/** @} (end addtogroup RMU) */
/** @} (end addtogroup EFM32_Library) */
