/******************** (C) COPYRIGHT 2005 STMicroelectronics ********************
* File Name          : boardtest.c
* Author             : MCD Application Team
* Date First Issued  : 09/27/2005 :  V1.0
* Description        : This file provides functions to test the MB403A board.
********************************************************************************
* History:
* 09/27/2005 :  V1.0
********************************************************************************
 THE PRESENT SOFTWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS WITH
 CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
 AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT, INDIRECT
 OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE CONTENT
 OF SUCH SOFTWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING INFORMATION
 CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Standard include ----------------------------------------------------------*/
#include "main.h"

/* Include of other module interface headers ---------------------------------*/
/* Local includes ------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define  I2C_EE_SectorSize 0xFF
#define  SPI_EE_Size       0x400

#define UART0_Rx_Pin 0x0100   
#define UART0_Tx_Pin 0x0200   

#define UART1_Rx_Pin 0x0200   
#define UART1_Tx_Pin 0x0100   

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
UART_InitTypeDef UART_InitStructure;
GPIO_InitTypeDef GPIO_InitStructure;

bool BoardTestDone = FALSE;
extern  vu8 TemperatureUnit;

u8 test_rslt[10] = {0,0,0,0,0,0,0,0,0,0};
string BoardTestFailedmsg[10]; 

string BoardTestmsg[10]={"PushButtons Test", "Red Leds Test   ", "PWM Leds Test   ",
                         "LM35 Test       ", "Var Resist Test ", "Buzzer SW10:1-2 ", 
                         "Buzzer SW10:2-3 ", "I2C Test        ", "BSPI Test       ",
                         "UART Test       "};
                                                 
u8 Tx_I2Cmsg[] = "MB403 I2C EEPROM Test";
u8 Tx_SPImsg[] = "MB403 SPI EEPROM Test";
u8 Tx_UARTmsg[] = "MB403 UART Test ";
u8 Rx_msg[22] = "";

/* Private function prototypes -----------------------------------------------*/
static void Delay(u32 nCount);

/* Interface functions -------------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
* Function Name  : BoardTest
* Description    : main function of the board test application.
* Input          : None
* Output         : BoardTestDone: this variable is set to indicate that the board
*                  test was done. 
* Return         : None
*******************************************************************************/
void BoardTest(void)
{
  u8 i = 0;
   
  IntExtOnOffConfig(DISABLE);
  
  LedShow(DISABLE);
  BicolorShow(DISABLE);
  
  test_rslt[i++] = PushButtonsTest();
  
  test_rslt[i++] = RedLedsTest(); 
  
  test_rslt[i++] = PWMLedsTest(); 

  test_rslt[i++] = LM35Test(); 
  
  test_rslt[i++] = VarResistTest(); 

  test_rslt[i++] = Buzzer12Test(); 
  
  test_rslt[i++] = Buzzer23Test(); 
	
  test_rslt[i++] = I2CTest(); 
 
  test_rslt[i++] = BSPITest();
  
  test_rslt[i++] = UARTTest();

  IntExtOnOffConfig(ENABLE);
  
  LedShow(ENABLE);
  BicolorShow(ENABLE);
  
  BoardTestDone = TRUE;
  
  BackFunc();  
  
} 

/*******************************************************************************
* Function Name  : TestReport
* Description    : Retrieves the board test report.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/ 
void TestReport(void)
{
  u8 i = 0, TestFailedCount = 0, Count = 0;
  u8 MyKey = 0;
  
  if(BoardTestDone == TRUE)
  {    
    LCD_SendString(Overalltestmsg, 1, 1);
    for(i = 0; i < 10; i++)
    {
      if(test_rslt[i] == 0)
      {
        BoardTestFailedmsg[TestFailedCount] = BoardTestmsg[i];
        TestFailedCount++;      
      }   
    }     
    
    if(TestFailedCount == 0)
    { 
      LCD_SendStringPinPong(Passedmsg, 2, 12);
      LCD_SendString(Passedmsg, 2, 3);
      
      IntExtOnOffConfig(DISABLE);
      while(( ReadKey() != BACK));
      IntExtOnOffConfig(ENABLE);
      BackFunc();
    }
    else
    { 
      LCD_SendStringPinPong(Failedmsg, 2, 12);  	
      Delay(0xAFFFF);
    
      LCD_LineClear(1);
      LCD_SetPosCur(1, 2);
      LCD_SendData((TestFailedCount/10)+48);
      LCD_SendData((TestFailedCount%10)+48);

      LCD_SendString(TestFailedmsg, 1, 5);        
    
      IntExtOnOffConfig(DISABLE);  
   
      LCD_SendString(BoardTestFailedmsg[Count], 2, 1);
    
      while(TRUE)
      { 
      	MyKey = ReadKey();
      		
        if(MyKey == NXT)
        {

          if(Count >= (TestFailedCount-1))
          {
            Count = 0;
          }
          else
          {
            Count++;
          }      
        }    
        else if(MyKey == PRV)
        { 
          if(Count >0)
          {
            Count--;
          }
          else
          {
            Count = TestFailedCount - 1;
          }                
        }
        else if(MyKey == BACK)
        {
          IntExtOnOffConfig(ENABLE);
          BackFunc();
          return;
        }
      LCD_SendString(BoardTestFailedmsg[Count], 2, 1);  
      }  
    }	
  }
  else
  { 
    LCD_Clear();
    LCD_SendStringByStep(Bordtestnotdonemsg,1,1);
    Delay(0xFFFFF);
    LCD_Clear();
    Delay(0xFFF);    
    LCD_SendStringByStep(Bordtestnotdonemsg,1,1);
    Delay(0xFFFFF);
    BackFunc();
  }
}

/*******************************************************************************
* Function Name  : msgcmp
* Description    : Compares Tx_msg and Rx_msg buffers values.
* Input          : - Tx_msg, Rx_msg: buffers to be compared.
*                : - msglength: buffer's length
* Output         : None
* Return         : PASS: Tx_msg identical to Rx_msg
*                  FAIL: Tx_msg differs from Rx_msg
*******************************************************************************/
TestStatus msgcmp(u8* Tx_msg, u8* Rx_msg, u16 msglength)
{
  u8 *msg;
  TestStatus status = PASS;

  msg = Passedmsg;
  
  while((msglength--) && (status == PASS))
  {
    if(*Tx_msg != *Rx_msg)
    {
      status = FAIL;
      msg = Failedmsg;
    }
    
    Tx_msg++;
    Rx_msg++;
  }
  
  LCD_SendStringPinPong(msg, 2, 12);
  
  LCD_SendString(NEXTmsg, 2, 1);
  while(ReadKey() != NXT);

  return status;
}

/*******************************************************************************
* Function Name  : IsTestOk
* Description    : Checks whether an elementary test is passed or not.
* Input          : None
* Output         : None
* Return         : PASS: test passed
*                  FAIL: test failed
*******************************************************************************/
TestStatus IsTestOk(void)
{
  u8 MyKey = 0;

  LCD_SendString(Passmsg, 1, 1);
  LCD_SendString(Failmsg, 2, 1);
  
  while(1)
  {
    MyKey = ReadKey();
    
    if(MyKey == SEL)
    {
      return PASS;
    }
    else if(MyKey == BACK)
    {
      return FAIL;
    }
  }
}
/*******************************************************************************
* Function Name  : PushButtonsTest
* Description    : Tests push-button.
* Input          : None
* Output         : None
* Return         : PASS: test passed
*******************************************************************************/
TestStatus PushButtonsTest(void)
{
  LCD_SendString(BoardTestmsg[0], 1, 1);
  
  /* test SEL push button */
  LCD_SendString(SELmsg, 2, 1);  
  while(GPIO_BitRead(GPIO5,GPIO_PIN_10) == 0);
  while(GPIO_BitRead(GPIO5,GPIO_PIN_10) == 1);
  
  /* test BACK push button */
  LCD_SendString(BACKmsg, 2, 1);
  while(GPIO_BitRead(GPIO5,GPIO_PIN_11) == 0);
  while(GPIO_BitRead(GPIO5,GPIO_PIN_11) == 1);
  
  /* test WAKE-UP push button */
  LCD_SendString(WAKEUPmsg, 2, 1);
  while(GPIO_BitRead(GPIO2,GPIO_PIN_10) == 0);
  while(GPIO_BitRead(GPIO2,GPIO_PIN_10) == 1);

  /* test NEXT push button */  
  LCD_SendString(NEXTmsg, 2, 1);
  while(GPIO_BitRead(GPIO5,GPIO_PIN_8) == 0);
  while(GPIO_BitRead(GPIO5,GPIO_PIN_8) == 1);
 
  /* test PREV push button */ 
  LCD_SendString(PREVmsg, 2, 1);
  while(GPIO_BitRead(GPIO5,GPIO_PIN_9) == 0);
  while(GPIO_BitRead(GPIO5,GPIO_PIN_9) == 1);
  
  return PASS;
}

/*******************************************************************************
* Function Name  : RedLedsTest
* Description    : Performs a successive lighting of red leds connected to GPIO4.
* Input          : None
* Output         : None
* Return         : PASS: test passed
*                  FAIL: test failed
*******************************************************************************/
TestStatus RedLedsTest(void)
{
  u32 i = 1;
  bool Dir = FALSE; 

  LCD_SendString(BoardTestmsg[1], 1, 1);

  LCD_SendString(NEXTmsg, 2, 1);

  while(ReadKey() != NXT)
  {
    if(! Dir)
    {
      GPIO_BitWrite(GPIO4, i , Bit_SET);
      Delay(0x5FFFF);
      i *= 4;
      if(i == 0x40000)
      {
        i = 0x8000;
        Dir = TRUE;
      }
    } 
    else
    {
      GPIO_BitWrite(GPIO4, i , Bit_SET);
      Delay(0x5FFFF);
      i = i/4;
      if(i == 0)
      {
        i = 1;
        Dir = FALSE;
      }
    }
    GPIO_WordWrite(GPIO4, GPIO_PIN_NONE);
  }

  GPIO_WordWrite(GPIO4,0x0000);

  return IsTestOk();
}
/*******************************************************************************
* Function Name  : PWMLedsTest
* Description    : Performs a successive lighting of bicolor leds connected to 
*                  GPIO2.
* Input          : None
* Output         : None
* Return         : PASS: test passed
*                  FAIL: test failed
*******************************************************************************/
TestStatus PWMLedsTest(void)
{
  u8 i = 0;
  u16 tmp0 = 0, tmp1 = 0, tmp2 = 0;
  u16 LedVal[6] = {0x0001, 0x0008, 0x0010, 0x0020, 0x0040, 0x0080};  
  
  /* store the current config of GPIO2  pins */ 
  tmp0 = GPIO2->PC0;
  tmp1 = GPIO2->PC1;
  tmp2 = GPIO2->PC2;

  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT_PP;
  GPIO_InitStructure.GPIO_Pins = GPIO_PIN_0 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 |
                                 GPIO_PIN_6 | GPIO_PIN_7;

  GPIO_Init(GPIO2, &GPIO_InitStructure);

  LCD_SendString(BoardTestmsg[2], 1, 1);
  LCD_SendString(NEXTmsg, 2, 1);
  
  while(ReadKey() != NXT)
  {
    GPIO_WordWrite(GPIO2, LedVal[i]); 
    Delay(0x5FFFF);
    i++;
   
    if(i == 6 )
    {
      i = 0;
    }
  }
  
  GPIO_WordWrite(GPIO2, GPIO_PIN_NONE);
  
  /* restore the previous config of GPIO2 pins */
  GPIO2->PC0 = tmp0;
  GPIO2->PC1 = tmp1;
  GPIO2->PC2 = tmp2;
  
  return IsTestOk();	
}

/*******************************************************************************
* Function Name  : LM35Test
* Description    : Displays the temperature, measured by the LM35 sensor, in
*                  Fahrenheit.
* Input          : None
* Output         : None
* Return         : PASS: test passed
*                  FAIL: test failed
*******************************************************************************/
TestStatus LM35Test(void)
{
  LCD_SendString(BoardTestmsg[3], 1, 1);
 
  /* select Fahrenheit as the temperature display unit */
  TemperatureUnit = Fahrenheit;

  /* initialize the peripherals used to measure the temperature */
  Converter_Init();

  /* enable ADC */
  ADC_Cmd(ENABLE);

  /* start ADC conversion */
  ADC_ConversionCmd(ADC_ConversionStart);

  /* display temperature message */
  LCD_SendString("       F /^NEXT^", 2, 1);

  while(ReadKey() != NXT)
  {
    Send_digits(ADC_GetConversionValue(ADC_CHANNEL1),2 ,2);	
  }
      
  return IsTestOk();
}

/*******************************************************************************
* Function Name  : VarResistTest
* Description    : Turns on/off the red leds connected to GPIO4 according to the 
*                  converted value of ADC Channel0 input voltage.
* Input          : None
* Output         : None
* Return         : PASS: test passed
*                  FAIL: test failed
*******************************************************************************/
TestStatus VarResistTest(void)
{
  u16 val = 0;

  /* ADC and P3.0 (trimmer input) configuration */
  Converter_Init();

  /* enable and start ADC */
  ADC_Cmd(ENABLE);
  ADC_ConversionCmd(ADC_ConversionStart);
   
  LCD_SendString(BoardTestmsg[4], 1, 1);
  LCD_SendString(NEXTmsg, 2, 1);
  
  while (ReadKey() != NXT)
  {
    val = ADC_GetConversionValue(ADC_CHANNEL0);
    
    if (val > 0x0300)  GPIO_WordWrite(GPIO4,0xFFFF);
    else if (val > 0x02A0)  GPIO_WordWrite(GPIO4,0x7FFF);
    else if (val > 0x0250)  GPIO_WordWrite(GPIO4,0x3FFF);
    else if (val > 0x0200)  GPIO_WordWrite(GPIO4,0x1FFF);
    else if (val > 0x01A0)  GPIO_WordWrite(GPIO4,0x0FFF);
    else if (val > 0x0180)  GPIO_WordWrite(GPIO4,0x07FF);
    else if (val > 0x0150)  GPIO_WordWrite(GPIO4,0x03FF);
    else if (val > 0x0120)  GPIO_WordWrite(GPIO4,0x01FF);
    else if (val > 0x0090)  GPIO_WordWrite(GPIO4,0x00FF);
    else if (val > 0x0070)  GPIO_WordWrite(GPIO4,0x007F);
    else if (val > 0x0060)  GPIO_WordWrite(GPIO4,0x003F);
    else if (val > 0x0040)  GPIO_WordWrite(GPIO4,0x001F);
    else if (val > 0x0020)  GPIO_WordWrite(GPIO4,0x000F);
    else if (val > 0x0010)  GPIO_WordWrite(GPIO4,0x0007);
    else if (val > 0x0010)  GPIO_WordWrite(GPIO4,0x0003);
    else  GPIO_WordWrite(GPIO4,0x0001);
  }
    
  GPIO_WordWrite(GPIO4,0x0000);
  return IsTestOk();
}
/*******************************************************************************
* Function Name  : Buzzer12Test
* Description    : Plays music notes with SW10 in position 1-2.
* Input          : None
* Output         : None
* Return         : PASS: test passed
*                  FAIL: test failed
*******************************************************************************/
TestStatus Buzzer12Test(void)
{
  u8 j = 0;
  u16 tmp0 = 0, tmp1 = 0, tmp2 = 0;
 
  /* store the current config of GPIO1  pins */ 
  tmp0 = GPIO1->PC0;
  tmp1 = GPIO1->PC1;
  tmp2 = GPIO1->PC2;
  
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT_PP;
  GPIO_InitStructure.GPIO_Pins = GPIO_PIN_8;
  GPIO_Init(GPIO1, &GPIO_InitStructure);                                   

  LCD_SendString(BoardTestmsg[5], 1, 1);
  LCD_SendString(NEXTmsg, 2, 1);
  
  while (ReadKey() != NXT)
  { 
    if (j<10)
    {
      GPIO_BitWrite(GPIO1,GPIO_PIN_8,Bit_SET); Delay(j*5000);
      GPIO_BitWrite(GPIO1,GPIO_PIN_8,Bit_RESET); Delay(j*1000);  
      j=j+2;
    }
    else
    {
      j=0;	
    }
  }
  
  /* restore the previous config of GPIO1 pins */
  GPIO1->PC0 = tmp0;
  GPIO1->PC1 = tmp1;
  GPIO1->PC2 = tmp2;
  
  return IsTestOk();
}

/*******************************************************************************
* Function Name  : Buzzer23Test
* Description    : Plays music notes with SW10 in position 2-3.
* Input          : None
* Output         : None
* Return         : PASS: test passed
*                  FAIL: test failed
*******************************************************************************/
TestStatus Buzzer23Test(void)
{    
  u8 j = 0;
  u16 tmp0 = 0, tmp1 = 0 , tmp2 = 0;

  /* store the current config of GPIO1  pins */  
  tmp0 = GPIO1->PC0;
  tmp1 = GPIO1->PC1;
  tmp2 = GPIO1->PC2;
  
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT_PP;
  GPIO_InitStructure.GPIO_Pins = GPIO_PIN_7 ;
  GPIO_Init(GPIO1, &GPIO_InitStructure);       

  LCD_SendString(BoardTestmsg[6], 1, 1);
  LCD_SendString(NEXTmsg, 2, 1);
  
  while(ReadKey() != NXT)
  {
    if (j<5 )
    {
      GPIO_BitWrite(GPIO1,GPIO_PIN_7,Bit_SET); Delay(j*1200);
      GPIO_BitWrite(GPIO1,GPIO_PIN_7,Bit_RESET); Delay(j*4000);
      j++;
    }
    else
    {
      j=0;	
    }
  }

  /* restore the previous config of GPIO1 pins */   
  GPIO1->PC0 = tmp0;
  GPIO1->PC1 = tmp1;
  GPIO1->PC2 = tmp2;
  
  return IsTestOk();	
}

/*******************************************************************************
* Function Name  : I2CTest
* Description    : Performs a write in the M24C08 I2C EEPROM, followed by a read
*                  of the written data.
* Input          : None
* Output         : None
* Return         : PASS: test passed
*                  FAIL: test failed
*******************************************************************************/
TestStatus I2CTest(void)
{
  u8 WRAddr = 0; 
  u16 NumByte = 0;

  LCD_SendString(BoardTestmsg[7], 1, 1);
  LCD_LineClear(2);
  
  NumByte = countof(Tx_I2Cmsg);
  WRAddr = I2C_EE_SectorSize - NumByte;
  
  I2C_EE_Init();  
  I2C_EE_BufferWrite(Tx_I2Cmsg, WRAddr, NumByte);
  I2C_EE_BufferRead(Rx_msg, WRAddr, NumByte);

  return (msgcmp(Tx_I2Cmsg, Rx_msg, NumByte));
}

/*******************************************************************************
* Function Name  : BSPITest
* Description    : Performs a write in the M95080 SPI EEPROM, followed by a read
*                  of the written data.
* Input          : None
* Output         : None
* Return         : PASS: test passed
*                  FAIL: test failed
*******************************************************************************/
TestStatus BSPITest(void)
{  
  u16 WRAddr = 0; 
  u16 NumByte = 0 ;

  LCD_SendString(BoardTestmsg[8], 1, 1);
  
  NumByte = countof(Tx_SPImsg);
  WRAddr = SPI_EE_Size - NumByte;
  
  SPI_EE_Init();  
  SPI_EE_BufferWrite(Tx_SPImsg, WRAddr, NumByte);	
  SPI_EE_BufferRead(Rx_msg, WRAddr, NumByte);

  return(msgcmp(Tx_SPImsg, Rx_msg, NumByte));
}

/*******************************************************************************
* Function Name  : UARTTest
* Description    : Performs a transfer of 16 bytes buffer between UART0 and UART1.
* Input          : None
* Output         : None
* Return         : PASS: test passed
*                  FAIL: test failed
*******************************************************************************/
TestStatus UARTTest(void)
{
  LCD_SendString(BoardTestmsg[9], 1, 1);
  
  /* P6.9 configuration */
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_InitStructure.GPIO_Pins = UART0_Tx_Pin ;
  GPIO_Init(GPIO6, &GPIO_InitStructure);
  
  /* P6.8 configuration */
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_TRI_TTL;
  GPIO_InitStructure.GPIO_Pins = UART0_Rx_Pin ;
  GPIO_Init(GPIO6, &GPIO_InitStructure);
  
  /* P2.9 configuration */
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_TRI_TTL;
  GPIO_InitStructure.GPIO_Pins = UART1_Rx_Pin;
  GPIO_Init(GPIO2, &GPIO_InitStructure);
  
  /* P2.8 configuration */
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_InitStructure.GPIO_Pins = UART1_Tx_Pin ;
  GPIO_Init(GPIO2, &GPIO_InitStructure);

  UART_DeInit(UART1);  
  UART_StructInit(&UART_InitStructure);
  UART_InitStructure.UART_BaudRate  = 9600;
  UART_InitStructure.UART_Mode = UART_Mode_8D ;  
  UART_InitStructure.UART_Loop_Standard = UART_Standard; 
  UART_InitStructure.UART_FIFO = UART_FIFO_Enable ;
  UART_Cmd(UART1, ENABLE);
  UART_Init(UART1, &UART_InitStructure);  

  UART_DeInit(UART0);  
  UART_InitStructure.UART_Rx = UART_Rx_Enable;
  UART_Cmd(UART0, ENABLE);
  UART_Init(UART0, &UART_InitStructure);  

  UART_FifoReset(UART1,UART_RxFIFO);
  UART_FifoReset(UART1,UART_TxFIFO);

  UART_FifoReset(UART0,UART_RxFIFO);
  UART_FifoReset(UART0,UART_TxFIFO);

  UART_ByteBufferSend(UART1 , Tx_UARTmsg, 16);

  UART_SetTimeOutValue(UART0, 0xFF);
  UART_ByteBufferReceive(UART0,  Rx_msg,16);
  
  return(msgcmp(Tx_UARTmsg, Rx_msg, 16));
}

/*******************************************************************************
* Function Name  : Delay
* Description    : Inserts a delay time.
* Input          : nCount: specifies the delay time length.
* Output         : None
* Return         : None
*******************************************************************************/
static void Delay(u32 nCount)
{
  u32 j = 0;

  for(j = nCount; j != 0; j--); 
}

/******************* (C) COPYRIGHT 2005 STMicroelectronics *****END OF FILE****/
