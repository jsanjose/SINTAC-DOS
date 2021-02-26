/****************************************************************************
				 EDITOR.C

	Biblioteca de funciones para gestionar un editor de textos.

			     (c)1993 JSJ Soft Ltd.

	Las siguientes funciones son p£blicas:
		- e_inicializa: inicializa el editor
		- e_borra_texto: borra texto del editor y libera memoria
		- e_editor: bucle principal del editor
		- e_dibuja_editor: dibuja ventana de edici¢n
		- e_carga_texto: carga un fichero en el editor
		- e_graba_texto: graba el texto del editor en un fichero
		- e_nombre_fichero: devuelve el nombre del fichero
		    cargado en el editor
		- e_inicia_busqueda: busca una cadena en el texto
		- e_continua_busqueda: contin£a la b£squeda de una cadena
		- e_modificado: comprueba si el texto del editor ha sido
		    modificado
		- e_cambia_modo: cambia modo de edici¢n
		- e_carga_textox: carga texto colocado en mitad de un
		    fichero
		- e_vacia_bloque: libera memoria ocupada por bloque de
		    texto
		- e_graba_bloque: graba bloque en un fichero
		- e_inserta_bloque: inserta bloque en la posici¢n actual
		    del cursor
****************************************************************************/

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <bios.h>
#include "ventana.h"
#include "raton.h"
#include "cuadro.h"
#include "editor.h"

/*** Variables globales internas ***/
static STC_VENTANA ved;         /* ventana de edici¢n */
static STC_CUADRO cbusca;       /* cuadro para b£squedas */
/* elementos del cuadro de di logo para b£squedas */
static STC_ELEM_INPUT cb_input={
	"^Texto"
};
static STC_ELEM_BOTON cb_vale={
	" ^Vale"
};
static STC_ELEM_BOTON cb_salir={
	" ^Salir"
};
static STC_ELEM_CHECK cb_chkmay={
	"^May£sculas/min£sculas"
};
static char cadena_busca[E_MAXLNGLIN];

static char *lin[E_MAXNUMLIN];  /* punteros a l¡neas de texto */
static STC_BLOQUE *bloque;      /* puntero a primera l¡nea bloque de texto */
static char palabra_cursor[E_MAXLNGLIN];

/* estructura con informaci¢n sobre el estado del editor */
static struct {
	int lin, col;           /* posici¢n del cursor en el texto */
	int filcur, colcur;     /* posici¢n del cursor en la ventana */
	int numlin;             /* n§ de l¡neas actual */
	int hdesplz;            /* desplazamiento horz. de la ventana */
	int modificado;         /* 1 si texto fue modificado, 0 si no */
	int modo_ed;            /* modo de edici¢n, E_EDITA permite editar */
				/* E_SOLOLECT s¢lo permite ver */
	char fich0[_MAX_PATH];  /* nombre de fichero por defecto */
	char fich[_MAX_PATH];   /* nombre del fichero que se est  editando */
	int lin_ib, col_ib;     /* posici¢n de inicio de bloque */
	int lin_fb, col_fb;     /* posici¢n final de bloque */
	int bloque;             /* 1 si en modo de bloque, 0 si no */
	BYTE attr_blq;          /* atributo para marcar bloque */
} ed={
	0, 0,
	0, 0,
	0,
	0,
	0,
	E_EDITA,
	"",
	"",
	0, 0,
	0, 0,
	0,
	0
};

/*** Prototipos de funciones internas ***/
static void pon_cursor(short fil, short col);
static void beep(void);
static char mayuscula(char c);
static void conv_mayuscula(char *cad);
static int esta_en_bloque(int lin, int col);
static void imprime_linea_ed(int l);
static void imprime_texto_ed(void);
static void scroll_der_ed(void);
static void scroll_izq_ed(void);
static void scroll_arr_ed(void);
static void scroll_abj_ed(void);
static void cursor_izq(void);
static void cursor_der(void);
static void cursor_abj(void);
static void cursor_arr(void);
static void cursor_fin_lin(void);
static void cursor_inicio_lin(void);
static void cursor_fin(void);
static void cursor_inicio(void);
static void pagina_arr(void);
static void pagina_abj(void);
static void cursor_pos(short fil, short col);
static int inserta_caracter(char c);
static int borra_caracter(int nlin, int col);
static int inserta_linea(void);
static int borra_linea(int l);
static int copia_linea(int lin1, int lin2);
static void imprime_info(void);
static void ajusta_linea(char *lin);
static void suprime_espacios_fin(int l);
static void pausa(clock_t p);
static int busca_cadena(void);
static void marca_bloque(void);
static int copia_bloque(int lini, int coli, int linf, int colf);
static int borra_bloque(int lini, int coli, int linf, int colf);
static int inserta_bloque(int nlin, int col);
static void coge_palabra_cursor(void);

/****************************************************************************
	PON_CURSOR: coloca el cursor en una posici¢n de pantalla.
	  Entrada:      'fil', 'col' fila y columna del cursor
****************************************************************************/
void pon_cursor(short fil, short col)
{

_asm {
	mov ah,02h              ; funci¢n definir posici¢n del cursor
	mov bh,0                ; supone p gina 0
	mov dh,BYTE PTR fil     ; DH = fila del cursor
	mov dl,BYTE PTR col     ; DL = columna del cursor
	int 10h
}

}

/****************************************************************************
	BEEP: produce un pitido en el altavoz del PC.
****************************************************************************/
void beep(void)
{

_asm {
	sub bx,bx               ; p gina 0
	mov ax,0E07h            ; escribe el car cter de alarma
	int 10h
}

}

/****************************************************************************
	MAYUSCULA: convierte una letra en may£scula.
	  Entrada:      'c' car cter a convertir
	  Salida:       may£scula del car cter
****************************************************************************/
char mayuscula(char c)
{

if((c>='a') && (c<='z')) return(c-(char)'a'+(char)'A');

switch(c) {
	case (char)'¤' :
		c=(char)'¥';
		break;
	case (char)' ' :
		c='A';
		break;
	case (char)'‚' :
		c='E';
		break;
	case (char)'¡' :
		c='I';
		break;
	case (char)'¢' :
		c='O';
		break;
	case (char)'£' :
	case (char)'' :
		c='U';
		break;
}

return(c);
}

/****************************************************************************
	CONV_MAYUSCULA: convierte una cadena en may£scula.
	  Entrada:      'cad' cadena a convertir
****************************************************************************/
void conv_mayuscula(char *cad)
{

while(*cad) {
	*cad=mayuscula(*cad);
	cad++;
}

}

/****************************************************************************
	ESTA_EN_BLOQUE: comprueba si un car cter pertenece a un bloque
	  marcado.
	  Entrada:      'lin', 'col' posici¢n del car cter dentro del
			texto
	  Salida:       1 si el car cter est  en el bloque marcado, 0 si no
****************************************************************************/
int esta_en_bloque(int lin, int col)
{

if(ed.lin_ib==ed.lin_fb) {
	if(lin!=ed.lin_ib) return(0);
	if((ed.col_ib<=ed.col_fb) && (col>=ed.col_ib) && (col<=ed.col_fb))
	  return(1);
	if((ed.col_ib>ed.col_fb) && (col>=ed.col_fb) && (col<=ed.col_ib))
	  return(1);
}
else if(ed.lin_ib<ed.lin_fb) {
	if((lin>ed.lin_ib) && (lin<ed.lin_fb)) return(1);
	else if((lin==ed.lin_ib) && (col>=ed.col_ib)) return(1);
	else if((lin==ed.lin_fb) && (col<=ed.col_fb)) return(1);
}
else {
	if((lin>ed.lin_fb) && (lin<ed.lin_ib)) return(1);
	else if((lin==ed.lin_fb) && (col>=ed.col_fb)) return(1);
	else if((lin==ed.lin_ib) && (col<=ed.col_ib)) return(1);
}

return(0);
}

/****************************************************************************
	IMPRIME_LINEA_ED: imprime una l¡nea en la ventana del editor.
	  Entrada:      'l' n£mero de l¡nea a imprimir
****************************************************************************/
void imprime_linea_ed(int l)
{
int i, fil, col;
char *txt;

if(l>(ed.numlin-1)) return;

r_puntero(R_OCULTA);

/* calcula fila de la l¡nea en la ventana */
if(l<ed.lin) fil=ed.filcur-(ed.lin-l);
else fil=ed.filcur+(l-ed.lin);

/* calcula desplazamiento horizontal de ventana de edici¢n */
ed.hdesplz=ed.col-ed.colcur;

/* imprime s¢lo si desplazamiento no hace que sobrepase el final de l¡nea */
fil+=ved.fil+1;
col=0;
if(ed.hdesplz<(int)strlen(lin[l])) {
	txt=lin[l]+ed.hdesplz;
	while(*txt && (col<ved.ancho-2)) {
		/* si hay bloque activo comprueba si car cter est  */
		/* dentro del bloque y lo imprime resaltado */
		if(ed.bloque && esta_en_bloque(l,col+ed.hdesplz))
		  v_impcar(fil,col+ved.col+1,*txt,ed.attr_blq);
		else v_impcar(fil,col+ved.col+1,*txt,ved.attr_princ);
		col++;
		txt++;
	}
}

/* rellena con espacios hasta el final de la ventana */
for(i=col; i<(ved.ancho-2); i++) v_impcar(fil,i+ved.col+1,' ',ved.attr_princ);

r_puntero(R_MUESTRA);

}

/****************************************************************************
	IMPRIME_TEXTO_ED: imprime el texto en la ventana del editor.
****************************************************************************/
void imprime_texto_ed(void)
{
int i, j, lin_pr, lin_ult;

lin_pr=ed.lin-ed.filcur;
lin_ult=lin_pr+ved.alto-2;
if(lin_ult>ed.numlin) lin_ult=ed.numlin;

for(i=lin_pr; i<lin_ult; i++) imprime_linea_ed(i);

/* rellena hasta fin de ventana */
for(i=lin_ult-lin_pr; i<(ved.alto-2); i++) {
	for(j=0; j<(ved.ancho-2); j++) v_impcar(i+ved.fil+1,j+ved.col+1,' ',
	  ved.attr_princ);
}

}

/****************************************************************************
	SCROLL_DER_ED: desplaza hacia la derecha la ventana de edici¢n.
****************************************************************************/
void scroll_der_ed(void)
{
int i, lin_pr, lin_ult;

v_scroll_der(&ved,1);

ed.hdesplz=ed.col-ed.colcur;

lin_pr=ed.lin-ed.filcur;
lin_ult=lin_pr+ved.alto-2;
if(lin_ult>ed.numlin) lin_ult=ed.numlin;

/* imprime los primeros caracteres de todas las l¡neas */
v_pon_cursor(&ved,0,0);
for(i=lin_pr; i<lin_ult; i++) {
	if(ed.hdesplz<(int)strlen(lin[i])) v_impc(&ved,*(lin[i]+ed.hdesplz));
	ved.filc++;
	ved.colc=0;
}

}

/****************************************************************************
	SCROLL_IZQ_ED: desplaza hacia la izquierda la ventana de edici¢n.
****************************************************************************/
void scroll_izq_ed(void)
{
int i, lin_pr, lin_ult, d;

v_scroll_izq(&ved,1);

ed.hdesplz=ed.col-ed.colcur;

lin_pr=ed.lin-ed.filcur;
lin_ult=lin_pr+ved.alto-2;
if(lin_ult>ed.numlin) lin_ult=ed.numlin;

/* imprime los £ltimos caracteres de todas las l¡neas */
v_pon_cursor(&ved,0,ved.ancho-3);
for(i=lin_pr; i<lin_ult; i++) {
	d=ed.hdesplz+ved.ancho-3;
	if(d<(int)strlen(lin[i])) v_impc(&ved,*(lin[i]+d));
	ved.filc++;
	ved.colc=ved.ancho-3;
}

}

/****************************************************************************
	SCROLL_ARR_ED: desplaza hacia arriba la ventana de edici¢n.
****************************************************************************/
void scroll_arr_ed(void)
{
int lin_ult;

v_scroll_arr(&ved,1);

/* imprime £tima l¡nea */
lin_ult=ed.lin-ed.filcur+ved.alto-3;
if(lin_ult>ed.numlin) lin_ult=ed.numlin;
imprime_linea_ed(lin_ult);

}

/****************************************************************************
	SCROLL_ABJ_ED: desplaza hacia abajo la ventana de edici¢n.
****************************************************************************/
void scroll_abj_ed(void)
{
int lin_pr;

v_scroll_abj(&ved,1);

/* imprime primera l¡nea */
lin_pr=ed.lin-ed.filcur;
imprime_linea_ed(lin_pr);

}

/****************************************************************************
	CURSOR_IZQ: mueve el cursor un car cter hacia la izquierda.
****************************************************************************/
void cursor_izq(void)
{

if(ed.col>0) {
	ed.col--;
	if(ed.colcur>0) ed.colcur--;
	else scroll_der_ed();
}

}

/****************************************************************************
	CURSOR_DER: mueve el cursor un car cter hacia la derecha.
****************************************************************************/
void cursor_der(void)
{
int i;

i=strlen(lin[ed.lin]);
if(ed.col<i) {
	ed.col++;
	if(ed.colcur<(ed.col-ed.hdesplz)) {
		if(ed.colcur<(ved.ancho-3)) ed.colcur++;
		else scroll_izq_ed();
	}
}

}

/****************************************************************************
	CURSOR_ABJ: mueve el cursor una l¡nea hacia abajo.
****************************************************************************/
void cursor_abj(void)
{
int i;

if(ed.lin<(ed.numlin-1)) {
	ed.lin++;
	if(ed.filcur<ed.lin) {
		if(ed.filcur<(ved.alto-3)) ed.filcur++;
		else scroll_arr_ed();
	}
}

i=strlen(lin[ed.lin]);
if(ed.col>i) ed.col=i;
ed.colcur=ed.col-ed.hdesplz;
if((ed.colcur<0) || (ed.colcur>(ved.ancho-3))) cursor_fin_lin();

}

/****************************************************************************
	CURSOR_ARR: mueve el cursor una l¡nea hacia arriba.
****************************************************************************/
void cursor_arr(void)
{
int i;

if(ed.lin>0) {
	ed.lin--;
	if(ed.filcur>0) ed.filcur--;
	else scroll_abj_ed();
}

i=strlen(lin[ed.lin]);
if(ed.col>i) ed.col=i;
ed.colcur=ed.col-ed.hdesplz;
if((ed.colcur<0) || (ed.colcur>(ved.ancho-3))) cursor_fin_lin();

}

/****************************************************************************
	CURSOR_FIN_LIN: mueve el cursor al final de la l¡nea actual.
****************************************************************************/
void cursor_fin_lin(void)
{

ed.col=strlen(lin[ed.lin]);
ed.colcur=ved.ancho-3;
if(ed.colcur>ed.col) ed.colcur=ed.col;

pon_cursor(ed.filcur+ved.fil+1,ed.colcur+ved.col+1);
imprime_texto_ed();

}

/****************************************************************************
	CURSOR_INICIO_LIN: mueve el cursor al inicio de la l¡nea actual.
****************************************************************************/
void cursor_inicio_lin(void)
{

ed.col=0;
ed.colcur=0;

pon_cursor(ed.filcur+ved.fil+1,ed.colcur+ved.col+1);
imprime_texto_ed();

}

/****************************************************************************
	CURSOR_FIN: mueve el cursor al final del texto.
****************************************************************************/
void cursor_fin(void)
{

ed.lin=ed.numlin-1;
ed.col=strlen(lin[ed.lin]);
ed.filcur=ed.lin;
if(ed.filcur>(ved.alto-3)) ed.filcur=ved.alto-3;
ed.colcur=ved.ancho-3;
if(ed.colcur>ed.col) ed.colcur=ed.col;

pon_cursor(ed.filcur+ved.fil+1,ed.colcur+ved.col+1);
imprime_texto_ed();

}

/****************************************************************************
	CURSOR_INICIO: mueve el cursor al inicio del texto.
****************************************************************************/
void cursor_inicio(void)
{

ed.lin=0;
ed.col=0;
ed.filcur=0;
ed.colcur=0;

pon_cursor(ed.filcur+ved.fil+1,ed.colcur+ved.col+1);
imprime_texto_ed();

}

/****************************************************************************
	PAGINA_ARR: desplaza una p gina hacia arriba.
****************************************************************************/
void pagina_arr(void)
{
int i;

ed.lin-=ved.alto-3;
if(ed.lin<0) ed.lin=0;

ed.filcur=0;

i=strlen(lin[ed.lin]);
if(ed.col>i) ed.col=i;
ed.colcur=ed.col-ed.hdesplz;
if((ed.colcur<0) || (ed.colcur>(ved.ancho-3))) cursor_fin_lin();

pon_cursor(ed.filcur+ved.fil+1,ed.colcur+ved.col+1);
imprime_texto_ed();

}

/****************************************************************************
	PAGINA_ABJ: desplaza una p gina hacia abajo.
****************************************************************************/
void pagina_abj(void)
{
int i;

ed.lin+=ved.alto-3;
if(ed.lin>(ed.numlin-1)) ed.lin=ed.numlin-1;

ed.filcur=ved.alto-3;
if(ed.filcur>ed.lin) ed.filcur=ed.lin;

i=strlen(lin[ed.lin]);
if(ed.col>i) ed.col=i;
ed.colcur=ed.col-ed.hdesplz;
if((ed.colcur<0) || (ed.colcur>(ved.ancho-3))) cursor_fin_lin();

pon_cursor(ed.filcur+ved.fil+1,ed.colcur+ved.col+1);
imprime_texto_ed();

}

/****************************************************************************
	CURSOR_POS: coloca el cursor en una posici¢n dentro de la
	  ventana de edici¢n.
	  Entrada:      'fil', 'col' posici¢n del cursor dentro de la
			ventana de edici¢n
****************************************************************************/
void cursor_pos(short fil, short col)
{
int lin_pr, col_pr, lng;

/* ajusta posici¢n si se sale de la ventana */
if(fil>(ved.alto-3)) fil=ved.alto-3;
if(col>(ved.ancho-3)) col=ved.ancho-3;

/* calcula l¡nea del cursor */
lin_pr=ed.lin-ed.filcur;
ed.lin=lin_pr+fil;
if(ed.lin>(ed.numlin-1)) ed.lin=ed.numlin-1;
ed.filcur=fil;
if(ed.filcur>ed.lin) ed.filcur=ed.lin;

/* calcula columna del cursor */
col_pr=ed.col-ed.colcur;
ed.col=col_pr+col;
lng=strlen(lin[ed.lin]);
if(ed.col>lng) ed.col=lng;
ed.colcur=col;
if((ed.colcur+ed.hdesplz)>ed.col) cursor_fin_lin();

}

/****************************************************************************
	INSERTA_CARACTER: inserta un car cter en la posici¢n actual del
	  cursor.
	  Entrada:      'c' car cter a insertar
	  Salida:       1 si se pudo insertar, 0 si no
****************************************************************************/
int inserta_caracter(char c)
{
char *l, *ll;
int lng, i;

l=lin[ed.lin];

/* longitud actual de la l¡nea contando '\0' final */
lng=strlen(l)+1;

/* si la l¡nea ha alcanzado la m xima longitud permitida, sale */
if(lng>=E_MAXLNGLIN) return(0);

/* indica texto modificado */
ed.modificado=1;

/* a¤ade un byte m s a la l¡nea */
if((l=(char *)realloc(l,lng+1))==NULL) return(0);
lin[ed.lin]=l;

/* inserta el car cter en la posici¢n correspondiente desplazando el */
/* resto de la l¡nea si es necesario */
for(i=0; i<ed.col; i++) l++;

/* puntero auxiliar al final de la l¡nea */
for(ll=l; *ll; ll++);

/* desplaza caracteres */
for(; ll>=l; ll--) *(ll+1)=*ll;
*l=c;

return(1);
}

/****************************************************************************
	BORRA_CARACTER: borra un car cter de una l¡nea.
	  Entrada:      'nlin' l¡nea
			'col' posici¢n del car cter dentro de la l¡nea
	  Salida:       1 si se pudo borrar, 0 si no
****************************************************************************/
int borra_caracter(int nlin, int col)
{
char *l;
int lng, i;

/* puntero a l¡nea, y longitud */
l=lin[nlin];
lng=strlen(l);

/* si sobrepasa la l¡nea, sale */
if(col>=lng) return(0);

/* indica texto modificado */
ed.modificado=1;

/* puntero a car cter a borrar */
l+=col;

/* borra car cter */
for(i=0; i<(lng-col); i++, l++) *l=*(l+1);

/* reasigna memoria */
if((l=(char *)realloc(lin[nlin],lng))!=NULL) lin[nlin]=l;

return(1);
}

/****************************************************************************
	INSERTA_LINEA: inserta una l¡nea debajo de la actual, moviendo el
	  texto desde el car cter actual hasta el final a la nueva l¡nea.
	  Salida:       1 si pudo insertar, 0 si no
****************************************************************************/
int inserta_linea(void)
{
char *l1, *l2;
int lng1, lng2, i;

/* si ya est n todas las l¡neas ocupadas, sale */
if(ed.numlin>=E_MAXNUMLIN) return(0);

/* indica texto modificado */
ed.modificado=1;

/* puntero a car cter actual de la l¡nea */
l1=lin[ed.lin];
lng1=strlen(l1);
l1+=ed.col;

/* reserva memoria para la nueva l¡nea */
lng2=lng1-ed.col;
l2=(char *)malloc(lng2+1);
if(l2==NULL) return(0);

/* desplaza l¡neas */
for(i=ed.numlin; i>(ed.lin+1); i--) lin[i]=lin[i-1];

/* traslada texto a nueva l¡nea */
lin[ed.lin+1]=l2;
for(i=0; i<lng2; i++) *l2++=*l1++;
*l2='\0';

/* ajusta tama¤o de l¡nea anterior */
if((l1=(char *)realloc(lin[ed.lin],ed.col+1))==NULL) return(0);
lin[ed.lin]=l1;
*(l1+ed.col)='\0';

/* incrementa n£mero de l¡neas */
ed.numlin++;

return(1);
}

/****************************************************************************
	BORRA_LINEA: borra una l¡nea de texto y desplaza el resto.
	  Entrada:      'l' n£mero de l¡nea a borrar
	  Salida:       1 si pudo borrar, 0 si no
****************************************************************************/
int borra_linea(int l)
{
int i;

/* si no es n£mero de l¡nea v lido, sale */
if(l>=ed.numlin) return(0);

/* indica texto modificado */
ed.modificado=1;

/* libera memoria */
free(lin[l]);

/* desplaza el resto de las l¡neas */
for(i=l; i<(ed.numlin-1); i++) lin[i]=lin[i+1];
lin[i]=NULL;

ed.numlin--;
if(ed.numlin==0) {
	lin[0]=(char *)malloc(1);
	*lin[0]='\0';
	ed.numlin=1;
	cursor_inicio_lin();
	return(1);
}
else if(ed.lin>=ed.numlin) cursor_arr();

return(1);
}

/****************************************************************************
	COPIA_LINEA: copia una l¡nea de texto al final de otra.
	  Entrada:      'lin1' n£mero de l¡nea de destino
			'lin2' n£mero de l¡nea a copiar
	  Salida:       1 si pudo copiar, 0 si no
****************************************************************************/
int copia_linea(int lin1, int lin2)
{
char *l1, *l2;
int i, lng1, lng2;

/* si n£mero de l¡nea no v lido, sale */
if((lin1>=ed.numlin) || (lin2>=ed.numlin)) return(0);

/* indica texto modificado */
ed.modificado=1;

l1=lin[lin1];
l2=lin[lin2];
lng1=strlen(l1);
lng2=strlen(l2);

/* si la segunda l¡nea est  vac¡a, sale sin hacer nada */
if(lng2==0) return(1);

/* si la l¡nea resultante sobrepasa la longitud m xima, sale */
if((lng1+lng2)>E_MAXLNGLIN) return(0);

/* reasigna memoria para la l¡nea destino */
if((l1=(char *)realloc(lin[lin1],lng1+lng2+1))==NULL) return(0);
lin[lin1]=l1;

/* copia l¡nea al final de la l¡nea de destino */
for(i=0; i<lng1; i++) l1++;
while(*l2) *l1++=*l2++;
*l1='\0';

return(1);
}

/****************************************************************************
	IMPRIME_INFO: imprime informaci¢n sobre estado del editor.
****************************************************************************/
void imprime_info(void)
{
int i, lng;
short fil, col, pos_ed;
char info[81], *txt;

r_puntero(R_OCULTA);

/* imprime marca de texto modificado y fila y columna del cursor */
fil=ved.fil+ved.alto-1;
col=ved.col+2;
if(ed.modificado) v_impcar(fil,col-1,E_CHRMOD,ved.attr_s2);
else v_impcar(fil,col-1,E_CHRNOMOD,ved.attr_s2);
sprintf(info," %04u:%03u ",ed.lin+1,ed.col+1);
for(txt=info; *txt; txt++, col++) v_impcar(fil,col,*txt,ved.attr_s2);

/* nombre de fichero */
fil=ved.fil;
lng=strlen(ed.fich);
if(lng>(ved.ancho-4)) lng=ved.ancho-4;
col=ved.col+((ved.ancho-lng-2)/2);
v_impcar(fil,col,E_CHR1F,ved.attr_s1);
col++;
for(txt=ed.fich, i=0; (*txt && (i<lng)); txt++, i++, col++) v_impcar(fil,col,
  *txt,ved.attr_s1);
v_impcar(fil,col,E_CHR2F,ved.attr_s1);

/* barra vertical de desplazamiento */
fil=ved.fil+1;
col=ved.col+ved.ancho-1;
v_impcar(fil,col,E_FLECHARR,ved.attr_s2);
fil++;
lng=ved.alto-4;
for(i=0; i<lng; i++) {
	v_impcar(fil,col,E_CARRELL1,ved.attr_s2);
	fil++;
}
v_impcar(fil,col,E_FLECHABJ,ved.attr_s2);
if(ed.numlin==1) pos_ed=0;
else pos_ed=(short)(((unsigned long)(lng-1)*ed.lin)/(ed.numlin-1));
pos_ed+=ved.fil+2;
v_impcar(pos_ed,col,E_CARRELL2,ved.attr_s2);

/* barra horizontal de desplazamiento */
fil=ved.fil+ved.alto-1;
col=ved.col+12;
v_impcar(fil,col,E_FLECHIZQ,ved.attr_s2);
col++;
lng=ved.ancho-15;
for(i=0; i<lng; i++) {
	v_impcar(fil,col,E_CARRELL1,ved.attr_s2);
	col++;
}
v_impcar(fil,col,E_FLECHDER,ved.attr_s2);
pos_ed=((lng*ed.col)/E_MAXLNGLIN)+ved.col+13;
v_impcar(fil,pos_ed,E_CARRELL2,ved.attr_s2);

r_puntero(R_MUESTRA);

}

/****************************************************************************
	AJUSTA_LINEA: expande las tabulaciones de una l¡nea de texto y
	  suprime el '\n' final si lo hay.
	  Entrada:      'lin' puntero a la l¡nea con el texto
****************************************************************************/
void ajusta_linea(char *lin)
{
int i, j;
char temp[E_MAXLNGLIN], *p, *pt;

/* puntero a la l¡nea */
p=lin;
/* puntero a buffer temporal */
pt=temp;

for(i=0; ((i<(E_MAXLNGLIN-1)) && *p && (*p!='\n')); i++, p++) {
	/* si es tabulaci¢n, la expande */
	if(*p=='\t') {
		do {
			*pt++=' ';
			i++;
		} while(((i % 8)!=0) && (i<(E_MAXLNGLIN-1)));
		i--;
	}
	else *pt++=*p;
}

/* coloca '\0' final */
*pt='\0';

/* devuelve cadena expandida */
strcpy(lin,temp);

}

/****************************************************************************
	SUPRIME_ESPACIOS_FIN: suprime los espacios al final de una l¡nea.
	  Entrada:      'l' n£mero de l¡nea
****************************************************************************/
void suprime_espacios_fin(int l)
{
char *pl;
int lng, i=0;

/* puntero al final de la l¡nea */
lng=strlen(lin[l]);
pl=lin[l]+lng-1;

/* busca £ltimo car cter que no sea espacio y cuenta espacios finales */
while(*pl==' ') {
	pl--;
	i++;
}

if(i) {
	/* indica texto modificado */
	ed.modificado=1;

	/* marca fin de la l¡nea */
	*(pl+1)='\0';

	/* reasigna memoria */
	if((pl=(char *)realloc(lin[l],lng-i+1))!=NULL) lin[l]=pl;
}

}

/****************************************************************************
	PAUSA: realiza una pausa.
	  Entrada:      'p' tiempo de la pausa en milisegundos
****************************************************************************/
void pausa(clock_t p)
{
clock_t t1, t2;

/* si pausa es 0, espera que se pulse una tecla */
if(!p) return;

t1=p+clock();
do {
	t2=clock();
} while(t2<t1);

}

/****************************************************************************
	BUSCA_CADENA: busca una cadena desde la posici¢n actual del cursor.
	  Salida:       1 cadena encontrada, 0 no
****************************************************************************/
int busca_cadena(void)
{
char linea[E_MAXLNGLIN], cadena[E_MAXLNGLIN];
int i;
char *l, *p;

/* si cadena a buscar vac¡a, sale */
if(*cadena_busca=='\0') return(0);

/* posici¢n actual del cursor */
i=ed.lin;
l=lin[i]+ed.col;
while(i<ed.numlin) {
	strcpy(linea,l);
	strcpy(cadena,cadena_busca);
	/* si no distingue may£sculas y min£sculas, convierte a may£sculas */
	if(cb_chkmay.estado==0) {
		conv_mayuscula(linea);
		conv_mayuscula(cadena);
	}
	p=strstr(linea,cadena);
	if(p!=NULL) break;
	i++;
	l=lin[i];
}

/* si cadena no encontrada, sale */
if(i==ed.numlin) return(0);

/* coloca cursor al inicio de la cadena encontrada */
ed.lin=i;
ed.col=p-linea+l-lin[i];
if(ed.col<(ved.ancho-3)) ed.colcur=ed.col;

return(1);
}

/****************************************************************************
	MARCA_BLOQUE: marca un bloque de texto.
****************************************************************************/
void marca_bloque(void)
{
STC_RATON r;
int fin_bloque=0, accion, boton=0, ccursor;
short minfil, mincol, maxfil, maxcol, posv;
unsigned tecla;

ed.bloque=1;
ed.lin_ib=ed.lin;
ed.col_ib=ed.col;

/* guarda posici¢n actual del cursor */
ccursor=ed.colcur;

while(!fin_bloque) {
	ed.lin_fb=ed.lin;
	ed.col_fb=ed.col;

	pon_cursor(ed.filcur+ved.fil+1,ed.colcur+ved.col+1);
	imprime_texto_ed();
	imprime_info();

	do {
		accion=0;
		r_estado(&r);
		if(r.boton1) {
			accion=1;
			if(boton<2) boton++;
		}
		else if(r.boton2) {
			boton=0;
			accion=2;
		}
		else {
			boton=0;
			tecla=_bios_keybrd(_KEYBRD_READY);
			if(tecla) {
				_bios_keybrd(_KEYBRD_READ);
				accion=3;
			}
		}
	} while(!accion);

	if(accion==1) {
		/* calcula l¡mites de la ventana de edici¢n posici¢n */
		/* del cuadro de desplazamiento */
		minfil=ved.fil;
		maxfil=ved.fil+ved.alto-1;
		mincol=ved.col;
		maxcol=ved.col+ved.ancho-1;

		if(ed.numlin==1) posv=0;
		else posv=(short)(((unsigned long)(ved.alto-5)*ed.lin)/
		  (ed.numlin-1));
		posv+=ved.fil+2;

		/* si est  fuera de la ventana de edici¢n, sale */
		if((r.fil<minfil) || (r.fil>maxfil) || (r.col<mincol) ||
		  (r.col>maxcol)) fin_bloque=1;
		/* comprueba si est  en barra vertical de desplazamiento */
		else if(r.col==maxcol) {
			if(r.fil==(minfil+1)) cursor_arr();
			else if(r.fil==(maxfil-1)) cursor_abj();
			else if(r.fil==minfil) cursor_inicio();
			else if(r.fil==maxfil) cursor_fin();
			else if(r.fil<posv) pagina_arr();
			else if(r.fil>posv) pagina_abj();
			else if(r.fil==posv) {
				if(posv==(ved.fil+2)) cursor_inicio();
				else if(posv==(ved.fil+ved.alto-3))
				  cursor_fin();
			}
		}
		/* comprueba si est  en barra horizontal de desplazamiento */
		else if(r.fil==maxfil) {
			if(r.col==(mincol+12)) cursor_izq();
			else if(r.col==(maxcol-1)) cursor_der();
		}
		/* comprueba si est  dentro de la ventana de edici¢n */
		else if((r.fil>minfil) && (r.fil<maxfil) &&
		  (r.col>mincol) && (r.col<maxcol)) cursor_pos(r.fil-minfil-1,
		  r.col-mincol-1);

		if(boton==1) pausa(E_PAUSA1);
		else pausa(E_PAUSA2);
	}
	else if(accion==2) {
		fin_bloque=1;
	}
	else {
		switch(tecla >> 8) {
			case TCUR_IZQ :
				cursor_izq();
				break;
			case TCUR_DER :
				cursor_der();
				break;
			case TCUR_ABJ :
				cursor_abj();
				break;
			case TCUR_ARR :
				cursor_arr();
				break;
			case TPAG_ABJ :
				pagina_abj();
				break;
			case TPAG_ARR :
				pagina_arr();
				break;
			case TFIN :
				cursor_fin_lin();
				break;
			case TORG :
				cursor_inicio_lin();
				break;
			case TCTR_FIN :
				cursor_fin();
				break;
			case TCTR_ORG :
				cursor_inicio();
				break;
			case TSUP :             /* BORRAR BLOQUE */
				if(!borra_bloque(ed.lin_ib,ed.col_ib,
				  ed.lin_fb,ed.col_fb)) beep();
				if(ed.lin_ib<=ed.lin_fb) ed.lin=ed.lin_ib;
				else ed.lin=ed.lin_fb;
				if(ed.col_ib<=ed.col_fb) ed.col=ed.col_ib;
				else ed.col=ed.col_fb;
				cursor_pos(ed.filcur,ccursor);
				fin_bloque=1;
				break;
			case TCTR_INS :         /* COPIAR BLOQUE */
				if(!copia_bloque(ed.lin_ib,ed.col_ib,
				  ed.lin_fb,ed.col_fb)) beep();
				fin_bloque=1;
				break;
			case TCTR_SUP :         /* CORTAR BLOQUE */
				if(!copia_bloque(ed.lin_ib,ed.col_ib,
				  ed.lin_fb,ed.col_fb)) beep();
				else if(!borra_bloque(ed.lin_ib,ed.col_ib,
				  ed.lin_fb,ed.col_fb)) beep();
				if(ed.lin_ib<=ed.lin_fb) ed.lin=ed.lin_ib;
				else ed.lin=ed.lin_fb;
				if(ed.col_ib<=ed.col_fb) ed.col=ed.col_ib;
				else ed.col=ed.col_fb;
				cursor_pos(ed.filcur,ccursor);
				fin_bloque=1;
				break;
			case TESC :
				fin_bloque=1;
				break;
		}
	}
}

ed.bloque=0;
imprime_texto_ed();

/* espera a que suelte bot¢n 2 */
do {
	r_estado(&r);
} while(r.boton2);

}

/****************************************************************************
	COPIA_BLOQUE: copia un bloque de texto en un buffer de memoria.
	  Entrada:      'lini', 'coli' inicio del bloque
			'linf', 'colf' final del bloque
	  Salida:       1 si se pudo copiar, 0 si no
****************************************************************************/
int copia_bloque(int lini, int coli, int linf, int colf)
{
STC_BLOQUE *b;
char temp[E_MAXLNGLIN], *txt;
int i, nlin, ncol;

if((lini==linf) && (coli>colf)) {
	i=colf;
	colf=coli;
	coli=i;
}
else if(lini>linf) {
	i=linf;
	linf=lini;
	lini=i;
	i=colf;
	colf=coli;
	coli=i;
}

/* si hay un bloque lo borra */
if(bloque!=NULL) e_vacia_bloque();

for(nlin=lini; nlin<=linf; nlin++) {
	/* si es primera l¡nea coge texto a partir de columna de inicio */
	/* si no coger texto a partir de primera columna */
	if(nlin==lini) {
		ncol=coli;
		b=(STC_BLOQUE *)malloc(sizeof(STC_BLOQUE));
		if(b==NULL) return(0);
		b->lin=NULL;
		b->sgte=NULL;
		bloque=b;
	}
	else {
		ncol=0;
		b->sgte=(STC_BLOQUE *)malloc(sizeof(STC_BLOQUE));
		if(b->sgte==NULL) {
			e_vacia_bloque();
			return(0);
		}
		b=b->sgte;
		b->lin=NULL;
		b->sgte=NULL;
	}
	i=0;
	txt=lin[nlin]+ncol;
	while(*txt && (i<(E_MAXLNGLIN-1))) {
		/* si es £ltima l¡nea de bloque comprueba que */
		/* no sobrepase columna final */
		if((nlin==linf) && (ncol>colf)) break;
		temp[i]=*txt;
		i++;
		ncol++;
		txt++;
	}
	temp[i]='\0';

	/* guarda texto en buffer */
	b->lin=(char *)malloc(strlen(temp)+1);
	if(b->lin==NULL) {
		e_vacia_bloque();
		return(0);
	}
	strcpy(b->lin,temp);
}

return(1);
}

/****************************************************************************
	BORRA_BLOQUE: borra un bloque de texto.
	  Entrada:      'lini', 'coli' inicio del bloque
			'linf', 'colf' final del bloque
	  Salida:       1 si se pudo borrar, 0 si no
****************************************************************************/
int borra_bloque(int lini, int coli, int linf, int colf)
{
char *txt;
int i, nlin, lng;

if((lini==linf) && (coli>colf)) {
	i=colf;
	colf=coli;
	coli=i;
}
else if(lini>linf) {
	i=linf;
	linf=lini;
	lini=i;
	i=colf;
	colf=coli;
	coli=i;
}

/* indica texto modificado */
ed.modificado=1;

/* si l¡nea de inicio es igual a l¡nea final */
if(lini==linf) {
	lng=strlen(lin[lini]);
	if(colf>(lng-1)) colf=lng-1;
	for(i=coli; i<=colf; i++) {
		if(!borra_caracter(lini,coli)) return(0);
	}
}
else {
	for(nlin=lini; nlin<=linf; nlin++) {
		/* si es primera l¡nea borra texto desde de columna inicial */
		if(nlin==lini) {
			lng=strlen(lin[nlin]);
			for(i=coli; i<lng; i++) {
				if(!borra_caracter(nlin,coli)) return(0);
			}
		}
		/* si es la £ltima borra texto hasta columna final */
		else if(nlin==linf) {
			lng=strlen(lin[linf]);
			if(colf>(lng-1)) colf=lng-1;
			for(i=0; i<=colf; i++) {
				if(!borra_caracter(nlin,0)) return(0);
			}
		}
		/* en cualquier otro caso borra l¡nea entera */
		else {
			free(lin[nlin]);
			lin[nlin]=NULL;
		}
	}

	/* si borr¢ l¡neas intermedias, desplaza texto */
	nlin=linf-lini-1;

	/* si borr¢ l¡nea inicial completa la elimina tambi‚n */
	if(!*lin[lini]) {
		free(lin[lini]);
		lin[lini]=NULL;
		nlin++;
	}
	/* ¡dem l¡nea final */
	if(!*lin[linf]) {
		free(lin[linf]);
		lin[linf]=NULL;
		linf++;
		nlin++;
	}

	if(nlin) {
		for(i=linf; i<ed.numlin; i++) lin[i-nlin]=lin[i];
		ed.numlin-=nlin;
	}

	/* si se han borrado todas la l¡neas crea una */
	if(!ed.numlin) {
		lin[0]=(char *)malloc(1);
		*lin[0]='\0';
		ed.numlin++;
	}
}

return(1);
}

/****************************************************************************
	INSERTA_BLOQUE: inserta un bloque en el texto.
	  Entrada:      'nlin', 'col' posici¢n d¢nde insertar bloque
	  Salida:       1 si se pudo borrar, 0 si no
****************************************************************************/
int inserta_bloque(int nlin, int col)
{
STC_BLOQUE *b;
int nlin_bloque=0, lng, i, j;
char *txtblq, *txt, temp[E_MAXLNGLIN];

/* si no hay bloque, sale */
if(bloque==NULL) return(0);

/* calcula n£mero de l¡neas de bloque (sin contar primera) */
b=bloque;
while(b->sgte!=NULL) {
	b=b->sgte;
	nlin_bloque++;
}

/* si n£mero de l¡neas de bloque sobrepasa m ximo permitido, sale */
if((ed.numlin+nlin_bloque)>E_MAXNUMLIN) return(0);

/* indica texto modificado */
ed.modificado=1;

/* inserta primera l¡nea de bloque */
txtblq=bloque->lin;

/* calcula n£mero de caracteres de bloque que se pueden insertar */
lng=strlen(lin[nlin])+strlen(txtblq);
if(lng>(E_MAXLNGLIN-1)) lng=(E_MAXLNGLIN-1)-strlen(lin[nlin]);
else lng=strlen(txtblq);

strcpy(temp,lin[nlin]);
txt=lin[nlin]+col;
for(i=col; ((lng!=0) && *txtblq); i++, txtblq++, lng--) temp[i]=*txtblq;
for(; ((i<(E_MAXLNGLIN-1)) && *txt); i++, txt++) temp[i]=*txt;
temp[i]='\0';
if((txt=realloc(lin[nlin],strlen(temp)+1))!=NULL) {
	lin[nlin]=txt;
	strcpy(lin[nlin],temp);
}
nlin++;

/* inserta siguientes l¡neas de bloque */
b=bloque->sgte;
for(i=0; i<nlin_bloque; i++) {
	if((txt=(char *)malloc(strlen(b->lin)+1))==NULL) return(0);
	strcpy(txt,b->lin);
	/* desplaza l¡neas hacia abajo */
	for(j=ed.numlin; j>nlin; j--) lin[j]=lin[j-1];
	lin[nlin]=txt;
	nlin++;
	ed.numlin++;
	b=b->sgte;
}

return(1);
}

/****************************************************************************
	COGE_PALABRA_CURSOR: recoge la palabra sobre la que se encuentra
	  el cursor y la guarda en buffer 'palabra_cursor'.
****************************************************************************/
void coge_palabra_cursor(void)
{
char *ltxt;
int i;

/* puntero a posici¢n actual de cursor */
ltxt=lin[ed.lin]+ed.col;

/* busca hacia atr s hasta encontrar el l¡mite izquierdo de */
/* la palabra (car cter no alfanum‚rico) */
for(; (ltxt>lin[ed.lin]) && isalnum(*ltxt);  ltxt--);
ltxt++;

/* almacena palabra en buffer */
for(i=0; (i<(E_MAXLNGLIN-1)) && *ltxt && isalnum(*ltxt); i++)
  palabra_cursor[i]=*ltxt++;

palabra_cursor[i]='\0';

}

/****************************************************************************
	E_INICIALIZA: inicializa el editor.
	  Entrada:      'fich' nombre de fichero por defecto
			'fil', 'col' origen de la ventana de edici¢n
			'ancho', 'alto' tama¤o de la ventana de edici¢n
			'attr_princ' atributo principal de la ventana
			'attr_s1', 'attr_s2' atributos de sombreado
			'attr_blq' atributo para marcar bloque
			'attr_princc', 'attr_s1c', 'attr_s2c',
			'attr_teclac', 'attr_selc' atributos para cuadro de
			di logo
			'modo_ed' modo de edici¢n inicial:
				E_EDITA permite editar
				E_SOLOLECT no permite edici¢n
****************************************************************************/
void e_inicializa(char *fich, short fil, short col, short ancho, short alto,
  BYTE attr_princ, BYTE attr_s1, BYTE attr_s2, BYTE attr_blq, BYTE attr_princc,
  BYTE attr_s1c, BYTE attr_s2c, BYTE attr_teclac, BYTE attr_selc, int modo_ed)
{
int i;

/* crea la ventana de edici¢n */
v_crea(&ved,fil,col,ancho,alto,attr_princ,attr_s1,attr_s2,NULL);

/* inicializa punteros a l¡neas de texto */
for(i=0; i<E_MAXNUMLIN; i++) lin[i]=NULL;

/* crea la primera l¡nea de texto */
lin[0]=(char *)malloc(1);
*lin[0]='\0';

/* inicializa estado del editor */
ed.lin=0;
ed.col=0;
ed.filcur=0;
ed.colcur=0;
ed.numlin=1;
ed.modificado=0;
ed.modo_ed=modo_ed;
strcpy(ed.fich0,fich);
strupr(ed.fich0);
strcpy(ed.fich,ed.fich0);
ed.lin_ib=0;
ed.col_ib=0;
ed.lin_fb=0;
ed.col_fb=0;
ed.bloque=0;
ed.attr_blq=attr_blq;

/* crea cuadro de di logo para b£squedas */
c_crea_cuadro(&cbusca," Buscar ",CB_FIL,CB_COL,CB_ANCHO,CB_ALTO,attr_princc,
  attr_s1c,attr_s2c,attr_teclac,attr_selc);
cadena_busca[0]='\0';
cb_input.cadena=cadena_busca;
cb_input.longitud=E_MAXLNGLIN-1;
c_crea_elemento(&cbusca,0,0,7,CB_ANCHO-9,3,attr_princc,attr_s2c,attr_s1c,
  C_ELEM_INPUT,&cb_input);
c_crea_elemento(&cbusca,1,3,CB_ANCHO-19,8,3,attr_princc,attr_s1c,attr_s2c,
  C_ELEM_BOTON,&cb_vale);
c_crea_elemento(&cbusca,2,3,CB_ANCHO-11,9,3,attr_princc,attr_s1c,attr_s2c,
  C_ELEM_BOTON,&cb_salir);
c_crea_elemento(&cbusca,3,4,1,0,0,attr_princc,attr_s1c,attr_s2c,
  C_ELEM_CHECK,&cb_chkmay);

/* inicializa puntero a buffer para bloque de texto */
bloque=NULL;

/* inicializa buffer para palabra sobre la que est  cursor */
*palabra_cursor='\0';

}

/****************************************************************************
	E_BORRA_TEXTO: borra todo el texto del editor, liberando la
	  memoria ocupada e inicializa variables internas.
****************************************************************************/
void e_borra_texto(void)
{
int i;

i=0;
while((lin[i]!=NULL) && (i<ed.numlin)) {
	free(lin[i]);
	lin[i]=NULL;
	i++;
}
_heapmin();

/* inicializa variables internas */
ed.lin=0;
ed.col=0;
ed.filcur=0;
ed.colcur=0;
ed.numlin=0;
ed.modificado=0;
strcpy(ed.fich,ed.fich0);

}

/****************************************************************************
	E_EDITOR: rutina de control principal del editor.
	  Salida:       0 si puls¢ ESCAPE
			1 si puls¢ F1 (ayuda)
			2 si puls¢ Shift+F1 (ayuda palabra del cursor)
****************************************************************************/
int e_editor(void)
{
STC_RATON r;
int fin_edicion=0, col, lin_ant, lin_act, accion, boton=0;
short minfil, mincol, maxfil, maxcol, posv;
unsigned tecla, shift;
char car;

/* si rat¢n no est  inicializado, lo inicializa */
if(!r_puntero(R_MUESTRA)) r_inicializa();

v_borra(&ved);

/* si no hay l¡neas, crea la primera */
if(!ed.numlin) {
	lin[0]=(char *)malloc(1);
	*lin[0]='\0';
	ed.numlin++;
}

pon_cursor(ed.filcur+ved.fil+1,ed.colcur+ved.col+1);
imprime_texto_ed();

lin_ant=ed.lin;
lin_act=ed.lin;

while(!fin_edicion) {
	/* comprueba si ha cambiado de l¡nea y est  editando */
	if((lin_act!=lin_ant) && (ed.modo_ed==E_EDITA)) {
		if(lin_ant<ed.numlin) suprime_espacios_fin(lin_ant);
	}
	lin_ant=lin_act;

	pon_cursor(ed.filcur+ved.fil+1,ed.colcur+ved.col+1);
	imprime_linea_ed(ed.lin);

	imprime_info();

	do {
		accion=0;
		r_estado(&r);
		if(r.boton1) {
			accion=1;
			if(boton<2) boton++;
		}
		else if(r.boton2) {
			boton=0;
			accion=2;
		}
		else {
			boton=0;
			tecla=_bios_keybrd(_KEYBRD_READY);
			if(tecla) {
				_bios_keybrd(_KEYBRD_READ);
				accion=3;
			}
		}
	} while(!accion);

	if(accion==1) {
		/* calcula l¡mites de la ventana de edici¢n posici¢n */
		/* del cuadro de desplazamiento */
		minfil=ved.fil;
		maxfil=ved.fil+ved.alto-1;
		mincol=ved.col;
		maxcol=ved.col+ved.ancho-1;

		if(ed.numlin==1) posv=0;
		else posv=(short)(((unsigned long)(ved.alto-5)*ed.lin)/
		  (ed.numlin-1));
		posv+=ved.fil+2;

		/* si est  fuera de la ventana de edici¢n, sale */
		if((r.fil<minfil) || (r.fil>maxfil) || (r.col<mincol) ||
		  (r.col>maxcol)) fin_edicion=1;
		/* comprueba si est  en barra vertical de desplazamiento */
		else if(r.col==maxcol) {
			if(r.fil==(minfil+1)) cursor_arr();
			else if(r.fil==(maxfil-1)) cursor_abj();
			else if(r.fil==minfil) cursor_inicio();
			else if(r.fil==maxfil) cursor_fin();
			else if(r.fil<posv) pagina_arr();
			else if(r.fil>posv) pagina_abj();
			else if(r.fil==posv) {
				if(posv==(ved.fil+2)) cursor_inicio();
				else if(posv==(ved.fil+ved.alto-3))
				  cursor_fin();
			}
		}
		/* comprueba si est  en barra horizontal de desplazamiento */
		else if(r.fil==maxfil) {
			if(r.col==(mincol+12)) cursor_izq();
			else if(r.col==(maxcol-1)) cursor_der();
		}
		/* comprueba si est  dentro de la ventana de edici¢n */
		else if((r.fil>minfil) && (r.fil<maxfil) && (r.col>mincol) &&
		  (r.col<maxcol)) cursor_pos(r.fil-minfil-1,r.col-mincol-1);

		if(boton==1) pausa(E_PAUSA1);
		else pausa(E_PAUSA2);
	}
	else if(accion==2) {
		/* calcula l¡mites de la ventana de edici¢n posici¢n */
		/* del cuadro de desplazamiento */
		minfil=ved.fil;
		maxfil=ved.fil+ved.alto-1;
		mincol=ved.col;
		maxcol=ved.col+ved.ancho-1;

		/* comprueba si est  dentro de la ventana de edici¢n */
		if((r.fil>minfil) && (r.fil<maxfil) && (r.col>mincol) &&
		  (r.col<maxcol)) {
			cursor_pos(r.fil-minfil-1,r.col-mincol-1);
			pon_cursor(ed.filcur+ved.fil+1,ed.colcur+ved.col+1);
			/* espera a que suelte bot¢n 2 */
			do {
				r_estado(&r);
			} while(r.boton2);
			if(ed.modo_ed==E_SOLOLECT) beep();
			else marca_bloque();
		}
	}
	else {
		shift=_bios_keybrd(_KEYBRD_SHIFTSTATUS);

		/* comprueba si est  pulsado alg£n SHIFT */
		if(shift & 0x0003) {
			switch(tecla >> 8) {
				case TINS :     /* INSERTAR BLOQUE */
					if(!inserta_bloque(ed.lin,ed.col))
					  beep();
					imprime_texto_ed();
					break;
				case TSF1 :     /* AYUDA PALABRA CURSOR */
					coge_palabra_cursor();
					return(2);
			}
		}
		else switch(tecla >> 8) {
			case TCUR_IZQ :
				cursor_izq();
				break;
			case TCUR_DER :
				cursor_der();
				break;
			case TCUR_ABJ :
				cursor_abj();
				break;
			case TCUR_ARR :
				cursor_arr();
				break;
			case TPAG_ABJ :
				pagina_abj();
				break;
			case TPAG_ARR :
				pagina_arr();
				break;
			case TFIN :
				cursor_fin_lin();
				break;
			case TORG :
				cursor_inicio_lin();
				break;
			case TCTR_FIN :
				cursor_fin();
				break;
			case TCTR_ORG :
				cursor_inicio();
				break;
			case TDEL :
				if(ed.modo_ed==E_SOLOLECT) {
					beep();
					break;
				}
				/* si no est  al inicio de la l¡nea */
				if(ed.col>0) {
					if(borra_caracter(ed.lin,ed.col-1))
					  cursor_izq();
				}
				else if(ed.lin>0) {
					col=strlen(lin[ed.lin-1]);
					if(copia_linea(ed.lin-1,ed.lin)) {
						if(ed.lin==(ed.numlin-1))
						  borra_linea(ed.lin);
						else {
							borra_linea(ed.lin);
							cursor_arr();
						}
						ed.col=col;
						ed.colcur=col;
						if(ed.colcur>(ved.ancho-3))
						  ed.colcur=ved.ancho-3;
						pon_cursor(ed.filcur+ved.fil+
						  1,ed.colcur+ved.col+1);
						imprime_texto_ed();
					}
					else beep();
				}
				/* elimina c¢digo ASCII */
				tecla &= 0xff00;
				break;
			case TSUP :
				if(ed.modo_ed==E_SOLOLECT) {
					beep();
					break;
				}
				col=strlen(lin[ed.lin]);
				if(ed.col>=col) {
					if(copia_linea(ed.lin,ed.lin+1)) {
						borra_linea(ed.lin+1);
						pon_cursor(ed.filcur+ved.fil+
						  1,ed.colcur+ved.col+1);
						imprime_texto_ed();
					}
					else beep();
				}
				else borra_caracter(ed.lin,ed.col);
				break;
			case TESC :
				fin_edicion=1;
				/* elimina c¢digo ASCII */
				tecla &= 0xff00;
				break;
			case TF1 :              /* AYUDA */
				return(1);
			case TF2 :              /* BUSCAR */
				e_inicia_busqueda();
				break;
			case TF3 :              /* CONTINUAR BUSQUEDA */
				e_continua_busqueda();
				break;
			case TF4 :              /* BORRAR LINEA */
				if(ed.modo_ed==E_SOLOLECT) beep();
				else {
					borra_linea(ed.lin);
					cursor_inicio_lin();
				}
				break;
			case TF5 :              /* MARCAR BLOQUE */
				if(ed.modo_ed==E_SOLOLECT) beep();
				else marca_bloque();
				break;
		}

		/* mira si tecle¢ un car cter v lido */
		car=(char)(tecla & 0x00ff);
		switch(car) {
			case 0 :
				break;
			case RETURN :
				if(ed.modo_ed==E_SOLOLECT) {
					beep();
					break;
				}
				if(inserta_linea()) {
					cursor_abj();
					cursor_inicio_lin();
				}
				else beep();
				break;
			case TAB :
				if(ed.modo_ed==E_SOLOLECT) beep();
				else do {
					if(inserta_caracter(' ')) cursor_der();
					else break;
				} while((ed.col % 8)!=0);
				break;
			default :
				if(ed.modo_ed==E_SOLOLECT) {
					beep();
					break;
				}
				if(inserta_caracter(car)) cursor_der();
				else beep();
				break;
		}
	}

	lin_act=ed.lin;

}

return(0);
}

/****************************************************************************
	E_DIBUJA_EDITOR: dibuja la ventana de edici¢n.
****************************************************************************/
void e_dibuja_editor(void)
{

v_borra(&ved);
imprime_texto_ed();
imprime_info();

}

/****************************************************************************
	E_CARGA_TEXTO: carga un fichero de texto en el editor.
	  Entrada:      'fich' nombre del fichero a cargar
	  Salida:       1 si se carg¢ correctamente, 0 si hubo errores
****************************************************************************/
int e_carga_texto(char *fich)
{
FILE *f_texto;
char temp[E_MAXLNGLIN], *txt;

if((f_texto=fopen(fich,"rt"))==NULL) return(0);

/* borra texto del editor */
e_borra_texto();

/* indica texto nuevo */
ed.modificado=0;

/* copia nombre de fichero */
strcpy(ed.fich,fich);
strupr(ed.fich);

/* lee l¡neas del fichero */
while(ed.numlin<E_MAXNUMLIN) {
	if(fgets(temp,E_MAXLNGLIN,f_texto)==NULL) {
		if(feof(f_texto)) break;
		else {
			fclose(f_texto);
			return(0);
		}
	}

	/* expande tabulaciones y suprime '\n' final */
	ajusta_linea(temp);

	/* inserta l¡nea en el editor */
	txt=(char *)malloc(strlen(temp)+1);
	if(txt==NULL) {
		e_borra_texto();
		return(0);
	}
	strcpy(txt,temp);
	lin[ed.numlin]=txt;

	ed.numlin++;
}

fclose(f_texto);

/* si fichero estaba totalmente vac¡o, crea una l¡nea al menos */
if(!ed.numlin) {
	lin[0]=(char *)malloc(1);
	*lin[0]='\0';
	ed.numlin=1;
}

return(1);
}

/****************************************************************************
	E_GRABA_TEXTO: graba el texto del editor en un fichero.
	  Entrada:      'fich' nombre del fichero, NULL si se quiere usar el
			mismo con el que se carg¢.
	  Salida:       1 si se grab¢ correctamente, 0 si hubo errores
****************************************************************************/
int e_graba_texto(char *fich)
{
FILE *f_texto;
int i, lng;
char temp[E_MAXLNGLIN+1], *txt;

if(fich!=NULL) {
	strcpy(ed.fich,fich);
	strupr(ed.fich);
}

if((f_texto=fopen(ed.fich,"wt"))==NULL) return(0);

for(i=0; i<ed.numlin; i++) {
	/* a¤ade '\n' al final */
	strcpy(temp,lin[i]);
	lng=strlen(temp);
	temp[lng]='\n';
	temp[lng+1]='\0';

	/* graba l¡nea */
	if(fputs(temp,f_texto)!=0) {
		fclose(f_texto);
		return(0);
	}
}

fclose(f_texto);

/* indica que fichero ya ha sido grabado */
ed.modificado=0;

return(1);
}

/****************************************************************************
	E_NOMBRE_FICHERO: devuelve el nombre del fichero cargado en el
	  editor.
	  Entrada:      'nf' puntero a buffer donde se dejar  el
			nombre del fichero (el buffer debe tener capacidad
			para _MAX_PATH caracteres)
****************************************************************************/
void e_nombre_fichero(char *nf)
{

strcpy(nf,ed.fich);

}

/****************************************************************************
	E_INICIA_BUSQUEDA: inicia la b£squeda de una cadena.
	  Salida:       1 si encontr¢ cadena, 0 si no
****************************************************************************/
int e_inicia_busqueda(void)
{
int op;

c_abre(&cbusca);
op=c_gestiona(&cbusca);
c_cierra(&cbusca);

/* si sali¢ del cuadro */
if((op==-1) || (op==2)) return(0);

if(busca_cadena()==0) {
	beep();
	imprime_texto_ed();
	return(0);
}

imprime_texto_ed();

return(1);
}

/****************************************************************************
	E_CONTINUA_BUSQUEDA: continua la b£squeda de una cadena.
	  Salida:       1 si encontr¢ cadena, 0 si no
****************************************************************************/
int e_continua_busqueda(void)
{
int op, lng, linc, colc;

/* si cadena vac¡a, la pide */
if(*cadena_busca=='\0') {
	c_abre(&cbusca);
	op=c_gestiona(&cbusca);
	c_cierra(&cbusca);

	/* si sali¢ del cuadro */
	if((op==-1) || (op==2)) return(0);
}

/* guarda posici¢n del cursor */
linc=ed.lin;
colc=ed.col;

/* pasa al siguiente car cter; si es el £ltimo, sale */
lng=strlen(lin[ed.lin]);
if(ed.col<(lng-1)) ed.col++;
else if(ed.lin<(ed.numlin-1)) ed.lin++;
else {
	beep();
	return(0);
}

if(busca_cadena()==0) {
	/* restaura posici¢n del cursor */
	ed.lin=linc;
	ed.col=colc;
	beep();
	imprime_texto_ed();
	return(0);
}

imprime_texto_ed();

return(1);
}

/****************************************************************************
	E_MODIFICADO: comprueba si el texto del editor ha sido modificado.
	  Salida:       1 si se modific¢ el texto del editor, 0 si no
****************************************************************************/
int e_modificado(void)
{

return(ed.modificado);
}

/****************************************************************************
	E_CAMBIA_MODO: cambia modo de edici¢n.
	  Entrada:      'modo_ed' modo de edici¢n, E_EDITA permite editar
			texto, E_SOLOLECT s¢lo permite ver
****************************************************************************/
void e_cambia_modo(int modo_ed)
{

ed.modo_ed=modo_ed;

}

/****************************************************************************
	E_CARGA_TEXTOX: carga texto colocado en mitad de un fichero.
	  Entrada:      'fich' nombre del fichero
			'desplz' posici¢n de inicio del texto respecto al
			origen del fichero
	  Salida:       1 si se carg¢ correctamente, 0 si hubo errores
****************************************************************************/
int e_carga_textox(char *fich, long desplz)
{
FILE *f_texto;
char c, temp[E_MAXLNGLIN], *txt;
int i;

if((f_texto=fopen(fich,"rb"))==NULL) return(0);

/* borra texto del editor */
e_borra_texto();

/* indica texto nuevo */
ed.modificado=0;

/* copia nombre de fichero */
strcpy(ed.fich,fich);
strupr(ed.fich);

fseek(f_texto,desplz,SEEK_SET);

/* lee l¡neas del fichero */
while(ed.numlin<E_MAXNUMLIN) {
	i=0;
	do {
		c=(char)fgetc(f_texto);
		if(feof(f_texto) || ferror(f_texto)) break;
		temp[i++]=c;
		if(i>(E_MAXLNGLIN-2)) break;
	} while(c!='\n');
	/* debe haber cogido al menos dos caracteres, si no, l¡nea de */
	/* texto no es v lida */
	if(i>=2) {
		temp[i]='\0';
		if(temp[i-1]=='\n') {
			if(temp[i-2]=='\r') {
				temp[i-2]='\n';
				temp[i-1]='\0';
			}
		}
	}
	else temp[0]='\0';
	if(feof(f_texto)) break;
	else if(ferror(f_texto)) {
		fclose(f_texto);
		return(0);
	}

	/* expande tabulaciones y suprime '\n' final */
	ajusta_linea(temp);

	/* inserta l¡nea en el editor */
	txt=(char *)malloc(strlen(temp)+1);
	if(txt==NULL) {
		e_borra_texto();
		return(0);
	}
	strcpy(txt,temp);
	lin[ed.numlin]=txt;

	ed.numlin++;
}

fclose(f_texto);

/* si fichero estaba totalmente vac¡o, crea una l¡nea al menos */
if(!ed.numlin) {
	lin[0]=(char *)malloc(1);
	*lin[0]='\0';
	ed.numlin=1;
}

return(1);
}

/****************************************************************************
	E_VACIA_BLOQUE: libera memoria ocupada por bloque de texto.
****************************************************************************/
void e_vacia_bloque(void)
{
STC_BLOQUE *b0, *b1;

if(bloque==NULL) return;

b0=bloque;

do {
	b1=b0->sgte;
	free(b0->lin);
	free(b0);
	b0=b1;
} while(b0!=NULL);
_heapmin();

bloque=NULL;

}

/****************************************************************************
	E_GRABA_BLOQUE: graba el bloque en un fichero.
	  Entrada:      'nfich' nombre del fichero
	  Salida:       1 si se pudo grabar, 0 si no
****************************************************************************/
int e_graba_bloque(char *nfich)
{
FILE *f_bloque;
STC_BLOQUE *b;
char temp[E_MAXLNGLIN+1];
int i;

if((f_bloque=fopen(nfich,"wt"))==NULL) return(0);

b=bloque;
while(b!=NULL) {
	/* copia l¡nea de bloque a buffer y a¤ade '\n' final */
	strcpy(temp,b->lin);
	i=strlen(temp);
	temp[i]='\n';
	temp[i+1]='\0';

	/* graba l¡nea */
	if(fputs(temp,f_bloque)!=0) {
		fclose(f_bloque);
		return(0);
	}

	/* siguiente l¡nea de bloque */
	b=b->sgte;
}

fclose(f_bloque);

return(1);
}

/****************************************************************************
	E_INSERTA_BLOQUE: inserta bloque en posici¢n actual del cursor.
	  Salida:       1 si se pudo insertar, 0 si no
****************************************************************************/
int e_inserta_bloque(void)
{

if(!inserta_bloque(ed.lin,ed.col)) {
	beep();
	imprime_texto_ed();
	return(0);
}

imprime_texto_ed();

return(1);
}

/****************************************************************************
	E_PON_CURSOR: cambia la posici¢n dentro del texto del cursor.
	  Entrada:      'lin', 'col' nueva posici¢n del cursor
****************************************************************************/
void e_pon_cursor(int lin, int col)
{

ed.lin=lin-1;
ed.col=col;
ed.filcur=0;
ed.colcur=0;

}

/****************************************************************************
	E_PALABRA_CURSOR: devuelve la palabra sobre la que se encontraba
	  el cursor la £ltima vez que se puls¢ la tecla de ayuda sobre
	  palabra.
	  Salida:       puntero a buffer que contiene la palabra
****************************************************************************/
char *e_palabra_cursor(void)
{

return(palabra_cursor);
}
