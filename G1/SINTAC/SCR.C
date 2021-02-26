/****************************************************************************
				     SCR.C

	Funciones para impresi¢n r pida en pantalla, as¡ como para
	crear y manejar ventanas de texto.

			    (c)1993 JSJ Soft Ltd.

	Las siguientes funciones son p£blicas:
		- pon_cursor: coloca el cursor en una posici¢n de pantalla
		- imp_linea_scr: imprime una l¡nea de texto en pantalla
		- borra_linea_scr: borra una l¡nea de pantalla
		- borra_scr: borra la pantalla
		- dibuja_ventana: dibuja una ventana
		- lee_input: lee una cadena de caracteres por teclado o
		    edita una existente
		- cuadro_fich: crea y gestiona un cuadro de selecci¢n de
		    ficheros
****************************************************************************/

#include <stdlib.h>
#include <bios.h>
#include <dos.h>
#include <malloc.h>
#include <string.h>
#include <direct.h>
#include "scr.h"

/*** Prototipos de funciones internas ***/
static int lista_fich(char *ruta, char *mascara, STC_LISTA_FICH *(*lf));
static void imp_lista_fich(STC_LISTA_FICH *lf, short f, short c, BYTE attr1,
  BYTE attr2, int prelem, int elem_selecc, int nelem);
static void borra_lista_fich(STC_LISTA_FICH *lf);

/****************************************************************************
	LISTA_FICH: crea una lista de ficheros.
	  Entrada:      'ruta' ruta del directorio
			'mascara' m scara de b£squeda de ficheros
			'lf' puntero variable para contener puntero a primer
			elemento de la lista
	  Salida:       n£mero de elementos de la lista
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

/* si no tiene '\' al final, se lo a¤ade */
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
			'f', 'c' origen, en pantalla, de la lista
			'attr1' color normal
			'attr2' color de elemento seleccionado
			'prelem' primer elemento a imprimir
			'elem_selecc' elemento seleccionado de la lista
			'nelem' n£mero de elementos a imprimir
****************************************************************************/
void imp_lista_fich(STC_LISTA_FICH *lf, short f, short c, BYTE attr1,
  BYTE attr2, int prelem, int elem_selecc, int nelem)
{
int i;

/* si pas¢ un puntero nulo, sale sin hacer nada */
if(lf==NULL) return;

/* busca el elemento 'prelem' */
for(i=0; i<prelem; i++) lf=lf->sgte_fich;

for(i=0; i<nelem; i++, f++) {
	if(i==(elem_selecc-prelem)) imp_linea_scr(f,c,attr2,lf->fich,
	  LNG_NFICH+1);
	else imp_linea_scr(f,c,attr1,lf->fich,LNG_NFICH+1);

	/* si es £ltimo elemento, sale independientemente del n£mero */
	/* de elementos dado */
	if(lf->sgte_fich==NULL) break;
	/* puntero a siguiente elemento */
	lf=lf->sgte_fich;
}

/* rellena lo que queda de la zona de impresi¢n de ficheros */
for(++i; i<nelem; i++) {
	f++;
	imp_linea_scr(f,c,attr1,"              ",LNG_NFICH+1);
}

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
	PON_CURSOR: coloca el cursor en una posici¢n de pantalla.
	  Entrada:      'fil', 'col' fila y columna del cursor
****************************************************************************/
void pon_cursor(BYTE fil, BYTE col)
{

_asm {
	mov ah,02h              ; funci¢n definir posici¢n del cursor
	mov bh,0                ; supone p gina 0
	mov dh,fil              ; DH = fila del cursor
	mov dl,col              ; DL = columna del cursor
	int 10h
}

}

#pragma check_pointer(off)
/****************************************************************************
	IMP_LINEA_SCR: imprime una l¡nea en pantalla.
	  Entrada:      'fil', 'col' posici¢n de impresi¢n
			'attr' color de impresi¢n
			'lin' texto a imprimir
			'ncar' n£mero de caracteres a imprimir (si 'lin'
			es de menor longitud que 'ncar' rellena hasta 'ncar')
****************************************************************************/
void imp_linea_scr(short fil, short col, BYTE attr, char *lin, int ncar)
{
BYTE _far *pscr;
int i=0;

/* calcula direcci¢n en buffer de v¡deo */
pscr=(BYTE _far *)0xb8000000+(((fil*80)+col)*2);

/* imprime l¡nea */
while((*lin) && (i<ncar)) {
	/* imprime car cter */
	*pscr++=*lin;
	/* y su atributo */
	*pscr++=attr;
	/* siguiente car cter */
	lin++;
	/* incrementa n£mero de caracteres imrpesos */
	i++;
}

/* rellena con espacios hasta completar 'ncar' caracteres */
for(; i<ncar; i++) {
	/* imprime espacio */
	*pscr++=' ';
	/* y atributo */
	*pscr++=attr;
}

}
#pragma check_pointer()

#pragma check_pointer(off)
/****************************************************************************
	BORRA_LINEA_SCR: borra una l¡nea de pantalla.
	  Entrada:      'linea' l¡nea a borrar
			'attr' atributo
****************************************************************************/
void borra_linea_scr(short linea, BYTE attr)
{
BYTE _far *pscr;
int i;

pscr=(BYTE _far *)0xb8000000+(linea*160);

for(i=0; i<80; i++) {
	/* borra car cter */
	*pscr++=' ';
	/* y atributo */
	*pscr++=attr;
}

}
#pragma check_pointer()

/****************************************************************************
	BORRA_SCR: borra la pantalla.
	  Entrada:      'attr' atributo
****************************************************************************/
void borra_scr(BYTE attr)
{
short i;

/* borra la pantalla y coloca cursor al inicio */
for(i=0; i<25; i++) borra_linea_scr(i,attr);
pon_cursor(0,0);

}

#pragma check_pointer(off)
/****************************************************************************
	DIBUJA_VENTANA: dibuja una ventana.
	  Entrada:      'fil', 'col' posici¢n de esquina superior izquierda
			'ancho', 'alto' dimensiones del marco
			'color' color del marco
****************************************************************************/
void dibuja_ventana(short fil, short col, short ancho, short alto, BYTE color)
{
BYTE _far *pscr, _far *p;
short i;

pscr=(BYTE _far *)0xb8000000+(((fil*80)+col)*2);

p=pscr;
*p++='Ú';
*p++=color;
for(i=0; i<(ancho-2); i++) {
	*p++='Ä';
	*p++=color;
}
*p++='¿';
*p=color;

p=pscr+160;
for(i=0; i<(alto-2); i++) {
	*p='³';
	*(p+1)=color;
	*(p+((ancho-1)*2))='³';
	*(p+((ancho-1)*2)+1)=color;
	p+=160;
}

p=pscr+((alto-1)*160);
*p++='À';
*p++=color;
for(i=0; i<(ancho-2); i++) {
	*p++='Ä';
	*p++=color;
}
*p++='Ù';
*p=color;

/* borra interior */
for(i=fil+1; i<(fil+alto-1); i++) imp_linea_scr(i,col+1,color,"",ancho-2);

}
#pragma check_pointer()

/****************************************************************************
	LEE_INPUT: lee una cadena de caracteres por teclado o edita una
	  cadena ya existente.
	  Entrada:      'fil', 'col' coordenadas de pantalla d¢nde se
			imprimir  la cadena tecleada
			'cadena' puntero a buffer d¢nde se guardar 
			la cadena tecleada; si el primer car cter no es '\0'
			se ajustar  la rutina para poder editar la cadena
			'longitud' longitud de la cadena (sin contar el
			\0 final)
			'ancho' anchura de la zona de entrada
			'attr' atributo con el que se imprimir n los
			caracteres tecleados
	  Salida:       n£mero de caracteres tecleados
			-1 si se puls¢ TAB
			-2 si se puls¢ ESCAPE
****************************************************************************/
int lee_input(BYTE fil, BYTE col, char *cadena, int longitud, int ancho,
  BYTE attr)
{
char *cur, *fin, *ptr;
int num_car=0;
unsigned tecla;
BYTE ccur;

/* inicializa punteros */
cur=cadena;

/* busca final de la cadena y n£mero de caracteres */
for(fin=cadena; *fin; fin++, num_car++);

/* coordenada relativa del cursor respecto origen de caja */
ccur=0;

do {
	/* imprime l¡nea */
	imp_linea_scr(fil,col,attr,cur-ccur,ancho);

	/* imprime cursor */
	pon_cursor(fil,(BYTE)(col+ccur));

	/* recoge tecla */
	tecla=_bios_keybrd(_KEYBRD_READ);

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
				if(ccur<(BYTE)(ancho-1)) ccur++;
			}
			break;
		case 0x47 :             /* cursor origen */
			cur=cadena;
			ccur=0;
			break;
		case 0x4f :             /* cursor fin */
			cur=fin;
			if(num_car<ancho) ccur=(BYTE)num_car;
			else ccur=(BYTE)(ancho-1);
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
			return(-1);
		case 0x01 :             /* ESCAPE */
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
		if(ccur<(BYTE)(ancho-1)) ccur++;
	}

} while(tecla!=0x0d);           /* hasta que pulse RETURN */

/* imprime l¡nea */
imp_linea_scr(fil,col,attr,cadena,ancho);

return(num_car);
}

/****************************************************************************
	CUADRO_FICH: crea en una ventana de pantalla un cuadro de selecci¢n
	  de ficheros.
	  Entrada:      'f', 'c' posici¢n en pantalla de la ventana
			'attr1' color normal
			'attr2' color de elemento resaltado
			'ruta' cadena con la ruta completa al directorio
			'mascara' mascara de b£squeda de ficheros
			'mensaje' mensaje que se presentar  en la parte
			superior de la ventana
			'fich' puntero a cadena donde se dejar  el nombre y
			ruta del fichero seleccionado; su longitud debe ser
			al menos _MAX_PATH
	  Salida:       0 si se puls¢ ESCAPE, 1 en otro caso
****************************************************************************/
int cuadro_fich(short f, short c, BYTE attr1, BYTE attr2, char *ruta,
  char *mascara, char *mensaje, char *fich)
{
STC_LISTA_FICH *lf, *fact;
int num_ficheros=0, fichero=0, pr_fichero=0, i, unid_orig;
unsigned tecla;
char ruta_cf[_MAX_PATH], dir[LNG_NFICH+2], dir_orig[_MAX_PATH], nf[LNG_NFICH];
char *barra="\\";

pon_cursor(25,0);

/* guarda directorio y unidad actuales */
getcwd(dir_orig,_MAX_PATH-1);
unid_orig=_getdrive();

/* copia ruta en buffer auxiliar */
strcpy(ruta_cf,ruta);
strupr(ruta_cf);
/* si tiene '\' final, lo elimina */
i=strlen(ruta_cf)-1;
if(ruta_cf[i]=='\\') ruta_cf[i]='\0';

if(ruta_cf[1]==':') {
	i=ruta_cf[0]-'A'+1;
	_chdrive(i);
}
chdir(ruta_cf);

/* crea la lista de ficheros de directorio dado */
num_ficheros=lista_fich(ruta_cf,mascara,&lf);

dibuja_ventana(f,c,CF_ANCHO,CF_ALTO,attr1);
dibuja_ventana(f+CF_ALTO,c,CF_ANCHO,3,attr1);
i=strlen(mensaje);
imp_linea_scr(f,c+((CF_ANCHO-2-i)/2),attr1,mensaje,i);
imp_linea_scr(f+CF_ALTO,c+((CF_ANCHO-9)/2),attr1," Fichero ",9);

/* nombre de fichero seleccionado */
*nf='\0';

while(1) {
	imp_lista_fich(lf,f+1,c+2,attr1,attr2,pr_fichero,fichero,CF_ALTO-2);
	imp_linea_scr(f+CF_ALTO-1,c+1,attr1,"",CF_ANCHO-2);
	imp_linea_scr(f+CF_ALTO-1,c+2,attr1,ruta_cf,CF_ANCHO-4);
	/* busca elemento seleccionado */
	fact=lf;
	for(i=0; i<fichero; i++) fact=fact->sgte_fich;
	/* mira si es nombre de directorio */
	if(*fact->fich==CF_CHRDIR1) *nf='\0';
	else {
		for(i=0; i<(LNG_NFICH-1), fact->fich[i]!=' '; i++) {
			nf[i]=fact->fich[i];
		}
		nf[i]='\0';
	}
	imp_linea_scr(f+CF_ALTO+1,c+1,attr1,nf,CF_ANCHO-2);

	tecla=_bios_keybrd(_KEYBRD_READ);

	/* recoge c¢digo de scan */
	tecla >>= 8;
	switch(tecla) {
		case 0x48 :             /* cursor arriba */
			if(fichero==0) break;
			else if(fichero==pr_fichero) pr_fichero--;
			fichero--;
			break;
		case 0x50 :             /* cursor abajo */
			if(fichero==(num_ficheros-1)) break;
			else if(fichero==(pr_fichero+CF_ALTO-3)) pr_fichero++;
			fichero++;
			break;
		case 0x0f :             /* TAB */
			imp_lista_fich(lf,f+1,c+2,attr1,attr1,pr_fichero,
			  fichero,CF_ALTO-2);
			/* pide nombre de fichero */
			i=lee_input((BYTE)(f+CF_ALTO+1),(BYTE)(c+1),nf,
			  LNG_NFICH-1,CF_ANCHO-2,attr1);

			/* mira si puls¢ TAB o ESCAPE */
			if(i==-1) {             /* TAB */
				pon_cursor(25,0);
				break;
			}
			else if(i==-2) {        /* ESCAPE */
				*fich='\0';
				borra_lista_fich(lf);

				/* restaura directorio y unidad de origen */
				_chdrive(unid_orig);
				chdir(dir_orig);
				pon_cursor(0,0);
				return(0);
			}

			strcpy(fich,ruta_cf);
			/* si no tiene '\' final, se lo a¤ade */
			i=strlen(fich)-1;
			if(fich[i]!='\\') strcat(fich,barra);
			strcat(fich,nf);

			borra_lista_fich(lf);

			/* restaura directorio y unidad de origen */
			_chdrive(unid_orig);
			chdir(dir_orig);
			pon_cursor(0,0);
			return(1);
		case 0x01 :             /* ESCAPE */
			*fich='\0';
			borra_lista_fich(lf);

			/* restaura directorio y unidad de origen */
			_chdrive(unid_orig);
			chdir(dir_orig);
			pon_cursor(0,0);
			return(0);
		case 0x1c :             /* RETURN */
			/* busca elemento seleccionado */
			fact=lf;
			for(i=0; i<fichero; i++) fact=fact->sgte_fich;
			/* mira si es nombre de directorio */
			if(*fact->fich==CF_CHRDIR1) {
				/* coge nombre de directorio */
				strcpy(dir,fact->fich+1);
				/* sustituye corchete final por '\0' */
				i=strcspn(dir,CF_STRDIR2);
				dir[i]='\0';

				/* cambia a ese directorio */
				chdir(dir);

				/* coge nueva ruta */
				getcwd(ruta_cf,_MAX_PATH-1);

				/* borra lista actual y crea una nueva */
				borra_lista_fich(lf);
				num_ficheros=lista_fich(ruta_cf,mascara,&lf);
				pr_fichero=0;
				fichero=0;
			}
			else {
				strcpy(fich,ruta_cf);
				/* si no tiene '\' final, se lo a¤ade */
				i=strlen(fich)-1;
				if(fich[i]!='\\') strcat(fich,barra);
				strcat(fich,fact->fich);

				borra_lista_fich(lf);

				/* restaura directorio y unidad de origen */
				_chdrive(unid_orig);
				chdir(dir_orig);
				pon_cursor(0,0);
				return(1);
			}
	}
}

}
