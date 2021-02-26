#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <dos.h>
#include <time.h>

#include "graf.h"

int mustdelay = 1 ;

int mydelay(int c)
{
	if (mustdelay) delay(c) ;
	return 1 ;
}


//#define delay(c)

void imp(char *t, int color1, int color2, int modo)
{
	while(*t) {
		imp_chr(*t,color1,color2,modo);
		t++;
	}
}

void text_test(void)
{
	int x, y ;

	g_borra_pantalla() ;
	imp_chr_pos (0,0) ;
	imp ("TESTS de textos", 0, 15, G_NORM) ;

	imp_chr_pos (0,32) ;
	imp ("Normal ", 0, 15, G_NORM) ;
	imp ("Inverso", 15, 0, G_NORM) ;
	imp (" ", 0, 15, G_NORM) ;
	imp ("azul sobre verde", 2, 1, G_NORM) ;
	imp (" ", 0, 15, G_NORM) ;
	imp ("verde sobre azul", 1, 2, G_NORM) ;
	while (!kbhit() && !raton.boton1) ;
	while (kbhit())	getch() ;
	while (raton.boton1) ;
	for (y = 1 ; y <= chr_maxfil() ; y++)
	   for (x = 0 ; x <= chr_maxcol() ; x++)
	   {
	      imp_chr_pos (8*x, chr_altura()*y) ;
	      imp ("A", 0, 15, G_NORM) ;
	   }
	while (!kbhit() && !raton.boton1) ;
	while (kbhit())	getch() ;
	while (raton.boton1) ;

	while (!kbhit() && !raton.boton1)
	{
		x = random(g_maxx()-80) ;
		y = random(g_maxy()-32)+16 ;
		imp_chr_pos (x, y) ;
		imp ("Abcdefghij", random(16), random(16), G_NORM) ;
		mydelay (150) ;
	}
	while (raton.boton1) ;
	while (kbhit()) getch() ;

	g_rectangulo (0, 0, g_maxx(), chr_altura()-1, 0, G_NORM, 1) ;
	imp_chr_pos (0,0) ;
	imp ("TESTS de textos: XOR", 0, 15, G_NORM) ;
	while (!kbhit() && !raton.boton1)
	{
		x = random(g_maxx()-80) ;
		y = random(g_maxy()-32)+16 ;
		imp_chr_pos (x, y) ;
		imp ("Abcdefghij", 0, random(16), G_XOR) ;
		mydelay (150) ;
	}
	while (raton.boton1) ;
	while (kbhit()) getch() ;
}

void rectangle_test(void)
{
	int rx0, rx1, ry0, ry1 ;

	g_borra_pantalla() ;
	imp_chr_pos (0,0) ;
	imp ("TESTS de rect ngulo: s¢lidos", 0, 15, G_NORM) ;

	while (!kbhit() && !raton.boton1)
	{
	  rx0 = random (g_maxx()) ;
	  ry0 = chr_altura()+random (g_maxy()-chr_altura()) ;
	  rx1 = rx0+random (g_maxx()-rx0) ;
	  ry1 = ry0+random (g_maxy()-ry0) ;
	  g_rectangulo (rx0, ry0, rx1, ry1, random(256), G_NORM, 1) ;
	  mydelay (250) ;
	}
	while (raton.boton1) ;
	while (kbhit()) getch() ;

	g_rectangulo (0, 0, g_maxx(), chr_altura()-1, 0, G_NORM, 1) ;
	imp_chr_pos (0,0) ;
	imp ("TESTS de rect ngulo: huecos", 0, 15, G_NORM) ;

	while (!kbhit() && !raton.boton1)
	{
	  rx0 = random (g_maxx()) ;
	  ry0 = chr_altura()+random (g_maxy()-chr_altura()) ;
	  rx1 = rx0+random (g_maxx()-rx0) ;
	  ry1 = ry0+random (g_maxy()-ry0) ;
	  g_rectangulo (rx0, ry0, rx1, ry1, random(256), G_NORM, 0) ;
	  mydelay (250) ;
	}
	while (raton.boton1) ;
	while (kbhit()) getch() ;

	g_rectangulo (0, 0, g_maxx(), chr_altura()-1, 0, G_NORM, 1) ;
	imp_chr_pos (0,0) ;
	imp ("TESTS de rect ngulo: s¢lidos, XOR", 0, 15, G_NORM) ;

	while (!kbhit() && !raton.boton1)
	{
	  rx0 = random (g_maxx()) ;
	  ry0 = chr_altura()+random (g_maxy()-chr_altura()) ;
	  rx1 = rx0+random (g_maxx()-rx0) ;
	  ry1 = ry0+random (g_maxy()-ry0) ;
	  g_rectangulo (rx0, ry0, rx1, ry1, random(256), G_XOR, 1) ;
	  mydelay (250) ;
	}
	while (raton.boton1) ;
	while (kbhit()) getch() ;
}

void lines_test(void)
{
	int rx0, rx1, ry0, ry1 ;
	int frx0[16], fry0[16], frx1[16], fry1[16], rptr, rlptr ;
	int i = chr_altura(), maxy = g_maxy() ;

	g_borra_pantalla() ;
	imp_chr_pos (0,0) ;
	imp ("TESTS de l¡neas", 0, 15, G_NORM) ;

	g_linea (0,i,g_maxx()-1,i,15, G_NORM) ;
	g_linea (0,i,0,maxy-1,15, G_NORM) ;
	g_linea (g_maxx()-1,i,g_maxx()-1,maxy-1,15, G_NORM) ;
	g_linea (0,maxy-1,g_maxx()-1,maxy-1,15, G_NORM) ;
	g_linea (0,i,g_maxx()-1,maxy-1,15, G_NORM) ;
	g_linea (0,maxy-1,g_maxx()-1,i,15, G_NORM) ;

	rx0 = random (g_maxx()) ;
	ry0 = chr_altura()+random (g_maxy()-chr_altura()) ;
	while (!kbhit() && !raton.boton1)
	{
	  rx1 = random (g_maxx()) ;
	  ry1 = chr_altura()+random (g_maxy()-chr_altura()) ;
	  g_linea (rx0, ry0, rx1, ry1, random(256), G_NORM) ;
	  mydelay (250) ;
	  rx0 = rx1 ;
	  ry0 = ry1 ;
	}
	while (raton.boton1) ;
	while (kbhit()) getch() ;

	g_borra_pantalla() ;
	imp_chr_pos (0,0) ;
	imp ("TESTS de l¡neas: XOR", 0, 15, G_NORM) ;

	rptr = 0 ;
	rlptr = -1 ;
	frx0[0] = random (g_maxx()) ;
	fry0[0] = chr_altura()+random (g_maxy()-chr_altura()) ;
	while (!kbhit() && !raton.boton1)
	{
	  frx1[rptr] = random (g_maxx()) ;
	  fry1[rptr] = chr_altura()+random (g_maxy()-chr_altura()) ;
	  g_linea (frx0[rptr], fry0[rptr], frx1[rptr], fry1[rptr], rptr, G_XOR) ;
	  if (rlptr >= 0)
		  g_linea (frx0[rlptr], fry0[rlptr], frx1[rlptr], fry1[rlptr], rlptr, G_XOR) ;
	  if (rlptr >= 0 && ++rlptr == 16) rlptr = 0 ;
	  if (++rptr == 16)
	  {
		  rptr = 0 ;
		  frx0[0] = frx0[15] ;
		  frx1[0] = frx1[15] ;
	  }
	  else
	  {
		  frx0[rptr] = frx1[rptr-1] ;
		  fry0[rptr] = fry1[rptr-1] ;
		  if (rptr == 15) rlptr = 0 ;
	  }

	  mydelay (75) ;
	}
	while (raton.boton1) ;
	while (kbhit()) getch() ;
}

void pixels_test(void)
{
	int x[64],y[64],rptr,lptr ;

	g_borra_pantalla() ;
	imp_chr_pos (0,0) ;
	imp ("TESTS de pixels", 0, 15, G_NORM) ;

	while (!kbhit() && !raton.boton1)
	  g_punto (random(g_maxx()), chr_altura()+random(g_maxy()-chr_altura()), random(256), G_NORM) ;
	while (raton.boton1) ;
	while (kbhit()) getch() ;

	g_borra_pantalla() ;
	imp_chr_pos (0,0) ;
	imp ("TESTS de pixels: cogepixel", 0, 15, G_NORM) ;

	lptr = -1 ;
	rptr = 0 ;
	while (!kbhit() && !raton.boton1)
	{
		x[rptr] = random(g_maxx()) ;
		y[rptr] = chr_altura()+random(g_maxy()-chr_altura()) ;
		g_punto (x[rptr], y[rptr], g_coge_pixel (x[rptr],y[rptr])+15, G_NORM) ;
		if (++rptr == 64) rptr = 0 ;
		if (lptr >= 0)
		{
			g_punto (x[rptr], y[rptr], g_coge_pixel (x[rptr],y[rptr])-15, G_NORM) ;
			if (++lptr == 64) lptr = 0 ;
		}
		if (rptr == 63) lptr = 0 ;
	}
	while (raton.boton1) ;
	while (kbhit()) getch() ;
}

void scroll_test(void)
{
	int i = 0 ;

	g_borra_pantalla() ;
	imp_chr_pos (0,0) ;
	imp ("TESTS de scroll", 0, 15, G_NORM) ;
	g_rectangulo (0,chr_altura(),g_maxx()-1,g_maxy()-1,1,G_NORM,1) ;

	while (!kbhit() && !raton.boton1)
	{
		g_scroll_arr (2,2,10,chr_altura(), i) ;
		g_scroll_abj (2,14,10,chr_altura(), i++) ;
		mydelay(250) ;
	}
	while (raton.boton1) ;
	while (kbhit()) getch() ;
}

void bloq_test(void)
{
	char *blq ;

	g_borra_pantalla() ;
	imp_chr_pos (0,0) ;
	imp ("TEST de bloque", 0, 15, G_NORM) ;

	g_linea (10,20,40,50, 15,G_NORM) ;
	g_linea (10,20,40,20, 15,G_NORM) ;
	g_linea (10,20,10,50, 15,G_NORM) ;
	g_linea (40,20,40,50, 15,G_NORM) ;
	g_linea (10,50,40,50, 15,G_NORM) ;
	g_linea (40,20,10,50, 15,G_NORM) ;
	g_linea (24,20,24,50, 14,G_NORM) ;
	g_linea (10,34,40,34, 14,G_NORM) ;

	blq = (char *)malloc(blq_tam(10,20,40,50)) ;
	if (blq)
	{
		blq_coge (10,20,40,50,blq) ;
		while (!kbhit() && !raton.boton1)
		{
			blq_pon (random(g_maxx()-30), chr_altura()+random(g_maxy()-chr_altura()-30), blq) ;
			mydelay(100) ;
		}
		while (raton.boton1) ;
		while (kbhit()) getch() ;
		g_rectangulo (0, 0, g_maxx(), chr_altura()-1, 0, G_NORM, 1) ;
		imp_chr_pos (0,0) ;
		imp ("TESTS de bloque: clipping", 0, 15, G_NORM) ;
		while (!kbhit() && !raton.boton1)
		{
			blq_clip (random(g_maxx()-15), chr_altura()+random(g_maxy()-chr_altura()-15), 15, 15, blq) ;
			mydelay(100) ;
		}
		while (raton.boton1) ;
		while (kbhit()) getch() ;
		free(blq) ;
	}
}

float seg_rect, seg_chr, seg_pix, seg_lin, seg_blo, seg_scr ;

void benchmark(void)
{
   clock_t c1, c2 ;
   int x, y, x2, y2 ;
   long nl ;
   char *bl, *bl2 ;

   g_borra_pantalla() ;
   imp_chr_pos (0,0) ;
   imp ("Benchmark... ­Espere!", 0, 15, G_NORM) ;

   /* ----------------------------------------------------------- */
   nl = 0 ;
   c2 = clock() ;
   y2 = 20000 ;
   if (chr_altura() == 16) y2 = 10000 ;
   while ((c1 = clock()) == c2) ;
   while (nl++ < y2)
   {
      imp_chr_pos (random(chr_maxcol()*8), (1+random(chr_maxfil()-1))*chr_altura()) ;
      imp_chr ('A',15,4,G_NORM) ;
   }
   c2 = clock() ;
   seg_chr = (c2-c1)/CLK_TCK ;

   /* ----------------------------------------------------------- */
   nl = 0 ;
   c2 = clock() ;
   while ((c1 = clock()) == c2) ;
   while (nl++ < 10000)
   {
      x = random(g_maxx()-32) ;
      y = chr_altura()+random(g_maxy()-32-chr_altura()) ;
      g_rectangulo (x,y,x+32,y+32, random(256), G_NORM, 1) ;
   }
   c2 = clock() ;
   seg_rect = (c2-c1)/CLK_TCK ;

   /* ----------------------------------------------------------- */
   nl = 0 ;
   c2 = clock() ;
   while ((c1 = clock()) == c2) ;
   while (nl++ < 20000)
   {
	   x = random(g_maxx()) ;
	   y = chr_altura()+random(g_maxy()-chr_altura()) ;
	   g_punto (x,y,random(256), G_NORM) ;
   }
   c2 = clock() ;
   seg_pix = (c2-c1)/CLK_TCK ;

   /* ----------------------------------------------------------- */
   nl = 0 ;
   c2 = clock() ;
   while ((c1 = clock()) == c2) ;
      while (nl++ < 10000)
   {
      x = random(g_maxx()-64) ;
      y = chr_altura()+random(g_maxy()-64-chr_altura()) ;
      g_linea (x,y,x+64,y+64, random(256), G_NORM) ;
      x = random(g_maxx()-64) ;
      y = 64+chr_altura()+random(g_maxy()-64-chr_altura()) ;
      g_linea (x,y,x+64,y-64, random(256), G_NORM) ;
   }
   c2 = clock() ;
   seg_lin = (c2-c1)/CLK_TCK ;

   /* ----------------------------------------------------------- */
   nl = 0 ;
   c2 = clock() ;
   while ((c1 = clock()) == c2) ;
   bl = (char *)malloc (blq_tam (0, 0, 63, 63)) ;
   bl2 = (char *)malloc (blq_tam (0, 0, 63, 63)) ;
   if (bl && bl2)
   while (nl++ < 1000)
      {
      x = 64*random(g_maxx()/64) ;
      y = chr_altura()+64*random((g_maxy()-chr_altura())/64) ;
      x2 = 64*random(g_maxx()/64) ;
      y2 = chr_altura()+64*random((g_maxy()-chr_altura())/64) ;
      blq_coge (x, y, x+63, y+63, bl) ;
      blq_coge (x2, y2, x2+63, y2+63, bl2) ;
      blq_pon (x, y, bl2) ;
      blq_pon (x2, y2, bl) ;
      }
   free (bl) ;
   free (bl2) ;
   c2 = clock() ;
   seg_blo = (c2-c1)/CLK_TCK ;

   /* ----------------------------------------------------------- */
   nl = 0 ;
   c2 = clock() ;
   y2 = 8 ;
   if (chr_altura() == 16) y2 = 4 ;
   while ((c1 = clock()) == c2) ;
   while (nl++ < 2000)
      {
      x = random(chr_maxcol()-8) ;
      y = 1+random(chr_maxfil()-y2-1) ;
      g_scroll_arr (y, x, 8, y2, random(256)) ;
      }
   c2 = clock() ;
   seg_scr = (c2-c1)/CLK_TCK ;
}

void main ()
{
	int modo ;

	g_modovideo (G_MV_T80C) ;

	while (1)
	{
	printf ("  14     640x200 16 colores\n"
			  "  16     640x350 16 colores\n"
			  "  18     640x480 16 colores\n"
			  "  19     320x200 256 colores\n"
			  " 240     720x480 16 colores\n"
			  " 254     360x240 256 colores\n"
			  " 255     360x480 256 colores\n\n"
			  " 256     640x400 256 colores\n"
			  " 257     640x480 256 colores\n"
			  " 259     800x600 256 colores\n"
			  " 261     1024x768 256 colores\n\n"
			  " ¨Modo? ") ;
	scanf ("%i", &modo) ;
	if (!modo) break ;

	printf ("¨Tipo de test? (0 r pido, 1 lento, 2 benchmark) ") ;
	scanf ("%i", &mustdelay) ;

	if (g_modovideo (modo))
	{
	   if (mustdelay < 2)
	   {
	      rg_inicializa() ;

	      text_test () ;
	      rectangle_test () ;
	      lines_test () ;
	      pixels_test () ;
	      scroll_test () ;
	      bloq_test() ;

	      rg_desconecta() ;
	      g_modovideo (G_MV_T80C) ;
	   }
	   else
	   {
	      benchmark() ;

	      g_modovideo (G_MV_T80C) ;
	      printf ("Datos del benchmark:\n"
		      "\n"
		      "Rect ngulos    %f s\n"
		      "Caracteres     %f s\n"
		      "Pixels         %f s\n"
		      "L¡neas         %f s\n"
		      "Bloques        %f s\n"
		      "Scroll         %f s\n\n"
		      , seg_rect, seg_chr, seg_pix, seg_lin, seg_blo, seg_scr) ;
	   }
	}
	else
	{
	   g_modovideo (G_MV_T80C) ;
	   printf ("Error al inicializar modo gr fico\n\n") ;
	}
	}
}