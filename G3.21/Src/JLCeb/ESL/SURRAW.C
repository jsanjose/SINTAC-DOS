#include <fcntl.h>
#include <io.h>
#include <conio.h>
#include <stdio.h>
#include <mem.h>

#include "esi.h"

int handle, fichero ;	   /* Handles del bloque de memoria y del fichero */
int handle2 ;
int instrumento ;	   /* N£mero de instrumento */
int instrumento2 ;
long tamano ;	   	   /* Tama¤o del fichero */

void CopyMem (int handledest, int handleorig, unsigned size)
   {
   char buffer[1024] ;
   unsigned offset = 0, as ;

   while (size)
      {
      as = size ;
      if (as > 1024) as = 1024 ;
      memcpy (buffer, AccessMem (handleorig)+offset, as) ;
      memcpy (AccessMem (handledest)+offset, buffer, as) ;
      offset += as ;
      size -= as ;
      }
   }

void main (int argc, char **argv)
   {
   unsigned int i ;
   unsigned char far *ptr ;

	   /* Abre el fichero de disco */

   fichero = open (argv[1], O_RDONLY) ;
   if (handle == -1 || argc < 2)
      {
      puts ("No se puede abrir el fichero") ;
      return ;
      }

	   /* Averigua el tama¤o del fichero */

   tamano = lseek (fichero, 0, SEEK_END) ;
   if (tamano > 65535)
      {
      puts ("No se pueden reproducir samples de m s de 64K.") ;
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

	   /* Crea un inverso del sample */

   handle2 = AllocMem (tamano) ;
   if (handle2 == -1)
      {
      puts ("No hay memoria suficiente.") ;
      return ;
      }
   CopyMem (handle2, handle, tamano) ;
   ptr = AccessMem (handle2) ;
   for (i = 0 ; i < tamano ; i++)
      *ptr++ = 256-*ptr ;

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
   instrumento2 = CreateInstrument (handle2, tamano, 11000, 0, 0) ;

	   /* Toca el instrumento en "dolby surround" */

   PlayInstrument (instrumento,  0, 63, NOTE_C, 4, PANNING_LEFT) ;
   PlayInstrument (instrumento2, 1, 63, NOTE_C, 4, PANNING_RIGHT) ;
   puts ("Instrumento sonando...") ;
   puts ("Pulse una tecla") ;
   getch() ;

	   /* Termina con la ESL */

   SBEnd() ;
   }