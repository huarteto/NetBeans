/***********************************************************************
 * $Id:: lh79524_clcdc_driver.c 27 2007-08-31 19:35:06Z kevinw         $
 *
 * Project: LH79524 CLCDC driver
 *
 * Description:
 *     This file contains driver support for the CLCDC module on the
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
 **********************************************************************/

#include "lh79524_chip.h"
#include "lh79524_rcpc.h"
#include "lh79524_iocon.h"
#include "lh79524_clcdc_driver.h"

/***********************************************************************
 * LCD driver private data and types
 **********************************************************************/

/* Defines for the ip block base address and driver control object */
#define REGS_T     CLCDC_REGS_T
#define TFT_REGS_T LCDICP_REGS_T
#define DRIVER_T   CLCDC_T
#define N_DEVS     N_LCDS

/* Private methods */
static void clcdc0_isr (void);
static void clcdc_default_isr (INT_32 devid);
static BOOL_32 lcd_initialize (INT_32 devid);
static void lcd_iocon_init (void);
static void lcd_TFT_iocon_init (void);

/* Table of device base addresses */
static UNS_32 driverid[N_DEVS] = 
{
  /* base Address */
  LCD_BASE
};

/* Instansiations of the driver control object */
static DRIVER_T driver[N_DEVS];

/* Address of the driver control objects */
static DRIVER_T* pdriver[N_DEVS] = 
{
  (DRIVER_T*)&driver[0]
};

/* Address of the driver interrupt handlers  */
static void* driver_isr[N_DEVS] =
{
  (void*)clcdc0_isr
};

/***********************************************************************
 * CLCDC driver public functions
 **********************************************************************/

/***********************************************************************
 *
 * Function: lcd_open
 *
 * Purpose: Open the LCD
 *
 * Processing:
 *     If init is not FALSE, return 0x00000000 to the caller. Otherwise,
 *     set init to TRUE, save the LCD peripheral register set address,
 *     disable the LCD, and check the arg value. If arg is not 0, then
 *     call the lcd_initialize with the value of arg as its parameter.
 *     Return a pointer to the LCD config structure to the caller. TBD
 *
 * Parameters:
 *     ipbase: LCD descriptor device address
 *     arg   : Not used
 *
 * Outputs: None
 *
 * Returns: The pointer to a LCD config structure or 0
 *
 * Notes: None
 *
 **********************************************************************/

INT_32 lcd_open (INT_32 ipbase, 
                 INT_32 arg)
{
  INT_32    dev_exists = FALSE;
  INT_32    devid      = 0;
  CHAR*     dst        = NULL;
  INT_32    idx        = 0;
  REGS_T*   pregs      = NULL;
  DRIVER_T* pdev       = NULL;
  CLCDC_SETTINGS_T* lcdcfg = (CLCDC_SETTINGS_T*)arg;
  
  /* Find the device in the driver table */
  for (devid = 0; devid < NELEMENTS(driverid); devid++)
  {
    /* Do we have a match */
    if (ipbase == driverid[devid])
    {
      /* Found the device */
      dev_exists = TRUE;
      break;
    }
  }
  /* Check to make sure that the device exists */
  if (dev_exists == FALSE)
  {
    return (0);
  }
  
  /* Attach to the base address of the device */
  pdev = (DRIVER_T*)pdriver[devid];
  
  /* Make sure the device is not already opened */
  if (pdev->init == TRUE)
  {
    /* Device has already been opened */
    return (0);
  }
  
  /* Clear the driver control structure */
  dst = (CHAR*)pdev;
  for (idx = 0; idx < sizeof(DRIVER_T); idx++)
  {
    *dst++ = 0;
  }
  
  /* Set LCD clock same as HCLK and the enable LCD clock in RCPC */
  RCPC->lcdclkprescale = RCPC_PRESCALER_DIV1;
  RCPC->periphclkctrl1 &= ~RCPC_CLKCTRL1_LCD_DISABLE;
  RCPC->ahbclkctrl &= ~RCPC_AHBCLKCTRL_LCD_DISABLE;
  pdev->clcdclk = RCPC_GET_HCLK(lcdcfg->board_xtal_in);
  
  /* Attach to the base address of the selected device */
  pdev->regs = (void*)driverid[devid];
  pdev->tft  = (void*)ALI_BASE;
  pdev->cfg  = (void*)arg;
  pregs      = (REGS_T*)pdev->regs;
  
  /* Enable LCD pins in IOCON registers */
  if (lcdcfg->lcdparam->lcd_panel_type == ADTFT ||
      lcdcfg->lcdparam->lcd_panel_type == HRTFT)
    lcd_iocon_init();

  /* Configure to TFT pins */  
  if (lcdcfg->lcdparam->lcd_panel_type == TFT)
    lcd_TFT_iocon_init();
  
  /* Disable the controller */
  pregs->lcdctrl &= ~CLCDC_LCDCTRL_ENABLE;
  
  /* make sure the CLCDC's DMA will only accesses memory on the AHB */
  pregs->lcdupbase = INTERNAL_SRAM_BASE; 
  pregs->lcdlpbase = INTERNAL_SRAM_BASE;
  
  /* Save the default rtc interrupt service routine */
  pdev->irq_hdlr = driver_isr[devid];
  
  /* If the passed argument was not NULL, then it was a pointer
  to a LCD device configuration structure that needs to be
  configured */
  if (arg != 0)
  {
    (void) lcd_initialize ((INT_32)pdev);
  }
  
  /* Mark the device as initialized */
  pdev->init = TRUE; 
  
  /* Return the device id */
  return ((INT_32)pdev);
}

/***********************************************************************
 *
 * Function: lcd_close
 *
 * Purpose: Close the LCD
 *
 * Processing:
 *     If init is not TRUE, then return _ERROR to the caller as the
 *     device was not previously opened. Otherwise, set init to FALSE,
 *     disable LCD controller, and return _NO_ERROR to the caller.
 *
 * Parameters:
 *     devid: Pointer to LCD config structure
 *
 * Outputs: None
 *
 * Returns: The status of the close operation
 *
 * Notes: None
 *
 **********************************************************************/

STATUS lcd_close (INT_32 devid)
{
  STATUS    status = _NO_ERROR;
  INT_32    idx    = 0;
  CHAR*     dst    = NULL;
  DRIVER_T* pdev   = NULL;
  REGS_T*   pregs  = NULL;

  /* Sanity check */
  if (devid == 0)
  {
      /* device not opened or non-valid file handle */
      return (_ERROR);
  }

  /* Bind to the device control structure */
  pdev = (DRIVER_T*)devid;
  pregs = (REGS_T*)pdev->regs;

  /* Make sure the device is already opened */
  if (pdev->init == FALSE)
  {
      return (_ERROR);
  }

  /* Reset the device */
  pregs->lcdctrl &= ~CLCDC_LCDCTRL_ENABLE;

  /* Invalidate the device */
  pdev->init = FALSE;

  /* Disable LCD clock in RCPC */
  RCPC->periphclkctrl1 |= RCPC_CLKCTRL1_LCD_DISABLE;
  RCPC->ahbclkctrl |= RCPC_AHBCLKCTRL_LCD_DISABLE;

  /* Clear the serial driver control structure */
  dst = (CHAR*)pdev;
  for (idx = 0; idx < sizeof (DRIVER_T); idx++)
  {
      *dst++ = 0;
  }

  /* Device is closed */
  return (status);
}

/***********************************************************************
 *
 * Function: lcd_read
 *
 * Purpose: LCD read function (stub only)
 *
 * Processing:
 *     Return 0 to the caller.
 *
 * Parameters:
 *     devid:     Pointer to LCD config structure
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

INT_32 lcd_read (INT_32 devid,
                 CHAR*  buffer,
                 INT_32 max_bytes)
{
  INT_32 n_bytes = 0;
  /* Not implemented - use ioctl */
  return (n_bytes);
}

/***********************************************************************
 *
 * Function: lcd_write
 *
 * Purpose: LCD write function (stub only)
 *
 * Processing:
 *     Return 0 to the caller.
 *
 * Parameters:
 *     devid:   Pointer to LCD config structure
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

INT_32 lcd_write (INT_32 devid,
                  CHAR*  buffer,
                  INT_32 n_bytes)
{
  INT_32 actual_bytes = 0;
  /* Not implemented - use ioctl */
  return (actual_bytes);
}

/***********************************************************************
 *
 * Function: lcd_ioctl
 *
 * Purpose: LCD configuration block
 *
 * Processing:
 *     This function is a large case block. Based on the passed function
 *     and option values, set or get the appropriate real time clock
 *     parameter.
 *
 * Parameters:
 *     devid: Pointer to LCD config structure
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

STATUS lcd_ioctl (INT_32 devid,
                  INT_32 cmd,
                  INT_32 arg)
{
  UNS_32    temp   = 0;
  STATUS    status = _NO_ERROR; 
  REGS_T*   pregs  = NULL;
  DRIVER_T* pdev   = NULL;
  
  /* Sanity check */
  if (devid == 0)
  {
    /* Null pointer */
    return (0);
  }
  
  /* Attach to the base address of the driver */
  pdev = (DRIVER_T*)devid;
  
  /* Make sure the device is already opened */
  if (pdev->init == FALSE)
  {
    return (_ERROR);
  }
  
  /* Attach to the base address of the selected timer */
  pregs = (REGS_T*)pdev->regs;
  
  /* Validate the request */
  switch (cmd)
  {
  case LCD_ENABLE:
    /* Enable */
    temp = pregs->lcdctrl;
    temp |= CLCDC_LCDCTRL_ENABLE;
    pregs->lcdctrl = temp;
    break;
  case LCD_DISABLE:
    /* Disable */
    temp = pregs->lcdctrl;
    temp &= ~CLCDC_LCDCTRL_ENABLE;
    pregs->lcdctrl = temp;
    break;
    
  case LCD_PW_ENABLE:
    /* Enable LCD power */
    temp = pregs->lcdctrl;
    temp |= (CLCDC_LCDCTRL_ENABLE | CLCDC_LCDCTRL_PWR);
    pregs->lcdctrl = temp;
    break;
  case LCD_PW_DISABLE:
    /* Enable LCD power */
    temp = pregs->lcdctrl;
    temp &= ~(CLCDC_LCDCTRL_ENABLE | CLCDC_LCDCTRL_PWR);
    pregs->lcdctrl = temp;
    break;
    
  case LCD_SET_FB:
    /* Sanity check */
    if (arg == 0)
    {
      /* Null pointer */
      status = _ERROR;
      break;
    }
    /* Set frame buffer address */
    pregs->lcdupbase = (UNS_32)arg;
    pregs->lcdlpbase = (UNS_32)arg;
    break;
    
  case LCD_GET_STATUS:
    /* Sanity check */
    if (arg == 0)
    {
      /* Null pointer */
      status = _ERROR;
      break;
    }
    *(INT_32*)arg = pregs->lcdctrl & CLCDC_LCDCTRL_ENABLE;
    break;
    
    /* Mask interrupts at the source */ 
  case LCD_DISABLE_INT:   
    /* Get the current state */
    temp = pregs->lcdintrenable;
    /* Mask off the interrupt */
    temp &= ~arg;
    /* Update the driver control object */
    pdev->imask = temp;
    /* Update the interrupt control register */
    pregs->lcdintrenable = temp;
    break;
    /* Unmask interrupts at the source */ 
  case LCD_ENABLE_INT:
    /* Get the current state */
    temp = pregs->lcdintrenable;
    /* UnMask the interrupt */
    temp |= arg;
    /* Update the driver control object */
    pdev->imask = temp;
    /* Update the interrupt control register */
    pregs->lcdintrenable = temp;
    break;
    
  case LCD_ENABLE_VCOMP_INT:
    /* Get the current state */
    temp = pregs->lcdctrl;
    /* Clear the current vcomp bit field B[13:12] */
    temp &= ~_SBF(12,3);
    /* Update the current vcomp bit field B[13:12] */
    temp |= _SBF(12,arg);
    /* Write the new value to the control register */
    pregs->lcdctrl = temp;
    break;
  case LCD_DISABLE_VCOMP_INT:
    /* Get the current state */
    temp = pregs->lcdctrl;
    /* Clear the current vcomp bit field B[13:12] */
    temp &= ~_SBF(12,3);
    /* Write the new value to the control register */
    pregs->lcdctrl = temp;
    break;
  case LCD_SET_RGB:
    /* Get the current state */
    temp = pregs->lcdctrl;
    /* Set the BGR bit field B[8] */
    temp |= CLCDC_LCDCTRL_RGB;
    /* Write the new value to the control register */
    pregs->lcdctrl = temp;
    break;
  case LCD_SET_BGR:
    /* Get the current state */
    temp = pregs->lcdctrl;
    /* Clear the BGR bit field B[8] */
    temp &= ~CLCDC_LCDCTRL_RGB;
    /* Write the new value to the control register */
    pregs->lcdctrl = temp;
    break;
  case LCD_DMA_ON_4MT:
  /* Set DMA requests to start when 4 or 8 entries are
    free in the CLCDC FIFOs */
    if (arg == 0)
    {
      /* Use 8 entries */
      pregs->lcdctrl &= ~CLCDC_LCDCTRL_WATERMARK;
    }
    else
    {
      /* Use 4 entries */
      pregs->lcdctrl |= CLCDC_LCDCTRL_WATERMARK;
    }
    break;
    
    /* Return pointer to device isr */
  case LCD_GET_ISR:
    /* Sanity check */
    if ((void*)arg == NULL)
    {
      status = _ERROR;
      break;
    }
    /* Return the driver interrupt service routine */
    *(INT_32**)arg = (void*)pdev->irq_hdlr;
    break;
    /* Bind a callback method to be used by the timer isr */
  case LCD_SET_CALLBACK:
    /* Save the interrupt callback method */
    pdev->callback = (PFI)arg;
    break;
    
  default:
    /* Unsupported parameter */
    status = _ERROR;
  }
  
  return (status);
}

/*********************************************************************** 
 * 
 * Function:  clcdc0_isr
 * 
 * Purpose: 
 *  Handle an lcd interrupt.
 * 
 * Processing: 
 *  See Purpose
 *
 * Parameters: 
 *  None
 * 
 * Outputs: 
 *  None
 * 
 * Returns: 
 *  None
 * 
 * Notes: 
 *  None
 * 
 **********************************************************************/

static void clcdc0_isr (void)
{
    /* Call the default handler */
    clcdc_default_isr ((INT_32)pdriver[0]);
}

/************************************************************************
*
* Function: clcdc_default_isr
*
* Purpose: 
*  Services the interrupt for the lcd controller.
*
* Processing:
*     
* Parameters: 
*  None
*
* Outputs: 
*  None
*
* Returns: 
*  None
*
* Notes: 
*  None
*
************************************************************************/

static void clcdc_default_isr (INT_32 devid)
{
}

/***********************************************************************
 * CLCDC driver private functions
 **********************************************************************/

/***********************************************************************
 *
 * Function: lcd_initialize
 *
 * Purpose: Initialize the LCD controller
 *
 * Processing:
 *     Prior to resetting LCD values, disable the LCD controller. Based
 *     on the values in the passed LCD_PARAM_T structure, generate
 *     the appropriate LCD configuration words and place them in the
 *     correct LCD registers.
 *
 * Parameters:
 *     lcdcfgptr : A pointer to an LCD configuration data structure
 *
 * Outputs: None
 *
 * Returns: TRUE if the LCD was initialized, otherwise FALSE
 *
 * Notes: This driver only supports TFT and HR-TFT panel types.
 *
 **********************************************************************/

static BOOL_32 lcd_initialize (INT_32 devid)
{
  UNS_32            tmp;
  BOOL_32           init  = FALSE;
  LCD_PARAM_T*      pcfg  = NULL; 
  REGS_T*           pregs = NULL; 
  TFT_REGS_T*       ptft  = NULL; 
  DRIVER_T*         pdev  = NULL;
  UNS_32            pixel_div = 0;
  
  /* Sanity check */
  if (devid == 0)
  {
    /* Null pointer */
    return (init);
  }
  
  /* Attach to the base address of the driver */
  pdev = (DRIVER_T*)devid;
  
  /* Bind to the clcdc regs, tft regs, and config structure */
  pregs = (REGS_T*)pdev->regs;
  ptft  = (LCDICP_REGS_T*)pdev->tft;
  pcfg  = ((CLCDC_SETTINGS_T*)pdev->cfg)->lcdparam;
  
  /* Disable the display in case it is on */
  pregs->lcdctrl &= ~CLCDC_LCDCTRL_ENABLE;
  
  /* Generate the timing 0 word */
  tmp = (CLCDC_LCDTIMING0_PPL(pcfg->pixels_per_line) |
    CLCDC_LCDTIMING0_HSW(pcfg->h_sync_pulse_width) |
    CLCDC_LCDTIMING0_HFP(pcfg->h_front_porch) |
    CLCDC_LCDTIMING0_HBP(pcfg->h_back_porch));
  pregs->lcdtiming0 = tmp;
  
  /* Generate the timing 1 word */
  tmp = (CLCDC_LCDTIMING1_LPP(pcfg->lines_per_panel) |
    CLCDC_LCDTIMING1_VSW(pcfg->v_sync_pulse_width) |
    CLCDC_LCDTIMING1_VFP(pcfg->v_front_porch) |
    CLCDC_LCDTIMING1_VBP(pcfg->v_back_porch));
  pregs->lcdtiming1 = tmp;
  
  /* Generate the timing 2 word */
  tmp = CLCDC_LCDTIMING2_ACB(pcfg->ac_bias_frequency);
  if (pcfg->invert_output_enable != 0)
  {
    tmp |= CLCDC_LCDTIMING2_IOE;
  }
  if (pcfg->invert_panel_clock != 0)
  {
    tmp |= CLCDC_LCDTIMING2_IPC;
  }
  if (pcfg->invert_hsync != 0)
  {
    tmp |= CLCDC_LCDTIMING2_IHS;
  }
  if (pcfg->invert_vsync != 0)
  {
    tmp |= CLCDC_LCDTIMING2_IVS;
  }
  /* Compute clocks per line based on panel type */
  switch (pcfg->lcd_panel_type)
  {
  case MONO_4BIT:
    /* Clocks per line is a quarter of pixels per line */
    tmp = tmp | CLCDC_LCDTIMING2_CPL(
      (pcfg->pixels_per_line / 4) - 1);
    break;
    
  case MONO_8BIT:
    /* Clocks per line is an eighth of pixels per line */
    tmp = tmp | CLCDC_LCDTIMING2_CPL(
      (pcfg->pixels_per_line / 8) - 1);
    break;
    
  case CSTN:
    /* CSTN Clocks per line (* 8 / 3) */
    tmp = tmp | CLCDC_LCDTIMING2_CPL((
      (pcfg->pixels_per_line * 8) / 3) - 1);
    break;
    
  case TFT:
  case ADTFT:
  case HRTFT:
  default:
    /* Clocks per line and pixels per line are the same */
    tmp = tmp |
      CLCDC_LCDTIMING2_CPL(pcfg->pixels_per_line - 1);
    break;
  }
  
  pixel_div = (UNS_32)pdev->clcdclk / pcfg->optimal_clock;
  
  /* Determine if 'div' or 'div + 1' is closer to desired
  frequency */
  if(((UNS_32)(pdev->clcdclk/pixel_div) - pcfg->optimal_clock) > 
    (pcfg->optimal_clock - (UNS_32)(pdev->clcdclk/(pixel_div + 1))) )
  {
    pixel_div += 1;
  }
  
  /* Determine pixel divider used in timing 2 word */
  if (pixel_div == 0) 
  {
    /* Pixel clock divider is 1, skip divider logic */
    tmp |= CLCDC_LCDTIMING2_BCD;
  }
  else
  {
    /* Use divider logic */
    tmp |= CLCDC_LCDTIMING2_PCD_LO(pixel_div);
    tmp |= CLCDC_LCDTIMING2_PCD_HI(pixel_div);
  }
  
  pregs->lcdtiming2 = tmp;
  
  /* Default with all interrupts of */
  pregs->lcdintrenable = 0x00000000; 
  
  /* Setup basic control with controller disabled */
  switch (pcfg->bits_per_pixel)
  {
  case 1:
    tmp = CLCDC_LCDCTRL_BPP1;
    break;
    
  case 2:
    tmp = CLCDC_LCDCTRL_BPP2;
    break;
    
  case 4:
    tmp = CLCDC_LCDCTRL_BPP4;
    break;
    
  case 8:
    tmp = CLCDC_LCDCTRL_BPP8;
    break;
    
  case 16:
  default:
    tmp = CLCDC_LCDCTRL_BPP16;            
    break;
  }
  
  switch (pcfg->lcd_panel_type)
  {
  case ADTFT:
  case HRTFT:
  case TFT:
    tmp |= CLCDC_LCDCTRL_TFT;
    break;
    
  default:
    init = FALSE;
  }
  
  tmp |= CLCDC_LCDCTRL_RGB;
  pregs->lcdctrl = tmp;
  
  /* ADTFT and HRTFT setup */
  if ((pcfg->lcd_panel_type == HRTFT) ||
    (pcfg->lcd_panel_type == ADTFT))
  {
    /* Setup HR-TFT controller 'setup' register */
    tmp = LCDICP_SETUP_PPL(pcfg->pixels_per_line) |
      LCDICP_SETUP_MODE_HRTFT | LCDICP_SETUP_RESERVED |
      LCDICP_SETUP_EN;
    ptft->lcdicp_setup = tmp;
    
    /* Setup HR-TFT controller 'control' register */
    tmp = 0x00000000;
    if (pcfg->hrtft_cls_enable != 0)
    {
      tmp |= LCDICP_CONTROL_CLSEN;
    }
    if (pcfg->hrtft_sps_enable != 0)
    {
      tmp |= LCDICP_CONTROL_SPSEN;
    }
    ptft->lcdicp_control = tmp;
    
    /* Setup HR-TFT timing 1 register */
    tmp = (LCDICP_TIMING1_LPDEL(pcfg->hrtft_lp_delay) |
      LCDICP_TIMING1_REVDEL(pcfg->hrtft_polarity_delay) |
      LCDICP_TIMING1_PS_CLSDEL(pcfg->hrtft_lp_to_ps_delay));
    ptft->lcdicp_timing1 = tmp;
    
    /* Setup HR-TFT timing 2 register */
    tmp =
      (LCDICP_TIMING2_PS_CLSDEL2(
      pcfg->hrtft_spl_to_cls_delay) |
      LCDICP_TIMING2_SPLVALUE(pcfg->hrtft_spl_delay));
    ptft->lcdicp_timing2 = tmp;
  }
  else
  {
    /* Normal display type, set HRTFT bypass mode */
    tmp = LCDICP_SETUP_MODE_BYPASS;
    /* Enable the ALI even it is for TFT */
    tmp |= LCDICP_SETUP_EN |  LCDICP_SETUP_RESERVED;          
    ptft->lcdicp_setup = tmp;        
  }
        
  return (init);
}


/***********************************************************************
 *
 * Function: lcd_iocon_init
 *
 * Purpose: Enable muxed LCD controller pins.
 *
 * Processing:
 *     Select LCD pins in IOCON mux control registers.
 *
 * Parameters:
 *     none
 *
 * Outputs: None
 *
 * Returns: None
 *
 **********************************************************************/

static void lcd_iocon_init (void)
{
  UNS_32 temp = 0;
  
  temp = (IOCON_MUX_MASK(IOCON_RES1_PL1_LCDVD15) & 
      IOCON_MUX_MASK(IOCON_RES1_PL0_LCDVD14));
  temp &= IOCON->mux_ctl_1;

  temp |= (IOCON_MUX1_LCDVD14 | IOCON_MUX1_LCDVD15);
  IOCON->mux_ctl_1 = temp;


  temp = (IOCON_MUX_MASK(IOCON_RES19_PE6_LCDVEEN_LCDMOD) & 
      IOCON_MUX_MASK(IOCON_RES19_PE4_LCDSPLEN_LCDREV) &
      IOCON_MUX_MASK(IOCON_RES19_PE5_LCDVDDEN) &
      IOCON_MUX_MASK(IOCON_RES19_PE3_LCDCLS));

  temp &= IOCON->mux_ctl_19;
  temp |= (IOCON_MUX19_LCDMOD | IOCON_MUX19_LCDVDDEN | IOCON_MUX19_LCDVEEN |
      IOCON_MUX19_LCDREV | IOCON_MUX19_LCDCLS);
  IOCON->mux_ctl_19 = temp;
      

  temp = (IOCON_MUX_MASK(IOCON_RES20_PE2_LCDPS) & 
      IOCON_MUX_MASK(IOCON_RES20_PE1_LCDDCLK) &
      IOCON_MUX_MASK(IOCON_RES20_PE0_LCDLP_LCDHRLP) &
      IOCON_MUX_MASK(IOCON_RES20_PF7_LCDFP_LCDSPS) &
      IOCON_MUX_MASK(IOCON_RES20_PF6_LCDEN_LCDSPL));
  temp &= IOCON->mux_ctl_20;

  temp |= (IOCON_MUX20_LCDPS | IOCON_MUX20_LCDDCLK |
      IOCON_MUX20_LCDHRLP | IOCON_MUX20_LCDSPS | IOCON_MUX20_LCDSPL);
  IOCON->mux_ctl_20 = temp;

  temp = (IOCON_MUX_MASK(IOCON_RES21_PF5_LCDVD11) & 
      IOCON_MUX_MASK(IOCON_RES21_PL3_LCDVD13) &
      IOCON_MUX_MASK(IOCON_RES21_PF4_LCDVD10) &
      IOCON_MUX_MASK(IOCON_RES21_PL2_LCDVD12) &
      IOCON_MUX_MASK(IOCON_RES21_PF3_LCDVD9) &
      IOCON_MUX_MASK(IOCON_RES21_PF2_LCDVD8));
  temp &= IOCON->mux_ctl_21;

  temp |= (IOCON_MUX21_LCDVD11 | IOCON_MUX21_LCDVD13 |
      IOCON_MUX21_LCDVD10 | IOCON_MUX21_LCDVD12 | 
      IOCON_MUX21_LCDVD9 | IOCON_MUX21_LCDVD8);
  IOCON->mux_ctl_21 = temp;
  
  temp = (IOCON_MUX_MASK(IOCON_RES22_PF1_LCDVD7) & 
      IOCON_MUX_MASK(IOCON_RES22_PF0_LCDVD6) &
      IOCON_MUX_MASK(IOCON_RES22_PG7_LCDVD5) &
      IOCON_MUX_MASK(IOCON_RES22_PG6_LCDVD4) &
      IOCON_MUX_MASK(IOCON_RES22_PG5_LCDVD3) &
      IOCON_MUX_MASK(IOCON_RES22_PG4_LCDVD2) &
      IOCON_MUX_MASK(IOCON_RES22_PG3_LCDVD1) &
      IOCON_MUX_MASK(IOCON_RES22_PG2_LCDVD0));
  temp &= IOCON->mux_ctl_22;

  temp |= (IOCON_MUX22_LCDVD7 | IOCON_MUX22_LCDVD6 |
      IOCON_MUX22_LCDVD5 | IOCON_MUX22_LCDVD4 | 
      IOCON_MUX22_LCDVD3 | IOCON_MUX22_LCDVD2 | 
      IOCON_MUX22_LCDVD1 | IOCON_MUX22_LCDVD0);
  IOCON->mux_ctl_22 = temp;
}

/***********************************************************************
 *
 * Function: lcd_TFT_iocon_init
 *
 * Purpose: Enable muxed LCD controller pins for TFT LCD
 *
 * Processing:
 *     Select LCD pins in IOCON mux control registers.
 *
 * Parameters:
 *     none
 *
 * Outputs: None
 *
 * Returns: None
 *
 **********************************************************************/

static void lcd_TFT_iocon_init (void)
{
  UNS_32 temp = 0;
  
  temp = (IOCON_MUX_MASK(IOCON_RES1_PL1_LCDVD15) & 
      IOCON_MUX_MASK(IOCON_RES1_PL0_LCDVD14));
  temp &= IOCON->mux_ctl_1;

  temp |= (IOCON_MUX1_LCDVD14 | IOCON_MUX1_LCDVD15);
  IOCON->mux_ctl_1 = temp;


  temp = (IOCON_MUX_MASK(IOCON_RES19_PE6_LCDVEEN_LCDMOD) & 
      IOCON_MUX_MASK(IOCON_RES19_PE4_LCDSPLEN_LCDREV) &
      IOCON_MUX_MASK(IOCON_RES19_PE5_LCDVDDEN) &
      IOCON_MUX_MASK(IOCON_RES19_PE3_LCDCLS));

  temp &= IOCON->mux_ctl_19;
  temp |= (IOCON_MUX19_LCDMOD | IOCON_MUX19_LCDVDDEN | IOCON_MUX19_LCDVEEN |
      IOCON_MUX19_LCDREV | IOCON_MUX19_LCDCLS);
  IOCON->mux_ctl_19 = temp;
      

  temp = (IOCON_MUX_MASK(IOCON_RES20_PE2_LCDPS) & 
      IOCON_MUX_MASK(IOCON_RES20_PE1_LCDDCLK) &
      IOCON_MUX_MASK(IOCON_RES20_PE0_LCDLP_LCDHRLP) &
      IOCON_MUX_MASK(IOCON_RES20_PF7_LCDFP_LCDSPS) &
      IOCON_MUX_MASK(IOCON_RES20_PF6_LCDEN_LCDSPL));
  temp &= IOCON->mux_ctl_20;

  temp |= (IOCON_MUX20_LCDPS | IOCON_MUX20_LCDDCLK |
      IOCON_MUX20_LCDLP | IOCON_MUX20_LCDFP | IOCON_MUX20_LCDEN);
  IOCON->mux_ctl_20 = temp;

  temp = (IOCON_MUX_MASK(IOCON_RES21_PF5_LCDVD11) & 
      IOCON_MUX_MASK(IOCON_RES21_PL3_LCDVD13) &
      IOCON_MUX_MASK(IOCON_RES21_PF4_LCDVD10) &
      IOCON_MUX_MASK(IOCON_RES21_PL2_LCDVD12) &
      IOCON_MUX_MASK(IOCON_RES21_PF3_LCDVD9) &
      IOCON_MUX_MASK(IOCON_RES21_PF2_LCDVD8));
  temp &= IOCON->mux_ctl_21;

  temp |= (IOCON_MUX21_LCDVD11 | IOCON_MUX21_LCDVD13 |
      IOCON_MUX21_LCDVD10 | IOCON_MUX21_LCDVD12 | 
      IOCON_MUX21_LCDVD9 | IOCON_MUX21_LCDVD8);
  IOCON->mux_ctl_21 = temp;
  
  temp = (IOCON_MUX_MASK(IOCON_RES22_PF1_LCDVD7) & 
      IOCON_MUX_MASK(IOCON_RES22_PF0_LCDVD6) &
      IOCON_MUX_MASK(IOCON_RES22_PG7_LCDVD5) &
      IOCON_MUX_MASK(IOCON_RES22_PG6_LCDVD4) &
      IOCON_MUX_MASK(IOCON_RES22_PG5_LCDVD3) &
      IOCON_MUX_MASK(IOCON_RES22_PG4_LCDVD2) &
      IOCON_MUX_MASK(IOCON_RES22_PG3_LCDVD1) &
      IOCON_MUX_MASK(IOCON_RES22_PG2_LCDVD0));
  temp &= IOCON->mux_ctl_22;

  temp |= (IOCON_MUX22_LCDVD7 | IOCON_MUX22_LCDVD6 |
      IOCON_MUX22_LCDVD5 | IOCON_MUX22_LCDVD4 | 
      IOCON_MUX22_LCDVD3 | IOCON_MUX22_LCDVD2 | 
      IOCON_MUX22_LCDVD1 | IOCON_MUX22_LCDVD0);
  IOCON->mux_ctl_22 = temp;
}
