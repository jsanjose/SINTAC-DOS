/****************************************************************************
			     INSTALACION SINTAC
			    (c)1993 JSJ Soft Ltd.
****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <fcntl.h>
#include <io.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <direct.h>
#include <errno.h>
#include <dos.h>
#include <bios.h>
#include <graph.h>
#include "..\ventanas\ventana.h"
#include "..\ventanas\raton.h"
#include "..\ventanas\menu.h"
#include "..\ventanas\cuadro.h"
#include "version.h"
#include "sintac.h"
#include "color.h"
#include "instalar.h"

/*** Variables globales ***/
STC_CFG cfg;                    /* almacena configuraci¢n */
char *Nf_Cfg=NF_CFG;            /* nombre de fichero de configuraci¢n */
char *OpcMenu=" ^Directorios                        : ^Colores| ^Instalar:"
	      " ^Salir";
char *OpcColor=" ^Men£ de opciones                  : ^Editor:"
	       " ^Cuadros di logo: ^Ventana ayuda: Ventana e^rrores:"
	       " C^ompilador| ^Salir";
STC_VENTANA vcab;
STC_VENTANA vconfig;

/* directorio para documentaci¢n */
char dir_doc[_MAX_PATH];

/* datos de ficheros a instalar */
STC_DATFICH datf[NUM_FICH_INST];
int num_fich_inst=0;    /* n£mero de ficheros a instalar */

/* mensajes de error */
char *MsgErr_NoFDat="Fichero INSTALAR.DAT no encontrado";
char *MsgErr_FDat="   Error en fichero INSTALAR.DAT";
char *MsgErr_Dir="   No se puede crear directorio";
char *MsgErr_Fich="      Error al copiar fichero";

/*** Programa principal ***/
void main(void)
{
FILE *fcfg;
STC_MENU *menu;
int num_fich_instalar, opcion;

if(!_setvideomode(_TEXTC80)) {
	printf("\nEste programa requiere tarjeta CGA o mejor.\n");
	return;
}
_settextcursor(CURSOR);

v_crea(&vcab,0,0,80,3,COLOR_MEN,COLOR_MENS1,COLOR_MENS2,NULL);
v_crea(&vconfig,3,0,80,22,COLOR_MEN,COLOR_MENS1,COLOR_MENS2,NULL);
menu=m_crea(MENU_VERT | MENU_FIJO,NULL,OpcMenu,MENU_FIL,MENU_COL,COLOR_MEN,
  COLOR_MENS2,COLOR_MENS1,COLOR_MENTEC,COLOR_MENSEL,0);

/* instala 'handler' de errores cr¡ticos */
_harderr(int24_hnd);

/* inicializa configuraci¢n */
inicializa_cfg();

v_abre(&vcab);
v_pon_cursor(&vcab,0,9);
v_impcad(&vcab,"Instalaci¢n del sistema "COPYRIGHT,V_NORELLENA);
v_abre(&vconfig);
m_abre(menu);

do {
	opcion=m_elige_opcion(menu);
	switch(opcion) {
		case 0 :
			directorios();
			break;
		case 1 :
			colores();
			break;
		case 3 :
			instalar();
			break;
	}
} while(opcion!=4);

m_elimina(menu);
v_cierra(&vconfig);
v_cierra(&vcab);

r_puntero(R_OCULTA);

_setvideomode(_DEFAULTMODE);

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
strcpy(cfg.dir_sintac,"C:\\SINTACG2\\");
strcpy(cfg.dir_bd,"C:\\SINTACG2\\DATOS\\");
strcpy(cfg.dir_util,"C:\\SINTACG2\\UTIL\\");
strcpy(dir_doc,"C:\\SINTACG2\\MANUAL\\");

/* inicializa colores */
cfg.color_men=COLOR_MEN;
cfg.color_mens1=COLOR_MENS1;
cfg.color_mens2=COLOR_MENS2;
cfg.color_mentec=COLOR_MENTEC;
cfg.color_mensel=COLOR_MENSEL;

cfg.color_ved=COLOR_VED;
cfg.color_veds1=COLOR_VEDS1;
cfg.color_veds2=COLOR_VEDS2;
cfg.color_vedblq=COLOR_VEDBLQ;

cfg.color_dlg=COLOR_DLG;
cfg.color_dlgs1=COLOR_DLGS1;
cfg.color_dlgs2=COLOR_DLGS2;
cfg.color_dlgtec=COLOR_DLGTEC;
cfg.color_dlgsel=COLOR_DLGSEL;

cfg.color_ayd=COLOR_AYD;
cfg.color_ayds1=COLOR_AYDS1;
cfg.color_ayds2=COLOR_AYDS2;
cfg.color_aydtec=COLOR_AYDTEC;
cfg.color_aydsel=COLOR_AYDSEL;

cfg.color_err=COLOR_ERR;
cfg.color_errs1=COLOR_ERRS1;
cfg.color_errs2=COLOR_ERRS2;
cfg.color_errtec=COLOR_ERRTEC;
cfg.color_errsel=COLOR_ERRSEL;

cfg.color_cs=COLOR_CS;
cfg.color_css1=COLOR_CSS1;
cfg.color_css2=COLOR_CSS2;
cfg.color_cstec=COLOR_CSTEC;
cfg.color_cssel=COLOR_CSSEL;

}

/****************************************************************************
	ESCONDE_CURSOR: oculta el cursor.
****************************************************************************/
void esconde_cursor(void)
{

_asm {
	mov ah,02h              ; funci¢n definir posici¢n del cursor
	mov bh,0                ; supone p gina 0
	mov dh,25               ; DH = fila del cursor
	mov dl,0                ; DL = columna del cursor
	int 10h
}

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
	DIRECTORIOS: fija los directorios de trabajo.
****************************************************************************/
void directorios(void)
{
STC_CUADRO cdir;
STC_ELEM_INPUT dir_sintac={
	"^SINTAC"
};
STC_ELEM_INPUT dir_bd={
	"^Bases de datos"
};
STC_ELEM_INPUT dir_util={
	"^Utilidades"
};
STC_ELEM_INPUT dir_manual={
	"^Documentaci¢n"
};
STC_ELEM_BOTON bvale={
	" ^Vale"
};
int i;

dir_sintac.cadena=cfg.dir_sintac;
dir_sintac.longitud=_MAX_PATH-1;
dir_bd.cadena=cfg.dir_bd;
dir_bd.longitud=_MAX_PATH-1;
dir_util.cadena=cfg.dir_util;
dir_util.longitud=_MAX_PATH-1;
dir_manual.cadena=dir_doc;
dir_manual.longitud=_MAX_PATH-1;

c_crea_cuadro(&cdir," Directorios del SINTAC ",C_CENT,C_CENT,CDIR_ANCHO,
  CDIR_ALTO,COLOR_DLG,COLOR_DLGS1,COLOR_DLGS2,COLOR_DLGTEC,COLOR_DLGSEL);
c_crea_elemento(&cdir,0,0,16,CDIR_ANCHO-18,3,COLOR_DLG,COLOR_DLGS2,COLOR_DLGS1,
  C_ELEM_INPUT,&dir_sintac);
c_crea_elemento(&cdir,1,3,16,CDIR_ANCHO-18,3,COLOR_DLG,COLOR_DLGS2,COLOR_DLGS1,
  C_ELEM_INPUT,&dir_bd);
c_crea_elemento(&cdir,2,6,16,CDIR_ANCHO-18,3,COLOR_DLG,COLOR_DLGS2,COLOR_DLGS1,
  C_ELEM_INPUT,&dir_util);
c_crea_elemento(&cdir,3,9,16,CDIR_ANCHO-18,3,COLOR_DLG,COLOR_DLGS2,COLOR_DLGS1,
  C_ELEM_INPUT,&dir_manual);
c_crea_elemento(&cdir,4,12,1,8,3,COLOR_DLG,COLOR_DLGS1,COLOR_DLGS2,
  C_ELEM_BOTON,&bvale);

c_abre(&cdir);
do {
	i=c_gestiona(&cdir);
} while((i!=-1) && (i!=4));
c_cierra(&cdir);

ajusta_ndir(cfg.dir_sintac);
ajusta_ndir(cfg.dir_bd);
ajusta_ndir(cfg.dir_util);
ajusta_ndir(dir_doc);

}

/****************************************************************************
	COLORES: selecciona colores del entorno y del compilador.
****************************************************************************/
void colores(void)
{
STC_MENU *mcolor;
int opcion;

mcolor=m_crea(MENU_VERT," Colores SINTAC ",OpcColor,MCOLOR_FIL,MCOLOR_COL,
  COLOR_MEN,COLOR_MENS2,COLOR_MENS1,COLOR_MENTEC,COLOR_MENSEL,0);

m_abre(mcolor);

do {
	opcion=m_elige_opcion(mcolor);
	switch(opcion) {
		case 0 :
			elige_colores(" Men£ opciones ",&cfg.color_men,
			  &cfg.color_mens1,&cfg.color_mens2,
			  &cfg.color_mentec,&cfg.color_mensel);
			break;
		case 1 :
			elige_colores(" Editor ",&cfg.color_ved,
			  &cfg.color_veds1,&cfg.color_veds2,
			  NULL,&cfg.color_vedblq);
			break;
		case 2 :
			elige_colores(" Cuadros di logo ",&cfg.color_dlg,
			  &cfg.color_dlgs1,&cfg.color_dlgs2,
			  &cfg.color_dlgtec,&cfg.color_dlgsel);
			break;
		case 3 :
			elige_colores(" Ventana de ayuda ",&cfg.color_ayd,
			  &cfg.color_ayds1,&cfg.color_ayds2,
			  &cfg.color_aydtec,&cfg.color_aydsel);
			break;
		case 4 :
			elige_colores(" Ventana de errores ",&cfg.color_err,
			  &cfg.color_errs1,&cfg.color_errs2,
			  &cfg.color_errtec,&cfg.color_errsel);
			break;
		case 5 :
			elige_colores(" Compilador ",&cfg.color_cs,
			  &cfg.color_css1,&cfg.color_css2,
			  &cfg.color_cstec,&cfg.color_cssel);
			break;
	}
} while((opcion!=-1) && (opcion!=7));

m_elimina(mcolor);

}

/****************************************************************************
	ELIGE_COLORES: selecci¢n de colores.
	  Entrada:      'titulo' t¡tulo de la ventana
			'attr_princ' puntero a atributo principal
			'attr_s1', 'attr_s2' punteros a atributos de
			sombreado
			'attr_tecla' puntero a atributo de tecla de
			activaci¢n; NULL si no se aplica
			'attr_sel' puntero a atributo de selecci¢n
****************************************************************************/
void elige_colores(char *titulo, BYTE *attr_princ, BYTE *attr_s1,
  BYTE *attr_s2, BYTE *attr_tecla, BYTE *attr_sel)
{
STC_VENTANA vcolor;
STC_CUADRO ccolor;
STC_ELEM_BOTON b1={
	" ^Principal"
};
STC_ELEM_BOTON b2={
	"  Borde ^1"
};
STC_ELEM_BOTON b3={
	"  Borde ^2"
};
STC_ELEM_BOTON b4={
	"   ^Teclas"
};
STC_ELEM_BOTON b5={
	"^Seleccionado"
};
STC_ELEM_BOTON bvale={
	"    ^Vale"
};
STC_ELEM_CHECK c1={
	"^Fondo"
};
int i;

v_crea(&vcolor,VCOLOR_FIL,VCOLOR_COL,VCOLOR_ANCHO,VCOLOR_ALTO,*attr_princ,
  *attr_s1,*attr_s2,titulo);
v_abre(&vcolor);

c_crea_cuadro(&ccolor,NULL,CCOLOR_FIL,CCOLOR_COL,CCOLOR_ANCHO,CCOLOR_ALTO,
  COLOR_DLG,COLOR_DLGS1,COLOR_DLGS2,COLOR_DLGTEC,COLOR_DLGSEL);
c_crea_elemento(&ccolor,0,0,0,(CCOLOR_ANCHO-2)/2,3,COLOR_DLG,COLOR_DLGS1,
  COLOR_DLGS2,C_ELEM_BOTON,&b1);
c_crea_elemento(&ccolor,1,0,(CCOLOR_ANCHO-2)/2,(CCOLOR_ANCHO-2)/2,3,COLOR_DLG,
  COLOR_DLGS1,COLOR_DLGS2,C_ELEM_BOTON,&b2);
c_crea_elemento(&ccolor,2,3,0,(CCOLOR_ANCHO-2)/2,3,COLOR_DLG,COLOR_DLGS1,
  COLOR_DLGS2,C_ELEM_BOTON,&b3);
c_crea_elemento(&ccolor,3,3,(CCOLOR_ANCHO-2)/2,(CCOLOR_ANCHO-2)/2,3,COLOR_DLG,
  COLOR_DLGS1,COLOR_DLGS2,C_ELEM_BOTON,&b4);
c_crea_elemento(&ccolor,4,6,0,(CCOLOR_ANCHO-2)/2,3,COLOR_DLG,COLOR_DLGS1,
  COLOR_DLGS2,C_ELEM_BOTON,&b5);
c_crea_elemento(&ccolor,5,7,((CCOLOR_ANCHO-2)/2)+1,(CCOLOR_ANCHO-2)/2,3,
  COLOR_DLG,COLOR_DLGS1,COLOR_DLGS2,C_ELEM_CHECK,&c1);
c_crea_elemento(&ccolor,6,9,0,(CCOLOR_ANCHO-2)/2,3,COLOR_DLG,COLOR_DLGS1,
  COLOR_DLGS2,C_ELEM_BOTON,&bvale);

c_abre(&ccolor);

do {
	vcolor.attr_princ=*attr_princ;
	vcolor.attr_s1=*attr_s1;
	vcolor.attr_s2=*attr_s2;
	v_dibuja(&vcolor,1);

	v_pon_cursor(&vcolor,1,1);
	v_color(&vcolor,*attr_princ);
	v_impcad(&vcolor," Texto normal ",V_NORELLENA);

	if(attr_tecla!=NULL) {
		v_pon_cursor(&vcolor,3,1);
		v_color(&vcolor,*attr_tecla);
		v_impcad(&vcolor," Teclas de activaci¢n ",V_NORELLENA);
	}

	v_pon_cursor(&vcolor,5,1);
	v_color(&vcolor,*attr_sel);
	v_impcad(&vcolor," Elemento seleccionado ",V_NORELLENA);

	i=c_gestiona(&ccolor);
	switch(i) {
		case 0 :
			if(c1.estado) *attr_princ=cambia_fondo(*attr_princ);
			else *attr_princ=cambia_tinta(*attr_princ);
			break;
		case 1 :
			if(c1.estado) *attr_s1=cambia_fondo(*attr_s1);
			else *attr_s1=cambia_tinta(*attr_s1);
			break;
		case 2 :
			if(c1.estado) *attr_s2=cambia_fondo(*attr_s2);
			else *attr_s2=cambia_tinta(*attr_s2);
			break;
		case 3 :
			if(attr_tecla!=NULL) {
				if(c1.estado) *attr_tecla=
				  cambia_fondo(*attr_tecla);
				else *attr_tecla=cambia_tinta(*attr_tecla);
			}
			break;
		case 4 :
			if(c1.estado) *attr_sel=cambia_fondo(*attr_sel);
			else *attr_sel=cambia_tinta(*attr_sel);
			break;
	}

} while((i!=-1) && (i!=6));

v_cierra(&vcolor);
c_cierra(&ccolor);

}

/****************************************************************************
	SELECCIONA_COLOR: presenta un cuadro de selecci¢n de color.
	  Entrada:      'c' color actual
	  Salida:       color seleccionado
****************************************************************************/
BYTE selecciona_color(BYTE c)
{
STC_VENTANA vselcol;
STC_RATON r;
int salida=0;
BYTE color;
short fil, col;
unsigned tecla;

v_crea(&vselcol,VSELCOL_FIL,VSELCOL_COL,VSELCOL_ANCHO,VSELCOL_ALTO,
  COLOR_DLG,COLOR_DLGS1,COLOR_DLGS2,NULL);
v_abre(&vselcol);

fil=0;
col=1;
for(color=0; color<16; color++) {
	if(color==8) {
		fil=3;
		col=1;
	}
	v_color(&vselcol,color);
	v_pon_cursor(&vselcol,fil,col);
	v_impc(&vselcol,VSELCOL_CHR1);
	v_impc(&vselcol,VSELCOL_CHR1);
	v_pon_cursor(&vselcol,fil+1,col);
	v_impc(&vselcol,VSELCOL_CHR1);
	v_impc(&vselcol,VSELCOL_CHR1);
	col+=3;
}

color=c;
v_color(&vselcol,COLOR_DLG);
do {
	if(color<8) fil=2;
	else fil=5;
	col=((color & 0x07)*3)+1;
	v_pon_cursor(&vselcol,fil,col);
	v_impc(&vselcol,VSELCOL_CHR2);

	r_estado(&r);
	if(r.boton2) salida=1;
	else if(r.boton1) {
		if((r.fil>vselcol.fil) &&
		  (r.fil<(vselcol.fil+vselcol.alto-1)) &&
		  (r.col>vselcol.col) &&
		  (r.col<(vselcol.col+vselcol.ancho-1))) {
			v_pon_cursor(&vselcol,fil,col);
			v_impc(&vselcol,' ');
			fil=r.fil-vselcol.fil-1;
			col=r.col-vselcol.col-1;
			color=(BYTE)((col/3)+((fil/3)*8));
		}
	}
	else if(_bios_keybrd(_KEYBRD_READY)) {
		tecla=_bios_keybrd(_KEYBRD_READ);
		v_pon_cursor(&vselcol,fil,col);
		v_impc(&vselcol,' ');
		switch(tecla >> 8) {
			case 0x4b :     /* cursor izquierda */
				if(color>0) color--;
				break;
			case 0x4d :     /* cursor derecha */
				if(color<15) color++;
				break;
			case 0x48 :     /* cursor arriba */
				if(color>7) color-=8;
				break;
			case 0x50 :     /* cursor abajo */
				if(color<8) color+=8;
				break;
			case 0x1c :     /* RETURN */
				salida=1;
				break;
		}
	}
} while(!salida);

v_cierra(&vselcol);

return(color);
}

/****************************************************************************
	CAMBIA_TINTA: cambia el color de tinta de un atributo.
	  Entrada:      'attr' atributo
	  Salida:       atributo modificado
****************************************************************************/
BYTE cambia_tinta(BYTE attr)
{
BYTE tinta, attr2;

tinta=selecciona_color((BYTE)(attr & 0x0f));
attr2=(BYTE)((attr & 0xf0) | tinta);

return(attr2);
}

/****************************************************************************
	CAMBIA_FONDO: cambia el color de fondo de un atributo.
	  Entrada:      'attr' atributo
	  Salida:       atributo modificado
****************************************************************************/
BYTE cambia_fondo(BYTE attr)
{
BYTE fondo, attr2;

fondo=selecciona_color((BYTE)(attr >> 4));
attr2=(BYTE)((attr & 0x0f) | (fondo << 4));

return(attr2);
}

/****************************************************************************
	IMPRIME_ERROR: imprime un mensaje de error y sale al sistema
	  operativo.
	  Entrada:      'msg' mensaje
****************************************************************************/
void imprime_error(char *msg)
{
STC_CUADRO c;
STC_ELEM_TEXTO t={
	"", C_TXTLINEA, C_TXTNOBORDE
};
STC_ELEM_BOTON b={
	" ^Vale"
};
int i;

c_crea_cuadro(&c," ERROR ",C_CENT,C_CENT,ERR_ANCHO,ERR_ALTO,cfg.color_err,
  cfg.color_errs1,cfg.color_errs2,cfg.color_errtec,cfg.color_errsel);
c_crea_elemento(&c,0,ERR_ALTO-5,(ERR_ANCHO-10)/2,8,3,cfg.color_err,
  cfg.color_errs1,cfg.color_errs2,C_ELEM_BOTON,&b);
t.texto=msg;
c_crea_elemento(&c,1,0,0,ERR_ANCHO-2,5,cfg.color_err,cfg.color_errs1,
  cfg.color_errs2,C_ELEM_TEXTO,&t);

c_abre(&c);
do {
	i=c_gestiona(&c);
} while((i!=-1) && (i!=0));
c_cierra(&c);

fcloseall();
_setvideomode(_DEFAULTMODE);

exit(1);

}

/****************************************************************************
	LEE_DATOS_FICH: lee los datos de los ficheros a instalar.
	  Entrada:      'nf_datfich' nombre del fichero con los datos
			de ficheros a instalar
	  Salida:     Variables globales:-
			'num_fich_inst' n£mero de ficheros a instalar
			'datf[i]' con los datos de los ficheros
			si hay alg£n error lo imprime y sale al sistema
			operativo
****************************************************************************/
void lee_datos_fich(char *nf_datfich)
{
FILE *ffich;
int i=0, c, tipo, disco;
char buff[129];

/* abre fichero con datos de ficheros a instalar */
if((ffich=fopen(nf_datfich,"rt"))==NULL) imprime_error(MsgErr_NoFDat);

/* lee datos de ficheros */
while(i<NUM_FICH_INST) {
	/* coge nombre de fichero */
	if(fgets(datf[i].nombre,LNG_NOMBRE,ffich)==NULL) break;
	/* coma */
	c=fgetc(ffich);
	if((char)c!=',') imprime_error(MsgErr_FDat);

	/* coge tipo (0..9) */
	c=fgetc(ffich);
	tipo=(int)(c-'0');
	if((tipo<0) || (tipo>9)) imprime_error(MsgErr_FDat);
	datf[i].tipo=tipo;
	/* coma */
	c=fgetc(ffich);
	if((char)c!=',') imprime_error(MsgErr_FDat);

	/* coge disco de instalaci¢n (0..9) */
	c=fgetc(ffich);
	disco=(int)(c-'0');
	if((disco<0) || (disco>9)) imprime_error(MsgErr_FDat);
	datf[i].disco=disco;

	i++;

	/* recoge hasta final de l¡nea y deshecha */
	fgets(buff,128,ffich);
}

fclose(ffich);

num_fich_inst=i;

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
	INSTALAR: rutina principal de instalaci¢n.
****************************************************************************/
void instalar(void)
{
STC_VENTANA v;
char nombre_fich[_MAX_PATH], unidad[_MAX_DRIVE], dir[_MAX_DIR],
  nombref[_MAX_FNAME], ext[_MAX_EXT], msg[41];
FILE *fich;
int i, disco_act=0;

esconde_cursor();

v_crea(&v,13,20,40,7,COLOR_DLG,COLOR_DLGS2,COLOR_DLGS1,NULL);
v_abre(&v);

/* coge datos de ficheros a instalar */
lee_datos_fich("INSTALAR.DAT");

/* crea directorios de destino */
v_pon_cursor(&v,2,4);
v_impcad(&v,"Creando directorios de destino",V_NORELLENA);

/* SINTAC */
strcpy(nombre_fich,cfg.dir_sintac);
i=strlen(nombre_fich);
if(nombre_fich[i-1]=='\\') nombre_fich[i-1]='\0';
if((mkdir(nombre_fich)==-1) && (errno!=EACCES)) imprime_error(MsgErr_Dir);

/* BASES DE DATOS */
strcpy(nombre_fich,cfg.dir_bd);
i=strlen(nombre_fich);
if(nombre_fich[i-1]=='\\') nombre_fich[i-1]='\0';
if((mkdir(nombre_fich)==-1) && (errno!=EACCES)) imprime_error(MsgErr_Dir);

/* UTILIDADES */
strcpy(nombre_fich,cfg.dir_util);
i=strlen(nombre_fich);
if(nombre_fich[i-1]=='\\') nombre_fich[i-1]='\0';
if((mkdir(nombre_fich)==-1) && (errno!=EACCES)) imprime_error(MsgErr_Dir);

/* DOCUMENTACION */
strcpy(nombre_fich,dir_doc);
i=strlen(nombre_fich);
if(nombre_fich[i-1]=='\\') nombre_fich[i-1]='\0';
if((mkdir(nombre_fich)==-1) && (errno!=EACCES)) imprime_error(MsgErr_Dir);

/* crea fichero de configuraci¢n */
v_borra(&v);
v_pon_cursor(&v,2,3);
v_impcad(&v,"Creando fichero de configuraci¢n",V_NORELLENA);
strcpy(nombre_fich,cfg.dir_sintac);
strcat(nombre_fich,Nf_Cfg);
if((fich=fopen(nombre_fich,"wb"))==NULL) imprime_error(MsgErr_Fich);
if(fwrite(&cfg,sizeof(STC_CFG),1,fich)!=1) imprime_error(MsgErr_Fich);
fclose(fich);

/* copia ficheros a sus directorios correspondientes, seg£n tipo */
v_borra(&v);
v_pon_cursor(&v,2,10);
v_impcad(&v,"Copiando ficheros",V_NORELLENA);

for(i=0; i<num_fich_inst; i++) {
	v_borra(&v);
	v_pon_cursor(&v,1,11);
	v_impcad(&v,"Copiando ficheros",V_NORELLENA);
	sprintf(msg,"Fichero %02i de %02i",i+1,num_fich_inst);
	v_pon_cursor(&v,3,11);
	v_impcad(&v,msg,V_NORELLENA);

	/* asigna directorio de destino seg£n tipo de fichero */
	switch(datf[i].tipo) {
		case 0 :
			strcpy(nombre_fich,cfg.dir_sintac);
			break;
		case 1 :
			strcpy(nombre_fich,cfg.dir_bd);
			break;
		case 2 :
			strcpy(nombre_fich,cfg.dir_util);
			break;
		case 3 :
			strcpy(nombre_fich,dir_doc);
			break;
		default :
			strcpy(nombre_fich,cfg.dir_sintac);
	}

	/* comprueba si hay que cambiar de disco */
	if(datf[i].disco!=disco_act) {
		disco_act=datf[i].disco;
		sprintf(msg,"Inserta disco %i y pulsa una tecla",disco_act+1);
		v_pon_cursor(&v,4,2);
		v_impcad(&v,msg,V_NORELLENA);
		beep();
		_bios_keybrd(_KEYBRD_READ);
		v_pon_cursor(&v,4,0);
		v_impcad(&v,"",V_RELLENA);
	}

	/* crea nombre de fichero de destino */
	_splitpath(datf[i].nombre,unidad,dir,nombref,ext);
	strcat(nombre_fich,nombref);
	strcat(nombre_fich,ext);

	if(copia(datf[i].nombre,nombre_fich)==1) imprime_error(MsgErr_Fich);
}

v_cierra(&v);

aviso();

}

/****************************************************************************
	AVISO: imprime un mensaje de aviso cuando a acabado de instalar.
****************************************************************************/
void aviso(void)
{
STC_CUADRO c;
STC_ELEM_TEXTO t={
	"Comprueba que el fichero CONFIG.SYS\n"
	"contenga: FILES=20", C_TXTLINEA, C_TXTBORDE
};
STC_ELEM_BOTON b={
	" ^Vale"
};
int i;

c_crea_cuadro(&c,NULL,C_CENT,C_CENT,AV_ANCHO,AV_ALTO,cfg.color_err,
  cfg.color_errs1,cfg.color_errs2,cfg.color_errtec,cfg.color_errsel);
c_crea_elemento(&c,0,AV_ALTO-5,(AV_ANCHO-10)/2,8,3,cfg.color_err,
  cfg.color_errs1,cfg.color_errs2,C_ELEM_BOTON,&b);
c_crea_elemento(&c,1,0,0,AV_ANCHO-2,4,cfg.color_err,cfg.color_errs2,
  cfg.color_errs1,C_ELEM_TEXTO,&t);

c_abre(&c);
do {
	i=c_gestiona(&c);
} while((i!=-1) && (i!=0));
c_cierra(&c);

}

