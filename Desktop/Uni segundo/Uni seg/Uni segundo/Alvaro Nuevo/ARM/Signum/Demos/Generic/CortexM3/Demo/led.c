/* Signum Systems - LED routines for targets without LED (Ver 1.00)   */

int ledport = 0;      /* global copy of last LED status (for Viper)   */

void led_init()
{
  ledport = 0;        /* no hardware initialization is necessary      */
}

void led_set(int v)   /****** function 'led_set' - display LED        */
{
  ledport = v;            /* copy it to global variable (for Viper)   */

  /* there is no LED on this system, so do nothing                    */
}
