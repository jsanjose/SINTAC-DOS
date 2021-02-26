/****************************************************************************
			       LINKADOR SINTAC
			    (c)1994 JSJ Soft Ltd.
****************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <alloc.h>
#include <string.h>
#include <dir.h>
#include <fcntl.h>
#include <io.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <dos.h>
#include "..\ventanas\ventana.h"
#include "..\ventanas\raton.h"
#include "..\ventanas\cuadro.h"
#include "sintac.h"
#include "version.h"
#include "color.h"
#include "lks.h"

/*** Variables globales ***/
/* tama¤o del STACK */
unsigned _stklen=8192;

/* configuraci¢n */
STC_CFG cfg;

/* indicador de si se ejecuta desde entorno */
BOOLEAN lks_eds=FALSE;

/* nombres de fichero de base de datos y ejecutable */
char nf_bd[MAXPATH];
char nf_exe[MAXPATH];

/* mensajes de error */
char *MsgErr_Rtime=" Error de apertura de m¢dulo 'runtime'";
char *MsgErr_Exe="   Error de apertura de fichero EXE";
char *MsgErr_Bd="  Error de apertura de base de datos";
char *MsgErr_Lect="           Error de lectura";
char *MsgErr_Escr="          Error de escritura";
char *MsgErr_Nobd="  Fichero de base de datos no v lido";

void main(int argc, char *argv[])
{
STC_CUADRO c;
STC_ELEM_INPUT bd={
	"^Fichero de base de datos"
};
STC_ELEM_INPUT exe={
	"Fichero ^EXE"
};
STC_ELEM_BOTON lnk={
	" ^Linkar"
};
STC_ELEM_BOTON salir={
	" ^Salir"
};
int i, modovideo;

/* instala 'handler' de errores cr¡ticos */
harderr(int24_hnd);

/* analiza par metros de entrada */
analiza_args(argc,argv);

if(lks_eds==FALSE) {
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
}

lee_cfg(argv[0]);

c_crea_cuadro(&c," LINKADOR "COPYRIGHT" versi¢n "VERSION_LKS" ",C_CENT,C_CENT,
  64,11,cfg.color_cs,cfg.color_css1,cfg.color_css2,cfg.color_cstec,
  cfg.color_cssel);
bd.cadena=nf_bd;
bd.longitud=MAXPATH-1;
c_crea_elemento(&c,0,0,26,36,3,cfg.color_cs,cfg.color_css2,cfg.color_css1,
  C_ELEM_INPUT,&bd);
exe.cadena=nf_exe;
exe.longitud=MAXPATH-1;
c_crea_elemento(&c,1,3,26,36,3,cfg.color_cs,cfg.color_css2,cfg.color_css1,
  C_ELEM_INPUT,&exe);
if(lks_eds==FALSE) {
	c_crea_elemento(&c,2,6,0,10,3,cfg.color_cs,cfg.color_css1,
	  cfg.color_css2,C_ELEM_BOTON,&lnk);
	c_crea_elemento(&c,3,6,10,9,3,cfg.color_cs,cfg.color_css1,
	  cfg.color_css2,C_ELEM_BOTON,&salir);
}

c_abre(&c);

if(lks_eds==FALSE) {
	i=c_gestiona(&c);
	if(i==2) linkar();
}
else linkar();

c_cierra(&c);

r_puntero(R_OCULTA);

if(lks_eds==FALSE) {
	asm {
		mov ax,0003h	// definir modo v¡deo (texto 80x25 color)
		int 10h
	}
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
	LEE_CFG: lee fichero de configuraci¢n si existe, si no asigna
	  valores por defecto.
	  Entrada:      'argv0' ruta y nombre del programa (normalmente
			contenido en argv[0])
****************************************************************************/
void lee_cfg(char *argv0)
{
char ruta[MAXPATH], drive[MAXDRIVE], dir[MAXDIR], fname[MAXFILE], ext[MAXEXT],
  nf_cfg[MAXPATH];
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
	ANALIZA_ARGS: analiza los argumentos de la l¡nea de llamada al
	  programa.
	  Entrada:      'argc' n£mero de argumentos en la l¡nea de llamada
			'argv' matriz de punteros a los argumentos, el primero
			ser  siempre el nombre del programa
	  Salida:       1 si error, 0 si no
		      variables globales:-
			'nf_bd', 'nf_exe' nombres de ficheros de entrada y
			salida e indicadores corrrespondientes actualizados
****************************************************************************/
int analiza_args(int argc, char *argv[])
{
int ppar=1;
char par[129];

*nf_bd='\0';
*nf_exe='\0';

while(ppar<argc) {
	/* comprueba si empieza por '/' o '-' */
	if((*argv[ppar]=='/') || (*argv[ppar]=='-')) {
		/* copia argumento sin car cter inicial y pasa a may£sculas */
		strcpy(par,argv[ppar]+1);
		strupr(par);

		/* mira si es alguno de los par metros v lidos */
		if(!strcmp(par,"E")) lks_eds=TRUE;
	}
	else {
		if(*nf_bd=='\0') {
			strcpy(nf_bd,argv[ppar]);
			strupr(nf_bd);
		}
		else if(*nf_exe=='\0') {
			strcpy(nf_exe,argv[ppar]);
			strupr(nf_exe);
		}
	}

	ppar++;
}

if(*nf_exe=='\0') construye_nfexe();

return(0);
}

/****************************************************************************
	CONSTRUYE_NFEXE: construye nombre de fichero de salida a partir
	  del de la base de datos.
	  Entrada:    variables globales:-
			'nfbd' nombre de fichero de base de datos
	  Salida:     variables globales:-
			'nfexe' nombre de fichero ejecutable
****************************************************************************/
void construye_nfexe(void)
{
char drive_n[MAXDRIVE], dir_n[MAXDIR], fname_n[MAXFILE], ext_n[MAXEXT];

if(*nf_bd=='\0') {
	*nf_exe='\0';
	return;
}

fnsplit(nf_bd,drive_n,dir_n,fname_n,ext_n);
strcpy(nf_exe,drive_n);
strcat(nf_exe,dir_n);
strcat(nf_exe,fname_n);
strcat(nf_exe,".EXE");
strupr(nf_exe);

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

if(lks_eds==FALSE) {
	asm {
		mov ax,0003h	// definir modo v¡deo (texto 80x25 color)
		int 10h
	}
}

remove(nf_exe);

exit(1);

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
	  Entrada:      'horg' handle de fichero origen
			'hdest' handle de fichero destino
	  Salida:       1 si pudo copiar, 0 si error
****************************************************************************/
void copia_fichero(int horg, int hdest)
{
char *buf, bufaux[256];
int bufaux_usado=0;
long tam=0xff00L, flng;

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
		close(horg);
		close(hdest);
		if(!bufaux_usado) free(buf);
		imprime_error(MsgErr_Lect);
	}
	if(write(hdest,buf,(unsigned)tam)==-1) {
		close(horg);
		close(hdest);
		if(!bufaux_usado) free(buf);
		imprime_error(MsgErr_Escr);
	}
}

if(!bufaux_usado) free(buf);

}

/****************************************************************************
	LINKAR: crea fichero ejecutable a partir de base de datos.
	  Entrada:    variables globales:-
			'nf_bd' nombre de fichero de base de datos
			'nf_exe' nombre de fichero ejecutable
****************************************************************************/
void linkar(void)
{
CAB_SINTAC cab;
char *srecon=SRECON;
char nf_runtime[MAXPATH];
long lng_runtime=0;
int hruntime, hexe, hbd;

esconde_cursor();

/* si no ha dado nombre de base de datos, sale */
if(!*nf_bd) return;

/* si no ha dado nombre de fichero ejecutable lo construye */
if(!*nf_exe) construye_nfexe();

/* nombre de fichero 'runtime' */
strcpy(nf_runtime,cfg.dir_sintac);
strcat(nf_runtime,"SINTAC.RUN");

/* longitud de m¢dulo 'runtime' */
if((hruntime=open(nf_runtime,O_BINARY | O_RDONLY))==-1)
  imprime_error(MsgErr_Rtime);
lng_runtime=filelength(hruntime);

if((hexe=open(nf_exe,O_BINARY | O_WRONLY | O_CREAT | O_TRUNC,
  S_IREAD | S_IWRITE))==-1) imprime_error(MsgErr_Exe);

/* copia m¢dulo 'runtime' en fichero ejecutable */
copia_fichero(hruntime,hexe);
close(hruntime);

/* copia base de datos en fichero ejecutable */
if((hbd=open(nf_bd,O_BINARY | O_RDONLY))==-1) {
	close(hexe);
	imprime_error(MsgErr_Bd);
}

/* lee cabecera */
if(read(hbd,&cab,sizeof(CAB_SINTAC))==-1) {
	close(hbd);
	close(hexe);
	imprime_error(MsgErr_Lect);
}

/* comprueba que la versi¢n de la base de datos sea correcta */
if((cab.srecon[L_RECON-2]!=srecon[L_RECON-2]) ||
  (cab.srecon[L_RECON-1]!=srecon[L_RECON-1])) {
	close(hbd);
	close(hexe);
	imprime_error(MsgErr_Nobd);
}

lseek(hbd,0,SEEK_SET);
copia_fichero(hbd,hexe);
close(hbd);

/* pone longitud de m¢dulo 'runtime' al final del fichero ejecutable */
if(write(hexe,&lng_runtime,sizeof(long))==-1) {
	close(hexe);
	imprime_error(MsgErr_Escr);
}

close(hexe);

}
