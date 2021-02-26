#include <stdio.h>
#include <conio.h>
#include <stdlib.h>

#include <time.h>

#include "esi.h"

char pp[64000] ;

void main (int argc, char **argv)
   {
   int testcount = 0 ;
   char *sbenv = getenv("BLASTER") ;
   clock_t t1, t2 ;

   directvideo = 0 ;
   Stereo = 0 ;

   if (argc < 2)
      {
      cputs ("Uso: ESITEST <nombrefichero>") ;
      return ;
      }

   if (sbenv) ProcessSBString (sbenv) ;

   switch (LoadS3M (argv[1]))
      {
      case ESI_ERRABRIR:
	 cputs ("Error al abrir fichero.") ;
	 break ;
      case ESI_ERRFORMATO:
	 cputs ("No es un fichero S3M.") ;
	 break ;
      case ESI_ERRVARIANTE:
	 cputs ("Variante de formato no soportada.") ;
	 break ;
      case ESI_ERRFICHERO:
	 cputs ("Fichero corrupto.") ;
	 break ;
      case ESI_ERRMEM:
	 cputs ("Sin memoria.") ;
	 break ;
      default:
	 cputs ("Fichero abierto con ‚xito.") ;
	 break ;
      }
   SBStart() ;
   cprintf ("SB: A%x I%d D%d\n\r", SB2x0, SBIrq, SBDMAChannel) ;
   t1 = clock() ;
   StartSong() ;
   cputs ("Pulsa una tecla para terminar la reproducci¢n, y otra para terminar el test...") ;
   while (!kbhit())
      {
      cprintf ("Haciendo cosas: %d\r", testcount++) ;
      if (testcount == 2000)
	 {
	 t2 = clock() ;
	 cprintf ("Tiempo transcurrido (clocks): %ld\n\r", t2-t1) ;
	 t1 = t2 ;
	 testcount = 0 ;
	 }
      }
   getch() ;
   SBEnd() ;
   t1 = clock() ;
   testcount = 0 ;
   while (!kbhit())
      {
      cprintf ("Haciendo cosas: %d\r", testcount++) ;
      if (testcount == 2000)
	 {
	 t2 = clock() ;
	 cprintf ("Tiempo transcurrido (clocks): %ld\n\r", t2-t1) ;
	 t1 = t2 ;
	 testcount = 0 ;
	 }
      }
   while (kbhit()) getch() ;
   }