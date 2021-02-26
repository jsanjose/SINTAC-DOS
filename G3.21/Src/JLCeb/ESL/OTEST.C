#include <stdio.h>
#include <conio.h>
#include <stdlib.h>

#include <time.h>

#include "esi.h"

/* Este programa lo he usado para hacer tests de velocidad comparando
   mi librer¡a con trackers conocidos, tocando en modo shell.

   Mi librer¡a ha resultado ser mucho m s lenta, y con diferencia :( */

void main ()
   {
   int testcount = 0 ;
   clock_t t1, t2 ;

   t1 = clock() ;
   puts ("Pulsa una tecla para terminar el test...") ;
   while (!kbhit())
      {
      printf ("Haciendo cosas: %d\r", testcount++) ;
      if (testcount == 2000)
	 {
	 t2 = clock() ;
	 printf ("Tiempo transcurrido (clocks): %ld\n", t2-t1) ;
	 t1 = t2 ;
	 testcount = 0 ;
	 }
      }
   getch() ;
   }
