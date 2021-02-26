/****************************************************************************
				   VENTANA.C

	Biblioteca de funciones para gestionar ventanas en pantalla en
	los modos de texto de color.

			     (c)1993 JSJ Soft Ltd.

	Las siguientes funciones son p£blicas:
		- v_impcar: imprime un car†cter y su atributo en una
		    posici¢n de pantalla
		- v_crea: crea una ventana
		- v_dibuja: dibuja una ventana
		- v_abre: abre una ventana
		- v_cierra: cierra una ventana
		- v_pon_cursor: cambia la posici¢n de impresi¢n del
		    texto dentro de una ventana
		- v_impc: imprime un car†cter en una ventana
		- v_impcad: imprime una cadena en una ventana
		- v_modo_texto: cambia el modo de impresi¢n del texto en
		    una ventana
		- v_borra: borra una ventana
		- v_color: cambia el color de impresi¢n del texto en una
		    ventana
		- v_borde: redefine los caracteres del borde de una ventana
		- v_scroll_arr: scroll hacia arriba de una ventana
		- v_scroll_abj: scroll hacia abajo de una ventana
		- v_scroll_izq: scroll hacia la izquierda de una ventana
		- v_scroll_der: scroll hacia la derecha de una ventana

	Las siguientes estructuras est†n definidas en VENTANA.H:
		STC_VENTANA: definici¢n de una ventana
****************************************************************************/

#include <stddef.h>
#include <alloc.h>
#include <string.h>
#include "raton.h"
#include "ventana.h"

/*** Variables globales internas ***/
static char *Wint_Borde="…Õ∏∫≥”ƒŸ";

/*** Prototipos de funciones internas ***/
static void v_guardafondo(BYTE far *fondo, short fil, short col, short ancho,
  short alto);
static void v_recuperafondo(BYTE far *fondo, short fil, short col,
  short ancho, short alto);

#pragma check_pointer(off)
/****************************************************************************
	V_GUARDAFONDO: guarda una porci¢n de la pantalla.
	  Entrada:      'fondo' puntero a buffer
			'fil', 'col' posici¢n de la ventana a guardar
			'ancho', 'alto' dimensiones de la ventana a guardar
****************************************************************************/
void v_guardafondo(BYTE far *fondo, short fil, short col, short ancho,
  short alto)
{
BYTE far *pscr=(BYTE far *)0xb8000000L;
BYTE far *p;
short i, j;

/* calcula puntero a origen de zona a guardar */
pscr+=((fil*80)+col)*2;

for(i=0; i<alto; i++) {
	p=pscr;
	for(j=0; j<(ancho*2); j++) *fondo++=*p++;
	/* siguiente l°nea */
	pscr+=160;
}

}
#pragma check_pointer()

#pragma check_pointer(off)
/****************************************************************************
	V_RECUPERAFONDO: recupera una porci¢n de la pantalla.
	  Entrada:      'fondo' puntero a buffer
			'fil', 'col' posici¢n de la ventana a recuperar
			'ancho', 'alto' dimensiones de la ventana a recuperar
****************************************************************************/
void v_recuperafondo(BYTE far *fondo, short fil, short col, short ancho,
  short alto)
{
BYTE far *pscr=(BYTE far *)0xb8000000L;
BYTE far *p;
short i, j;

/* calcula puntero a origen de zona a recuperar */
pscr+=((fil*80)+col)*2;

for(i=0; i<alto; i++) {
	p=pscr;
	for(j=0; j<(ancho*2); j++) *p++=*fondo++;
	/* siguiente l°nea */
	pscr+=160;
}

}
#pragma check_pointer()

#pragma check_pointer(off)
/****************************************************************************
	V_IMPCAR: imprime un car†cter en una posici¢n de pantalla y con
	  un atributo dado.
	  Entrada:      'fil', 'col' fila y columna d¢nde se imprimir† el
			car†cter (origen de pantalla en 0,0)
			'car' car†cter a imprimir
			'attr' atributo con que se imprimir†
****************************************************************************/
void v_impcar(short fil, short col, BYTE car, BYTE attr)
{
BYTE far *pscr=(BYTE far *)0xb8000000L;

/* calcula puntero a posici¢n de car†cter */
pscr+=((fil*80)+col)*2;

/* imprime car†cter y su atributo */
*pscr=car;
*(pscr+1)=attr;

}
#pragma check_pointer()

/****************************************************************************
	V_CREA: inicializa una ventana rellenando la estructura con los
	  datos suministrados
	  Entrada:      'ventana' puntero a estructura con datos de ventana
			'fil', 'col' posici¢n de la ventana
			'ancho', 'alto' dimensiones de la ventana
			'attr_princ' atributo principal de la ventana
			'attr_s1' atributo para sombra 1
			'attr_s2' atributo para sombra 2
			'titulo' texto para encabezamiento de ventana (NULL
			si ninguno)
****************************************************************************/
void v_crea(STC_VENTANA *ventana, short fil, short col, short ancho,
  short alto, BYTE attr_princ, BYTE attr_s1, BYTE attr_s2, char *titulo)
{

/* inicializa datos ventana */
ventana->fil=fil;
ventana->col=col;
ventana->ancho=ancho;
ventana->alto=alto;
ventana->attr_princ=attr_princ;
ventana->attr_s1=attr_s1;
ventana->attr_s2=attr_s2;
ventana->titulo=titulo;

/* caracteres del borde por defecto */
ventana->borde=Wint_Borde;

/* puntero a buffer para guardar el fondo */
ventana->fondo=NULL;

/* modo de impresi¢n de texto */
ventana->modo_texto=V_LINEA_LINEA;

/* posici¢n de impresi¢n dentro de la ventana */
ventana->filc=0;
ventana->colc=0;

/* color del texto */
ventana->attr_texto=attr_princ;

}

/****************************************************************************
	V_DIBUJA: dibuja una ventana.
	  Entrada:     'ventana' puntero a estructura con datos de ventana
		       'rellena' 0 s¢lo dibuja marco, 1 rellena interior
****************************************************************************/
void v_dibuja(STC_VENTANA *ventana, int rellena)
{
short fil, col, ancho, alto, i, j;
int lng;
char *t;

r_puntero(R_OCULTA);

fil=ventana->fil;
col=ventana->col;
ancho=ventana->ancho;
alto=ventana->alto;

/* dibuja esquinas */
v_impcar(fil,col,ventana->borde[0],ventana->attr_s1);
v_impcar(fil+alto-1,col,ventana->borde[5],ventana->attr_s1);
v_impcar(fil,col+ancho-1,ventana->borde[2],ventana->attr_s2);
v_impcar(fil+alto-1,col+ancho-1,ventana->borde[7],ventana->attr_s2);

/* dibuja bordes superior e inferior */
for(i=col+1; i<(col+ancho-1); i++) {
	v_impcar(fil,i,ventana->borde[1],ventana->attr_s1);
	v_impcar(fil+alto-1,i,ventana->borde[6],ventana->attr_s2);
}

/* dibuja bordes laterales */
for(i=fil+1; i<(fil+alto-1); i++) {
	v_impcar(i,col,ventana->borde[3],ventana->attr_s1);
	v_impcar(i,col+ancho-1,ventana->borde[4],ventana->attr_s2);
}

/* rellena interior */
if(rellena) {
	for(i=fil+1; i<(fil+alto-1); i++) {
		for(j=col+1; j<(col+ancho-1); j++) v_impcar(i,j,' ',
		  ventana->attr_princ);
	}
}

/* imprime encabezamiento */
if(ventana->titulo!=NULL) {
	lng=strlen(ventana->titulo);
	col+=((ancho-lng)/2);
	if(col<=ventana->col) col=ventana->col+1;
	for(t=ventana->titulo; *t; t++, col++) {
		if(col<(ventana->col+ventana->ancho-1))
		  v_impcar(fil,col,*t,ventana->attr_princ);
	}
}

r_puntero(R_MUESTRA);

}

/****************************************************************************
	V_ABRE: abre una ventana.
	  Entrada:      'ventana' puntero a estructura con datos de ventana
****************************************************************************/
void v_abre(STC_VENTANA *ventana)
{
size_t tam;

r_puntero(R_OCULTA);

/* calcula tama§o de buffer para guardar fondo */
tam=ventana->ancho*ventana->alto*2;

/* reserva memoria y guarda el fondo */
ventana->fondo=farmalloc(tam);
if(ventana->fondo!=NULL) v_guardafondo(ventana->fondo,ventana->fil,
  ventana->col,ventana->ancho,ventana->alto);

/* dibuja la ventana */
v_dibuja(ventana,1);

r_puntero(R_MUESTRA);

}

/****************************************************************************
	V_CIERRA: cierra una ventana.
	  Entrada:      'ventana' puntero a estructura con datos de ventana
****************************************************************************/
void v_cierra(STC_VENTANA *ventana)
{

r_puntero(R_OCULTA);

/* si tiene fondo guardado lo recupera y libera memoria */
if(ventana->fondo!=NULL) {
	v_recuperafondo(ventana->fondo,ventana->fil,ventana->col,
	  ventana->ancho,ventana->alto);
	farfree(ventana->fondo);
	ventana->fondo=NULL;
}

r_puntero(R_MUESTRA);

}

/****************************************************************************
	V_PON_CURSOR: cambia la posici¢n de impresi¢n del texto dentro de
	  una ventana.
	  Entrada:      'ventana' puntero a estructura de ventana
			'fil', 'col' posici¢n relativa dentro de la
			ventana:
				'fil' = 0 .. (alto-3)
				'col' = 0 .. (ancho-3)
****************************************************************************/
void v_pon_cursor(STC_VENTANA *ventana, short fil, short col)
{

ventana->filc=fil;
ventana->colc=col;

}

/****************************************************************************
	V_IMPC: imprime un car†cter dentro de una ventana, en la posici¢n
	  actual de impresi¢n.
	  Entrada:      'ventana' puntero a estructura de ventana
			'car' car†cter a imprimir
	  NOTA: si el car†cter cae fuera de la ventana, no lo imprime
****************************************************************************/
void v_impc(STC_VENTANA *ventana, char car)
{
STC_RATON r;
int sobre_car=0;
short maxfil, maxcol, fil, col;

/* calcula m†xima fila y columna */
maxfil=ventana->alto-3;
maxcol=ventana->ancho-3;

/* si el car†cter est† fuera de la ventana, sale */
if((ventana->filc>maxfil) | (ventana->colc>maxcol)) return;

/* posici¢n del car†cter en pantalla */
fil=ventana->fil+ventana->filc+1;
col=ventana->col+ventana->colc+1;

/* da valores por si el rat¢n no est† inicializado */
r.fil=9999;
r.col=9999;

/* si el puntero del rat¢n est† sobre el car†cter lo oculta */
r_estado(&r);
if((r.fil==fil) && (r.col==col)) {
	r_puntero(R_OCULTA);
	sobre_car=1;
}

v_impcar(fil,col,(BYTE)car,ventana->attr_texto);

/* siguiente columna */
ventana->colc++;

if(sobre_car) r_puntero(R_MUESTRA);

}

/****************************************************************************
	V_IMPCAD: imprime una cadena dentro de una ventana, en la posici¢n
	  actual de impresi¢n.
	  Entrada:      'ventana' puntero a estructura de ventana
			'cad' cadena a imprimir
			'rellena' V_RELLENA si se quiere rellenar hasta
			el final de la ventana con espacios, V_NORELLENA
			en otro caso
****************************************************************************/
void v_impcad(STC_VENTANA *ventana, char *cad, int rellena)
{

if(ventana->filc>=(ventana->alto-2)) return;

r_puntero(R_OCULTA);

while(*cad) {
	if(*cad=='\n') {
		ventana->colc=0;
		ventana->filc++;
		if(ventana->filc>=(ventana->alto-2)) return;
	}
	else {
		if(ventana->colc<(ventana->ancho-2))
		  v_impcar(ventana->fil+ventana->filc+1,
		  ventana->col+ventana->colc+1,*cad,ventana->attr_texto);
		else if(ventana->modo_texto==V_PASA_LINEA) {
			ventana->colc=0;
			ventana->filc++;
			v_impcar(ventana->fil+ventana->filc+1,
			ventana->col+ventana->colc+1,*cad,ventana->attr_texto);
		}
		ventana->colc++;
	}
	cad++;
}

/* rellena con espacios */
if(rellena==V_RELLENA) {
	for(; ventana->colc<(ventana->ancho-2); ventana->colc++)
	  v_impcar(ventana->fil+ventana->filc+1,ventana->col+ventana->colc+1,
	  ' ',ventana->attr_texto);
}

r_puntero(R_MUESTRA);

}

/****************************************************************************
	V_MODO_TEXTO:  cambia el modo de impresi¢n del texto dentro de
	  una ventana.
	  Entrada:      'ventana' puntero a estructura de ventana
			'modo' modo de impresi¢n:
				V_LINEA_LINEA imprime una l°nea de texto en
				  cada l°nea de la ventana, si las l°neas
				  de texto son m†s largas que la ventana
				  quedan recortadas
				V_PASA_LINEA si una l°nea de texto es m†s
				  larga que la ventana, la pasa a la
				  siguiente
****************************************************************************/
void v_modo_texto(STC_VENTANA *ventana, int modo)
{

ventana->modo_texto=modo;

}

/****************************************************************************
	V_BORRA: borra el interior de una ventana.
	  Entrada:      'ventana' puntero a estructura de ventana
****************************************************************************/
void v_borra(STC_VENTANA *ventana)
{

/* coloca posici¢n de impresi¢n en origen */
ventana->filc=0;
ventana->colc=0;

/* restaura color del texto a color principal */
ventana->attr_texto=ventana->attr_princ;

v_dibuja(ventana,1);

}

/****************************************************************************
	V_COLOR: cambia el color de impresi¢n del texto en la ventana.
	  Entrada:      'ventana' puntero a estructura de ventana
			'attr' nuevo color para el texto de la ventana
****************************************************************************/
void v_color(STC_VENTANA *ventana, BYTE attr)
{

ventana->attr_texto=attr;

}

/****************************************************************************
	V_BORDE: redefine los caracteres de borde de una ventana.
	  Entrada:      'ventana' puntero a estructura de ventana
			'borde' puntero a cadena con los nuevos caracteres
			del borde
****************************************************************************/
void v_borde(STC_VENTANA *ventana, char *borde)
{

ventana->borde=borde;

}

#pragma check_pointer(off)
/****************************************************************************
	V_SCROLL_ARR: realiza scroll hacia arriba de una ventana.
	  Entrada:      'ventana' puntero a estructura de ventana
			'n' n£mero de l°neas a desplazar
****************************************************************************/
void v_scroll_arr(STC_VENTANA *ventana, int n)
{
BYTE far *pscr0=(BYTE far *)0xb8000000L;
BYTE far *pscr;
int i, s, anch, alt;

r_puntero(R_OCULTA);

/* origen y dimensiones de la zona a desplazar */
pscr0+=(((ventana->fil+1)*80)+(ventana->col+1))*2;
anch=ventana->ancho-2;
alt=ventana->alto-2;

for(s=0; s<n; s++) {
	pscr=pscr0;
	for(i=0; i<(alt-1); i++) {
		_fmemcpy(pscr,pscr+160,anch*2);
		pscr+=160;
	}

	/* rellena £ltima l°nea */
	for(i=0; i<anch; i++) {
		*pscr++=' ';
		*pscr++=ventana->attr_princ;
	}
}

r_puntero(R_MUESTRA);

}
#pragma check_pointer()

#pragma check_pointer(off)
/****************************************************************************
	V_SCROLL_ABJ: realiza scroll hacia abajo de una ventana.
	  Entrada:      'ventana' puntero a estructura de ventana
			'n' n£mero de l°neas a desplazar
****************************************************************************/
void v_scroll_abj(STC_VENTANA *ventana, int n)
{
BYTE far *pscr0=(BYTE far *)0xb8000000L;
BYTE far *pscr;
int i, s, anch, alt;
r_puntero(R_OCULTA);

/* dimensiones de la zona a desplazar y puntero a £ltima l°nea */
pscr0+=(((ventana->fil+ventana->alto-2)*80)+(ventana->col+1))*2;
anch=ventana->ancho-2;
alt=ventana->alto-2;

for(s=0; s<n; s++) {
	pscr=pscr0;
	for(i=0; i<(alt-1); i++) {
		_fmemcpy(pscr,pscr-160,anch*2);
		pscr-=160;
	}

	/* rellena primera l°nea */
	for(i=0; i<anch; i++) {
		*pscr++=' ';
		*pscr++=ventana->attr_princ;
	}
}

r_puntero(R_MUESTRA);

}
#pragma check_pointer()

#pragma check_pointer(off)
/****************************************************************************
	V_SCROLL_IZQ: realiza scroll hacia la izquierda de una ventana.
	  Entrada:      'ventana' puntero a estructura de ventana
			'n' n£mero de caracteres a desplazar
****************************************************************************/
void v_scroll_izq(STC_VENTANA *ventana, int n)
{
BYTE far *pscr0=(BYTE far *)0xb8000000L;
BYTE far *pscr;
int i, s, anch, alt;

r_puntero(R_OCULTA);

/* origen y dimensiones de la zona a desplazar */
pscr0+=(((ventana->fil+1)*80)+(ventana->col+1))*2;
anch=(ventana->ancho-3)*2;
alt=ventana->alto-2;

for(s=0; s<n; s++) {
	pscr=pscr0;
	for(i=0; i<alt; i++) {
		_fmemcpy(pscr,pscr+2,anch);

		/* rellena £ltimo car†cter */
		*(pscr+anch)=' ';
		*(pscr+anch+1)=ventana->attr_princ;

		/* siguiente fila*/
		pscr+=160;
	}
}

r_puntero(R_MUESTRA);

}
#pragma check_pointer()

#pragma check_pointer(off)
/****************************************************************************
	V_SCROLL_DER: realiza scroll hacia la derecha de una ventana.
	  Entrada:      'ventana' puntero a estructura de ventana
			'n' n£mero de caracteres a desplazar
****************************************************************************/
void v_scroll_der(STC_VENTANA *ventana, int n)
{
BYTE far *pscr0=(BYTE far *)0xb8000000L;
BYTE far *pscr;
int i, s, anch, alt;

r_puntero(R_OCULTA);

/* origen y dimensiones de la zona a desplazar */
pscr0+=(((ventana->fil+1)*80)+(ventana->col+1))*2;
anch=(ventana->ancho-3)*2;
alt=ventana->alto-2;

for(s=0; s<n; s++) {
	pscr=pscr0;
	for(i=0; i<alt; i++) {
		_fmemmove(pscr+2,pscr,anch);

		/* rellena primer car†cter */
		*pscr=' ';
		*(pscr+1)=ventana->attr_princ;

		/* siguiente fila*/
		pscr+=160;
	}
}

r_puntero(R_MUESTRA);

}
#pragma check_pointer()

