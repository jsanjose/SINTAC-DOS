/*

	Librer¡a gr fica SINTAC

	(c) 1995 Javier San Jos‚ y Jos‚ Luis Cebri n
								*/

#include "grf.h"

#include "defchr.h"

/* MODULO PRINCIPAL

	Contiene la funci¢n que establece cada modo de v¡deo y sus
	variables internas, y las funciones que llaman a las
	sub-funciones correspondientes a cada una

*/

#define max(a,b)    (((a) > (b)) ? (a) : (b))
#define min(a,b)    (((a) < (b)) ? (a) : (b))

int g_granularity ;

int g_modo, g_max_x, g_max_y, g_memmodel, g_bytesfila ;
long g_bytespantalla ;

int g_chr_alto, g_chr_ancho, g_chr_maxfil, g_chr_maxcol ;

int g_actuando = 0 ;

/* punteros a tablas con las definiciones de caracteres de 8xCHR_ALT y 8x8 */
unsigned char *_defchr=&_defchr_sys[0][0];
unsigned char *_defchr8x8=&_defchr8x8_sys[0][0];

/* punteros a tablas con las anchuras de caracteres de 8xCHR_ALT y 8x8 */
unsigned char *_anchchr=&_anchchr_sys[0];
unsigned char *_anchchr8x8=&_anchchr8x8_sys[0];

/* coordenadas de impresi¢n */
int ult_x, ult_y ;

/****************************************************************************
	G_MODOVIDEO: establece un modo de v¡deo.
	  Entrada:	'mvid' modo de v¡deo (G_MV_XXXXXX)
	  Salida:	0 si no pudo establecer modo de v¡deo, 1 si pudo
****************************************************************************/

char vesainfo[256] ;

int g_modovideo (int n)
{

	/* ESTABLECE MODO DE VIDEO */

   g_granularity = 0 ;

   switch (n)
   {
      case G_MV_T40:
      case G_MV_T80:
      case G_MV_T40C:
      case G_MV_T80C:
	asm {
		mov ax, n
		mov ah, 0
		int 10h
	   }
	return 1 ;

      case G_640x200x16:
      case G_640x350x16:
      case G_640x480x16:
      case G_320x200x256:
	asm {
		mov ax, n
		mov ah, 0
		int 10h
	   }
	break ;

      case G_640x400x256:
      case G_640x480x256:
      case G_800x600x256:
      case G_1024x768x256:
	asm {
		mov ax, 04F02h
		mov bx, n
		int 10h
	   }
	if (_AX != 0x4F)
	   return 0 ;
	asm {				/* Obtiene informaci¢n del modo */
		push es
		mov di, offset vesainfo
		mov ax, seg vesainfo
		mov es, ax
		mov ax, 04F01h
		mov cx, n
		int 10h
		pop es
	   }
	if (_AX != 0x4F)
	   return 0 ;
	g_granularity = *(int *)(vesainfo+4) ;
	if ((unsigned)*(int *)(vesainfo+8) != 0xA000 || 64 != g_granularity*(64/g_granularity))
	   return 0 ;
	break ;

      case G_360x240x256:
      case G_360x480x256:
	asm {
		push di

		mov ax,0013h	// modo 320x200x256
		int 10h
		mov dx,03c4h	// puerto del secuenciador
		mov ax,0604h	// desactiva 'Chain 4'
		out dx,ax
		mov ax,0f02h	// permite escribir en todos los planos
		out dx,ax

		mov ax,0a000h
		mov es,ax
		xor di,di	// ES:DI=inicio buffer de v¡deo
		mov cx,21600	// bytes en buffer de v¡deo, 360*480/4=43200/2=21600
		xor ax,ax
		rep stosw	// limpia la pantalla

		mov dx,03c4h	// puerto del secuenciador
		mov ax,0100h	// reset s¡ncrono
		out dx,ax
		mov dx,03c2h	// puerto miscel neo
		mov al,0e7h	// usar reloj de puntos de 28 MHz
		out dx,al
		mov dx,03c4h	// puerto del secuenciador
		mov ax,0300h	// reinicializar secuenciador
		out dx,ax
		mov dx,03d4h	// puerto del controlador del CRT
		mov al,11h	// registro 11 del CRT
		out dx,al
		inc dx
		in al,dx
		and al,7fh	// desproteger registros CR0-CR7
		out dx,al

		mov dx,03d4h	// puerto del controlador del CRT
		mov ax,6b00h	// total horizontal
		out dx,ax
		mov ax,5901h	// horizontal visualizado
		out dx,ax
		mov ax,5a02h	// inicio blanqueo horizontal
		out dx,ax
		mov ax,8e03h	// fin blanqueo horizontal
		out dx,ax
		mov ax,5e04h	// inicio sincronismo h
		out dx,ax
		mov ax,8a05h	// fin sincronismo h
		out dx,ax
		mov ax,0d06h	// total vertical
		out dx,ax
		mov ax,3e07h	// rebosamiento
		out dx,ax
		mov ax,4009h	// altura de celdilla
		out dx,ax
		mov ax,0ea10h	// inicio sincronismo v
		out dx,ax
		mov ax,0ac11h	// fin sincronismo v y protecci¢n registros CR0-CR7
		out dx,ax
		mov ax,0df12h	// vertical visualizado
		out dx,ax
		mov ax,2d13h	// desplazamiento
		out dx,ax
		mov ax,0014h	// desactiva modo DWORD
		out dx,ax
		mov ax,0e715h	// inicio blanqueo vertical
		out dx,ax
		mov ax,0616h	// fin blanqueo vertical
		out dx,ax
		mov ax,0e317h	// activa modo BYTE
		out dx,ax
		pop di
	   }
	   if (n == G_360x240x256)
	   asm {
		mov ax, 4109h
		mov dx, 03d4h	// desactiva doble altura de celdilla
		out dx, ax
	   }
	break ;

      case G_720x480x16:
	asm {
		mov al, G_640x480x16
		mov ah, 0
		int 10h

		mov dx, 03C4h
		mov ax, 100h
		out dx, ax

		mov dx, 03C2h
		mov al, 0e7h
		out dx, al

		mov dx, 03C4h
		mov ax, 300h
		out dx, ax

		mov dx, 3D4h
		mov al, 11h
		out dx, al
		inc dx
		in al, dx
		and al, 7
		out dx, al
		dec dx

		mov ax, 06b00h
		out dx, ax
		mov ax, 05901h
		out dx, ax
		mov ax, 05a02h
		out dx, ax
		mov ax, 08e03h
		out dx, ax
		mov ax, 05e04h
		out dx, ax
		mov ax, 08a05h
		out dx, ax
		mov ax, 2D13h
		out dx, ax
	   }
	break ;

      default:
	 return 0 ;
   }

	/* ESTABLECE VARIABLES INTERNAS */

   g_modo = n ;
   g_default_pointer = 0 ;
   g_raton_resx = 2 ;
   g_raton_resy = 2 ;
   switch (n)
   {
      case G_640x200x16:
	g_max_x = 639 ;
	g_max_y = 199 ;
	g_memmodel = G_MM_16C ;
	g_bytesfila = 80 ;
	g_chr_alto = 8 ;
	g_raton_resy = 4 ;
	g_default_pointer = 2 ;
	break ;
      case G_640x350x16:
	g_max_x = 639 ;
	g_max_y = 349 ;
	g_memmodel = G_MM_16C ;
	g_bytesfila = 80 ;
	g_chr_alto = 8 ;
	break ;
      case G_640x480x16:
	g_max_x = 639 ;
	g_max_y = 479 ;
	g_memmodel = G_MM_16C ;
	g_bytesfila = 80 ;
	g_chr_alto = 16 ;
	break ;
      case G_720x480x16:
	g_max_x = 719 ;
	g_max_y = 479 ;
	g_memmodel = G_MM_16C ;
	g_bytesfila = 90 ;
	g_chr_alto = 16 ;
	break ;
      case G_320x200x256:
	g_max_x = 319 ;
	g_max_y = 199 ;
	g_memmodel = G_MM_256C ;
	g_bytesfila = 320 ;
	g_chr_alto = 8 ;
	g_raton_resx = 4 ;
	g_raton_resy = 4 ;
	break ;
      case G_360x240x256:
	g_max_x = 359 ;
	g_max_y = 239 ;
	g_memmodel = G_MM_256C_X ;
	g_bytesfila = 90 ;
	g_chr_alto = 8 ;
	g_raton_resx = 4 ;
	g_raton_resy = 4 ;
	break ;
      case G_360x480x256:
	g_max_x = 359 ;
	g_max_y = 479 ;
	g_memmodel = G_MM_256C_X ;
	g_bytesfila = 90 ;
	g_chr_alto = 16 ;
	g_raton_resx = 4 ;
	g_default_pointer = 1 ;
	break ;
      case G_640x400x256:
	g_max_x = 639 ;
	g_max_y = 399 ;
	g_memmodel = G_MM_256C_V ;
	g_bytesfila = 640 ;
	g_chr_alto = 16 ;
	break ;
      case G_640x480x256:
	g_max_x = 639 ;
	g_max_y = 479 ;
	g_memmodel = G_MM_256C_V ;
	g_bytesfila = 640 ;
	g_chr_alto = 16 ;
	break ;
      case G_800x600x256:
	g_max_x = 799 ;
	g_max_y = 599 ;
	g_memmodel = G_MM_256C_V ;
	g_bytesfila = 800 ;
	g_chr_alto = 16 ;
	break ;
      case G_1024x768x256:
	g_max_x = 1023 ;
	g_max_y = 767 ;
	g_memmodel = G_MM_256C_V ;
	g_bytesfila = 1024 ;
	g_chr_alto = 16 ;
	break ;
   }
   g_bytespantalla = (long)g_bytesfila*(g_max_y+1) ;
   g_chr_ancho = 8 ;
   g_chr_maxfil = g_max_y/g_chr_alto ;
   g_chr_maxcol = g_max_x/g_chr_ancho ;

   return 1 ;
}

/****************************************************************************
		     Funciones necesarias para modos VESA
****************************************************************************/

int lzone = 0, lizone = 0 ;

void g_setwindow (int zone)
{
   if (!g_granularity || lizone == zone) return ;
   lizone = zone ;
   zone *= 64 ;
   zone /= g_granularity ;
   lzone = zone ;
   asm {
	mov ax, 04F05h
	mov bx, 0
	mov dx, zone
	int 10h
      }
}

void g_incwindow ()
{
   lzone += 64/g_granularity ;
   lizone++ ;
   asm {
	mov ax, 04F05h
	mov bx, 0
	mov dx, lzone
	int 10h
      }
}

/****************************************************************************
	Estas funciones se conservan por razones de compatibilidad
****************************************************************************/


int g_maxx() { return g_max_x+1 ; }
int g_maxy() { return g_max_y+1 ; }
int chr_altura() { return g_chr_alto ; }
int chr_maxfil() { return g_chr_maxfil ; }
int chr_maxcol() { return g_chr_maxcol ; }

/***************************************************************************/

void g_borra_pantalla16 (void) ;
void g_borra_pantalla256 (void) ;
void g_borra_pantalla256x (void) ;
void g_borra_pantalla256v (void) ;

void g_borra_pantalla ()
{
   g_actuando = 1 ;
   if (g_autoraton)
      rg_desactivar() ;

   switch (g_memmodel)
   {
      case G_MM_16C: 	g_borra_pantalla16 () ; break ;
      case G_MM_256C: 	g_borra_pantalla256 () ; break ;
      case G_MM_256C_X: g_borra_pantalla256x () ; break ;
      case G_MM_256C_V: g_borra_pantalla256v () ; break ;
   }

   if (g_autoraton)
      rg_activar() ;
   g_actuando = 0 ;
}

unsigned char g_coge_pixel16 (int x, int y) ;
unsigned char g_coge_pixel256 (int x, int y) ;
unsigned char g_coge_pixel256x (int x, int y) ;
unsigned char g_coge_pixel256v (int x, int y) ;

unsigned char g_coge_pixel (int x, int y)
{
   int rv ;

   g_actuando = 1 ;
   if (g_autoraton)
   if (g_rg_x <= x && g_rg_y <= y && g_rg_x2 >= x && g_rg_y2 >= y)
      rg_desactivar() ;

   rv = 0 ;
   switch (g_memmodel)
   {
      case G_MM_16C: 	rv = g_coge_pixel16 (x,y) ; break ;
      case G_MM_256C: 	rv = g_coge_pixel256 (x,y) ; break ;
      case G_MM_256C_X: rv = g_coge_pixel256x (x,y) ; break ;
      case G_MM_256C_V: rv = g_coge_pixel256v (x,y) ; break ;
   }

   if (g_autoraton)
   if (g_rg_x <= x && g_rg_y <= y && g_rg_x2 >= x && g_rg_y2 >= y)
      rg_activar() ;
   else
      rg_actualizar() ;

   g_actuando = 0 ;
   return rv ;
}

void g_punto16 (int x, int y, unsigned char color, unsigned char modo) ;
void g_punto256 (int x, int y, unsigned char color, unsigned char modo) ;
void g_punto256x (int x, int y, unsigned char color, unsigned char modo) ;
void g_punto256v (int x, int y, unsigned char color, unsigned char modo) ;

void g_punto (int x, int y, unsigned char color, unsigned char modo)
{
   g_actuando = 1 ;
   if (g_autoraton)
   if (g_rg_x <= x && g_rg_y <= y && g_rg_x2 >= x && g_rg_y2 >= y)
      rg_desactivar() ;

   switch (g_memmodel)
   {
      case G_MM_16C: 	g_punto16 (x,y,color,modo) ; break ;
      case G_MM_256C: 	g_punto256 (x,y,color,modo) ; break ;
      case G_MM_256C_X: g_punto256x (x,y,color,modo) ; break ;
      case G_MM_256C_V: g_punto256v (x,y,color,modo) ; break ;
   }

   if (g_autoraton)
   if (g_rg_x <= x && g_rg_y <= y && g_rg_x2 >= x && g_rg_y2 >= y)
      rg_activar() ;
   else
      rg_actualizar() ;
   g_actuando = 0 ;
}

void g_linea16 (int x0, int y0, int x1, int y1, unsigned char color, unsigned char modo) ;
void g_linea256 (int x0, int y0, int x1, int y1, unsigned char color, unsigned char modo) ;
void g_linea256x (int x0, int y0, int x1, int y1, unsigned char color, unsigned char modo) ;
void g_linea256v (int x0, int y0, int x1, int y1, unsigned char color, unsigned char modo) ;

void g_linea (int x0, int y0, int x1, int y1, unsigned char color, unsigned char modo)
{
   g_actuando = 1 ;
   if (g_autoraton)
   if (g_rg_x < max(x0,x1) && g_rg_y < max(y0,y1) && g_rg_x2 > min(x0,x1) && g_rg_y2 > min(y0,y1))
      rg_desactivar() ;

   switch (g_memmodel)
   {
      case G_MM_16C: 	g_linea16 (x0,y0,x1,y1,color,modo) ; break ;
      case G_MM_256C: 	g_linea256 (x0,y0,x1,y1,color,modo) ; break ;
      case G_MM_256C_X: g_linea256x (x0,y0,x1,y1,color,modo) ; break ;
      case G_MM_256C_V: g_linea256v (x0,y0,x1,y1,color,modo) ; break ;
   }

   if (g_autoraton)
   if (g_rg_x < max(x0,x1) && g_rg_y < max(y0,y1) && g_rg_x2 > min(x0,x1) && g_rg_y2 > min(y0,y1))
      rg_activar() ;
   else
      rg_actualizar() ;
   g_actuando = 0 ;
}

void g_rectangulo16 (int x0, int y0, int x1, int y1, unsigned char color, unsigned char modo) ;
void g_rectangulo256 (int x0, int y0, int x1, int y1, unsigned char color, unsigned char modo) ;
void g_rectangulo256x (int x0, int y0, int x1, int y1, unsigned char color, unsigned char modo) ;
void g_rectangulo256xnorm (int x0, int y0, int x1, int y1, unsigned char color) ;
void g_rectangulo256v (int x0, int y0, int x1, int y1, unsigned char color, unsigned char modo) ;

void g_rectangulo (int x0, int y0, int x1, int y1, unsigned char color, unsigned char modo, int relleno)
{
   if(!relleno) {
	   g_linea(x0,y0,x1,y0,color,modo);
	   g_linea(x1,y0+1,x1,y1,color,modo);
	   g_linea(x1-1,y1,x0,y1,color,modo);
	   g_linea(x0,y1-1,x0,y0+1,color,modo);
	   return ;
   }

   g_actuando = 1 ;
   if (g_autoraton)
   if (g_rg_x < x1 && g_rg_y < y1 && g_rg_x2 > x0 && g_rg_y2 > y0)
      rg_desactivar() ;

   switch (g_memmodel)
      {
	 case G_MM_16C: 	g_rectangulo16 (x0,y0,x1,y1,color,modo) ; break ;
	 case G_MM_256C: 	g_rectangulo256 (x0,y0,x1,y1,color,modo) ; break ;
	 case G_MM_256C_X: if (modo == G_NORM)
			       g_rectangulo256xnorm (x0,y0,x1,y1,color) ;
			   else
			       g_rectangulo256x (x0,y0,x1,y1,color,modo) ;
			   break ;
	 case G_MM_256C_V: 	g_rectangulo256v (x0,y0,x1,y1,color,modo) ; break ;
      }

   if (g_autoraton)
   if (g_rg_x < x1 && g_rg_y < y1 && g_rg_x2 > x0 && g_rg_y2 > y0)
      rg_activar() ;
   else
      rg_actualizar() ;
   g_actuando = 0 ;
}

void g_scroll_arr16 (int l, int c, int an, int al, unsigned char color) ;
void g_scroll_arr256 (int l, int c, int an, int al, unsigned char color) ;
void g_scroll_arr256x (int l, int c, int an, int al, unsigned char color) ;
void g_scroll_arr256v(int l, int c, int an, int al, unsigned char color) ;

void g_scroll_arr (int l, int c, int an, int al, unsigned char color)
{
   int rx, ry ;

   g_actuando = 1 ;
   if (g_autoraton)
   {
      rx = c*8 ;
      ry = l*chr_altura() ;
      if (g_rg_x < rx+an*8 && g_rg_y < ry+al*chr_altura() && g_rg_x2 > rx && g_rg_y2 > ry)
	 rg_desactivar() ;
   }

   switch (g_memmodel)
   {
      case G_MM_16C: 	g_scroll_arr16 (l,c,an,al,color) ; break ;
      case G_MM_256C: 	g_scroll_arr256 (l,c,an,al,color) ; break ;
      case G_MM_256C_X: g_scroll_arr256x (l,c,an,al,color) ; break ;
      case G_MM_256C_V: g_scroll_arr256v (l,c,an,al,color) ; break ;
   }

   if (g_autoraton)
      if (g_rg_x < rx+an*8 && g_rg_y < ry+al*chr_altura() && g_rg_x2 > rx && g_rg_y2 > ry)
	 rg_activar() ;
      else
	 rg_actualizar() ;
   g_actuando = 0 ;
}

void g_scroll_abj16 (int l, int c, int an, int al, unsigned char color) ;
void g_scroll_abj256 (int l, int c, int an, int al, unsigned char color) ;
void g_scroll_abj256x (int l, int c, int an, int al, unsigned char color) ;
void g_scroll_abj256v (int l, int c, int an, int al, unsigned char color) ;

void g_scroll_abj (int l, int c, int an, int al, unsigned char color)
{
   int rx, ry ;

   g_actuando = 1 ;
   if (g_autoraton)
   {
      rx = c*8 ;
      ry = l*chr_altura() ;
      if (g_rg_x < rx+an*8 && g_rg_y < ry+al*chr_altura() && g_rg_x2 > rx && g_rg_y2 > ry)
	 rg_desactivar() ;
   }

   switch (g_memmodel)
   {
      case G_MM_16C: 	g_scroll_abj16 (l,c,an,al,color) ; break ;
      case G_MM_256C: 	g_scroll_abj256 (l,c,an,al,color) ; break ;
      case G_MM_256C_X: g_scroll_abj256x (l,c,an,al,color) ; break ;
      case G_MM_256C_V: g_scroll_abj256v (l,c,an,al,color) ; break ;
   }

   if (g_autoraton)
      if (g_rg_x < rx+an*8 && g_rg_y < ry+al*chr_altura() && g_rg_x2 > rx && g_rg_y2 > ry)
	 rg_activar() ;
      else
	 rg_actualizar() ;
   g_actuando = 0 ;
}

int imp_chr16(unsigned char chr, unsigned char colorf, unsigned char color,
  unsigned char modo) ;
int imp_chr256(unsigned char chr, unsigned char colorf, unsigned char color,
  unsigned char modo) ;
int imp_chr256x(unsigned char chr, unsigned char colorf, unsigned char color,
  unsigned char modo) ;
int imp_chr256v(unsigned char chr, unsigned char colorf, unsigned char color,
  unsigned char modo) ;


int imp_chr(unsigned char chr, unsigned char colorf, unsigned char color,
  unsigned char modo)
{
   int rx, ry, rv ;

   g_actuando = 1 ;
   if (g_autoraton)
   {
      rx = ult_x ;
      ry = ult_y ;
      if (g_rg_x < rx+8 && g_rg_y < ry+chr_altura() && g_rg_x2 > rx && g_rg_y2 > ry)
	 rg_desactivar() ;
   }

   rv = 0 ;
   switch (g_memmodel)
   {
      case G_MM_16C: 	rv = imp_chr16(chr,colorf,color,modo) ; break ;
      case G_MM_256C: 	rv = imp_chr256(chr,colorf,color,modo) ; break ;
      case G_MM_256C_X:	rv = imp_chr256x(chr,colorf,color,modo) ; break ;
      case G_MM_256C_V:	rv = imp_chr256v(chr,colorf,color,modo) ; break ;
   }

   if (g_autoraton)
      if (g_rg_x < rx+8 && g_rg_y < ry+chr_altura() && g_rg_x2 > rx && g_rg_y2 > ry)
	 rg_activar() ;
      else
	 rg_actualizar() ;

   g_actuando = 0 ;
   return rv ;
}

void blq_coge16(int x0, int y0, int x1, int y1, unsigned char far *blq) ;
void blq_coge256(int x0, int y0, int x1, int y1, unsigned char far *blq) ;
void blq_coge256x(int x0, int y0, int x1, int y1, unsigned char far *blq) ;
void blq_coge256v(int x0, int y0, int x1, int y1, unsigned char far *blq) ;

void blq_coge(int x0, int y0, int x1, int y1, unsigned char far *blq)
{
   g_actuando = 1 ;
   if (g_autoraton)
   if (g_rg_x < x1 && g_rg_y < y1 && g_rg_x2 > x0 && g_rg_y2 > y0)
      rg_desactivar() ;

   switch (g_memmodel)
   {
      case G_MM_16C: 	blq_coge16 (x0,y0,x1,y1,blq) ; break ;
      case G_MM_256C: 	blq_coge256 (x0,y0,x1,y1,blq) ; break ;
      case G_MM_256C_X: blq_coge256x (x0,y0,x1,y1,blq) ; break ;
      case G_MM_256C_V: blq_coge256v (x0,y0,x1,y1,blq) ; break ;
   }

   if (g_autoraton)
   if (g_rg_x < x1 && g_rg_y < y1 && g_rg_x2 > x0 && g_rg_y2 > y0)
      rg_activar() ;
   else
      rg_actualizar() ;
   g_actuando = 0 ;
}

void blq_pon16(int x, int y, unsigned char far *blq) ;
void blq_pon256(int x, int y, unsigned char far *blq) ;
void blq_pon256x(int x, int y, unsigned char far *blq) ;
void blq_pon256v(int x, int y, unsigned char far *blq) ;

void blq_pon(int x, int y, unsigned char far *blq)
{
   g_actuando = 1 ;
   if (g_autoraton)
   if (g_rg_x < x+blq_ancho(blq) && g_rg_y < y+blq_alto(blq) &&
	  g_rg_x2 > x && g_rg_y2 > y)
      rg_desactivar() ;

   switch (g_memmodel)
   {
      case G_MM_16C: 	blq_pon16 (x,y,blq) ; break ;
      case G_MM_256C: 	blq_pon256 (x,y,blq) ; break ;
      case G_MM_256C_X: blq_pon256x (x,y,blq) ; break ;
      case G_MM_256C_V: blq_pon256v (x,y,blq) ; break ;
   }

   if (g_autoraton)
   if (g_rg_x < x+blq_ancho(blq) && g_rg_y < y+blq_alto(blq) &&
	  g_rg_x2 > x && g_rg_y2 > y)
      rg_activar() ;
   else
      rg_actualizar() ;
   g_actuando = 0 ;
}

void blq_clip16(int x, int y, int ancho, int alto, unsigned char far *blq) ;
void blq_clip256(int x, int y, int ancho, int alto, unsigned char far *blq) ;
void blq_clip256x(int x, int y, int ancho, int alto, unsigned char far *blq) ;
void blq_clip256v(int x, int y, int ancho, int alto, unsigned char far *blq) ;

void blq_clip(int x, int y, int ancho, int alto, unsigned char far *blq)
{
   ancho = min (blq_ancho(blq), ancho) ;
   alto = min (blq_alto(blq), alto) ;

   g_actuando = 1 ;
   if (g_autoraton)
   if (g_rg_x < x+ancho && g_rg_y < y+alto && g_rg_x2 > x && g_rg_y2 > y)
      rg_desactivar() ;

   switch (g_memmodel)
   {
      case G_MM_16C: 	blq_clip16 (x,y,ancho,alto,blq) ; break ;
      case G_MM_256C: 	blq_clip256 (x,y,ancho,alto,blq) ; break ;
      case G_MM_256C_X: blq_clip256x (x,y,ancho,alto,blq) ; break ;
      case G_MM_256C_V: blq_clip256v (x,y,ancho,alto,blq) ; break ;
   }

   if (g_autoraton)
   if (g_rg_x < x+ancho && g_rg_y < y+alto && g_rg_x2 > x && g_rg_y2 > y)
      rg_activar() ;
   else
      rg_actualizar() ;
   g_actuando = 0 ;
}

/***************************************************************************/

void imp_chr_pos(int x, int y)
{
   ult_x=x;
   ult_y=y;
}

void def_chr_set(unsigned char *ptr_set1, unsigned char *ptr_set2,
  unsigned char *ptr_anch1, unsigned char *ptr_anch2)
{
   if(ptr_set1==0)
      _defchr=&_defchr_sys[0][0];
   else
      _defchr=ptr_set1;

   if(ptr_set2==0)
      _defchr8x8=&_defchr8x8_sys[0][0];
   else
      _defchr8x8=ptr_set2;

   if(ptr_anch1==0)
      _anchchr=&_anchchr_sys[0];
   else
      _anchchr=ptr_anch1;

   if(ptr_anch2==0)
      _anchchr8x8=&_anchchr8x8_sys[0];
   else
      _anchchr8x8=ptr_anch2;
}

unsigned long blq_tam(int x0, int y0, int x1, int y1)
{
   if (g_memmodel == G_MM_16C)
      return (x1-x0+1)*(long)(y1-y0+1)/2L + 5L ;
   if (g_memmodel == G_MM_256C || g_memmodel == G_MM_256C_X || g_memmodel == G_MM_256C_V)
      return (x1-x0+1)*(long)(y1-y0+1) + 4L ;
   return 4L ;
}

int chr_anchura(unsigned char chr)
{
   if(g_chr_alto==8) return(_anchchr8x8[chr]);
   if(g_chr_alto==16) return(_anchchr[chr]);
   return 0 ;
}

int blq_ancho (char *b)
{
   if (g_memmodel == G_MM_16C)
      return *(int *)(b+2) * 8 ;
   if (g_memmodel == G_MM_256C || g_memmodel == G_MM_256C_X || g_memmodel == G_MM_256C_V)
      return *(int *)b ;
   return 0 ;
}

int blq_alto (char *b)
{
   if (g_memmodel == G_MM_16C)
      return *(int *)b ;
   if (g_memmodel == G_MM_256C || g_memmodel == G_MM_256C_X || g_memmodel == G_MM_256C_V)
      return *(int *)(b+2) ;
   return 0 ;
}