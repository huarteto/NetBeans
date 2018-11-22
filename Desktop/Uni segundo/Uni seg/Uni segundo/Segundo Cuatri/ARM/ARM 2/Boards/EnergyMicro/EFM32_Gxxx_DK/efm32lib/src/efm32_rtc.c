/***************************************************************************//**
 * @file
 * @brief Real Time Counter (RTC) peripheral module library implementation for
 *   EFM32 devices.
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

#include "efm32_rtc.h"
#include "efm32_assert.h"
#include "efm32_bitband.h"

/***************************************************************************//**
 * @addtogroup EFM32_Library
 * @{
 ******************************************************************************/

/***************************************************************************//**
 * @addtogroup RTC
 * @brief EFM32 real time counter utilities.
 * @{
 ******************************************************************************/

/*******************************************************************************
 *******************************   DEFINES   ***********************************
 ******************************************************************************/

/** Validation of valid comparator register for assert statements. */
#define RTC_COMP_REG_VALID(reg) (((reg) <= 1))

/** Validation of RTC register block pointer reference for assert statements. */
#define RTC_REF_VALID(ref) ((ref) == RTC)


/*******************************************************************************
 **************************   LOCAL FUNCTIONS   ********************************
 ******************************************************************************/

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */

/***************************************************************************//**
 * @brief
 *   Wait for ongoing sync of register(s) to low frequency domain to complete.
 *
 * @param[in] mask
 *   Bitmask corresponding to SYNCBUSY register defined bits, indicating
 *   registers that must complete any ongoing synchronization.
 ******************************************************************************/
static __INLINE void RTC_Sync(uint32_t mask)
{
  /* Avoid deadlock if modifying the same register twice when freeze mode is */
  /* activated. */
  if (RTC->FREEZE & RTC_FREEZE_REGFREEZE)
    return;

  /* Wait for any pending previous write operation to have been completed */
  /* in low frequency domain */
  while (RTC->SYNCBUSY & mask);
}

/** @endcond (DO_NOT_INCLUDE_WITH_DOXYGEN) */

/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/

/***************************************************************************//**
 * @brief
 *   Set RTC compare register value.
 *
 * @note
 *   The setting of a compare register requires synchronization into the
 *   low frequency domain. If the same register is modified before a previous
 *   update has completed, this function will stall until the previous
 *   synchronization has completed.
 *
 * @param[in] rtc
 *   Pointer to RTC peripheral register block
 *
 * @param[in] comp
 *   Compare register to set, either 0 or 1
 *
 * @param[in] value
 *   Initialization value (<= 0x00ffffff)
 ******************************************************************************/
void RTC_CompareSet(RTC_TypeDef *rtc, unsigned int comp, uint32_t value)
{
  volatile uint32_t *compReg;
  uint32_t syncbusy;

  EFM_ASSERT(RTC_REF_VALID(rtc) &&
             RTC_COMP_REG_VALID(comp) &&
             ((value & ~(_RTC_COMP0_COMP0_MASK >> _RTC_COMP0_COMP0_SHIFT)) == 0));

  /* Initialize selected compare value */
  switch (comp)
  {
  case 0:
    compReg  = &(rtc->COMP0);
    syncbusy = RTC_SYNCBUSY_COMP0;
    break;

  case 1:
    compReg  = &(rtc->COMP1);
    syncbusy = RTC_SYNCBUSY_COMP1;
    break;

  default:
    /* Unknown compare register selected, abort */
    return;
  }

  /* LF register about to be modified require sync. busy check */
  RTC_Sync(syncbusy);

  *compReg = value;
}


/***************************************************************************//**
 * @brief
 *   Get RTC counter value.
 *
 * @param[in] rtc
 *   Pointer to RTC peripheral register block.
 *
 * @return
 *   Current RTC counter value.
 ******************************************************************************/
uint32_t RTC_CounterGet(RTC_TypeDef *rtc)
{
  EFM_ASSERT(RTC_REF_VALID(rtc));

  return(rtc->CNT);
}


/***************************************************************************//**
 * @brief
 *   Enable/disable RTC.
 *
 * @note
 *   The enabling/disabling of the RTC modifies the RTC CTRL register which
 *   requires synchronization into the low frequency domain. If this register is
 *   modified before a previous update to the same register has completed, this
 *   function will stall until the previous synchronization has completed.
 *
 * @param[in] rtc
 *   Pointer to RTC peripheral register block.
 *
 * @param[in] enable
 *   true to enable counting, false to disable.
 ******************************************************************************/
void RTC_Enable(RTC_TypeDef *rtc, bool enable)
{
  EFM_ASSERT(RTC_REF_VALID(rtc));

  /* LF register about to be modified require sync. busy check */
  RTC_Sync(RTC_SYNCBUSY_CTRL);

  BITBAND_Peripheral(&(rtc->CTRL), _RTC_CTRL_EN_SHIFT, (unsigned int)enable);
}


/***************************************************************************//**
 * @brief
 *   RTC register synchronization freeze control.
 *
 * @details
 *   Some RTC registers requires synchronization into the low frequency (LF)
 *   domain. The freeze feature allows for several such registers to be
 *   modified before passing them to the LF domain simultaneously (which
 *   takes place when the freeze mode is disabled).
 *
 * @note
 *   When enabling freeze mode, this function will wait for all current
 *   ongoing RTC synchronization to LF domain to complete (Normally
 *   synchronization will not be in progress.) However for this reason, when
 *   using freeze mode, modifications of registers requiring LF synchronization
 *   should be done within one freeze enable/disable block to avoid unecessary
 *   stalling.
 *
 * @param[in] rtc
 *   Pointer to RTC peripheral register block.
 *
 * @param[in] enable
 *   @li true - enable freeze, modified registers are not propagated to the
 *       LF domain
 *   @li false - disables freeze, modified registers are propagated to LF
 *       domain
 ******************************************************************************/
void RTC_FreezeEnable(RTC_TypeDef *rtc, bool enable)
{
  EFM_ASSERT(RTC_REF_VALID(rtc));

  if (enable)
  {
    /*
     * Wait for any ongoing LF synchronization to complete. This is just to
     * protect against the rare case when a user
     * - modifies a register requiring LF sync
     * - then enables freeze before LF sync completed
     * - then modifies the same register again
     * since modifying a register while it is in sync progress should be
     * avoided.
     */
    while (rtc->SYNCBUSY);

    rtc->FREEZE = RTC_FREEZE_REGFREEZE;
  }
  else
  {
    rtc->FREEZE = 0;
  }
}


/***************************************************************************//**
 * @brief
 *   Initialize RTC.
 *
 * @details
 *   Note that the compare values must be set separately with RTC_CompareSet().
 *   That should probably be done prior to the use of this function if
 *   configuring the RTC to start when initialization is completed.
 *
 * @note
 *   The initialization of the RTC modifies the RTC CTRL register which requires
 *   synchronization into the low frequency domain. If this register is
 *   modified before a previous update to the same register has completed, this
 *   function will stall until the previous synchronization has completed.
 *
 * @param[in] rtc
 *   Pointer to RTC peripheral register block.
 *
 * @param[in] init
 *   Pointer to RTC initialization structure.
 ******************************************************************************/
void RTC_Init(RTC_TypeDef *rtc, const RTC_Init_TypeDef *init)
{
  uint32_t tmp;

  EFM_ASSERT(RTC_REF_VALID(rtc));

  if (init->enable)
    tmp = RTC_CTRL_EN;
  else
    tmp = 0;

  /* Configure DEBUGRUN flag, sets whether or not counter should be
   * updated when debugger is active */
  if (init->debugRun)
  {
    tmp |= RTC_CTRL_DEBUGRUN;
  }

  /* Configure COMP0TOP, this will use the COMP0 compare value as an
   * overflow value, instead of default 24-bit 0x00ffffff */
  if (init->comp0Top)
  {
    tmp |= RTC_CTRL_COMP0TOP;
  }

  /* LF register about to be modified require sync. busy check */
  RTC_Sync(RTC_SYNCBUSY_CTRL);

  rtc->CTRL = tmp;
}


/** @} (end addtogroup RTC) */
/** @} (end addtogroup EFM32_Library) */
