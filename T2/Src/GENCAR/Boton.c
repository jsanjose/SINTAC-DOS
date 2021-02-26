/****************************************************************************
				   BOTON.C

	Conjunto de funciones para crear y gestionar a trav‚s de un puntero
	barras de botones en pantalla.

			    (c)1992 JSJ Soft Ltd.

	Las siguientes funciones son p£blicas:
		- dibuja_cuadro_bot¢n: dibuja un cuadro de bot¢n relleno o
		    s¢lo el marco
		- dibuja_bot¢n: dibuja un bot¢n en la pantalla
		- escribe_texto_boton: escribe texto en el interior de un
		  bot¢n
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

	NOTA IMPORTANTE: es necesario que la fuente Helv‚tica (HELVB.FON)
	  est‚ cargada (ver funci¢n _registerfonts()), si no es as¡ los
	  textos de los botones no aparecer n.
****************************************************************************/

#include <graph.h>
#include <bios.h>
#include <ctype.h>
#include "puntero.h"
#include "boton.h"

/* constantes usadas por lee_tecla() */
enum {NO_ESPERA, ESPERA, LIMPIA};

/*****  Prototipos de funciones internas  *****/
static unsigned lee_tecla(int control);

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

/* calcula coordenadas de esquina inferior derecha */
x2=x0+ancho-1;
y2=y0+alto-1;

if(fondo) {
	/* color de fondo del cuadro */
	_setcolor(colorf);
	/* dibuja fondo */
	_rectangle(_GFILLINTERIOR,x0,y0,x2,y2);
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
_setcolor(col1);
/* calcula coordenadas para primer nivel de la sombra d‚bil */
x1=x0+1;
y1=y0+1;
x2=x0+ancho-3;
y2=y0+alto-2;
/* dibuja primer nivel de la sombra d‚bil */
_moveto(x1,y1);
_lineto(x2,y1);
_moveto(x1,y1);
_lineto(x1,y2);
/* calcula coordenadas para segundo nivel de la sombra d‚bil */
x1=x0+2;
y1=y0+2;
x2=x0+ancho-4;
y2=y0+alto-3;
/* dibuja segundo nivel de la sombra d‚bil */
_moveto(x1,y1);
_lineto(x2,y1);
_moveto(x1,y1);
_lineto(x1,y2);

/* color de fondo de sombra fuerte */
_setcolor(col2);
/* calcula coordenadas para primer nivel de la sombra fuerte */
x1=x0+ancho-2;
y1=y0+alto-2;
x2=x0+2;
y2=y0+1;
/* dibuja primer nivel de la sombra fuerte */
_moveto(x1,y1);
_lineto(x2,y1);
_moveto(x1,y1);
_lineto(x1,y2);
/* calcula coordenadas para segundo nivel de la sombra fuerte */
x1=x0+ancho-3;
y1=y0+alto-3;
x2=x0+3;
y2=y0+3;
/* dibuja segundo nivel de la sombra fuerte */
_moveto(x1,y1);
_lineto(x2,y1);
_moveto(x1,y1);
_lineto(x1,y2);

}

/****************************************************************************
	ESCRIBE_TEXTO_BOTON: escribe texto del interior de un bot¢n.
	  Entrada:      'texto' puntero a texto a escribir
			'boton' puntero a estructura con datos del bot¢n
			'colorp', 'colort' color de primer plano y de tecla
			de activaci¢n
****************************************************************************/
void escribe_texto_boton(char *texto, STC_BOTON *boton, short colorp,
  short colort)
{
short alt_texto, anch_texto, long_texto, x1, y1;
char *cht, ctxt[2]=" ";

/* se supone que la fuente Helv‚tica (HELVB.FON) est  cargada */
/* si no lo est  o se produce alg£n error sale sin imprimir texto */
/* elegimos tama¤o de acuerdo a 'tam_texto' */
/* si 'tam_texto' no es v lido elige tama¤o mediano */
switch(boton->tam_texto) {
	case TAMTXT_PEQ :
		if(_setfont("t'helv'h10")<0) return;
		alt_texto=10;
		anch_texto=5;
		break;
	case TAMTXT_MED :
		if(_setfont("t'helv'h12")<0) return;
		alt_texto=12;
		anch_texto=7;
		break;
	case TAMTXT_GRA :
		if(_setfont("t'helv'h15")<0) return;
		alt_texto=15;
		anch_texto=8;
		break;
	default :
		if(_setfont("t'helv'h12")<0) return;
		alt_texto=12;
}

/* calcula la longitud del texto a imprimir */
long_texto=_getgtextextent(texto);

/* si tiene tecla de activaci¢n asociada, resta el ancho del car cter */
/* '&' que no se imprimir  */
if(boton->tecla) long_texto-=anch_texto;

/* calcula origen del texto para que salga centrado */
x1=boton->xb+((boton->ancho-long_texto)/2);
y1=boton->yb+((boton->alto-alt_texto)/2);

/* imprime el texto */
_moveto(x1,y1);
_setcolor(colorp);
cht=texto;

while(*cht) {
	/* si encuentra '&' el siguiente car cter lo imprime de otro color */
	if(*cht=='&') {
		_setcolor(colort);
		cht++;
		ctxt[0]=*cht++;
		_outgtext(ctxt);
		_setcolor(colorp);
	}
	else{
		ctxt[0]=*cht++;
		_outgtext(ctxt);
	}
}

}

/****************************************************************************
	DIBUJA_BOTON: dibuja un bot¢n en la pantalla.
	  Entrada:      'boton' puntero a estructura con datos del bot¢n
			'colorf' ¡ndice del color de fondo del bot¢n
			'colorp' ¡ndice del color de primer plano del bot¢n
			'colors1', 'colors2' ¡ndices de color de sombra
			d‚bil y sombra fuerte del bot¢n
	NOTA: preserva las coordenadas de texto y gr ficos as¡ como los
	 colores activos antes de llamar a la rutina
****************************************************************************/
void dibuja_boton(STC_BOTON *boton, short colorf, short colorp, short colors1,
  short colors2)
{
short color_graf;               /* color para gr ficos */
struct xycoord coord_graf;      /* coordenadas gr ficos */

/* guarda colores y coordenadas */
color_graf=_getcolor();
coord_graf=_getcurrentposition();

/* dibuja cuadro del bot¢n */
dibuja_cuadro_boton(boton->xb,boton->yb,boton->ancho,boton->alto,colorf,
  colors1,colors2,boton->modo,ON);

/* imprime texto dentro del bot¢n */
escribe_texto_boton(boton->texto_bot,boton,colorp,colors1);

/* restaura colores y coordenadas */
_setcolor(color_graf);
_moveto(coord_graf.xcoord,coord_graf.ycoord);

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
char *cht, c;

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
	if(!cht) bb->bot[i].tecla=0;
	/* si encontr¢ '&' mete el siguiente car cter como tecla de */
	/* activaci¢n conviertiendo a may£scula */
	else bb->bot[i].tecla=toupper(*(cht+1));
}

}

/****************************************************************************
	DIBUJA_BARRA_BOTON: dibuja una barra de botones.
	  Entrada:      'bb' puntero a estructura con los datos de la barra
			(usar antes CREA_BARRA_BOTON)
****************************************************************************/
void dibuja_barra_boton(STC_BBOTON *bb)
{
int i;

for(i=0; i<bb->num_botones; i++) dibuja_boton((bb->bot+i),bb->color_fondo,
  bb->color_pplano,bb->color_s1,bb->color_s2);

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
			{99, 99, 0, 0}
	  NOTA: las coordenadas relativas se dan con respecto al interior
	    del marco del bot¢n
****************************************************************************/
STC_CODBOTON pulsa_boton(short x, short y, STC_BBOTON *barras,
  int num_barras)
{
short xb2, yb2;
STC_CODBOTON botpuls={99, 99, 0, 0};
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
			estructura STC_CODBOTON o {99, 99, 0, 0} si no se
			puls¢ ninguno
****************************************************************************/
STC_CODBOTON esta_pulsado(STC_BBOTON *barras, int nbarr)
{
STC_CODBOTON p, pulsado={99, 99, 0, 0};
STC_PUNTERO puntero;
short ancho, alto;
unsigned tecla;
int i, j;

/* si hay alguna tecla en el buffer de teclado mira si es alguna */
/* tecla de activaci¢n */
if((tecla=lee_tecla(NO_ESPERA))!=0) {
	tecla=toupper(tecla);
	for(i=0; i<nbarr; i++) {
		for(j=0; j<(barras+i)->num_botones; j++) {
			if(tecla==(barras+i)->bot[j].tecla) {
				pulsado.boton=j;
				pulsado.barra=i;
				pulsado.x=0;
				pulsado.y=0;
				return(pulsado);
			}
		}
	}
}
else {
	/* recoge la posici¢n del puntero */
	coge_pos_puntero(&puntero);
	/* comprueba si el puntero est  sobre alg£n bot¢n */
	p=pulsa_boton(puntero.x,puntero.y,barras,nbarr);

	if((p.boton!=99) && (puntero.botones & PULSADO_IZQ)) {
		/* dibuja efecto pulsaci¢n bot¢n */
		/* este efecto lo har  s¢lo para botones cuyo modo sea ON */
		if((barras+p.barra)->bot[p.boton].modo==ON) {
			/* esconde puntero para que no interfiera */
			vis_puntero(ESCONDE);

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

			/* muestra el puntero */
			vis_puntero(MUESTRA);

			/* espera hasta que se deje de presionar el puntero */
			do {
				coge_pos_puntero(&puntero);
			} while(puntero.botones & PULSADO_IZQ);

			/* dibuja efecto liberaci¢n bot¢n */
			/* esconde puntero para que no interfiera */
			vis_puntero(ESCONDE);

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

			/* muestra el puntero */
			vis_puntero(MUESTRA);
		}

		/* devuelve datos sobre bot¢n pulsado */
		pulsado=p;
	}
}

return(pulsado);
}

/****************************************************************************
	LEE_TECLA: devuelve informaci¢n sobre las teclas pulsadas.
	  Entrada:      'control' c¢digo que indica c¢me debe actuar la
			rutina:
			  NO_ESPERA devuelve 0 si no hay tecla en el buffer,
			  si no devuelve la tecla
			  ESPERA espera hasta que haya una tecla disponible
			  en el buffer y la devuelve
			  LIMPIA limpia el buffer y espera hasta que haya
			  una tecla disponible
	  Salida:
		Tipo de tecla                          Byte alto   Byte bajo
		-------------                          ---------   ---------
		Tecla no disponible (con NO_ESPERA)        0           0
		valor ASCII                                0         ASCII
	  NOTA: un valor ASCII igual a 0 indica tecla de funci¢n, flechas de
	    cursor,...
****************************************************************************/
unsigned lee_tecla(int control)
{
unsigned tecla;

/* si LIMPIA vac¡a el buffer de teclado */
if(control==LIMPIA) {
	while(_bios_keybrd(_KEYBRD_READY)) _bios_keybrd(_KEYBRD_READ);
}

/* si NO_ESPERA devuelve 0 si no hay tecla en buffer */
if((control==NO_ESPERA) && !_bios_keybrd(_KEYBRD_READY)) return(0);

/* coge c¢digo de la tecla */
tecla=_bios_keybrd(_KEYBRD_READ);

/* aisla c¢digo ASCII */
tecla &= 0x00ff;

return(tecla);
}

/****************************************************************************
	LEE_INPUT: lee una cadena de caracteres por teclado.
	  Entrada:      'x', 'y' coordenadas de pantalla d¢nde se imprimir 
			la cadena tecleada
			'cadena' puntero a buffer d¢nde se guardar 
			la cadena tecleada
			'longitud' longitud de la cadena (sin contar el
			\0 final)
			'color' color con el que se imprimir n los caracteres
			tecleados
			'colorf' color del fondo d¢nde se imprimir n los
			caracteres
	  Salida:       n£mero de caracteres tecleados + 1; 0 indica
			que hubo un error al seleccionar la fuente Helv‚tica
	  NOTA: la m xima longitud de la cadena debe ser menor que
	    LONG_MAX_INPUT
****************************************************************************/
int lee_input(short x, short y, char *cadena, int longitud, short color,
  short colorf)
{
char *pcad, ncar[2]=" ";
int lng_inp=0, i;
unsigned tecla;
struct xycoord antpos;
short antcolor, x0, coord[LONG_MAX_INPUT];

pcad=cadena;
/* inicializa buffer */
*pcad='\0';

/* elige Helv‚tica de 12 puntos de altura */
/* si la fuente no est  cargada sale indicando error */
if(_setfont("t'helv'h12")<0) return(0);

/* posiciona en origen y selecciona color guardando los antiguos */
antpos=_moveto(x,y);
antcolor=_setcolor(color);

x0=x;

while(1) {
	/* imprime cursor */
	_moveto(x0,y);
	_lineto(x0,y+12);

	/* espera hasta que se pulse una tecla cuyo valor ASCII */
	/* sea 32 o mayor o sea 13 (RETURN) o 8 (BACKSPACE) */
	do {
		tecla=lee_tecla(ESPERA);
	} while((tecla<32) && (tecla!=8) && (tecla!=13));

	/* borra cursor */
	_setcolor(colorf);
	_moveto(x0,y);
	_lineto(x0,y+12);
	_setcolor(color);

	switch(tecla & 0x00ff) {
		case 13 :       /* RETURN */
			/* recupera posici¢n y color de dibujo anteriores */
			_moveto(antpos.xcoord,antpos.ycoord);
			_setcolor(antcolor);
			return(lng_inp+1);
		case 8 :        /* BACKSPACE */
			/* si est  al inicio, sale */
			if(!lng_inp) break;

			/* retrocede un car cter */
			lng_inp--;

			/* recoge £ltimo car cter tecleado */
			ncar[0]=*(--pcad);

			/* lo borra en buffer y en pantalla */
			*pcad='\0';
			x0=coord[lng_inp];
			_moveto(x0,y);
			_setcolor(colorf);
			_outgtext(ncar);
			_setcolor(color);
			break;
		default :       /* otras */
			/* comprueba n£mero de caracteres tecleados */
			if(lng_inp>=longitud) break;

			/* guarda coordenada 'x' actual */
			coord[lng_inp]=x0;

			/* a¤ade car cter tecleado */
			ncar[0]=(char)tecla;
			*pcad++=ncar[0];
			*pcad='\0';

			/* imprime nuevo car cter */
			_moveto(x0,y);
			_outgtext(ncar);

			/* siguiente posici¢n */
			x0+=_getgtextextent(ncar);

			/* incrementa n£mero de caracteres escritos */
			lng_inp++;

			break;
	}
}

}
