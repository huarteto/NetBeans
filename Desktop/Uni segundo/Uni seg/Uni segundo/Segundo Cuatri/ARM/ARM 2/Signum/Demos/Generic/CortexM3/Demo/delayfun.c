/* Signum Systems Demo - delay function                    (Ver 1.00) */

int delaycnt = 5;    /* small value (for better ETM trace demo)       */

int delaytemp = 0;   /* temporary - to add processing to load CPU     */

int delay1(int v)    /* this function will will assure more CPU load  */
{
  delaytemp = v;
  return v;
}

int delay(int cnt)   /*************** delay function ******************/
{
  int i, j;	  /* loop counters                                    */
  int k;          /* dummy local variable (just to kill loop time)    */

  k = 0;	
  for (i = 0; i < cnt; i++) {
    for (j = 0; j < delaycnt; j++) {
      k = k + delay1(j);     /* some processing to kill more time     */
    }
  }

  return k;
}

