#include <fcntl.h>
#include <io.h>
#include <conio.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dos.h>

#include "esi.h"

int handle, fichero ;	   /* Handles del bloque de memoria y del fichero */
int handle2 ;
int instrumento ;	   /* N£mero de instrumento */
int instrumento2 ;
long tamano ;	   	   /* Tama¤o del fichero */
long tdata ;

char filename[80] ;
int surround = 0 ;	   /* ¨Hay surround? 0 no */
int eco = 0 ;		   /* Delay del eco. 0 desactivado */
int forcehz = 0 ;	   /* Hz en l¡nea de comandos */
int forceohz = 0 ;
int hz = 0 ;		   /* Hz en el fichero */
int lastchan = 0 ;	   /* Ultimo canal con datos */
int modopiano = 0 ;
int nota = NOTE_C, tecla ;

void CopyMem (int handledest, int handleorig, unsigned size)
   {
   char buffer[2048] ;
   unsigned offset = 0, as ;

   while (size)
      {
      as = size ;
      if (as > 2048) as = 2048 ;
      memcpy (buffer, AccessMem (handleorig)+offset, as) ;
      memcpy (AccessMem (handledest)+offset, buffer, as) ;
      offset += as ;
      size -= as ;
      }
   }

long IsWAV (int handle)
   {
   /* Devuelve el tama¤o de los datos y el handle posicionado
      o 0 si no es un fichero WAV */

   char buf[4] ;
   long size ;

   lseek (handle, 8, SEEK_SET) ;
   read (handle, &buf, 4) ;
   lseek (handle, 0, SEEK_SET) ;
   if (buf[0] != 'W' || buf[1] != 'A' || buf[2] != 'V' || buf[3] != 'E')
      return 0 ;
   lseek (handle, 8+4+0xC, SEEK_SET) ;
   read (handle, (char *)&hz, 2) ;
   lseek (handle, 0x28, SEEK_SET) ;
   read (handle, (char *)&size, 4) ;
   return size ;
   }

void main (int argc, char **argv)
   {
   unsigned int i ;
   unsigned char *ptr ;

	   /* Comprueba l¡nea de comandos y rellena "filename" */

   filename[0] = 0 ;
   for (i = 1 ; i < argc ; i++)
      if (argv[i][0] == '-' || argv[i][0] == '/')
	 {
	 strupr (argv[i]) ;
	 if (argv[i][1] == 'S')
	    surround = 1 ;
	 if (argv[i][1] == 'E')
	    {
	    eco = atoi (argv[i]+2) ;
	    if (!eco) eco = 750 ;
	    }
	 if (argv[i][1] == 'H')
	    forcehz = atoi (argv[i]+2) ;
	 if (argv[i][1] == 'O')
	    forceohz = atoi (argv[i]+2) ;
	 if (argv[i][1] == 'P')
		modopiano = 1 ;
	 }
	  else
	 strcpy (filename, argv[i]) ;
   if (filename[0] == 0)
	  {
	  puts ("PLAY 1.0 ÄÄÄÄÄÄÄÄÄ (c) Jos‚ Luis Cebri n Page 1994\n"
		"\n"
		"PLAY filename [/E[xxx]] [/S] [/Hxxxx] [/P]\n"
		"\n"
		"\t/E\tReproduce el fichero con eco, xxx milisegundos cada pulso\n"
		"\t/S\tReproduce el fichero en Surround\n"
		"\t/H\tReproduce el fichero a frecuencia xxxx\n"
		"\t/H\tReproduce el fichero con frecuencia de salida xxxx\n"
		"\t/P\tEntra en \"modo piano\"\n"
		"\n"
		"Si <filename> es un fichero S3M no se tienen en cuenta los par metros.") ;
	  return ;
	  }
   strupr (filename) ;

	  /* Comprueba si es un S3M y lo ejecuta en caso afirmativo */

   if (LoadS3M (filename) >= 0)
	  {
	  if (SBStart() == -1)
		 {
		 puts ("No hay tarjeta de sonido.") ;
		 return ;
		 }
	  puts ("Tocando S3M...") ;
	  StartSong() ;
	  while (!kbhit()) ;
	  while (kbhit()) getch() ;
	  StopSong() ;
	  SBEnd() ;
	  return ;
	  }

	   /* Abre el fichero de disco */

   fichero = open (filename, O_RDONLY) ;
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

	   /* Comprueba si es un fichero WAV */

   tdata = IsWAV (fichero) ;
   if (!tdata)
      tdata = tamano ;
   else
      puts ("Fichero de formato WAVE") ;

	   /* Recupera el fichero de disco y lo cierra */

   if (read (fichero, AccessMem(handle), tdata) == -1)
      {
      puts ("Error al leer el fichero.") ;
      close (fichero) ;
      return ;
      }
   close (fichero) ;

   /* -------------------------------------------------------- */

	   /* Prepara la ESL */

   if (!forceohz)
      Hz = 32000 ;
   else
      Hz = forceohz ;
   Stereo = 1 ;

	   /* Crea un inverso del sample si hay surround */

   if (surround)
      {
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
      }

	   /* Activa la ESL */

   if (SBStart() == -1)
      {
      puts ("No hay tarjeta Sound Blaster") ;
      return ;
      }
   if (!Stereo)
      {
      puts ("Para el efecto Surround necesita stereo.") ;
      surround = 0 ;
      }

	   /* Crea un instrumento (o dos, en surround) */

   if (!hz)
      if (!forcehz)
	 hz = 13000 ;
      else
	 hz = forcehz ;

   instrumento = CreateInstrument (handle, tamano, hz, 0, 0) ;
   if (surround)
      instrumento2 = CreateInstrument (handle2, tamano, hz, 0, 0) ;

	   /* Toca el instrumento */

   if (modopiano)
      puts ("Entrando en modo piano. ESC para salir.") ;
   puts ("Reproduciendo...") ;
   while (1)
      {
      if (surround)
	 {
	 PlayInstrument (instrumento, 0, 63, nota, 4, PANNING_LEFT) ;
	 PlayInstrument (instrumento2, 1, 63, nota, 4, PANNING_RIGHT) ;
	 }
      else
	 PlayInstrument (instrumento, 0, 63, nota, 4, PANNING_MID) ;

      if (eco)
	 {
	 puts ("Creando eco...") ;
	 lastchan = 5 ;
	 delay (eco) ;
	 PlayInstrument (instrumento, 2, 32, nota, 4, PANNING_LEFT) ;
	 delay (eco) ;
	 PlayInstrument (instrumento, 3, 16, nota, 4, PANNING_RIGHT) ;
	 delay (eco) ;
	 PlayInstrument (instrumento, 4, 8, nota, 4, PANNING_MID) ;
	 delay (eco) ;
	 PlayInstrument (instrumento, 5, 4, nota, 4, PANNING_LEFT) ;
	 }

      strupr (argv[1]) ;
      if (!modopiano)
	 {
	 while (!kbhit() && Channels[lastchan].sample)
	    printf ("%s: %uK\r", argv[1], Channels[0].offset/1024) ;
	 while (kbhit()) getch() ;
	 break ;
	 }
      tecla = getch() ;
      if (!tecla) tecla = 1000+getch() ;
      if (tecla >= 'a' && tecla <= 'z') tecla -= 'a'-'A' ;
      if (tecla == 27)
	 break ;
      nota = -1 ;
      switch (tecla)
	 {
	 case 'Z':	nota = 0 ; break ;
	 case 'S':	nota = 1 ; break ;
	 case 'X':	nota = 2 ; break ;
	 case 'D':	nota = 3 ; break ;
	 case 'C':	nota = 4 ; break ;
	 case 'V':	nota = 5 ; break ;
	 case 'G':	nota = 6 ; break ;
	 case 'B':	nota = 7 ; break ;
	 case 'H':	nota = 8 ; break ;
	 case 'N':	nota = 9 ; break ;
	 case 'J':	nota = 10 ; break ;
	 case 'M':	nota = 11 ; break ;
	 }
      }

	   /* Termina con la ESL */

   SBEnd() ;
   }
