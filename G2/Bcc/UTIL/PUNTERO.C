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
		- hay_raton: indica si hay conectado o no un rat¢n

	Las siguientes estructuras est n definidas en PUNTERO.H:
		STC_PUNTERO: para almacenar informaci¢n sobre el estado
		  del puntero
****************************************************************************/

#include <bios.h>
#include <graphics.h>
#include "puntero.h"

/* informaci¢n interna usada por varias funciones del rat¢n */
static struct {
	int rat_on, rat_inic;
	short xvirtual, yvirtual;
	short xmaxima, ymaxima;
	short xultim, yultim;
	unsigned bot_ultim, num_bot;
	int paso;
	unsigned dir_mov;
} rat={
	0, 0,
	0, 0,
	0, 0,
	0, 0,
	0, 0,
	VEL_INIC,
	0,
};

/*** Prototipos de funciones internas ***/
static void dibuja_puntero(short x, short y);

/****************************************************************************
	DIBUJA_PUNTERO: dibuja el puntero cuando se usa teclado.
	  Entrada:      'x', 'y' coordenadas del puntero
****************************************************************************/
void dibuja_puntero(short x, short y)
{
int color, antx, anty;

/* guarda color y coordenadas */
color=getcolor();
antx=getx();
anty=gety();
setcolor(15);

setwritemode(1);
line(x,y,x+6,y+5);
line(x,y,x,y+7);
line(x,y,x+5,y+10);
setwritemode(0);

/* restaura color y coordenadas */
setcolor(color);
moveto(antx,anty);

}

/****************************************************************************
	INIC_PUNTERO: inicializa las variables internas usadas por otras
	  funciones de control del puntero, esta funci¢n se llamar  siempre
	  al inicio del programa o cuando se cambie el modo de pantalla.
	  Salida:       0 si no hay rat¢n conectado o el n£mero de botones
			en otro caso
****************************************************************************/
int inic_puntero(void)
{

/* indica que el rat¢n ha sido inicializado */
rat.rat_inic=1;

asm {
	sub ax,ax               	// funci¢n 0, resetear rat¢n
	int 33h
	mov word ptr rat.rat_on,ax
	or ax,ax                	// si AX=0 no hay rat¢n conectado
	jnz existe
	mov bx,0
}
existe:
asm {
	mov word ptr rat.num_bot,bx 	// guarda n£mero de botones
}

rat.ymaxima=getmaxy();
rat.xmaxima=getmaxx();

/* si no detect¢ rat¢n, inicializa coordenadas de puntero y otras variables */
if(!rat.num_bot) {
	rat.xultim=rat.xmaxima/2;
	rat.yultim=rat.ymaxima/2;
	rat.paso=VEL_INIC;
	rat.dir_mov=0;
	return(0);
}

/* el rat¢n trabaja en una superficie virtual de 640x200 */
rat.xvirtual=639;
rat.yvirtual=getmaxy();

asm {
	mov ax,8                	// coordenadas verticales
	sub cx,cx               	// la m¡nima es 0
	mov dx,word ptr rat.yvirtual	// la m xima es 8*filas
	int 33h

	mov ax,1                	// activa puntero del rat¢n
	int 33h

	mov ax,3                	// coge posici¢n inicial
	int 33h				// y estado de botones
	mov word ptr rat.xultim,cx
	mov word ptr rat.yultim,dx
	mov word ptr rat.bot_ultim,bx
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
			0 si no hay rat¢n conectado o el n£mero de botones
			en otro caso
****************************************************************************/
int coge_pos_puntero(STC_PUNTERO *puntero)
{
unsigned tecla;
short xp, yp;

/* comprueba que el rat¢n existe y ha sido inicializado */
if(!rat.rat_inic) inic_puntero();

/* si no hay rat¢n conectado, mueve con teclas de cursor */
if(!rat.rat_on) {
	dibuja_puntero(rat.xultim,rat.yultim);
	while(1) {
		/* coge tecla, pero no la saca del buffer */
		do {
			tecla=bioskey(1);
		} while(!tecla);

		/* recoge s¢lo c¢digo scan */
		tecla >>= 8;

		/* coge coordenadas actuales del puntero */
		xp=rat.xultim;
		yp=rat.yultim;

		switch(tecla) {
			case CUR_IZQ :
				xp-=(rat.paso/MULT_VEL);
				break;
			case CUR_DER :
				xp+=(rat.paso/MULT_VEL);
				break;
			case CUR_ARR :
				yp-=(rat.paso/MULT_VEL);
				break;
			case CUR_ABJ :
				yp+=(rat.paso/MULT_VEL);
				break;
			case TEC_ESP :
				puntero->x=rat.xultim;
				puntero->y=rat.yultim;
				puntero->botones=PULSADO_IZQ;
				/* saca tecla del buffer */
				bioskey(0);
			default :
				dibuja_puntero(rat.xultim,rat.yultim);
				rat.paso=VEL_INIC;
				return(0);
		}

		/* si se movi¢ el puntero */
		if((xp!=rat.xultim) || (yp!=rat.yultim)) {
			/* comprueba coordenadas */
			if(xp<0) xp=0;
			else if(xp>rat.xmaxima) xp=rat.xmaxima;
			if(yp<0) yp=0;
			else if(yp>rat.ymaxima) yp=rat.ymaxima;

			/* borra puntero de su antigua posici¢n */
			dibuja_puntero(rat.xultim,rat.yultim);
			/* lo imprime en la nueva */
			dibuja_puntero(xp,yp);
			/* actualiza coordenadas */
			rat.xultim=xp;
			rat.yultim=yp;
			/* saca tecla del buffer */
			bioskey(0);

			/* aumenta velocidad de puntero si es que se est  */
			/* moviendo en la misma direcci¢n */
			if(rat.dir_mov!=tecla) {
				rat.paso=VEL_INIC;
				rat.dir_mov=tecla;
			}
			else if(rat.paso<MAX_VEL) rat.paso++;
		}
	}
}

asm {
	mov ax,3                // coge informaci¢n sobre rat¢n y botones
	int 33h
	les di,puntero
	mov es:[di].(STC_PUNTERO)x,cx
	mov es:[di].(STC_PUNTERO)y,dx
	mov es:[di].(STC_PUNTERO)botones,bx
}

puntero->x=(short)(((long)puntero->x*rat.xmaxima)/rat.xvirtual);
puntero->y=(short)(((long)puntero->y*rat.ymaxima)/rat.yvirtual);

/* devuelve el n£mero de botones del rat¢n */
return(rat.num_bot);
}

/****************************************************************************
	VIS_PUNTERO: selecciona si muestra o no el puntero.
	  Entrada:      'pv' visibilidad (ON=visible, OFF=invisible)
	  Salida:       0 si no hay rat¢n conectado o el n£mero de botones
			en otro caso
****************************************************************************/
int vis_puntero(enum PTRVIS pv)
{

/* comprueba si el rat¢n existe y est  inicializado */
if(!rat.rat_inic) inic_puntero();
if(!rat.rat_on) return(0);

asm {
	mov ax,pv               // muestra o esconde puntero del rat¢n
	int 33h
}

/* devuelve el n£mero de botones del rat¢n */
return(rat.num_bot);
}

/****************************************************************************
	HAY_RATON: indica si hay o no conectado un rat¢n.
	  Salida:       0 si no hay conectado un rat¢n o el n£mero de
			botones en otro caso
****************************************************************************/
int hay_raton(void)
{

if(!rat.rat_on) return(0);
else return(rat.num_bot);

}
