/*

   Librer¡a gr fica SINTAC

   (c) 1995 Javier San Jos‚ y Jos‚ Luis Cebri n
			*/

/* MODULO GRFM

   Contiene las funciones de rat¢n

*/

#include <mem.h>

#include "grf.h"

#define max(a,b)    (((a) > (b)) ? (a) : (b))
#define min(a,b)    (((a) < (b)) ? (a) : (b))

int g_raton = 0 ;
int g_raton_activado = 0 ;
int g_default_pointer = 0 ;
int g_raton_resx, g_raton_resy ;
int g_autoraton = 0 ;
int g_raton_color = 15 ;

int g_rg_x, g_rg_y, g_rg_x2, g_rg_y2 ;

STC_RATONG raton ;
STC_RATONG old_raton ;

#define MAX_GRF 1024

char back[MAX_GRF], mouse[MAX_GRF], todraw[MAX_GRF] ;
int mb_x, mb_y, mb_x2, mb_y2 ;
int mg_x, mg_y, mg_x2, mg_y2 ;
int solapado ;

char pointer[16][16] ;

char pointers[3][16][16] =
{
   {
      { 2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
      { 2,1,2,0,0,0,0,0,0,0,0,0,0,0,0,0 },
      { 2,1,1,2,0,0,0,0,0,0,0,0,0,0,0,0 },
      { 2,1,1,1,2,0,0,0,0,0,0,0,0,0,0,0 },
      { 2,1,1,1,1,2,0,0,0,0,0,0,0,0,0,0 },
      { 2,1,1,1,1,1,2,0,0,0,0,0,0,0,0,0 },
      { 2,1,1,1,1,1,1,2,0,0,0,0,0,0,0,0 },
      { 2,1,1,1,1,2,2,0,0,0,0,0,0,0,0,0 },
      { 2,1,1,1,1,2,0,0,0,0,0,0,0,0,0,0 },
      { 2,1,2,2,1,1,2,0,0,0,0,0,0,0,0,0 },
      { 2,2,0,2,1,1,2,0,0,0,0,0,0,0,0,0 },
      { 0,0,0,0,2,1,1,2,0,0,0,0,0,0,0,0 },
      { 0,0,0,0,2,1,1,2,0,0,0,0,0,0,0,0 },
      { 0,0,0,0,0,2,2,0,0,0,0,0,0,0,0,0 },
      { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
      { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
   }
,
   {
      { 2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
      { 2,1,2,0,0,0,0,0,0,0,0,0,0,0,0,0 },
      { 2,1,2,0,0,0,0,0,0,0,0,0,0,0,0,0 },
      { 2,1,1,2,0,0,0,0,0,0,0,0,0,0,0,0 },
      { 2,1,1,2,0,0,0,0,0,0,0,0,0,0,0,0 },
      { 2,1,1,1,2,0,0,0,0,0,0,0,0,0,0,0 },
      { 2,1,1,1,2,0,0,0,0,0,0,0,0,0,0,0 },
      { 2,1,1,1,1,2,0,0,0,0,0,0,0,0,0,0 },
      { 2,1,1,1,2,2,0,0,0,0,0,0,0,0,0,0 },
      { 2,1,1,1,2,0,0,0,0,0,0,0,0,0,0,0 },
      { 2,1,2,1,2,0,0,0,0,0,0,0,0,0,0,0 },
      { 2,2,2,1,2,0,0,0,0,0,0,0,0,0,0,0 },
      { 0,0,2,1,1,2,0,0,0,0,0,0,0,0,0,0 },
      { 0,0,0,2,1,2,0,0,0,0,0,0,0,0,0,0 },
      { 0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0 },
      { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
   }
,
   {
      { 2,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0 },
      { 2,1,1,2,2,0,0,0,0,0,0,0,0,0,0,0 },
      { 2,1,1,1,1,2,2,0,0,0,0,0,0,0,0,0 },
      { 2,1,1,1,1,1,1,2,2,0,0,0,0,0,0,0 },
      { 2,1,1,1,1,1,1,1,1,2,2,0,0,0,0,0 },
      { 2,1,1,1,1,1,1,1,1,1,1,2,0,0,0,0 },
      { 2,1,1,1,1,1,1,1,2,2,0,0,0,0,0,0 },
      { 2,1,1,1,2,1,1,1,2,0,0,0,0,0,0,0 },
      { 2,2,2,2,0,2,2,1,1,2,0,0,0,0,0,0 },
      { 0,0,0,0,0,0,0,2,1,1,2,0,0,0,0,0 },
      { 0,0,0,0,0,0,0,0,2,2,0,0,0,0,0,0 },
      { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
      { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
      { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
      { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
      { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
   }
} ;

/* No s‚ si tendr s alg£n problema al compilar _loadds y _saveregs, en
   cualquier caso son necesarios para el handler */
void _loadds _saveregs rg_interrupt_handler ()
{
   if (!g_actuando) rg_actualizar() ;
}

/****************************************************************************
   RG_INICIALIZA: inicializa las rutinas de control de rat¢n.
     Salida:       1 si hay rat¢n conectado, 0 si no
   RG_DESCONECTA: desconecta las rutinas de control de rat¢n
****************************************************************************/
int rg_inicializa ()
{
   if (!g_raton)
   {
      asm mov ax, 0
      asm int 33h
      asm mov g_raton, ax

      asm mov ax, 15
      asm mov cx, 4
      asm mov dx, 4
      asm int 33h

      asm push es
      asm mov ax, seg rg_interrupt_handler
      asm mov dx, offset rg_interrupt_handler
      asm mov es, ax
      asm mov cx, 7
      asm mov ax, 12
      asm int 33h
      asm pop es
   }

   rg_setpointer (g_default_pointer) ;
   rg_rango (0, 0, g_max_x, g_max_y) ;

   g_autoraton = g_raton ;
   return g_raton ;
}

void rg_desconecta ()
{
   rg_desactivar() ;

   asm push es
   asm mov ax, seg rg_interrupt_handler
   asm mov dx, offset rg_interrupt_handler
   asm mov es, ax
   asm mov cx, 0
   asm mov ax, 12
   asm int 33h
   asm pop es

   g_autoraton = g_raton = 0 ;
}

/****************************************************************************
   RG_RANGO: establece el rango de coordenadas por donde puede moverse
      el rat¢n
   Entrada: x1,y1,x2,y2 - Coordenadas del rect ngulo
 ****************************************************************************/
void rg_rango (int x1, int y1, int x2, int y2)
   {
   int my, mx ;

   x1 *= g_raton_resx ;
   x2 *= g_raton_resx ;
   y1 *= g_raton_resy ;
   y2 *= g_raton_resy ;

   asm mov ax, 7
   asm mov cx, x1
   asm mov dx, x2
   asm int 33h
   asm mov ax, 8
   asm mov cx, y1
   asm mov dx, y2
   asm int 33h

   my = (y2-y1)/2 + y1;    /* Posiciona el rat¢n en mitad del rango */
   mx = (x2-x1)/2 + x1;

   asm mov dx, my
   asm mov cx, mx
   asm mov ax, 4
   asm int 33h
   }

/****************************************************************************
   RG_PON_PUNTERO: coloca el puntero en una nueva posici¢n de pantalla.
     Entrada:  'x', 'y' nuevas coordenadas del rat¢n
     Salida:       1 si rat¢n inicializado, 0 si no o no hay rat¢n
         conectado
****************************************************************************/
int rg_pon_puntero(int x, int y)
{
if (!g_raton) return 0 ;

x *= g_raton_resx ;
y *= g_raton_resy ;

/* coloca puntero en nueva posici¢n */
asm {
   mov ax,4
   mov cx,x
   mov dx,y
   int 33h
}

return(1);
}

/****************************************************************************
   RG_ESTADO: recoge la posici¢n actual del rat¢n y el estado de los
     botones.
     Entrada:      'r' puntero a estructura en la que se devolver 
	 la informaci¢n del rat¢n
     Salida:       1 si rat¢n inicializado, 0 si no o no hay rat¢n
	 conectado
****************************************************************************/
int rg_estado(STC_RATONG *r)
{
int bot, x, y, mvid;

r->boton1=0;
r->boton2=0;
r->xv=RG_NOVAL;
r->yv=RG_NOVAL;
r->x=RG_NOVAL;
r->y=RG_NOVAL;
r->fil=RG_NOVAL;
r->col=RG_NOVAL;

/* si rat¢n no est  inicializado, sale */
if(!g_raton) return(0);

asm {
   mov ax,3
   int 33h
   mov bot,bx
   mov x,cx
   mov y,dx
}

/* coordenadas virtuales */
r->xv=x;
r->yv=y;

/* coordenadas reales y fila y columna suponiendo caracteres de 8x16 en los */
/* modos de 16 colores y de 8x8 para el de 256 colores */
r->y=y/g_raton_resy;
r->x=x/g_raton_resx;
r->fil=r->y/g_chr_alto;
r->col=r->x/64;

/* comprueba estado de los botones */
if(bot & 0x0001) r->boton1=1;
if(bot & 0x0002) r->boton2=1;

return(1);
}

void rg_getback(void) ;
void rg_drawpointer(void) ;
void rg_erasepointer(void) ;
void rg_draw (int x, int y, char *b) ;

/****************************************************************************
   RG_SETPOINTER: cambia el aspecto del puntero
****************************************************************************/

void rg_setpointer (int n)
{
   memcpy ((char *)pointer, (char *)pointers[n], sizeof(pointer)) ;
}

/****************************************************************************
   RG_ACTIVAR:    activa el cursor del rat¢n y lo muestra en pantalla
   RG_DESACTIVAR: desactiva el cursor del rat¢n y lo oculta
   RG_PUNTERO:    se mantiene por campatibilidad
****************************************************************************/
void rg_activar()
{
   if (g_raton_activado) return ;

   rg_estado (&raton) ;
   old_raton = raton ;

   rg_getback () ;
   memcpy (mouse, back, MAX_GRF) ;
   mg_x = mb_x ;
   mg_y = mb_y ;
   mg_x2 = mb_x2 ;
   mg_y2 = mb_y2 ;
   rg_drawpointer () ;
   rg_draw (mg_x, mg_y, mouse) ;

   g_rg_x = mg_x-1 ;
   g_rg_y = mg_y-1 ;
   g_rg_x2 = mg_x2+1 ;
   g_rg_y2 = mg_y2+1 ;
   g_raton_activado = 1 ;
}

void rg_desactivar()
{
   if (!g_raton_activado) return ;
   g_raton_activado = 0 ;

   rg_estado (&raton) ;
   old_raton = raton ;

   rg_draw (mb_x, mb_y, back) ;
}

int rg_puntero (int n)
{
   if (n == RG_MUESTRA) rg_activar() ;
   if (n == RG_OCULTA) rg_desactivar() ;
   return g_raton ;
}

/****************************************************************************
   RG_ACTUALIZAR: actualiza la posici¢n del cursor en pantalla
   ****************************************************************************/
void rg_actualizar()
{
   if (!g_raton_activado) return ;

   old_raton = raton ;
   rg_estado (&raton) ;
   if (raton.x == old_raton.x && raton.y == old_raton.y) return ;

   memcpy (mouse, back, MAX_GRF) ;
   mg_x = mb_x ;
   mg_y = mb_y ;
   mg_x2 = mb_x2 ;
   mg_y2 = mb_y2 ;
   rg_getback () ;
   if (solapado)
      rg_erasepointer () ;
   else
      rg_draw (mg_x, mg_y, mouse) ;
   memcpy (mouse, back, MAX_GRF) ;
   mg_x = mb_x ;
   mg_y = mb_y ;
   mg_x2 = mb_x2 ;
   mg_y2 = mb_y2 ;
   rg_drawpointer () ;
   rg_draw (mg_x, mg_y, mouse) ;

   g_rg_x = min(mg_x, mb_x)-1 ;
   g_rg_y = min(mb_y, mg_y)-1 ;
   g_rg_x2 = max (mg_x2, mb_x2)+1 ;
   g_rg_y2 = max (mg_y2, mb_y2)+1 ;
}

/* rg_getback: recupera en el buffer indicado la parte de pantalla que
	       engloba tanto la posici¢n antigua del rat¢n como la nueva
	       si se solapan, o s¢lo la nueva en caso contrario */
void rg_getback()
{
   int o_g_autoraton = g_autoraton ;
   g_autoraton = 0 ;

   if (old_raton.x > raton.x+15 || old_raton.y > raton.y+15 ||
       old_raton.x+15 < raton.x || old_raton.y+15 < raton.y)
   {
      /* No se solapan */
      mb_x = raton.x ;
      mb_y = raton.y ;
      mb_x2 = raton.x+15 ;
      mb_y2 = raton.y+15 ;
      solapado = 0 ;
   }
   else
   {
      /* Se solapan */
      mb_x = min (raton.x, old_raton.x) ;
      mb_y = min (raton.y, old_raton.y) ;
      mb_x2 = max (raton.x+15, old_raton.x+15) ;
      mb_y2 = max (raton.y+15, old_raton.y+15) ;
      solapado = 1 ;
   }
   blq_coge (mb_x, mb_y, mb_x2, mb_y2, back) ;

   g_autoraton = o_g_autoraton ;
}

/* rg_drawpointer: dibuja el gr fico del rat¢n sobre el buffer "mouse"
		   seg£n el modo de memoria actual */
void rg_drawpointer()
{
   int jumpx, i, j, k, bfila, nfilas, bplane ;
   char *ptr, *optr ;
   char *graf = &pointer[0][0] ;
   int masc ;

   switch (g_memmodel)
   {
      case G_MM_256C:         /* Modos de 256 colores */
      case G_MM_256C_V:
      case G_MM_256C_X:
	 jumpx = (mg_x2-mg_x+1)-16 ;
	 ptr = mouse+4+(raton.x-mg_x)+(raton.y-mg_y)*(mg_x2-mg_x+1) ;
	 for (i = 0 ; i < 16 ; i++)
	 {
	    for (j = 0 ; j < 16 ; j++)
	    {
	       if (*graf == 1) *ptr = g_raton_color ;
	       else if (*graf == 2) *ptr = 0 ;
	       ptr++, graf++ ;
	    }
	    ptr += jumpx ;
	 }
	 break ;
      case G_MM_16C:         /* Modos de 16 colores */
	 bfila = *(int *)(mouse+2) ;
	 nfilas = *(int *)(mouse) ;
	 bplane = bfila*nfilas ;
	 for (k = 0 ; k < 4 ; k++)
	 {
	    ptr = mouse+5+k*bplane+(raton.x-mg_x)/8+(raton.y-mg_y)*bfila ;
	    graf = &pointer[0][0] ;
	    for (i = 0 ; i < 16 ; i++)
	    {
	       masc = (1 << (7-((raton.x-mg_x)&7))) ;
	       optr = ptr ;
	       for (j = 0 ; j < 16 ; j++)
	       {
		  if (*graf == 1) *ptr |= masc ;
		  else if (*graf == 2) *ptr &= ~masc ;
		  graf++ ;
		  if (!(masc >>= 1))
		  {
		     masc = 0x80 ;
		     ptr++ ;
		  }
	       }
	       ptr = optr+bfila ;
	    }
	 }
	 break ;
   }
}

/* rg_erasepointer: borra el puntero del rat¢n, suponiendo que en
		    old_raton.x(y) est n las coordenadas del lugar a
		    borrar, que el contenido est  en "mouse" y que
		    el lugar a borrar est  en "back" */
void rg_erasepointer()
{
   int jumpx_back, jumpx_mouse, bfila_back, bfila_mouse, i, j, k ;
   int masc_back, masc_mouse, planesize_back, planesize_mouse ;
   char *ptr_back, *ptr_mouse ;
   char *optr_back, *optr_mouse ;

   switch (g_memmodel)
   {
      case G_MM_256C:
      case G_MM_256C_V:
      case G_MM_256C_X:
	 jumpx_back = (mb_x2-mb_x+1) ;
	 jumpx_mouse = (mg_x2-mg_x+1) ;
	 ptr_back = back+4+(old_raton.x-mb_x) +
		    (old_raton.y-mb_y)*(mb_x2-mb_x+1) ;
	 ptr_mouse = mouse+4+(old_raton.x-mg_x) +
		     (old_raton.y-mg_y)*(mg_x2-mg_x+1) ;
	 for (i = 0 ; i < 16 ; i++)
	 {
	    memcpy (ptr_back, ptr_mouse, 16) ;
	    ptr_back += jumpx_back ;
	    ptr_mouse += jumpx_mouse ;
	 }
	 break ;
      case G_MM_16C:
	 bfila_back = *(int *)(back+2) ;
	 bfila_mouse = *(int *)(mouse+2) ;
	 planesize_back = bfila_back * *(int *)(back) ;
	 planesize_mouse = bfila_mouse * *(int *)(mouse) ;
	 for (k = 0 ; k < 4 ; k++)
	 {
	    ptr_back = back+5+k*planesize_back+
		       (old_raton.x-mb_x)/8+(old_raton.y-mb_y)*bfila_back ;
	    ptr_mouse = mouse+5+k*planesize_mouse+
			(old_raton.x-mg_x)/8+(old_raton.y-mg_y)*bfila_mouse ;
	    for (i = 0 ; i < 16 ; i++)
	    {
	       masc_mouse = (1 << (7-((old_raton.x-mg_x)&7))) ;
	       masc_back = (1 << (7-((old_raton.x-mb_x)&7))) ;
	       optr_mouse = ptr_mouse ;
	       optr_back = ptr_back ;
	       for (j = 0 ; j < 16 ; j++)
	       {
		  if ((*ptr_mouse)&masc_mouse)
		     (*ptr_back) |= masc_back ;
		  else
		     (*ptr_back) &= ~masc_back ;
		  if (!(masc_mouse >>= 1))
		  {
		     masc_mouse = 0x80 ;
		     ptr_mouse++ ;
		  }
		  if (!(masc_back >>= 1))
		  {
		     masc_back = 0x80 ;
		     ptr_back++ ;
		  }
	       }
	       ptr_mouse = optr_mouse+bfila_mouse ;
	       ptr_back = optr_back+bfila_back ;
	    }
	 }
	 break ;
   }
}

//char tmasc[8] = { 0xFF, 0x7F, 0x3F, 0x1F, 0x0F, 0x07, 0x03, 0x01 } ;
char tmasc[8] = { 0xFF, 0xFE, 0xFC, 0xF8, 0xF0, 0xE0, 0xC0, 0x80 } ;

/* rg_draw: dibuja uno de los buffers en la posici¢n dada, teniendo en
	    cuenta el "clipping" */
void rg_draw (int x, int y, char *b)
{
   int alto, ancho, i, j, bfila, bfila_todraw, bplane, bplane_todraw ;
   char *ptr, *iptr ;

   int o_g_autoraton = g_autoraton ;
   g_autoraton = 0 ;

   switch (g_memmodel)
   {
      case G_MM_256C:
      case G_MM_256C_X:
      case G_MM_256C_V:
	 ancho = min (*(int *)b, g_max_x-x+1) ;
	 alto = min (*(int *)(b+2), g_max_y-y+1) ;
	 if (ancho < 1 || alto < 1) return ;
	 if (ancho == (*(int *)b) && alto == *(int *)(b+2))
	    blq_pon (x, y, b) ;
	 else
	 {
	    *(int *)todraw = ancho ;
	    *(int *)(todraw+2) = alto ;
	    ptr = todraw+4 ;
	    iptr = b+4 ;
	    for (i = 0 ; i < alto ; i++)
	    {
	       memcpy (ptr, iptr, ancho) ;
	       ptr += ancho ;
	       iptr += *(int *)b ;
	    }
	    blq_pon (x, y, todraw) ;
	 }
	 break ;
      case G_MM_16C:
	 bfila = *(int *)(b+2) ;
	 bplane = bfila * (*(int *)b) ;
	 ancho = min (bfila*8, g_max_x-x+1) ;
	 alto = min (*(int *)b, g_max_y-y+1) ;
	 if (ancho < 1 || alto < 1) return ;
	 if (ancho == bfila*8 && alto == *(int *)b)
	    blq_pon (x, y, b) ;
	 else
	 {
	    *(int *)todraw = alto ;
	    bfila_todraw = ancho/8 ;
	    if (bfila_todraw*8 < ancho) bfila_todraw++ ;
	    *(int *)(todraw+2) = bfila_todraw ;
	    bplane_todraw = alto*bfila_todraw ;
	    todraw[4] = tmasc[(bfila_todraw*8-ancho)] ;
	    for (j = 0 ; j < 4 ; j++)
	    {
	       ptr = todraw+bplane_todraw*j+5 ;
	       iptr = b+bplane*j+5 ;
	       for (i = 0 ; i < alto ; i++)
	       {
		  memcpy (ptr, iptr, bfila_todraw) ;
		  ptr += bfila_todraw ;
		  iptr += bfila ;
	       }
	    }
	    blq_pon (x, y, todraw) ;
	 }
	 break ;
   }

   g_autoraton = o_g_autoraton ;
}