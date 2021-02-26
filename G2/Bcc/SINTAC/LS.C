/****************************************************************************
			       LISTADOR SINTAC
			    (c)1994 JSJ Soft Ltd.
****************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <alloc.h>
#include <dir.h>
#include <dos.h>
#include "..\ventanas\ventana.h"
#include "..\ventanas\raton.h"
#include "..\ventanas\menu.h"
#include "..\ventanas\cuadro.h"
#include "..\ventanas\editor.h"
#include "sintac.h"
#include "version.h"
#include "color.h"
#include "ls.h"

/* tabla de nombre-tipo de condactos */
#include "tabcond.h"

/*** Variables globales ***/
/* tama¤o del STACK */
unsigned _stklen=8192;

/* configuraci¢n */
STC_CFG cfg;

/* nombre de fichero de base de datos */
char nombre_fichbd[MAXPATH];

/* cabecera */
CAB_SINTAC cab;

/* variables para Vocabulario */
struct palabra vocabulario[NUM_PAL];    /* para almacenar vocabulario */

/* variables para Mensajes de Sistema */
unsigned tab_desp_msy[MAX_MSY];         /* tabla desplazamientos mens.sist. */
char *tab_msy;                          /* ptro. inicio zona mens. sistema */

/* variables para Mensajes */
unsigned tab_desp_msg[MAX_MSG];         /* tabla desplazamientos mensajes */
char *tab_msg;                          /* puntero a inicio zona mensajes */

/* variables para Localidades */
unsigned tab_desp_loc[MAX_LOC];         /* tabla desplazamientos textos loc. */
char *tab_loc;                          /* puntero inicio textos localidades */
/* variables para Conexiones */
unsigned tab_desp_conx[MAX_LOC];        /* tabla desplazamientos conexiones */
BYTE *tab_conx;                         /* puntero inicio zona conexiones */

/* variables para Objetos */
unsigned tab_desp_obj[MAX_OBJ];         /* tabla desplazamientos de objetos */
char *tab_obj;                          /* puntero a inicio zona de objetos */

/* variables para Procesos */
unsigned tab_desp_pro[MAX_PRO];         /* tabla desplazamientos procesos */
BYTE *tab_pro;                          /* puntero a inicio zona de procesos */

/* nombre de fichero temporal para listados */
char *Nf_Lst="$LST$";

/*** Programa principal ***/
void main(int argc, char *argv[])
{
STC_MENU *m;
char *opciones_m=" ^Cargar : ^Info| ^Voc: Ms^y: ^Msg: ^Loc: ^Obj: ^Pro|"
		 " S^alir";
int i, modovideo;

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

m=m_crea(MENU_VERT | MENU_FIJO,NULL,opciones_m,0,0,cfg.color_men,
  cfg.color_mens1,cfg.color_mens2,cfg.color_mentec,cfg.color_mensel,0);
m_abre(m);
e_inicializa("",0,10,70,25,cfg.color_ved,cfg.color_veds1,cfg.color_veds2,
  cfg.color_vedblq,cfg.color_dlg,cfg.color_dlgs1,cfg.color_dlgs2,
  cfg.color_dlgtec,cfg.color_dlgsel,E_SOLOLECT);
e_dibuja_editor();

acerca_de();

if(argc<2) *nombre_fichbd='\0';
else {
	for(i=0; argv[1][i]; i++) nombre_fichbd[i]=mayuscula(argv[1][i]);
	nombre_fichbd[i]='\0';
}

if(*nombre_fichbd) {
	if(!carga_bd(nombre_fichbd)) *nombre_fichbd='\0';
}

do {
	i=m_elige_opcion(m);
	switch(i) {
		case -1 :
			e_editor();
			break;
		case 0 :
			c_selecc_ficheros(C_CENT,C_CENT,
			  " Cargar base de datos ",cfg.color_dlg,
			  cfg.color_dlgs1,cfg.color_dlgs2,cfg.color_dlgtec,
			  cfg.color_dlgsel,cfg.dir_bd,"*.DAT",nombre_fichbd);
			if(*nombre_fichbd) {
				if(!carga_bd(nombre_fichbd))
				  *nombre_fichbd='\0';
			}
			break;
		case 1 :
			info_bd();
			break;
		case 3 :
			lista_voc();
			break;
		case 4 :
			lista_msy();
			break;
		case 5 :
			lista_msg();
			break;
		case 6 :
			lista_loc();
			break;
		case 7 :
			lista_obj();
			break;
		case 8 :
			lista_pro();
			break;
	}
} while(i!=10);

/* elimina fichero temporal de listados */
remove(Nf_Lst);

e_borra_texto();

m_cierra(m);
m_elimina(m);

free(tab_msy);
free(tab_msg);
free(tab_loc);
free(tab_conx);
free(tab_obj);
free(tab_pro);

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
	ACERCA_DE: informaci¢n sobre el listador.
****************************************************************************/
void acerca_de(void)
{
STC_CUADRO c;
STC_ELEM_TEXTO t={
	"    Listador de bases de datos versi¢n "VERSION_LS
	"\n       "COPYRIGHT, C_TXTPASA, C_TXTNOBORDE
};
STC_ELEM_BOTON b={
	" ^Vale"
};
int i;

c_crea_cuadro(&c,NULL,C_CENT,C_CENT,52,10,cfg.color_dlg,cfg.color_dlgs1,
  cfg.color_dlgs2,cfg.color_dlgtec,cfg.color_dlgsel);
c_crea_elemento(&c,0,5,21,8,3,cfg.color_dlg,cfg.color_dlgs1,cfg.color_dlgs2,
  C_ELEM_BOTON,&b);
c_crea_elemento(&c,1,0,0,50,5,cfg.color_dlg,cfg.color_dlgs1,cfg.color_dlgs2,
  C_ELEM_TEXTO,&t);

c_abre(&c);
do {
	i=c_gestiona(&c);
} while((i!=-1) && (i!=0));
c_cierra(&c);

}

/****************************************************************************
	CODIFICA: codifica/decodifica una tabla de secci¢n.
	  Entrada:      'mem' puntero a la tabla a codificar/decodificar
			'bytes_mem' tama¤o de la tabla
****************************************************************************/
void codifica(BYTE *mem, unsigned bytes_mem)
{
BYTE *p, *ult_p;

p=mem;
ult_p=p+bytes_mem;

for(; p<ult_p; p++) *p=CODIGO(*p);

}

/****************************************************************************
	SACA_PAL: devuelve el n£mero de la primera entrada en el vocabulario
	  que se corresponda con el n£mero y tipo de palabra dado.
	  Entrada:      'num_pal' n£mero de palabra de vocabulario
			'tipo_pal' tipo de palabra a buscar
	  Salida:       posici¢n dentro de la tabla de vocabulario o
			(NUM_PAL+1) si no se encontr¢
****************************************************************************/
int saca_pal(BYTE num_pal, BYTE tipo_pal)
{
int i;

for(i=0; i<cab.pal_voc; i++) {
	if((vocabulario[i].num==num_pal) && (vocabulario[i].tipo==tipo_pal))
	  return(i);
}

return(NUM_PAL+1);
}

/****************************************************************************
	FPRINT_BIN: imprime un BYTE en forma binaria en un fichero.
	  Entrada:      'f' puntero a fichero
			'b' valor a imprimir
	  Salida:       valor impreso en fichero en binario
****************************************************************************/
void fprint_bin(FILE *f, BYTE b)
{
int i;
BYTE mascara=0x80;

for(i=0; i<8; i++) {
	if(b & mascara) fprintf(f,"1");
	else fprintf(f,"0");
	mascara >>= 1;
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
	NPAR_CONDACTO: devuelve el n£mero de par metros de un condacto.
	  Entrada:      'tcond' tipo de condacto
	  Salida:       n£mero de par metros del condacto
****************************************************************************/
int npar_condacto(BYTE tcond)
{

switch(tcond) {
	case 0 :
		return(0);
	case 1 :
	case 3 :
	case 6 :
	case 7 :
	case 8 :
	case 10 :
	case 12 :
	case 13 :
	case 14 :
	case 17 :
	case 19 :
	case 20 :
	case 23 :
	case 26 :
		return(1);
	case 4 :
	case 9 :
	case 11 :
	case 15 :
	case 16 :
	case 18 :
	case 22 :
	case 27 :
	case 28 :
		return(2);
	case 2 :
		return(7);
	case 21 :
		return(3);
	case 29 :
	case 30 :
		return(4);
}

return(0);
}

/****************************************************************************
	CARGA_BD: carga una base de datos.
	  Entrada:      'nombre_fich' nombre de fichero de base de datos
	  Salida:       1 si se pudo cargar, 0 si error
****************************************************************************/
int carga_bd(char *nombre_fich)
{
FILE *fbd;
char *errmem="No hay suficiente memoria.\n";
char *srecon=SRECON;
unsigned bytes_msg;
int i;

if((fbd=fopen(nombre_fich,"rb"))==NULL) return(0);

/* lee cabecera */
fread(&cab,sizeof(CAB_SINTAC),1,fbd);

/* comprueba que la versi¢n de la base de datos sea correcta */
if((cab.srecon[L_RECON-2]!=srecon[L_RECON-2]) ||
  (cab.srecon[L_RECON-1]!=srecon[L_RECON-1])) return(0);

/* Reserva de memoria para las distintas secciones */
/* Mensajes del sistema */
if((tab_msy=(char *)malloc((size_t)cab.bytes_msy))==NULL) {
	printf(errmem);
	return(0);
}

/* Mensajes */
/* reserva memoria para tabla de mensajes m s grande */
bytes_msg=0;
for(i=0; i<MAX_TMSG; i++) {
	if(cab.fpos_msg[i]!=(fpos_t)0) {
		if(cab.bytes_msg[i]>bytes_msg) bytes_msg=cab.bytes_msg[i];
	}
}
if((tab_msg=(char *)malloc((size_t)bytes_msg))==NULL) {
	printf(errmem);
	return(0);
}

/* Localidades */
if((tab_loc=(char *)malloc((size_t)cab.bytes_loc))==NULL) {
	printf(errmem);
	return(0);
}
if((tab_conx=(BYTE *)malloc((size_t)cab.bytes_conx))==NULL) {
	printf(errmem);
	return(0);
}

/* Objetos */
if((tab_obj=(char *)malloc((size_t)cab.bytes_obj))==NULL) {
	printf(errmem);
	return(0);
}

/* Procesos */
if((tab_pro=(BYTE *)malloc((size_t)cab.bytes_pro))==NULL) {
	printf(errmem);
	return(0);
}

fseek(fbd,cab.fpos_voc,SEEK_SET);
fread(vocabulario,sizeof(struct palabra),cab.pal_voc,fbd);

fseek(fbd,cab.fpos_msy,SEEK_SET);
fread(tab_desp_msy,sizeof(unsigned),(size_t)MAX_MSY,fbd);
fread(tab_msy,sizeof(char),cab.bytes_msy,fbd);

fseek(fbd,cab.fpos_loc,SEEK_SET);
fread(tab_desp_loc,sizeof(unsigned),(size_t)MAX_LOC,fbd);
fread(tab_loc,sizeof(char),cab.bytes_loc,fbd);
fread(tab_desp_conx,sizeof(unsigned),(size_t)MAX_LOC,fbd);
fread(tab_conx,sizeof(BYTE),cab.bytes_conx,fbd);

fseek(fbd,cab.fpos_obj,SEEK_SET);
fread(tab_desp_obj,sizeof(unsigned),(size_t)MAX_OBJ,fbd);
fread(tab_obj,sizeof(char),cab.bytes_obj,fbd);

fseek(fbd,cab.fpos_pro,SEEK_SET);
fread(tab_desp_pro,sizeof(unsigned),(size_t)MAX_PRO,fbd);
fread(tab_pro,sizeof(BYTE),cab.bytes_pro,fbd);

fclose(fbd);

/* decodifica las secciones */
codifica((BYTE *)tab_msy,cab.bytes_msy);
codifica((BYTE *)tab_loc,cab.bytes_loc);
codifica(tab_conx,cab.bytes_conx);
codifica((BYTE *)tab_obj,cab.bytes_obj);
codifica(tab_pro,cab.bytes_pro);

return(1);
}

/****************************************************************************
	INFO_BD: imprime informaci¢n sobre la base de datos.
****************************************************************************/
void info_bd(void)
{
STC_CUADRO c;
STC_ELEM_LISTA lst={
	" ^Informaci¢n de la base de datos ", C_LSTPRIMERO, C_LSTSINORDEN
};
STC_ELEM_BOTON vale={
	" ^Vale"
};
char *formato="%-20s : %5i %5u", buff[81], drive[MAXDRIVE], dir[MAXDIR],
  fname[MAXFILE], ext[MAXEXT];
int i;
unsigned long bytes_msg=0, total;

if(!*nombre_fichbd) return;

c_crea_cuadro(&c,NULL,C_CENT,C_CENT,70,23,cfg.color_dlg,cfg.color_dlgs1,
  cfg.color_dlgs2,cfg.color_dlgtec,cfg.color_dlg);
c_crea_elemento(&c,0,0,0,68,18,cfg.color_dlg,cfg.color_dlgs2,cfg.color_dlgs1,
  C_ELEM_LISTA,&lst);
c_crea_elemento(&c,1,18,0,8,3,cfg.color_dlg,cfg.color_dlgs1,cfg.color_dlgs2,
  C_ELEM_BOTON,&vale);
fnsplit(nombre_fichbd,drive,dir,fname,ext);

sprintf(buff,"Base de datos: %s%s",fname,ext);
c_mete_en_lista(&lst,buff);
c_mete_en_lista(&lst," ");
c_mete_en_lista(&lst,"SECCION                  NUM BYTES");
c_mete_en_lista(&lst,"----------------------------------");

sprintf(buff,formato,"Vocabulario",cab.pal_voc,
  (unsigned)(cab.pal_voc*sizeof(struct palabra)));
c_mete_en_lista(&lst,buff);

sprintf(buff,formato,"Mensajes del Sistema",(int)cab.num_msy,cab.bytes_msy);
c_mete_en_lista(&lst,buff);

/* lista informaci¢n de tablas de mensajes */
c_mete_en_lista(&lst,"Mensajes");
for(i=0; i<MAX_TMSG; i++) {
	if(cab.fpos_msg[i]!=(fpos_t)0) {
		bytes_msg+=cab.bytes_msg[i];
		sprintf(buff,"  Tabla %3i          : %5i %5u",i,
		  (int)cab.num_msg[i],cab.bytes_msg[i]);
		c_mete_en_lista(&lst,buff);
	}
}

sprintf(buff,formato,"Localidades",(int)cab.num_loc,cab.bytes_loc);
c_mete_en_lista(&lst,buff);
sprintf(buff,formato,"Conexiones",(int)cab.num_loc,cab.bytes_conx);
c_mete_en_lista(&lst,buff);

sprintf(buff,formato,"Objetos",(int)cab.num_obj,cab.bytes_obj);
c_mete_en_lista(&lst,buff);

sprintf(buff,formato,"Procesos",(int)cab.num_pro,cab.bytes_pro);
c_mete_en_lista(&lst,buff);

total=(unsigned)(cab.pal_voc*sizeof(struct palabra))+cab.bytes_msy+
  bytes_msg+cab.bytes_loc+cab.bytes_conx+cab.bytes_obj+cab.bytes_pro;
sprintf(buff,"%-20s :  %10lu","TOTAL",total);
c_mete_en_lista(&lst,buff);

sprintf(buff,"Palabras de movimiento < %3u",(unsigned)cab.v_mov);
c_mete_en_lista(&lst,buff);
sprintf(buff,"Nombres convertibles   < %3u",(unsigned)cab.n_conv);
c_mete_en_lista(&lst,buff);
sprintf(buff,"Nombres propios        < %3u",(unsigned)cab.n_prop);
c_mete_en_lista(&lst,buff);

c_abre(&c);
do {
	i=c_gestiona(&c);
} while((i!=-1) && (i!=1));
c_cierra(&c);

c_borra_lista(&lst);

}

/****************************************************************************
	LISTA_VOC: lista la secci¢n de vocabulario.
****************************************************************************/
void lista_voc(void)
{
FILE *flst;
int i;

if(!*nombre_fichbd) return;

if((flst=fopen(Nf_Lst,"wt"))==NULL) return;

fprintf(flst,"VOCABULARIO\n");
fprintf(flst,"\n\tPALABRA\tNUMERO\tTIPO");
for(i=0; i<cab.pal_voc; i++) {
	fprintf(flst,"\n%3i:\t%s\t%i\t",i,vocabulario[i].p,vocabulario[i].num);
	switch(vocabulario[i].tipo) {
		case _VERB :
			if(vocabulario[i].num<cab.v_mov) fprintf(flst,
			  "verbo movimiento");
			else fprintf(flst,"verbo");
			break;
		case _NOMB :
			if(vocabulario[i].num<cab.v_mov) fprintf(flst,
			  "movimiento");
			else if(vocabulario[i].num<cab.n_conv) fprintf(flst,
			  "nombre convertible");
			else if(vocabulario[i].num<cab.n_prop) fprintf(flst,
			  "nombre propio");
			else fprintf(flst,"nombre");
			break;
		case _ADJT :
			fprintf(flst,"adjetivo");
			break;
		case _CONJ :
			fprintf(flst,"conjunci¢n");
			break;
	}
}

fclose(flst);

e_carga_texto(Nf_Lst);
e_editor();

}

/****************************************************************************
	LISTA_MSY: lista la secci¢n de mensajes del sistema.
****************************************************************************/
void lista_msy(void)
{
FILE *flst;
char *pc;
int i;

if(!*nombre_fichbd) return;

if((flst=fopen(Nf_Lst,"wt"))==NULL) return;

fprintf(flst,"MENSAJES DEL SISTEMA\n");
for(i=0; i<(int)cab.num_msy; i++) {
	fprintf(flst,"\n%3i: ",i);
	pc=tab_msy+tab_desp_msy[i];
	/* comprobamos antes por si es mensaje nulo */
	while(*pc) fprintf(flst,"%c",*pc++);
}

fclose(flst);

e_carga_texto(Nf_Lst);
e_editor();

}

/****************************************************************************
	LISTA_MSG: lista la secci¢n de mensajes.
****************************************************************************/
void lista_msg(void)
{
FILE *flst, *fbd;
char *pc;
int i, j;

if(!*nombre_fichbd) return;

if((flst=fopen(Nf_Lst,"wt"))==NULL) return;
if((fbd=fopen(nombre_fichbd,"rb"))==NULL) return;

fprintf(flst,"MENSAJES");
for(i=0; i<MAX_TMSG; i++) {
	if(cab.fpos_msg[i]!=(fpos_t)0) {
		fseek(fbd,cab.fpos_msg[i],SEEK_SET);
		fread(tab_desp_msg,sizeof(unsigned),(size_t)MAX_MSG,fbd);
		fread(tab_msg,sizeof(char),cab.bytes_msg[i],fbd);
		codifica((BYTE *)tab_msg,cab.bytes_msg[i]);

		fprintf(flst,"\n\nTabla de mensajes %i\n",i);

		for(j=0; j<(int)cab.num_msg[i]; j++) {
			fprintf(flst,"\n%3i: ",j);
			pc=tab_msg+tab_desp_msg[j];
			/* comprobamos antes por si es mensaje nulo */
			while(*pc) fprintf(flst,"%c",*pc++);
		}
	}
}

fclose(fbd);
fclose(flst);

e_carga_texto(Nf_Lst);
e_editor();

}

/****************************************************************************
	LISTA_LOC: lista la secci¢n de localidades.
****************************************************************************/
void lista_loc(void)
{
FILE *flst;
char *pc;
BYTE *pcp;
int i, j;

if(!*nombre_fichbd) return;

if((flst=fopen(Nf_Lst,"wt"))==NULL) return;

fprintf(flst,"LOCALIDADES\n");
for(i=0; i<(int)cab.num_loc; i++) {
	fprintf(flst,"\n%3i: ",i);
	pc=tab_loc+tab_desp_loc[i];
	/* comprobamos antes por si es mensaje nulo */
	while(*pc) fprintf(flst,"%c",*pc++);
	fprintf(flst,"\nCONEXIONES:\n");
	pcp=tab_conx+tab_desp_conx[i];
	if(*pcp==0) fprintf(flst,"Ninguna.");
	else do {
		j=saca_pal(*pcp,_VERB);
		/* si no es verbo, quiz  sea nombre */
		if(j==(NUM_PAL+1)) j=saca_pal(*pcp,_NOMB);
		fprintf(flst," %s a %3u\n",vocabulario[j].p,*(pcp+1));
		pcp+=2;
	} while(*pcp);
}

fclose(flst);

e_carga_texto(Nf_Lst);
e_editor();

}

/****************************************************************************
	LISTA_OBJ: lista la secci¢n de objetos.
****************************************************************************/
void lista_obj(void)
{
FILE *flst;
char *pc;
int i;

if(!*nombre_fichbd) return;

if((flst=fopen(Nf_Lst,"wt"))==NULL) return;

fprintf(flst,"OBJETOS\n");
for(i=0; i<(int)cab.num_obj; i++) {
	fprintf(flst,"\n%3i: ",i);
	pc=tab_obj+tab_desp_obj[i];
	fprintf(flst,"%s",vocabulario[saca_pal(*pc,_NOMB)].p);
	pc++;
	if(*pc==(char)NO_PAL) fprintf(flst," _     ");
	else fprintf(flst," %s",vocabulario[saca_pal(*pc,_ADJT)].p);
	pc++;
	fprintf(flst," %3u ",(BYTE)*pc);
	pc++;
	fprint_bin(flst,*pc);
	fprintf(flst," ");
	pc++;
	fprint_bin(flst,*pc);
	fprint_bin(flst,*(pc+1));
	fprintf(flst," ");
	pc+=2;
	/* comprobamos antes por si es mensaje nulo */
	while(*pc) fprintf(flst,"%c",*pc++);
}

fclose(flst);

e_carga_texto(Nf_Lst);
e_editor();

}

/****************************************************************************
	LISTA_PRO: lista la secci¢n de procesos.
****************************************************************************/
void lista_pro(void)
{
FILE *flst;
int i, j, k, pralin, dirrel, npar;
BYTE *pcp, cd, tcond, indir, msc_indir;

if(!*nombre_fichbd) return;

if((flst=fopen(Nf_Lst,"wt"))==NULL) return;

fprintf(flst,"PROCESOS\n");
for(i=0; i<(int)cab.num_pro; i++) {
	fprintf(flst,"\nProceso %i\n",i);
	pcp=tab_pro+tab_desp_pro[i];
	while(*pcp) {
		/* imprime verbo-nombre entrada */
		fprintf(flst,"\n%5u: ",pcp-tab_pro);
		if(*pcp==NO_PAL) fprintf(flst,"_      ");
		else {
			/* si no es verbo, quiz  sea nombre */
			j=saca_pal(*pcp,_VERB);
			if(j==(NUM_PAL+1)) j=saca_pal(*pcp,_NOMB);
			fprintf(flst,"%s ",vocabulario[j].p);
		}
		pcp++;
		if(*pcp==NO_PAL) fprintf(flst,"_      ");
		else fprintf(flst,"%s ",vocabulario[saca_pal(*pcp,_NOMB)].p);
		pcp++;
		/* imprime desplazamiento siguiente entrada */
		fprintf(flst,"(%5u) ",(unsigned)((*(pcp+1) << 8) | *pcp));
		/* salta desplazamiento siguiente entrada */
		pcp+=2;
		/* indica que es 1era l¡nea de entrada */
		pralin=1;
		while(*pcp) {
			if(!pralin) fprintf(flst,"%5u:              "
			  "         ",pcp-tab_pro);

			if(*pcp==INDIR) {
				pcp++;
				indir=*pcp++;
			}
			else indir=0;

			/* coge n£mero de condacto y lo imprime */
			cd=*pcp++;
			fprintf(flst,"%s   ",condacto[cd].cnd);
			tcond=condacto[cd].tipo;
			switch(tcond) {
				case 0 :
					fprintf(flst,"\n");
					break;
				case 11 :
					dirrel=(*(pcp+1) << 8) | *pcp;
					/* ajusta si 1era l¡nea */
					if(pralin) dirrel+=4;
					fprintf(flst,"%5i\n",dirrel);
					pcp+=2;
					break;
				case 13 :
					fprintf(flst,"%s\n",
					  vocabulario[saca_pal(*pcp,_ADJT)].p);
					pcp++;
					break;
				case 14 :
					fprintf(flst,"%s\n",
					  vocabulario[saca_pal(*pcp,_NOMB)].p);
					pcp++;
					break;
				case 16 :
					if(*pcp==NO_PAL)
					  fprintf(flst,"_       ");
					else {
						/* quiz  sea nombre */
						/* convertible */
						j=saca_pal(*pcp,_VERB);
						if(j==(NUM_PAL+1))
						  j=saca_pal(*pcp,_NOMB);
						fprintf(flst,"%s  ",
						  vocabulario[j].p);
					}
					pcp++;
					if(*pcp==NO_PAL) fprintf(flst,
					  "_     \n");
					else
					  fprintf(flst,"%s\n",
					    vocabulario[saca_pal(*pcp,
					    _NOMB)].p);
					pcp++;
					break;
				default :
					msc_indir=0x01;
					npar=npar_condacto(tcond);
					for(k=0; k<npar; k++) {
						if(indir & msc_indir)
						  fprintf(flst,"[%u] ",*pcp);
						else fprintf(flst,"%u ",*pcp);
						pcp++;
						msc_indir <<= 1;
					}
					fprintf(flst,"\n");
					break;
			}
		pralin=0;
		}
	pcp++;          /* salta fin de entrada */
	}
}

fclose(flst);

e_carga_texto(Nf_Lst);
e_editor();

}

