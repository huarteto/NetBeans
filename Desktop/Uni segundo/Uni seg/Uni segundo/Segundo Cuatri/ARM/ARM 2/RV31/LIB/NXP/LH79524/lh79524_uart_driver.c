/***********************************************************************
 * $Id:: lh79524_uart_driver.c 27 2007-08-31 19:35:06Z kevinw          $
 * 
 * Project: LH79524 CSP library
 * 
 * Description: 
 *      This file contains the implementation of the LH79524 UART driver
 *      for UARTS 0, 1 and 2.
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
#include "lh79524_iocon.h"
#include "lh79524_rcpc.h"
#include "lh79524_uart_driver.h"
 
/* Serial driver interrupt priorities */
typedef enum
{
  UART_TX_POS = 0,
  UART_RX_POS,
  UART_RT_POS,
  UART_FE_POS,
  UART_PE_POS,
  UART_BE_POS,
  UART_OE_POS
} UART_INT_PRIO_T;

/* Defines for the ip block base address and driver control object */
#define REGS_T   UART_REGS_T
#define DRIVER_T SERIAL_T

/* Forward declarations of private methods */
static void uart0_enable (BOOL_32 enable); 
static void uart1_enable (BOOL_32 enable); 
static void uart2_enable (BOOL_32 enable); 
static void uart0_isr (void); 
static void uart1_isr (void); 
static void uart2_isr (void); 
static void uart_isr (void*);
static void uart_rx_isr (void*);
static void uart_tx_isr (void*);
static void uart_err_isr (void*, INT_32);

/* Table of device base addresses */
static UNS_32 driverid[N_UARTS] = 
{
  UART0_BASE, /* uart0 base address */ 
  UART1_BASE, /* uart1 base address */
  UART2_BASE  /* uart2 base address */
};

/* Instansiations of the driver control object */
static DRIVER_T driver[N_UARTS];

/* Address of the driver control objects */
static DRIVER_T* pdriver[N_UARTS] = 
{
  (DRIVER_T*)&driver[0],
  (DRIVER_T*)&driver[1],
  (DRIVER_T*)&driver[2]
};

/* Address of the driver interrupt handlers  */
static void* driver_isr[N_UARTS] =
{
  (void*)uart0_isr,
  (void*)uart1_isr,
  (void*)uart2_isr
};

/***********************************************************************
* 
* Function: uart_open
* 
* Purpose: 
*  Initialize the specified 16C550 UART with the paramters specified.
* 
* Processing: 
*  Sets up the device control structure for the UART.
*
* Parameters: 
*  uart  - uart to open
*  arg   - XTAL System clock
* 
* Outputs: 
*  None
* 
* Returns: 
*  device id or 0
* 
* Notes: 
*  None
* 
**********************************************************************/

INT_32 uart_open (INT_32 ipbase, INT_32 arg)
{
  DRIVER_T* pdev       = NULL;      
  INT_32    dev_exists = FALSE;
  INT_32    devid      = 0;
  CHAR*     dst        = NULL;
  INT_32    idx        = 0;
  REGS_T*   pregs      = NULL;
  
  /* Find the device in the device table */
  for (devid = 0; devid < NELEMENTS(driver); devid++)
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
  
  /* Attach to the base address of the serial device */
  pdev = (DRIVER_T*)pdriver[devid];
  
  /* Make sure the device is not already opened */
  if (pdev->init == TRUE)
  {
    /* Device has already been opened */
    return (0);
  }
  
  /* Clear the serial driver control structure */
  dst = (CHAR*)pdev;
  for (idx = 0; idx < sizeof (DRIVER_T); idx++)
  {
    *dst++ = 0;
  }
  
  /* Attach to the base address of the selected uart */
  pdev->regs = (void*)driverid[devid];
  pregs      = (REGS_T*)pdev->regs;
  
  /*set the XTAL system clock */
  pdev->cfg.sys_clk = (UNS_32)arg;
  
  /* Init the uart clock and pin muxing (RX/TX lines) */
  switch(devid)
  {
  case 0:
    uart0_enable(TRUE);
    break;
  case 1:
    uart1_enable(TRUE);
    break;
  case 2:
    uart2_enable(TRUE);
    break;
  }
  
  /* Init the dma buffers */
  pdev->rx_dma = NULL; 
  pdev->tx_dma = NULL;
  
  /* Bind to the tx and rx ring buffers data sections */
  pdev->rx_id.data = (CHAR*)&pdev->rx_data[0];
  pdev->tx_id.data = (CHAR*)&pdev->tx_data[0];
  
  /* Init the ring buffers that are to be used for buffer IO */
  RING_INIT (&pdev->rx_id, NELEMENTS(pdev->rx_data));
  RING_INIT (&pdev->tx_id, NELEMENTS(pdev->tx_data));
  
  /* Mask all uart interrupts at the source */
  pregs->imsc = UART_IMASK_ALL;
  
  /* Clear any pending errors */
  pregs->rsr_ecr = UART_ECR_CLEAR;
  
  /* clear any pending interrupts  */
  pregs->icr = UART_CLR_ALL;
  
  /* Install the interrupt handlers */
  pdev->isr[UART_TX_POS] = (PFV)uart_tx_isr;
  pdev->isr[UART_RX_POS] = (PFV)uart_rx_isr;
  pdev->isr[UART_RT_POS] = (PFV)uart_rx_isr;
  pdev->isr[UART_FE_POS] = (PFV)uart_err_isr;
  pdev->isr[UART_PE_POS] = (PFV)uart_err_isr;
  pdev->isr[UART_BE_POS] = (PFV)uart_err_isr;
  pdev->isr[UART_OE_POS] = (PFV)uart_err_isr;
  
  /* Save the default handler isr */
  pdev->irq_hdlr = driver_isr[devid];
  
  /* set the device in default config state 115200-8-N-1*/
  pregs->ibrd = UART_BRINT(pdev->cfg.sys_clk, 115200);
  pregs->fbrd = UART_BRFRAC(pdev->cfg.sys_clk, 115200);
  pdev->cfg.baud_rate = 115200;
  pregs->lcr_h = UART_LCR_WLEN8;
  
  /* Mark the device as open */
  pdev->init = TRUE;
  
  /* return device id */
  return ((INT_32)pdev);
}

/***********************************************************************
* 
* Function: uart_close
* 
* Purpose: 
*  Turn the driver off
* 
* Processing: 
*  This routine flushes the device, disables interrupts, disables the
*  device, and marks it as closed.
*
* Parameters: 
*  devid - device id (UART0 or UART1)
* 
* Outputs: 
*  None
* 
* Returns: 
*  _NO_ERROR if device is closed successfully.
*  _ERROR otherwise.
* 
* Notes: None
* 
**********************************************************************/

STATUS uart_close (INT_32 devid)
{
  STATUS    status = _NO_ERROR;
  INT_32    arg    = 0;
  INT_32    idx    = 0;
  CHAR*     dst    = NULL;
  DRIVER_T* pdev   = NULL;
  
  /* Sanity check */
  if (devid == 0)
  {
    /* device not opened or non-valid file handle */
    return (_ERROR);
  }
  
  /* Bind to the device control structure */
  pdev = (DRIVER_T*)devid;
  
  /* Make sure the device is already opened */
  if (pdev->init == FALSE)
  {
    return (_ERROR);
  }
  
  /* Flush the device */
  (void) uart_ioctl (devid, UART_FLUSH, 0);
  
  /* Disable the rx and tx sections */
  (void) uart_ioctl (devid, UART_DISABLE_TX, 0);
  (void) uart_ioctl (devid, UART_DISABLE_RX, 0);
  
  /* Mask (Disable) off the interrupts */
  arg = UART_INT_TX | UART_INT_RX | 
    UART_INT_RT | UART_INT_FE | 
    UART_INT_PE | UART_INT_BE | 
    UART_INT_OE;
  uart_ioctl (devid, UART_DISABLE_INT, arg);
  
  /* Un-install the interrupts */
  pdev->isr[UART_TX_POS] = NULL;
  pdev->isr[UART_RX_POS] = NULL;
  pdev->isr[UART_RT_POS] = NULL;
  pdev->isr[UART_FE_POS] = NULL;
  pdev->isr[UART_PE_POS] = NULL;
  pdev->isr[UART_BE_POS] = NULL;
  pdev->isr[UART_OE_POS] = NULL;
  
  /* Turn the device off */
  (void) uart_ioctl (devid, UART_STOP, 0);
  
  /* Disable the uart clock and pin muxing (RX/TX lines) */
  switch(devid)
  {
  case 0:
    uart0_enable(FALSE);
    break;
  case 1:
    uart1_enable(FALSE);
    break;
  case 2:
    uart2_enable(FALSE);
    break;
  }
  
  /* Invalidate the device */
  pdev->init = FALSE;
  
  /* Clear the serial driver control structure */
  dst = (CHAR*)pdev;
  for (idx = 0; idx < sizeof (DRIVER_T); idx++)
  {
    *dst++ = 0;
  }
  
  /* Done */
  return (status);
}

/***********************************************************************
* 
* Function: uart_read
* 
* Purpose: 
*  Read the specified UART data.
* 
* Processing: 
*  Checks to see if the device is running in polled or interrupt mode.
*  In polled mode data is read directly from the uart rx fifo, while
*  in interrupt mode data is read from a ring buffer. Data is read 
*  until ring buffer is empty, the rx fifo is empty, or the max
*  number of bytes has been read. 
*
* Parameters: 
*  devid      - device id (UART0 or UART1)
*  buffer     - data buffer
*  max_bytes  - maximum number of bytes to read
* 
* Outputs: 
*  None
* 
* Returns: 
*  n_bytes - the actual number of bytes read 
*  
* Notes: 
*  None
* 
**********************************************************************/

INT_32 uart_read (INT_32 devid, 
                  CHAR*  buffer, 
                  INT_32 max_bytes)
{
  INT_32    n_bytes = 0;   
  DRIVER_T* pdev    = NULL;
  REGS_T*   pregs   = NULL;
  
  /* Sanity check */
  if ((buffer == NULL) || (devid == 0))
  {
    /* Null pointer */
    return (0);
  }
  
  /* Bind to the device control structure */
  pdev = (DRIVER_T*)devid;
  
  /* Make sure the device is already opened */
  if (pdev->init == FALSE)
  {
    return (0);
  }
  
  /* Attach to the base address of the selected uart (0 or 1) */
  pregs = (REGS_T*)pdev->regs;
  
  /* Check to see if we are in polled or interrupt mode */
  if (pdev->cfg.poll_mode == TRUE)
  {
    /* Read data from the receive buffer */
    for (n_bytes = 0; n_bytes < max_bytes; n_bytes++)
    {
      /* Make sure there is data in the receive fifo */
      if ((pregs->fr & UART_FR_RXFE) == 
        UART_FR_RXFE)
      {
        /* No more data */
        break;
      }
      /* Read data from the device */
      *buffer++ = pregs->dr;
    }  
  }
  else
  {
    /* get the data from the device */
    for (n_bytes = 0; n_bytes < max_bytes; n_bytes++)
    {
      /* Is there data in the ring buffer */
      if (RING_COUNT (&pdev->rx_id) != 0)
      {
        /* Add the data to the ring buffer */
        RING_GETC (&pdev->rx_id, *buffer++);
      }
      else
      {
        /* break out and return the number of bytes written */ 
        break;
      }
    }
  }
  
  /* return */
  return (n_bytes);
}

/***********************************************************************
* 
* Function: uart_write
* 
* Purpose: 
*  Write data to the UART.
* 
* Processing: 
*
* Parameters: 
*  devid   - device id (UART0 or UART1)
*  buffer  - data buffer 
*  n_bytes - number of bytes to write.
* 
* Outputs: 
*  None
* 
* Returns: 
*  actual_bytes - number of bytes written.
* 
* Notes: 
*  None
* 
**********************************************************************/

INT_32 uart_write (INT_32 devid, 
                   CHAR*  buffer, 
                   INT_32 n_bytes)
{
  UNS_32    actual_bytes = 0;   
  DRIVER_T* pdev         = NULL;
  REGS_T*   pregs        = NULL;
  
  /* Sanity check */
  if ((buffer == NULL) || (devid == 0))
  {
    /* Null pointer */
    return (0);
  }
  
  /* Bind to the device control structure */
  pdev = (DRIVER_T*)devid;
  
  /* Make sure the device is already opened */
  if (pdev->init == FALSE)
  {
    return (0);
  }
  
  /* Attach to the base address of the selected uart (0 or 1) */
  pregs = (REGS_T*)pdev->regs;
  
  /* Check to see if we are in polled or interrupt mode */
  if (pdev->cfg.poll_mode == TRUE)
  {
    /* Write the data into the transmit buffer */
    for (actual_bytes = 0; actual_bytes < n_bytes; actual_bytes++)
    {
      /* Wait for the transmit fifo to drain */
      while ((pregs->fr & UART_FR_TXFF) 
        == UART_FR_TXFF); 
      /* Write the data to the device */
      pregs->dr = *buffer++;
    }  
  }
  else
  {
    /* Lock the system irq */
    /*INT_LOCK();*/
    
    /* Clear pending interrupts */
    pregs->icr = UART_INT_TX;
    
    /* Write data into the transmit ring buffer */
    for (actual_bytes = 0; actual_bytes < n_bytes; actual_bytes++)
    {
      /* Load the ring buffer */
      RING_PUTC (&pdev->tx_id, *buffer++);
    }
    
    /* Kick start the tranasmitter */
    while ( (RING_COUNT(&pdev->tx_id) != 0) &&
      !(pregs->fr & UART_FR_TXFF) )      
    {
      /* Write the data to the Tx fifo */
      RING_GETC (&pdev->tx_id, pregs->dr);
    }
    
    /* UnMask (Enable) the tx interrupt */
    (void) uart_ioctl 
      (devid, UART_ENABLE_INT, UART_INT_TX);
    
    /* Enable the uart transmitter */
    (void) uart_ioctl (devid, UART_ENABLE_TX, 0);
    
    /* Unlock the system irq */
    /*INT_UNLOCK();*/
  }
  
  /* Return number of bytes written */
  return (actual_bytes);
}

/***********************************************************************
* 
* Function: uart_ioctl
* 
* Purpose: 
*  handle uart control requests.
* 
* Processing: Set/resets designated state in the device.
*  
*
* Parameters:
*  devid - device id
*  cmd   - request to process
*  arg   - generic argument 
* 
* Outputs: None
* 
* Returns: 
*  _NO_ERROR if device controlled successfully.
*  _ERROR otherwise.
* 
* Notes: None
* 
**********************************************************************/
                   
STATUS uart_ioctl (INT_32 devid, 
                   INT_32 cmd, 
                   INT_32 arg)
{
  UNS_32    temp   = 0; 
  INT_32    status = _NO_ERROR; 
  REGS_T*   pregs  = NULL;
  DRIVER_T* pdev   = NULL;
  
  /* Sanity check */
  if (devid == 0)
  {
    /* Null pointer */
    return (0);
  }
  
  /* Attach to the base address of the serial device */
  pdev = (DRIVER_T*)devid;
  
  /* Make sure the device is already opened */
  if (pdev->init == FALSE)
  {
    return (_ERROR);
  }
  
  /* Attach to the base address of the selected uart (0 or 1) */
  pregs = (REGS_T*)pdev->regs;
  
  /* Service the users request */
  switch (cmd)
  {
    /* Get and set requests for the baud rate */
  case UART_SET_BAUD_RATE:
    /* Save the new baud rate */
    pregs->ibrd = UART_BRINT(pdev->cfg.sys_clk,arg);
    pregs->fbrd = UART_BRFRAC(pdev->cfg.sys_clk,arg);
    /* 
    * read and write lcr_h to ensure baud 
    * divisors are set 
    */
    temp = pregs->lcr_h;
    pregs->lcr_h = temp;
    /* Save the config data */
    pdev->cfg.baud_rate = arg;
    break;
    
    /* Get the baud rate */
  case UART_GET_BAUD_RATE:
    if ((void*)arg == NULL)
    {
      status = _ERROR;
      break;
    }
    *(INT_32*)arg = pdev->cfg.baud_rate;
    break;
    
    /* Set tx fifo level */
  case UART_SET_TX_FIFO_LVL:
    temp = pregs->ifls;
    temp &= UART_TX_FIFO_MASK;
    temp |= ((UNS_32)arg<<0);
    pregs->ifls = temp; 
    /* Save the config data */
    pdev->cfg.tx_fifo_depth = arg;
    break;
    /* Get tx fifo level */
  case UART_GET_TX_FIFO_LVL:
    /* Enable the device */
    if ((void*)arg == NULL)
    {
      status = _ERROR;
      break;
    }
    *(INT_32*)arg = pdev->cfg.tx_fifo_depth;
    break;
    
    /* set rx fifo level */
  case UART_SET_RX_FIFO_LVL:
    temp = pregs->ifls;
    temp &= UART_RX_FIFO_MASK;
    temp |= ((UNS_32)arg<<3);
    pregs->ifls = temp; 
    /* Save the config data */
    pdev->cfg.rx_fifo_depth = arg;
    break;
    /* Get rx fifo level */
  case UART_GET_RX_FIFO_LVL:
    /* Enable the device */
    if ((void*)arg == NULL)
    {
      status = _ERROR;
      break;
    }
    *(INT_32*)arg = pdev->cfg.rx_fifo_depth;
    break;
    
    /* Get and set requests for the frame type (8-N-1 etc).*/
  case UART_SET_FRAME_OPTIONS:
    pregs->lcr_h = arg;
    /* Save the options */
    pdev->cfg.options = arg;
    break;
  case UART_GET_FRAME_OPTIONS:
    /* Enable the device */
    if ((void*)arg == NULL)
    {
      status = _ERROR;
      break;
    }
    *(INT_32*)arg = pdev->cfg.options;
    break;
    
    /* Return the uart interrupt service routine */
  case UART_GET_ISR:
    /* Sanity check */
    if ((void*)arg == NULL)
    {
      status = _ERROR;
      break;
    }
    /* Bind the ISR for uart to the arg pointer */
    *(INT_32**)arg = (void*)pdev->irq_hdlr;
    break;
    
    /* Enable polling mode */
  case UART_SET_POLL_MODE:
    /* Set the device into polled mode */
    pdev->cfg.poll_mode = TRUE;
    /* Mask all uart interrupts at the source */
    pregs->imsc = UART_IMASK_ALL;
    /* Clear any pending errors */
    pregs->rsr_ecr = UART_ECR_CLEAR;
    /* clear any pending interrupts  */
    pregs->icr = UART_CLR_ALL;
    break;
    
    /* Enable interrupt mode */
  case UART_SET_INT_MODE:
    /* Set the device into interrupt */
    pdev->cfg.poll_mode = FALSE;
    break;
    
    /* Get and set requests for the 16 byte fifo */
  case UART_ENABLE_FIFO:
    /* Enable the fifo */
    temp = pregs->lcr_h;
    temp |= UART_LCR_FEN; 
    pregs->lcr_h = temp;
    break;
    
    /* Disable 16 byte fifo */
  case UART_DISABLE_FIFO:
    /* Disable the fifo */
    temp = pregs->lcr_h;
    temp &= ~UART_LCR_FEN;
    pregs->lcr_h = temp;
    break;
    
    /* Mask (disable) interrupt source */
  case UART_DISABLE_INT:
    /* Mask the desired interrupt */
    temp = pregs->imsc;
    temp &= ~arg;
    pregs->imsc = temp;
    break;
    /* UnMask (enable) interrupt source */
  case UART_ENABLE_INT:
    /* Unmask the desired interrupt */
    temp = pregs->imsc;
    temp |= arg;
    pregs->imsc = temp;
    /* Save the state */
    pdev->imask = temp;
    break;
    
    /* enable loopback */
  case UART_ENABLE_LOOPBACK:
    temp = pregs->cr;
    temp |= UART_CR_LBE;
    pregs->cr = temp;
    break;
    /* disable loopback */
  case UART_DISABLE_LOOPBACK:
    temp = pregs->cr;
    temp &= ~UART_CR_LBE;
    pregs->cr = temp;
    break;
    
    /* Flush the Rx and Tx fifos and ring buffers */
  case UART_FLUSH:
    /* Save state */
    temp = pregs->cr;
    
    /* Enable the receiver */
    pregs->cr = UART_CR_RXE | UART_CR_ENABLE;
    
    /* Drain the reciever */            
    while (!(pregs->fr & UART_FR_RXFE)) 
    {
      temp = pregs->dr;
    }
    
    /* Enable the tranmitter */
    pregs->cr = UART_CR_TXE | UART_CR_ENABLE; 
    
    /* Wait for transmitter to drain */
    while (!(pregs->fr & UART_FR_TXFE));
    
    /* Restore state */
    pregs->cr = temp; 
    
    /* reset the fifo tx and rx buffers */
    RING_FLUSH (&pdev->rx_id);
    RING_FLUSH (&pdev->tx_id);
    break;
    
    /* Enable and disable the reciever */
  case UART_ENABLE_RX:
    /* Enable the receiver */
    temp = pregs->cr;
    temp |= UART_CR_RXE;
    pregs->cr = temp;
    break;
  case UART_DISABLE_RX:
    /* Disable the receiver */
    temp = pregs->cr;
    temp &= ~UART_CR_RXE;
    pregs->cr = temp;
    break;
    
    /* Enable and disable the transmitter */
  case UART_ENABLE_TX:
    /* Enable the transmitter */
    temp = pregs->cr;
    temp |= UART_CR_TXE;
    pregs->cr = temp;
    break;
  case UART_DISABLE_TX:
    /* Disable the transmitter */
    temp = pregs->cr;
    temp &= ~UART_CR_TXE;
    pregs->cr = temp;
    break;
    
    /* Enable and disable the device */
  case UART_START:
    temp = pregs->cr;
    temp |= UART_CR_ENABLE;
    pregs->cr = temp;
    break;
  case UART_STOP:
    /* Disable the device */
    temp = pregs->cr;
    temp &= ~UART_CR_ENABLE;
    pregs->cr = temp;
    break;
    
  default:
    status = _ERROR;
    break;
    }
    
    /* Return the io control status */
    return (status);
}

/***********************************************************************
* 
* Function: uart0_enable
* 
* Purpose: 
*  Enable multiplexed uart0 pins and clock.
* 
* Processing: 
*  Configure IOCON and RCPC registers.
*
* Parameters: 
*  enable - Specified whether to enable or diable the port
* 
* Outputs: None
* 
* Returns: None  
* 
* Notes: None
* 
**********************************************************************/

static void uart0_enable (BOOL_32 enable) 
{
  UNS_32 muxval = 0;
  
  if(enable == FALSE)
  {
    /* disable the clock */
    RCPC->periphclkctrl0 |= RCPC_CLKCTRL0_U0_DISABLE;
    return;
  }
  
  /* read the mux register */
  muxval = IOCON->mux_ctl_5;
  /* clear the required fields */
  muxval &= ~(_SBF(IOCON_RES5_PB6_INT0_UARTRX0_UARTIRRX0,0x11) |
    _SBF(IOCON_RES5_PB7_INT1_UARTTX0_UARTIRTX0,0x11));
  /* select URTTX0 & UARTRX0 */ 
  muxval |= IOCON_MUX5_UARTTX0 | IOCON_MUX5_UARTRX0;
  IOCON->mux_ctl_5 = muxval; 
  
  /* read the mux register */
  muxval = IOCON->mux_ctl_6;
  /* clear the required fields */
  muxval &= ~(_SBF(IOCON_RES6_PB1_DREQ_UARTRTS0,0x11) |
    _SBF(IOCON_RES6_PB0_DACK_UARTCTS0,0x11));
  /* select URTTX0 & UARTRX0 */ 
  muxval |= IOCON_MUX6_UARTRTS0 | IOCON_MUX6_UARTCTS0;
  IOCON->mux_ctl_6 = muxval; 
  
  /* now enable the UART0 clock */  
  RCPC->periphclkctrl0 &= ~RCPC_CLKCTRL0_U0_DISABLE;
  
}

/***********************************************************************
* 
* Function: uart1_enable
* 
* Purpose: 
*  Enable multiplexed uart1 pins and clock.
* 
* Processing: 
*  Configure IOCON and RCPC registers.
*
* Parameters: 
*  enable - Specified whether to enable or diable the port
* 
* Outputs: None
* 
* Returns: None  
* 
* Notes: None
* 
**********************************************************************/

static void uart1_enable (BOOL_32 enable) 
{
  UNS_32 muxval = 0;
  
  if(enable == FALSE)
  {
    /* disable the clock */
    RCPC->periphclkctrl0 |= RCPC_CLKCTRL0_U1_DISABLE;
    return;
  }
  
  /* read the mux register */
  muxval = IOCON->mux_ctl_5;
  /* clear the required fields */
  muxval &= ~(_SBF(IOCON_RES5_PB4_SSPRX_I2STXD_UARTRX1_UARTIRRX1,0x11) |
    _SBF(IOCON_RES5_PB5_SSPTX_I2STXD_UARTTX1_UARTIRTX1,0x11));
  /* select URTTX1 & UARTRX1 */ 
  muxval |= IOCON_MUX5_UARTTX1 | IOCON_MUX5_UARTRX1;
  IOCON->mux_ctl_5 = muxval; 
  
  /* now enable the UART1 clock */  
  RCPC->periphclkctrl0 &= ~RCPC_CLKCTRL0_U1_DISABLE;
  
}

/***********************************************************************
* 
* Function: uart2_enable
* 
* Purpose: 
*  Enable multiplexed uart2 pins and clock.
* 
* Processing: 
*  Configure IOCON and RCPC registers.
*
* Parameters: 
*  enable - Specified whether to enable or diable the port
* 
* Outputs: None
* 
* Returns: None  
* 
* Notes: None
* 
**********************************************************************/

static void uart2_enable (BOOL_32 enable) 
{
  UNS_32 muxval = 0;
  
  if(enable == FALSE)
  {
    /* disable the clock */
    RCPC->periphclkctrl0 |= RCPC_CLKCTRL0_U2_DISABLE;
    return;
  }
  
  /* read the mux register */
  muxval = IOCON->mux_ctl_5;
  /* clear the required fields */
  muxval &= ~(_SBF(IOCON_RES5_PA0_INT2_UARTRX2_UARTIRRX2,0x11) |
    _SBF(IOCON_RES5_PA1_INT3_UARTTX2_UARTIRTX2,0x11));
  /* select URTTX0 & UARTRX0 */ 
  muxval |= IOCON_MUX5_UARTRX2 | IOCON_MUX5_UARTTX2;
  IOCON->mux_ctl_5 = muxval; 
  
  /* now enable the UART0 clock */  
  RCPC->periphclkctrl0 &= ~RCPC_CLKCTRL0_U2_DISABLE;
  
}

/***********************************************************************
* 
* Function: uart0_isr
* 
* Purpose: 
*  handle uart0 interrupts, sets device control struct and calls common
*        int handler
* 
* Processing: 
*  Assigns appropriate value to p_dev pointer and calls common routine.
*
* Parameters: None
* 
* Outputs: None
* 
* Returns: None  
* 
* Notes: None
* 
**********************************************************************/

static void uart0_isr (void) 
{
  uart_isr ((void*)pdriver[0]);
}


/*********************************************************************** 
* 
* Function: uart1_isr
* 
* Purpose: 
*  handle uart1 interrupts, sets device control struct and calls common
*        int handler
* 
* Processing: 
*  Assigns appropriate value to p_dev pointer and calls common routine.
*
* Parameters: None
* 
* Outputs: None
* 
* Returns: None  
* 
* Notes: None
* 
**********************************************************************/

static void uart1_isr (void) 
{
  uart_isr ((void*)pdriver[1]);
}

/*********************************************************************** 
* 
* Function: uart2_isr
* 
* Purpose: 
*  handle uart2 interrupts, sets device control struct and calls common
*        int handler
* 
* Processing: 
*  Assigns appropriate value to p_dev pointer and calls common routine.
*
* Parameters: None
* 
* Outputs: None
* 
* Returns: None  
* 
* Notes: None
* 
**********************************************************************/

static void uart2_isr (void) 
{
  uart_isr ((void*)pdriver[2]);
}

/***********************************************************************
* 
* Function: uart_isr
* 
* Purpose: 
*  common handler for uart01 interrupts
* 
* Processing: 
*  Reads interrupt status and tests for bit set.  Clears error ints.
*  RX calls handler function.  TX int loads data from ring buffer to 
*  device until FIFO is full.
*
* Parameters: 
*  p_dev: pointer to device control structure.
* 
* Outputs: None
* 
* Returns: None  
* 
* Notes: 
*  For CONSISTENCY with the UART2 drivers, the priority is hard 
*  coded.
* 
**********************************************************************/

static void uart_isr (void* dev) 
{
  UNS_16    imask = 0; /* value of masked interrupts */
  DRIVER_T* pdev  = (DRIVER_T*)dev;
  REGS_T*   pregs = (REGS_T*)pdev->regs;
  
  /* Get status of currently masked interrupts */
  imask = pregs->mis;
  
  /* uart overrun error */
  if ((imask & UART_INT_OE) == UART_INT_OE)
  {
    pdev->isr[UART_OE_POS](dev, UART_INT_OE);
  }
  
  /* uart line break error */
  if ((imask & UART_INT_BE) == UART_INT_BE)
  {
    pdev->isr[UART_BE_POS](dev, UART_INT_BE);
  }
  
  /* uart parity bit error */
  if ((imask & UART_INT_PE) == UART_INT_PE)
  {
    pdev->isr[UART_PE_POS](dev, UART_INT_PE);
  }
  
  /* uart framing error */
  if ((imask & UART_INT_FE) == UART_INT_FE)
  {
    pdev->isr[UART_FE_POS](dev, UART_INT_FE);
  }
  
  /* uart transmitter empty */
  if ((imask & UART_INT_TX) == UART_INT_TX)
  {
    pdev->isr[UART_TX_POS](dev);
  } 
  /* uart reciever is full */
  else if ((imask & UART_INT_RX) == UART_INT_RX)
  {
    pdev->isr[UART_RX_POS](dev);
  }
  /* uart receiver timeout error */
  else if ((imask & UART_INT_RT) == UART_INT_RT)
  {
    pdev->isr[UART_RT_POS](dev);
  }
}

/***********************************************************************
* 
* Function: uart_rx_isr
* 
* Purpose: 
*  handler for uart01 read and read timeout interrupts
* 
* Processing: 
*  Loads the data into the ring buffer and clears the interrupt
*
* Parameters: p_dev: pointer to device control structure.
* 
* Outputs: None
* 
* Returns: None
* 
* Notes: None
* 
**********************************************************************/

static void uart_rx_isr (void* dev) 
{
  DRIVER_T* pdev  = (DRIVER_T*)dev;
  REGS_T*   pregs = (REGS_T*)pdev->regs;
  
  /* While there is data in the fifo */
  while ( !(pregs->fr & UART_FR_RXFE) )
  {
    /* There is data so pass it up the sio channel buffer */
    RING_PUTC (&pdev->rx_id, (CHAR)pregs->dr);
  }
  
  /* Clear the interrupt */
  pregs->icr |= (UART_INT_RX | UART_INT_RT);
  
  return;
}

/***********************************************************************
* 
* Function: uart_tx_isr
* 
* Purpose: 
*  handler for uart write interrupts
* 
* Processing: 
*  loads data from ring buffer to device until FIFO is full.
*
* Parameters: p_dev: pointer to device control structure.
* 
* Outputs: None
* 
* Returns: None
* 
* Notes: None
* 
**********************************************************************/

static void uart_tx_isr (void* dev)
{
  DRIVER_T* pdev  = (DRIVER_T*)dev;
  REGS_T*   pregs = (REGS_T*)pdev->regs;
  
  /* clear the interrupt at the source */
  pregs->icr = UART_INT_TX;
  
  /* Make sure there is data to be sent */
  if ( (RING_COUNT (&pdev->tx_id) == 0) &&
    (pregs->fr & UART_FR_TXFE) &&
    !(pregs->fr & UART_FR_BUSY) )
  {
    /* Turn off the Tx interrupts */
    (void) uart_ioctl 
      ((INT_32)dev, UART_DISABLE_INT, UART_INT_TX);
    
    /* Clear the interrupt */
    pregs->icr = UART_INT_TX;
    
    /* Disable the transmitter */
    (void) uart_ioctl ((INT_32)dev, UART_DISABLE_TX, 0);
  }
  else
  {
    /* There is data so fill the fifo */
    while ( (RING_COUNT (&pdev->tx_id) != 0)  &&
      !(pregs->fr & UART_FR_TXFF) )
    {
      /* Write the data to the device */
      RING_GETC (&pdev->tx_id, pregs->dr);
    }
  } 
}

/***********************************************************************
* 
* Function: uart_err_isr
* 
* Purpose: 
*  handler for uart error interrupts
* 
* Processing: 
*  Clears the interrupt
*
* Parameters: p_dev: pointer to device control structure.
* 
* Outputs: None
* 
* Returns: None
* 
* Notes: None
* 
**********************************************************************/

static void uart_err_isr (void*  dev, 
                          INT_32 src) 
{
  DRIVER_T* pdev  = (DRIVER_T*)dev;
  REGS_T*   pregs = (REGS_T*)pdev->regs;
  /* clear the interrupt */
  pregs->icr = src;
}




