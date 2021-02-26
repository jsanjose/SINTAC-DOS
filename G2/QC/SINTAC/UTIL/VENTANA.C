/****************************************************************************
				  VENTANA.C

	Rutinas para gestionar ventanas y cuadros de di logo en pantalla.

			    (c)1993 JSJ Soft Ltd.

	Las siguientes funciones son p£blicas:
		- abre_ventana: abre una ventana
		- cierra_ventana: cierra una ventana
		- cuadro_fich: crea un cuadro de selecci¢n de ficheros
		- cuadro_aviso: crea un cuadro para mensaje de aviso
		- cuadro_siono: crea un cuadro de seleccion SI o NO

	Las siguientes estructuras est n definidas en VENTANA.H
		STC_VENTANA: para la definici¢n de una ventana

****************************************************************************/

#include <graph.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <direct.h>
#include <dos.h>
#include "puntero.h"
#include "boton.h"
#include "chrimp.h"
#include "ventana.h"

/*** Prototipos de funciones internas ***/
static int lista_fich(char *ruta, char *mascara, STC_LISTA_FICH *(*lf));
static void imp_lista_fich(STC_LISTA_FICH *lf, short x, short y, short colorf,
  short color, int prelem, int nelem);
static void borra_lista_fich(STC_LISTA_FICH *lf);
static void recorta_espacios(char *cad);
static void calcula_tam_cuadro(char *texto, short *ancho, short *alto);
void imp_texto_centro(char *texto, short x, short y, short ancho,
  short alto, short colorf, short color);

/*** Variables globales ***/
/* iconos para cuadro selecci¢n de ficheros */
#define PCF_ICOF_ARR 3
static struct xycoord cf_icof_arr[PCF_ICOF_ARR]={
	{7, 3}, {11, 11}, {3, 11}
};

#define PCF_ICOF_ABJ 3
static struct xycoord cf_icof_abj[PCF_ICOF_ABJ]={
	{3, 3}, {11, 3}, {7, 11}
};

#define PCF_ICOF_ARRR 8
static struct xycoord cf_icof_arrr[PCF_ICOF_ARRR]={
	{3, 11}, {7, 3}, {11, 11}, {9, 11}, {7, 5}, {5, 11},
	{3, 11}, {11, 11}
};

#define PCF_ICOF_ABJR 8
static struct xycoord cf_icof_abjr[PCF_ICOF_ABJR]={
	{3, 3}, {7, 11}, {11, 3}, {9, 3}, {7, 9}, {5, 3},
	{3, 3}, {11, 3}
};

/****************************************************************************
	LISTA_FICH: crea una lista de ficheros.
	  Entrada:      'ruta' ruta del directorio
			'mascara' m scara de b£squeda de ficheros
	  Salida:       'lf' puntero a primer elemento de la lista
			n£mero de elementos de la lista
****************************************************************************/
int lista_fich(char *ruta, char *mascara, STC_LISTA_FICH *(*lf))
{
STC_LISTA_FICH *plf1=NULL, *plf2=NULL, *plf_primer=NULL;
struct find_t find;
char nfich[_MAX_PATH+LNG_NFICH];
unsigned encontrado;
int nelem=0, i;

/* crea nombre completo de fichero: ruta + "*.*" */
nfich[0]='\0';
strcat(nfich,ruta);

/* si no tiene '\' al final, se la a¤ade */
i=strlen(nfich)-1;
if(nfich[i]!='\\') strcat(nfich,"\\");

strcat(nfich,"*.*");

/* busca primero subdirectorios */
encontrado=_dos_findfirst(nfich,_A_SUBDIR,&find);

while(!encontrado) {
	if(find.attrib & _A_SUBDIR) {
		/* crea nuevo elemento de lista y almacena su */
		/* direcci¢n en el anterior (si no es el primero) */
		plf2=(STC_LISTA_FICH *)malloc(sizeof(STC_LISTA_FICH));
		if(plf1!=NULL) plf1->sgte_fich=plf2;
		else plf_primer=plf2;

		/* mete nombre de subdirectorio en lista */
		/* pone corchete inicial */
		plf2->fich[0]=CF_CHRDIR1;
		strcpy(&plf2->fich[1],find.name);
		/* pone corchete final */
		strcat(plf2->fich,CF_STRDIR2);

		/* rellena con espacios */
		for(i=strlen(plf2->fich); i<(LNG_NFICH+1); i++) {
			plf2->fich[i]=' ';
		}
		plf2->fich[i]='\0';

		plf2->sgte_fich=NULL;
		nelem++;

		/* guarda direcci¢n de nuevo elemento */
		plf1=plf2;
	}
	/* busca siguiente */
	encontrado=_dos_findnext(&find);
}

/* crea nombre completo de fichero: ruta + m scara */
nfich[0]='\0';
strcat(nfich,ruta);

/* si no tiene '\' al final, se la a¤ade */
i=strlen(nfich)-1;
if(nfich[i]!='\\') strcat(nfich,"\\");

strcat(nfich,mascara);

/* ahora busca ficheros */
encontrado=_dos_findfirst(nfich,_A_NORMAL,&find);

while(!encontrado) {
	/* crea nuevo elemento de lista y almacena su */
	/* direcci¢n en el anterior (si no es el primero) */
	plf2=(STC_LISTA_FICH *)malloc(sizeof(STC_LISTA_FICH));
	if(plf1!=NULL) plf1->sgte_fich=plf2;
	else plf_primer=plf2;

	/* mete nombre de fichero en lista */
	strcpy(plf2->fich,find.name);

	/* rellena con espacios */
	for(i=strlen(plf2->fich); i<(LNG_NFICH+1); i++) {
		plf2->fich[i]=' ';
	}
	plf2->fich[i]='\0';

	plf2->sgte_fich=NULL;
	nelem++;

	/* guarda direcci¢n de nuevo elemento */
	plf1=plf2;
	/* busca siguiente */
	encontrado=_dos_findnext(&find);
}

*lf=plf_primer;
return(nelem);
}

/****************************************************************************
	IMP_LISTA_FICH: imprime una lista de ficheros.
	  Entrada:      'lf' puntero a primer elemento de la lista
			'x', 'y' origen, en pantalla, de la lista
			'colorf', 'color' colores de fondo y primer plano
			'prelem' primer elemento a imprimir
			'nelem' n£mero de elementos a imprimir
****************************************************************************/
void imp_lista_fich(STC_LISTA_FICH *lf, short x, short y, short colorf,
  short color, int prelem, int nelem)
{
int i;

/* oculta el puntero */
vis_puntero(ESCONDE);

/* si pas¢ un puntero nulo, sale sin hacer nada */
if(lf==NULL) {
	vis_puntero(MUESTRA);
	return;
}

/* busca el elemento 'prelem' */
for(i=0; i<prelem; i++) lf=lf->sgte_fich;

for(i=0; i<nelem; i++) {
	_moveto(x,y);
	imp_str(lf->fich,colorf,color,CHR_NORM);
	/* si es £ltimo elemento, sale independientemente del n£mero */
	/* de elementos dado */
	if(lf->sgte_fich==NULL) break;
	/* puntero a siguiente elemento */
	lf=lf->sgte_fich;
	/* siguiente l¡nea */
	y+=CFICH_SEPLF;
}

/* rellena lo que queda de la zona de impresi¢n de ficheros */
for(++i; i<nelem; i++) {
	y+=CFICH_SEPLF;
	_moveto(x,y);
	imp_str("              ",colorf,color,CHR_NORM);
}

/* muestra el puntero */
vis_puntero(MUESTRA);

}

/****************************************************************************
	BORRA_LISTA_FICH: borra de memoria una lista de ficheros.
	  Entrada:      'lf' puntero a primer elemento de la lista
****************************************************************************/
void borra_lista_fich(STC_LISTA_FICH *lf)
{
STC_LISTA_FICH *sgte_fich;

while(lf!=NULL) {
	/* guarda puntero a siguiente entrada */
	sgte_fich=lf->sgte_fich;
	/* borra elemento de memoria */
	free(lf);
	/* recupera puntero a siguiente entrada */
	lf=sgte_fich;
}

}

/****************************************************************************
	RECORTA_ESPACIOS: elimina los espacios iniciales y finales de una
	  cadena.
	  Entrada:      'cad' puntero a la cadena
	  Salida:       cadena sin espacios iniciales ni finales
****************************************************************************/
void recorta_espacios(char *cad)
{
char *pcad;

/* busca primer car cter que no sea espacio */
for(pcad=cad; *pcad==' '; pcad++);
/* elimina espacios iniciales */
strcpy(cad,pcad);

/* busca hacia atr s el primer car cter que no sea espacio */
for(pcad=cad+strlen(cad)-1; *pcad==' '; pcad--);
/* elimina espacios finales */
*(pcad+1)='\0';

}

/****************************************************************************
	CALCULA_TAM_CUADRO: calcula el tama¤o de una ventana de acuerdo
	  al texto que se deber  escribir en ella.
	  Entrada:      'texto' puntero al texto a escribir en la ventana
	  Salida:       'ancho', 'alto' actualizados para contener las
			dimensiones de la ventana, suponiendo que el texto a
			imprimir lo sea con caracteres de 8x8
****************************************************************************/
void calcula_tam_cuadro(char *texto, short *ancho, short *alto)
{
short longitud=0, max_long=0, num_lin=0;

/* si no es cadena vac¡a es que al menos hay una l¡nea de texto */
if(*texto) num_lin=1;

/* recorre la cadena hasta el final */
while(*texto) {
	if(*texto=='\n') {
		/* incrementa n£mero de l¡neas */
		num_lin++;
		/* mira si la longitud de la cadena excede la m xima */
		/* hasta el momento */
		if(longitud>max_long) max_long=longitud;
		longitud=0;
	}
	else longitud++;

	texto++;
}

/* comprueba longitud de £ltima l¡nea */
if(longitud>max_long) max_long=longitud;

/* calcula dimensiones de la ventana */
*ancho=max_long*8;
*alto=num_lin*8;

}

/****************************************************************************
	IMP_TEXTO_CENTRO: imprime texto en una ventana, centrado.
	  Entrada:      'texto' puntero al texto a imprimir
			'x', 'y' origen de la ventana
			'ancho', 'alto' dimensiones de la ventana
			'colorf', 'color' colores de fondo y primer plano
			de la ventana
****************************************************************************/
void imp_texto_centro(char *texto, short x, short y, short ancho,
  short alto, short colorf, short color)
{
char buf_txt[81];
int i;
short y0;

vis_puntero(ESCONDE);

y0=y;

while(*texto) {
	i=0;
	/* copia frase en buffer */
	while(*texto && (*texto!='\n')) buf_txt[i++]=*texto++;

	/* salta el '\n' */
	if(*texto=='\n') texto++;

	/* imprime frase centrada */
	buf_txt[i]='\0';
	_moveto(x+((ancho-(strlen(buf_txt)*8))/2),y0);
	imp_str(buf_txt,colorf,color,CHR_NORM);

	/* siguiente l¡nea */
	y0+=8;
}

vis_puntero(MUESTRA);

}

/****************************************************************************
	ABRE_VENTANA: crea y abre una ventana en pantalla.
	  Entrada:      'ventana' puntero a estructura con la definici¢n
			de la ventana
	  Salida:       0 si no pudo abrir la ventana (falta de memoria)
			1 si la pudo abrir
****************************************************************************/
int abre_ventana(STC_VENTANA *ventana)
{
long mem;
short x1, y1;
struct videoconfig vid;

/* oculta puntero */
vis_puntero(ESCONDE);

/* coge informaci¢n del sistema de v¡deo */
_getvideoconfig(&vid);

/* mira si se quiere centrar la ventana en X */
if(ventana->x==V_CENT) ventana->x=(vid.numxpixels-ventana->ancho)/2;
/* mira si se quiere centrar la ventana en Y */
if(ventana->y==V_CENT) ventana->y=(vid.numypixels-ventana->alto)/2;

x1=ventana->x+ventana->ancho-1;
y1=ventana->y+ventana->alto-1;

/* calcula el n£mero de bytes que ocupa el fondo sobre el que se */
/* superpondr  la ventana */
mem=_imagesize(ventana->x,ventana->y,x1,y1);

/* intenta reservar la memoria para guardar fondo */
ventana->buff=(char _huge *)halloc(mem,sizeof(char));
if(ventana->buff==NULL) {
	/* muestra puntero */
	vis_puntero(MUESTRA);
	return(0);
}

/* guarda fondo */
_getimage(ventana->x,ventana->y,x1,y1,ventana->buff);

/* dibuja ventana */
dibuja_cuadro_boton(ventana->x,ventana->y,ventana->ancho,ventana->alto,
  ventana->color_fondo,ventana->color_s1,ventana->color_s2,ON,ON);

/* muestra puntero */
vis_puntero(MUESTRA);

return(1);
}

/****************************************************************************
	CIERRA_VENTANA: cierra una ventana elimin ndola de pantalla y
	  restaura el fondo.
	  Entrada:      'ventana' puntero a estructura con la definici¢n
			de la ventana
	  Salida:       0 si no es una definici¢n v lida de ventana (no
			se abri¢ con la funci¢n correspondiente)
			1 si la pudo cerrar
****************************************************************************/
int cierra_ventana(STC_VENTANA *ventana)
{

/* si la ventana est  cerrada ya o no fue abierta, sale con error */
if(ventana->buff==NULL) return(0);

/* oculta puntero */
vis_puntero(ESCONDE);

/* recupera fondo de pantalla */
_putimage(ventana->x,ventana->y,ventana->buff,_GPSET);

/* libera memoria y reinicializa puntero */
hfree(ventana->buff);
ventana->buff=NULL;

/* muestra puntero */
vis_puntero(MUESTRA);

return(1);
}

/****************************************************************************
	CUADRO_FICH: crea y gestiona un cuadro de selecci¢n de ficheros.
	  Entrada:      'x', 'y' origen del cuadro
			'color_fondo' color de fondo
			'color_pplano' color de los textos
			'color_s1' color de sombra d‚bil
			'color_s2' color de sombra fuerte
			'color_tecla' color de la tecla de activaci¢n
			'texto' puntero a texto para bot¢n de acci¢n
			'nomb_fich' puntero a buffer d¢nde dejar nombre de
			fichero seleccionado (debe ser de al menos
			_MAX_PATH) caracteres; si no seleccion¢ fichero,
			devolver  cadena vac¡a
	  Salida:       0 si no se pudo crear el cuadro
			CF_ACCION si se cerr¢ el cuadro con el bot¢n
			  de acci¢n
			CF_SALIDA si se cerr¢ el cuadro con el bot¢n
			  de salida
****************************************************************************/
int cuadro_fich(short x, short y, short color_fondo,short color_pplano,
  short color_s1, short color_s2, short color_tecla,char *texto,
  char *nomb_fich)
{
STC_VENTANA v;
STC_BBOTON b;
STC_CODBOTON boton;
static char nfich[LNG_NFICH]="*.*";
static char rfich[_MAX_PATH];
char dir_orig[_MAX_PATH], nombre[LNG_NFICH+2];
STC_LISTA_FICH *lf, *pelem;
int unid_orig, nelem, act_elem=0, elem, i;
STC_PUNTERO puntero;

/* guarda unidad actual */
unid_orig=_getdrive();

/* inicializa buffer para nombre de fichero seleccionado */
*nomb_fich='\0';

/* crea la ventana */
v.x=x;
v.y=y;
v.color_fondo=color_fondo;
v.color_pplano=color_pplano;
v.color_s1=color_s1;
v.color_s2=color_s2;
v.ancho=CFICH_ANCHO;
v.alto=CFICH_ALTO;

/* abre la ventana que contendr  el cuadro */
if(!abre_ventana(&v)) return(0);

/* barra de botones */
b.tipo=BBOTON_INDF;
b.color_fondo=v.color_fondo;
b.color_pplano=v.color_pplano;
b.color_s1=v.color_s1;
b.color_s2=v.color_s2;
b.color_tecla=color_tecla;
b.num_botones=CFICH_NBOT;

/* bot¢n 0, 'Fichero' */
strcpy(b.bot[0].texto_bot,"&Fich.");
b.bot[0].icono=0;
b.bot[0].nump_ico=0;
b.bot[0].modo=ON;
b.bot[0].tipo=ACT;
b.bot[0].xb=v.x+CFICH_MARG-1;
b.bot[0].yb=v.y+CFICH_MARG-1;
b.bot[0].ancho=CFICH_ANCHB;
b.bot[0].alto=CFICH_ALTB;

/* bot¢n 1, zona entrada nombre de fichero */
b.bot[1].texto_bot[0]='\0';
b.bot[1].icono=0;
b.bot[1].nump_ico=0;
b.bot[1].modo=OFF;
b.bot[1].tipo=ACT;
b.bot[1].xb=v.x+CFICH_MARG+CFICH_ANCHB+CFICH_SEPAR-1;
b.bot[1].yb=v.y+CFICH_MARG-1;
b.bot[1].ancho=CFICH_ANCHE;
b.bot[1].alto=CFICH_ALTB;

/* bot¢n 2, 'Ruta' */
strcpy(b.bot[2].texto_bot,"&Ruta");
b.bot[2].icono=0;
b.bot[2].nump_ico=0;
b.bot[2].modo=ON;
b.bot[2].tipo=ACT;
b.bot[2].xb=v.x+CFICH_MARG-1;
b.bot[2].yb=v.y+CFICH_MARG+CFICH_ALTB+CFICH_SEPAR-1;
b.bot[2].ancho=CFICH_ANCHB;
b.bot[2].alto=CFICH_ALTB;

/* bot¢n 3, zona entrada ruta de fichero */
b.bot[3].texto_bot[0]='\0';
b.bot[3].icono=0;
b.bot[3].nump_ico=0;
b.bot[3].modo=OFF;
b.bot[3].tipo=ACT;
b.bot[3].xb=v.x+CFICH_MARG+CFICH_ANCHB+CFICH_SEPAR-1;
b.bot[3].yb=v.y+CFICH_MARG+CFICH_ALTB+CFICH_SEPAR-1;
b.bot[3].ancho=CFICH_ANCHE;
b.bot[3].alto=CFICH_ALTB;

/* bot¢n 4, para presentar lista de ficheros */
b.bot[4].texto_bot[0]='\0';
b.bot[4].icono=0;
b.bot[4].nump_ico=0;
b.bot[4].modo=OFF;
b.bot[4].tipo=ACT;
b.bot[4].xb=v.x+CFICH_MARG-1;
b.bot[4].yb=v.y+CFICH_MARG+(CFICH_ALTB*2)+(CFICH_SEPAR*2)-1;
b.bot[4].ancho=CFICH_ANCHC;
b.bot[4].alto=CFICH_ALTC;

/* bot¢n 5, bot¢n con la acci¢n */
strcpy(b.bot[5].texto_bot,texto);
b.bot[5].icono=0;
b.bot[5].nump_ico=0;
b.bot[5].modo=ON;
b.bot[5].tipo=ACT;
b.bot[5].xb=v.x+CFICH_MARG+CFICH_ANCHC+(CFICH_ANCHB/3)+(CFICH_SEPAR*2)-1;
b.bot[5].yb=v.y+CFICH_MARG+(CFICH_ALTB*2)+(CFICH_SEPAR*2)-1;
b.bot[5].ancho=CFICH_ANCHB;
b.bot[5].alto=CFICH_ALTB;

/* bot¢n 6, bot¢n de salida */
strcpy(b.bot[6].texto_bot,"&Salir");
b.bot[6].icono=0;
b.bot[6].nump_ico=0;
b.bot[6].modo=ON;
b.bot[6].tipo=ACT;
b.bot[6].xb=v.x+CFICH_MARG+CFICH_ANCHC+(CFICH_ANCHB/3)+(CFICH_SEPAR*2)-1;
b.bot[6].yb=v.y+CFICH_MARG+(CFICH_ALTB*3)+(CFICH_SEPAR*3)-1;
b.bot[6].ancho=CFICH_ANCHB;
b.bot[6].alto=CFICH_ALTB;

/* bot¢n 7, bot¢n desplazamiento arriba */
strcpy(b.bot[7].texto_bot,"&8");
b.bot[7].icono=cf_icof_arr;
b.bot[7].nump_ico=PCF_ICOF_ARR;
b.bot[7].modo=ON;
b.bot[7].tipo=ACT;
b.bot[7].xb=v.x+CFICH_MARG+CFICH_ANCHC+CFICH_SEPAR-1;
b.bot[7].yb=v.y+CFICH_MARG+(CFICH_ALTB*2)+(CFICH_SEPAR*2)-1;
b.bot[7].ancho=CFICH_ANCHB/3;
b.bot[7].alto=CFICH_ALTB;

/* bot¢n 8, bot¢n desplazamiento abajo */
strcpy(b.bot[8].texto_bot,"&2");
b.bot[8].icono=cf_icof_abj;
b.bot[8].nump_ico=PCF_ICOF_ABJ;
b.bot[8].modo=ON;
b.bot[8].tipo=ACT;
b.bot[8].xb=v.x+CFICH_MARG+CFICH_ANCHC+CFICH_SEPAR-1;
b.bot[8].yb=v.y+v.alto-CFICH_MARG-CFICH_ALTB-1;
b.bot[8].ancho=CFICH_ANCHB/3;
b.bot[8].alto=CFICH_ALTB;

/* bot¢n 9, bot¢n desplazamiento arriba r pido */
strcpy(b.bot[9].texto_bot,"&9");
b.bot[9].icono=cf_icof_arrr;
b.bot[9].nump_ico=PCF_ICOF_ARRR;
b.bot[9].modo=ON;
b.bot[9].tipo=ACT;
b.bot[9].xb=v.x+CFICH_MARG+CFICH_ANCHC+CFICH_SEPAR-1;
b.bot[9].yb=v.y+CFICH_MARG+(CFICH_ALTB*3)+(CFICH_SEPAR*3)-1;
b.bot[9].ancho=CFICH_ANCHB/3;
b.bot[9].alto=CFICH_ALTB;

/* bot¢n 10, bot¢n desplazamiento abajo r pido */
strcpy(b.bot[10].texto_bot,"&3");
b.bot[10].icono=cf_icof_abjr;
b.bot[10].nump_ico=PCF_ICOF_ABJR;
b.bot[10].modo=ON;
b.bot[10].tipo=ACT;
b.bot[10].xb=v.x+CFICH_MARG+CFICH_ANCHC+CFICH_SEPAR-1;
b.bot[10].yb=v.y+v.alto-CFICH_MARG-(CFICH_ALTB*2)-CFICH_SEPAR-1;
b.bot[10].ancho=CFICH_ANCHB/3;
b.bot[10].alto=CFICH_ALTB;

/* oculta puntero */
vis_puntero(ESCONDE);

/* crea la barra de botones */
crea_barra_boton(&b);

/* dibuja los botones */
dibuja_barra_boton(&b);

/* pone nombre de fichero y ruta actual */
_moveto(b.bot[1].xb+6,b.bot[1].yb+((b.bot[1].alto-8)/2));
imp_str(nfich,b.color_fondo,b.color_pplano,CHR_NORM);

/* construye ruta al directorio actual */
getcwd(rfich,_MAX_PATH-1);

/* la guarda */
strcpy(dir_orig,rfich);

_moveto(b.bot[3].xb+6,b.bot[3].yb+((b.bot[3].alto-8)/2));
if(strlen(rfich)>(size_t)((b.bot[3].ancho-12)/8))
  impn_str(rfich,(b.bot[3].ancho-12)/8,b.color_fondo,b.color_pplano,
    CHR_NORM);
else imp_str(rfich,b.color_fondo,b.color_pplano,CHR_NORM);

/* crea e imprime lista de ficheros */
nelem=lista_fich(rfich,nfich,&lf);
imp_lista_fich(lf,b.bot[4].xb+6,b.bot[4].yb+6,b.color_fondo,
  b.color_pplano,act_elem,CF_NELEM);

/* muestra puntero */
vis_puntero(MUESTRA);

while(1) {
	do {
		boton=esta_pulsado(&b,1);
	} while(boton.boton==NO_BOTON);

	switch(boton.boton) {
		case 0 :
		case 1 :        /* zona nombre fichero */
			lee_input(b.bot[1].xb+6,
			  b.bot[1].yb+((b.bot[1].alto-8)/2),
			  nfich,LNG_NFICH-1,b.bot[1].ancho-12,
			  b.color_fondo,b.color_pplano);
			/* actualiza lista de ficheros */
			borra_lista_fich(lf);
			dibuja_boton(&b.bot[4],b.color_fondo,
			  b.color_pplano,b.color_s1,b.color_s2,
			  b.color_tecla);
			nelem=lista_fich(rfich,nfich,&lf);
			act_elem=0;
			imp_lista_fich(lf,b.bot[4].xb+6,b.bot[4].yb+6,
			  b.color_fondo,b.color_pplano,act_elem,
			  CF_NELEM);
			break;
		case 2 :
		case 3 :        /* zona ruta fichero */
			vis_puntero(ESCONDE);

			lee_input(b.bot[3].xb+6,
			  b.bot[3].yb+((b.bot[3].alto-8)/2),
			  rfich,_MAX_PATH-1,b.bot[3].ancho-12,
			  b.color_fondo,b.color_pplano);
			recorta_espacios(rfich);

			/* intenta cambiar de unidad */
			i=_getdrive();
			if(*(rfich+1)==':') {
				/* si puede cambiar de unidad */
				/* intenta cambiar de directorio */
				/* si no puede, vuelve a unidad actual */
				if(!_chdrive(toupper(*rfich)-'A'+1))
				  if(chdir(rfich)) _chdrive(i);
			}

			/* borra zona de ruta */
			dibuja_boton(&b.bot[3],b.color_fondo,
			  b.color_pplano,b.color_s1,b.color_s2,
			  b.color_tecla);

			/* coge directorio actual */
			getcwd(rfich,_MAX_PATH-1);

			/* imprime directorio actual */
			_moveto(b.bot[3].xb+6,
			  b.bot[3].yb+((b.bot[3].alto-8)/2));
			i=strlen(rfich);
			if(i>((b.bot[3].ancho-12)/8))
			  impn_str(rfich,(b.bot[3].ancho-12)/8,
			    b.color_fondo,b.color_pplano,CHR_NORM);
			else imp_str(rfich,b.color_fondo,b.color_pplano,
			  CHR_NORM);

			/* actualiza lista de ficheros */
			borra_lista_fich(lf);
			dibuja_boton(&b.bot[4],b.color_fondo,
			  b.color_pplano,b.color_s1,b.color_s2,
			  b.color_tecla);
			nelem=lista_fich(rfich,nfich,&lf);
			act_elem=0;
			imp_lista_fich(lf,b.bot[4].xb+6,b.bot[4].yb+6,
			  b.color_fondo,b.color_pplano,act_elem,
			  CF_NELEM);

			vis_puntero(MUESTRA);

			break;
		case 4 :        /* lista de ficheros */
			vis_puntero(ESCONDE);

			/* calcula coordenada Y de pulsaci¢n respecto a */
			/* origen de la lista */
			boton.y-=4;
			/* n£mero de elemento de 0 a (CF_NELEM-1) */
			elem=boton.y/CFICH_SEPLF;

			/* reimprime lista */
			imp_lista_fich(lf,b.bot[4].xb+6,b.bot[4].yb+6,
			  b.color_fondo,b.color_pplano,act_elem,
			  CF_NELEM);

			/* recuadra elemento seleccionado */
			_setcolor(b.color_pplano);
			_rectangle(_GBORDER,b.bot[4].xb+6,
			  b.bot[4].yb+(elem*10)+6,b.bot[4].xb+117,
			  b.bot[4].yb+(elem*10)+13);

			/* busca el elemento 'act_elem' + 'elem' */
			pelem=lf;
			for(i=0; i<(act_elem+elem); i++)
			  pelem=pelem->sgte_fich;
			/* recoge nombre de fichero */
			strcpy(nombre,&pelem->fich[0]);

			/* si es nombre de directorio */
			if(nombre[0]==CF_CHRDIR1) {
				/* anula nombre de fichero seleccionado */
				*nomb_fich='\0';

				/* traslada hacia izquierda */
				for(i=1; nombre[i]; i++) nombre[i-1]=nombre[i];
				/* sustituye corchete final por '\0' */
				i=strcspn(nombre,CF_STRDIR2);
				nombre[i]='\0';

				/* cambia de directorio */
				chdir(nombre);

				/* borra zona de ruta */
				dibuja_boton(&b.bot[3],b.color_fondo,
				  b.color_pplano,b.color_s1,
				  b.color_s2,b.color_tecla);

				/* construye ruta al directorio actual */
				getcwd(rfich,_MAX_PATH);

				_moveto(b.bot[3].xb+6,
				  b.bot[3].yb+((b.bot[3].alto-8)/2));
				i=strlen(rfich);
				if(i>((b.bot[3].ancho-12)/8))
				  impn_str(rfich,(b.bot[3].ancho-12)/8,
				    b.color_fondo,b.color_pplano,
				    CHR_NORM);
				else imp_str(rfich,b.color_fondo,
				  b.color_pplano,CHR_NORM);

				/* borra lista de ficheros actual */
				borra_lista_fich(lf);

				/* crea lista nueva */
				nelem=lista_fich(rfich,nfich,&lf);
				act_elem=0;
				imp_lista_fich(lf,b.bot[4].xb+6,
				  b.bot[4].yb+6,b.color_fondo,
				  b.color_pplano,act_elem,CF_NELEM);
			}
			/* si es nombre de fichero */
			else {
				/* guarda nombre de fichero con ruta */
				strcpy(nomb_fich,rfich);
				/* si no tiene '\' al final, se la a¤ade */
				i=strlen(nomb_fich)-1;
				if(nomb_fich[i]!='\\') strcat(nomb_fich,"\\");
				strcat(nomb_fich,nombre);
			}

			vis_puntero(MUESTRA);

			/* espera hasta que suelte bot¢n del rat¢n */
			if(hay_raton()) do {
				coge_pos_puntero(&puntero);
			} while(puntero.botones & PULSADO_IZQ);

			break;
		case 5 :        /* acci¢n */
			/* borra lista de ficheros */
			borra_lista_fich(lf);
			/* cierra el cuadro */
			cierra_ventana(&v);

			/* vuelve a la unidad y directorio originales */
			_chdrive(unid_orig);
			chdir(dir_orig);

			/* si el nombre de fichero elegido es nulo */
			/* copia contenido de cuadro ruta y nombre */
			if(!*nomb_fich) {
				strcpy(nomb_fich,rfich);
				/* si no tiene '\' al final, se la a¤ade */
				i=strlen(nomb_fich)-1;
				if(nomb_fich[i]!='\\') strcat(nomb_fich,"\\");
				strcat(nomb_fich,nfich);
			}

			return(CF_ACCION);
		case 6 :        /* salir */
			/* borra lista de ficheros */
			borra_lista_fich(lf);
			/* cierra el cuadro */
			cierra_ventana(&v);

			/* vuelve a la unidad y directorio originales */
			_chdrive(unid_orig);
			chdir(dir_orig);

			return(CF_SALIDA);
		case 7 :        /* lista arriba */
			if(act_elem>0) {
				act_elem--;
				imp_lista_fich(lf,b.bot[4].xb+6,
				  b.bot[4].yb+6,b.color_fondo,
				  b.color_pplano,act_elem,CF_NELEM);
			}
			break;
		case 8 :        /* lista abajo */
			if((act_elem+CF_NELEM)<nelem) {
				act_elem++;
				imp_lista_fich(lf,b.bot[4].xb+6,
				  b.bot[4].yb+6,b.color_fondo,
				  b.color_pplano,act_elem,CF_NELEM);
			}
			break;
		case 9 :        /* lista arriba r pido */
			act_elem-=CF_NELEM;
			if(act_elem<0) act_elem=0;
			imp_lista_fich(lf,b.bot[4].xb+6,b.bot[4].yb+6,
			  b.color_fondo,b.color_pplano,act_elem,
			  CF_NELEM);
			break;
		case 10 :       /* lista abajo r pido */
			act_elem+=CF_NELEM;
			if(act_elem>(nelem-CF_NELEM)) act_elem=nelem-CF_NELEM;
			imp_lista_fich(lf,b.bot[4].xb+6,b.bot[4].yb+6,
			  b.color_fondo,b.color_pplano,act_elem,
			  CF_NELEM);
			break;
	}
}

}

/****************************************************************************
	CUADRO_AVISO: crea un cuadro para presentar mensajes de aviso.
	  Entrada:      'aviso' puntero a la cadena con el mensaje de aviso;
			cada l¡nea del mensaje se mostrar  centrada en la
			ventana, cuyas dimensiones se calcular n de acuerdo
			con el mensaje
			'x', 'y' posici¢n de la ventana en pantalla
			'color_fondo' color de fondo de la ventana
			'color_pplano' color de primer plano de la ventana
			'color_s1', 'color_s2' color de sombras d‚bil y
			fuerte de la ventana
			'color_tecla' color de tecla de activaci¢n
	  Salida:       0 si no se pudo crear el cuadro
			1 en otro caso
****************************************************************************/
int cuadro_aviso(char *aviso, short x, short y, short color_fondo,
  short color_pplano, short color_s1, short color_s2, short color_tecla)
{
short ancho, alto;
STC_VENTANA v;
STC_BBOTON b;
STC_CODBOTON pulsado;

/* calcula las dimensiones de la ventana */
calcula_tam_cuadro(aviso,&ancho,&alto);

/* comprueba que la anchura no sea menor que la del bot¢n */
if(ancho<AV_ANCHOBOT) ancho=AV_ANCHOBOT;

/* crea la ventana */
v.x=x;
v.y=y;
v.ancho=ancho+14;
v.alto=alto+AV_ALTOBOT+14;
v.color_fondo=color_fondo;
v.color_pplano=color_pplano;
v.color_s1=color_s1;
v.color_s2=color_s2;

/* abre la ventana que contendr  en cuadro de aviso */
if(!abre_ventana(&v)) return(0);

/* crea barra de botones con un bot¢n */
b.x=v.x+((v.ancho-AV_ANCHOBOT)/2);
b.y=v.y+v.alto-AV_ALTOBOT-3;
b.tipo=BBOTON_HORZ;
b.color_fondo=color_fondo;
b.color_pplano=color_pplano;
b.color_s1=color_s1;
b.color_s2=color_s2;
b.color_tecla=color_tecla;
b.ancho_bot=AV_ANCHOBOT;
b.alto_bot=AV_ALTOBOT;
b.num_botones=1;
/* define bot¢n */
strcpy(b.bot[0].texto_bot,"&VALE");
b.bot[0].icono=0;
b.bot[0].nump_ico=0;
b.bot[0].modo=ON;
b.bot[0].tipo=ACT;

crea_barra_boton(&b);

/* dibuja el cuadro de aviso en pantalla */
dibuja_barra_boton(&b);
imp_texto_centro(aviso,v.x,v.y+3,v.ancho,v.alto,v.color_fondo,v.color_pplano);

/* espera hasta que se pulse el bot¢n */
do {
	pulsado=esta_pulsado(&b,1);
} while(pulsado.boton==NO_BOTON);

cierra_ventana(&v);

return(1);
}

/****************************************************************************
	CUADRO_SIONO: crea un cuadro de selecci¢n SI o NO.
	  Entrada:      'mensaje' puntero a la cadena con el mensaje;
			cada l¡nea del mensaje se mostrar  centrada en la
			ventana, cuyas dimensiones se calcular n de acuerdo
			con el mensaje
			'x', 'y' posici¢n de la ventana en pantalla
			'color_fondo' color de fondo de la ventana
			'color_pplano' color de primer plano de la ventana
			'color_s1', 'color_s2' color de sombras d‚bil y
			fuerte de la ventana
			'color_tecla' color de tecla de activaci¢n
	  Salida:       0 si no se pudo crear el cuadro
			SIONO_SI si puls¢ el bot¢n SI
			SIONO_NO si puls¢ el bot¢n NO
****************************************************************************/
int cuadro_siono(char *mensaje, short x, short y, short color_fondo,
  short color_pplano, short color_s1, short color_s2, short color_tecla)
{
short ancho, alto;
STC_VENTANA v;
STC_BBOTON b;
STC_CODBOTON pulsado;

/* calcula las dimensiones de la ventana */
calcula_tam_cuadro(mensaje,&ancho,&alto);

/* comprueba que la anchura no sea menor que la de los botones */
if(ancho<(AV_ANCHOBOT*2)) ancho=AV_ANCHOBOT*2;

/* crea la ventana */
v.x=x;
v.y=y;
v.ancho=ancho+14;
v.alto=alto+AV_ALTOBOT+14;
v.color_fondo=color_fondo;
v.color_pplano=color_pplano;
v.color_s1=color_s1;
v.color_s2=color_s2;

/* abre la ventana que contendr  en cuadro de aviso */
if(!abre_ventana(&v)) return(0);

/* crea barra de botones con dos botones */
b.x=v.x+((v.ancho-(AV_ANCHOBOT*2))/2);
b.y=v.y+v.alto-AV_ALTOBOT-3;
b.tipo=BBOTON_HORZ;
b.color_fondo=color_fondo;
b.color_pplano=color_pplano;
b.color_s1=color_s1;
b.color_s2=color_s2;
b.color_tecla=color_tecla;
b.ancho_bot=AV_ANCHOBOT;
b.alto_bot=AV_ALTOBOT;
b.num_botones=2;
/* define bot¢n SI */
strcpy(b.bot[0].texto_bot,"&SI");
b.bot[0].icono=0;
b.bot[0].nump_ico=0;
b.bot[0].modo=ON;
b.bot[0].tipo=ACT;
/* define bot¢n NO */
strcpy(b.bot[1].texto_bot,"&NO");
b.bot[1].icono=0;
b.bot[1].nump_ico=0;
b.bot[1].modo=ON;
b.bot[1].tipo=ACT;

crea_barra_boton(&b);

/* dibuja el cuadro de aviso en pantalla */
dibuja_barra_boton(&b);
imp_texto_centro(mensaje,v.x,v.y+3,v.ancho,v.alto,v.color_fondo,
  v.color_pplano);

/* espera hasta que se pulse el bot¢n */
do {
	pulsado=esta_pulsado(&b,1);
} while(pulsado.boton==NO_BOTON);

cierra_ventana(&v);

if(pulsado.boton==0) return(SIONO_SI);
else return(SIONO_NO);

}

