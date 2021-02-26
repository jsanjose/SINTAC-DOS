/****************************************************************************
			    CONFIGURACION SINTAC
			    (c)1994 JSJ Soft Ltd.
****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <dir.h>
#include <dos.h>
#include <bios.h>
#include "..\ventanas\ventana.h"
#include "..\ventanas\raton.h"
#include "..\ventanas\menu.h"
#include "..\ventanas\cuadro.h"
#include "version.h"
#include "sintac.h"
#include "color.h"
#include "config.h"

/*** Variables globales ***/
/* tama¤o del STACK */
unsigned _stklen=8192;

STC_CFG cfg;                    /* almacena configuraci¢n */
char *Nf_Cfg=NF_CFG;            /* nombre de fichero de configuraci¢n */
char *OpcMenu=" ^Directorios: ^Colores: ^Restablecer configuraci¢n         :"
	      " ^Guardar configuraci¢n| ^Salir";
char *OpcColor=" ^Men£ de opciones                  : ^Editor:"
	       " ^Cuadros di logo: ^Ventana ayuda: Ventana e^rrores:"
	       " C^ompilador| ^Salir";
STC_VENTANA vcab;
STC_VENTANA vconfig;

/*** Programa principal ***/
void main(void)
{
FILE *fcfg;
STC_MENU *menu;
int modovideo, opcion;

asm {
	mov ax,0003h	// definir modo v¡deo (texto 80x25 color)
	int 10h
	mov ah,0fh	// recoge modo de v¡deo
	int 10h
	mov ah,0
	mov modovideo,ax
}
/* si no pudo establecer modo de v¡deo */
if(modovideo!=0x03) {
	printf("\nEste programa requiere tarjeta CGA o mejor.\n");
	exit(1);
}

v_crea(&vcab,0,0,80,3,COLOR_MEN,COLOR_MENS1,COLOR_MENS2,NULL);
v_crea(&vconfig,3,0,80,22,COLOR_MEN,COLOR_MENS1,COLOR_MENS2,NULL);
menu=m_crea(MENU_VERT | MENU_FIJO," Configuraci¢n SINTAC ",OpcMenu,MENU_FIL,
  MENU_COL,COLOR_MEN,COLOR_MENS2,COLOR_MENS1,COLOR_MENTEC,COLOR_MENSEL,0);

/* instala 'handler' de errores cr¡ticos */
harderr(int24_hnd);

/* inicializa configuraci¢n */
inicializa_cfg();

/* lee fichero de configuraci¢n, si hay uno creado */
if(!access(Nf_Cfg,0)) {
	if((fcfg=fopen(Nf_Cfg,"rb"))!=NULL) {
		fread(&cfg,sizeof(STC_CFG),1,fcfg);
		fclose(fcfg);
	}
}

v_abre(&vcab);
v_pon_cursor(&vcab,0,9);
v_impcad(&vcab,"Configuraci¢n del sistema "COPYRIGHT,V_NORELLENA);
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
		case 2 :
			inicializa_cfg();
			break;
		case 3 :
			esconde_cursor();
			if((fcfg=fopen(Nf_Cfg,"wb"))==NULL) break;
			fwrite(&cfg,sizeof(STC_CFG),1,fcfg);
			fclose(fcfg);
			break;
	}
} while(opcion!=5);

m_elimina(menu);
v_cierra(&vconfig);
v_cierra(&vcab);

r_puntero(R_OCULTA);

asm {
	mov ax,0003h	// definir modo v¡deo (texto 80x25 color)
	int 10h
}

}

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
	INICIALIZA_CFG: inicializa configuraci¢n asignando valores por
	  defecto.
****************************************************************************/
void inicializa_cfg(void)
{
char dir[MAXPATH];

/* coge directorio actual (damos longitud _MAX_PATH-1 por si hay que */
/* a¤adir '\' al final) */
getcwd(dir,MAXPATH);

/* a¤ade '\' al final si no lo tiene */
ajusta_ndir(dir);

/* inicializa directorios */
strcpy(cfg.dir_sintac,dir);
strcpy(cfg.dir_bd,dir);
strcpy(cfg.dir_util,dir);

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

asm {
	mov ah,02h              // funci¢n definir posici¢n del cursor
	mov bh,0                // supone p gina 0
	mov dh,25               // DH = fila del cursor
	mov dl,0                // DL = columna del cursor
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
STC_ELEM_BOTON bvale={
	" ^Vale"
};
int i;

dir_sintac.cadena=cfg.dir_sintac;
dir_sintac.longitud=MAXPATH-1;
dir_bd.cadena=cfg.dir_bd;
dir_bd.longitud=MAXPATH-1;
dir_util.cadena=cfg.dir_util;
dir_util.longitud=MAXPATH-1;

c_crea_cuadro(&cdir," Directorios del SINTAC ",C_CENT,C_CENT,CDIR_ANCHO,
  CDIR_ALTO,COLOR_DLG,COLOR_DLGS1,COLOR_DLGS2,COLOR_DLGTEC,COLOR_DLGSEL);
c_crea_elemento(&cdir,0,0,16,CDIR_ANCHO-18,3,COLOR_DLG,COLOR_DLGS2,COLOR_DLGS1,
  C_ELEM_INPUT,&dir_sintac);
c_crea_elemento(&cdir,1,3,16,CDIR_ANCHO-18,3,COLOR_DLG,COLOR_DLGS2,COLOR_DLGS1,
  C_ELEM_INPUT,&dir_bd);
c_crea_elemento(&cdir,2,6,16,CDIR_ANCHO-18,3,COLOR_DLG,COLOR_DLGS2,COLOR_DLGS1,
  C_ELEM_INPUT,&dir_util);
c_crea_elemento(&cdir,3,9,1,8,3,COLOR_DLG,COLOR_DLGS1,COLOR_DLGS2,
  C_ELEM_BOTON,&bvale);

c_abre(&cdir);
do {
	i=c_gestiona(&cdir);
} while((i!=-1) && (i!=3));
c_cierra(&cdir);

ajusta_ndir(cfg.dir_sintac);
ajusta_ndir(cfg.dir_bd);
ajusta_ndir(cfg.dir_util);

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
	else if(bioskey(1)) {
		tecla=bioskey(0);
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

