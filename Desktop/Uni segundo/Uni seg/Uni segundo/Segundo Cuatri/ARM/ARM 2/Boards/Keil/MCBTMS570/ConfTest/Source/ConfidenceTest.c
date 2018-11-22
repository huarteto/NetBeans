/************************************************************************/
/*	Name		: Confidence Test for MCBTMS570 board   	   	     	*/
/*				                                                  	  	*/
/* 	Module 		: ConfidenceTest.c				                	    */
/* 				                                                	  	*/
/*  Description : Main module to test most board components             */
/* 				            								    	  	*/
/************************************************************************/
/* 	Revision History :         								    	  	*/
/* 				            								    	  	*/
/*  21-April-2010	1.0 (Initial)                                       */
/************************************************************************/

/* Includes ----------------------------------------------------------- */

#include <stdio.h>
#include <TMS570.h>
#include "EMIF_Config.h"
#include "GLCD.h"
#include "SDCard.h"
#include "ConfidenceTest.h"


/* Defines ------------------------------------------------------------ */

/* Globals  ----------------------------------------------------------- */

static unsigned int PageSelector;   // Specifies which test is running / will run next


void delay(void) {
  static volatile unsigned int delayval;
    
  delayval = 100000;   // 100000 are about 10ms
  while(delayval--);
}

void small_delay(void) {
  static volatile unsigned int delayval;
    
  delayval = 10;
  while(delayval--);
}


// defines the size of the external RAM = 512K
#define  NO_OF_WORDS  (0x00080000 / 4)


// Tables with screen coordinates for touch-buttons
static TouchTable MainTouch[] = {
//  x_min x_max y_min y_max action;
  {    5,  155,  180,  230,    1 },  // Memory
  {    5,  155,  120,  170,    2 },  // SD-Card
  {    5,  155,   60,  110,    3 },  // Tmp/Light
  {  165,  315,  180,  230,    4 },  // Sensors
  {  165,  315,  120,  170,    5 },  // Solenoid
  {  165,  315,   60,  110,    6 },  // Air
  {    0,    0,    0,    0,    0 },  // end of table
};

static TouchTable OKTouch[] = {
//  x_min x_max y_min y_max action;
  {   85,  235,  180,  230,    0 },  // OK button bottom center
  {    0,    0,    0,    0,    0 },  // end of table
};

static TouchTable SolenoidTouch[] = {
//  x_min x_max y_min y_max action;
  {    5,  155,    5,   55,    1 },  // Sol 1
  {    5,  155,   65,  115,    2 },  // Sol 2
  {    5,  155,  125,  175,    3 },  // Sol 3
  {    5,  155,  185,  235,    4 },  // Sol 4
  {  165,  315,    5,   55,    5 },  // Relay
  {  165,  315,  185,  235,    6 },  // OK
  {    0,    0,    0,    0,    0 },  // end of table
};

static TouchTable AirTouch[] = {
//  x_min x_max y_min y_max action;
  {    5,  155,   65,  115,    1 },  // Pump
  {  165,  315,   65,  115,    2 },  // Valve
  {   85,  235,  180,  230,    0 },  // OK button bottom center
  {    0,    0,    0,    0,    0 },  // end of table
};

/*----------------------------------------------------------------------------
  This function tests all external memory devices and prints the result on the LCD
  Parameter:    none
  Return Value: none
 *----------------------------------------------------------------------------*/
static void Memory_Test_Screen(void) {
  unsigned int i;

  // clear LCD screen and print a headline in inverse color
  GLCD_Clear(White);
  GLCD_SetBackColor(Blue);
  GLCD_SetTextColor(White);
  GLCD_DisplayString(0, 0, " MCBTMS570 Mem-Test ");
  GLCD_SetBackColor(White);
  GLCD_SetTextColor(Blue);

  // call test procedure and output the result for every test on the LCD
  GLCD_DisplayString(2,  0, "512K RAM:");         // test external 512K RAM with different access widths
  GLCD_DisplayString(2, 14, "32 bit");
  i = TEST_EMIF_MEMORY_32BIT(EMIF_CS0_ADDRMAP_START, NO_OF_WORDS, 0xAA5555AA);
  if (i == 0) {
    GLCD_DisplayString(2, 14, "16 bit");
    i = TEST_EMIF_MEMORY_16BIT(EMIF_CS0_ADDRMAP_START, NO_OF_WORDS, 0x55AA55AA);
  }            
  if (i == 0) {
    GLCD_DisplayString(2, 14, " 8 bit");
    i = TEST_EMIF_MEMORY_8BIT(EMIF_CS0_ADDRMAP_START, NO_OF_WORDS, 0x12345678);
  }            
  if (i == 0) {
    GLCD_DisplayString(2, 14, "Size  ");
    i = TEST_EMIF_MEMORY_ADR(EMIF_CS0_ADDRMAP_START, NO_OF_WORDS);
  }
  if (i == 0) {
    GLCD_SetTextColor(Green);
    GLCD_DisplayString(2, 14, "passed");
  } else {
    GLCD_SetTextColor(Red);
    GLCD_DisplayString(2, 14, "failed");
  }            
  GLCD_SetBackColor(White);
  GLCD_SetTextColor(Blue);
  GLCD_DisplayString(3,  0, "EEPROM(2K):");       // test EEPROM on IO board
  i = Test_EEPROM();
  if (i == 0) {
    GLCD_SetTextColor(Green);
    GLCD_DisplayString(3, 14, "passed");
  } else {
    GLCD_SetTextColor(Red);
    GLCD_DisplayString(3, 14, "failed");
  }            

  GLCD_SetBackColor(White);
  GLCD_SetTextColor(Blue);
  GLCD_DisplayString(4,  0, "FRAM(8K):");         // test FRAM on IO board
  i = Test_FRAM();
  if (i == 0) {
    GLCD_SetTextColor(Green);
    GLCD_DisplayString(4, 14, "passed");
  } else {
    GLCD_SetTextColor(Red);
    GLCD_DisplayString(4, 14, "failed");
  }            

  GLCD_SetBackColor(White);
  GLCD_SetTextColor(Blue);
  GLCD_DisplayString(5,  0, "SW Revision:  1.0");// print software revision into this screen
  GLCD_Bmp (  85, 180, 150,  50, Button_OK_16bpp  +70);
  CheckForTouch(&OKTouch[0], 1);
  PageSelector = 0;                              // go back to main page
}

/*----------------------------------------------------------------------------
  This function tests the presence and size of a SD-Memory card and prints the
  results on the LCD
  Parameter:    none
  Return Value: none
 *----------------------------------------------------------------------------*/
static void SDC_Test_Screen(void) {
  unsigned int i;
  char buffer[20];

  // clear LCD screen and print a headline in inverse color
  GLCD_Clear(White);
  GLCD_SetBackColor(Blue);
  GLCD_SetTextColor(White);
  GLCD_DisplayString(0, 0, " MCBTMS570 SDC-Test ");
  GLCD_SetBackColor(White);
  GLCD_SetTextColor(Blue);

  GLCD_DisplayString(2,  0, "SDC detect:  ");
  i = gioPORTA->DIN & 0x0004;                    // GIOA2 is SDC detect pin
  if (i == 0) {
    GLCD_SetTextColor(Green);
    GLCD_DisplayString(2, 14, "passed");
  } else {
    GLCD_SetTextColor(Red);
    GLCD_DisplayString(2, 14, "failed");
  }            
  GLCD_SetTextColor(Blue);
  GLCD_DisplayString(3, 0, "SDC-Init: ");
  i = SDC_init();
  if (i == 1) {
    GLCD_SetTextColor(Green);
    GLCD_DisplayString(3, 14, "passed");
  } else {
    GLCD_SetTextColor(Red);
    GLCD_DisplayString(3, 14, "failed");
  }            
  if (i) {
    GLCD_SetTextColor(Blue);
    SDC_rdCfg(&SDC_cfg);
    GLCD_DisplayString(4, 0, "Ser.No:");
    sprintf(&buffer[0], "%08X", SDC_cfg.sernum);
    GLCD_DisplayString(4, 12, &buffer[0]);
    GLCD_DisplayString(5, 0, "Size(B):");
    sprintf(&buffer[0], "%10d", SDC_cfg.blocknr*SDC_cfg.read_blen);
    GLCD_DisplayString(5, 10, &buffer[0]);
  }
  SDC_UnInit();
  GLCD_Bmp (  85, 180, 150,  50, Button_OK_16bpp  +70);
  CheckForTouch(&OKTouch[0], 1);
  PageSelector = 0;     // go back to main page
}

/*----------------------------------------------------------------------------
  This function shows the temperature and ligh sensor values with bar-graphs on the LCD.
  Parameter:    none
  Return Value: none
 *----------------------------------------------------------------------------*/
static void Tmp_Light_Test_Screen(void) {
  unsigned int i;
  unsigned int Light, Temp, TempLimitLow, TempLimitHigh;
  char  text[40];

  // clear LCD screen and print a headline in inverse color
  GLCD_Clear(White);
  GLCD_SetBackColor(Blue);
  GLCD_SetTextColor(White);
  GLCD_DisplayString(0, 0, "  Light/Temp Test   ");
  GLCD_SetBackColor(White);
  GLCD_SetTextColor(Blue);

  // draw OK button on the screen
  GLCD_Bmp (  85, 180, 150,  50, Button_OK_16bpp  +70);
  GLCD_DisplayString(2, 0, "Light: ");
  GLCD_DisplayString(4, 0, "Temp:  ");
  GLCD_SetTextColor(Red);
  GLCD_SetBackColor(Black);
  TempLimitLow  = 0x0840;
  TempLimitHigh = 0x0840 + 80;
  for (i = 0; i < 0x20; i++) {       // call conversion function a few times to fill the moving average buffer
  	GetLightAndTemp (&Light, &Temp);
  }
  while(1) {
    GetLightAndTemp (&Light, &Temp);
    GLCD_Bargraph (144, 48, 160, 20, (Light * 160) >> 12);  // scale Light-value from 12 bit to 0-160
    if (Temp < (TempLimitLow  - 10)) TempLimitLow  = Temp - 10;
    if (Temp > (TempLimitHigh + 10)) TempLimitHigh = Temp + 10;
    GLCD_Bargraph (144, 96, 160, 20, ((Temp - TempLimitLow) * 160) / (TempLimitHigh - TempLimitLow) );
    i = CheckForTouch(&OKTouch[0], 0);    // check if the OK touch screen button is selected. Don't wait
    if (i == 0) break;                    // OK button was pressed

    /* print out unscaled values */
    if (clock_1s) {
      clock_1s = 0;

      GLCD_SetTextColor(Blue);
      GLCD_SetBackColor(White);
      sprintf(text, " AD Light: 0x%04X", Light);
    GLCD_DisplayString(5, 0,  (char *)text);
      printf ("%s\n\r", text);            // print to serial interface

      sprintf(text, " AD Temp : 0x%04X", Temp);
    GLCD_DisplayString(6, 0,  (char *)text);
      printf ("%s\n\r", text);            // print to serial interface
      GLCD_SetTextColor(Red);
      GLCD_SetBackColor(Black);
    }
  }
  PageSelector = 0;     // go back to main page
}

/*----------------------------------------------------------------------------
  With this function a user can switch the solenoid outputs and relay state.
  The current of the solenoid outputs are printed on the LCD
  Parameter:    none
  Return Value: none
 *----------------------------------------------------------------------------*/
static void Solenoid_Test_Screen(void) {
  unsigned int i, UpdateButtons;
  unsigned int RelayState, Sol1State, Sol2State, Sol3State, Sol4State;
  unsigned int I1, I2, I3, I4;
  double       Current;
  char         buffer[20];
  
  // clear LCD screen and select colors
  GLCD_Clear(White);
  GLCD_SetBackColor(White);
  GLCD_SetTextColor(Blue);

  // draw OK touch button on the screen
  GLCD_Bmp (  165, 185, 150,  50, Button_OK_16bpp    +70);

  UpdateButtons = 1;
  RelayState = (nhetREG->CCDIN & bit28) ? 1 : 0;
  Sol1State  = (nhetREG->CCDIN & bit24) ? 1 : 0;
  Sol2State  = (nhetREG->CCDIN & bit25) ? 1 : 0;
  Sol3State  = (nhetREG->CCDIN & bit26) ? 1 : 0;
  Sol4State  = (nhetREG->CCDIN & bit27) ? 1 : 0;
  while(1) {
    // draw touch screen buttons on the screen, either in black or green depending on the status of the output
    if (UpdateButtons) {
      if (Sol1State) GLCD_Bmp (    5,   5, 150,  50, Button_Sol1_sel_16bpp  +70);
      else           GLCD_Bmp (    5,   5, 150,  50, Button_Sol1_16bpp      +70);
      if (Sol2State) GLCD_Bmp (    5,  65, 150,  50, Button_Sol2_sel_16bpp  +70);
      else           GLCD_Bmp (    5,  65, 150,  50, Button_Sol2_16bpp      +70);
      if (Sol3State) GLCD_Bmp (    5, 125, 150,  50, Button_Sol3_sel_16bpp  +70);
      else           GLCD_Bmp (    5, 125, 150,  50, Button_Sol3_16bpp      +70);
      if (Sol4State) GLCD_Bmp (    5, 185, 150,  50, Button_Sol4_sel_16bpp  +70);
      else           GLCD_Bmp (    5, 185, 150,  50, Button_Sol4_16bpp      +70);
      if (RelayState)GLCD_Bmp (  165,   5, 150,  50, Button_Relay_16bpp     +70);
      else           GLCD_Bmp (  165,   5, 150,  50, Button_Relay_sel_16bpp +70);
      UpdateButtons = 0;
    }
    GetSolCurrent (&I1, &I2, &I3, &I4);
    delay();
    Current = (double)I1 * 0.64453125;
    sprintf(&buffer[0], "I1=%5.0fmA", Current);
    GLCD_DisplayString(3, 10, &buffer[0]);
    Current = (double)I2 * 0.64453125;
    sprintf(&buffer[0], "I2=%5.0fmA", Current);
    GLCD_DisplayString(4, 10, &buffer[0]);
    Current = (double)I3 * 0.64453125;
    sprintf(&buffer[0], "I3=%5.0fmA", Current);
    GLCD_DisplayString(5, 10, &buffer[0]);
    Current = (double)I4 * 0.64453125;
    sprintf(&buffer[0], "I4=%5.0fmA", Current);
    GLCD_DisplayString(6, 10, &buffer[0]);

    i = CheckForTouch(&SolenoidTouch[0], 0);      // check if one of the touch screen button is selected. Don't wait
    switch (i) {
      case 1:                 // Solenoid 1 Button
        if (Sol1State) {
          nhetREG->CCDCLR = bit24;
          Sol1State = 0;
        } else {
          nhetREG->CCDSET = bit24;
          Sol1State = 1;
        }
        UpdateButtons = 1;
        break;
      case 2:                 // Solenoid 2 Button
        if (Sol2State) {
          nhetREG->CCDCLR = bit25;
          Sol2State = 0;
        } else {
          nhetREG->CCDSET = bit25;
          Sol2State = 1;
        }
        UpdateButtons = 1;
        break;
      case 3:                 // Solenoid 3 Button
        if (Sol3State) {
          nhetREG->CCDCLR = bit26;
          Sol3State = 0;
        } else {
          nhetREG->CCDSET = bit26;
          Sol3State = 1;
        }
        UpdateButtons = 1;
        break;
      case 4:                 // Solenoid 4 Button
        if (Sol4State) {
          nhetREG->CCDCLR = bit27;
          Sol4State = 0;
        } else {
          nhetREG->CCDSET = bit27;
          Sol4State = 1;
        }
        UpdateButtons = 1;
        break;

      case 5:                 // Relay Button
        if (RelayState) {
          nhetREG->CCDCLR = bit28;
          RelayState = 0;
        } else {
          nhetREG->CCDSET = bit28;
          RelayState = 1;
        }
        UpdateButtons = 1;
        break;

      case 6:                 // OK Button
        PageSelector = 0;     // go back to main page
        return;
    }
  }
}

/*----------------------------------------------------------------------------
  This function shows the two sensor input values with bar-graphs on the LCD.
  Parameter:    none
  Return Value: none
 *----------------------------------------------------------------------------*/
static void Sensors_Test_Screen(void) {
  unsigned int i;
  unsigned int Input1, Input2;
  char  text[40];

  // clear LCD screen and print a headline in inverse color
  GLCD_Clear(White);
  GLCD_SetBackColor(Blue);
  GLCD_SetTextColor(White);
  GLCD_DisplayString(0, 0, "   Sensor Inputs    ");
  GLCD_SetBackColor(White);
  GLCD_SetTextColor(Blue);

  // draw OK touch screen button on the screen
  GLCD_Bmp (  85, 180, 150,  50, Button_OK_16bpp  +70);
  GLCD_DisplayString(2, 0, "Input 1: ");
  GLCD_DisplayString(4, 0, "Input 2: ");
  GLCD_SetTextColor(Red);
  GLCD_SetBackColor(Black);
  while(1) {
    GetSensorInput (&Input1, &Input2);                      // INA128 OpAmp is set to a gain of 8.35 
    GLCD_Bargraph (144, 48, 160, 20, (Input1 * 160) >> 12); // scale Input1-value from 12 bit to 0-160
    GLCD_Bargraph (144, 96, 160, 20, (Input2 * 160) >> 12); // scale Input2-value from 12 bit to 0-160
    i = CheckForTouch(&OKTouch[0], 0);                      // check if the OK touch screen button is selected. Don't wait
    if (i == 0) break;                                      // OK button was pressed

    /* print out unscaled values */
    if (clock_1s) {
      clock_1s = 0;

      GLCD_SetTextColor(Blue);
      GLCD_SetBackColor(White);
      sprintf(text, "0x%04X", Input1);
      GLCD_DisplayString(3, 10,  (char *)text);
      sprintf(text, " Input1: 0x%04X", Input1);
      printf ("%s\n\r", text);            // print to serial interface

      sprintf(text, "0x%04X", Input2);
      GLCD_DisplayString(5, 10,  (char *)text);
      sprintf(text, " Input2: 0x%04X", Input2);
      printf ("%s\n\r", text);            // print to serial interface
      GLCD_SetTextColor(Red);
      GLCD_SetBackColor(Black);
    }
  }
  PageSelector = 0;     // go back to main page
}

/*----------------------------------------------------------------------------
  This function provides buttons on the LCD screen to switch the air-pump and 
  air-valve outputs.
  Parameter:    none
  Return Value: none
 *----------------------------------------------------------------------------*/
static void Air_Test_Screen(void) {
  unsigned int i, UpdateButtons;
  unsigned int PumpState, ValveState;
  
  // clear LCD screen and print a headline in inverse color
  GLCD_Clear(White);
  GLCD_SetBackColor(Blue);
  GLCD_SetTextColor(White);
  GLCD_DisplayString(0, 0, "  Air Pump/Valve    ");
  GLCD_SetBackColor(White);
  GLCD_SetTextColor(Blue);

  // draw OK touch screen button on the screen
  GLCD_Bmp (  85,  180, 150,  50, Button_OK_16bpp    +70);

  UpdateButtons = 1;
  PumpState  = (gioPORTA->DIN & 0x40) ? 1 : 0;   // GIOA6 drives the air-pump output
  ValveState = (gioPORTA->DIN & 0x80) ? 1 : 0;   // GIOA7 drives the air-valve output
  while(1) {
    // draw touch screen buttons on the screen, either in black or green depending on the status of the output
    if (UpdateButtons) {
      if (PumpState)  GLCD_Bmp (    5,  65, 150,  50, Button_Pump_sel_16bpp  +70);
      else            GLCD_Bmp (    5,  65, 150,  50, Button_Pump_16bpp      +70);
      if (ValveState) GLCD_Bmp (  165,  65, 150,  50, Button_Valve_sel_16bpp +70);
      else            GLCD_Bmp (  165,  65, 150,  50, Button_Valve_16bpp     +70);
      UpdateButtons = 0;
    }
    delay();

    i = CheckForTouch(&AirTouch[0], 0);   // check if one of the touch screen buttons are selected. Don't wait
    switch (i) {
      case 1:                 // Pump Button
        if (PumpState) {
          gioPORTA->DCLR = 0x40;
          PumpState = 0;
        } else {
          gioPORTA->DSET = 0x40;
          PumpState = 1;
        }
        UpdateButtons = 1;
        break;
      case 2:                 // Valve Button
        if (ValveState) {
          gioPORTA->DCLR = 0x80;
          ValveState = 0;
        } else {
          gioPORTA->DSET = 0x80;
          ValveState = 1;
        }
        UpdateButtons = 1;
        break;

      case 0:                 // OK Button
        PageSelector = 0;     // go back to main page
        return;
    }
  }
}

/*----------------------------------------------------------------------------
  This function shows the main menu on the LCD screen so that one of the test 
  can be selected 
  Parameter:    none
  Return Value: none
 *----------------------------------------------------------------------------*/
static void Main_Menu_Screen(void) {

  GLCD_Clear(White);      // clear LCD screen
  // draw logos and touch screen buttons on the screen
  GLCD_Bmp (    5,   5, 118,  30, TI_Logo_16bpp         +70);
  GLCD_Bmp (  213,   5,  94,  30, Keil_Logo_16bpp       +70);
  GLCD_Bmp (    5,  60, 150,  50, Button_TmpLight_16bpp +70);
  GLCD_Bmp (    5, 120, 150,  50, Button_SDCard_16bpp   +70);
  GLCD_Bmp (    5, 180, 150,  50, Button_Memory_16bpp   +70);
  GLCD_Bmp (  165,  60, 150,  50, Button_Air_16bpp      +70);
  GLCD_Bmp (  165, 120, 150,  50, Button_Solenoid_16bpp +70);
  GLCD_Bmp (  165, 180, 150,  50, Button_Sensors_16bpp  +70);

  PageSelector = CheckForTouch(&MainTouch[0], 1);   // wait for ever until one of the touch buttons are selected
}

/*----------------------------------------------------------------------------
  'main' initializes all peripherals and calls the selected screen menus 
  Parameter:    none
  Return Value: none
 *----------------------------------------------------------------------------*/
int main(void) {

  EMIF_Init ();     // initialize external memory interface
  gioInit   ();     // initialize IO ports for LED etc.
  NHET_Init ();     // initialize NHET pins as port pins for LED, solenoid control etc.
  spi3Init  ();     // initialize SPI3 interface for touch screen controller and SD-Card interface
  spi1Init  ();     // initialize SPI1 interface for FRAM and EEPROM
  TIM_init  ();     // initialize RTI for delays
  SER_init  ();     // initialize serial interface
  GLCD_Init ();     // initialize LCD
  Touch_Init();     // initialize touch screen controller
  adcInit   ();     // initialize A/D converter

  beep ();          // beep one time at the beginning

  PageSelector = 0; //select main Screen as first menu
  
  while(1) {                            // endless loop
    switch (PageSelector) {
      case 0:                           // Main screen to select test procedure
        Main_Menu_Screen();
        break;

      case 1:                           // Memory test
        Memory_Test_Screen();
        break;

      case 2:                           // SD-Card test
        SDC_Test_Screen();
        break;

      case 3:                           // Temp/Light sensors
        Tmp_Light_Test_Screen();
        break;

      case 4:                           // Sensors input test
        Sensors_Test_Screen();
        break;

      case 5:                           // Solenoid test
        Solenoid_Test_Screen();
        break;

      case 6:                           // Air Pump/Valve test
        Air_Test_Screen();
        break;

      default:                          // should never get here
        PageSelector = 0;
    }
  }    
}
