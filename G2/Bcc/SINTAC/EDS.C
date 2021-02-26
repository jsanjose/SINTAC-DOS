/****************************************************************************
			 ENTORNO DE DESARROLLO SINTAC
			     (c)1994 JSJ Soft Ltd.
****************************************************************************/

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <alloc.h>
#include <process.h>
#include <dir.h>
#include <io.h>
#include <fcntl.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <ctype.h>
#include <bios.h>
#include <dos.h>
#include "..\ventanas\ventana.h"
#include "..\ventanas\raton.h"
#include "..\ventanas\menu.h"
#include "..\ventanas\cuadro.h"
#include "..\ventanas\editor.h"
#include "..\ventanas\ayuda.h"
#include "version.h"
#include "sintac.h"
#include "color.h"
#include "eds.h"

/*** Variables globales ***/
/* tama¤o del STACK */
unsigned _stklen=10240;

STC_CFG cfg;
STC_MENU *m;

/* marcas de secci¢n */
char *Mconst="ORG";
char *Mfin_Secc="\\END";
char *Mmsy="\\MSY";
char *Mvoc="\\VOC";
char *Mmsg="\\MSG";
char *Mloc="\\LOC";
char *Mobj="\\OBJ";
char *Mpro="\\PRO";

/* nombres de ficheros */
char *Nf_Bdtemp="$BD";
char *Nf_Bdaux="$BDAUX";
char *Nf_Secc="$%s";
char *Nf_Seccmult="$%s.%i";
char *Nf_Err=NF_ERR;

/* cadena con n£meros */
char *CNum="0123456789";

/* argumentos de llamada al compilador */
char *Arg_Cs="-E";

/* argumentos de llamada al int‚rprete-debugger */
char *Arg_Ds1="-L00";
char *Arg_Ds2="-E";

/* argumentos de llamada al linkador */
char *Arg_Lks="-E";

/* nombre de fichero de base de datos en uso */
char base_datos[MAXPATH];

/* indicador de base de datos modificada */
BOOLEAN bd_modificada=FALSE;

/* tablas para comprobar existencia de ficheros temporales de */
/* secciones m£ltiples */
BOOLEAN pro_existe[MAX_PRO];
BOOLEAN msg_existe[MAX_TMSG];

/* mensajes */
char *Merr_Lect=" Error de lectura de fichero";
char *Merr_Escr="Error de escritura en fichero";
char *Merr_Aper="Error de apertura de fichero";
char *Merr_Nosecc="    Secci¢n no encontrada";
char *Merr_Noend="Falta marca de fin de secci¢n";
char *Merr_Proex="      Proceso ya existe";
char *Merr_Pronoex="      Proceso no existe";
char *Merr_Pro0borr="No se puede borrar Proceso 0";
char *Merr_Msgex=" Tabla de mensajes ya existe";
char *Merr_Msgnoex=" Tabla de mensajes no existe";
char *Merr_Msg0borr=" No se puede borrar tabla 0";
char *Merr_Npro=" N£mero de proceso no v lido";
char *Merr_Ntmsg="  N£mero de tabla no v lida";
char *Merr_Nocomp="   No se pudo ejecutar CS";
char *Merr_Nolink="   No se pudo ejecutar LKS";
char *Merr_Nods="   No se pudo ejecutar DS";
char *Merr_Lin="     L¡nea no encontrada";
char *Merr_Noutil=" No se pudo ejecutar utilidad";

/*** Programa principal ***/
#pragma warn -par
void main(int argc, char *argv[])
{
STC_MENU *m0, *m1, *m2, *m3, *mayd;
char *menu=" ÿ^ðÿ:ÿ^Ficheroÿ:ÿ^Seccionesÿ:ÿ^Compilarÿ:ÿ^Utilidadesÿ:    "
	   "          ÿ^Ayudaÿ ";
char *opcionesx=" ^Acerca de...| ^Comando del DOS ";
char *opcionesf=" ^Nuevo: ^Abrir: ^Grabar: Grabar co^mo... |"
		" Grabar ^bloque: ^Liberar bloque| ^Salir";
char *opcioness=" ^Constantes: Mensajes ^sistema : ^Vocabulario:"
		" ^Mensajes: ^Localidades: ^Objetos: ^Procesos";
char *opcionesc=" ^Compilar base de datos : ^Ejecutar base de datos: E^rrores|"
		" Crear e^jecutable";
char *opcionesa=" ^Indice: ^Ayuda sobre...| ^Teclas del editor:"
		" Teclas ^cuadros di logo : ^Variables y banderas:"
		" ^Lista de condactos";
int modovideo, i, o, o1, salida=0;
char nbd[MAXPATH], csecc[81];

/* instala 'handler' de errores cr¡ticos */
harderr(int24_hnd);

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

lee_cfg(argv[0]);

m=m_crea(MENU_HORZ | MENU_FIJO," Entorno de desarrollo S.I.N.T.A.C. ",menu,0,
  0,cfg.color_men,cfg.color_mens1,cfg.color_mens2,cfg.color_mentec,
  cfg.color_mensel,2);
m0=m_crea(MENU_VERT,NULL,opcionesx,2,2,cfg.color_men,cfg.color_mens1,
  cfg.color_mens2,cfg.color_mentec,cfg.color_mensel,0);
m1=m_crea(MENU_VERT,NULL,opcionesf,2,7,cfg.color_men,cfg.color_mens1,
  cfg.color_mens2,cfg.color_mentec,cfg.color_mensel,0);
m2=m_crea(MENU_VERT,NULL,opcioness,2,18,cfg.color_men,cfg.color_mens1,
  cfg.color_mens2,cfg.color_mentec,cfg.color_mensel,0);
m3=m_crea(MENU_VERT,NULL,opcionesc,2,31,cfg.color_men,cfg.color_mens1,
  cfg.color_mens2,cfg.color_mentec,cfg.color_mensel,0);
mayd=m_crea(MENU_VERT,NULL,opcionesa,2,52,cfg.color_men,cfg.color_mens1,
  cfg.color_mens2,cfg.color_mentec,cfg.color_mensel,0);
m_abre(m);

e_inicializa("",3,0,80,22,cfg.color_ved,cfg.color_veds1,cfg.color_veds2,
  cfg.color_vedblq,cfg.color_dlg,cfg.color_dlgs1,cfg.color_dlgs2,
  cfg.color_dlgtec,cfg.color_dlgsel,E_EDITA);
e_dibuja_editor();

/* inicializa variables */
*base_datos='\0';
for(i=0; i<MAX_PRO; i++) pro_existe[i]=FALSE;
for(i=0; i<MAX_TMSG; i++) msg_existe[i]=FALSE;

acerca_de();

do {
	o=m_elige_opcion(m);

	switch(o) {
		case -1 :
			edita_seccion(NULL,1);
			break;
		case 0 :
			m_abre(m0);
			o1=m_elige_opcion(m0);
			m_cierra(m0);
			if(o1==0) acerca_de();
			else if(o1==2) comando_dos();
			break;
		case 1 :
			m_abre(m1);
			o1=m_elige_opcion(m1);
			m_cierra(m1);
			if(o1==0) {
				graba_bd_modif();
				c_selecc_ficheros(C_CENT,C_CENT,
				  " Nueva base datos ",cfg.color_dlg,
				  cfg.color_dlgs1,cfg.color_dlgs2,
				  cfg.color_dlgtec,cfg.color_dlgsel,
				  cfg.dir_bd,"*.BD",nbd);
				if(*nbd) {
					borra_fichtemp();
					if(!crea_bd(nbd)) break;
					strcpy(base_datos,nbd);
					csecc[0]='$';
					strcpy(&csecc[1],Mconst);
					separa_seccion(Mconst,csecc);
					e_carga_texto(csecc);
					e_dibuja_editor();
				}
			}
			if(o1==1) {
				graba_bd_modif();
				c_selecc_ficheros(C_CENT,C_CENT,
				  " Abrir base datos ",cfg.color_dlg,
				  cfg.color_dlgs1,cfg.color_dlgs2,
				  cfg.color_dlgtec,cfg.color_dlgsel,
				  cfg.dir_bd,"*.BD",nbd);
				if(*nbd) {
					borra_fichtemp();
					if(!abre_bd(nbd)) break;
					strcpy(base_datos,nbd);
					csecc[0]='$';
					strcpy(&csecc[1],Mconst);
					separa_seccion(Mconst,csecc);
					e_carga_texto(csecc);
					e_dibuja_editor();
				}
			}
			else if(o1==2) {
				if(*base_datos) graba_bd(base_datos);
			}
			else if(o1==3) {
				c_selecc_ficheros(C_CENT,C_CENT,
				  " Grabar base datos ",cfg.color_dlg,
				  cfg.color_dlgs1,cfg.color_dlgs2,
				  cfg.color_dlgtec,cfg.color_dlgsel,
				  cfg.dir_bd,"*.BD",nbd);
				if(*nbd) {
					strcpy(base_datos,nbd);
					graba_bd(base_datos);
				}
			}
			else if(o1==5) graba_bloque();
			else if(o1==6) e_vacia_bloque();
			else if(o1==8) salida=1;
			break;
		case 2 :
			m_abre(m2);
			o1=m_elige_opcion(m2);
			m_cierra(m2);
			if(o1==0) edita_seccion(Mconst,1);
			else if(o1==1) edita_seccion(Mmsy,1);
			else if(o1==2) edita_seccion(Mvoc,1);
			else if(o1==3) {
				if(!*base_datos) break;
				i=elige_msg();
				if(i!=-1) {
					sprintf(csecc,"%s %i",Mmsg,i);
					edita_seccion(csecc,1);
				}
			}
			else if(o1==4) edita_seccion(Mloc,1);
			else if(o1==5) edita_seccion(Mobj,1);
			else if(o1==6) {
				if(!*base_datos) break;
				i=elige_pro();
				if(i!=-1) {
					sprintf(csecc,"%s %i",Mpro,i);
					edita_seccion(csecc,1);
				}
			}
			break;
		case 3 :
			m_abre(m3);
			o1=m_elige_opcion(m3);
			m_cierra(m3);
			if(o1==0) compila_bd();
			else if(o1==1) ejecuta_bd();
			else if(o1==2) errores_compil();
			else if(o1==4) linka_bd();
			break;
		case 4 :
			utilidades();
			break;
		case 5 :
			m_abre(mayd);
			o1=m_elige_opcion(mayd);
			m_cierra(mayd);
			if(o1==0) ayuda("");
			else if(o1==1) ayuda_sobre();
			else if(o1==3) ayuda("TECLAS DEL EDITOR");
			else if(o1==4) ayuda("CUADROS DE DIALOGO");
			else if(o1==5) ayuda("VARIABLES Y BANDERAS "
			  "DEL SISTEMA");
			else if(o1==6) ayuda("LISTA DE CONDACTOS");
			break;
	}
} while(!salida);

graba_bd_modif();
e_borra_texto();
borra_fichtemp();

m_cierra(m);
m_elimina(m0);
m_elimina(m1);
m_elimina(m2);
m_elimina(m2);
m_elimina(m3);
m_elimina(mayd);
m_elimina(m);

r_puntero(R_OCULTA);

asm {
	mov ax,0003h	// definir modo v¡deo (texto 80x25 color)
	int 10h
}

}
#pragma warn +par

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
	LEE_CFG: lee fichero de configuraci¢n si existe, si no asigna
	  valores por defecto.
	  Entrada:      'argv0' ruta y nombre del programa (normalmente
			contenido en argv[0])
****************************************************************************/
void lee_cfg(char *argv0)
{
char ruta[MAXPATH], drive[MAXDRIVE], dir[MAXDIR], fname[MAXFILE],
  ext[MAXEXT], nf_cfg[MAXPATH];
FILE *fcfg;

/* coge ruta de programa */
fnsplit(argv0,drive,dir,fname,ext);
strcpy(ruta,drive);
strcat(ruta,dir);
strupr(ruta);

/* valores de configuraci¢n por defecto */
strcpy(cfg.dir_sintac,ruta);
strcpy(cfg.dir_bd,ruta);
strcpy(cfg.dir_util,ruta);

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

/* lee fichero de configuraci¢n, si existe */
strcpy(nf_cfg,ruta);
strcat(nf_cfg,NF_CFG);
if((fcfg=fopen(nf_cfg,"rb"))!=NULL) {
	fread(&cfg,sizeof(STC_CFG),1,fcfg);
	fclose(fcfg);
}

}

/****************************************************************************
	ACERCA_DE: informaci¢n sobre el entorno.
****************************************************************************/
void acerca_de(void)
{
STC_CUADRO c;
STC_ELEM_TEXTO t={
	"      Entorno de desarrollo versi¢n "VERSION_EDS
	"\n       "COPYRIGHT, C_TXTPASA, C_TXTNOBORDE
};
STC_ELEM_BOTON b={
	" ^Vale"
};
int i;

c_crea_cuadro(&c,NULL,C_CENT,C_CENT,INF_ANCHO,INF_ALTO,
  cfg.color_dlg,cfg.color_dlgs1,cfg.color_dlgs2,cfg.color_dlgtec,
  cfg.color_dlgsel);
c_crea_elemento(&c,0,INF_ALTO-5,(INF_ANCHO-10)/2,8,3,cfg.color_dlg,
  cfg.color_dlgs1,cfg.color_dlgs2,C_ELEM_BOTON,&b);
c_crea_elemento(&c,1,0,0,INF_ANCHO-2,5,cfg.color_dlg,cfg.color_dlgs1,
  cfg.color_dlgs2,C_ELEM_TEXTO,&t);

c_abre(&c);
do {
	i=c_gestiona(&c);
} while((i!=-1) && (i!=0));
c_cierra(&c);

}

/****************************************************************************
	IMPRIME_ERROR: imprime mensaje de error.
	  Entrada:      'msg' mensaje de error
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

}

#if SHARE==0
/****************************************************************************
	AYUDA: muestra ayuda del entorno.
	  Entrada:      'tema' tema sobre el que mostrar ayuda
****************************************************************************/
void ayuda(char *tema)
{
char nf_ayd[MAXPATH];

a_inicializa(C_CENT,C_CENT,AYD_ANCHO,AYD_ALTO,cfg.color_ayd,cfg.color_ayds1,
  cfg.color_ayds2,cfg.color_aydtec,cfg.color_aydsel);
strcpy(nf_ayd,cfg.dir_sintac);
strcat(nf_ayd,"EDS");
a_ayuda(nf_ayd,tema);

}
#else
/****************************************************************************
	AYUDA: muestra mensaje de aviso de versi¢n "shareware"
	  Entrada:      'tema' tema sobre el que mostrar ayuda
****************************************************************************/
void ayuda(char *tema)
{
STC_CUADRO c;
STC_ELEM_TEXTO txt={
	" Ayuda en l¡nea no disponible\n"
	"   en la versi¢n 'shareware'", C_TXTPASA, C_TXTBORDE
};
STC_ELEM_BOTON vale={
	" ^Vale"
};
int i;

c_crea_cuadro(&c," AVISO ",C_CENT,C_CENT,34,9,cfg.color_err,cfg.color_errs1,
  cfg.color_errs2,cfg.color_errtec,cfg.color_errsel);
c_crea_elemento(&c,0,4,0,8,3,cfg.color_err,cfg.color_errs1,cfg.color_errs2,
  C_ELEM_BOTON,&vale);
c_crea_elemento(&c,1,0,0,32,4,cfg.color_err,cfg.color_errs2,cfg.color_errs1,
  C_ELEM_TEXTO,&txt);

c_abre(&c);
do {
	i=c_gestiona(&c);
} while((i!=-1) && (i!=0));
c_cierra(&c);

}
#endif

/****************************************************************************
	COMANDO_DOS: ejecuta un comando del sistema operativo.
****************************************************************************/
void comando_dos(void)
{
STC_CUADRO c;
STC_ELEM_INPUT inpcom={
	" ^Comando"
};
STC_ELEM_BOTON bejec={
	" ^Ejecutar"
};
STC_ELEM_BOTON bsalir={
	" ^Salir"
};
STC_ELEM_CHECK chkpausa={
	"^Pausa"
};
char comando[129], ant_dir[MAXDIR];
int i, shell=0, ant_drive;

comando[0]='\0';
inpcom.cadena=comando;
inpcom.longitud=128;

c_crea_cuadro(&c," Comando del DOS ",C_CENT,C_CENT,DOS_ANCHO,DOS_ALTO,
  cfg.color_dlg,cfg.color_dlgs1,cfg.color_dlgs2,cfg.color_dlgtec,
  cfg.color_dlgsel);
c_crea_elemento(&c,0,0,DOS_ANCHO-31,29,3,cfg.color_dlg,cfg.color_dlgs2,
  cfg.color_dlgs1,C_ELEM_INPUT,&inpcom);
c_crea_elemento(&c,1,3,DOS_ANCHO-23,12,3,cfg.color_dlg,cfg.color_dlgs1,
  cfg.color_dlgs2,C_ELEM_BOTON,&bejec);
c_crea_elemento(&c,2,3,DOS_ANCHO-11,9,3,cfg.color_dlg,cfg.color_dlgs1,
  cfg.color_dlgs2,C_ELEM_BOTON,&bsalir);
c_crea_elemento(&c,3,4,1,0,0,cfg.color_dlg,cfg.color_dlgs1,cfg.color_dlgs2,
  C_ELEM_CHECK,&chkpausa);
chkpausa.estado=1;

c_abre(&c);
i=c_gestiona(&c);
c_cierra(&c);
if(i==1) {
	m_cierra(m);
	r_puntero(R_OCULTA);

	asm {
		mov ax,0003h	// definir modo v¡deo (texto 80x25 color)
		int 10h
	}

	/* guarda unidad y directorio actuales */
	ant_drive=getdisk();
	getcwd(ant_dir,MAXDIR);

	if(!*comando) {
		strcpy(comando,"COMMAND");
		shell=1;
		printf("\nTeclea EXIT para regresar.\n");
	}
	system(comando);
	if(chkpausa.estado && !shell) {
		printf("\nPulsa una tecla.");
		bioskey(0);
	}

	/* recupera unidad y directorio actuales */
	setdisk(ant_drive);
	chdir(ant_dir);

	asm {
		mov ax,0003h	// definir modo v¡deo (texto 80x25 color)
		int 10h
	}

	m_abre(m);
	e_dibuja_editor();
	return;
}

}

/****************************************************************************
	GRABA_BLOQUE: graba el £ltimo bloque seleccionado.
****************************************************************************/
void graba_bloque(void)
{
char fichero[MAXPATH];

c_selecc_ficheros(C_CENT,C_CENT," Grabar bloque ",cfg.color_dlg,
  cfg.color_dlgs1,cfg.color_dlgs2,cfg.color_dlgtec,cfg.color_dlgsel,"","*.*",
  fichero);

if(*fichero) {
	if(e_graba_bloque(fichero)==0) imprime_error(Merr_Escr);
}

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
	MAX_MEM: comprueba m xima memoria disponible para funciones de
	  reserva de memoria (malloc)
	  Salida:	m xima cantidad de memoria disponible
****************************************************************************/
unsigned long max_mem(void)
{
struct heapinfo hi;
unsigned long maxtam=0;

/* si hay errores en memoria, devuelve 0 */
if(heapcheck()<0) return(0);

hi.ptr=NULL;

while(heapwalk(&hi)==_HEAPOK) {
	if((!hi.in_use) && (hi.size>maxtam)) maxtam=hi.size;
}

return(maxtam);
}

/****************************************************************************
	COPIA_FICHERO: copia un fichero en otro.
	  Entrada:      'nfichorg' nombre de fichero de origen
			'nfichdest' nombre de fichero destino
	  Salida:       1 si pudo copiar, 0 si error
****************************************************************************/
int copia_fichero(char *nfichorg, char *nfichdest)
{
char *buf, bufaux[256];
int horg, hdest, bufaux_usado=0;
long tam=0xff00L, flng;

esconde_cursor();

if((horg=open(nfichorg,O_BINARY | O_RDONLY))==-1) {
	imprime_error(Merr_Aper);
	return(0);
}
if((hdest=open(nfichdest,O_BINARY | O_WRONLY | O_CREAT | O_TRUNC,
  S_IREAD | S_IWRITE))==-1) {
	close(horg);
	imprime_error(Merr_Aper);
	return(0);
}

flng=filelength(horg);
if(flng<tam) tam=flng;

/* reserva memoria para buffer, si no hay suficiente memoria busca */
/* la m xima cantidad disponible */
if((buf=(char *)malloc((size_t)tam))==NULL) {
	tam=max_mem();
	/* si a£n asi no pudo reservar memoria, usa buffer auxiliar */
	if((buf=(char *)malloc((size_t)tam))==NULL) {
		buf=bufaux;
		tam=sizeof(bufaux);
		bufaux_usado=1;
	}
}

while(!eof(horg)) {
	if((tam=read(horg,buf,(unsigned)tam))==-1) {
		imprime_error(Merr_Lect);
		close(horg);
		close(hdest);
		if(!bufaux_usado) free(buf);
		return(0);
	}
	if(write(hdest,buf,(unsigned)tam)==-1) {
		imprime_error(Merr_Escr);
		close(horg);
		close(hdest);
		if(!bufaux_usado) free(buf);
		return(0);
	}
}

close(horg);
close(hdest);
if(!bufaux_usado) free(buf);

return(1);
}

/****************************************************************************
	CREA_BD: crea una nueva base de datos.
	  Entrada:      'bd' nombre del fichero de base de datos
	  Salida:       1 si la pudo crear, 0 si error
****************************************************************************/
int crea_bd(char *bd)
{
STC_CUADRO c;
STC_ELEM_TEXTO txt={
	"           ¨Usar base de datos de inicio?"
};
STC_ELEM_BOTON si={
	" ^Si"
};
STC_ELEM_BOTON no={
	" ^No"
};
STC_ELEM_BOTON salir={
	" Sa^lir"
};
FILE *fbd;
char *marca_fin="\n\\END\n\n";
char nf_bdinic[MAXPATH];
int i;

c_crea_cuadro(&c,NULL,C_CENT,C_CENT,AV_ANCHO,AV_ALTO,cfg.color_dlg,
  cfg.color_dlgs1,cfg.color_dlgs2,cfg.color_dlgtec,cfg.color_dlgsel);
c_crea_elemento(&c,0,AV_ALTO-5,17,6,3,cfg.color_dlg,cfg.color_dlgs1,
  cfg.color_dlgs2,C_ELEM_BOTON,&si);
c_crea_elemento(&c,1,AV_ALTO-5,23,6,3,cfg.color_dlg,cfg.color_dlgs1,
  cfg.color_dlgs2,C_ELEM_BOTON,&no);
c_crea_elemento(&c,2,AV_ALTO-5,29,9,3,cfg.color_dlg,cfg.color_dlgs1,
  cfg.color_dlgs2,C_ELEM_BOTON,&salir);
c_crea_elemento(&c,3,0,0,AV_ANCHO-2,3,cfg.color_dlg,cfg.color_dlgs1,
  cfg.color_dlgs2,C_ELEM_TEXTO,&txt);

c_abre(&c);
i=c_gestiona(&c);
c_cierra(&c);

if((i==-1) || (i==2)) return(0);
else if(i==0) {
	strcpy(nf_bdinic,cfg.dir_bd);
	strcat(nf_bdinic,NF_BDINIC);
	if(!copia_fichero(nf_bdinic,bd)) return(0);
}
else if(i==1) {
	if((fbd=fopen(bd,"wt"))==NULL) return(0);

	fputs("\\\\V_MOV\t\t14\t; m ximo n§ para verbos de movimiento\n",fbd);
	fputs("\\\\N_CONV\t20\t; m ximo n§ para nombres convertibles\n",fbd);
	fputs("\\\\N_PROP\t50\t; m xnimo n§ para nombres propios\n\n",fbd);

	fputs("\\MSY\n",fbd);
	fputs(marca_fin,fbd);

	fputs("\\VOC\n",fbd);
	fputs(marca_fin,fbd);

	fputs("\\MSG 0\n",fbd);
	fputs(marca_fin,fbd);

	fputs("\\LOC\n",fbd);
	fputs(marca_fin,fbd);

	fputs("\\OBJ\n",fbd);
	fputs(marca_fin,fbd);

	fputs("\\PRO 0\n",fbd);
	fputs(marca_fin,fbd);

	fclose(fbd);
}

bd_modificada=TRUE;
if(!abre_bd(bd)) return(0);

return(1);
}

/****************************************************************************
	ABRE_BD: abre una base de datos.
	  Entrada:      'bd' nombre del fichero de base de datos
	  Salida:       1 si la pudo abrir, 0 si error
****************************************************************************/
int abre_bd(char *bd)
{

/* copia el fichero de base de datos en fichero temporal */
if(!copia_fichero(bd,Nf_Bdtemp)) return(0);

return(1);
}

/****************************************************************************
	ES_MARCA_SECC: comprueba si una l¡nea es marca de secci¢n.
	  Entrada:      'lin' l¡nea a comprobar
	  Salida:       1 si es marca de secci¢n, 0 si no
****************************************************************************/
int es_marca_secc(char *lin)
{

/* salta blancos iniciales */
for(; ((*lin==' ') || (*lin=='\t')); lin++);

/* mira es marca de secci¢n (y no definici¢n de constante) */
if((*lin==MARCA_S) && (*(lin+1)!=MARCA_S)) return(1);

return(0);
}

/****************************************************************************
	BUSCA_MARCA: busca una marca de inicio o final de secci¢n.
	  Entrada:      'marca' marca a buscar ("\MSY", "\MSG", "\END",
			etc...)
			'l' l¡nea a analizar
	  Salida:       1 si la encontr¢, 0 si no
****************************************************************************/
int busca_marca(char *marca, char *l)
{

/* salta blancos iniciales */
for(; (*l && ((*l==' ') || (*l=='\t'))); l++);

/* mira si encuentra la marca */
for(; (*marca && *l); marca++, l++) if(*marca!=*l) return(0);

return(1);
}

/****************************************************************************
	SALTA_HASTA_MARCA: salta texto de fichero de base de datos
	  hasta la siguiente marca de secci¢n, deja el puntero del fichero
	  en la l¡nea que contiene la marca.
	  Entrada:      'fbd' puntero a fichero de base de datos
	  Salida:       1 se complet¢ operaci¢n, 0 si error
****************************************************************************/
int salta_hasta_marca(FILE *fbd)
{
char c[E_MAXLNGLIN];
fpos_t pos;

while(1) {
	fgetpos(fbd,&pos);
	if(fgets(c,E_MAXLNGLIN,fbd)==NULL) {
		if(feof(fbd)) break;
		else {
			imprime_error(Merr_Lect);
			return(0);
		}
	}

	if(es_marca_secc(c)) {
		fsetpos(fbd,&pos);
		break;
	}
}

return(1);
}

/****************************************************************************
	COGE_MARCA: recoge la marca de secci¢n de una l¡nea de texto.
	  Entrada:      'l' l¡nea de texto a analizar
			'marca' puntero a buffer donde se guardar  la
			marca de secci¢n (m ximo 80 caracteres)
****************************************************************************/
void coge_marca(char *l, char *marca)
{
int i;

/* salta blancos iniciales */
for(; (*l && ((*l==' ') || (*l=='\t'))); l++);

for(i=0; ((i<80) && (*l!=' ') && (*l!='\t') && (*l!='\n') &&
  (*l!=CHR_COMENT) && *l); l++, marca++, i++) *marca=*l;
*marca='\0';

}

/****************************************************************************
	PON_EXTENSIONSECC: coloca al nombre de fichero temporal la
	  extensi¢n correspondiente de una secci¢n m£ltiple (el n£mero
	  que acompa¤a a la marca de inicio de secci¢n)
	  Entrada:      'csecc' marca de inicio de secci¢n
			'nf' nombre de fichero temporal sin extensi¢n
	  Salida:       n£mero que acompa¤a a marca de secci¢n, -1 si error
****************************************************************************/
int pon_extensionsecc(char *csecc, char *nf)
{
int i;
char num[4];

/* salta blancos iniciales */
for(; ((*csecc==' ') || (*csecc=='\t')); csecc++);

/* salta caracteres hasta siguiente car cter blanco o fin de cadena */
for(; ((*csecc!=' ') && (*csecc!='\t') && (*csecc!='\0') && (*csecc!='\n'));
  csecc++);

/* si ha encontrado final de cadena, da error */
if((*csecc=='\0') || (*csecc=='\n')) return(-1);

/* salta blancos */
for(; ((*csecc==' ') || (*csecc=='\t')); csecc++);

/* se supone que lo siguiente, hasta el final de la cadena o */
/* hasta el siguiente blanco, es un n£mero de 3 d¡gitos m ximo */
for(i=0; ((*csecc!=' ') && (*csecc!='\t') && (*csecc!='\0') &&
  (*csecc!='\n') && (i<3)); csecc++, i++) {
	num[i]=*csecc;
}
num[i]='\0';

/* a¤ade extensi¢n a nombre de fichero temporal */
strcat(nf,".");
strcat(nf,num);

return(atoi(num));
}

/****************************************************************************
	COPIA_FICHSECC: copia fichero temporal de secci¢n en fichero
	  de base de datos.
	  Entrada:      'nfsecc' nombre de fichero de secci¢n
			'fbd' puntero a fichero de base de datos
			'msecc' marca de inicio de secci¢n (NULL ninguna)
	  Salida:       1 si lo pudo copiar, 0 si error
****************************************************************************/
int copia_fichsecc(char *nfsecc, FILE *fbd, char *msecc)
{
FILE *fsecc;
char *buf, bufaux[256];
int bufaux_usado=0;
long tam=0xff00L, flng;

if((fsecc=fopen(nfsecc,"rt"))==NULL) {
	imprime_error(Merr_Aper);
	return(0);
}

flng=filelength(fileno(fsecc));
if(flng<tam) tam=flng;

/* reserva memoria para buffer, si es un fichero de longitud 0 */
/* indica buffer auxiliar usado (aunque realmente no se usa) para */
/* evitar tener que reservar y liberar memoria */
if(!flng) bufaux_usado=1;
else if((buf=(char *)malloc((size_t)tam))==NULL) {
	/* si no hay suficiente memoria busca lo m ximo disponible */
	tam=max_mem();
	/* si a£n asi no pudo reservar memoria, usa buffer auxiliar */
	if((buf=(char *)malloc((size_t)tam))==NULL) {
		buf=bufaux;
		tam=sizeof(bufaux);
		bufaux_usado=1;
	}
}

/* escribe marca de inicio de secci¢n */
if(msecc!=NULL) fprintf(fbd,"%s\n",msecc);

if(flng) while(!feof(fsecc)) {
	tam=fread(buf,sizeof(char),(size_t)tam,fsecc);
	if(ferror(fsecc)) {
		imprime_error(Merr_Lect);
		fclose(fsecc);
		if(!bufaux_usado) free(buf);
		return(0);
	}

	if(fwrite(buf,sizeof(char),(size_t)tam,fbd)!=tam) {
		imprime_error(Merr_Escr);
		fclose(fsecc);
		if(!bufaux_usado) free(buf);
		return(0);
	}
}

/* escribe marca de fin de secci¢n */
if(msecc!=NULL) fprintf(fbd,"%s\n",Mfin_Secc);

fclose(fsecc);
if(!bufaux_usado) free(buf);

return(1);
}

/****************************************************************************
	COPIA_SECC: copia secci¢n de fichero de base de datos a otro
	  fichero.
	  Entrada:      'fbd' puntero a fichero de base de datos
			'faux' puntero a fichero donde se copiar  secci¢n
			'msecc' marca de inicio de secci¢n (NULL ninguna)
	  Salida:       1 si pudo copiar, 0 si error
****************************************************************************/
int copia_secc(FILE *fbd, FILE *faux, char *msecc)
{
char c[E_MAXLNGLIN];

/* escribe marca de inicio de secci¢n */
if(msecc!=NULL) fprintf(faux,"%s\n",msecc);

/* copia l¡neas hasta encontrar marca de secci¢n */
while(1) {
	if(fgets(c,E_MAXLNGLIN,fbd)==NULL) {
		if(feof(fbd)) break;
		else {
			imprime_error(Merr_Lect);
			return(0);
		}
	}

	if(es_marca_secc(c)) break;

	if(fputs(c,faux)==EOF) {
		imprime_error(Merr_Escr);
		return(0);
	}
}

/* escribe marca de fin de secci¢n */
if(msecc!=NULL) fprintf(faux,"%s\n",Mfin_Secc);

return(1);
}

/****************************************************************************
	ACTUALIZA_BDTEMP: actualiza el fichero de base de datos temporal,
	  'Nf_Bdtemp' con el contenido de los fichero temporales asociados
	  a cada secci¢n.
	  Salida:       1 si se complet¢ la operaci¢n, 0 si error
****************************************************************************/
int actualiza_bdtemp(void)
{
FILE *fbd, *faux;
char nfsecc[13], c[E_MAXLNGLIN], marca[81];
int i;

esconde_cursor();

/* si base de datos no ha sido modificada, no hace falta actualizarla */
if(bd_modificada==FALSE) return(1);

if((fbd=fopen(Nf_Bdtemp,"rt"))==NULL) {
	imprime_error(Merr_Aper);
	return(0);
}
if((faux=fopen(Nf_Bdaux,"wt"))==NULL) {
	imprime_error(Merr_Aper);
	fclose(fbd);
	return(0);
}

/* mira secciones de la base de datos, si alguna tiene asociado */
/* un fichero temporal copia el contenido del fichero, si no deja */
/* la secci¢n como est  */

/* constantes */
sprintf(nfsecc,Nf_Secc,Mconst);
if(!access(nfsecc,0)) {
	if(!copia_fichsecc(nfsecc,faux,NULL)) {
		fclose(faux);
		fclose(fbd);
		return(0);
	}
	if(!salta_hasta_marca(fbd)) {
		fclose(faux);
		fclose(fbd);
		return(0);
	}
}
else {
	if(!copia_secc(fbd,faux,NULL)) {
		fclose(faux);
		fclose(fbd);
		return(0);
	}
	rewind(fbd);
	if(!salta_hasta_marca(fbd)) {
		fclose(faux);
		fclose(fbd);
		return(0);
	}
}

while(1) {
	if(fgets(c,E_MAXLNGLIN,fbd)==NULL) {
		if(feof(fbd)) break;
		else {
			imprime_error(Merr_Lect);
			fclose(faux);
			fclose(fbd);
			return(0);
		}
	}

	/* si encuentra marca de secci¢n, mira cual es y comprueba si */
	/* existe fichero temporal para esa secci¢n */
	if(es_marca_secc(c)) {
		coge_marca(c,marca);
		sprintf(nfsecc,Nf_Secc,&marca[1]);
		if(!strcmp(marca,Mpro)) {
			i=pon_extensionsecc(c,nfsecc);
			sprintf(marca,"%s %i",Mpro,i);
			if(pro_existe[i]) {
				if(!copia_fichsecc(nfsecc,faux,marca)) {
					fclose(faux);
					fclose(fbd);
					return(0);
				}
				if(!salta_hasta_marca(fbd)) {
					fclose(faux);
					fclose(fbd);
					return(0);
				}
				fgets(c,E_MAXLNGLIN,fbd);
			}
			else if(!copia_secc(fbd,faux,marca)) {
				fclose(faux);
				fclose(fbd);
				return(0);
			}
		}
		else if(!strcmp(marca,Mmsg)) {
			i=pon_extensionsecc(c,nfsecc);
			sprintf(marca,"%s %i",Mmsg,i);
			if(msg_existe[i]) {
				if(!copia_fichsecc(nfsecc,faux,marca)) {
					fclose(faux);
					fclose(fbd);
					return(0);
				}
				if(!salta_hasta_marca(fbd)) {
					fclose(faux);
					fclose(fbd);
					return(0);
				}
				fgets(c,E_MAXLNGLIN,fbd);
			}
			else if(!copia_secc(fbd,faux,marca)) {
				fclose(faux);
				fclose(fbd);
				return(0);
			}
		}
		else if(!access(nfsecc,0)) {
			if(!copia_fichsecc(nfsecc,faux,marca)) {
				fclose(faux);
				fclose(fbd);
				return(0);
			}
			if(!salta_hasta_marca(fbd)) {
				fclose(faux);
				fclose(fbd);
				return(0);
			}
			fgets(c,E_MAXLNGLIN,fbd);
		}
		else if(!copia_secc(fbd,faux,marca)) {
			fclose(faux);
			fclose(fbd);
			return(0);
		}
	 }
}

fclose(faux);
fclose(fbd);

/* borra fichero temporal de base de datos y renombra fichero auxiliar */
/* a fichero temporal de base de datos */
remove(Nf_Bdtemp);
rename(Nf_Bdaux,Nf_Bdtemp);

return(1);
}

/****************************************************************************
	GRABA_BD: graba la base de datos actual.
	  Entrada:      'bd' nombre de fichero donde se grabar 
	  Salida:       1 si se pudo grabar, 0 si error
****************************************************************************/
int graba_bd(char *bd)
{

if(!actualiza_bdtemp()) return(0);
if(!copia_fichero(Nf_Bdtemp,bd)) return(0);

bd_modificada=FALSE;

return(1);
}

/****************************************************************************
	GRABA_BD_MODIF: graba la base de datos actual si ha sido modificada.
	  Salida:       1 si se pudo grabar, 0 si error
****************************************************************************/
int graba_bd_modif(void)
{
STC_CUADRO c;
STC_ELEM_TEXTO txt={
	"          Base de datos ha sido modificada\n"
	"                     ¨Grabarla?", C_TXTLINEA, C_TXTNOBORDE
};
STC_ELEM_BOTON si={
	" ^Si"
};
STC_ELEM_BOTON no={
	" ^No"
};
int i;

if(bd_modificada==TRUE) {
	c_crea_cuadro(&c,NULL,C_CENT,C_CENT,AV_ANCHO,AV_ALTO,
	  cfg.color_dlg,cfg.color_dlgs1,cfg.color_dlgs2,cfg.color_dlgtec,
	  cfg.color_dlgsel);
	c_crea_elemento(&c,0,AV_ALTO-5,21,6,3,cfg.color_dlg,
	  cfg.color_dlgs1,cfg.color_dlgs2,C_ELEM_BOTON,&si);
	c_crea_elemento(&c,1,AV_ALTO-5,27,6,3,cfg.color_dlg,
	  cfg.color_dlgs1,cfg.color_dlgs2,C_ELEM_BOTON,&no);
	c_crea_elemento(&c,2,0,0,AV_ANCHO-2,4,cfg.color_dlg,
	  cfg.color_dlgs1,cfg.color_dlgs2,C_ELEM_TEXTO,&txt);

	c_abre(&c);
	i=c_gestiona(&c);
	c_cierra(&c);

	if(i==0) {
		if(!graba_bd(base_datos)) return(0);
		else return(1);
	}
}

return(1);
}

/****************************************************************************
	BORRA_FICHTEMP: borra ficheros temporales usados por el entorno.
****************************************************************************/
void borra_fichtemp(void)
{
char nfsecc[13];
int i;

esconde_cursor();

remove(Nf_Bdtemp);
remove(Nf_Bdaux);
remove(Nf_Err);

sprintf(nfsecc,Nf_Secc,Mconst);
remove(nfsecc);
sprintf(nfsecc,Nf_Secc,&Mmsy[1]);
remove(nfsecc);
sprintf(nfsecc,Nf_Secc,&Mvoc[1]);
remove(nfsecc);

for(i=0; i<MAX_TMSG; i++) {
	if(msg_existe[i]==TRUE) {
		sprintf(nfsecc,Nf_Seccmult,&Mmsg[1],i);
		remove(nfsecc);
		msg_existe[i]=FALSE;
	}
}

sprintf(nfsecc,Nf_Secc,&Mloc[1]);
remove(nfsecc);
sprintf(nfsecc,Nf_Secc,&Mobj[1]);
remove(nfsecc);

for(i=0; i<MAX_PRO; i++) {
	if(pro_existe[i]==TRUE) {
		sprintf(nfsecc,Nf_Seccmult,&Mpro[1],i);
		remove(nfsecc);
		pro_existe[i]=FALSE;
	}
}

}

/****************************************************************************
	SEPARA_SECCION: separa del fichero de base de datos temporal
	  'Nf_Bdtemp' una secci¢n y la almacena en un fichero temporal.
	  Entrada:      'csecc' cabecera de la secci¢n ("ORG", "\VOC",
			"\PRO 1",...), si es "ORG" separa el inicio del
			fichero hasta la primera marca de secci¢n
			'nfsecc' nombre del fichero donde se guardar  la
			secci¢n
	  Salida:       1 si se pudo completar operaci¢n, 0 si error
****************************************************************************/
int separa_seccion(char *csecc, char *nfsecc)
{
FILE *fbd, *fsecc;
char c[E_MAXLNGLIN];
BOOLEAN fin_seccion=FALSE;

esconde_cursor();

if((fbd=fopen(Nf_Bdtemp,"rt"))==NULL) {
	imprime_error(Merr_Aper);
	return(0);
}

if((fsecc=fopen(nfsecc,"wt"))==NULL) {
	imprime_error(Merr_Aper);
	fclose(fbd);
	return(0);
}

if(!strcmp(csecc,Mconst)) while(1) {
	if(fgets(c,E_MAXLNGLIN,fbd)==NULL) {
		if(feof(fbd)) break;
		else {
			imprime_error(Merr_Lect);
			fclose(fbd);
			fclose(fsecc);
			return(0);
		}
	}

	/* comprueba si ha encontrado una marca de secci¢n */
	if(es_marca_secc(c)) break;

	if(fputs(c,fsecc)==EOF) {
		imprime_error(Merr_Escr);
		fclose(fbd);
		fclose(fsecc);
		return(0);
	}
}
else while(fin_seccion==FALSE) {
	if(fgets(c,E_MAXLNGLIN,fbd)==NULL) {
		if(feof(fbd)) {
			imprime_error(Merr_Nosecc);
			fclose(fbd);
			fclose(fsecc);
			return(0);
		}
		else {
			imprime_error(Merr_Lect);
			fclose(fbd);
			fclose(fsecc);
			return(0);
		}
	}

	/* si encuentra encabezamiento de secci¢n, copia l¡neas hasta */
	/* encontrar marca de fin de secci¢n */
	if(busca_marca(csecc,c)) {
		while(1) {
			if(fgets(c,E_MAXLNGLIN,fbd)==NULL) {
				if(feof(fbd)) {
					imprime_error(Merr_Noend);
					fclose(fbd);
					fclose(fsecc);
					return(0);
				}
				else {
					imprime_error(Merr_Lect);
					fclose(fbd);
					fclose(fsecc);
					return(0);
				}
			}

			if(busca_marca(Mfin_Secc,c)) {
				fin_seccion=TRUE;
				break;
			}

			if(fputs(c,fsecc)==EOF) {
				imprime_error(Merr_Escr);
				fclose(fbd);
				fclose(fsecc);
				return(0);
			}
		}
	}
}

fclose(fbd);
fclose(fsecc);

return(1);
}

/****************************************************************************
	EDITA_SECCION: edita una secci¢n de la base de datos.
	  Entrada:      'csecc' marca de inicio de secci¢n ("ORG", "\VOC",
			"\PRO 1",...), NULL si quiere editar £ltima secci¢n
			que fue editada
			'nlin' n£mero de l¡nea donde se colocar  el cursor
	  Salida:       1 si se pudo editar, 0 si error
****************************************************************************/
int edita_seccion(char *csecc, int nlin)
{
static char ultsecc[9]="ORG";
char *secc, nftemp[13];
int i;

if(!*base_datos) return(1);

/* comprueba si quiere editar £ltima secci¢n editada */
if(csecc==NULL) secc=ultsecc;
else secc=csecc;

/* crea nombre de fichero temporal correspondiente a la secci¢n */
nftemp[0]='$';
if(!strcmp(secc,Mconst)) strcpy(&nftemp[1],Mconst);
else if(strstr(secc,Mpro)!=NULL) {
	strcpy(&nftemp[1],&Mpro[1]);
	i=pon_extensionsecc(secc,nftemp);
}
else if(strstr(secc,Mmsg)!=NULL) {
	strcpy(&nftemp[1],&Mmsg[1]);
	i=pon_extensionsecc(secc,nftemp);
}
else strcpy(&nftemp[1],&secc[1]);

/* si no existe fichero temporal de esa secci¢n, lo crea */
if(access(nftemp,0)) {
	if(!separa_seccion(secc,nftemp)) {
		remove(nftemp);
		return(0);
	}
}

/* si es secci¢n m£ltiple, indica que ha sido creado fichero temporal */
if(strstr(secc,Mpro)!=NULL) pro_existe[i]=TRUE;
else if(strstr(secc,Mmsg)!=NULL) msg_existe[i]=TRUE;

/* edita secci¢n */
if(!e_carga_texto(nftemp)) {
	imprime_error(Merr_Lect);
	e_borra_texto();
	return(0);
}

e_pon_cursor(nlin,0);

do {
	i=e_editor();
	if(i==1) ayuda("");
	else if(i==2) ayuda(e_palabra_cursor());
} while(i);

/* guarda secci¢n que ha sido editada */
if(secc!=ultsecc) strcpy(ultsecc,secc);

if(e_modificado()) {
	bd_modificada=TRUE;
	if(!e_graba_texto(NULL)) {
		imprime_error(Merr_Escr);
		return(0);
	}
}

return(1);
}

/****************************************************************************
	CREA_SECCMULT: crea una secci¢n m£ltiple nueva en fichero temporal
	  de base de datos 'Nf_Bdtemp'.
	  Entrada:      'ns' n£mero de secci¢n
			'msecc' marca de inicio de secci¢n
			'merr_ex' mensaje de error que se imprimir  si la
			secci¢n ya existe
			'secc_existe' puntero a tabla de comprobaci¢n de
			existencia de ficheros temporales de secciones
			m£ltiples
	  Salida:       1 si pudo crearla, 0 si error
****************************************************************************/
int crea_seccmult(int ns, char *msecc, char *merr_ex, BOOLEAN *secc_existe)
{
FILE *fbd, *faux;
char c[E_MAXLNGLIN], *p, num[17], nfsecc[13];
int i, numero, secc_encontrada=0;

esconde_cursor();

if((fbd=fopen(Nf_Bdtemp,"rt"))==NULL) {
	imprime_error(Merr_Aper);
	return(0);
}

if((faux=fopen(Nf_Bdaux,"wt"))==NULL) {
	imprime_error(Merr_Aper);
	fclose(fbd);
	return(0);
}

while(1) {
	if(fgets(c,E_MAXLNGLIN,fbd)==NULL) {
		if(feof(fbd)) break;
		else {
			imprime_error(Merr_Lect);
			fclose(fbd);
			fclose(faux);
			return(0);
		}
	}

	/* si es marca de inicio de secci¢n, comprueba su n£mero */
	if(busca_marca(msecc,c)) {
		/* indica que se ha encontrado secci¢n */
		secc_encontrada=1;

		/* busca '\' */
		for(p=c; *p!=MARCA_S; p++);

		/* avanza hasta primer blanco */
		for(; ((*p!=' ') && (*p!='\t')); p++);

		/* salta espacios */
		for(; ((*p==' ') || (*p=='\t')); p++);

		/* recoge n£mero, m ximo 3 d¡gitos */
		for(i=0; i<3; i++, p++) {
			if(strchr(CNum,*p)==NULL) break;
			num[i]=*p;
		}
		num[i]='\0';

		numero=atoi(num);

		/* si n£mero de secci¢n encontrada es mayor que la */
		/* que queremos insertar, sale de bucle */
		if(numero>ns) break;

		/* si es igual finaliza */
		if(numero==ns) {
			imprime_error(merr_ex);
			fclose(fbd);
			fclose(faux);
			return(0);
		}
	}
	/* si ha encontrado otra secci¢n, para de buscar e inserta */
	/* secci¢n m£ltiple (se supone que las secciones m£ltiples */
	/* van una detr s de otra en la base de datos) */
	else if(es_marca_secc(c) && !busca_marca(Mfin_Secc,c) &&
	  secc_encontrada) break;

	if(fputs(c,faux)==EOF) {
		imprime_error(Merr_Escr);
		fclose(fbd);
		fclose(faux);
		return(0);
	}
}

bd_modificada=TRUE;

/* inserta secci¢n nueva */
fprintf(faux,"%s %i\n",msecc,ns);
fprintf(faux,"%s\n",Mfin_Secc);

/* copia resto de 'Nf_Bdtemp' en 'Nf_Bdaux' */
/* pone condici¢n de que no est‚ al final del fichero 'fbd' ya que si */
/* es as¡, la nueva tabla se insert¢ al final */
while(!feof(fbd)) {
	if(fputs(c,faux)==EOF) {
		imprime_error(Merr_Escr);
		fclose(fbd);
		fclose(faux);
		return(0);
	}

	if(fgets(c,E_MAXLNGLIN,fbd)==NULL) {
		if(feof(fbd)) break;
		else {
			imprime_error(Merr_Lect);
			fclose(fbd);
			fclose(faux);
			return(0);
		}
	}
}

fclose(fbd);
fclose(faux);

/* borra fichero temporal de base de datos y renombra fichero auxiliar */
/* a fichero temporal de base de datos */
remove(Nf_Bdtemp);
rename(Nf_Bdaux,Nf_Bdtemp);

/* crea fichero temporal vac¡o */
sprintf(nfsecc,Nf_Seccmult,&msecc[1],ns);
if((faux=fopen(nfsecc,"wt"))==NULL) {
	imprime_error(Merr_Aper);
	return(0);
}
fclose(faux);
secc_existe[ns]=TRUE;

return(1);
}

/****************************************************************************
	BORRA_SECCMULT: borra una secci¢n m£ltiple del fichero temporal
	  de base de datos 'Nf_Bdtemp'.
	  Entrada:      'ns' n£mero de secci¢n
			'msecc' marca de inicio de secci¢n
			'merr_noex' mensaje de error que se imprimir  si la
			secci¢n no existe
			'secc_existe' puntero a tabla de comprobaci¢n de
			existencia de ficheros temporales de secciones
			m£ltiples
	  Salida:       1 si pudo borrarla, 0 si error
****************************************************************************/
int borra_seccmult(int ns, char *msecc, char *merr_noex, BOOLEAN *secc_existe)
{
FILE *fbd, *faux;
char c[E_MAXLNGLIN], *p, num[17], nfsecc[13];
int i, numero;

esconde_cursor();

if((fbd=fopen(Nf_Bdtemp,"rt"))==NULL) {
	imprime_error(Merr_Aper);
	return(0);
}

if((faux=fopen(Nf_Bdaux,"wt"))==NULL) {
	imprime_error(Merr_Aper);
	fclose(fbd);
	return(0);
}

while(1) {
	if(fgets(c,E_MAXLNGLIN,fbd)==NULL) {
		if(feof(fbd)) break;
		else {
			imprime_error(Merr_Lect);
			fclose(fbd);
			fclose(faux);
			return(0);
		}
	}

	/* si es marca de inicio de secci¢n, comprueba su n£mero */
	if(busca_marca(msecc,c)) {
		/* busca '\' */
		for(p=c; *p!=MARCA_S; p++);

		/* avanza hasta primer blanco */
		for(; ((*p!=' ') && (*p!='\t')); p++);

		/* salta espacios */
		for(; ((*p==' ') || (*p=='\t')); p++);

		/* recoge n£mero, m ximo 3 d¡gitos */
		for(i=0; i<3; i++, p++) {
			if(strchr(CNum,*p)==NULL) break;
			num[i]=*p;
		}
		num[i]='\0';

		numero=atoi(num);

		/* si n£mero de secci¢n encontrada es mayor que la */
		/* que queremos borrar finaliza */
		if(numero>ns) {
			imprime_error(merr_noex);
			fclose(fbd);
			fclose(faux);
			return(0);
		}

		/* si es igual sale de bucle */
		if(numero==ns) break;
	}

	if(fputs(c,faux)==EOF) {
		imprime_error(Merr_Escr);
		fclose(fbd);
		fclose(faux);
		return(0);
	}
}

bd_modificada=TRUE;

/* salta hasta marca de fin de seccion */
while(1) {
	if(fgets(c,E_MAXLNGLIN,fbd)==NULL) {
		if(feof(fbd)) break;
		else {
			imprime_error(Merr_Lect);
			fclose(fbd);
			fclose(faux);
			return(0);
		}
	}

	if(busca_marca(Mfin_Secc,c)) break;
}

/* copia resto de 'Nf_Bdtemp' en 'Nf_Bdaux' */
while(1) {
	if(fgets(c,E_MAXLNGLIN,fbd)==NULL) {
		if(feof(fbd)) break;
		else {
			imprime_error(Merr_Lect);
			fclose(fbd);
			fclose(faux);
			return(0);
		}
	}

	if(fputs(c,faux)==EOF) {
		imprime_error(Merr_Escr);
		fclose(fbd);
		fclose(faux);
		return(0);
	}
}

fclose(fbd);
fclose(faux);

/* borra fichero temporal de base de datos y renombra fichero auxiliar */
/* a fichero temporal de base de datos */
remove(Nf_Bdtemp);
rename(Nf_Bdaux,Nf_Bdtemp);

/* si existe fichero temporal asociado a esa secci¢n, lo borra */
if(secc_existe[ns]==TRUE) {
	sprintf(nfsecc,Nf_Seccmult,&msecc[1],ns);
	remove(nfsecc);
	secc_existe[ns]=FALSE;
}

return(1);
}

/****************************************************************************
	PREGUNTA_SIONO: presenta un cuadro de di logo con una pregunta
	  y dos opciones (si o no).
	  Entrada:      'preg' texto de la pregunta
	  Salida:       1 si contest¢ si, 0 si no
****************************************************************************/
int pregunta_siono(char *preg)
{
STC_CUADRO c;
STC_ELEM_BOTON si={
	" ^Si"
};
STC_ELEM_BOTON no={
	" ^No"
};
STC_ELEM_TEXTO txt={
	NULL, C_TXTLINEA, C_TXTBORDE
};
short ancho;
int i;

ancho=strlen(preg)+4;
txt.texto=preg;

c_crea_cuadro(&c,NULL,C_CENT,C_CENT,ancho,8,cfg.color_dlg,cfg.color_dlgs1,
  cfg.color_dlgs2,cfg.color_dlgtec,cfg.color_dlgsel);
c_crea_elemento(&c,0,3,0,6,3,cfg.color_dlg,cfg.color_dlgs1,cfg.color_dlgs2,
  C_ELEM_BOTON,&si);
c_crea_elemento(&c,1,3,6,6,3,cfg.color_dlg,cfg.color_dlgs1,cfg.color_dlgs2,
  C_ELEM_BOTON,&no);
c_crea_elemento(&c,2,0,0,ancho-2,3,cfg.color_dlg,cfg.color_dlgs2,
  cfg.color_dlgs1,C_ELEM_TEXTO,&txt);

c_abre(&c);
i=c_gestiona(&c);
c_cierra(&c);

if(i==0) return(1);

return(0);
}

/****************************************************************************
	ELIGE_PRO: presenta una lista de procesos de la base de datos
	  y permite elegir uno; se usa la base de datos temporal
	  'Nf_Bdtemp'.
	  Salida:       n£mero de proceso seleccionado o -1 si no
			seleccion¢ ninguno o error
****************************************************************************/
int elige_pro(void)
{
STC_CUADRO c;
STC_ELEM_LISTA pro={
	" ^Procesos ", C_LSTNORMAL, C_LSTSINORDEN
};
STC_ELEM_BOTON vale={
	" ^Vale"
};
STC_ELEM_BOTON borrar={
	" ^Borrar"
};
STC_ELEM_BOTON salir={
	" ^Salir"
};
STC_ELEM_BOTON nuevo_pro={
	" ^Nuevo proceso"
};
STC_ELEM_INPUT numpro={
	"P^ro"
};
FILE *fbd, *fsecc;
char l[E_MAXLNGLIN], nfsecc[13], buff[81], npro[4];
int i, lng, encontrado;

c_crea_cuadro(&c,NULL,C_CENT,C_CENT,PRO_ANCHO,PRO_ALTO,cfg.color_dlg,
  cfg.color_dlgs1,cfg.color_dlgs2,cfg.color_dlgtec,cfg.color_dlgsel);
c_crea_elemento(&c,0,0,0,PRO_ANCHO-2,PRO_ALTO-5,cfg.color_dlg,cfg.color_dlgs2,
  cfg.color_dlgs1,C_ELEM_LISTA,&pro);
c_crea_elemento(&c,1,PRO_ALTO-5,0,8,3,cfg.color_dlg,cfg.color_dlgs1,
  cfg.color_dlgs2,C_ELEM_BOTON,&vale);
c_crea_elemento(&c,2,PRO_ALTO-5,8,10,3,cfg.color_dlg,
  cfg.color_dlgs1,cfg.color_dlgs2,C_ELEM_BOTON,&borrar);
c_crea_elemento(&c,3,PRO_ALTO-5,18,9,3,cfg.color_dlg,cfg.color_dlgs1,
  cfg.color_dlgs2,C_ELEM_BOTON,&salir);
c_crea_elemento(&c,4,PRO_ALTO-5,PRO_ANCHO-31,17,3,cfg.color_dlg,
  cfg.color_dlgs1,cfg.color_dlgs2,C_ELEM_BOTON,&nuevo_pro);
npro[0]='\0';
numpro.cadena=npro;
numpro.longitud=3;
c_crea_elemento(&c,5,PRO_ALTO-5,PRO_ANCHO-8,6,3,cfg.color_dlg,
  cfg.color_dlgs2,cfg.color_dlgs1,C_ELEM_INPUT,&numpro);

/* crea lista de procesos */
if((fbd=fopen(Nf_Bdtemp,"rt"))==NULL) {
	imprime_error(Merr_Aper);
	return(-1);
}

while(1) {
	if(fgets(l,E_MAXLNGLIN,fbd)==NULL) {
		if(feof(fbd)) break;
		else {
			imprime_error(Merr_Lect);
			fclose(fbd);
			return(-1);
		}
	}

	/* si encuentra proceso comprueba si existe fichero temporal */
	/* asociado; si es as¡ lee l¡nea de fichero temporal, si no */
	/* lee l¡nea de fichero de base de datos */
	if(busca_marca(Mpro,l)) {
		sprintf(nfsecc,Nf_Secc,&Mpro[1]);
		i=pon_extensionsecc(l,nfsecc);
		if(pro_existe[i]==TRUE) {
			if((fsecc=fopen(nfsecc,"rt"))==NULL) {
				imprime_error(Merr_Aper);
				fclose(fbd);
				return(-1);
			}
			fgets(l,E_MAXLNGLIN,fsecc);
			fclose(fsecc);
		}
		else fgets(l,E_MAXLNGLIN,fbd);

		/* almacena entrada en lista */
		lng=strlen(l);
		if(l[lng-1]=='\n') l[lng-1]='\0';
		sprintf(buff,"%3i: %-75s",i,l);
		c_mete_en_lista(&pro,buff);
	}
}

fclose(fbd);

c_abre(&c);

while(1) {
	i=c_gestiona(&c);

	if((i==-1) || (i==3)) break;
	else if(i==2) {
		i=atoi(pro.selecc);
		if(!i) imprime_error(Merr_Pro0borr);
		else if(pregunta_siono(" ¨Quieres borrar proceso? ")) {
			borra_seccmult(i,Mpro,Merr_Pronoex,pro_existe);
			break;
		}
	}
	else if(i==4) {
		i=atoi(npro);

		/* comprueba si n£mero dentro de rango permitido */
		if((i<0) || (i>=MAX_PRO)) {
			imprime_error(Merr_Npro);
			continue;
		}

		/* comprueba si existe fichero asociado a proceso */
		if(pro_existe[i]==TRUE) {
			imprime_error(Merr_Proex);
			continue;
		}

		/* comprueba si existe proceso en fichero de base de datos */
		if((fbd=fopen(Nf_Bdtemp,"rt"))==NULL) {
			imprime_error(Merr_Aper);
			c_cierra(&c);
			return(-1);
		}
		sprintf(buff,"%s %i",Mpro,i);
		encontrado=0;
		while(1) {
			if(fgets(l,E_MAXLNGLIN,fbd)==NULL) {
				if(feof(fbd)) break;
				else {
					imprime_error(Merr_Lect);
					fclose(fbd);
					c_cierra(&c);
					return(-1);
				}
			}
			if(busca_marca(buff,l)) {
				imprime_error(Merr_Proex);
				encontrado=1;
				break;
			}
		}
		fclose(fbd);

		if(encontrado) continue;
		else if(crea_seccmult(i,Mpro,Merr_Proex,pro_existe)) {
			c_cierra(&c);
			return(i);
		}
		break;
	}
	else {
		if(*pro.selecc) {
			c_cierra(&c);
			return(atoi(pro.selecc));
		}
		break;
	}
}

c_cierra(&c);

return(-1);
}

/****************************************************************************
	ELIGE_MSG: presenta una lista de tablas de mensajes de la base de
	  datos y permite elegir una; se usa la base de datos temporal
	  'Nf_Bdtemp'.
	  Salida:       n£mero de tabla seleccionada o -1 si no
			seleccion¢ ninguna o error
****************************************************************************/
int elige_msg(void)
{
STC_CUADRO c;
STC_ELEM_LISTA tmsg={
	" ^Tablas de mensajes ", C_LSTNORMAL, C_LSTSINORDEN
};
STC_ELEM_BOTON vale={
	" ^Vale"
};
STC_ELEM_BOTON borrar={
	" ^Borrar"
};
STC_ELEM_BOTON salir={
	" ^Salir"
};
STC_ELEM_BOTON nueva_tmsg={
	" ^Nueva tabla"
};
STC_ELEM_INPUT numtmsg={
	"T^abla"
};
FILE *fbd, *fsecc;
char l[E_MAXLNGLIN], nfsecc[13], buff[81], nmsg[4];
int i, lng, encontrado;

c_crea_cuadro(&c,NULL,C_CENT,C_CENT,MSG_ANCHO,MSG_ALTO,cfg.color_dlg,
  cfg.color_dlgs1,cfg.color_dlgs2,cfg.color_dlgtec,cfg.color_dlgsel);
c_crea_elemento(&c,0,0,0,MSG_ANCHO-2,MSG_ALTO-5,cfg.color_dlg,cfg.color_dlgs2,
  cfg.color_dlgs1,C_ELEM_LISTA,&tmsg);
c_crea_elemento(&c,1,MSG_ALTO-5,0,8,3,cfg.color_dlg,cfg.color_dlgs1,
  cfg.color_dlgs2,C_ELEM_BOTON,&vale);
c_crea_elemento(&c,2,MSG_ALTO-5,8,10,3,cfg.color_dlg,
  cfg.color_dlgs1,cfg.color_dlgs2,C_ELEM_BOTON,&borrar);
c_crea_elemento(&c,3,MSG_ALTO-5,18,9,3,cfg.color_dlg,cfg.color_dlgs1,
  cfg.color_dlgs2,C_ELEM_BOTON,&salir);
c_crea_elemento(&c,4,MSG_ALTO-5,MSG_ANCHO-31,15,3,cfg.color_dlg,
  cfg.color_dlgs1,cfg.color_dlgs2,C_ELEM_BOTON,&nueva_tmsg);
nmsg[0]='\0';
numtmsg.cadena=nmsg;
numtmsg.longitud=3;
c_crea_elemento(&c,5,MSG_ALTO-5,MSG_ANCHO-8,6,3,cfg.color_dlg,
  cfg.color_dlgs2,cfg.color_dlgs1,C_ELEM_INPUT,&numtmsg);

/* crea lista de tablas de mensajes */
if((fbd=fopen(Nf_Bdtemp,"rt"))==NULL) {
	imprime_error(Merr_Aper);
	return(-1);
}

while(1) {
	if(fgets(l,E_MAXLNGLIN,fbd)==NULL) {
		if(feof(fbd)) break;
		else {
			imprime_error(Merr_Lect);
			fclose(fbd);
			return(-1);
		}
	}

	/* si encuentra tabla de mensajes comprueba si existe fichero */
	/* temporal asociado; si es as¡ lee l¡nea de fichero temporal */
	/* si no lee l¡nea de fichero de base de datos */
	if(busca_marca(Mmsg,l)) {
		sprintf(nfsecc,Nf_Secc,&Mmsg[1]);
		i=pon_extensionsecc(l,nfsecc);
		if(msg_existe[i]==TRUE) {
			if((fsecc=fopen(nfsecc,"rt"))==NULL) {
				imprime_error(Merr_Aper);
				fclose(fbd);
				return(-1);
			}
			fgets(l,E_MAXLNGLIN,fsecc);
			fclose(fsecc);
		}
		else fgets(l,E_MAXLNGLIN,fbd);

		/* almacena entrada en lista */
		lng=strlen(l);
		if(l[lng-1]=='\n') l[lng-1]='\0';
		sprintf(buff,"%3i: %-75s",i,l);
		c_mete_en_lista(&tmsg,buff);
	}
}

fclose(fbd);

c_abre(&c);

while(1) {
	i=c_gestiona(&c);

	if((i==-1) || (i==3)) break;
	else if(i==2) {
		i=atoi(tmsg.selecc);
		if(!i) imprime_error(Merr_Msg0borr);
		else if(pregunta_siono(" ¨Quieres borrar tabla "
		  "de mensajes? ")) {
			borra_seccmult(i,Mmsg,Merr_Msgnoex,msg_existe);
			break;
		}
	}
	else if(i==4) {
		i=atoi(nmsg);

		/* comprueba si n£mero dentro de rango permitido */
		if((i<0) || (i>=MAX_TMSG)) {
			imprime_error(Merr_Ntmsg);
			continue;
		}

		/* comprueba si existe fichero asociado a tabla */
		if(msg_existe[i]==TRUE) {
			imprime_error(Merr_Msgex);
			continue;
		}

		/* comprueba si existe tabla en fichero de base de datos */
		if((fbd=fopen(Nf_Bdtemp,"rt"))==NULL) {
			imprime_error(Merr_Aper);
			c_cierra(&c);
			return(-1);
		}
		sprintf(buff,"%s %i",Mmsg,i);
		encontrado=0;
		while(1) {
			if(fgets(l,E_MAXLNGLIN,fbd)==NULL) {
				if(feof(fbd)) break;
				else {
					imprime_error(Merr_Lect);
					fclose(fbd);
					c_cierra(&c);
					return(-1);
				}
			}
			if(busca_marca(buff,l)) {
				imprime_error(Merr_Msgex);
				encontrado=1;
				break;
			}
		}
		fclose(fbd);

		if(encontrado) continue;
		else if(crea_seccmult(i,Mmsg,Merr_Msgex,msg_existe)) {
			c_cierra(&c);
			return(i);
		}
		break;
	}
	else {
		if(*tmsg.selecc) {
			c_cierra(&c);
			return(atoi(tmsg.selecc));
		}
		break;
	}
}

c_cierra(&c);

return(-1);
}

/****************************************************************************
	EJECUTA: ejecuta un programa externo.
	  Entrada:      'ruta' ruta d¢nde se encuentra en programa
			'prg' nombre del programa
			'arg' puntero a matriz de punteros a los argumentos,
			el primer elemento de la matriz se dejar  vac¡o y el
			puntero siguiente al del £ltimo argumento debe ser
			NULL
	  Salida:       1 si se pudo ejecutar, 0 si no
****************************************************************************/
int ejecuta(char *ruta, char *prg, char **arg)
{
char programa[MAXPATH];
int codigo;

r_puntero(R_OCULTA);

/* reutiliza memoria */
e_vacia_bloque();
e_borra_texto();

/* crea nombre completo con ruta */
strcpy(programa,ruta);
strcat(programa,prg);

/* pone puntero a nombre completo de programa en primer elemento de matriz */
arg[0]=programa;

codigo=spawnv(P_WAIT,programa,arg);

asm {
	mov ax,0003h	// definir modo v¡deo (texto 80x25 color)
	int 10h
}
m_cierra(m);
m_abre(m);
e_dibuja_editor();

if(codigo==-1) return(0);

return(1);
}

/****************************************************************************
	HAZ_FICH_SAL: crea nombre de fichero de salida del compilador a
	  partir del de entrada.
	  Entrada:      'fich' nombre de fichero de entrada
			'fsal' puntero a buffer d¢nde se dejar  nombre de
			fichero de salida
****************************************************************************/
void haz_fich_sal(char *fich, char *fsal)
{
char drive_n[MAXDRIVE], dir_n[MAXDIR], fname_n[MAXFILE], ext_n[MAXEXT];

fnsplit(fich,drive_n,dir_n,fname_n,ext_n);

/* el nombre del fichero de salida ser  como el de entrada pero */
/* con extensi¢n .DAT */
strcpy(fsal,drive_n);
strcat(fsal,dir_n);
strcat(fsal,fname_n);
strcat(fsal,".DAT");

}

/****************************************************************************
	COMPILA_BD: compila la base de datos actual.
	  Salida:       1 si se compil¢, 0 si error
****************************************************************************/
int compila_bd(void)
{
char fichero_csal[MAXPATH], *argum[5], fichero_ed[MAXPATH];
int error=0;

if(!*base_datos) return(1);

/* borra fichero de errores */
remove(Nf_Err);

/* ruta completa de fichero de salida */
haz_fich_sal(base_datos,fichero_csal);

/* actualiza fichero temporal de base de datos */
if(!actualiza_bdtemp()) return(0);

argum[1]=Nf_Bdtemp;
argum[2]=fichero_csal;
argum[3]=Arg_Cs;
argum[4]=NULL;

e_nombre_fichero(fichero_ed);

if(!ejecuta(cfg.dir_sintac,"CS",argum)) {
	imprime_error(Merr_Nocomp);
	error=1;
}

e_carga_texto(fichero_ed);
e_dibuja_editor();

if(error) return(0);

/* procesa mensajes error del compilador */
if(errores_compil()) return(0);

return(1);
}

/****************************************************************************
	HAZ_FICH_EXE: crea nombre de fichero ejecutable del linkador a
	  partir del de entrada.
	  Entrada:      'fich' nombre de fichero de entrada
			'fexe' puntero a buffer d¢nde se dejar  nombre de
			fichero ejecutable
****************************************************************************/
void haz_fich_exe(char *fich, char *fexe)
{
char drive_n[MAXDRIVE], dir_n[MAXDIR], fname_n[MAXFILE], ext_n[MAXEXT];

fnsplit(fich,drive_n,dir_n,fname_n,ext_n);

/* el nombre del fichero ejecutable ser  como el de entrada pero */
/* con extensi¢n .EXE */
strcpy(fexe,drive_n);
strcat(fexe,dir_n);
strcat(fexe,fname_n);
strcat(fexe,".EXE");

}

/****************************************************************************
	LINKA_BD: crea un ejecutable con la base de datos actual.
	  Salida:       1 si se compil¢, 0 si error
****************************************************************************/
int linka_bd(void)
{
char fichero_dat[MAXPATH], fichero_exe[MAXPATH], *argum[5],
  fichero_ed[MAXPATH];
int error=0;

if(!*base_datos) return(1);

/* primero compila la base de datos */
if(!compila_bd()) return(0);

/* ruta completa de fichero compilado */
haz_fich_sal(base_datos,fichero_dat);

/* ruta completa de fichero de ejecutable */
haz_fich_exe(base_datos,fichero_exe);

argum[1]=fichero_dat;
argum[2]=fichero_exe;
argum[3]=Arg_Cs;
argum[4]=NULL;

e_nombre_fichero(fichero_ed);

if(!ejecuta(cfg.dir_sintac,"LKS",argum)) {
	imprime_error(Merr_Nolink);
	error=1;
}

e_carga_texto(fichero_ed);
e_dibuja_editor();

if(error) return(0);

return(1);
}

/****************************************************************************
	EJECUTA_BD: ejecuta la base de datos compilada, si el fichero de
	  base de datos ha sido modificado o si no se compil¢ lo compila.
	  Salida:       1 si se pudo ejecutar, 0 si error
****************************************************************************/
int ejecuta_bd(void)
{
STC_CUADRO c;
STC_ELEM_TEXTO txt={
	"         Base de datos ha sido modificada\n"
	"               ¨Quieres compilarla?", C_TXTLINEA, C_TXTNOBORDE
};
STC_ELEM_BOTON si={
	" ^Si"
};
STC_ELEM_BOTON no={
	" ^No"
};
STC_ELEM_BOTON salir={
	" Sa^lir"
};
struct stat stfbd, stfdat;
char fichero_dat[MAXPATH], drive_n[MAXDRIVE], dir_n[MAXDIR],
  fname_n[MAXFILE], ext_n[MAXEXT], ant_dir[MAXDIR], *argum[5],
  fichero_ed[MAXPATH];
int ant_drive, i, error=0;

if(!*base_datos) return(1);

c_crea_cuadro(&c,NULL,C_CENT,C_CENT,AV_ANCHO,AV_ALTO,cfg.color_dlg,
  cfg.color_dlgs1,cfg.color_dlgs2,cfg.color_dlgtec,cfg.color_dlgsel);
c_crea_elemento(&c,0,AV_ALTO-5,16,6,3,cfg.color_dlg,cfg.color_dlgs1,
  cfg.color_dlgs2,C_ELEM_BOTON,&si);
c_crea_elemento(&c,1,AV_ALTO-5,22,6,3,cfg.color_dlg,cfg.color_dlgs1,
  cfg.color_dlgs2,C_ELEM_BOTON,&no);
c_crea_elemento(&c,2,AV_ALTO-5,28,9,3,cfg.color_dlg,cfg.color_dlgs1,
  cfg.color_dlgs2,C_ELEM_BOTON,&salir);
c_crea_elemento(&c,3,0,0,AV_ANCHO-2,4,cfg.color_dlg,cfg.color_dlgs1,
  cfg.color_dlgs2,C_ELEM_TEXTO,&txt);

/* ruta completa de fichero de base de datos compilada */
haz_fich_sal(base_datos,fichero_dat);

/* si no existe base de datos compilada, la compila */
if(access(fichero_dat,0)){
	if(!compila_bd()) return(0);
}
else {
	stat(base_datos,&stfbd);
	stat(fichero_dat,&stfdat);

	/* si fecha de fichero de base de datos es posterior a */
	/* fecha de fichero de base de datos compilada, o base de datos */
	/* ha sido modificada, recompila */
	if((stfbd.st_atime>stfdat.st_atime) || (bd_modificada==TRUE)) {
		c_abre(&c);
		i=c_gestiona(&c);
		c_cierra(&c);

		if((i==-1) || (i==2)) return(1);
		else if(i==0) {
			if(!compila_bd()) return(0);
		}
	}
}

/* cambia a unidad y directorio de datos */
fnsplit(base_datos,drive_n,dir_n,fname_n,ext_n);
ant_drive=getdisk();
getcwd(ant_dir,MAXDIR);
setdisk((int)(toupper(*drive_n)-'A'+1));
/* anula '\' final */
i=strlen(dir_n)-1;
if(dir_n[i]=='\\') dir_n[i]='\0';
chdir(dir_n);

argum[1]=fichero_dat;
argum[2]=Arg_Ds1;
argum[3]=Arg_Ds2;
argum[4]=NULL;

e_nombre_fichero(fichero_ed);

if(!ejecuta(cfg.dir_sintac,"DS",argum)) {
	imprime_error(Merr_Nods);
	error=1;
}

/* restaura unidad y directorio */
setdisk(ant_drive);
chdir(ant_dir);

e_carga_texto(fichero_ed);
e_dibuja_editor();

if(error) return(0);

return(1);
}

/****************************************************************************
	BUSCA_LINEA_SECC: busca a que secci¢n pertenece una l¡nea, y devuelve
	  la marca identificadora de esa secci¢n.
	  Entrada:      'nlin' n£mero de l¡nea (distinto de 0)
			'secc' puntero a cadena donde se colocar  marca de
			secci¢n (debe tener capacidad para al menos 9
			caracteres)
			'lin_secc' puntero a variable donde se guardar  el
			n£mero de l¡nea relativa al inicio de la secci¢n
	  Salida:       1 si se encontr¢, 0 si error
****************************************************************************/
int busca_linea_secc(unsigned long nlin, char *secc, int *lin_secc)
{
FILE *fbd;
char c[E_MAXLNGLIN], marca[9], *l;
unsigned long lin_marca=0, lin_act=0;
int i;

/* si el n£mero de l¡nea es 0, sale con error */
if(!nlin) return(0);

if((fbd=fopen(Nf_Bdtemp,"rt"))==NULL) {
	imprime_error(Merr_Aper);
	return(0);
}

/* inicializa cadenas de marca de secci¢n */
*secc='\0';
marca[0]='\0';

while(1) {
	if(fgets(c,E_MAXLNGLIN,fbd)==NULL) {
		if(feof(fbd)) break;
		else {
			imprime_error(Merr_Lect);
			fclose(fbd);
			return(0);
		}
	}

	/* incrementa n£mero de l¡nea del fichero de base de datos */
	lin_act++;

	/* si encuentra una marca de secci¢n, mira si el n£mero de l¡nea es */
	/* mayor que la que buscamos, si es as¡ devuelve la marca anterior */
	/* y n£mero de l¡nea relativo; si no almacena la marca y su l¡nea */
	if(es_marca_secc(c)==TRUE) {
		if(lin_act>nlin) {
			strcpy(secc,marca);
			*lin_secc=(int)(nlin-lin_marca);
			fclose(fbd);
			return(1);
		}
		else {
			lin_marca=lin_act;

			/* recoge la marca de secci¢n */
			for(l=c; *l!=MARCA_S; l++);
			for(i=0; i<4; i++, l++) marca[i]=(char)toupper(*l);
			marca[i]='\0';

			/* si es marca de secci¢n m£ltiple tambi‚n recoge */
			/* su n£mero */
			if(!strcmp(marca,Mpro) || !strcmp(marca,Mmsg)) {
				/* salta espacios */
				for(; ((*l==' ') || (*l=='\t')); l++);

				/* inserta espacio */
				marca[i++]=' ';

				/* recoge n£mero, m ximo 3 d¡gitos */
				for(; i<8; i++, l++) {
					if(strchr(CNum,*l)==NULL) break;
					marca[i]=*l;
				}
				marca[i]='\0';
			}
		}
	}
}

imprime_error(Merr_Lin);
fclose(fbd);

return(0);
}

/****************************************************************************
	ERRORES_COMPIL: muestra los errores del compilador, si se produjo
	  alguno.
	  Salida:       1 si se complet¢ operaci¢n, 0 si error o si no
			hay errores
****************************************************************************/
int errores_compil(void)
{
STC_CUADRO c;
STC_ELEM_LISTA lsterr={
	" ^Errores ", C_LSTNORMAL, C_LSTSINORDEN
};
STC_ELEM_BOTON vale={
	" ^Vale"
};
STC_ELEM_BOTON salir={
	" ^Salir"
};
int i;
FILE *ferr;
char lerr[81], *l, nl[41], seccion[9];
unsigned long nlin;
int lin_seccion;

/* si no hay fichero de errores, sale */
if(access(Nf_Err,0)!=0) return(0);

c_crea_cuadro(&c,NULL,C_CENT,C_CENT,CERR_ANCHO,CERR_ALTO,cfg.color_err,
  cfg.color_errs1,cfg.color_errs2,cfg.color_errtec,cfg.color_errsel);
c_crea_elemento(&c,0,0,0,CERR_ANCHO-2,CERR_ALTO-5,cfg.color_err,
  cfg.color_errs2,cfg.color_errs1,C_ELEM_LISTA,&lsterr);
c_crea_elemento(&c,1,CERR_ALTO-5,0,8,3,cfg.color_err,cfg.color_errs1,
  cfg.color_errs2,C_ELEM_BOTON,&vale);
c_crea_elemento(&c,2,CERR_ALTO-5,8,9,3,cfg.color_err,cfg.color_errs1,
  cfg.color_errs2,C_ELEM_BOTON,&salir);

if((ferr=fopen(Nf_Err,"rt"))==NULL) {
	imprime_error(Merr_Aper);
	return(0);
}

/* lee y almacena mensajes de error */
while(1) {
	if(fgets(lerr,81,ferr)==NULL) break;

	/* elimina '\n' final, si lo hay */
	i=strlen(lerr);
	if(lerr[i-1]=='\n') lerr[i-1]='\0';

	if(!c_mete_en_lista(&lsterr,lerr)) break;
}

fclose(ferr);

c_abre(&c);
i=c_gestiona(&c);
c_cierra(&c);

if(i==1) {
	/* separa n£mero de l¡nea */
	for(l=lsterr.selecc, i=0; ((*l!=':') && (i<41)); l++, i++) nl[i]=*l;
	nl[i]='\0';
	nlin=atol(nl);

	if(busca_linea_secc(nlin,seccion,&lin_seccion)) {
		if(nlin==(unsigned long)lin_seccion) edita_seccion(Mconst,
		  lin_seccion);
		else edita_seccion(seccion,lin_seccion);
	}
}

return(1);
}

/****************************************************************************
	AYUDA_SOBRE: busca ayuda sobre un tema.
****************************************************************************/
void ayuda_sobre(void)
{
STC_CUADRO c;
STC_ELEM_INPUT inp={
	"^Tema"
};
STC_ELEM_TEXTO txt={
	" Teclea el tema sobre el que buscar\n"
	" ayuda, p. ej. un condacto.", C_TXTLINEA, C_TXTBORDE
};
STC_ELEM_BOTON vale={
	" ^Vale"
};
STC_ELEM_BOTON salir={
	" ^Salir"
};
char tema[A_LNGREF];
int i;

c_crea_cuadro(&c," Ayuda sobre... ",C_CENT,C_CENT,AYT_ANCHO,AYT_ALTO,
  cfg.color_dlg,cfg.color_dlgs1,cfg.color_dlgs2,cfg.color_dlgtec,
  cfg.color_dlgsel);
c_crea_elemento(&c,0,0,6,AYT_ANCHO-8,3,cfg.color_dlg,cfg.color_dlgs2,
  cfg.color_dlgs1,C_ELEM_INPUT,&inp);
inp.cadena=tema;
inp.longitud=A_LNGREF-1;
c_crea_elemento(&c,1,3,0,AYT_ANCHO-2,4,cfg.color_dlg,cfg.color_dlgs2,
  cfg.color_dlgs1,C_ELEM_TEXTO,&txt);
c_crea_elemento(&c,2,AYT_ALTO-5,0,8,3,cfg.color_dlg,cfg.color_dlgs1,
  cfg.color_dlgs2,C_ELEM_BOTON,&vale);
c_crea_elemento(&c,3,AYT_ALTO-5,8,9,3,cfg.color_dlg,cfg.color_dlgs1,
  cfg.color_dlgs2,C_ELEM_BOTON,&salir);

*tema='\0';
c_abre(&c);
do {
	i=c_gestiona(&c);
	if(i==2) {
		c_cierra(&c);
		ayuda(tema);
		c_abre(&c);
	}
} while((i!=-1) && (i!=3));

c_cierra(&c);

}

/****************************************************************************
	UTILIDADES: men£ de utilidades, se usa un fichero ASCII que debe
	  contener la ruta, texto de men£ y nombre de las utilidades que
	  se quieren incluir (una utilidad en cada l¡nea del fichero, los
	  apartados separados por comas).
****************************************************************************/
void utilidades(void)
{
STC_MENU *mutil;
FILE *futil;
char nf_utl[MAXPATH], lin[81], menu_util[1024], *l, ruta[MAX_NUTILS][129],
  texto_opc[30], nombre_utls[MAX_NUTILS][13], *argum[2],
  fichero_ed[MAXPATH];
int i, num_opc=0;

strcpy(nf_utl,cfg.dir_sintac);
strcat(nf_utl,"EDS.UTL");
if((futil=fopen(nf_utl,"rt"))==NULL) return;

*menu_util='\0';

while(1) {
	if(fgets(lin,81,futil)==NULL) {
		if(feof(futil)) break;
		else {
			fclose(futil);
			return;
		}
	}

	l=lin;

	/* recoge ruta */
	/* NOTA: la ruta debe acabar en barra invertida '\' */
	i=0;
	while(*l && (*l!='\n') && (*l!=',') && (i<128))
	  ruta[num_opc][i++]=*l++;
	ruta[num_opc][i]='\0';
	/* si ruta = '*SINTAC' asigna directorio del SINTAC */
	if(!strcmp(ruta[num_opc],"*SINTAC")) strcpy(ruta[num_opc],
	  cfg.dir_sintac);
	/* si ruta = '*UTIL' asigna directorio de utilidades */
	if(!strcmp(ruta[num_opc],"*UTIL")) strcpy(ruta[num_opc],cfg.dir_util);

	/* recoge texto de la opci¢n */
	if(*l) l++;
	else return;
	i=0;
	while(*l && (*l!='\n') && (*l!=',') && (i<29)) texto_opc[i++]=*l++;
	texto_opc[i]='\0';
	if(num_opc!=0) strcat(menu_util,":");
	strcat(menu_util,texto_opc);

	/* recoge nombre de la utilidad */
	if(*l) l++;
	else return;
	i=0;
	while(*l && (*l!='\n') && (i<12)) nombre_utls[num_opc][i++]=*l++;
	nombre_utls[num_opc][i]='\0';

	num_opc++;
	if(num_opc>=MAX_NUTILS) break;
}

fclose(futil);

if(!num_opc) return;

mutil=m_crea(MENU_VERT,NULL,menu_util,2,43,cfg.color_men,cfg.color_mens1,
  cfg.color_mens2,cfg.color_mentec,cfg.color_mensel,0);

m_abre(mutil);
i=m_elige_opcion(mutil);
m_cierra(mutil);
m_elimina(mutil);

e_nombre_fichero(fichero_ed);

if((i>=0) && (i<num_opc)) {
	argum[1]=NULL;
	if(!ejecuta(ruta[i],nombre_utls[i],argum)) imprime_error(Merr_Noutil);
}

e_carga_texto(fichero_ed);
e_dibuja_editor();

}
