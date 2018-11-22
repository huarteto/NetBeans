/***********************************************************************
 * $Id:: lh79524_adc_driver.c 27 2007-08-31 19:35:06Z kevinw           $
 *
 * Project: LH79524 ADC driver
 *
 * Description:
 *     This file contains driver support for the ADC module on the
 *     LH79524
 *
 * Notes:
 *     The number of different configurations supported by the ADC
 *     is beyond the scope of this driver. This driver provides the
 *     following basic functions:
 *         Sequenced analog to digital conversions (polled, interrupt)
 *         Touchscreen conversions (polled, interrupt)
 *         Pendown state detect to touchscreen conversions (interrupt)
 *         Brownout interrupt handling (interrupt only)
 *         ADC status polling and general configuration
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
 *********************************************************************/

#include "lh79524_adc_driver.h"
#include "lh79524_rcpc.h"
#include "lh79524_iocon.h"

/***********************************************************************
* ADC driver private data and types
**********************************************************************/

/* Size of the ADC receive ring buffer */
#define ADC_RING_BUFSIZE 64

/* Function prototype used for polled and interrupt driven reads */
typedef INT_32 (*ADC_RFUNC_T) (void *, void *, INT_32);

/* ADC device configuration structure type */
typedef struct
{
  BOOL_32 init;       /* Device initialized flag */
  ADC_REGS_T *regptr; /* Pointer to ADC registers */
  UNS_16 rx[ADC_RING_BUFSIZE];  /* ADC data ring buffer */
  INT_32 rx_head;     /* ADC ring buffer head index */
  INT_32 rx_tail;     /* ADC ring buffer tail index */
  UNS_16 conv_count;  /* Number of conversions to perform */
  PFV bro_cbfunc;     /* Brown interrupt callback function */
  PFV pd_cbfunc;      /* Pendown interrupt completion callback
												 function */
  PFV eos_cbfunc;     /* End of sequence interrupt callback
												 function */
  UNS_32 adc_clk_source; /* clock coming to ADC block from RCPC */
} ADC_CFG_T;

/* ADC device configuration structure */
static ADC_CFG_T adccfg;

/***********************************************************************
* ADC driver private functions
**********************************************************************/
/*********************************************************************** 
* 
* Function: adc_get_clock
* 
* Purpose: 
*  Return the ADC's time base in Hz
* 
* Processing: 
*  Compute the time base using the system clock and the divider.
* 
* Parameters: 
*     adccfgptr:     Pointer to ADC config structure
* 
* Outputs: None. 
* 
* Returns: 
*  the actual base frequency in Hz or _ERROR if the frequency is 
*  out of range for the converter or the system clock.
* 
* Notes: None
* 
**********************************************************************/ 
static UNS_32 adc_get_clock(ADC_CFG_T* adccfgptr)
{
  UNS_32 freq;        
  
  freq = adccfgptr->adc_clk_source /
    (1 << ADC_CLKSEL_GET(adccfgptr->regptr->adcpcr));
  
  if (freq < ADC_MIN_CLK || freq > ADC_MAX_CLK)
    return 0;
  
  return freq;
}

/*********************************************************************** 
* 
* Function: adc_set_clock
* 
* Purpose: 
*  Set the ADC's time base
* 
* Processing: 
*  Use the system clock and the divider to get as close as possible
*  without going outside the min and max operating frequency of the
*  ADC.
* 
* Parameters: 
*  freq:				the desired base frequency in Hz
*  adccfgptr:   Pointer to ADC config structure
* 
* Outputs: None. 
* 
* Returns: 
*  the actual base frequency in Hz or _ERROR if a legal frequency is
*  not obtainable using the system clock.
* 
* Notes: 
*  If the requested frequency is too high, the maximum frequency
*  is returned. If the requested frequency is too low, the minimum
*  frequency is returned. If the requested frequency, even after
*  clipping it to be in range, cannot be generated using the system
*  clock, then this function returns _ERROR. This implies that the
*  system clock is too high or too low to use the ADC.
* 
**********************************************************************/ 
static UNS_32 adc_set_clock(UNS_32 freq, ADC_CFG_T* adccfgptr)
{
  /* clk is the freq. of the clock that drives the ADC block, in Hz */
  INT_32 clk;
  INT_32 divisor_idx;      /* the power of 2 of the current divisor */
  INT_32 best_divisor_idx; /* the value of divisor_idx for best_delta*/
  INT_32 divisor;          /* 1 << divisor_idx*/
  INT_32 new_freq;         /* clk / divisor*/
  INT_32 delta;            /* abs(new_freq - freq)*/
  INT_32 best_delta;       /* the smallest value of delta found so far*/
  UNS_32 temp;             /* intermediate value of the PC register*/
  /* max_clk is the highest value of freq the ADC can handle */
  INT_32 max_clk = (ADC_MAX_CLK < adccfgptr->adc_clk_source) ? 
                    ADC_MAX_CLK : adccfgptr->adc_clk_source;
  
  if (freq > max_clk)
    freq = max_clk;
  if (freq < ADC_MIN_CLK)
    freq = ADC_MIN_CLK;
  
  clk = adccfgptr->adc_clk_source;
  
  best_divisor_idx = 0;
  best_delta = freq - clk;
  if (best_delta < 0)
    best_delta = -best_delta;
  
  for (divisor_idx = 0; 
  divisor_idx < (1 << ADC_CLKSEL_WIDTH); 
  divisor_idx++)
  {
    divisor = (1 << divisor_idx);
    new_freq = clk / divisor;
    
    if (new_freq >= ADC_MIN_CLK && new_freq <= ADC_MAX_CLK)
    {
      delta = freq - new_freq;
      if (delta < 0)
        delta = -delta;
      
      if (delta < best_delta)
      {
        best_delta = delta;
        best_divisor_idx = divisor_idx;
      }
    }
  }
  
  /* do logic in temp to prevent multiple writes to ADC->pcr */
  temp = (adccfgptr->regptr->adcpcr & ~ADC_CLKSEL_MASK);
  temp |= ADC_CLKSEL_SET(best_divisor_idx);
  adccfgptr->regptr->adcpcr = temp;
  
  return adc_get_clock(adccfgptr);
}

/*********************************************************************** 
* 
* Function: adc_time_2_clock
* 
* Purpose: 
*  Convert usec delay time to ADC clocks.
* 
* Processing: 
*  Compute the number of ADC clocks equivalent to the time passed.
* 
* Parameters: 
*  adccfgptr:   Pointer to ADC config structure
*  time:				time duration to be converted in to ADC clocks
* 
* Outputs:  
* 
* Returns: 
*  The number of ADC clocks equivalent to the time passed. 
* 
* Notes: None
* 
**********************************************************************/ 
static UNS_32 adc_time_2_clock(ADC_CFG_T* adccfgptr, UNS_32 time)
{
  UNS_32 adc_clcks = 0;        
  
  if(time > 0)
  {
    /* calculate the settling time clocks */
    adc_clcks = adc_get_clock(adccfgptr) / 
      (1000000 / time);
    if(adc_clcks > _BIT(ADC_SETTIME_WIDTH))
      adc_clcks = _BITMASK(ADC_SETTIME_WIDTH);
  }
  
  return adc_clcks;
}

/***********************************************************************
*
* Function: adc_default
*
* Purpose: Places the ADC interface and controller in a default state
*
* Processing:
*     Disable ADC interrupts, set ADC power control to off (idle) with
*     maximum clock divider and 1 conversion, set general configuration
*     to continuous conversion mode. Set up each MUX input to maximum
*     clock with default references. Set up idle high and low control
*     words. Clear the ADC FIFO and clear any latched interrupts.
*
* Parameters:
*     adcregs: Pointer to an ADC register set
*     xtal_in: On board clock osillator frequency. If 0 the ADC clock
*							 settings are not changed.
*     sel_SCOC: Specifies whether to use XTAL_IN as ADC clock source
*
* Outputs: 
*
* Returns: 
*  ADC clock frequency. 
*
* Notes: None
*
**********************************************************************/
static UNS_32 adc_default(ADC_REGS_T *adcregs, UNS_32 xtal_in, 
                          BOOL_32 sel_SCOC)
{
  INT_32 idx;
  UNS_32 temp;
  UNS_32 adc_clock_source;
  
  if(xtal_in)
  {
    /* setup and enable ADC clock in RCPC registers */
    RCPC->adcclkprescale = RCPC_PRESCALER_DIV1;
    if(sel_SCOC == TRUE)
    {
      RCPC->pclksel1 |= RCPC_PCLKSEL1_ADC_SCOC;
      /* since we set ADC predivisor to 1 ADC clock source equal to 
      System oscilator - XTAL_IN */
      adc_clock_source = xtal_in;
    }
    else
    {  
      /* select HCLK as ADC clock source */
      RCPC->pclksel1 &= ~RCPC_PCLKSEL1_ADC_SCOC;
      /* since we set ADC predivisor to 1 ADC clock source equal to 
      HCLK*/
      adc_clock_source = RCPC_GET_HCLK(xtal_in);
    }
   
    /* enable clock to ADC block */
    RCPC->periphclkctrl1 &= ~RCPC_CLKCTRL1_ADC_DISABLE;
  }
  
  
  /* enable ADC pins in IOCON registers */
  temp = IOCON->mux_ctl_25;
  temp &= (IOCON_MUX_MASK(IOCON_RES25_AN2_PJ3) & 
    IOCON_MUX_MASK(IOCON_RES25_AN3_PJ0) &
    IOCON_MUX_MASK(IOCON_RES25_AN4_PJ1) &
    IOCON_MUX_MASK(IOCON_RES25_AN5_PJ5_INT5) &
    IOCON_MUX_MASK(IOCON_RES25_AN6_PJ7_INT7) &
    IOCON_MUX_MASK(IOCON_RES25_AN7_PJ6_INT6) &
    IOCON_MUX_MASK(IOCON_RES25_AN8_PJ4) &
    IOCON_MUX_MASK(IOCON_RES25_AN9_PJ2));
  
  temp |= IOCON_MUX25_AN2_LL | IOCON_MUX25_AN3_LR |
    IOCON_MUX25_AN4_WIPER | IOCON_MUX25_AN5 |
    IOCON_MUX25_AN6 | IOCON_MUX25_AN7 | IOCON_MUX25_AN8 |
    IOCON_MUX25_AN9;
  
  IOCON->mux_ctl_25 = temp;
  
  /* Setup default state of ADC as idle, with all channels set up
  in a safe configuration, and all interrupts masked */
  /* Mask all interrupts */
  adcregs->adcie = 0;
  
  /* Setup mode as disabled, 1 conversion in a sequence, and
  maximum ADC clock divider */
  adcregs->adcpcr = (ADC_NOC_SET(1) | ADC_PWM_ALL_OFF |
    ADC_CLKSEL_DIV128);
  
    /* Set conversion mode to continuous with a single FIFO entry
  watermark level */
  adcregs->adcgencfg = (ADC_SSM_SSB | ADC_WM_SET(1));
  
  /* Set up each control bank register to use a linear analog
  input mapped array with maximum settling time */
  for (idx = 0; idx <= 15; idx++)
  {
    /* High control word */
    adcregs->adchicl[idx] = (ADC_SETTIME(0x1FF) |
      ADC_INPSEL(idx) | ADC_INM_GND | ADC_REF_VREFP);
    
    /* Low control word */
    adcregs->adclocl[idx] = ADC_REF_VREFN;
  }
  
  /* Setup idle mode high and low words using analog input 0
  as the default idle input */
  adcregs->adcidlehi = (ADC_SETTIME(0x1FF) | ADC_INP_AN0 |
    ADC_INM_GND | ADC_REF_VREFP);
  adcregs->adcidlelo = ADC_REF_VREFP;
  
  /* Make sure the FIFO is clear */
  while ((adcregs->adcfifosts & ADC_FIFO_EMPTY) == 0)
  {
    idx = (INT_32) adcregs->adcresults;
  }
  
  /* Clear pending ADC interrupts */
  adcregs->adcic = (ADC_EOS_CLR | ADC_PEN_CLR | ADC_BROWNOUT_CLR);
  
  return adc_clock_source;
}

/***********************************************************************
*
* Function: adc_add_sequence_entry
*
* Purpose: Adds a new sample entry to the sequence list
*
* Processing:
*     If the number of existing conversion sequence entries is greater
*     than 16, then return _ERROR to the caller. Otherwise, generate
*     the high and low sequence control words and place them into the
*     the sequence register array indexed by conv_count. Set status to
*     conv_count and increment conv_count by one. Return conv_count to
*     the caller.
*
* Parameters:
*     adc_ch_cfgptr: Pointer to a channel configuration structure
*     adccfgptr:     Pointer to ADC config structure
*
* Outputs: None
*
* Returns: A key to the new sequence entry (0 to 15)
*
* Notes: None
*
**********************************************************************/
static STATUS adc_add_sequence_entry(ADC_CH_CONV_T *adc_ch_cfgptr,
                                     ADC_CFG_T *adccfgptr)
{
  UNS_32 tmphi, tmplo;
  STATUS status = _ERROR;
  
  /* Limit conversion count to 16 conversions */
  if (adccfgptr->conv_count < 16)
  {
  /* Build the high and low control words based on the passed
    channel configuration values */
    tmphi = (ADC_SETTIME(adc_time_2_clock(adccfgptr, adc_ch_cfgptr->settle_time))
      | ADC_INPSEL(adc_ch_cfgptr->inpc) 
      | ADC_INNSEL(adc_ch_cfgptr->innc) 
      | ADC_REFP_SEL(adc_ch_cfgptr->vrefp));
    
    /* Build and save the control bank low word register */
    tmplo = (ADC_REFN_SEL(adc_ch_cfgptr->vrefn) |
      adc_ch_cfgptr->bias);
    
    /* Write registers */
    adccfgptr->regptr->adchicl[adccfgptr->conv_count] = tmphi;
    adccfgptr->regptr->adclocl[adccfgptr->conv_count] = tmplo;
    
    /* Update the number of conversions in the sequence */
    tmphi = adccfgptr->regptr->adcpcr & ~ADC_NOC_MASK;
    adccfgptr->regptr->adcpcr = (tmphi | adccfgptr->conv_count);
    
    /* Save key return value */
    status = (STATUS) adccfgptr->conv_count;
    
    /* Increment conversion count value */
    adccfgptr->conv_count++;
  }
  
  return status;
}

/***********************************************************************
*
* Function: adc_chg_sequence_entry
*
* Purpose: Change an existing sequence entry
*
* Processing:
*     If the sequence key (in the ADC_CH_CONV_T structure) is not
*     within the limits of the converion list, return _ERROR to the
*     caller. Otherwise, generate the new high and low configuration
*     words for the channel indicated by the sequence key and update
*     the registers in the ADC for that channel. Return _NO_ERROR to
*     the caller.
*
* Parameters:
*     adc_ch_cfgptr: Pointer to a channel configuration structure
*     adccfgptr:     Pointer to ADC config structure
*
* Outputs: None
*
* Returns: _ERROR is the sequence key is invalid, otherwise _NO_ERROR.
*
* Notes: None
*
**********************************************************************/
static STATUS adc_chg_sequence_entry(ADC_CH_CONV_T *adc_ch_cfgptr,
                                     ADC_CFG_T *adccfgptr)
{
  UNS_32 tmphi, tmplo;
  STATUS status = _ERROR;
  
  /* Make sure the passed sequence entry is valid */
  if (adc_ch_cfgptr->chkey <= adccfgptr->conv_count)
  {
    /* Key is valid */
    
    /* Build the high and low control words based on the passed
    channel configuration values */
    tmphi = (ADC_SETTIME(adc_time_2_clock(adccfgptr, adc_ch_cfgptr->settle_time))
      | ADC_INPSEL(adc_ch_cfgptr->inpc) 
      | ADC_INNSEL(adc_ch_cfgptr->innc) 
      | ADC_REFP_SEL(adc_ch_cfgptr->vrefp));
    
    /* Build and save the control bank low word register */
    tmplo = (ADC_REFN_SEL(adc_ch_cfgptr->vrefn) |
      adc_ch_cfgptr->bias);
    
    /* Write registers */
    adccfgptr->regptr->adchicl[adc_ch_cfgptr->chkey] = tmphi;
    adccfgptr->regptr->adclocl[adc_ch_cfgptr->chkey] = tmplo;
    
    status = _NO_ERROR;
  }
  
  return status;
}

/***********************************************************************
*
* Function: adc_ring_fill
*
* Purpose: Move data from the ADC FIFO to the driver ring buffer
*
* Processing:
*     While the ADC FIFO is not empty, copy an entry from the ADC
*     FIFO into the ADC driver ring buffer. Increment the ring buffer
*     head pointer. If it exceeds the ring buffer size, reset the
*     head pointer to 0.
*
* Parameters:
*     adccfgptr: Pointer to ADC config structure
*
* Outputs: None
*
* Returns: Nothing
*
* Notes: None
*
**********************************************************************/
static void adc_ring_fill(ADC_CFG_T *adccfgptr)
{
  ADC_REGS_T *adcregsptr = (ADC_REGS_T *) adccfgptr->regptr;
  
  /* Continue stuffing the receive ring FIFO until the receive
  FIFO is empty */
  while ((adcregsptr->adcfifosts & ADC_FIFO_EMPTY) == 0)
  {
    adccfgptr->rx[adccfgptr->rx_head] =
      (UNS_16) adcregsptr->adcresults;
    
    /* Increment receive ring buffer head pointer */
    adccfgptr->rx_head++;
    if (adccfgptr->rx_head >= ADC_RING_BUFSIZE)
    {
      adccfgptr->rx_head = 0;
    }
  }
}

/*********************************************************************** 
* 
* Function: adc_setup_4wire_touchscreen
* 
* Purpose: 
*  Set up the sequencer for a 4-wire touch screen measurement.
* 
* Processing: 
*  Initialize the adc to its default state.
*  Map all touchscreen pins to ADC input
*  Set up the idle step to prevent false pen auto-triggering.
*  Flush the result FIFO.
*  Clear the pen and end of sequence interrupts.
*  Set up the sequence as follows:
*      0. Strong pullup on X+ pin, other pins float.
*         Measure on pin x+. This measurement will be discarded.
*         This step de-glitches any partial triggers.
*      1. Weak pullup on X+ pin, X- floats, Y- to Ground and neg
*         ref, Y+ floats (ratiometric).
*         Measure on X+ pin. This measurement is used to verify
*         the pen was down at the beginning of the coordinate
*         measurement.
*      2. X+ and X- float, Y- to Ground and neg
*         ref, Y+ to VDDA_ADC and positive reference (ratiometric).
*         Measure on pin x+. This is the X measurement.
*      3. Repeat the X measurement 3 more times (we will average
*         4 measurements to improve signal-to-noise.
*      4. Y+ and Y- float, X- to Ground and neg
*         ref, X+ to VDDA_ADC and positive reference (ratiometric).
*         Measure on pin Y+. This is the Y measurement.
*      5. Repeat the Y measurement 3 more times (we will average
*         4 measurements to improve signal-to-noise.
*      6. Strong pullup on X+ pin, other pins float.
*         Measure on pin x+. This measurement will be discarded.
*         This step de-glitches any partial triggers.
*      7. Weak pullup on X+ pin, X- floats, Y- to Ground and neg
*         ref, Y+ floats (ratiometric).
*         Measure on X+ pin. This measurement is used to verify
*         the pen was down at the end of the coordinate
*         measurement. 
* 
* Parameters: 
*  tsc_cfg:				Pointer to touch screen config structure
*  adccfgptr:     Pointer to ADC config structure
* 
* Outputs: None. 
* 
* Returns: Nothing
*
* Notes: 
*  This ADC configuration is intended to be used in conjunction
*  with an end of sequence interrupt handler that follows the following
*  algorithm
*      1. Read and discard the first measurement in the result FIFO
*      2. Read the 2nd result. If the measured voltage is below a
*         threashhold, then the pen is down, otherwise it is up. If
*         the pen is up, read and discard the remaining measurements;
*         then return from interrupt.
*      3. Pen is down. Read and average the next 4 results as the X
*         coordinate.
*      4. Read and average the next 4 results as the Y coordinate
*         measurement.
*      5  Read and discard the next result.
*      6. Read the last FIFO result. If the measured voltage is
*         below a threashhold, then the pen is still down, otherwise
*         it is up. If the pen is up, return from interrupt. If the
*         pen is still down, report the new X and Y measurements.
*
*  There are sevreal ways to trigger measurements. 
*  The recomended way that is more power efficient is to set up a a pen
*  interrupt handler that switches the ADC module for pen-triggered
*  to soft_triggered measurements. Initially, the ADC is set up
*  for pen-triggered measurements. On pen down, the pen interrupt
*  handler switches the ADC to software-triggered measurements.
*  The system polls the touch screen periodically 
*  until the end of sequence interrupt handler detects pen up. 
*  The handler then switches the ADC back to pen-triggered measurements.
* 
**********************************************************************/ 
static STATUS adc_setup_4wire_touchscreen(ADC_TSC_CONV_T* tsc_cfg, 
                                          ADC_CFG_T *adccfgptr)
{
  ADC_CH_CONV_T step; /* a step in the sequence*/
  ADC_REGS_T *adcregsptr = adccfgptr->regptr;
  UNS_32 tmp;
  ADC_VREFP_T pen_detect_p = ADC_REFP_VREF;
  ADC_VREFN_T pen_detect_n = ADC_REFN_VREF;
  
  while ((adcregsptr->adcfifosts & ADC_FIFO_EMPTY) == 0)
  {
    tmp = adcregsptr->adcresults;
  }
  
  adcregsptr->adcic = (ADC_EOS_CLR | ADC_PEN_CLR);
  /* Empty ring buffer and set conversion counter
  to 0 */
  adccfgptr->rx_head = adccfgptr->rx_tail = 0;
  adccfgptr->conv_count = 0;

  /* It is only necessary to turn on the reference buffer if the on-chip
  reference is used as the positive reference for a measurement. If
  an external reference is used (e.g. for ratiometric measurements),
  then the reference buffer can be off (saves power) */
  if(tsc_cfg->vrefp_x != ADC_REFP_VN8)
  {
    adcregsptr->adcpcr |= ADC_REF_ENABLE;
  }
  else
  {
    pen_detect_p = tsc_cfg->vrefp_x;
    pen_detect_n = tsc_cfg->vrefn_x;
  }
  
  /* Build and save the idle channel step */
  tmp = (ADC_SETTIME(adc_time_2_clock(adccfgptr, tsc_cfg->settle_time)) |
    ADC_INPSEL(ADC_ANALOG_1) |
    ADC_INNSEL(ADC_REFN_VREFN) |
    ADC_REFP_SEL(pen_detect_p));
  adcregsptr->adcidlehi = tmp;
  
  tmp = (ADC_REFN_SEL(pen_detect_n) |
    (ADC_FET_AN0_VDDA1K | ADC_FET_AN3_GND));
  adcregsptr->adcidlelo = tmp;
  
  /* for rest of the steps keep settling time same */
  step.settle_time = tsc_cfg->settle_time;
  
  /* Build step 0 - prior to 1st pen-down check*/
  step.inpc = ADC_ANALOG_0;
  step.innc = ADC_REFN_VREFN;
  step.vrefp = pen_detect_p; 
  step.vrefn = pen_detect_n; 
  step.bias = ADC_FET_AN0_VDDA;
  adc_add_sequence_entry(&step, adccfgptr);
  
  /* Build step 1 - check pen down state prior to coords*/
  step.inpc = ADC_ANALOG_0;
  step.innc = ADC_REFN_VREFN;
  step.vrefp = pen_detect_p; 
  step.vrefn = pen_detect_n; 
  step.bias = (ADC_FET_AN0_VDDA1K | ADC_FET_AN3_GND);
  adc_add_sequence_entry(&step, adccfgptr);
  
  /* Build step 2,3,4 & 5 - measure x coords*/
  for(tmp = 0; tmp < 4; tmp++)
  {
    step.inpc = ADC_ANALOG_2;
    step.innc = ADC_REFN_VREFN;
    step.vrefp = tsc_cfg->vrefp_x; 
    step.vrefn = tsc_cfg->vrefn_x; 
    step.bias = (ADC_FET_AN1_GND | ADC_FET_AN0_VDDA);
    adc_add_sequence_entry(&step, adccfgptr);
  }
  
  /* Build step 6,7,8 & 9 - measure y coords*/
  for(tmp = 0; tmp < 4; tmp++)
  {
    step.inpc = ADC_ANALOG_0;
    step.innc = ADC_REFN_VREFN;
    step.vrefp = tsc_cfg->vrefp_y; 
    step.vrefn = tsc_cfg->vrefn_y; 
    step.bias = (ADC_FET_AN3_GND | ADC_FET_AN2_VDDA);
    adc_add_sequence_entry(&step, adccfgptr);
  }
  
  /* Build step 10 - prior to last pen-down check*/
  step.inpc = ADC_ANALOG_0;
  step.innc = ADC_REFN_VREFN;
  step.vrefp = pen_detect_p; 
  step.vrefn = pen_detect_n; 
  step.bias = ADC_FET_AN0_VDDA;
  adc_add_sequence_entry(&step, adccfgptr);
  
  /* Build step 11 - prior to last pen-down check*/
  step.inpc = ADC_ANALOG_0;
  step.innc = ADC_REFN_VREFN;
  step.vrefp = pen_detect_p; 
  step.vrefn = pen_detect_n; 
  step.bias = (ADC_FET_AN0_VDDA1K | ADC_FET_AN3_GND);
  adc_add_sequence_entry(&step, adccfgptr);
  
  
  return _NO_ERROR;
}

/*********************************************************************** 
* 
* Function: adc_setup_5wire_touchscreen
* 
* Purpose: 
*  Set up the sequencer for a 5-wire touch screen measurement.
* 
* Processing: 
*  Initialize the adc to its default state.
*  Map all touchscreen pins to ADC input
*  Set up the idle step to prevent false pen auto-triggering.
*  Flush the result FIFO.
*  Clear the pen and end of sequence interrupts.
*  Set up the sequence as follows:
*      0. Strong pullup on UL pin, other pins float.
*         Measure on pin x+. This measurement will be discarded.
*         This step de-glitches any partial triggers.
*      1. Weak pullup on UL pin, wiper to VSSA_ADC, other pins float.
*         Measure on UL pin. This measurement is used to verify
*         the pen was down at the beginning of the coordinate
*         measurement.
*      2. UL and LL to VDDA_ADC. UR and LR to VSSA_ADC.
*         UL to positive reference and LR to negative reference
*         (ratiometric). Wiper floats.
*         Measure on pin Wiper. This is the X measurement.
*      3. Repeat the X measurement 3 more times (we will average
*         4 measurements to improve signal-to-noise.
*      4. UL and UR to VDDA_ADC. LL and LR to VSSA_ADC.
*         UL to positive reference and LR to negative reference
*         (ratiometric). Wiper floats.
*         Measure on pin Wiper. This is the Y measurement.
*      5. Repeat the Y measurement 3 more times (we will average
*         4 measurements to improve signal-to-noise.
*      6. Strong pullup on UL pin, other pins float.
*         Measure on pin UL. This measurement will be discarded.
*         This step de-glitches any partial triggers.
*      7. Weak pullup on UL pin, wiper to ground.
*         Measure on X+ pin. This measurement is used to verify
*         the pen was down at the end of the coordinate
*         measurement. 
* 
* Parameters: 
*  tsc_cfg:				Pointer to touch screen config structure
*  adccfgptr:     Pointer to ADC config structure
* 
* Outputs: None. 
* 
* Returns: Nothing
*
* Notes: 
*  This ADC configuration is intended to be used in conjunction
*  with an end of sequence interrupt handler that follows the following
*  algorithm
*      1. Read and discard the first measurement in the result FIFO
*      2. Read the 2nd result. If the measured voltage is below a
*         threashhold, then the pen is down, otherwise it is up. If
*         the pen is up, read and discard the remaining measurements;
*         then return from interrupt.
*      3. Pen is down. Read and average the next 4 results as the X
*         coordinate.
*      4. Read and average the next 4 results as the Y coordinate
*         measurement.
*      5  Read and discard the next result.
*      6. Read the last FIFO result. If the measured voltage is
*         below a threashhold, then the pen is still down, otherwise
*         it is up. If the pen is up, return from interrupt. If the
*         pen is still down, report the new X and Y measurements.
*
*  There are sevreal ways to trigger measurements. 
*  The recomended way that is more power efficient is to set up a a pen
*  interrupt handler that switches the ADC module for pen-triggered
*  to soft_triggered measurements. Initially, the ADC is set up
*  for pen-triggered measurements. On pen down, the pen interrupt
*  handler switches the ADC to software-triggered measurements.
*  The system polls the touch screen periodically 
*  until the end of sequence interrupt handler detects pen up. 
*  The handler then switches the ADC back to pen-triggered measurements.
*
**********************************************************************/ 
static STATUS adc_setup_5wire_touchscreen(ADC_TSC_CONV_T* tsc_cfg, 
                                          ADC_CFG_T *adccfgptr)
{
  ADC_CH_CONV_T step; /* a step in the sequence*/
  ADC_REGS_T *adcregsptr = adccfgptr->regptr;
  UNS_32 tmp;
  ADC_VREFP_T pen_detect_p = ADC_REFP_VREF;
  ADC_VREFN_T pen_detect_n = ADC_REFN_VREF;
  
  while ((adcregsptr->adcfifosts & ADC_FIFO_EMPTY) == 0)
  {
    tmp = adcregsptr->adcresults;
  }
  
  adcregsptr->adcic = (ADC_EOS_CLR | ADC_PEN_CLR);
  /* Empty ring buffer and set conversion counter
  to 0 */
  adccfgptr->rx_head = adccfgptr->rx_tail = 0;
  adccfgptr->conv_count = 0;
  
  /* It is only necessary to turn on the reference buffer if the on-chip
  reference is used as the positive reference for a measurement. If
  an external reference is used (e.g. for ratiometric measurements),
  then the reference buffer can be off (saves power) */
  if(tsc_cfg->vrefp_x != ADC_REFP_VN8)
  {
    adcregsptr->adcpcr |= ADC_REF_ENABLE;
  }
  else
  {
    pen_detect_p = tsc_cfg->vrefp_x;
    pen_detect_n = tsc_cfg->vrefn_x;
  }
  
  /* Build and save the idle channel step */
  tmp = (ADC_SETTIME(adc_time_2_clock(adccfgptr, tsc_cfg->settle_time)) |
    ADC_INPSEL(ADC_ANALOG_1) |
    ADC_INNSEL(ADC_REFN_VREFN) |
    ADC_REFP_SEL(ADC_REFP_VREF));
  adcregsptr->adcidlehi = tmp;
  
  tmp = (ADC_REFN_SEL(ADC_REFN_VREF) |
    (ADC_FET_AN0_VDDA1K | ADC_FET_AN4_GND));
  adcregsptr->adcidlelo = tmp;
  
  /* for rest of the steps keep settling time same */
  step.settle_time = tsc_cfg->settle_time;
  
  /* Build step 0 - prior to 1st pen-down check*/
  step.inpc = ADC_ANALOG_0;
  step.innc = ADC_REFN_VREFN;
  step.vrefp = pen_detect_p;
  step.vrefn = pen_detect_n;
  step.bias = (ADC_FET_AN0_VDDA | ADC_FET_AN4_GND);
  adc_add_sequence_entry(&step, adccfgptr);
  
  /* Build step 1 - check pen down state prior to coords*/
  step.inpc = ADC_ANALOG_0;
  step.innc = ADC_REFN_VREFN;
  step.vrefp = pen_detect_p;
  step.vrefn = pen_detect_n;
  step.bias = (ADC_FET_AN0_VDDA1K | ADC_FET_AN4_GND);
  adc_add_sequence_entry(&step, adccfgptr);
  
  /* Build step 2,3,4 & 5 - measure x coords*/
  for(tmp = 0; tmp < 4; tmp++)
  {
    step.inpc = ADC_ANALOG_4;
    step.innc = ADC_REFN_VREFN;
    step.vrefp = tsc_cfg->vrefp_x; 
    step.vrefn = tsc_cfg->vrefn_x; 
    step.bias = (ADC_FET_AN0_VDDA | ADC_FET_AN2_VDDA 
      | ADC_FET_AN3_GND | ADC_FET_AN1_GND);
    adc_add_sequence_entry(&step, adccfgptr);
  }
  
  /* Build step 6,7,8 & 9 - measure y coords*/
  for(tmp = 0; tmp < 4; tmp++)
  {
    step.inpc = ADC_ANALOG_4;
    step.innc = ADC_REFN_VREFN;
    step.vrefp = tsc_cfg->vrefp_y; 
    step.vrefn = tsc_cfg->vrefn_y; 
    step.bias = (ADC_FET_AN0_VDDA | ADC_FET_AN1_VDDA 
      | ADC_FET_AN3_GND | ADC_FET_AN2_GND);
    adc_add_sequence_entry(&step, adccfgptr);
  }
  
  /* Build step 10 - prior to last pen-down check*/
  step.inpc = ADC_ANALOG_0;
  step.innc = ADC_REFN_VREFN;
  step.vrefp = pen_detect_p;
  step.vrefn = pen_detect_n;
  step.bias = ADC_FET_AN0_VDDA | ADC_FET_AN4_GND;
  adc_add_sequence_entry(&step, adccfgptr);
  
  /* Build step 11 - prior to last pen-down check*/
  step.inpc = ADC_ANALOG_0;
  step.innc = ADC_REFN_VREFN;
  step.vrefp = pen_detect_p;
  step.vrefn = pen_detect_n;
  step.bias = (ADC_FET_AN0_VDDA1K | ADC_FET_AN3_GND);
  adc_add_sequence_entry(&step, adccfgptr);
  
  /* It is only necessary to turn on the reference buffer if the on-chip
  reference is used as the positive reference for a measurement. If
  an external reference is used (e.g. for ratiometric measurements),
  then the reference buffer can be off (saves power) */
  if(tsc_cfg->vrefp_x != ADC_REFP_VN8)
    adcregsptr->adcpcr |= ADC_REF_ENABLE;
  
  return _NO_ERROR;
}

/***********************************************************************
* ADC driver public functions
**********************************************************************/

/***********************************************************************
*
* Function: adc_open
*
* Purpose: Open the ADC
*
* Processing:
*     If init is not FALSE, return 0x00000000 to the caller. Otherwise,
*     set init to TRUE, save the ADC peripheral register set address,
*     and initialize the ADC interface and controller to a default
*     state by calling adc_default(), and return a pointer to the ADC
*     config structure to the caller.
*
* Parameters:
*     ipbase: ADC descriptor device address
*     arg   : On board clock oscillator frequency (XTAL_IN)
*
* Outputs: None
*
* Returns: The pointer to a ADC config structure or 0
*
* Notes: None
*
**********************************************************************/
INT_32 adc_open(INT_32 ipbase,
                INT_32 arg)
{
  INT_32 status = 0;
  
  if ((adccfg.init == FALSE) && ((ADC_REGS_T *) ipbase == ADC))
  {
    /* Device is valid and not previously initialized */
    adccfg.init = TRUE;
    
    /* Save and return address of peripheral block */
    adccfg.regptr = (ADC_REGS_T *) ipbase;
    
    /* Place ADC in a default state and
    store the ADC clock source frequency */
    adccfg.adc_clk_source = adc_default(adccfg.regptr, (UNS_32)arg,
                                        FALSE);
    
    /* Empty ring buffer and set conversion counter to 0 */
    adccfg.rx_head = adccfg.rx_tail = 0;
    adccfg.conv_count = 0;
    
    /* Clear default callback vector addresses for pendown and
    brownout interrupts */
    adccfg.bro_cbfunc = (PFV) NULL;
    adccfg.pd_cbfunc = (PFV) NULL;
    
    /* Return pointer to ADC configuration structure */
    status = (INT_32) &adccfg;
  }
  
  return status;
}

/***********************************************************************
*
* Function: adc_close
*
* Purpose: Close the ADC
*
* Processing:
*     If init is not TRUE, then return _ERROR to the caller as the
*     device was not previously opened. Otherwise, disable the ADC
*     interface and controller by calling adc_default(), and return
*     _NO_ERROR to the caller.
*
* Parameters:
*     devid: Pointer to ADC config structure
*
* Outputs: None
*
* Returns: The status of the close operation
*
* Notes: None
*
**********************************************************************/
STATUS adc_close(INT_32 devid)
{
  ADC_CFG_T *adccfgptr = (ADC_CFG_T *) devid;
  STATUS status = _ERROR;
  
  if (adccfgptr->init == TRUE)
  {
    /* Restore ADC to a default state */
    adc_default(adccfg.regptr, 0, FALSE);
    
    status = _NO_ERROR;
    adccfgptr->init = FALSE;
  }
  
  return status;
}

/***********************************************************************
*
* Function: adc_ioctl
*
* Purpose: ADC configuration block
*
* Processing:
*     This function is a large case block. Based on the passed function
*     and option values, set or get the appropriate ADC parameter.
*
* Parameters:
*     devid: Pointer to ADC config structure
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
STATUS adc_ioctl(INT_32 devid,
                 INT_32 cmd,
                 INT_32 arg)
{
  ADC_REGS_T *adcregsptr;
  UNS_32 tmp;
  ADC_CH_CONV_T *chcfgptr;
  ADC_CFG_T *adccfgptr = (ADC_CFG_T *) devid;
  STATUS status = _ERROR;
  
  if (adccfgptr->init == TRUE)
  {
    status = _NO_ERROR;
    adcregsptr = adccfgptr->regptr;
    
    switch (cmd)
    {
    case ADC_SET_CLOCK:
      /* Set base speed of ADC clock in Hz */
      if(adc_set_clock((UNS_32)arg, adccfgptr) == 0)
        status = _ERROR;
      break;
      
    case ADC_CLEAR_FIFO:
      /* Clear the ADC sample FIFO */
      while ((adcregsptr->adcfifosts & ADC_FIFO_EMPTY) == 0)
      {
        tmp = adcregsptr->adcresults;
      }
      break;
      
    case ADC_ENABLE_IREF:
      /* Enable or disable the internal reference buffer */
      if (arg == 1)
      {
        adcregsptr->adcpcr |= ADC_REF_ENABLE;
      }
      else
      {
        adcregsptr->adcpcr &= ~ADC_REF_ENABLE;
      }
      break;
      
    case ADC_CLEAR_INTS:
      /* Clear pending ADC interrupts */
      adcregsptr->adcic = ((UNS_32) arg &
        (ADC_EOS_CLR | ADC_PEN_CLR | ADC_BROWNOUT_CLR));
      break;
      
    case ADC_INT_ENABLE:
      /* Enable ADC interrupts */
      adcregsptr->adcie |= ((UNS_32) arg &
        (ADC_FIFO_OV_INT | ADC_WATERM_INT | ADC_BROWN_IRQ_INT |
        ADC_EOS_INT | ADC_PEN_INT)) | ADC_GLOBAL_INT;
      break;
      
    case ADC_INT_DISABLE:
      /* Disable ADC interrupts */
      adcregsptr->adcie &= ~((UNS_32) arg &
        (ADC_FIFO_OV_INT | ADC_WATERM_INT | ADC_BROWN_IRQ_INT |
        ADC_EOS_INT | ADC_PEN_INT | ADC_GLOBAL_INT));
      if (adcregsptr->adcie == ADC_GLOBAL_INT)
      {
        adcregsptr->adcie = 0;
      }                    
      break;
      
    case ADC_INSTALL_BRO:
      /* Install the brownout interrupt callback function */
      adccfgptr->bro_cbfunc = (PFV) arg;
      break;
      
    case ADC_INSTALL_PDC:
      /* Install the pendown interrupt callback function */
      adccfgptr->pd_cbfunc = (PFV) arg;
      break;
      
    case ADC_INSTALL_EOS:
    /* Install the end of sequence interrupt callback
      function */
      adccfgptr->eos_cbfunc = (PFV) arg;
      break;
      
    case ADC_SET_IDLE_CH:
    /* Configure or reconfigure the ADC inputs used
      during idle mode */
      chcfgptr = (ADC_CH_CONV_T *) arg;
      tmp = (
        ADC_SETTIME(adc_time_2_clock(adccfgptr, chcfgptr->settle_time))
        | ADC_INPSEL(chcfgptr->inpc) |
        ADC_INNSEL(chcfgptr->innc) |
        ADC_REFP_SEL(chcfgptr->vrefp));
      adcregsptr->adcidlehi = tmp;
      
      /* Build and save the control bank low word register */
      tmp = (ADC_REFN_SEL(chcfgptr->vrefn) |
        chcfgptr->bias);
      adcregsptr->adcidlelo = tmp;
      break;
      
    case ADC_SET_WMLEVEL:
      /* Set ADC FIFO watermark (interrupt trip) level */
      tmp = adcregsptr->adcgencfg & ~ADC_WM_MASK;
      adcregsptr->adcgencfg = (tmp |
        ADC_WM_SET((UNS_32) arg));
      break;
      
    case ADC_ADD_CHANNEL:
      /* Add a conversion sequence entry */
      chcfgptr = (ADC_CH_CONV_T *) arg;
      status = adc_add_sequence_entry(chcfgptr, adccfgptr);
      break;
      
    case ADC_CHG_CHANNEL:
    /* Changes a conversion sequence entry with the passed
      key argument */
      chcfgptr = (ADC_CH_CONV_T *) arg;
      status = adc_chg_sequence_entry(chcfgptr, adccfgptr);
      break;
      
    case ADC_START_SAMP:
      /* Starts an ADC sample */
      adcregsptr->adcgencfg |= ADC_SSB_START;
      break;
      
    case ADC_SET_STATE:
      /* Set sampling state */
      if (arg == ADC_MODE_RESET)
      {
        /* Reset ADC controller and driver */
        adc_default(adcregsptr, 0, FALSE);
        
        /* Empty ring buffer and set conversion counter
        to 0 */
        adccfgptr->rx_head = adccfgptr->rx_tail = 0;
        adccfgptr->conv_count = 0;
      }
      else if (arg == ADC_MODE_STOP)
      {
        /* Stop the ADC controller */
        tmp = adcregsptr->adcpcr & ~ADC_PWM_MASK;
        adcregsptr->adcpcr = (tmp | ADC_PWM_ALL_OFF);
      }
      else if (arg == ADC_SW_TRIGGERED)
      {
        /* ADC mode is polled/sw triggered */
        /* Standby mode  */
        tmp = adcregsptr->adcpcr & ~ADC_PWM_MASK;
        adcregsptr->adcpcr = (tmp | ADC_PWM_STANDBY);
        
        /* Enable triggered mode  */
        tmp = adcregsptr->adcgencfg & ~ADC_SSM_MASK;
        adcregsptr->adcgencfg = (tmp | ADC_SSM_SSB);
      }
      else if (arg == ADC_PENDOWN_TRIGGERED)
      {
      /* An ADC sequence is started on a pendown
        detection event */
        /* Standby mode  */
        tmp = adcregsptr->adcpcr & ~ADC_PWM_MASK;
        adcregsptr->adcpcr = (tmp | ADC_PWM_STANDBY);
        
        /* Enable pendown triggered mode  */
        tmp = adcregsptr->adcgencfg & ~ADC_SSM_MASK;
        adcregsptr->adcgencfg = (tmp | ADC_SSM_PEN);
      }
      else if (arg == ADC_PENDOWN_SW_TRIGGERED)
      {
      /* An ADC sequence is started on a pendown
        detection event or software request */
        /* Standby mode  */
        tmp = adcregsptr->adcpcr & ~ADC_PWM_MASK;
        adcregsptr->adcpcr = (tmp | ADC_PWM_STANDBY);
        
        /* Enable pendown triggered mode  */
        tmp = adcregsptr->adcgencfg & ~ADC_SSM_MASK;
        adcregsptr->adcgencfg = (tmp | ADC_SSM_SSB_PEN);
      }
      else if (arg == ADC_CONTINUOUS)
      {
        /* Place ADC in run mode */
        tmp = adcregsptr->adcpcr & ~ADC_PWM_MASK;
        adcregsptr->adcpcr = (tmp | ADC_PWM_RUN);
        
        /* Continuous sampling */
        tmp = adcregsptr->adcgencfg & ~ADC_SSM_MASK;
        adcregsptr->adcgencfg = (tmp | ADC_SSM_CONT);
      }
      else
      {
        /* Unknown mode */
        status = SMA_BAD_PARAMS;
      }
      break;                
      
    case ADC_GET_STATUS:
      switch (arg)
      {
      case ADC_RAWINT_ST:
        /* Return raw interrupt statuses */
        status = (INT_32) (adcregsptr->adcrawint &
          (ADC_FIFO_OV_IRQ | ADC_WATERM_IRQ |
          ADC_EOS_IRQ |ADC_PEN_IRQ |
          ADC_BROWNOUT_IRQ));
        break;
        
      case ADC_INT_ST:
      /* Return pending (enabled) interrupt
        statuses */
        status = (INT_32) (adcregsptr->adcmis &
          (ADC_FIFO_OV_IRQ | ADC_WATERM_IRQ |
          ADC_EOS_IRQ |ADC_PEN_IRQ |
          ADC_BROWNOUT_IRQ));
        break;
        
      case ADC_GEN_ST:
        /* Returns general status register value */
        status = adcregsptr->adcgensts;
        break;
        
      case ADC_CONV_COUNT:
      /* Return the number of conversions in the
        conversion count sequence */
        status = adccfgptr->conv_count;
        break;
        
      default:
        /* Unsupported parameter */
        status = SMA_BAD_PARAMS;
        break;
      }
      break;

    case ADC_SET_ADCLK_SCOC:
      tmp = adc_get_clock(adccfgptr);
      adccfgptr->adc_clk_source = adc_default(adcregsptr,
                                     (UNS_32)arg,
                                      TRUE);
      if(adc_set_clock(tmp, adccfgptr) == 0)
        status = _ERROR;
        break;
    
    case ADC_SET_4WIRE_TSC_MODE:
      status = adc_setup_4wire_touchscreen((ADC_TSC_CONV_T*)arg, 
                  adccfgptr);
      break;
      
    case ADC_SET_5WIRE_TSC_MODE:
      status = adc_setup_5wire_touchscreen((ADC_TSC_CONV_T*)arg, 
                  adccfgptr);
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
* Function: adc_read_polled
*
* Purpose: Reads data directly from the ADC FIFO
*
* Processing:
*     If the device is not initialized, return 0 to the caller.
*     Otherwise, hile the number of bytes read is less than max_bytes
*     and the ADC FIFO is not empty, read a FIFO entry from the ADC
*     FIFO and place it into the passed user buffer. Increment bytes
*     by 2. When the loop is exited, return the number of bytes read
*     to the caller.
*
* Parameters:
*     devid:     Pointer to an ADC configuration structure
*     buffer:    Pointer to data buffer to copy to
*     max_bytes: Number of bytes to read
*
* Outputs: None
*
* Returns: The number of bytes read from the FIFO
*
* Notes: None
*
**********************************************************************/
INT_32 adc_read_polled(INT_32 devid,
                       void *buffer,
                       INT_32 max_bytes)
{
  ADC_CFG_T *adccfgptr = (ADC_CFG_T *) devid;
  ADC_REGS_T *adcregptr = adccfgptr->regptr;
  UNS_16 *adcbuf = (UNS_16 *) buffer;
  INT_32 bytes = 0;
  
  if (adccfgptr->init == TRUE)
  {
    /* Grab all entries or until size equals max_bytes */
    while ((bytes < max_bytes) &&
      ((adcregptr->adcfifosts & ADC_FIFO_EMPTY) == 0))
    {
      *adcbuf = (UNS_16) adcregptr->adcresults;
      adcbuf++;
      bytes = bytes + 2;
    }
  }
  
  return bytes;
}

/***********************************************************************
*
* Function: adc_read_ring
*
* Purpose: Reads data from the ADC ring buffer
*
* Processing:
*     If the init flag for the ADC structure is FALSE, return 0 to
*     the caller. Otherwise, save the state of the ADC interrupts and
*     disable the ADC interrupts. Loop until max_bytes equals 0 or
*     until the receive ring buffer is empty, whichever comes
*     first. Read the data from the ring buffer  indexed by the tail
*     pointer and place it into the user buffer. Increment the tail
*     pointer and user buffer pointer. If the tail pointer exceeds the
*     buffer size, set the tail pointer to 0. Increment bytes, and
*     decrement max_bytes. Exit the loop based on the loop conditions,
*     re-enable the receive interrupts, and return the number of bytes
*     read to the caller.
*
* Parameters:
*     devid:     Pointer to an ADC configuration structure
*     buffer:    Pointer to data buffer to copy to
*     max_bytes: Number of bytes to read
*
* Outputs: None
*
* Returns: The number of bytes actually read from the ring buffer
*
* Notes: None
*
**********************************************************************/
INT_32 adc_read_ring(INT_32 devid,
                     void *buffer,
                     INT_32 max_bytes)
{
  ADC_CFG_T *adccfgptr = (ADC_CFG_T *) devid;
  ADC_REGS_T *adcregs;
  UNS_32 tmp1;
  UNS_16 *data = (UNS_16 *) buffer;
  INT_32 bytes = 0;
  
  if (adccfgptr->init == TRUE)
  {
    adcregs = adccfgptr->regptr;
    
    /* Temporarily lock out ADC receive interrupt during this
    read so the ADC receive interrupt won't cause problems
    with the ring buffer index values */
    tmp1 = adcregs->adcie & (ADC_FIFO_OV_INT | ADC_WATERM_INT |
      ADC_EOS_INT | ADC_PEN_INT | ADC_GLOBAL_INT);
    adcregs->adcie &= ~(ADC_FIFO_OV_INT | ADC_WATERM_INT |
      ADC_EOS_INT | ADC_PEN_INT | ADC_GLOBAL_INT);
    
      /* Loop until receive ring buffer is empty or until max_bytes
    expires */
    while ((max_bytes > 0) &&
      (adccfgptr->rx_tail != adccfgptr->rx_head))
    {
      /* Read data from ring buffer into user buffer */
      *data = adccfgptr->rx[adccfgptr->rx_tail];
      data++;
      
      /* Update tail pointer */
      adccfgptr->rx_tail++;
      
      /* Make sure tail didn't overflow */
      if (adccfgptr->rx_tail >= ADC_RING_BUFSIZE)
      {
        adccfgptr->rx_tail = 0;
      }
      
      /* Increment data count and decrement buffer size count */
      bytes = bytes + 2;
      max_bytes = max_bytes - 2;
    }
    
    /* Re-enable ADC receive interrupt(s) */
    adcregs->adcie |= tmp1;
  }
  
  return bytes;
}

/***********************************************************************
*
* Function: adc_write
*
* Purpose: ADC write function (stub only)
*
* Processing:
*     Returns 0 to the caller.
*
* Parameters:
*     devid: Pointer to ADC config structure
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
INT_32 adc_write(INT_32 devid,
                 void *buffer,
                 INT_32 n_bytes)
{
  return 0;
}

/***********************************************************************
*
* Function: adc_isr
*
* Purpose: ADC interrupt handler
*
* Processing:
*     On an interrupt, read the (masked) interrupt status. Based on
*     the status, handle the watermark or end of sequence interrupts
*     by calling the adc_ring_fill() to move the data from the ADC
*     FIFO to the ADC driver ring buffer. If the interrupt was due to
*     an end of sequence and the end of sequence callback function
*     exists, then call the user function. If the interrupt was due
*     to a pendown event and the pendown callback function exists,
*     then call the user function. If the interrupt was due to a
*     brownout interrupt and the brownout callback function exists,
*     then call the user function. 
*
* Parameters: None
*
* Outputs: None
*
* Returns: Nothing
*
* Notes: None
*
**********************************************************************/
void adc_isr(void)
{
  UNS_32 int_status;
  
  /* Read interrupt status register */
  int_status = ADC->adcmis;
  
  /* Perform function based on interrupt status */
  if ((int_status & ADC_WATERM_INT) != 0)
  {
    /* FIFO needs servicing, called ring fill handler */
    adc_ring_fill(&adccfg);
  }
  
  if ((int_status & ADC_EOS_INT) != 0)
  {
  /* ADC end of sequence interrupt, move data from FIFO
    to ring buffer */
    adc_ring_fill(&adccfg);
    
    /* Call user function if it exists */
    if (adccfg.eos_cbfunc != (PFV) 0)
    {
      adccfg.eos_cbfunc();
    }
    
    /* Clear end of sequence interrupt */
    ADC->adcic = ADC_EOS_CLR;
  }
  
  if ((int_status & ADC_PEN_INT) != 0)
  {
    /* Call user function if it exists */
    if (adccfg.pd_cbfunc != (PFV) 0)
    {
      adccfg.pd_cbfunc();
    }
    
    /* Clear end of pendown interrupt and call user function */
    ADC->adcic = ADC_PEN_CLR;
    
  }
  
  if ((int_status & ADC_BROWNOUT_IRQ) != 0)
  {
    
    /* If the brownout interrupt callback function exists, call it */
    if (adccfg.bro_cbfunc != (PFV) 0)
    {
      adccfg.bro_cbfunc();
    }
    
    /* Brownout interrupt, clear it */
    ADC->adcic = ADC_BROWNOUT_CLR;
  }
}

/***********************************************************************
*
* Function: adc_pen_isr
*
* Purpose: ADC Pen Vectored interrupt handler
*
* Processing:
*     If pendown callback function exists, then call the user function
*     and clear the interrupt. 
*
* Parameters: None
*
* Outputs: None
*
* Returns: Nothing
*
* Notes: None
*
**********************************************************************/
void adc_pen_isr(void)
{
  
  /* Call user function if it exists */
  if (adccfg.pd_cbfunc != (PFV) 0)
  {
    adccfg.pd_cbfunc();
  }
  /* Clear end of pendown interrupt and call user function */
  ADC->adcic = ADC_PEN_CLR;
}

/***********************************************************************
*
* Function: adc_brownout_isr
*
* Purpose: ADC brownout Vectored interrupt handler
*
* Processing:
*     If brownout callback function exists, then call the user function
*     and clear the interrupt. 
*
* Parameters: None
*
* Outputs: None
*
* Returns: Nothing
*
* Notes: None
*
**********************************************************************/
void adc_brownout_isr(void)
{
  /* If the brownout interrupt callback function exists, call it */
  if (adccfg.bro_cbfunc != (PFV) 0)
  {
    adccfg.bro_cbfunc();
  }
  
  /* Brownout interrupt, clear it */
  ADC->adcic = ADC_BROWNOUT_CLR;
}

/***********************************************************************
*
* Function: adc_strip_data
*
* Purpose: Strip out and justify analog data from a sample
*
* Processing:
*     The data that is returned from the adc_read() function returns a
*     value that has data and the analog input number mixed into it.
*     This function strips off the analog input number and returns only
*     the converted 10-bit data value right justified into a 16-bit
*     field.
*
* Parameters:
*     data : The raw ADC sample to extract the data from
*
* Outputs: None
*
* Returns: The right justified analog data value
*
* Notes: None
*
**********************************************************************/
UNS_16 adc_strip_data(UNS_16 data)
{
  return ADC_READ_OUTPUT(data);
}

/***********************************************************************
*
* Function: adc_strip_input
*
* Purpose: Strip out the analog input channel from a sample
*
* Processing:
*     The data that is returned from the adc_read() function returns a
*     value that has data and the analog input number mixed into it.
*     This function strips off the converted data and returns only the
*     analog input channel number.
*
* Parameters:
*     data : The raw ADC sample to extract the data from
*
* Outputs: None
*
* Returns: The right justified analog data value
*

  * Notes: None
  *
**********************************************************************/
UNS_16 adc_strip_input(UNS_16 data)
{
  return (data & ADC_CBANKTAG);
}
