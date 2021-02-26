/****************************************************************************
				   BOTON.C

	Conjunto de funciones para crear y gestionar a trav‚s de un puntero
	barras de botones en pantalla.

			    (c)1993 JSJ Soft Ltd.

	Las siguientes funciones son p£blicas:
		- dibuja_cuadro_bot¢n: dibuja un cuadro de bot¢n relleno o
		    s¢lo el marco
		- escribe_texto_boton: escribe texto en el interior de un
		    bot¢n
		- dibuja_bot¢n: dibuja un bot¢n en la pantalla
		- crea_barra_boton: crea una barra de botones horizontal o
		    vertical (pero no la dibuja), esta funci¢n se llamar 
		    SIEMPRE antes de usar una barra de botones para inicia-
		    lizar tanto las coordenadas de cada bot¢n como las teclas
		    de activaci¢n asociadas
		- dibuja_barra_boton: dibuja una barra de botones
		- pulsa_boton: comprueba si unas coordenadas dadas 'pulsan'
		    alg£n bot¢n
		- cambia_boton: cambia el modo (ON, OFF) y el tipo (ACT,
		    NO_ACT) de un bot¢n dado
		- esta_pulsado: comprueba si con el puntero se puls¢ en un
		    bot¢n y devuelve informaci¢n referente a ese bot¢n, as¡
		    mismo controla el efecto de pulsaci¢n en pantalla
		- lee_input: permite la entrada de una cadena de caracteres
		    por teclado

	Las siguientes estructuras est n definidas en BOTON.H:
		STC_BOTON: para la definici¢n de un bot¢n
		STC_BBOTON: para la definici¢n de una barra de botones
		STC_CODBOTON: usada para devolver informaci¢n de un
		  bot¢n pulsado
****************************************************************************/

#include <bios.h>
#include <ctype.h>
#include <string.h>
#include <graphics.h>
#include "puntero.h"
#include "chrimp.h"
#include "boton.h"

/****************************************************************************
	DIBUJA_CUADRO_BOTON: dibuja un cuadro de bot¢n en la pantalla.
	  Entrada:      'x0', 'y0' posici¢n de la esquina superior izquierda
			'ancho', 'alto' anchura y altura del cuadro en pixels
			'colorf' ¡ndice del color de fondo del cuadro
			'colors1', 'colors2' ¡ndices de color de sombra
			d‚bil y sombra fuerte del cuadro
			'modo' especifica si el cuadro de bot¢n est  activado
			(ON) o desactivado (OFF)
			'fondo' indica si se debe dibujar relleno (ON) o s¢lo
			el marco (OFF)
****************************************************************************/
void dibuja_cuadro_boton(short x0, short y0, short ancho, short alto,
  short colorf, short colors1, short colors2, int modo, int fondo)
{
short x1, y1, x2, y2, col1, col2;

vis_puntero(ESCONDE);

/* calcula coordenadas de esquina inferior derecha */
x2=x0+ancho-1;
y2=y0+alto-1;

if(fondo) {
	/* color de fondo del cuadro */
	setfillstyle(SOLID_FILL,colorf);
	/* dibuja fondo */
	bar(x0,y0,x2,y2);
}

/* si el cuadro est  activado selecciona los colores de sombras */
if(modo) {
	col1=colors1;
	col2=colors2;
}
/* si est  desactivado invierte los colores de sombras */
else {
	col1=colors2;
	col2=colors1;
}

/* color de fondo de sombra d‚bil */
setcolor(col1);
/* calcula coordenadas para primer nivel de la sombra d‚bil */
x1=x0+1;
y1=y0+1;
x2=x0+ancho-3;
y2=y0+alto-2;
/* dibuja primer nivel de la sombra d‚bil */
moveto(x1,y1);
lineto(x2,y1);
moveto(x1,y1);
lineto(x1,y2);
/* calcula coordenadas para segundo nivel de la sombra d‚bil */
x1=x0+2;
y1=y0+2;
x2=x0+ancho-4;
y2=y0+alto-3;
/* dibuja segundo nivel de la sombra d‚bil */
moveto(x1,y1);
lineto(x2,y1);
moveto(x1,y1);
lineto(x1,y2);

/* color de fondo de sombra fuerte */
setcolor(col2);
/* calcula coordenadas para primer nivel de la sombra fuerte */
x1=x0+ancho-2;
y1=y0+alto-2;
x2=x0+2;
y2=y0+1;
/* dibuja primer nivel de la sombra fuerte */
moveto(x1,y1);
lineto(x2,y1);
moveto(x1,y1);
lineto(x1,y2);
/* calcula coordenadas para segundo nivel de la sombra fuerte */
x1=x0+ancho-3;
y1=y0+alto-3;
x2=x0+3;
y2=y0+2;
/* dibuja segundo nivel de la sombra fuerte */
moveto(x1,y1);
lineto(x2,y1);
moveto(x1,y1);
lineto(x1,y2);

vis_puntero(MUESTRA);

}

/****************************************************************************
	ESCRIBE_TEXTO_BOTON: escribe texto del interior de un bot¢n.
	  Entrada:      'texto' puntero a texto a escribir
			'boton' puntero a estructura con datos del bot¢n
			'colorp', 'colort' color de primer plano y de tecla
			de activaci¢n
			'colorf' color del fondo
****************************************************************************/
void escribe_texto_boton(char *texto, STC_BOTON *boton, short colorp,
  short colort, short colorf)
{
short long_texto, x1, y1;

vis_puntero(ESCONDE);

/* calcula la longitud del texto a imprimir */
long_texto=strlen(texto)*8;

/* si tiene tecla de activaci¢n asociada, resta el ancho del car cter */
/* '&' que no se imprimir  */
if(boton->tecla) long_texto-=8;

/* calcula origen del texto para que salga centrado */
x1=boton->xb+((boton->ancho-long_texto)/2);
y1=boton->yb+((boton->alto-8)/2);

/* imprime el texto */
moveto(x1,y1);
while(*texto) {
	/* si encuentra '&' el siguiente car cter lo imprime de otro color */
	if(*texto=='&') {
		texto++;
		imp_chr(*texto,colorf,colort,CHR_NORM);
	}
	else imp_chr(*texto,colorf,colorp,CHR_NORM);

	/* siguiente car cter */
	texto++;
}

vis_puntero(MUESTRA);

}

/****************************************************************************
	DIBUJA_BOTON: dibuja un bot¢n en la pantalla.
	  Entrada:      'boton' puntero a estructura con datos del bot¢n
			'colorf' ¡ndice del color de fondo del bot¢n
			'colorp' ¡ndice del color de primer plano del bot¢n
			'colors1', 'colors2' ¡ndices de color de sombra
			'colort' color de tecla de activaci¢n
			d‚bil y sombra fuerte del bot¢n
	NOTA: preserva las coordenadas de texto y gr ficos as¡ como los
	  colores activos antes de llamar a la rutina
****************************************************************************/
void dibuja_boton(STC_BOTON *boton, short colorf, short colorp, short colors1,
  short colors2, short colort)
{
int color_graf, cx_graf, cy_graf, maxx, maxy;

vis_puntero(ESCONDE);

/* guarda colores y coordenadas */
color_graf=getcolor();
cx_graf=getx();
cy_graf=gety();
maxx=getmaxx();
maxy=getmaxy();

/* dibuja cuadro del bot¢n */
dibuja_cuadro_boton(boton->xb,boton->yb,boton->ancho,boton->alto,colorf,
  colors1,colors2,boton->modo,ON);

/* si no hay definido un icono imprime texto dentro del bot¢n */
/* si no, dibuja el icono */
if(!boton->icono)
  escribe_texto_boton(boton->texto_bot,boton,colorp,colort,colorf);
else {
	setcolor(colorp);
	/* selecciona zona de dibujo en el interior del bot¢n */
	setviewport(boton->xb+3,boton->yb+3,boton->xb+boton->ancho-3,
	  boton->yb+boton->alto-3,1);
	drawpoly(boton->nump_ico,boton->icono);
	/* restaura zona de dibujo a toda la pantalla */
	setviewport(0,0,maxx,maxy,1);
}

/* restaura colores y coordenadas */
setcolor(color_graf);
moveto(cx_graf,cy_graf);

vis_puntero(MUESTRA);

}

/****************************************************************************
	CREA_BARRA_BOTON: crea una barra de botones.
	  Entrada:      'bb' puntero a estructura con los datos de la barra
	  NOTA: se llama a esta funci¢n para calcular autom ticamente las
	    coordenadas y dimensiones de los botones para un tipo de barra
	    est ndar, si se desea otra disposici¢n habr  que rellenarlas a
	    mano y especificar como tipo de barra BBOTON_INDF, a esta funci¢n
	    habr  que llamarla siempre antes de usar una barra de botones
	    para inicializar los datos sobre teclas de activaci¢n
****************************************************************************/
void crea_barra_boton(STC_BBOTON *bb)
{
short xb, yb;
int i;
char *cht;

/* coordenadas de inicio de la barra */
xb=bb->x;
yb=bb->y;

/* calcula coordenadas y dimensiones de cada bot¢n de acuerdo */
/* al tipo de barra, si esta no es BBOTON_INDF */
if(bb->tipo!=BBOTON_INDF) {
	for(i=0; i<bb->num_botones; i++) {
		bb->bot[i].xb=xb;
		bb->bot[i].yb=yb;
		bb->bot[i].ancho=bb->ancho_bot;
		bb->bot[i].alto=bb->alto_bot;

		/* de acuerdo al tipo de barra dado actualiza las */
		/* coordenadas de inicio del siguiente bot¢n */
		switch(bb->tipo) {
			/* barra de botones horizontal */
			case BBOTON_HORZ :
				xb+=bb->ancho_bot-1;
				break;
			/* barra de botones vertical */
			case BBOTON_VERT :
				yb+=bb->alto_bot-1;
				break;
		}
	}
}

/* rellena datos sobre teclas de activaci¢n */
for(i=0; i<bb->num_botones; i++) {
	cht=bb->bot[i].texto_bot;
	/* busca '&' detr s del cual estar  la tecla de activaci¢n */
	while(*cht && (*cht!='&')) cht++;
	/* si no encontr¢ '&' no mete tecla de activaci¢n */
	if(!*cht) bb->bot[i].tecla=0;
	/* si encontr¢ '&' mete el siguiente car cter como tecla de */
	/* activaci¢n convirtiendo a may£scula */
	else bb->bot[i].tecla=toupper(*(cht+1));
}

}

/****************************************************************************
	DIBUJA_BARRA_BOTON: dibuja una barra de botones.
	  Entrada:      'bb' puntero a estructura con los datos de la barra
			(usar antes crea_barra_boton(...))
****************************************************************************/
void dibuja_barra_boton(STC_BBOTON *bb)
{
int i;

for(i=0; i<bb->num_botones; i++) dibuja_boton((bb->bot+i),bb->color_fondo,
  bb->color_pplano,bb->color_s1,bb->color_s2,bb->color_tecla);

}

/****************************************************************************
	PULSA_BOTON: comprueba si las coordenadas dadas 'pulsan' alg£n
	  bot¢n.
	  Entrada:      'x', 'y' coordenadas absolutas de pantalla
			'barras' puntero a matriz conteniendo datos sobre
			las barras de botones actualmente en pantalla,
			cada elemento son los datos de una barra
			'num_barr' n£mero de barras en la matriz
	  Salida:       bot¢n pulsado, barra a la que se corresponde y
			coordenadas relativas al origen del bot¢n d¢nde
			se produjo la pulsaci¢n en forma de estructura
			STC_CODBOTON, si no se puls¢ ning£ bot¢n devuelve
			{NO_BOTON, NO_BARRA, 0, 0}
	  NOTA: las coordenadas relativas se dan con respecto al interior
	    del marco del bot¢n
****************************************************************************/
STC_CODBOTON pulsa_boton(short x, short y, STC_BBOTON *barras,
  int num_barras)
{
short xb2, yb2;
STC_CODBOTON botpuls={NO_BOTON, NO_BARRA, 0, 0};
int i, j;

/* comprueba todas las barras */
for(i=0; i<num_barras; i++) {
	/* y todos los botones dentro de las barras */
	for(j=0; j<(barras+i)->num_botones; j++) {
		/* calcula coordenadas de esquina inferior izquierda */
		xb2=(barras+i)->bot[j].xb+(barras+i)->bot[j].ancho-1;
		yb2=(barras+i)->bot[j].yb+(barras+i)->bot[j].alto-1;
		/* si las coordenadas pasadas caen dentro de alg£n bot¢n */
		/* (dentro del marco) devuelve el n£mero de bot¢n y de */
		/* barra y coordenadas relativas de pulsaci¢n */
		if((x>(barras+i)->bot[j].xb+2) && (x<xb2-2) &&
		  (y>(barras+i)->bot[j].yb+2) && (y<yb2-2) &&
		  ((barras+i)->bot[j].tipo==ACT)) {
			botpuls.boton=j;
			botpuls.barra=i;
			/* coordenadas relativas de pulsaci¢n */
			/* respecto al interior del marco del bot¢n */
			botpuls.x=x-(barras+i)->bot[j].xb-3;
			botpuls.y=y-(barras+i)->bot[j].yb-3;
			return(botpuls);
		}
	}
}

return(botpuls);
}

/****************************************************************************
	CAMBIA_BOTON: cambia el estado de un bot¢n de una barra.
	  Entrada:      'bb' puntero a la barra que contiene el bot¢n
			'boton' n£mero de bot¢n dentro de la barra
			'modo' nuevo modo de dibujo (ON, OFF, IGUAL o INV)
			'tipo' nuevo tipo (ACT, NO_ACT, IGUAL, INV)
			NOTA: IGUAL deja el modo o tipo sin modificar, INV
			invierte el estado
****************************************************************************/
void cambia_boton(STC_BBOTON *bb, int boton, int modo, int tipo)
{
/* si quiere invertir modo, coloca en 'modo' el contrario del que ten¡a */
/* el bot¢n */
if(modo==INV) {
	if(bb->bot[boton].modo==ON) modo=OFF;
	else modo=ON;
}

/* si quiere invertir tipo, coloca en 'tipo' el contrario del que ten¡a */
/* el bot¢n */
if(tipo==INV) {
	if(bb->bot[boton].tipo==ACT) tipo=NO_ACT;
	else tipo=ACT;
}

if(modo!=IGUAL) bb->bot[boton].modo=modo;
if(tipo!=IGUAL) bb->bot[boton].tipo=tipo;
}

/****************************************************************************
	ESTA_PULSADO: comprueba si se pulsa con el puntero alguno de los
	  botones.
	  Entrada:      'barras' puntero a matriz conteniendo datos sobre
			las barras de botones actualmente en pantalla,
			cada elemento son los datos de una barra
			'num_barr' n£mero de barras en la matriz
	  Salida:       n£mero de bot¢n pulsado, n£mero de barra a la que
			pertenece y coordenadas relativas al origen del bot¢n
			del punto d¢nde se produjo la pulsaci¢n en forma de
			estructura STC_CODBOTON o
			{NO_BOTON, NO_BARRA, 0, 0} si no se puls¢ ninguno
****************************************************************************/
STC_CODBOTON esta_pulsado(STC_BBOTON *barras, int nbarr)
{
STC_CODBOTON p, pulsado={NO_BOTON, NO_BARRA, 0, 0};
STC_PUNTERO puntero;
unsigned tecla;
int i, j;

/* si no hay rat¢n entra en rutina de movimiento de cursor */
/* hasta que se pulsa una tecla que no sea de movimiento */
if(!hay_raton()) coge_pos_puntero(&puntero);

/* si hay alguna tecla en el buffer de teclado, coge el c¢digo ASCII y */
/* mira si es alguna tecla de activaci¢n */
tecla=bioskey(1);
if(tecla) {
	/* coge s¢lo c¢digo ASCII */
	tecla &= 0x00ff;
	/* pasa a may£scula */
	tecla=toupper(tecla);
	for(i=0; i<nbarr; i++) {
		for(j=0; j<(barras+i)->num_botones; j++) {
			if(tecla==(barras+i)->bot[j].tecla) {
				pulsado.boton=j;
				pulsado.barra=i;
				pulsado.x=0;
				pulsado.y=0;
				/* retira tecla del buffer */
				bioskey(0);
				return(pulsado);
			}
		}
	}
}
else {
	/* si hay rat¢n, recoge ahora la posici¢n del puntero */
	if(hay_raton()) coge_pos_puntero(&puntero);

	/* comprueba si el puntero est  sobre alg£n bot¢n */
	p=pulsa_boton(puntero.x,puntero.y,barras,nbarr);

	if((p.boton!=NO_BOTON) && (puntero.botones & PULSADO_IZQ)) {
		/* dibuja efecto pulsaci¢n bot¢n */
		/* este efecto lo har  s¢lo para botones cuyo modo sea ON */
		/* y si se puls¢ con el rat¢n (no con teclado) */
		if(hay_raton() && ((barras+p.barra)->bot[p.boton].modo==ON)) {
			/* invierte el modo del bot¢n (de ON a OFF) */
			cambia_boton(barras+p.barra,p.boton,INV,IGUAL);

			dibuja_cuadro_boton((barras+p.barra)->bot[p.boton].xb,
			  (barras+p.barra)->bot[p.boton].yb,
			  (barras+p.barra)->bot[p.boton].ancho,
			  (barras+p.barra)->bot[p.boton].alto,
			  (barras+p.barra)->color_fondo,
			  (barras+p.barra)->color_s1,
			  (barras+p.barra)->color_s2,
			  (barras+p.barra)->bot[p.boton].modo,OFF);

			/* espera hasta que se deje de presionar el puntero */
			do {
				coge_pos_puntero(&puntero);
			} while(puntero.botones & PULSADO_IZQ);

			/* dibuja efecto liberaci¢n bot¢n */
			/* invierte el modo del bot¢n (de OFF a ON) */
			cambia_boton(barras+p.barra,p.boton,INV,IGUAL);

			dibuja_cuadro_boton((barras+p.barra)->bot[p.boton].xb,
			  (barras+p.barra)->bot[p.boton].yb,
			  (barras+p.barra)->bot[p.boton].ancho,
			  (barras+p.barra)->bot[p.boton].alto,
			  (barras+p.barra)->color_fondo,
			  (barras+p.barra)->color_s1,
			  (barras+p.barra)->color_s2,
			  (barras+p.barra)->bot[p.boton].modo,OFF);
		}

		/* devuelve datos sobre bot¢n pulsado */
		pulsado=p;
	}
}

/* retira tecla del buffer */
if(tecla) bioskey(0);

return(pulsado);
}

/****************************************************************************
	LEE_INPUT: lee una cadena de caracteres por teclado o edita una
	  cadena ya existente.
	  Entrada:      'x', 'y' coordenadas de pantalla d¢nde se imprimir 
			la cadena tecleada
			'cadena' puntero a buffer d¢nde se guardar 
			la cadena tecleada; si el primer car cter no es '\0'
			se ajustar  la rutina para poder editar la cadena
			'longitud' longitud de la cadena (sin contar el
			\0 final)
			'ancho' anchura de la zona de entrada en pixels
			'colorf', 'color' color de fondo y primer plano con
			el que se imprimir n los caracteres tecleados
	  Salida:       n£mero de caracteres tecleados
****************************************************************************/
int lee_input(short x, short y, char *cadena, int longitud, int ancho,
  short colorf, short color)
{
char *cur, *fin, *ptr;
int num_car=0;
unsigned tecla;
short xcur, i, borr=0;

/* oculta el puntero durante la entrada de datos */
vis_puntero(ESCONDE);

/* posiciona en origen */
moveto(x,y);

/* calcula anchura de zona de entrada en caracteres */
ancho /= 8;

/* inicializa punteros */
cur=cadena;
/* busca final de la cadena y n£mero de caracteres */
for(fin=cadena; *fin; fin++, num_car++);

/* coordenada relativa del cursor respecto origen de caja (en caracteres) */
xcur=0;

/* recoge caracteres hasta que pulse RETURN */
do {
	/* si hay que borrar la caja */
	if(borr) {
		for(i=0; i<ancho; i++) {
			moveto(x+(i*8),y);
			imp_chr(' ',colorf,color,CHR_NORM);
		}
		borr=0;
	}

	/* imprime l¡nea */
	moveto(x,y);

	/* si el trozo a imprimir es menor que el ancho de la caja */
	/* lo imprime hasta el final */
	if((cur-xcur+ancho)>fin) imp_cad(cur-xcur,colorf,color,CHR_NORM);
	else impn_cad(cur-xcur,ancho,colorf,color,CHR_NORM);

	/* imprime cursor */
	/* si cursor al final de caja y al final de l¡nea */
	/* borra £ltimo car cter por si la l¡nea, anteriormente, */
	/* ocupaba toda la caja */
	if((cur==fin) && (xcur==(ancho-1))) {
		moveto(x+(xcur*8),y);
		imp_chr(BOT_CCUR,colorf,color,CHR_NORM);
	}
	else {
		moveto(x+(xcur*8),y);
		imp_chr(BOT_CCUR,0,colorf,CHR_XOR);
	}

	/* recoge tecla */
	tecla=bioskey(0);

	/* borra cursor */
	moveto(x+(xcur*8),y);
	imp_chr(BOT_CCUR,0,colorf,CHR_XOR);

	/* comprueba si es una tecla de movimiento de cursor */
	switch(tecla >> 8) {
		case 0x4b :             /* cursor izquierda */
			if(cur>cadena) {
				cur--;
				if(xcur>0) xcur--;
			}
			break;
		case 0x4d :             /* cursor derecha */
			if(cur<fin) {
				cur++;
				if(xcur<(ancho-1)) xcur++;
			}
			break;
		case 0x47 :             /* cursor origen */
			cur=cadena;
			xcur=0;
			break;
		case 0x4f :             /* cursor fin */
			cur=fin;
			if(num_car<ancho) xcur=num_car;
			else xcur=ancho-1;
			break;
		case 0x0e :             /* borrado hacia atr s */
			if(cur>cadena) {
				cur--;
				fin--;
				for(ptr=cur; ptr<=fin; ptr++) *ptr=*(ptr+1);
				num_car--;
				if(xcur>0) xcur--;
			}
			/* indica borrado de car cter */
			borr=1;
			break;
		case 0x53 :             /* borrado */
			if(cur<fin) {
				for(ptr=cur; ptr<fin; ptr++) *ptr=*(ptr+1);
				fin--;
				num_car--;
			}
			/* indica borrado de car cter */
			borr=1;
	}

	/* recoge c¢digo ASCII, y desprecia c¢digo de 'scan' */
	tecla &= 0x00ff;

	/* si es un car cter v lido y no se ha alcanzado el */
	/* n£mero m ximo de caracteres permitidos */
	if((tecla>31) && (num_car<longitud)) {
		/* si cursor est  en zona intermedia de l¡nea */
		/* inserta car cter desplazando el resto */
		if(cur!=fin) {
			/* desplaza caracteres */
			for(ptr=fin; ptr>=cur; ptr--) *(ptr+1)=*ptr;
		}
		/* inserta car cter tecleado */
		*cur++=(char)tecla;
		fin++;
		/* inserta fin de l¡nea */
		*fin='\0';
		/* incrementa n£mero de caracteres tecleados */
		num_car++;
		/* incrementa posici¢n del cursor */
		if(xcur<(ancho-1)) xcur++;
	}

} while(tecla!=0x0d);           /* hasta que se pulse RETURN */

/* imprime l¡nea */
moveto(x,y);
cur=cadena;
xcur=0;
if((cur-xcur+ancho)>fin) imp_cad(cur-xcur,colorf,color,CHR_NORM);
else impn_cad(cur-xcur,ancho,colorf,color,CHR_NORM);

/* muestra el puntero */
vis_puntero(MUESTRA);

return(num_car);
}
