#include <fcntl.h>
#include <io.h>
#include <conio.h>
#include <stdio.h>
#include <dos.h>

#include "esi.h"

int handle, fichero ;	   /* Handles del bloque de memoria y del fichero */
int instrumento ;	   /* N�mero de instrumento */
long tamano ;	   	   /* Tama�o del fichero */

void main (int argc, char **argv)
   {

	   /* Abre el fichero de disco */

   fichero = open (argv[1], O_RDONLY) ;
   if (handle == -1 || argc < 2)
      {
      puts ("No se puede abrir el fichero") ;
      return ;
      }

	   /* Averigua el tama�o del fichero */

   tamano = lseek (fichero, 0, SEEK_END) ;
   if (tamano > 65535)
      {
      puts ("No se pueden reproducir samples de m�s de 64K.") ;
      return ;
      }
   lseek (fichero, 0, SEEK_SET) ;

	   /* Reserva memoria para el fichero */

   handle = AllocMem (tamano) ;
   if (handle == -1)
      {
      puts ("No hay memoria EMS o convencional, para el sample.") ;
      close (fichero) ;
      return ;
      }

	   /* Recupera el fichero de disco y lo cierra */

   if (read (fichero, AccessMem(handle), tamano) == -1)
      {
      puts ("Error al leer el fichero.") ;
      close (fichero) ;
      return ;
      }
   close (fichero) ;

   /* -------------------------------------------------------- */

	   /* Prepara la ESL para una frecuencia de 22000 Hz y stereo */

   Hz = 22000 ;
   Stereo = 1 ;

	   /* Activa la ESL */

   if (SBStart() == -1)
      {
      puts ("No hay tarjeta Sound Blaster") ;
      return ;
      }

	   /* Crea un instrumento */

   instrumento = CreateInstrument (handle, tamano, 11000, 0, 0) ;

	   /* Toca el instrumento con eco */

   PlayInstrument (instrumento, 0, 63, NOTE_C, 4, PANNING_MID) ;
   delay (750) ;
   PlayInstrument (instrumento, 1, 32, NOTE_C, 4, PANNING_RIGHT) ;
   delay (750) ;
   PlayInstrument (instrumento, 2, 16, NOTE_C, 4, PANNING_LEFT) ;
   delay (750) ;
   PlayInstrument (instrumento, 3, 8, NOTE_C, 4, 4) ;
   delay (750) ;
   PlayInstrument (instrumento, 4, 4, NOTE_C, 4, 12) ;
   puts ("Instrumento sonando...") ;
   puts ("Pulse una tecla") ;
   getch() ;

	   /* Termina con la ESL */

   SBEnd() ;
   }