/* Signum Systems Demo - blink LED and text processing     (Ver 1.00) */

extern int  delay(int);           /* external functions               */
extern void led_init(void);
extern void led_set(int);

struct MYSTRUCT {
  int   Counter;    /* counter 0 .. 5  */
  char *Pointer;    /* pointer to text */
};

char Buf1[20] = "signum systems"; /* buffer with text to be processed */

struct MYSTRUCT GlobalStruct;     /* structure to be shown in Watch   */

void processing(char *text) /* function 'processing' -replace letters */
{
  while (*text != '\0') {         /* do so for all characters in text */
    if (*text != ' ') {           /* is it space, if not so then      */
      *text ^= 0x20;                   /* flip this character (X<->x) */
    }
    text++;                       /* advance to next character        */
  }
}

int delayloop = 3;        /* small value (for better ETM trace demo) */

int main()
{
  int i;

  GlobalStruct.Counter = 0;                /* initialize structure    */  
  GlobalStruct.Pointer = Buf1;

  led_init();                              /* initialize LED module   */

  for (;;) {                               /* repeat forever          */
    if (GlobalStruct.Counter & 1) {        /* toggle the LED          */
      led_set(1);
    } else {
      led_set(0);
    }
    for (i = 0; i < delayloop; i++) {      /* repeat                  */
      delay(1);                            /* wait for a while        */
      processing(Buf1);                    /* do some processing      */
    }
    processing(Buf1);                      /* do some processing      */

    GlobalStruct.Counter++;                /* count characters        */
    if (GlobalStruct.Counter >= 6) {       /* 5 characters processed? */
      GlobalStruct.Counter = 0;            /* initialize again        */
      GlobalStruct.Pointer = Buf1;
    } else {
      GlobalStruct.Pointer++;              /* move to next character  */
    }

    if (delayloop == 0) {
        /* This is to make compiler happy about unreached return      */
        break;
    }
  }

  return 0; /* will not happen, but main needs to be 'int' */
}
