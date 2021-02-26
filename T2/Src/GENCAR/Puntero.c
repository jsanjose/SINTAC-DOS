/****************************************************************************
				  PUNTERO.C

	Conjunto de funciones para gestionar un puntero en los modos de
	gr ficos.

			    (c)1992 JSJ Soft Ltd.

	Las siguientes funciones son p£blicas:
		- inic_puntero: inicializa las variables internas y activa
		    el puntero
		- coge_pos_puntero: devuelve la posici¢n actual del puntero
		    y el estado de los botones
		- vis_puntero: muestra/esconde el puntero

	Las siguientes estructuras est n definidas en PUNTERO.H:
		STC_PUNTERO: para almacenar informaci¢n sobre el estado
		  del puntero
****************************************************************************/

#include <graph.h>
#include "puntero.h"

/* informaci¢n interna usada por varias funciones del rat¢n */
static struct STC_INFRAT {
	int rat_on, rat_inic;
	short xvirtual, yvirtual;
	short xmaxima, ymaxima;
	short xultim, yultim;
	unsigned bot_ultim, num_bot;
} rat={
	0, 0,
	0, 0,
	0, 0,
	0, 0,
	0, 0,
};

/****************************************************************************
	INIC_PUNTERO: inicializa las variables internas usadas por otras
	  funciones de control del puntero, esta funci¢n se llamar  siempre
	  al inicio del programa o cuando se cambie el modo de pantalla.
	  Salida:       0 si no hay rat¢n disponible o el n£mero de botones
			en otro caso
****************************************************************************/
int inic_puntero(void)
{
struct videoconfig vc;

/* coge la configuraci¢n actual de pantalla */
_getvideoconfig(&vc);

/* indica que el rat¢n ha sido inicializado */
rat.rat_inic=1;

_asm {
	sub ax,ax               ; funci¢n 0, resetear rat¢n
	int 33h
	mov rat.rat_on,ax       ; activa flag de existencia de rat¢n
	or ax,ax                ; si AX=0 no hay rat¢n conectado
	jnz existe
	mov bx,0
existe:
	mov rat.num_bot,bx      ; guarda n£mero de botones
}

rat.ymaxima=vc.numypixels-1;
rat.xmaxima=vc.numxpixels-1;

/* el rat¢n trabaja en una superficie virtual de 640x200 */
rat.xvirtual=639;
rat.yvirtual=vc.numypixels-1;

_asm {
	mov ax,8                ; coordenadas m¡nima y m xima verticales
	sub cx,cx               ; la m¡nima es 0
	mov dx,rat.yvirtual     ; la m xima es 8*filas
	int 33h

	mov ax,1                ; activa puntero del rat¢n
	int 33h

	mov ax,3                ; coge posici¢n inicial y estado de botones
	int 33h
	mov rat.xultim,cx       ; los guarda
	mov rat.yultim,dx
	mov rat.bot_ultim,bx
}

/* devuelve el n£mero de botones del rat¢n */
return(rat.num_bot);
}

/****************************************************************************
	COGE_POS_PUNTERO: devuelve la posici¢n y el estado de los botones
	  del puntero.
	  Entrada:      'puntero' puntero a estructura de informaci¢n
			de estado del puntero
	  Salida:       'puntero' actualizada
			0 si no hay rat¢n conectado o 1 en otro caso
****************************************************************************/
int coge_pos_puntero(STC_PUNTERO _far *puntero)
{
/* comprueba que el rat¢n existe y ha sido inicializado */
if(!rat.rat_inic) inic_puntero();
if(!rat.rat_on) return(0);

_asm {
	mov ax,3                ; coge informaci¢n sobre rat¢n y botones
	int 33h
	les di,puntero
	mov es:[di].x,cx
	mov es:[di].y,dx
	mov es:[di].botones,bx
}

puntero->x=(short)(((long)puntero->x*rat.xmaxima)/rat.xvirtual);
puntero->y=(short)(((long)puntero->y*rat.ymaxima)/rat.yvirtual);

return(1);
}

/****************************************************************************
	VIS_PUNTERO: selecciona si muestra o no el puntero.
	  Entrada:      'pv' visibilidad (ON=visible, OFF=invisible)
	  Salida:       0 si no hay rat¢n conectado o 1 en otro caso
****************************************************************************/
int vis_puntero(enum PTRVIS pv)
{
/* comprueba si el rat¢n existe y est  inicializado */
if(!rat.rat_inic) inic_puntero();
if(!rat.rat_on) return(0);

_asm {
	mov ax,pv               ; muestra o esconde puntero del rat¢n
	int 33h
}

return(1);
}

