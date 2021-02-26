/****************************************************************************
			    CONFIGURACION SINTAC
			    (c)1993 JSJ Soft Ltd.
****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <direct.h>
#include <bios.h>
#include <dos.h>
#include <io.h>
#include "version.h"
#include "sintac.h"
#include "color.h"
#include "config.h"

/*** Variables globales ***/
STC_CFG cfg;                    /* almacena configuraci¢n */

/* men£ de opciones */
char opciones[N_OPC][LNG_OPC]= {
	"Directorio del sistema SINTAC",
	"Directorio del GENERADOR DE CARACTERES",
	"Directorio de BASES DE DATOS",
	"Color general de pantalla",
	"Color de zona de edici¢n",
	"Color de regleta",
	"Color de cabecera",
	"Color de l¡nea de estado",
	"Color de men£ opciones",
	"Color de tecla de activaci¢n",
	"Color opci¢n resaltada",
	"Color de ventana del compilador",
	"Color de ventana de errores compilador",
	"Color de ventana de errores entorno",
	"Color de error resaltado",
	"Color de ventana de ayuda",
	"Color de cabecera de apartado",
	"Color de referencia de ayuda",
	"RESTABLECER CONFIGURACION ORIGINAL",
	"GUARDAR CONFIGURACION",
	"SALIR",
};

char *Nf_Cfg=NF_CFG;            /* nombre de fichero de configuraci¢n */

/*** Programa principal ***/
void main(void)
{
FILE *fcfg;
int opcion;

/* instala 'handler' de errores cr¡ticos */
_harderr(int24_hnd);

borra_scr(0x07);

/* inicializa configuraci¢n */
inicializa_cfg();

/* lee fichero de configuraci¢n, si hay uno creado */
if(!access(Nf_Cfg,0)) {
	if((fcfg=fopen(Nf_Cfg,"rb"))!=NULL) {
		fread(&cfg,sizeof(STC_CFG),1,fcfg);
		fclose(fcfg);
	}
}

do {
	opcion=menu_cfg();
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
		case 3 :                /* color general de pantalla */
			cfg.color_scr=elige_color(5,VCOLOR_COL,
			  cfg.color_scr);
			break;
		case 4 :                /* color zona edici¢n */
			cfg.color_editor=elige_color(6,VCOLOR_COL,
			  cfg.color_editor);
			break;
		case 5 :                /* color de regleta */
			cfg.color_regla=elige_color(7,VCOLOR_COL,
			  cfg.color_regla);
			break;
		case 6 :                /* color cabecera */
			cfg.color_cab=elige_color(8,VCOLOR_COL,cfg.color_cab);
			break;
		case 7 :                /* color l¡nea de estado */
			cfg.color_linst=elige_color(9,VCOLOR_COL,
			  cfg.color_linst);
			break;
		case 8 :                /* color men£ opciones */
			cfg.color_menu=elige_color(10,VCOLOR_COL,
			  cfg.color_menu);
			break;
		case 9 :                /* color tecla de activaci¢n */
			cfg.color_tecla=elige_color(11,VCOLOR_COL,
			  cfg.color_tecla);
			break;
		case 10 :               /* color opci¢n resaltada */
			cfg.color_opcion=elige_color(12,VCOLOR_COL,
			  cfg.color_opcion);
			break;
		case 11 :               /* color ventana compilador */
			cfg.color_wcomp=elige_color(13,VCOLOR_COL,
			  cfg.color_wcomp);
			break;
		case 12 :               /* color ventana de errores */
			cfg.color_werr=elige_color(14,VCOLOR_COL,
			  cfg.color_werr);
			break;
		case 13 :               /* color ventana errores entorno */
			cfg.color_err=elige_color(15,VCOLOR_COL,
			  cfg.color_err);
			break;
		case 14 :               /* color de error resaltado */
			cfg.color_errsel=elige_color(16,VCOLOR_COL,
			  cfg.color_errsel);
			break;
		case 15 :               /* color de ventana de ayuda */
			cfg.color_ay=elige_color(17,VCOLOR_COL,cfg.color_ay);
			break;
		case 16 :               /* color de cabecera de apartado */
			cfg.color_aptdo=elige_color(18,VCOLOR_COL,
			  cfg.color_aptdo);
			break;
		case 17 :               /* color de referencia de ayuda */
			cfg.color_ref=elige_color(19,VCOLOR_COL,
			  cfg.color_ref);
			break;
		case (N_OPC-3) :        /* restablecer configuraci¢n */
			inicializa_cfg();
			break;
		case (N_OPC-2) :        /* guardar configuraci¢n */
			if((fcfg=fopen(Nf_Cfg,"wb"))==NULL) break;
			fwrite(&cfg,sizeof(STC_CFG),1,fcfg);
			fclose(fcfg);
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
char dir[_MAX_PATH];

/* coge directorio actual (damos longitud _MAX_PATH-1 por si hay que */
/* a¤adir '\' al final */
getcwd(dir,_MAX_PATH-1);

/* a¤ade '\' al final si no lo tiene */
ajusta_ndir(dir);

/* inicializa directorios */
strcpy(cfg.dir_sintac,dir);
strcpy(cfg.dir_gcs,dir);
strcpy(cfg.dir_bd,dir);

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
	MENU_CFG: men£ de opciones de configuraci¢n.
	  Salida:       opci¢n elegida
****************************************************************************/
int menu_cfg(void)
{
int i;
static int opcion=0;
size_t lng_opc;
unsigned(tecla);

pon_cursor(25,0);

imp_linea_scr(0,0,COLOR_CAB,"          CONFIGURACION DEL SISTEMA "COPYRIGHT,
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

/* imprime muestras de color */
imp_linea_scr(5,VCOLOR_COL,cfg.color_scr,TXT_VCOLOR,16);
imp_linea_scr(6,VCOLOR_COL,cfg.color_editor,TXT_VCOLOR,16);
imp_linea_scr(7,VCOLOR_COL,cfg.color_regla,TXT_VCOLOR,16);
imp_linea_scr(8,VCOLOR_COL,cfg.color_cab,TXT_VCOLOR,16);
imp_linea_scr(9,VCOLOR_COL,cfg.color_linst,TXT_VCOLOR,16);
imp_linea_scr(10,VCOLOR_COL,cfg.color_menu,TXT_VCOLOR,16);
imp_linea_scr(11,VCOLOR_COL,cfg.color_tecla,TXT_VCOLOR,16);
imp_linea_scr(12,VCOLOR_COL,cfg.color_opcion,TXT_VCOLOR,16);
imp_linea_scr(13,VCOLOR_COL,cfg.color_wcomp,TXT_VCOLOR,16);
imp_linea_scr(14,VCOLOR_COL,cfg.color_werr,TXT_VCOLOR,16);
imp_linea_scr(15,VCOLOR_COL,cfg.color_err,TXT_VCOLOR,16);
imp_linea_scr(16,VCOLOR_COL,cfg.color_errsel,TXT_VCOLOR,16);
imp_linea_scr(17,VCOLOR_COL,cfg.color_ay,TXT_VCOLOR,16);
imp_linea_scr(18,VCOLOR_COL,cfg.color_aptdo,TXT_VCOLOR,16);
imp_linea_scr(19,VCOLOR_COL,cfg.color_ref,TXT_VCOLOR,16);

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

#pragma check_pointer(off)
/****************************************************************************
	ELIGE_COLOR: presenta una ventana de selecci¢n de colores.
	  Entrada:      'fila' fila de ventana de selecci¢n
			'columna' columna de ventana de selecci¢n
			'color_act' color actual
	  Salida:       color elegido
****************************************************************************/
BYTE elige_color(BYTE fila, BYTE columna, BYTE color_act)
{
BYTE _far *pscr, _far *p;
int tinta, fondo;
BYTE f, t, color;
unsigned tecla;

pon_cursor(25,0);

pscr=(BYTE _far *)0xb8000000+(((VCOLOR_FIL*80)+columna+20)*2);
for(fondo=0; fondo<8; fondo++) {
	for(tinta=0, p=pscr; tinta<16; tinta++) {
		color=(BYTE)((fondo << 4) | tinta);
		*p++=MARCA1;
		*p++=color;
	}
	pscr+=160;
}

/* calcula fondo y tinta de color actual */
f=(BYTE)((color_act & 0xf0) >> 4);
t=(BYTE)(color_act & 0x0f);

pscr=(BYTE _far *)0xb8000000+((((VCOLOR_FIL+f)*80)+(columna+t+20))*2);
*pscr=MARCA2;
*(pscr+1)=(BYTE)((f<< 4) | COLOR_MARCA);

while(1) {
	/* imprime texto de prueba */
	imp_linea_scr(fila,columna,(BYTE)((f << 4) | t),TXT_VCOLOR,16);

	tecla=_bios_keybrd(_KEYBRD_READ);

	/* borra marcador */
	*pscr=MARCA1;
	*(pscr+1)=(BYTE)((f << 4) | t);

	switch(tecla >> 8) {
		case TCUR_ARR :
			if(f>0) f--;
			break;
		case TCUR_ABJ :
			if(f<7) f++;
			break;
		case TCUR_IZQ :
			if(t>0) t--;
			break;
		case TCUR_DER :
			if(t<15) t++;
			break;
		case TRETURN :
			/* borra ventana de selecci¢n */
			pscr=(BYTE _far *)0xb8000000+
			  (((VCOLOR_FIL*80)+columna+20)*2);
			for(fondo=0; fondo<8; fondo++) {
				for(tinta=0, p=pscr; tinta<16; tinta++) {
					*p++=' ';
					*p++=0x07;
				}
				pscr+=160;
			}

			pon_cursor(0,0);
			color=(BYTE)((f << 4) | t);
			return(color);
	}

	/* imprime marcador */
	pscr=(BYTE _far *)0xb8000000+((((VCOLOR_FIL+f)*80)+(columna+t+20))*2);
	*pscr=MARCA2;
	*(pscr+1)=(BYTE)((f<< 4) | COLOR_MARCA);
}

}
#pragma check_pointer()
