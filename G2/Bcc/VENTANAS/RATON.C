/****************************************************************************
				  RATON.C

	Biblioteca de funciones para gestionar el rat¢n.

			     (c)1993 JSJ Soft Ltd.

	Las siguientes funciones son p£blicas:
		- r_inicializa: inicializa las rutinas de control del rat¢n
		- r_puntero: muestra u oculta el puntero del rat¢n
		- r_estado: devuelve el estado actual del rat¢n

	Las siguientes estructuras est n definidas en RATON.H
		STC_RATON: informaci¢n sobre el estado del rat¢n
****************************************************************************/

#include "raton.h"

/* Variables globales internas */
static struct {
	int inicializado;       /* 0 no inicializado, 1 inicializado */
	int existe;             /* 0 no hay rat¢n, 1 hay rat¢n */
} raton={
	0,
	0
};

/****************************************************************************
	R_INICIALIZA: inicializa las rutinas de control de rat¢n.
	  Salida:       1 si hay rat¢n conectado, 0 si no
****************************************************************************/
int r_inicializa(void)
{

raton.inicializado=1;

/* inicializa rutinas de control */
asm {
	mov ax,0        // inicializar 'driver' de rat¢n
	int 33h
	or ax,ax        // si AX = 0, no hay rat¢n
	jz no_existe
	mov ax,1
}
no_existe:
asm {
	mov word ptr raton.existe,ax
}

/* si hay rat¢n, visualiza el puntero */
if(raton.existe) {
	asm {
		mov ax,1
		int 33h
	}
}

return(raton.existe);
}

/****************************************************************************
	R_PUNTERO: muestra u oculta el puntero del rat¢n.
	  Entrada:      'modo' puede ser R_MUESTRA para mostrar el puntero
			o R_OCULTA para ocultarlo
	  Salida:       1 si rat¢n inicializado, 0 si no o no hay rat¢n
			conectado
****************************************************************************/
int r_puntero(int modo)
{

/* si rat¢n no est  inicializado o no est  conectado, sale */
if(!raton.inicializado || !raton.existe) return(0);

asm {
	mov ax,modo
	int 33h
}

return(1);
}

/****************************************************************************
	R_ESTADO: recoge la posici¢n actual del rat¢n y el estado de los
	  botones.
	  Entrada:      'r' puntero a estructura en la que se devolver 
			la informaci¢n del rat¢n
	  Salida:       1 si rat¢n inicializado, 0 si no o no hay rat¢n
			conectado
****************************************************************************/
int r_estado(STC_RATON *r)
{
short bot, x, y;

r->boton1=0;
r->boton2=0;
r->xv=0;
r->yv=0;
r->fil=0;
r->col=0;

/* si rat¢n no est  inicializado o no est  conectado, sale */
if(!raton.inicializado || !raton.existe) return(0);

/* recoge estado de los botones y posici¢n del rat¢n */
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

/* comprueba estado de los botones */
if(bot & 0x0001) r->boton1=1;
else r->boton1=0;
if(bot & 0x0002) r->boton2=1;
else r->boton2=0;

/* calcula fila, columna suponiendo una pantalla virtual de 640x200 */
/* y que est  en un modo de texto de 80x25 */
r->col=x/8;
r->fil=y/8;

return(1);
}
