#include <TMS570.h>
#include "ConfidenceTest.h"


void NHET_Init(void) {

  // NHET1  drives one LED
  // NHET24 drives the solenoid 1 output
  // NHET25 drives the solenoid 2 output
  // NHET26 drives the solenoid 3 output
  // NHET27 drives the solenoid 4 output
  // NHET28 drives the relay
  // NHET30 drives the speaker
  // NHET31 drives one LED

  // bring output register into a defined state before direction is set
  nhetREG->CCDCLR = bit1 | bit24 | bit25 | bit26 | bit27 | bit30 | bit31;
  nhetREG->CCDSET = bit28;
  // select direction (output)
  nhetREG->CCDIR  |= bit1 | bit24 | bit25 | bit26 | bit27 | bit28 | bit30 | bit31;
}
