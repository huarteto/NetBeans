//-----------------------------------------------------------------------------
// 91X_MEMS
//-----------------------------------------------------------------------------

#include "91x_mems.h"

/* SSP Flag Mask */
#define SSP_Flag_Mask  0x001F

/* SSP Flags */
#define SSP_FLAG_Busy	         0x0024
#define SSP_FLAG_RxFifoFull	 0x0023
#define SSP_FLAG_RxFifoNotEmpty	 0x0022
#define SSP_FLAG_TxFifoNotFull	 0x0021
#define SSP_FLAG_TxFifoEmpty	 0x0020
#define SSP_FLAG_TxFifo	         0x0043
#define SSP_FLAG_RxFifo	         0x0042
#define SSP_FLAG_RxTimeOut	 0x0041
#define SSP_FLAG_RxOverrun	 0x0040

t_mems_data mems_data = {0,0,0,0,0,0};

//-----------------------------------------------------------------------------
//-------------------               SPI                 -----------------------
//-----------------------------------------------------------------------------
// Initializes all peripherals related to communication with MEMs sensor
void Periph_Config_MEMs(){

  // Configurate pins 2.4(SCLK) and 2.5(MOSI) for SPI
  // Configurate pins 2.6(MISO) and 2.7(NSS) for SPI
  SCU->GPIOOUT[2]  = 0x0A00;  // AltOut1/AlrOut2
  SCU->GPIOIN[2]   = 0xF0;    // Peripheral connected
  SCU->GPIOTYPE[2] = 0x00;    // PushPull
  GPIO2->DDR       = 0x30;    // Output/Input

  // Configurate pin 8.1(NNS-manual) for SPI -   MEMS!!!
  SCU->GPIOTYPE[8] = 0x00;    // PushPull
  GPIO8->DDR       = 0x02;    // Output
  SCU->GPIOEMI     = 0x00;    // GPIO8 and 9 not connected to EMI
  GPIO8->DR[0xFF*4] = 0x02;   // unselect MEMs

  // Configure SSP0
  SSP0->CR0 = 0x00C7; // Motorla Frame Format, Serial Clock Held high when no data, 
                      // Data captured on 2nd edge, data size 8bits
  SSP0->PR  = 0x0008; // SSP prescaler set to 8 -> 3 MHz SPI clocks
  SSP0->CR1 = 0x0002;  // Master Mode + Enable SPP

}

//-----------------------------------------------------------------------------
// Check status flag of SSPx peripheral                              
FlagStatus SSP_GetFlagStatus(SSP_TypeDef* SSPx, u16 SSP_FLAG)
{
    u32 SSPReg = 0, FlagPos = 0;
    u32 StatusReg = 0;

  /* Get the SSP register index */
  SSPReg = SSP_FLAG >> 5;

  /* Get the flag position */
  FlagPos = SSP_FLAG & SSP_Flag_Mask;

  /* Find the register of the flag to check */
  if(SSPReg == 1)
  {
    /* The flag to check is in SR register */
    StatusReg = SSPx->SR;  	
  }
  else if (SSPReg == 2)
  {
    /* The flag to check is in RISR register */
    StatusReg = SSPx->RISR;
  }

  /* Check the status of the specified SSP flag */
  if((StatusReg & (1 << FlagPos)) != RESET)
  {
    /* Return SET if the SSP flag is set */
    return SET;
  }
  else
  {
    /* Return RESET if the SSP flag is reset */
    return RESET;
  }
}

//-----------------------------------------------------------------------------
// Writes data to MEMs register via SPI
void spi_write_reg (u8 reg_addr, u8 reg_value)
{
  u16 addr = reg_addr + MEMS_SPI_WRITE;

  GPIO8->DR[0xFF*4] = 0x00; // Drive NSS-manual pin

  // Send Address byte
  SSP0->DR = addr;
  // Send Data byte
  SSP0->DR = reg_value;

  while(!SSP_GetFlagStatus(SSP0, SSP_FLAG_TxFifoEmpty));
  // Empty receive FIFO
  addr = SSP0->DR;
  addr = SSP0->DR;

  GPIO8->DR[0xFF*4] = 0x02; // Drive NSS-manual pin
}

//-----------------------------------------------------------------------------
// Reads data from MEMs register via SPI
void spi_read_reg (u8 reg_addr, u8 *pBuffer, u8 NoOfBytes)
{
  u8 i;
  u16 addr = reg_addr + MEMS_SPI_READ;
  u16 dummy = 0xAA58;

  if (NoOfBytes > 1) 
    addr += MEMS_SPI_MULTIPLE_BYTES;
  
  GPIO8->DR[0xFF*4] = 0x00; // Drive NSS-manual pin
  
  // Send Address byte
  SSP0->DR = addr;   
  // Receive Data byte(s)
  for(i=0; i < NoOfBytes; i++) {
    SSP0->DR = dummy;
  }  
  while(!SSP_GetFlagStatus(SSP0, SSP_FLAG_TxFifoEmpty)); // wait to send all data
  
  addr = SSP0->DR;
  for(i=0; i < NoOfBytes; i++) {
      *(pBuffer+i) = SSP0->DR;
  }
  GPIO8->DR[0xFF*4] = 0x02; // Drive NSS-manual pin

}
//-----------------------------------------------------------------------------
// Initializes MEMs sensor
void SPI_MEMS_Init ()
{
  spi_write_reg (STATUS_REG, 0x00);    //no flag
  spi_write_reg (FF_WU_CFG, 0x00);     // all off
  spi_write_reg (DD_CFG, 0x00);     // all off
  spi_write_reg (CTRL_REG2, LIS_FS_2G + LIS_BDU_CONTINUOUS + LIS_BLE_XX + LIS_DAS_12BIT + LIS_DRDY_ENABLE);
  spi_write_reg (CTRL_REG1, LIS_PD_ON + LIS_DF_BY128 + LIS_ST_NORMAL + LIS_EA_ALL);       
}

//-----------------------------------------------------------------------------
// Reads MEMs data from the sensor cia SPI
void SPI_MEMS_Read (t_mems_data *mems_data)
{
  u8 spi_buffer[6];
  
  spi_read_reg (OUTX_L, spi_buffer, 6);

  mems_data->outx_h = spi_buffer[0];
  mems_data->outx_l = spi_buffer[1];
  mems_data->outy_h = spi_buffer[2];
  mems_data->outy_l = spi_buffer[3];
  mems_data->outz_h = spi_buffer[4];
  mems_data->outz_l = spi_buffer[5];
  
}
//-----------------------------------------------------------------------------

