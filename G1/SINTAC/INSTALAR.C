/****************************************************************************
			     INSTALACION SINTAC
			    (c)1993 JSJ Soft Ltd.
****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <bios.h>
#include <dos.h>
#include <direct.h>
#include <io.h>
#include <fcntl.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <malloc.h>
#include <errno.h>
#include "version.h"
#include "sintac.h"
#include "color.h"
#include "instalar.h"

/*** Variables globales ***/
STC_CFG cfg;

/* men£ de opciones */
char opciones[N_OPC][LNG_OPC]= {
	"Directorio del sistema SINTAC",
	"Directorio del GENERADOR DE CARACTERES",
	"Directorio de BASES DE DATOS",
	"Directorio de ficheros de DOCUMENTACION",
	"INSTALAR SINTAC",
	"SALIR",
};

/* directorio para ficheros de documentaci¢n */
char dir_doc[_MAX_PATH];

/* nombre de fichero de configuraci¢n */
char *Nf_Cfg=NF_CFG;

/* datos de ficheros a instalar */
STC_DATFICH datf[NUM_FICH];

/* mensajes de error */
char *MsgErr_NoFDat="\nFichero %s no encontrado.\n";
char *MsgErr_FDat="\nError en fichero %s.\n";
char *MsgErr_Dir="No se puede crear directorio ";
char *MsgErr_Fich="Error al copiar fichero ";

/*** Programa principal ***/
void main(void)
{
int i, num_fich_instal, opcion;

/* n£mero de ficheros a instalar */
num_fich_instal=lee_datos_fich(NF_DATFICH);

/* instala 'handler' de errores cr¡ticos */
_harderr(int24_hnd);

borra_scr(0x07);

/* inicializa configuraci¢n */
inicializa_cfg();

do {
	opcion=menu_instal();
	switch(opcion) {
		case 0 :                /* directorio SINTAC */
			lee_input(2,42,cfg.dir_sintac,_MAX_PATH-1,36,
			  COLOR_INPUT);
			ajusta_ndir(cfg.dir_sintac);
			break;
		case 1 :                /* directorio generador caracteres */
			lee_input(3,42,cfg.dir_gcs,_MAX_PATH-1,36,COLOR_INPUT);
			ajusta_ndir(cfg.dir_gcs);
			break;
		case 2 :                /* directorio bases de datos */
			lee_input(4,42,cfg.dir_bd,_MAX_PATH-1,36,COLOR_INPUT);
			ajusta_ndir(cfg.dir_bd);
			break;
		case 3 :                /* directorio ficheros documentaci¢n */
			lee_input(5,42,dir_doc,_MAX_PATH-1,36,COLOR_INPUT);
			ajusta_ndir(dir_doc);
			break;
		case (N_OPC-2) :        /* instalar SINTAC */
			if(instalar(num_fich_instal)==0) {
				beep();
				imp_linea_scr(LIN_INST+5,0,COLOR_MSGI,
				  "ERRORES EN INSTALACION.",80);
			}
			else imp_linea_scr(LIN_INST+5,0,COLOR_MSGI,
			  "INSTALACION CORRECTA.",80);
			imp_linea_scr(LIN_INST+6,0,COLOR_MSGI,
			  "Pulsa una tecla.",80);
			_bios_keybrd(_KEYBRD_READ);
			for(i=LIN_INST; i<24; i++) borra_linea_scr(i,0x07);
			break;
	}
} while(opcion!=(N_OPC-1));

borra_scr(0x07);
pon_cursor(0,0);

}

/****************************************************************************
	INT24_HND: rutina de manejo de errores cr¡ticos de hardware.
****************************************************************************/
void _far int24_hnd(unsigned deverror, unsigned errcode,
  unsigned _far *devhdr)
{

_hardresume(_HARDERR_FAIL);

}

/****************************************************************************
	INICIALIZA_CFG: inicializa configuraci¢n asignando valores por
	  defecto.
****************************************************************************/
void inicializa_cfg(void)
{

/* inicializa directorios */
strcpy(cfg.dir_sintac,DIRDEF_SINTAC);
strcpy(cfg.dir_gcs,DIRDEF_GENCAR);
strcpy(cfg.dir_bd,DIRDEF_DATOS);
strcpy(dir_doc,DIRDEF_DOC);

/* inicializa colores */
cfg.color_scr=COLOR_SCR;
cfg.color_editor=COLOR_EDIT;
cfg.color_regla=COLOR_REGLA;
cfg.color_cab=COLOR_CAB;
cfg.color_linst=COLOR_LINST;
cfg.color_menu=COLOR_MENU;
cfg.color_tecla=COLOR_TECLA;
cfg.color_opcion=COLOR_OPC;
cfg.color_wcomp=COLOR_WCOMP;
cfg.color_werr=COLOR_WERR;
cfg.color_err=COLOR_ERR;
cfg.color_errsel=COLOR_ERRSEL;
cfg.color_ay=COLOR_AY;
cfg.color_aptdo=COLOR_APTDO;
cfg.color_ref=COLOR_REF;

}

/****************************************************************************
	AJUSTA_NDIR: ajusta ruta a directorio a¤adiendo un '\' al final
	  si no lo tiene, y adem s convierte a may£sculas.
	  Entrada:      'dir' puntero a cadena con la ruta
****************************************************************************/
void ajusta_ndir(char *dir)
{
char *c;

/* va hasta el final de la cadena */
for(c=dir; *c; c++);

/* si no tiene '\' al final, se la a¤ade */
if(*(c-1)!='\\') {
	*c='\\';
	*(c+1)='\0';
}

/* pasa cadena a may£sculas */
strupr(dir);

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
****************************************************************************/
int lee_input(BYTE fil, BYTE col, char *cadena, int longitud, int ancho,
  BYTE attr)
{
char *cur, *fin, *ptr;
int num_car=0;
unsigned tecla;
BYTE ccur, i;

/* inicializa punteros */
cur=cadena;

/* busca final de la cadena y n£mero de caracteres */
for(fin=cadena; *fin; fin++, num_car++);

/* coordenada relativa del cursor respecto origen de caja */
ccur=0;

/* recoge caracteres hasta que pulse RETURN */
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

} while(tecla!=0x0d);           /* hasta que se pulse RETURN */

/* imprime l¡nea */
imp_linea_scr(fil,col,attr,cadena,ancho);

return(num_car);
}

/****************************************************************************
	MENU_INSTAL: men£ de opciones de instalaci¢n.
	  Salida:       opci¢n elegida
****************************************************************************/
int menu_instal(void)
{
int i;
static int opcion=0;
size_t lng_opc;
unsigned(tecla);

pon_cursor(25,0);

imp_linea_scr(0,0,COLOR_CAB,"           INSTALACION DEL SISTEMA "COPYRIGHT,
  80);

/* imprime men£ opciones */
for(i=0; i<N_OPC; i++) {
	lng_opc=strlen(opciones[i]);
	imp_linea_scr(i+2,(40-lng_opc)/2,COLOR_MENUC,opciones[i],lng_opc);
}

/* imprime directorios */
imp_linea_scr(2,42,COLOR_INPUT,cfg.dir_sintac,36);
imp_linea_scr(3,42,COLOR_INPUT,cfg.dir_gcs,36);
imp_linea_scr(4,42,COLOR_INPUT,cfg.dir_bd,36);
imp_linea_scr(5,42,COLOR_INPUT,dir_doc,36);

lng_opc=strlen(opciones[opcion]);
imp_linea_scr(opcion+2,(40-lng_opc)/2,COLOR_OPCC,opciones[opcion],lng_opc);

while(1) {
	tecla=_bios_keybrd(_KEYBRD_READ);

	switch(tecla >> 8) {
		case TCUR_ARR :
			if(opcion>0) {
				lng_opc=strlen(opciones[opcion]);
				imp_linea_scr(opcion+2,(40-lng_opc)/2,
				  COLOR_MENUC,opciones[opcion],lng_opc);
				opcion--;
				lng_opc=strlen(opciones[opcion]);
				imp_linea_scr(opcion+2,(40-lng_opc)/2,
				  COLOR_OPCC,opciones[opcion],lng_opc);
			}
			break;
		case TCUR_ABJ :
			if(opcion<(N_OPC-1)) {
				lng_opc=strlen(opciones[opcion]);
				imp_linea_scr(opcion+2,(40-lng_opc)/2,
				  COLOR_MENUC,opciones[opcion],lng_opc);
				opcion++;
				lng_opc=strlen(opciones[opcion]);
				imp_linea_scr(opcion+2,(40-lng_opc)/2,
				  COLOR_OPCC,opciones[opcion],lng_opc);
			}
			break;
		case TRETURN :
			pon_cursor(0,0);
			return(opcion);
	}
}

}

/****************************************************************************
	LEE_DATOS_FICH: lee los datos de los ficheros a instalar.
	  Entrada:      'nf_datfich' nombre del fichero con los datos
			de ficheros a instalar
	  Salida:       n£mero de ficheros a instalar
			'datf[i]' con los datos de los ficheros
			si hay alg£n error lo imprime y sale al sistema
			operativo
****************************************************************************/
int lee_datos_fich(char *nf_datfich)
{
FILE *ffich;
int i, c, tipo;
char buff[129];

/* abre fichero con datos de ficheros a instalar */
if((ffich=fopen(nf_datfich,"rt"))==NULL) {
	printf(MsgErr_NoFDat,nf_datfich);
	exit(1);
}

/* lee datos de ficheros */
i=0;
while(i<NUM_FICH) {
	/* coge nombre de fichero */
	if(fgets(datf[i].nombre,LNG_NOMBRE,ffich)==NULL) break;
	/* coma */
	c=fgetc(ffich);
	if((char)c!=',') {
		printf(MsgErr_FDat,nf_datfich);
		exit(1);
	}
	/* coge tipo 0..9 */
	c=fgetc(ffich);
	tipo=(int)(c-'0');
	if((tipo<0) || (tipo>9)) {
		printf(MsgErr_FDat,nf_datfich);
		exit(1);
	}
	datf[i].tipo=tipo;
	i++;
	/* recoge hasta final de l¡nea y deshecha */
	fgets(buff,128,ffich);
}

fclose(ffich);

return(i);
}

/****************************************************************************
	COPIA: copia un fichero en otro.
	  Entrada:      'origen' fichero de origen
			'destino' fichero destino
	  Salida:       0 si no hubo errores o 1 si lo hubo
****************************************************************************/
int copia(char *origen, char *destino)
{
char *buff;
int forg, fdest;
unsigned cuenta=0xff00;

/* abre fichero de origen */
if((forg=open(origen,O_BINARY | O_RDONLY))==-1) return(1);

/* crea fichero de destino, si existe lo borra */
if((fdest=open(destino,O_BINARY | O_WRONLY | O_CREAT,
  S_IREAD | S_IWRITE))==-1) return(1);

if(filelength(forg)<(long)cuenta) cuenta=(int)filelength(forg);

/* reserva un buffer de memoria, si no hay memoria suficiente */
/* reserva la m xima posible */
if((buff=(char *)malloc((size_t)cuenta))==NULL) {
    cuenta=_memmax();
    if((buff=(char *)malloc((size_t)cuenta))==NULL) return(1);
}

/* lee-escribe hasta final de fichero */
while(!eof(forg)) {
    if((cuenta=read(forg,buff,cuenta))==-1) return(1);
    if((cuenta=write(fdest,buff,cuenta))==-1) return(1);
}

/* cierra ficheros y libera memoria */
close(forg);
close(fdest);
free(buff);

return(0);
}

/****************************************************************************
	INSTALAR: rutina para instalar los ficheros en los subdirectorios
	  adecuados, adem s crea el fichero de configuraci¢n apropiado.
	  Entrada:      'n' n£mero de ficheros a instalar
		      Variables globales:-
			'datf' tabla con datos de ficheros a instalar
			'cfg' datos de configuraci¢n
	  Salida:       1 instalaci¢n correcta, 0 si hubo errores
****************************************************************************/
int instalar(int n)
{
char nombre_fich[_MAX_PATH], unidad[_MAX_DRIVE], dir[_MAX_DIR],
  nombref[_MAX_FNAME], ext[_MAX_EXT], num[20];
FILE *fich;
int l, i;

pon_cursor(25,0);

imp_linea_scr(LIN_INST,0,COLOR_MSGI,MSG_INSTL,80);

/* crea directorios de destino */
/* copia nombre de directorio y elimina '\' final */
strcpy(nombre_fich,cfg.dir_sintac);
l=strlen(nombre_fich);
if(nombre_fich[l-1]=='\\') nombre_fich[l-1]='\0';

/* crea directorio */
if(mkdir(nombre_fich)==-1) {
	if(errno!=EACCES) {
		l=strlen(MsgErr_Dir);
		imp_linea_scr(LIN_INST+2,0,COLOR_MSGI,MsgErr_Dir,l);
		imp_linea_scr(LIN_INST+2,l,COLOR_MSGI,nombre_fich,80-l);
		pon_cursor(0,0);
		return(0);
	}
}

/* copia nombre de directorio y elimina '\' final */
strcpy(nombre_fich,cfg.dir_gcs);
l=strlen(nombre_fich);
if(nombre_fich[l-1]=='\\') nombre_fich[l-1]='\0';

/* crea directorio */
if(mkdir(nombre_fich)==-1) {
	if(errno!=EACCES) {
		l=strlen(MsgErr_Dir);
		imp_linea_scr(LIN_INST+2,0,COLOR_MSGI,MsgErr_Dir,l);
		imp_linea_scr(LIN_INST+2,l,COLOR_MSGI,nombre_fich,80-l);
		pon_cursor(0,0);
		return(0);
	}
}

/* copia nombre de directorio y elimina '\' final */
strcpy(nombre_fich,cfg.dir_bd);
l=strlen(nombre_fich);
if(nombre_fich[l-1]=='\\') nombre_fich[l-1]='\0';

/* crea directorio */
if(mkdir(nombre_fich)==-1) {
	if(errno!=EACCES) {
		l=strlen(MsgErr_Dir);
		imp_linea_scr(LIN_INST+2,0,COLOR_MSGI,MsgErr_Dir,l);
		imp_linea_scr(LIN_INST+2,l,COLOR_MSGI,nombre_fich,80-l);
		pon_cursor(0,0);
		return(0);
	}
}

/* copia nombre de directorio y elimina '\' final */
strcpy(nombre_fich,dir_doc);
l=strlen(nombre_fich);
if(nombre_fich[l-1]=='\\') nombre_fich[l-1]='\0';

/* crea directorio */
if(mkdir(nombre_fich)==-1) {
	if(errno!=EACCES) {
		l=strlen(MsgErr_Dir);
		imp_linea_scr(LIN_INST+2,0,COLOR_MSGI,MsgErr_Dir,l);
		imp_linea_scr(LIN_INST+2,l,COLOR_MSGI,nombre_fich,80-l);
		pon_cursor(0,0);
		return(0);
	}
}

imp_linea_scr(LIN_INST+2,0,COLOR_MSGI,"Directorios creados...",80);
imp_linea_scr(LIN_INST+3,0,COLOR_MSGI,"Copiando ficheros...",80);

strcpy(nombre_fich,cfg.dir_sintac);
strcat(nombre_fich,Nf_Cfg);
l=strlen(MsgErr_Fich);
if((fich=fopen(nombre_fich,"wb"))==NULL) {
	imp_linea_scr(LIN_INST+4,0,COLOR_MSGI,MsgErr_Fich,l);
	imp_linea_scr(LIN_INST+4,l,COLOR_MSGI,nombre_fich,80-l);
	pon_cursor(0,0);
	return(0);
}
if(fwrite(&cfg,sizeof(STC_CFG),1,fich)!=1) {
	imp_linea_scr(LIN_INST+4,0,COLOR_MSGI,MsgErr_Fich,l);
	imp_linea_scr(LIN_INST+4,l,COLOR_MSGI,nombre_fich,80-l);
	fclose(fich);
	pon_cursor(0,0);
	return(0);
}
fclose(fich);

/* copia ficheros a sus directorios correspondientes, seg£n tipo */
for(i=0; i<n; i++) {
	itoa(i+1,num,10);
	imp_linea_scr(LIN_INST+3,21,COLOR_MSGI,num,59);

	/* asigna directorio de destino seg£n tipo de fichero */
	switch(datf[i].tipo) {
		case 0 :
			strcpy(nombre_fich,cfg.dir_sintac);
			break;
		case 1 :
			strcpy(nombre_fich,cfg.dir_bd);
			break;
		case 2 :
			strcpy(nombre_fich,cfg.dir_gcs);
			break;
		case 3 :
			strcpy(nombre_fich,dir_doc);
			break;
		default :
			strcpy(nombre_fich,cfg.dir_sintac);
	}

	/* crea nombre de fichero de destino */
	_splitpath(datf[i].nombre,unidad,dir,nombref,ext);
	strcat(nombre_fich,nombref);
	strcat(nombre_fich,ext);

	if(copia(datf[i].nombre,nombre_fich)==1) {
		l=strlen(MsgErr_Fich);
		imp_linea_scr(LIN_INST+4,0,COLOR_MSGI,MsgErr_Fich,l);
		imp_linea_scr(LIN_INST+4,l,COLOR_MSGI,datf[i].nombre,80-l);
		pon_cursor(0,0);
		return(0);
	}
}

pon_cursor(0,0);

return(1);
}

