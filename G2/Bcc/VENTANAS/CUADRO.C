/****************************************************************************
				 CUADRO.C

	Biblioteca de funciones para gestionar cuadros de di logo.

			     (c)1993 JSJ Soft Ltd.

	Las siguientes funciones son p£blicas:
		- c_crea_cuadro: crea e inicializa un cuadro de di logo
		- c_crea_elemento: crea o modifica un elemento de un cuadro
		- c_dibuja_elemento: dibuja un elemento de un cuadro
		- c_abre: abre un cuadro de di logo
		- c_cierra: cierra un cuadro de di logo
		- c_lee_input: permite la introducci¢n de una cadena de
		    caracteres en una ventana
		- c_accion: env¡a una acci¢n a un cuadro de di logo
		- c_gestiona: gestiona un cuadro de di logo
		- c_elem_input: gestiona un elemento de entrada
		- c_elem_lista: gestiona un elemento de lista
		- c_mete_en_lista: inserta una cadena al final de una lista
		- c_borra_lista: elimina una lista y libera memoria
		- c_selecc_ficheros: gestiona cuadro de selecci¢n de ficheros

	Las siguientes estructuras est n definidas en MENU.H:
		STC_ELEM: elemento de un cuadro de di logo
		STC_CUADRO: cuadro de di logo
		STC_ACCION_C: acci¢n para un cuadro de di logo
		STC_ELEM_BOTON: elemento bot¢n
		STC_ELEM_INPUT: elemento de entrada
		STC_ELEM_LISTA: elemento de lista
		STC_ELEM_CHECK: elemento de caja de comprobaci¢n
****************************************************************************/

#include <stdlib.h>
#include <stddef.h>
#include <bios.h>
#include <time.h>
#include <alloc.h>
#include <string.h>
#include <dos.h>
#include <dir.h>
#include "ventana.h"
#include "raton.h"
#include "cuadro.h"

/*** Variables globales internas ***/
static char *Borde1_Elem="ÚÄ¿³³ÀÄÙ";
static char *Borde2_Elem="ÉÍ»ººÈÍ¼";

/*** Prototipos de funciones internas ***/
static int int24_hnd(int errval, int ax, int bp, int si);
static char mayuscula(char c);
static void may_str(char *s);
static void coge_pos_cursor(short *fil, short *col);
static void pon_cursor(short fil, short col);
static void imprime_txt_elem(STC_VENTANA *v, short fil, short col, char *txt,
 BYTE colort, int modo);
static void resalta_elemento(STC_CUADRO *cuad);
static int siguiente_elemento(STC_CUADRO *c);
static int anterior_elemento(STC_CUADRO *c);
static void imprime_lista(STC_VENTANA *v, STC_LISTA *l, int elemento,
  int elemento_sel, BYTE attr_sel);
static void busca_elem_selecc(STC_ELEM_LISTA *e);
static void dibuja_flechas_vert(STC_VENTANA *v, int nelem, int elem);
static int lista_ficheros(STC_ELEM_LISTA *lista, char *ruta_masc);
static void pon_barra_dir(char *dir);
static void inicializa_elem_boton(STC_ELEM_BOTON *e);
static void inicializa_elem_input(STC_ELEM_INPUT *e);
static void inicializa_elem_lista(STC_ELEM_LISTA *e);
static void inicializa_elem_check(STC_ELEM_CHECK *e);

#pragma warn -par
/****************************************************************************
	INT24_HND: rutina de manejo de errores cr¡ticos de hardware.
****************************************************************************/
int int24_hnd(int errval, int ax, int bp, int si)
{

hardretn(2);

return(2);
}
#pragma warn +par

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
	MAY_STR: convierte una cadena en may£scula.
	  Entrada:      's' puntero a cadena
****************************************************************************/
void may_str(char *s)
{

while(*s) {
	*s=mayuscula(*s);
	s++;
}

}

/****************************************************************************
	COGE_POS_CURSOR: devuelve la posici¢n del cursor.
	  Entrada:      'fil', 'col' punteros a variables donde se
			devolver  la posici¢n del cursor
	  Salida:       posici¢n del cursor en las variables a las que
			apuntan 'fil' y 'col'
****************************************************************************/
void coge_pos_cursor(short *fil, short *col)
{
short fila=0, columna=0;

asm {
	mov ah,03h              // funci¢n buscar posici¢n del cursor
	mov bh,0                // supone p gina 0
	int 10h
	mov byte ptr fila,dh
	mov byte ptr columna,dl
}

*fil=fila;
*col=columna;

}

/****************************************************************************
	PON_CURSOR: coloca el cursor en una posici¢n de pantalla.
	  Entrada:      'fil', 'col' fila y columna del cursor
****************************************************************************/
void pon_cursor(short fil, short col)
{

asm {
	mov ah,02h              // funci¢n definir posici¢n del cursor
	mov bh,0                // supone p gina 0
	mov dh,byte ptr fil     // DH = fila del cursor
	mov dl,byte ptr col     // DL = columna del cursor
	int 10h
}

}

/****************************************************************************
	IMPRIME_TXT_ELEM: imprime el texto de un elemento, resaltando la
	  tecla asociada.
	  Entrada:      'v' puntero a ventana de elemento
			'fil', 'col' posici¢n del texto
			'txt' texto a imprimir
			'colort' color para tecla de activaci¢n
			'modo' modo de impresi¢n; 0 no se tiene en cuenta
			C_CARTECLA, 1 si
****************************************************************************/
void imprime_txt_elem(STC_VENTANA *v, short fil, short col, char *txt,
 BYTE colort, int modo)
{

r_puntero(R_OCULTA);

while(*txt) {
	if(col<(v->col+v->ancho-1)) {
		/* si es tecla asociada, la imprime en otro color */
		if((*txt==C_CARTECLA) && modo) {
			txt++;
			v_impcar(fil,col,*txt,colort);
		}
		else v_impcar(fil,col,*txt,v->attr_princ);
	}

	txt++;
	col++;
}

r_puntero(R_MUESTRA);

}

/****************************************************************************
	RESALTA_ELEMENTO: resalta el elemento actual de un cuadro de
	  di logo.
	  Entrada:      'cuad' puntero a estructura de cuadro
****************************************************************************/
void resalta_elemento(STC_CUADRO *cuad)
{
int i;

/* pone borde normal a todos los elementos */
for(i=0; i<C_MAXELEM; i++) {
	v_borde(&cuad->elem[i].v,Borde1_Elem);
	c_dibuja_elemento(cuad,i);
}

/* resalta el elemento actual */
v_borde(&cuad->elem[cuad->elemento].v,Borde2_Elem);
c_dibuja_elemento(cuad,cuad->elemento);

}

/****************************************************************************
	SIGUIENTE_ELEMENTO: selecciona el siguiente elemento v lido de un
	  cuadro de di logo.
	  Entrada:      'c' puntero a estructura de cuadro
	  Salida:       elemento seleccionado
****************************************************************************/
int siguiente_elemento(STC_CUADRO *c)
{
int i;

i=c->elemento;
do {
	i++;
	if(i>=C_MAXELEM) i=0;
} while((c->elem[i].tipo==C_ELEM_NULO) || (c->elem[i].tipo==C_ELEM_TEXTO));
c->elemento=i;

return(i);
}

/****************************************************************************
	ANTERIOR_ELEMENTO: selecciona el elemento anterior v lido de un
	  cuadro de di logo.
	  Entrada:      'cuad' puntero a estructura de cuadro
	  Salida:       elemento seleccionado
****************************************************************************/
int anterior_elemento(STC_CUADRO *c)
{
int i;

i=c->elemento;
do {
	i--;
	if(i<0) i=C_MAXELEM-1;
} while((c->elem[i].tipo==C_ELEM_NULO) || (c->elem[i].tipo==C_ELEM_TEXTO));
c->elemento=i;

return(i);
}

/****************************************************************************
	IMPRIME_LISTA: imprime una lista de cadenas de un elemento de
	  lista.
	  Entrada:      'v' ventana donde se imprimir  la lista
			'l' puntero a primer elemento de la lista
			'elemento' n£mero de orden del primer elemento a
			imprimir
			'elemento_sel' n£mero de orden del elemento
			seleccionado
			'texto' texto de encabezamiento
			'attr_sel' atributo del elemento seleccionado
****************************************************************************/
void imprime_lista(STC_VENTANA *v, STC_LISTA *l, int elemento,
  int elemento_sel, BYTE attr_sel)
{
STC_LISTA *elem;
int i, j;

/* si lista vac¡a, sale */
if(l==NULL) return;

r_puntero(R_OCULTA);

/* posiciona el puntero en el primer elemento a imprimir */
elem=l;
for(i=0; i<elemento; i++) {
	if(elem->sgte==NULL) break;
	else elem=elem->sgte;
}

v_pon_cursor(v,0,0);

/* imprime hasta el £ltimo elemento o hasta que se llene la ventana */
j=0;
while((elem!=NULL) && (j<(v->alto-2))) {
	/* si es elemento seleccionado lo imprime resaltado */
	if((i+j)==elemento_sel) {
		v_color(v,attr_sel);
		v_impcad(v,elem->cadena,V_RELLENA);
		v_color(v,v->attr_princ);
	}
	else v_impcad(v,elem->cadena,V_RELLENA);
	v->colc=0;
	v->filc++;
	elem=elem->sgte;
	j++;
}

/* rellena resto de la ventana con l¡neas en blanco */
for(; j<(v->alto-2); j++) {
	v_impcad(v,"",V_RELLENA);
	v->colc=0;
	v->filc++;
}

r_puntero(R_MUESTRA);

}

/****************************************************************************
	BUSCA_ELEM_SELECC: busca elemento seleccionado de una lista.
	  Entrada:      'e' puntero a lista
****************************************************************************/
void busca_elem_selecc(STC_ELEM_LISTA *e)
{
STC_LISTA *elem;
int i;

elem=e->elemento;
for(i=0; i<e->elemento_sel; i++) elem=elem->sgte;
e->selecc=elem->cadena;

}

/****************************************************************************
	DIBUJA_FLECHAS_VERT: dibuja flechas de direcci¢n verticales en
	  en borde derecho de una ventana.
	  Entrada:      'v' puntero a ventana
			'nelem' n£mero de elementos
			'elem' elemento seleccionado
****************************************************************************/
void dibuja_flechas_vert(STC_VENTANA *v, int nelem, int elem)
{
short i, col, lng, pos_v;

r_puntero(R_OCULTA);

col=v->col+v->ancho-1;
v_impcar(v->fil+1,col,C_FLECHARR,v->attr_s2);
v_impcar(v->fil+v->alto-2,col,C_FLECHABJ,v->attr_s2);

for(i=v->fil+2; i<(v->fil+v->alto-2); i++) {
	v_impcar(i,col,C_CARRELL1,v->attr_s2);
}

lng=v->alto-4;
if(nelem==1) pos_v=0;
else pos_v=(short)(((unsigned long)(lng-1)*elem)/(nelem-1));
pos_v+=v->fil+2;
v_impcar(pos_v,col,C_CARRELL2,v->attr_s2);

r_puntero(R_MUESTRA);

}

/****************************************************************************
	LISTA_FICHEROS: crea una lista con los ficheros y subdirectorios de
	  un directorio; si la lista estaba llena, la vac¡a.
	  Entrada:      'lista' puntero a elemento de lista
			'ruta_masc' ruta al directorio + m scara de
			b£squeda de ficheros
	  Salida:       1 si pudo crear la lista, 0 si no pudo crear lista
			completa
****************************************************************************/
int lista_ficheros(STC_ELEM_LISTA *lista, char *ruta_masc)
{
void interrupt (*int24_old)();
struct ffblk find;
char disq[MAXDRIVE], direct[MAXDIR], fich[MAXFILE], ext[MAXEXT],
  nfich[MAXPATH], dir[15];
int i;
unsigned encontrado;

/* instala 'handler' de errores cr¡ticos */
int24_old=getvect(0x24);
harderr(int24_hnd);

/* separa ruta de m scara */
fnsplit(ruta_masc,disq,direct,fich,ext);

/* vac¡a la lista */
c_borra_lista(lista);

/* crea ruta + "*.*" */
strcpy(nfich,disq);
strcat(nfich,direct);
strcat(nfich,"*.*");

/* busca primero subdirectorios */
encontrado=findfirst(nfich,&find,FA_DIREC);

while(!encontrado) {
	if(find.ff_attrib & FA_DIREC) {
		/* a¤ade caracteres indicadores de directorio */
		dir[0]=C_CHRDIR1;
		strcpy(&dir[1],find.ff_name);
		i=strlen(dir);
		dir[i]=C_CHRDIR2;
		dir[i+1]='\0';

		/* mete nombre de subdirectorio en lista */
		if(c_mete_en_lista(lista,dir)==0) {
			/* restaura 'handler' de errores cr¡ticos */
			setvect(0x24,int24_old);
			return(0);
		}
	}

	/* busca siguiente */
	encontrado=findnext(&find);
}

/* ahora busca ficheros */
encontrado=findfirst(ruta_masc,&find,0);

while(!encontrado) {
	/* mete nombre de fichero en lista */
	if(c_mete_en_lista(lista,find.ff_name)==0) {
		/* restaura 'handler' de errores cr¡ticos */
		setvect(0x24,int24_old);
		return(0);
	}

	/* busca siguiente */
	encontrado=findnext(&find);
}

/* restaura 'handler' de errores cr¡ticos */
setvect(0x24,int24_old);

return(1);
}

/****************************************************************************
	PON_BARRA_DIR: a¤ade barra directorio ('\') al final de una cadena
	  si no la tiene. La cadena debe tener espacio para a¤adir el
	  car cter.
	  Entrada:      'dir' puntero a cadena
****************************************************************************/
void pon_barra_dir(char *dir)
{
int i;

i=strlen(dir);
if(dir[i-1]!='\\') strcat(dir,"\\");

}

/****************************************************************************
	INICIALIZA_ELEM_BOTON: inicializa un elemento de tipo bot¢n.
	  Entrada:      'e' puntero a informaci¢n de elemento
****************************************************************************/
void inicializa_elem_boton(STC_ELEM_BOTON *e)
{
char *txt;

e->tecla=0;
for(txt=e->cadena; *txt; txt++) {
	if(*txt==C_CARTECLA) {
		e->tecla=mayuscula(*(txt+1));
		break;
	}
}

}

/****************************************************************************
	INICIALIZA_ELEM_INPUT: inicializa un elemento de entrada.
	  Entrada:      'e' puntero a informaci¢n de elemento
****************************************************************************/
void inicializa_elem_input(STC_ELEM_INPUT *e)
{
char *txt;

e->tecla=0;
for(txt=e->texto; *txt; txt++) {
	if(*txt==C_CARTECLA) {
		e->tecla=mayuscula(*(txt+1));
		break;
	}
}

}

/****************************************************************************
	INICIALIZA_ELEM_LISTA: inicializa un elemento de tipo lista.
	  Entrada:      'e' puntero a informaci¢n de elemento
****************************************************************************/
void inicializa_elem_lista(STC_ELEM_LISTA *e)
{
char *txt;

e->tecla=0;
for(txt=e->texto; *txt; txt++) {
	if(*txt==C_CARTECLA) {
		e->tecla=mayuscula(*(txt+1));
		break;
	}
}

e->elemento=NULL;
e->selecc=NULL;
e->num_elementos=0;
e->elemento_pr=0;
e->elemento_sel=0;

}

/****************************************************************************
	INICIALIZA_ELEM_CHECK: inicializa un elemento de caja de
	  comprobaci¢n.
	  Entrada:      'e' puntero a informaci¢n de elemento
****************************************************************************/
void inicializa_elem_check(STC_ELEM_CHECK *e)
{
char *txt;

e->tecla=0;
for(txt=e->texto; *txt; txt++) {
	if(*txt==C_CARTECLA) {
		e->tecla=mayuscula(*(txt+1));
		break;
	}
}

e->estado=0;

}

/****************************************************************************
	C_CREA_CUADRO: crea un cuadro de di logo.
	  Entrada:      'cuad' puntero a estructura del cuadro a crear
			'titulo' texto de encabezamiento (NULL si ninguno)
			'fil', 'col' posici¢n del cuadro; si 'fil' es igual
			a C_CENT se centra el cuadro verticalmente, si 'col'
			es igual a C_CENT se centra horizontalmente
			'ancho', 'alto' tama¤o del cuadro
			'attr_princ' atributo principal del cuadro
			'attr_s1', 'attr_s2' atributos para sombra
			'attr_tecla' atributo para teclas de activaci¢n
			'attr_sel' atributo para elemento seleccionado
			(p.ej.: cadena en listas )
****************************************************************************/
void c_crea_cuadro(STC_CUADRO *cuad, char *titulo, short fil, short col,
  short ancho, short alto, BYTE attr_princ, BYTE attr_s1, BYTE attr_s2,
  BYTE attr_tecla, BYTE attr_sel)
{
int i;

/* comprueba si hay que centrar el cuadro */
if(fil==C_CENT) fil=(25-alto)/2;
if(col==C_CENT) col=(80-ancho)/2;

v_crea(&cuad->v,fil,col,ancho,alto,attr_princ,attr_s1,attr_s2,titulo);
cuad->attr_tecla=attr_tecla;
cuad->attr_sel=attr_sel;

/* inicializa elementos del cuadro */
for(i=0; i<C_MAXELEM; i++) {
	cuad->elem[i].tipo=C_ELEM_NULO;
	cuad->elem[i].info=NULL;
}

cuad->elemento=0;

}

/****************************************************************************
	C_CREA_ELEMENTO: crea o modifica un elemento de un cuadro.
	  Entrada:      'cuad' puntero a estructura del cuadro
			'elem' n£mero de elemento del cuadro
			'fil', 'col' posici¢n del elemento dentro del cuadro
			'ancho', 'alto' tama¤o del elemento
			'attr_princ' atributo principal del elemento
			'attr_s1', 'attr_s2' atributos para sombra
			'tipo' tipo de elemento
			'info' puntero a informaci¢n del elemento
	  Salida:       1 si pudo crear elemento, 0 si no
****************************************************************************/
int c_crea_elemento(STC_CUADRO *cuad, int elem, short fil, short col,
  short ancho, short alto, BYTE attr_princ, BYTE attr_s1, BYTE attr_s2,
  int tipo, void *info)
{
STC_ELEM *e;
short fil_elem, col_elem;

if(elem>=C_MAXELEM) return(0);

e=&cuad->elem[elem];

/* calcula posici¢n del elemento en pantalla */
fil_elem=cuad->v.fil+fil+1;
col_elem=cuad->v.col+col+1;

v_crea(&e->v,fil_elem,col_elem,ancho,alto,attr_princ,attr_s1,attr_s2,NULL);
v_borde(&e->v,Borde1_Elem);

e->tipo=tipo;
e->info=info;

/* inicializa elemento seg£n tipo */
switch(tipo) {
	case C_ELEM_BOTON :
		inicializa_elem_boton(e->info);
		break;
	case C_ELEM_INPUT :
		inicializa_elem_input(e->info);
		break;
	case C_ELEM_LISTA :
		inicializa_elem_lista(e->info);
		break;
	case C_ELEM_CHECK :
		inicializa_elem_check(e->info);
		e->v.alto=1;
		e->v.ancho=strlen(((STC_ELEM_CHECK *)e->info)->texto)+4;
		break;
	case C_ELEM_TEXTO :
		if(((STC_ELEM_TEXTO *)e->info)->modo!=0) v_modo_texto(&e->v,
		  V_PASA_LINEA);
		break;
}

return(1);
}

/****************************************************************************
	C_DIBUJA_ELEMENTO: dibuja un elemento de un cuadro.
	  Entrada:      'cuad' puntero a la estructura con la informaci¢n
			del cuadro
			'elem' elemento del cuadro
	  Salida:       1 si pudo dibujarlo, 0 si no
****************************************************************************/
int c_dibuja_elemento(STC_CUADRO *cuad, int elem)
{
STC_ELEM *e;
char *txt;

if(elem>=C_MAXELEM) return(0);

r_puntero(R_OCULTA);

e=&cuad->elem[elem];

switch(cuad->elem[elem].tipo) {
	case C_ELEM_NULO :
		break;
	case C_ELEM_BOTON :
		v_dibuja(&e->v,1);
		imprime_txt_elem(&e->v,e->v.fil+1,e->v.col+1,
		  ((STC_ELEM_BOTON *)e->info)->cadena,cuad->attr_tecla,1);
		break;
	case C_ELEM_INPUT :
		v_dibuja(&e->v,1);
		txt=((STC_ELEM_INPUT *)e->info)->texto;
		imprime_txt_elem(&e->v,e->v.fil+1,e->v.col-strlen(txt),txt,
		  cuad->attr_tecla,1);
		imprime_txt_elem(&e->v,e->v.fil+1,e->v.col+1,
		  ((STC_ELEM_INPUT *)e->info)->cadena,cuad->attr_tecla,0);
		break;
	case C_ELEM_LISTA :
		v_dibuja(&e->v,0);
		txt=((STC_ELEM_LISTA *)e->info)->texto;
		imprime_txt_elem(&e->v,e->v.fil,e->v.col+((e->v.ancho-
		  strlen(txt))/2),txt,cuad->attr_tecla,1);
		imprime_lista(&e->v,((STC_ELEM_LISTA *)e->info)->elemento,
		  ((STC_ELEM_LISTA *)e->info)->elemento_pr,
		  ((STC_ELEM_LISTA *)e->info)->elemento_sel,
		  cuad->attr_sel);
		dibuja_flechas_vert(&e->v,
		  ((STC_ELEM_LISTA *)e->info)->num_elementos,
		  ((STC_ELEM_LISTA *)e->info)->elemento_sel);
		break;
	case C_ELEM_CHECK :
		v_impcar(e->v.fil,e->v.col,CCHECK_BOR1,e->v.attr_princ);
		v_impcar(e->v.fil,e->v.col+2,CCHECK_BOR2,e->v.attr_princ);
		if(e->v.borde[0]==Borde1_Elem[0]) v_impcar(e->v.fil,
		  e->v.col+3,' ',e->v.attr_princ);
		else v_impcar(e->v.fil,e->v.col+3,CCHECK_MARC,e->v.attr_princ);
		imprime_txt_elem(&e->v,e->v.fil,e->v.col+4,
		  ((STC_ELEM_CHECK *)e->info)->texto,cuad->attr_tecla,1);
		if(((STC_ELEM_CHECK *)e->info)->estado) v_impcar(e->v.fil,
		  e->v.col+1,CCHECK_ON,e->v.attr_princ);
		else v_impcar(e->v.fil,e->v.col+1,CCHECK_OFF,e->v.attr_princ);
		break;
	case C_ELEM_TEXTO :
		if(((STC_ELEM_TEXTO *)e->info)->borde==C_TXTBORDE)
		  v_dibuja(&e->v,1);
		txt=((STC_ELEM_TEXTO *)e->info)->texto;
		v_pon_cursor(&e->v,0,0);
		v_impcad(&e->v,txt,V_NORELLENA);
		break;
}

r_puntero(R_MUESTRA);

return(1);
}

/****************************************************************************
	C_ABRE: abre un cuadro de di logo.
	  Entrada:      'cuad' puntero a la estructura con la informaci¢n
			del cuadro
****************************************************************************/
void c_abre(STC_CUADRO *cuad)
{
int i;

v_abre(&cuad->v);
for(i=0; i<C_MAXELEM; i++) c_dibuja_elemento(cuad,i);

}

/****************************************************************************
	C_CIERRA: cierra un cuadro de di logo.
	  Entrada:      'cuad' puntero a la estructura con la informaci¢n
			del cuadro
****************************************************************************/
void c_cierra(STC_CUADRO *cuad)
{

v_cierra(&cuad->v);

}

/****************************************************************************
	C_LEE_INPUT: lee una cadena de caracteres por teclado o edita una
	  cadena ya existente.
	  Entrada:      'v' ventana donde se recoger  la cadena
			'cadena' puntero a buffer d¢nde se guardar 
			la cadena tecleada; si el primer car cter no es '\0'
			se ajustar  la rutina para poder editar la cadena
			'longitud' longitud de la cadena (sin contar el
			\0 final)
	  Salida:       n£mero de caracteres tecleados
			-1 si se puls¢ TAB o se puls¢ fuera de la ventana
			-2 si se puls¢ ESCAPE
****************************************************************************/
int c_lee_input(STC_VENTANA *v, char *cadena, int longitud)
{
STC_RATON r;
short fila_ant, columna_ant, maxfil, maxcol;
char *cur, *fin, *ptr;
int num_car=0, ccur;
unsigned tecla;

/* coge posici¢n actual del cursor */
coge_pos_cursor(&fila_ant,&columna_ant);

/* inicializa punteros */
cur=cadena;

/* busca final de la cadena y n£mero de caracteres */
for(fin=cadena; *fin; fin++, num_car++);

/* coordenada relativa del cursor respecto origen de ventana */
ccur=0;

do {
	/* imprime l¡nea */
	v_pon_cursor(v,0,0);
	v_impcad(v,cur-ccur,V_NORELLENA);
	v_impc(v,' ');

	/* imprime cursor */
	pon_cursor(v->fil+1,v->col+ccur+1);

	do {
		/* recoge posici¢n del rat¢n */
		r_estado(&r);
		if(r.boton1) {
			/* comprueba si se puls¢ fuera de la ventana */
			/* y si es as¡ sale */
			maxfil=v->fil+v->alto-1;
			maxcol=v->col+v->ancho-1;
			if((r.fil<v->fil) || (r.fil>maxfil) ||
			  (r.col<v->col) || (r.col>maxcol)) {
				/* imprime l¡nea */
				v_pon_cursor(v,0,0);
				v_impcad(v,cadena,V_NORELLENA);
				pon_cursor(fila_ant,columna_ant);
				return(-1);
			}
		}

		/* mira si hay tecla pulsada */
		tecla=bioskey(1);

	} while(!tecla);

	/* saca tecla del buffer */
	bioskey(0);

	/* comprueba si es una tecla de movimiento de cursor */
	switch(tecla >> 8) {
		case 0x4b :             /* cursor izquierda */
			if(cur>cadena) {
				cur--;
				if(ccur>0) ccur--;
			}
			break;
		case 0x4d :             /* cursor derecha */
			if(cur<fin) {
				cur++;
				if(ccur<(v->ancho-3)) ccur++;
			}
			break;
		case 0x47 :             /* cursor origen */
			cur=cadena;
			ccur=0;
			break;
		case 0x4f :             /* cursor fin */
			cur=fin;
			if(num_car<(v->ancho-2)) ccur=num_car;
			else ccur=v->ancho-3;
			break;
		case 0x0e :             /* borrado hacia atr s */
			if(cur>cadena) {
				cur--;
				fin--;
				for(ptr=cur; ptr<=fin; ptr++) *ptr=*(ptr+1);
				num_car--;
				if(ccur>0) ccur--;
			}
			break;
		case 0x53 :             /* borrado */
			if(cur<fin) {
				for(ptr=cur; ptr<fin; ptr++) *ptr=*(ptr+1);
				fin--;
				num_car--;
			}
			break;
		case 0x0f :             /* TAB */
			/* imprime l¡nea */
			v_pon_cursor(v,0,0);
			v_impcad(v,cadena,V_NORELLENA);
			pon_cursor(fila_ant,columna_ant);
			return(-1);
		case 0x01 :             /* ESCAPE */
			/* imprime l¡nea */
			v_pon_cursor(v,0,0);
			v_impcad(v,cadena,V_NORELLENA);
			pon_cursor(fila_ant,columna_ant);
			return(-2);
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
		if(ccur<(v->ancho-3)) ccur++;
	}

} while(tecla!=0x0d);           /* hasta que pulse RETURN */

/* imprime l¡nea */
v_pon_cursor(v,0,0);
v_impcad(v,cadena,V_NORELLENA);

pon_cursor(fila_ant,columna_ant);

return(num_car);
}

/****************************************************************************
	C_ACCION: env¡a una acci¢n a un cuadro de di logo.
	  Entrada:      'c' puntero a cuadro de di logo
			'acc' puntero a estructura con la acci¢n sobre el
			cuadro
	  Salida:       elemento seleccionado o c¢digo de acci¢n (si <0)
****************************************************************************/
int c_accion(STC_CUADRO *c, STC_ACCION_C *acc)
{
STC_ELEM *e;
STC_RATON r;
int i;
char tecla, tecla_elem;
short minfil, maxfil, mincol, maxcol;

switch(acc->accion) {
	case CUADRO_NULA :              /* acci¢n nula */
		break;
	case CUADRO_SALIDA :            /* salir del cuadro */
		c_cierra(c);
		break;
	case CUADRO_SGTE_ELEM :         /* siguiente elemento */
		i=siguiente_elemento(c);
		resalta_elemento(c);
		return(CUADRO_NULA);
	case CUADRO_ANT_ELEM :          /* elemento anterior */
		i=anterior_elemento(c);
		resalta_elemento(c);
		return(CUADRO_NULA);
	case CUADRO_SELECC :
		return(c->elemento);
	case CUADRO_TECLA :
		if(!acc->tecla) return(CUADRO_NULA);
		tecla=mayuscula(acc->tecla);
		for(i=0; i<C_MAXELEM; i++) {
			e=&c->elem[i];
			switch(c->elem[i].tipo) {
			    case C_ELEM_BOTON :
				tecla_elem=((STC_ELEM_BOTON *)e->info)->tecla;
				break;
			    case C_ELEM_INPUT :
				tecla_elem=((STC_ELEM_INPUT *)e->info)->tecla;
				break;
			    case C_ELEM_LISTA :
				tecla_elem=((STC_ELEM_LISTA *)e->info)->tecla;
				break;
			    case C_ELEM_CHECK :
				tecla_elem=((STC_ELEM_CHECK *)e->info)->tecla;
				break;
			}
			if(tecla==tecla_elem) {
				c->elemento=i;
				resalta_elemento(c);
				return(i);
			}
		}
		return(CUADRO_NULA);
	case CUADRO_RATON :
		do {
			for(i=0; i<C_MAXELEM; i++) {
				if((c->elem[i].tipo!=C_ELEM_NULO) &&
				  (c->elem[i].tipo!=C_ELEM_TEXTO)) {
					minfil=(c->elem[i].v).fil;
					maxfil=minfil+(c->elem[i].v).alto-1;
					mincol=(c->elem[i].v).col;
					maxcol=mincol+(c->elem[i].v).ancho-1;
					if((acc->fil>=minfil) &&
					  (acc->fil<=maxfil) &&
					  (acc->col>=mincol) &&
					  (acc->col<=maxcol)) {
						if(c->elemento!=i) {
							c->elemento=i;
							resalta_elemento(c);
						}
						break;
					}
				}
			}
			r_estado(&r);
			acc->fil=r.fil;
			acc->col=r.col;
		} while(r.boton1);
		if(i==C_MAXELEM) return(CUADRO_NULA);
		else return(i);
}

return(acc->accion);
}

/****************************************************************************
	C_GESTIONA: gestiona un cuadro de di logo.
	  Entrada:      'c' puntero a cuadro de di logo
	  Salida:       £ltimo elemento seleccionado, -1 si se sali¢ del
			cuadro
****************************************************************************/
int c_gestiona(STC_CUADRO *c)
{
STC_ACCION_C acc;
STC_RATON r;
unsigned shift, tecla;
int i, accion, elemento;
short filcur, colcur;

/* si rat¢n no est  inicializado, lo inicializa */
if(!r_puntero(R_MUESTRA)) r_inicializa();

/* oculta cursor */
coge_pos_cursor(&filcur,&colcur);
pon_cursor(25,0);

c->elemento=0;
resalta_elemento(c);
acc.accion=CUADRO_NULA;

while(1) {
	/* si elemento actual es de entrada o lista, lo selecciona */
	if((c->elem[c->elemento].tipo==C_ELEM_INPUT) ||
	  (c->elem[c->elemento].tipo==C_ELEM_LISTA)) {
		acc.accion=CUADRO_SELECC;
		accion=3;
	}
	else do {
		accion=0;
		r_estado(&r);
		/* si pulsado el bot¢n 1 del rat¢n, indica acci¢n del rat¢n */
		if(r.boton1) accion=1;
		else {
			/* si hay tecla esperando, indica acci¢n de teclado */
			tecla=bioskey(1);
			if(tecla) {
				bioskey(0);
				shift=bioskey(2);
				accion=2;
			}
		}
	} while(!accion);

	if(accion==1) {
		acc.accion=CUADRO_RATON;
		acc.fil=r.fil;
		acc.col=r.col;
	}
	else if(accion==2) switch(tecla >> 8) {
		case 0x01 :     /* ESCAPE */
			acc.accion=CUADRO_SALIDA;
			break;
		case 0x1c :     /* RETURN */
			acc.accion=CUADRO_SELECC;
			break;
		case 0x0f :     /* TAB */
			if(shift & 0x0003) acc.accion=CUADRO_ANT_ELEM;
			else acc.accion=CUADRO_SGTE_ELEM;
			break;
		default :
			acc.accion=CUADRO_TECLA;
			acc.tecla=(char)(tecla & 0x00ff);
			break;
	}

	elemento=c_accion(c,&acc);

	/* ejecuta acciones seg£n el tipo de elemento */
	if(elemento>=0) {
		switch(c->elem[elemento].tipo) {
			case C_ELEM_NULO :
				break;
			case C_ELEM_BOTON :
				pon_cursor(filcur,colcur);
				return(elemento);
			case C_ELEM_INPUT :
				i=c_elem_input(&c->elem[elemento].v,
				  c->elem[elemento].info);
				/* si puls¢ TAB o fuera de elemento */
				if(i==-1) {
					shift=bioskey(2);
					if(shift & 0x0003) anterior_elemento(c);
					else siguiente_elemento(c);
					resalta_elemento(c);
				}
				/* si puls¢ ESC */
				else if(i==-2) {
					pon_cursor(filcur,colcur);
					return(-1);
				}
				break;
			case C_ELEM_LISTA :
				i=c_elem_lista(&c->elem[elemento].v,
				  c->elem[elemento].info,c->attr_tecla,
				  c->attr_sel);
				/* si puls¢ TAB o fuera de elemento */
				if(i==-1) {
					shift=bioskey(2);
					if(shift & 0x0003) anterior_elemento(c);
					else siguiente_elemento(c);
					resalta_elemento(c);
				}
				/* si puls¢ ESC */
				else if(i==-2) {
					pon_cursor(filcur,colcur);
					return(-1);
				}
				/* si lista est  vac¡a */
				else if(i==-3) {
					siguiente_elemento(c);
					resalta_elemento(c);
				}
				break;
			case C_ELEM_CHECK :
				c_elem_check(c->elem[elemento].info,c,
				  elemento);
				break;
			case C_ELEM_TEXTO :
				break;
		}
	}
	else if(elemento==CUADRO_SALIDA) {
		pon_cursor(filcur,colcur);
		return(-1);
	}
}

}

/****************************************************************************
	C_ELEM_INPUT: gestiona un elemento de entrada.
	  Entrada:      'v' puntero a ventana de elemento
			'e' puntero a informaci¢n de elemento de entrada
	  Salida:       n£mero de caracteres tecleados.
			-1 si se puls¢ TAB o se puls¢ fuera de la ventana
			-2 si se puls¢ ESCAPE
****************************************************************************/
int c_elem_input(STC_VENTANA *v, STC_ELEM_INPUT *e)
{
int i;

i=c_lee_input(v,e->cadena,e->longitud);

return(i);
}

/****************************************************************************
	C_ELEM_LISTA: gestiona un elemento de lista.
	  Entrada:      'v' puntero a ventana de elemento
			'e' puntero a informaci¢n de elemento de lista
			'attr_tecla' atributo para tecla de activaci¢n
			'attr_sel' atributo para elemento seleccionado
	  Salida:       n£mero de elemento seleccionado
			-1 si se puls¢ TAB o se puls¢ fuera de la ventana
			-2 si se puls¢ ESCAPE
			-3 si la lista est  vac¡a
****************************************************************************/
int c_elem_lista(STC_VENTANA *v, STC_ELEM_LISTA *e, BYTE attr_tecla,
  BYTE attr_sel)
{
STC_RATON r;
int elemento_ant=-1;
short maxfil, maxcol, fil, col, filcur, colcur;
unsigned tecla;

/* si la lista est  vac¡a, sale */
if(!e->num_elementos) return(-3);

/* oculta cursor */
coge_pos_cursor(&filcur,&colcur);
pon_cursor(25,0);

/* busca elemento seleccionado */
busca_elem_selecc(e);

while(1) {
	if(e->elemento_sel!=elemento_ant) imprime_lista(v,e->elemento,
	  e->elemento_pr,e->elemento_sel,attr_sel);
	elemento_ant=e->elemento_sel;

	do {
		tecla=0;
		/* recoge posici¢n del rat¢n */
		r_estado(&r);
		if(r.boton1) {
			/* comprueba si se puls¢ fuera de la ventana */
			/* y si es as¡ sale */
			maxfil=v->fil+v->alto-1;
			maxcol=v->col+v->ancho-1;
			if((r.fil<v->fil) || (r.fil>maxfil) ||
			  (r.col<v->col) || (r.col>maxcol)) {
				v_dibuja(v,0);
				imprime_txt_elem(v,v->fil,v->col+((v->ancho-
				  strlen(e->texto))/2),e->texto,attr_tecla,1);
				imprime_lista(v,e->elemento,e->elemento_pr,
				  e->elemento_sel,attr_sel);
				dibuja_flechas_vert(v,e->num_elementos,
				  e->elemento_sel);
				busca_elem_selecc(e);
				pon_cursor(filcur,colcur);
				return(-1);
			}
			else {
				/* calcula coordenadas relativas al */
				/* origen de la ventana */
				fil=r.fil-v->fil;
				col=r.col-v->col;

				if((fil==0) && (col==(v->ancho-1)))
				  tecla=0x4700;
				else if((fil==(v->alto-1)) &&
				  (col==(v->ancho-1))) tecla=0x4f00;
				else if((fil==1) && (col==(v->ancho-1)))
				  tecla=0x4800;
				else if((fil==(v->alto-2)) &&
				  (col==(v->ancho-1))) tecla=0x5000;
				else if((fil==0) && (col!=0) &&
				  (col!=(v->ancho-1))) tecla=0x4800;
				else if((fil==(v->alto-1)) && (col!=0) &&
				  (col!=(v->ancho-1))) tecla=0x5000;
				else if((fil<(v->alto/2)) &&
				  (col==(v->ancho-1))) tecla=0x4900;
				else if((fil>=(v->alto/2)) &&
				  (col==(v->ancho-1))) tecla=0x5100;
				else if((fil>0) && (fil<(v->alto-1)) &&
				  (col>0) && (col<(v->ancho-1))) {
					elemento_ant=e->elemento_sel;
					e->elemento_sel=e->elemento_pr+fil-1;
					if(e->elemento_sel>
					  (e->num_elementos-1))
					  e->elemento_sel=e->num_elementos-1;

					/* si elemento seleccionado es */
					/* distinto a elemento anterior */
					/* lo resalta; si no, lo selecciona */
					/* y sale */
					if(e->elemento_sel!=elemento_ant) {
						imprime_lista(v,e->elemento,
						  e->elemento_pr,
						  e->elemento_sel,
						  attr_sel);
						dibuja_flechas_vert(v,
						  e->num_elementos,
						  e->elemento_sel);
					}
					else {
						r_estado(&r);
						if(r.boton1) break;
						v_dibuja(v,0);
						imprime_txt_elem(v,v->fil,
						  v->col+((v->ancho-
						  strlen(e->texto))/2),
						  e->texto,attr_tecla,1);
						imprime_lista(v,e->elemento,
						  e->elemento_pr,
						  e->elemento_sel,
						  attr_sel);
						dibuja_flechas_vert(v,
						  e->num_elementos,
						  e->elemento_sel);
						busca_elem_selecc(e);
						pon_cursor(filcur,colcur);
						return(e->elemento_sel);
					}
				}
			}

			delay(C_PAUSA);
		}
		else {
			tecla=bioskey(1);
			/* saca tecla del buffer */
			if(tecla) bioskey(0);
		}

	} while(!tecla);

	/* comprueba si es una tecla de movimiento de cursor */
	switch(tecla >> 8) {
		case 0x48 :             /* cursor arriba */
			if(e->elemento_sel>0) e->elemento_sel--;
			if(e->modo==0) {
				if(e->elemento_sel<e->elemento_pr)
				  e->elemento_pr--;
			}
			else e->elemento_pr=e->elemento_sel;
			dibuja_flechas_vert(v,e->num_elementos,
			  e->elemento_sel);
			break;
		case 0x50 :             /* cursor abajo */
			if(e->elemento_sel<(e->num_elementos-1))
			  e->elemento_sel++;
			if(e->modo==0) {
				if(e->elemento_sel>(e->elemento_pr+v->alto-3))
				  e->elemento_pr++;
			}
			else e->elemento_pr=e->elemento_sel;
			dibuja_flechas_vert(v,e->num_elementos,
			  e->elemento_sel);
			break;
		case 0x49 :             /* p gina arriba */
			if(e->elemento_pr==0) break;
			e->elemento_pr-=v->alto-2;
			if(e->elemento_pr<0) e->elemento_pr=0;
			e->elemento_sel=e->elemento_pr;
			v_dibuja(v,0);
			imprime_txt_elem(v,v->fil,v->col+((v->ancho-
			  strlen(e->texto))/2),e->texto,attr_tecla,1);
			imprime_lista(v,e->elemento,e->elemento_pr,
			  e->elemento_sel,attr_sel);
			dibuja_flechas_vert(v,e->num_elementos,
			  e->elemento_sel);
			break;
		case 0x51 :             /* p gina abajo */
			if(e->elemento_pr==(e->num_elementos-1)) break;
			e->elemento_pr+=v->alto-2;
			if(e->elemento_pr>(e->num_elementos-1))
			  e->elemento_pr=e->num_elementos-1;
			e->elemento_sel=e->elemento_pr;
			v_dibuja(v,0);
			imprime_txt_elem(v,v->fil,v->col+((v->ancho-
			  strlen(e->texto))/2),e->texto,attr_tecla,1);
			imprime_lista(v,e->elemento,e->elemento_pr,
			  e->elemento_sel,attr_sel);
			dibuja_flechas_vert(v,e->num_elementos,
			  e->elemento_sel);
			break;
		case 0x47 :             /* cursor origen */
			e->elemento_pr=0;
			e->elemento_sel=0;
			dibuja_flechas_vert(v,e->num_elementos,
			  e->elemento_sel);
			break;
		case 0x4f :             /* cursor fin */
			e->elemento_pr=e->num_elementos-(v->alto-2);
			if(e->elemento_pr<0) e->elemento_pr=0;
			e->elemento_sel=e->num_elementos-1;
			dibuja_flechas_vert(v,e->num_elementos,
			  e->elemento_sel);
			break;
		case 0x1c :             /* RETURN */
			v_dibuja(v,0);
			imprime_txt_elem(v,v->fil,v->col+((v->ancho-
			  strlen(e->texto))/2),e->texto,attr_tecla,1);
			imprime_lista(v,e->elemento,e->elemento_pr,
			  e->elemento_sel,attr_sel);
			dibuja_flechas_vert(v,e->num_elementos,
			  e->elemento_sel);
			busca_elem_selecc(e);
			pon_cursor(filcur,colcur);
			return(e->elemento_sel);
		case 0x0f :             /* TAB */
			v_dibuja(v,0);
			imprime_txt_elem(v,v->fil,v->col+((v->ancho-
			  strlen(e->texto))/2),e->texto,attr_tecla,1);
			imprime_lista(v,e->elemento,e->elemento_pr,
			  e->elemento_sel,attr_sel);
			dibuja_flechas_vert(v,e->num_elementos,
			  e->elemento_sel);
			busca_elem_selecc(e);
			pon_cursor(filcur,colcur);
			return(-1);
		case 0x01 :             /* ESCAPE */
			v_dibuja(v,0);
			imprime_txt_elem(v,v->fil,v->col+((v->ancho-
			  strlen(e->texto))/2),e->texto,attr_tecla,1);
			imprime_lista(v,e->elemento,e->elemento_pr,
			  e->elemento_sel,attr_sel);
			dibuja_flechas_vert(v,e->num_elementos,
			  e->elemento_sel);
			e->selecc=NULL;
			pon_cursor(filcur,colcur);
			return(-2);
	}
}

}

/****************************************************************************
	C_METE_EN_LISTA: inserta una cadena al final de la lista.
	  Entrada:      'e' puntero a la estructura con la informaci¢n de
			la lista
			'cadena' puntero a cadena de caracteres a a¤adir
			al final de la lista
	  Salida:       1 si se pudo crear y a¤adir elemento a lista, si
			no devuelve 0
****************************************************************************/
int c_mete_en_lista(STC_ELEM_LISTA *e, char *cadena)
{
STC_LISTA *l_ultimo, *l_anterior, *l;

/* crea nuevo elemento */
l=(STC_LISTA *)malloc(sizeof(STC_LISTA));
if(l==NULL) return(0);
l->cadena=(char *)malloc(strlen(cadena)+1);
if(l->cadena==NULL) {
	free(l);
	return(0);
}
strcpy(l->cadena,cadena);
l->sgte=NULL;

/* puntero a primer elemento de la lista */
l_ultimo=e->elemento;

/* si el primer elemento est  vac¡o, mete ah¡ la cadena */
if(l_ultimo==NULL) {
	e->elemento=l;
	e->num_elementos++;
	return(1);
}

/* si lista no ordenada, a¤ade elemento al final */
if(e->orden==C_LSTSINORDEN) {
	/* busca £ltimo elemento de la lista */
	while(l_ultimo->sgte!=NULL) l_ultimo=l_ultimo->sgte;
	l_ultimo->sgte=l;
}
else {
	l_anterior=NULL;
	/* busca elemento anterior al que se quiere insertar */
	while((l_ultimo->sgte!=NULL) && (strcmp(l_ultimo->cadena,cadena)<0)) {
		l_anterior=l_ultimo;
		l_ultimo=l_ultimo->sgte;
	}

	/* si elemento debe ir en primera posici¢n */
	if(l_anterior==NULL) {
		/* si s¢lo hay un elemento, comprueba si el que */
		/* se quiere insertar va antes o despu‚s */
		if(l_ultimo->sgte==NULL) {
			if(strcmp(l_ultimo->cadena,cadena)<0) {
				l_ultimo->sgte=l;
				l->sgte=NULL;
			}
			else {
				l->sgte=l_ultimo;
				e->elemento=l;
			}
		}
		else {
			l->sgte=e->elemento;
			e->elemento=l;
		}
	}
	else {
		/* si se lleg¢ a £ltimo elemento comprueba si el que */
		/* se quiere insertar va antes o despu‚s */
		if(l_ultimo->sgte==NULL) {
			if(strcmp(l_ultimo->cadena,cadena)<0) {
				l_ultimo->sgte=l;
				l->sgte=NULL;
			}
			else {
				l->sgte=l_ultimo;
				l_anterior->sgte=l;
			}
		}
		else {
			l->sgte=l_anterior->sgte;
			l_anterior->sgte=l;
		}
	}
}

/* incrementa n£mero de elementos en lista */
e->num_elementos++;

return(1);
}

/****************************************************************************
	C_BORRA_LISTA: elimina todos los elementos de una lista, liberando
	  la memoria ocupada.
	  Entrada:      'e' puntero a la estructura con la informaci¢n de
			la lista
****************************************************************************/
void c_borra_lista(STC_ELEM_LISTA *e)
{
STC_LISTA *l, *l_sgte;

/* puntero a primer elemento de la lista */
l=e->elemento;

/* sale si lista est  vac¡a */
if(l==NULL) return;

do {
	/* puntero a siguiente elemento de la lista */
	l_sgte=l->sgte;

	free(l->cadena);
	free(l);

	l=l_sgte;

} while(l_sgte!=NULL);

e->elemento=NULL;
e->selecc=NULL;
e->num_elementos=0;
e->elemento_pr=0;
e->elemento_sel=0;

}

/****************************************************************************
	C_ELEM_CHECK: gestiona un elemento de caja de comprobaci¢n.
	  Entrada:      'e' puntero a informaci¢n de elemento de entrada
			'cuad' puntero a cuadro al que pertenece
			'elem' n£mero de elemento dentro del cuadro
	  Salida:       estado; 0 desactivada, 1 activada
****************************************************************************/
int c_elem_check(STC_ELEM_CHECK *e, STC_CUADRO *cuad, int elem)
{

if(e->estado==0) e->estado=1;
else e->estado=0;

c_dibuja_elemento(cuad,elem);

return(e->estado);
}

/****************************************************************************
	C_SELECC_FICHEROS: gestiona un cuadro de selecci¢n de ficheros.
	  Entrada:      'fil', 'col' posici¢n del cuadro; si 'fil' o 'col'
			son iguales a C_CENT se centrar  el cuadro
			'titulo' t¡tulo del cuadro
			'attr_princ' atributo principal
			'attr_s1', 'attr_s2' atributos de sombreado
			'attr_tecla' atributo de teclas de activaci¢n
			'attr_sel' atributo elemento seleccionado
			'ruta' ruta completa al directorio; si se da una
			cadena vac¡a se coge la ruta al directorio actual
			'mascara' m scara de b£squeda de ficheros
			'fichero' puntero a buffer donde se dejar  la
			ruta completa y el nombre del fichero seleccionado o
			una cadena vac¡a si no seleccion¢ ning£n fichero; el
			buffer debe ser de una longitud de _MAX_PATH bytes
****************************************************************************/
void c_selecc_ficheros(short fil, short col, char *titulo, BYTE attr_princ,
  BYTE attr_s1, BYTE attr_s2, BYTE attr_tecla, BYTE attr_sel, char *ruta,
  char *mascara, char *fichero)
{
STC_CUADRO cfich;
STC_ACCION_C acc;
STC_RATON r;
STC_ELEM_INPUT nombre_fich={
	"^Fichero"
};
STC_ELEM_LISTA lista_fich={
	" ^Lista ", C_LSTNORMAL, C_LSTORDENADA
};
STC_ELEM_BOTON vale={
	" ^Vale"
};
STC_ELEM_BOTON salir={
	" ^Salir"
};
char nfich[MAXPATH], disq[MAXDRIVE], direct[MAXDIR], fich[MAXFILE],
  ext[MAXEXT], dir_orig[MAXPATH], dir[15];
int i, elemento, accion, unid_orig;
unsigned shift, tecla;
short filcur, colcur;

/* guarda directorio y unidad actuales */
getcwd(dir_orig,MAXPATH);
unid_orig=getdisk();

/* si rat¢n no est  inicializado, lo inicializa */
if(!r_puntero(R_MUESTRA)) r_inicializa();

/* oculta cursor */
coge_pos_cursor(&filcur,&colcur);
pon_cursor(25,0);

/* inicializa buffer para nombre de fichero */
if(*ruta) strcpy(nfich,ruta);
else strcpy(nfich,dir_orig);

/* cambia a directorio */
i=strlen(nfich);
if(nfich[i-1]=='\\') nfich[i-1]='\0';
chdir(nfich);

pon_barra_dir(nfich);
strcat(nfich,mascara);
may_str(nfich);

nombre_fich.cadena=nfich;
nombre_fich.longitud=MAXPATH-1;

/* crea cuadro de selecci¢n de ficheros */
c_crea_cuadro(&cfich,titulo,fil,col,C_FICH_ANCHO,C_FICH_ALTO,attr_princ,
  attr_s1,attr_s2,attr_tecla,attr_sel);
c_crea_elemento(&cfich,0,0,9,C_FICH_ANCHO-11,3,attr_princ,attr_s2,attr_s1,
  C_ELEM_INPUT,&nombre_fich);
c_crea_elemento(&cfich,1,3,0,C_FICH_ANCHO-20,C_FICH_ALTO-5,attr_princ,attr_s2,
  attr_s1,C_ELEM_LISTA,&lista_fich);
c_crea_elemento(&cfich,2,C_FICH_ALTO-5,C_FICH_ANCHO-19,8,3,attr_princ,attr_s1,
  attr_s2,C_ELEM_BOTON,&vale);
c_crea_elemento(&cfich,3,C_FICH_ALTO-5,C_FICH_ANCHO-11,9,3,attr_princ,attr_s1,
  attr_s2,C_ELEM_BOTON,&salir);

cfich.elemento=0;
lista_ficheros(&lista_fich,nfich);
c_abre(&cfich);
resalta_elemento(&cfich);
acc.accion=CUADRO_NULA;

while(1) {
	/* si elemento actual es de entrada o lista, lo selecciona */
	if((cfich.elem[cfich.elemento].tipo==C_ELEM_INPUT) ||
	  (cfich.elem[cfich.elemento].tipo==C_ELEM_LISTA)) {
		acc.accion=CUADRO_SELECC;
		accion=3;
	}
	else do {
		accion=0;
		r_estado(&r);
		/* si pulsado el bot¢n 1 del rat¢n, indica acci¢n del rat¢n */
		if(r.boton1) accion=1;
		else {
			/* si hay tecla esperando, indica acci¢n de teclado */
			tecla=bioskey(1);
			if(tecla) {
				bioskey(0);
				shift=bioskey(2);
				accion=2;
			}
		}
	} while(!accion);

	if(accion==1) {
		acc.accion=CUADRO_RATON;
		acc.fil=r.fil;
		acc.col=r.col;
	}
	else if(accion==2) switch(tecla >> 8) {
		case 0x01 :     /* ESCAPE */
			acc.accion=CUADRO_SALIDA;
			break;
		case 0x1c :     /* RETURN */
			acc.accion=CUADRO_SELECC;
			break;
		case 0x0f :     /* TAB */
			if(shift & 0x0003) acc.accion=CUADRO_ANT_ELEM;
			else acc.accion=CUADRO_SGTE_ELEM;
			break;
		default :
			acc.accion=CUADRO_TECLA;
			acc.tecla=(char)(tecla & 0x00ff);
			break;
	}

	elemento=c_accion(&cfich,&acc);

	/* ejecuta acciones seg£n el elemento seleccionado */
	switch(elemento) {
		case 0 :        /* ventana introducci¢n nombre fichero */
			i=c_elem_input(&cfich.elem[0].v,cfich.elem[0].info);
			lista_ficheros(&lista_fich,nfich);
			c_dibuja_elemento(&cfich,1);
			/* si puls¢ TAB o fuera de elemento */
			if(i==-1) {
				shift=bioskey(2);
				if(shift & 0x0003) anterior_elemento(&cfich);
				else siguiente_elemento(&cfich);
				resalta_elemento(&cfich);
			}
			/* si puls¢ ESC */
			else if(i==-2) {
				c_cierra(&cfich);
				c_borra_lista(&lista_fich);
				*fichero='\0';
				/* restaura directorio y unidad de origen */
				setdisk(unid_orig);
				chdir(dir_orig);
				pon_cursor(filcur,colcur);
				return;
			}
			break;
		case 1 :        /* lista de ficheros */
			i=c_elem_lista(&cfich.elem[1].v,cfich.elem[1].info,
			  cfich.attr_tecla,cfich.attr_sel);
			/* si puls¢ TAB o fuera de elemento */
			if(i==-1) {
				shift=bioskey(2);
				if(shift & 0x0003) anterior_elemento(&cfich);
				else siguiente_elemento(&cfich);
				resalta_elemento(&cfich);
			}
			/* si puls¢ ESC */
			else if(i==-2) {
				c_cierra(&cfich);
				c_borra_lista(&lista_fich);
				*fichero='\0';
				/* restaura directorio y unidad de origen */
				setdisk(unid_orig);
				chdir(dir_orig);
				pon_cursor(filcur,colcur);
				return;
			}
			/* si lista est  vac¡a */
			else if(i==-3) {
				siguiente_elemento(&cfich);
				resalta_elemento(&cfich);
			}
			/* si seleccion¢ un elemento de la lista */
			/* comprueba si es directorio o fichero */
			else if((i>=0) && (lista_fich.selecc!=NULL)) {
				fnsplit(nfich,disq,direct,fich,ext);
				if(*lista_fich.selecc==C_CHRDIR1) {
					/* coge nombre de directorio */
					strcpy(dir,lista_fich.selecc+1);
					/* elimina car cter final */
					i=strlen(dir);
					dir[i-1]='\0';

					/* cambia a ese directorio */
					chdir(dir);
					/* coge nueva ruta */
					getcwd(nfich,MAXPATH);
					pon_barra_dir(nfich);
					strcat(nfich,mascara);

					/* crea lista nueva */
					lista_ficheros(&lista_fich,nfich);
					c_dibuja_elemento(&cfich,0);
					c_dibuja_elemento(&cfich,1);
				}
				else {
					strcpy(nfich,disq);
					strcat(nfich,direct);
					strcat(nfich,lista_fich.selecc);
					c_cierra(&cfich);
					c_borra_lista(&lista_fich);
					strcpy(fichero,nfich);
					/* restaura directorio y unidad */
					/* de origen */
					setdisk(unid_orig);
					chdir(dir_orig);
					pon_cursor(filcur,colcur);
					return;
				}
			}
			break;
		case 2 :        /* bot¢n 'Vale' */
			fnsplit(nfich,disq,direct,fich,ext);
			/* si hay elemento seleccionado de lista */
			/* y no es nombre de directorio, coge ese */
			if((lista_fich.selecc!=NULL) &&
			  (*lista_fich.selecc!=C_CHRDIR1)) {
				strcpy(nfich,disq);
				strcat(nfich,direct);
				strcat(nfich,lista_fich.selecc);
			}
			c_cierra(&cfich);
			c_borra_lista(&lista_fich);
			strcpy(fichero,nfich);
			/* restaura directorio y unidad de origen */
			setdisk(unid_orig);
			chdir(dir_orig);
			pon_cursor(filcur,colcur);
			return;
		case 3 :        /* bot¢n 'Salir' */
		case CUADRO_SALIDA :
			c_cierra(&cfich);
			c_borra_lista(&lista_fich);
			*fichero='\0';
			/* restaura directorio y unidad de origen */
			setdisk(unid_orig);
			chdir(dir_orig);
			pon_cursor(filcur,colcur);
			return;
	}
}

}
