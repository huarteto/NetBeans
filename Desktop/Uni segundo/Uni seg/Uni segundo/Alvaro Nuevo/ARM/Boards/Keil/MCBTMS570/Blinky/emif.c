/* Includes ----------------------------------------------------------- */
#include <TMS570.h>

__asm void MPU_setRegion (unsigned int region,unsigned int base_addr, unsigned int access, unsigned int size) {
	MCR p15, #0, r0, c6, c2, #0
	MCR p15, #0, r1, c6, c1, #0
	MCR p15, #0, r2, c6, c1, #4
	MCR p15, #0, r3, c6, c1, #2
    BX  lr
}

__asm void MPU_enable(void) {
	MRC p15, #0, r1, c1, c0, #0 
	ORR r1, r1, #0x1
	DMB
	MCR p15, #0, r1, c1, c0, #0 
	ISB
	BX  lr
}


#if( NUM_EMIF_CS == 1)

unsigned int EMIF_CSx_START_ADDRESS[NUM_EMIF_CS]
		= { EMIF_CS0_ADDRMAP_START	};

unsigned int EMIF_CSx_END_ADDRESS[NUM_EMIF_CS]
		= { EMIF_CS0_ADDRMAP_END };
#endif

#if( NUM_EMIF_CS ==2)

unsigned int EMIF_CSx_START_ADDRESS[NUM_EMIF_CS]
		= { EMIF_CS0_ADDRMAP_START,
			EMIF_CS1_ADDRMAP_START,
		  };

unsigned int EMIF_CSx_END_ADDRESS[NUM_EMIF_CS]
		= { EMIF_CS0_ADDRMAP_END,
			EMIF_CS1_ADDRMAP_END,
		  };
#endif

#if( NUM_EMIF_CS == 3)

unsigned int EMIF_CSx_START_ADDRESS[NUM_EMIF_CS]
		= { EMIF_CS0_ADDRMAP_START,
			EMIF_CS1_ADDRMAP_START,
			EMIF_CS2_ADDRMAP_START,
		  };

unsigned int EMIF_CSx_END_ADDRESS[NUM_EMIF_CS]
		= { EMIF_CS0_ADDRMAP_END,
			EMIF_CS1_ADDRMAP_END,
			EMIF_CS2_ADDRMAP_END,
		  };
#endif

#if( NUM_EMIF_CS == 4)

unsigned int EMIF_CSx_START_ADDRESS[NUM_EMIF_CS]
		= { EMIF_CS0_ADDRMAP_START,
			EMIF_CS1_ADDRMAP_START,
			EMIF_CS2_ADDRMAP_START,
			EMIF_CS3_ADDRMAP_START};

unsigned int EMIF_CSx_END_ADDRESS[NUM_EMIF_CS]
		= { EMIF_CS0_ADDRMAP_END,
			EMIF_CS1_ADDRMAP_END,
			EMIF_CS2_ADDRMAP_END,
			EMIF_CS3_ADDRMAP_END};
#endif


void  ENABLE_EMIF_STRONGLY_ORDERED(void)
{
 	unsigned int region,base_addr,access,size;
/* --------------------------------------------------------------------- */
/*	Configure the Default R4 MPU Region for RAM/FLASH of the Device		 */
/* --------------------------------------------------------------------- */

	//Set the attributes for Region1 
	region 	    = 0x00000001; 
	base_addr 	= 0x00000000;
	size 		= 0x0000001B; //16KB
	access		= 0x0000030C; // Execute Enable ,TEX:000, Sharable, NonBufferable ,NonCachable

 	//Set Region1
 	MPU_setRegion (region,base_addr,access,size);                               

/* --------------------------------------------------------------------- */
/*	Configure the EMIF MPU Region to be Strongly Ordered 				 */
/* --------------------------------------------------------------------- */

	//Set the attributes for Region2
	region 	    = 0x00000002; 
	base_addr 	= 0x60000000; //emif
	size 		= 0x00000037; //256MB
	access		= 0x00000300; // Execute Enable,Full Access,TEX:001(Normal), Sharable, NonBufferable ,NonCachable,

 	//Set Region2
 	MPU_setRegion (region,base_addr,access,size);                               

/* --------------------------------------------------------------------- */
/*	Configure the Peripheral MPU Region to be Strongly Ordered 			 */
/* --------------------------------------------------------------------- */

	//Set the attributes for Region3
	region 	    = 0x00000003; 
	base_addr 	= 0xFF000000; //Peripheral Modules Frame
	size 		= 0x00000039; 
	access		= 0x00000300; // Execute Enable,Full Access,TEX:001(Normal), Sharable, NonBufferable ,NonCachable,

 	//Set Region3
 	MPU_setRegion (region,base_addr,access,size);      

/* --------------------------------------------------------------------- */
/*	Configure  R4 MPU Region for RAM/FLASH of the Device				 */
/* --------------------------------------------------------------------- */

	//Set the attributes for Region3
	region 		= 0x00000004; 
	base_addr 	= 0x08000000; //Peripheral Modules Frame
	size 		= 0x00000039; 
	access		= 0x0000030C; // Execute Enable,Full Access,TEX:001(Normal), Sharable, NonBufferable ,NonCachable,

 	//Set Region3
 	MPU_setRegion (region,base_addr,access,size);      


 	//Enable MPU
 	MPU_enable();
}

#define pEMIF   ((EMIF_ST *)EMIF)
//EMIF_ST    *EMIF_Ptr = (EMIF_ST *)   EMIF;

void EMIF_ASYNC_MEM_CONFIG(	unsigned int ASYNC_Mem_Config,
							unsigned int ASYNC_MEM_READ_CONF,
							unsigned int ASYNC_MEM_WRITE_CONF,
							unsigned int ASYNC_MEM_TURN_ARND_CYC,
							unsigned int Chip_Sel)
{

    pEMIF->ASYNC_CR_UN[Chip_Sel].ASYNC_CR_UL = ((ASYNC_Mem_Config        <<  0) |
    										    (ASYNC_MEM_TURN_ARND_CYC <<  2) |
    										    (ASYNC_MEM_READ_CONF     <<  4) |
    										    (ASYNC_MEM_WRITE_CONF    << 17)  );
}

/*----------------------------------------------------------------------------
  Function that initializes EMIF MPU
 *----------------------------------------------------------------------------*/
void EMIF_MPU_init (void) {
  unsigned int i;
  unsigned int ASYNC_MEM_CONF;
  unsigned int ASYNC_MEM_READ_CONF;			
  unsigned int ASYNC_MEM_WRITE_CONF;		
  unsigned int ASYNC_MEM_TURN_ARND_CYC;

  /*	Configure the EMIF MPU Region to be Strongly Ordered 				 */
  ENABLE_EMIF_STRONGLY_ORDERED();

  ASYNC_MEM_READ_CONF  = ((0x07 << 0) |       /* read  strobe hold     time */
                          (0x3F << 3) |       /* read  strobe duration time */
                          (0x0F << 9)  );	  /* read  strobe setup    time */
  ASYNC_MEM_WRITE_CONF = ((0x07 << 0) |       /* write strobe hold     time */
                          (0x3F << 3) |       /* write strobe duration time */
                          (0x0F << 9)  );	  /* write strobe setup    time */
  ASYNC_MEM_CONF       = (SELECT_STROBE_MODE_DISABLE |
                          EXTEND_WAIT_MODE_DISABLE   | 
                          ASIZE_16BIT_DATA            );
  ASYNC_MEM_TURN_ARND_CYC = 4;

  for(i = 0; i < NUM_EMIF_CS; i++) {
    EMIF_ASYNC_MEM_CONFIG(ASYNC_MEM_CONF,
                          ASYNC_MEM_READ_CONF,
                          ASYNC_MEM_WRITE_CONF,
                          ASYNC_MEM_TURN_ARND_CYC,
                          (Chip_Select) i);
  }
}

