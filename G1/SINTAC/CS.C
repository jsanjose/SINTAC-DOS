/****************************************************************************
			      COMPILADOR SINTAC
			    (c)1993 JSJ Soft Ltd.
****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <io.h>
#include <direct.h>
#include <dos.h>
#include "win.h"
#include "sintac.h"
#include "version.h"
#include "cserr.h"
#include "compil.h"
#include "color.h"
#include "cs.h"

/* ventanas */
STC_WINDOW wcab;                /* ventana para cabecera */
STC_WINDOW wcomp;               /* ventana del compilador */
STC_WINDOW werr;                /* ventana de errores */

/* colores */
BYTE color_cab=COLOR_CAB;	/* color de cabecera */
BYTE color_wcomp=COLOR_WCOMP;	/* color de ventana del compilador */
BYTE color_werr=COLOR_WERR;	/* color de ventana de errores */

/* buffer para mandar mensajes a ventana */
char buff[81];

/* indicadores del compilador */
BOOLEAN msg_memoria=FALSE;      /* si debe presentar mensajes memoria */
BOOLEAN entorno=FALSE;          /* si se ejecuta desde el entorno */

char *Nferr=NF_ERR;             /* nombre de fichero para almacenar errores */
FILE *ferr=NULL;                /* puntero a fichero de errores */

/* nombres de ficheros de entrada y salida */
char nf_in[_MAX_PATH], nf_out[_MAX_PATH];

/* tabla de nombres de ficheros temporales para cada secci¢n */
/* se reservan _MAX_PATH caracteres para almacenar path completo */
/* adem s hace falta una tabla adicional para indicar que ficheros */
/* temporales est n abiertos */
char tf_nombre[N_SECCS][_MAX_PATH];
BOOLEAN tf_open[N_SECCS]={FALSE, FALSE, FALSE, FALSE, FALSE, FALSE};

/* mensajes */
char *Msg_Err="*** ERROR: %s";
char *MsgHay_Err="*** ERROR ***";

/* cabecera del fichero compilado */
CAB_SINTAC cab;

/* tabla para guardar el vocabulario */
struct palabra vocabulario[NUM_PAL];

/* tablas para guardar mensajes del sistema */
char *tab_msy;
unsigned tab_desp_msy[MAX_MSY];

/* tablas para guardar mensajes */
char *tab_msg;
unsigned tab_desp_msg[MAX_MSG];

/* tablas para guardar localidades y conexiones */
char *tab_loc;
unsigned tab_desp_loc[MAX_LOC];
BYTE *tab_conx;
unsigned tab_desp_conx[MAX_LOC];

/* tablas para guardar objetos */
char *tab_obj;
unsigned tab_desp_obj[MAX_OBJ];

/* tablas para guardar procesos */
BYTE *tab_pro;
unsigned tab_desp_pro[MAX_PRO];

/*** Programa principal ***/
void main(int argc, char *argv[])
{
/* tabla de punteros a ficheros temporales para cada secci¢n */
FILE *tf_file[N_SECCS];
/* puntero a fichero de entrada principal y al de salida */
FILE *f_in, *f_out;
int i, seccion;
BOOLEAN secc_encontrada[N_SECCS]={FALSE, FALSE, FALSE, FALSE, FALSE, FALSE};
char linea[LONG_LIN];
char *l;
char sec[4];
/* almacena el n£mero de linea actual del fichero de entrada */
unsigned long num_lin;
char lnum[40];
BOOLEAN err_sec=FALSE;
STC_ERR err;
const char *Str_Compil=" %-20s : %5u   %5u bytes\n";
const char *Str_Mem_Secc="Secci¢n de %-20s : %5u\n";
const char *Secc_Conx="Conexiones";
BYTE err_conx;
int num_conx;
COD_ERR err_const;
size_t memoria=(size_t)TAM_MEM;

/* instala 'handler' de errores cr¡ticos */
_harderr(int24_hnd);

/* analiza par metros de entrada */
analiza_args(argc,argv);

/* lee fichero de configuraci¢n */
lee_config();

/* crea ventanas */
if(entorno==FALSE) borra_scr(0x07);
pon_cursor(25,0);

if(entorno==FALSE) w_crea(LIN_WCOMP,0,80,ALT_WCOMP,color_wcomp,BORDE_1,&wcomp);
else w_crea((25-ALT_WCOMP)/2,0,80,ALT_WCOMP,color_wcomp,BORDE_1,&wcomp);
w_crea(0,0,80,1,color_cab,NO_BORDE,&wcab);
w_crea(LIN_WERR,0,80,ALT_WERR,color_werr,BORDE_2,&werr);

w_cls(&wcomp);
if(entorno==FALSE) {
	w_cls(&wcab);
	w_cls(&werr);
	w_imps("          COMPILADOR "COPYRIGHT" versi¢n "VERSION_CS,&wcab);
}

/* inicializa variables de cabecera */
cab.v_mov=V_MOV;       /* m ximo n£m. de verbo de movimiento */
cab.n_conv=N_CONV;     /*   "     "   de nombre convertible */
cab.n_prop=N_PROP;     /*   "     "   de nombre propio */

/* reserva memoria para las distintas secciones */
/* reduce a la mitad el tama¤o del bloque de memoria (inicialmente TAM_MEM) */
/* hasta que encuentra un tama¤o que quepa en la memoria disponible */
/* o hasta que se haya reducido a MIN_TAM_MEM, en cuyo caso da error */

/* inicializa punteros a NULL */
tab_msy=NULL;
tab_msg=NULL;
tab_loc=NULL;
tab_conx=NULL;
tab_obj=NULL;
tab_pro=NULL;

while(1) {
	/* libera los bloques de memoria que pueden haber sido */
	/* reservados en anteriores pasadas */
	free(tab_msy);
	free(tab_msg);
	free(tab_loc);
	free(tab_conx);
	free(tab_obj);
	free(tab_pro);

	/* si se ha alcanzado el m¡nimo tama¤o permitido, error */
	if(memoria<=MIN_TAM_MEM) imp_error(_E_MMEM,0);

	if((tab_msy=(char *)malloc(memoria))==NULL) {
		memoria/=2;
		continue;
	}
	if((tab_msg=(char *)malloc(memoria))==NULL) {
		memoria/=2;
		continue;
	}
	if((tab_loc=(char *)malloc(memoria))==NULL) {
		memoria/=2;
		continue;
	}
	if((tab_conx=(BYTE *)malloc(memoria/8))==NULL) {
		memoria/=2;
		continue;
	}
	if((tab_obj=(char *)malloc(memoria/2))==NULL) {
		memoria/=2;
		continue;
	}
	if((tab_pro=(BYTE *)malloc(memoria))==NULL) {
		memoria/=2;
		continue;
	}

	/* aqu¡ se llegar  si se ha logrado reservar memoria para */
	/* todas las secciones */
	break;
}

/* si no cogi¢ nombre de fichero de entrada, lo pide */
while(*nf_in=='\0') {
	w_imps("\nNombre del fichero a compilar: ",&wcomp);
	w_input(nf_in,128,W_CURSOR,&wcomp);
}

/* si no cogi¢ nombre de fichero de salida, lo pide */
if(*nf_out=='\0') input_nsal(nf_in,nf_out);

/* comprueba si el fichero de salida ya existe */
/* si existe, da la oportunidad de terminar el proceso */
if(!access(nf_out,0)) {
	sprintf(buff,"\nFichero %s ya existe. ¨Continuar?\n",nf_out);
	w_imps(buff,&wcomp);
	if(mayuscula((char)w_lee_tecla())!='S') fin_prg(0);
}

if((f_in=fopen(nf_in,"rt"))==NULL) imp_error(_E_AFIN,0);

/* crea fichero para almacenar errores si se ejecuta desde entorno */
if(entorno==TRUE) if((ferr=fopen(Nferr,"wt"))==NULL) imp_error(_E_FTMP,0);

if((msg_memoria==TRUE) && (entorno==FALSE)) {
	w_imps("MEMORIA DISPONIBLE:\n",&werr);
	sprintf(buff,Str_Mem_Secc,nomb_secc[MSY],memoria);
	w_imps(buff,&werr);
	sprintf(buff,Str_Mem_Secc,nomb_secc[MSG],memoria);
	w_imps(buff,&werr);
	sprintf(buff,Str_Mem_Secc,nomb_secc[LOC],memoria);
	w_imps(buff,&werr);
	sprintf(buff,Str_Mem_Secc,Secc_Conx,memoria/8);
	w_imps(buff,&werr);
	sprintf(buff,Str_Mem_Secc,nomb_secc[OBJ],memoria/2);
	w_imps(buff,&werr);
	sprintf(buff,Str_Mem_Secc,nomb_secc[PRO],memoria);
	w_imps(buff,&werr);
}

/* imprime pantalla de compilaci¢n */
w_cls(&wcomp);
if(entorno==FALSE) {
	w_imps(" Fichero de entrada: ",&wcomp);
	sprintf(buff,"%-56s",nf_in);
	w_imps(buff,&wcomp);
}
w_imps("\n Fichero de salida : ",&wcomp);
sprintf(buff,"%-56s\n\n",nf_out);
w_imps(buff,&wcomp);

for(i=0; i<N_SECCS; i++) {
	sprintf(buff,Str_Compil,nomb_secc[i],0,0);
	w_imps(buff,&wcomp);
	if(i==LOC) {
		sprintf(buff,Str_Compil,Secc_Conx,0,0);
		w_imps(buff,&wcomp);
	}
}

/* inicializa variables */
for(i=0; i<N_SECCS; secc_encontrada[i]=FALSE, i++);
/* aunque se considera que la primera l¡nea es la n£mero 1 */
/* la variable hay que inicializarla a 0 para que al leer la primera */
/* l¡nea se incremente a 1 */
num_lin=0;

/* lee el fichero de entrada separando las distintas secciones en */
/* ficheros temporales */
do {
	/* va leyendo lineas del fichero de entrada */
	/* sigue mientras encuentre comentarios o lineas nulas */
	/* si encuentra una marca de secci¢n comienza proceso de */
	/* separaci¢n y si encuentra una L_NORM saldr  con error */
	do {
		lee_linea(f_in,linea,&num_lin);
		/* si encuentra una linea que no es comentario, nula */
		/* o marca de secci¢n indica error */
                if((i=tipo_lin(linea))==L_NORM) imp_error(_E_MSCC,num_lin);
	} while(((i==L_COMENT) || (i==L_NULA)) && !feof(f_in));

	/* si encontr¢ fin de fichero, sale del bucle de creaci¢n de */
	/* ficheros temporales */
	if(feof(f_in)) break;

	/* comprueba si encontr¢ una definici¢n de constante */
	if(i==L_CONST) {
		l=linea;
		/* salta espacios iniciales y deja puntero */
		/* al inicio del nombre de la constante */
		l=salta_espacios(l);
		l+=2;
		l=salta_espacios(l);

		/* intenta almacenar la constante en la tabla */
		err_const=mete_const(l,num_lin);
		if(err_const) imp_error(err_const,num_lin);

		/* vuelve al inicio del bucle */
		continue;
	}

	/* aqu¡ llegar  si encontr¢ una linea con formato de marca */
	/* de secci¢n, es decir, su primer car cter no blanco ser  '\' */
	l=linea;	/* inicializa puntero auxiliar */
	/* salta los espacios iniciales */
	l=salta_espacios(l);
	l++;		/* salta el car cter '\' */
	/* copia identificador de secci¢n en sec[] */
	/* a¤adiendo un '\0' en el £ltimo car cter */
	/* y convirtiendo en may£sculas */
	for(i=0; i<3; i++, l++) sec[i]=mayuscula(*l);
	sec[i]='\0';

	/* mira si la marca de secci¢n es v lida */
	/* si es v lida i ser  un n£mero entre 0 y N_SECCS-1 */
	/* y si no es v lida ser  N_SECCS */
	for(i=0; i<N_SECCS; i++) {
		if(!strcmp(sec,id_secc[i])) break;
	}
	seccion=i;	/* guarda n£mero de secci¢n encontrada */
        if(seccion==N_SECCS) imp_error(_E_SCCI,num_lin);

	/* mira si esa secci¢n ya fue encontrada */
	/* en caso de que sea una de Procesos no da ese error */
	if((seccion!=PRO) && (secc_encontrada[seccion]))
	  imp_error(_E_SCRP,num_lin);

	/* crea un nombre de fichero temporal para esa secci¢n */
	/* que ser  XXX$SINT.TMP, donde XXX ser  el identificador de */
	/* la secci¢n, y lo abre si no est  abierto ya */
	if(!tf_open[seccion]) {
		strcpy(tf_nombre[seccion],id_secc[seccion]);
		strcat(tf_nombre[seccion],"$SINT.TMP");
		if((tf_file[seccion]=fopen(tf_nombre[seccion],"w+t"))==NULL)
		  imp_error(_E_FTMP,0);
		tf_open[seccion]=TRUE;
	}
	/* activa el indicador de secci¢n encontrada */
	/* en caso de Procesos el indicador secc_encontrada[PRO] */
	/* se usa para que no se abra de nuevo el fichero temporal */
	secc_encontrada[seccion]=TRUE;

	/* si se trata de una secci¢n de procesos, coge su n£mero y */
	/* lo guarda antes del c¢digo de dicha secci¢n con el */
	/* siguiente formato:   'num_lin:\nnn...' d¢nde */
	/* '\' es el indicador de inicio de proceso, 'num_lin' es */
	/* el n£mero de linea dentro del fichero de entrada de la */
	/* marca de proceso y 'nnn...' es la cadena de caracteres */
	/* precede al '\PRO' en el fichero de entrada */
	if(seccion==PRO) {
		l=hasta_espacio(l);
		l=salta_espacios(l);
		ultoa(num_lin,lnum,10);
		strcat(lnum,":\\");
		if(fwrite(lnum,strlen(lnum),1,tf_file[seccion])!=1)
		  imp_error(_E_ETMP,0);
		if(fwrite(l,strlen(l),1,tf_file[seccion])!=1)
		  imp_error(_E_ETMP,0);
	}

	/* lee lineas desde el fichero de entrada y las escribe en el */
	/* fichero temporal si estas no son comentarios ni definiciones */
	/* de constantes (estas tratar  de almacenarlas) */
	/* en cuanto encuentra una l¡nea que es marca de secci¢n, para */
	/* adem s a¤ade el n£mero de l¡nea actual dentro del fichero */
	/* de entrada a cada l¡nea que escribe en el fichero temporal */
	do {
		lee_linea(f_in,linea,&num_lin);
		/* si encuentra fin de fichero da error */
		if(feof(f_in)) imp_error(_E_EOFI,num_lin);
		if((i=tipo_lin(linea))==L_NORM) {
			ultoa(num_lin,lnum,10);
			strcat(lnum,":");
			if(fwrite(lnum,strlen(lnum),1,tf_file[seccion])!=1)
			  imp_error(_E_ETMP,0);
			if(fwrite(linea,strlen(linea),1,tf_file[seccion])!=1)
			  imp_error(_E_ETMP,0);
		}
		/* si es una definici¢n de constante */
		if(i==L_CONST) {
			l=linea;
			/* salta espacios iniciales y deja puntero */
			/* al inicio del nombre de la constante */
			l=salta_espacios(l);
			l+=2;
			l=salta_espacios(l);

			/* intenta almacenar la constante en la tabla */
			err_const=mete_const(l,num_lin);
			if(err_const) imp_error(err_const,num_lin);
		}
	} while(i!=L_MARCA);
	/* mira si encuentra marca de fin de secci¢n */
	l=linea;
	l=salta_espacios(l);
	l++;
	for(i=0; i<3; i++, l++) sec[i]=mayuscula(*l);
	sec[i]='\0';
	if(strcmp(sec,"END")) imp_error(_E_MEND,num_lin);
} while(!feof(f_in));

for(i=0; i<N_SECCS; i++) if(!secc_encontrada[i]) {
	aviso(0,"secci¢n de %s no encontrada",nomb_secc[i]);
	err_sec=TRUE;
}
if(err_sec) imp_error(_E_NOSC,0);

/* mira a ver si est n definidas las constantes V_MOV, N_CONV y N_PROP */
if(coge_const("V_MOV",&cab.v_mov)) aviso(0,"constante V_MOV no definida");
if(coge_const("N_CONV",&cab.n_conv)) aviso(0,"constante N_CONV no definida");
if(coge_const("N_PROP",&cab.n_prop)) aviso(0,"constante N_PROP no definida");

/* coloca puntero al inicio de los ficheros temporales */
for(i=0; i<N_SECCS; i++) rewind(tf_file[i]);

/* compila Vocabulario */
wcomp.cwx=45;
wcomp.cwy=3;
w_impc(MARCA_C,&wcomp);
wcomp.cwx=0;
err=compila_voc(tf_file[VOC],vocabulario,&cab.pal_voc);
if(err.codigo) imp_error(err.codigo,err.linea);
sprintf(buff,Str_Compil,nomb_secc[VOC],cab.pal_voc,
  sizeof(struct palabra)*cab.pal_voc);
w_imps(buff,&wcomp);
/* elimina fichero temporal de vocabulario */
fclose(tf_file[VOC]);
tf_open[VOC]=FALSE;
remove(tf_nombre[VOC]);

/* compila Mensajes del Sistema */
wcomp.cwx=45;
wcomp.cwy=4;
w_impc(MARCA_C,&wcomp);
wcomp.cwx=0;
err=compila_men(tf_file[MSY],tab_msy,memoria,(MAX_MSY-1),tab_desp_msy,
  &cab.num_msy,&cab.bytes_msy);
if(err.codigo) imp_error(err.codigo,err.linea);
sprintf(buff,Str_Compil,nomb_secc[MSY],cab.num_msy,cab.bytes_msy);
w_imps(buff,&wcomp);
/* comprueba si el n£mero de mensajes compilados es el m¡nimo requerido */
/* por el sistema, si no da un aviso */
if(cab.num_msy<NUM_MSY)
  aviso(0,"hay menos mensajes del sistema que los requeridos");
/* elimina fichero temporal de mensajes del sistema */
fclose(tf_file[MSY]);
tf_open[MSY]=FALSE;
remove(tf_nombre[MSY]);

/* compila Mensajes */
wcomp.cwx=45;
wcomp.cwy=5;
w_impc(MARCA_C,&wcomp);
wcomp.cwx=0;
err=compila_men(tf_file[MSG],tab_msg,memoria,(MAX_MSG-1),tab_desp_msg,
  &cab.num_msg,&cab.bytes_msg);
if(err.codigo) imp_error(err.codigo,err.linea);
sprintf(buff,Str_Compil,nomb_secc[MSG],cab.num_msg,cab.bytes_msg);
w_imps(buff,&wcomp);
/* elimina fichero temporal de mensajes */
fclose(tf_file[MSG]);
tf_open[MSG]=FALSE;
remove(tf_nombre[MSG]);

/* compila Localidades */
wcomp.cwx=45;
wcomp.cwy=6;
w_impc(MARCA_C,&wcomp);
wcomp.cwx=0;
err=compila_loc(tf_file[LOC],tab_loc,memoria,(MAX_LOC-1),tab_desp_loc,
  tab_conx,memoria/8,tab_desp_conx,vocabulario,cab.pal_voc,cab.v_mov,
  &cab.num_loc,&cab.bytes_loc,&cab.bytes_conx);
if(err.codigo) imp_error(err.codigo,err.linea);
sprintf(buff,Str_Compil,nomb_secc[LOC],cab.num_loc,cab.bytes_loc);
w_imps(buff,&wcomp);
sprintf(buff,Str_Compil,Secc_Conx,(cab.bytes_conx-cab.num_loc)/2,
  cab.bytes_conx);
w_imps(buff,&wcomp);
/* comprueba validez de la tabla de conexiones */
err.codigo=chequea_conx(cab.num_loc,tab_conx,tab_desp_conx,&err_conx,
  &num_conx);
if(err.codigo) {
	if(entorno==FALSE) {
		sprintf(buff,Msg_Err,men_error[err.codigo]);
		w_imps(buff,&werr);
		sprintf(buff,", localidad %u, conexi¢n %i\n",err_conx,
		  num_conx);
		w_imps(buff,&werr);
	}
	else fprintf(ferr,"0: %s, localidad %u, conexi¢n %i\n",
	  men_error[err.codigo],err_conx,num_conx);

	wcomp.cwx=COL_ERROR;
	wcomp.cwy=FIL_ERROR;
	w_imps(MsgHay_Err,&wcomp);
	fin_prg(1);
}
/* elimina fichero temporal de localidades */
fclose(tf_file[LOC]);
tf_open[LOC]=FALSE;
remove(tf_nombre[LOC]);

/* compila Objetos */
wcomp.cwx=45;
wcomp.cwy=8;
w_impc(MARCA_C,&wcomp);
wcomp.cwx=0;
err=compila_obj(tf_file[OBJ],tab_obj,memoria/2,(MAX_OBJ-1),tab_desp_obj,
  vocabulario,cab.pal_voc,cab.num_loc,&cab.num_obj,&cab.bytes_obj);
if(err.codigo) imp_error(err.codigo,err.linea);
sprintf(buff,Str_Compil,nomb_secc[OBJ],cab.num_obj,cab.bytes_obj);
w_imps(buff,&wcomp);
/* elimina fichero temporal de objetos */
fclose(tf_file[OBJ]);
tf_open[OBJ]=FALSE;
remove(tf_nombre[OBJ]);

/* compila Procesos */
wcomp.cwx=45;
wcomp.cwy=9;
w_impc(MARCA_C,&wcomp);
wcomp.cwx=24;
err=compila_pro(tf_file[PRO],tab_pro,memoria,MAX_PRO-1,tab_desp_pro,
  vocabulario,cab.pal_voc,cab.num_loc,cab.num_msy,cab.num_msg,cab.num_obj,
  cab.n_conv,&cab.num_pro,&cab.bytes_pro);
if(err.codigo) imp_error(err.codigo,err.linea);
sprintf(buff,Str_Compil,nomb_secc[PRO],cab.num_pro,cab.bytes_pro);
wcomp.cwx=0;
wcomp.cwy=9;
w_imps(buff,&wcomp);
/* elimina fichero temporal de procesos */
fclose(tf_file[PRO]);
tf_open[PRO]=FALSE;
remove(tf_nombre[PRO]);

/* se graba la base de datos compilada */
/* codifica datos */
codifica((BYTE *)tab_msy,cab.bytes_msy);
codifica((BYTE *)tab_msg,cab.bytes_msg);
codifica((BYTE *)tab_loc,cab.bytes_loc);
codifica(tab_conx,cab.bytes_conx);
codifica((BYTE *)tab_obj,cab.bytes_obj);
codifica(tab_pro,cab.bytes_pro);

/* abrir fichero de salida */
if((f_out=fopen(nf_out,"wb"))==NULL) imp_error(_E_AFOU,0);

/* cadena de reconocimiento */
strcpy(cab.srecon,SRECON);

if(fwrite(&cab,sizeof(CAB_SINTAC),1,f_out)!=1) imp_error(_E_EFOU,0);

/* guarda el vocabulario */
if(fwrite(vocabulario,sizeof(struct palabra),(size_t)cab.pal_voc,
  f_out)!=(size_t)cab.pal_voc) imp_error(_E_EFOU,0);

/* guarda punteros a mensajes del sistema */
if(fwrite(tab_desp_msy,sizeof(unsigned),(size_t)MAX_MSY,f_out)!=MAX_MSY)
  imp_error(_E_EFOU,0);
/* guarda mensajes del sistema */
if(fwrite(tab_msy,sizeof(char),(size_t)cab.bytes_msy,f_out)!=
  (size_t)cab.bytes_msy) imp_error(_E_EFOU,0);

/* guarda punteros a mensajes */
if(fwrite(tab_desp_msg,sizeof(unsigned),(size_t)MAX_MSG,f_out)!=MAX_MSG)
  imp_error(_E_EFOU,0);
/* guarda mensajes */
if(fwrite(tab_msg,sizeof(char),(size_t)cab.bytes_msg,f_out)!=
  (size_t)cab.bytes_msg) imp_error(_E_EFOU,0);

/* guarda punteros a localidades */
if(fwrite(tab_desp_loc,sizeof(unsigned),(size_t)MAX_LOC,f_out)!=MAX_LOC)
  imp_error(_E_EFOU,0);
/* guarda localidades */
if(fwrite(tab_loc,sizeof(char),(size_t)cab.bytes_loc,f_out)!=
  (size_t)cab.bytes_loc) imp_error(_E_EFOU,0);
/* guarda punteros a conexiones */
if(fwrite(tab_desp_conx,sizeof(unsigned),(size_t)MAX_LOC,f_out)!=MAX_LOC)
  imp_error(_E_EFOU,0);
/* guarda conexiones */
if(fwrite(tab_conx,sizeof(char),(size_t)cab.bytes_conx,f_out)!=
  (size_t)cab.bytes_conx) imp_error(_E_EFOU,0);

/* guarda punteros a objetos */
if(fwrite(tab_desp_obj,sizeof(unsigned),(size_t)MAX_OBJ,f_out)!=MAX_OBJ)
  imp_error(_E_EFOU,0);
/* guarda objetos */
if(fwrite(tab_obj,sizeof(char),(size_t)cab.bytes_obj,f_out)!=
  (size_t)cab.bytes_obj) imp_error(_E_EFOU,0);

/* guarda punteros a procesos */
if(fwrite(tab_desp_pro,sizeof(unsigned),(size_t)MAX_PRO,f_out)!=MAX_PRO)
  imp_error(_E_EFOU,0);
/* guarda procesos */
if(fwrite(tab_pro,sizeof(char),(size_t)cab.bytes_pro,f_out)!=
  (size_t)cab.bytes_pro) imp_error(_E_EFOU,0);

/* cierra el fichero de salida */
fclose(f_out);

fin_prg(0);
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
	FIN_PRG: finaliza el programa cerrando ficheros y eliminando
	  los temporales (si se abrieron).
	  Entrada:      'codigo' c¢digo de retorno del programa
****************************************************************************/
void fin_prg(int codigo)
{
int i;

fcloseall();

/* elimina los ficheros temporales que existan todav¡a tras */
/* acabar la compilaci¢n */
for(i=0; i<N_SECCS; i++) if(tf_open[i]) remove(tf_nombre[i]);

/* libera memoria */
free(tab_msy);
free(tab_msg);
free(tab_loc);
free(tab_conx);
free(tab_obj);
free(tab_pro);

/* si hay fichero de mensajes de error y est  vac¡o, lo borra */
if(!access(Nferr,0)) {
	if((ferr=fopen(Nferr,"rt"))!=NULL) {
		i=fileno(ferr);
		if(filelength(i)==0) remove(Nferr);
	}
}

/* espera a que se pulse una tecla */
wcomp.cwx=62;
wcomp.cwy=(BYTE)(wcomp.lyi-1);
w_imps("Pulsa una tecla",&wcomp);
w_lee_tecla();

borra_scr(0x07);
pon_cursor(0,0);

exit(codigo);
}

/****************************************************************************
	IMP_ERROR: imprime un mensaje de error y sale del programa.
	  Los c¢digos de error y sus mensajes asociados se guardan en dos
	  variables globales 'cod_error' y 'men_error' (mirar CS.H).
	  Entrada:	'cod_err' c¢digo del error
	  		'nlin' n£mero de l¡nea del fichero de entrada donde
			se detect¢ el error, si es 0 no se imprime n£mero de
			l¡nea
****************************************************************************/
void imp_error(unsigned cod_err, unsigned long nlin)
{

if(entorno==FALSE) {
	sprintf(buff,Msg_Err,men_error[cod_err]);
	w_imps(buff,&werr);

	if(nlin) {
		sprintf(buff,", l¡nea %lu",nlin);
		w_imps(buff,&werr);
	}
	w_impc('\n',&werr);
}

/* si hay abierto fichero de errores, manda l¡nea y mensaje a fichero */
if(ferr!=NULL) fprintf(ferr,"%lu: %s\n",nlin,men_error[cod_err]);

wcomp.cwx=COL_ERROR;
wcomp.cwy=FIL_ERROR;
w_imps(MsgHay_Err,&wcomp);

fin_prg(1);

}

/****************************************************************************
	MAY_STR: convierte una cadena en may£sculas.
	  Entrada:	'cadena' con la cadena a convertir
	  Salida:	el n£mero de caracteres en la cadena
****************************************************************************/
int may_str(char *cadena)
{
int i;

for(i=0; cadena[i]; cadena[i]=mayuscula(cadena[i]), i++);

return(i);
}

/****************************************************************************
	ANALIZA_ARGS: analiza los argumentos de la l¡nea de llamada al
	  programa.
	  Entrada:      'argc' n£mero de argumetos en la l¡nea de llamada
			'argv' matriz de punteros a los argumetos, el primero
			ser  siempre el nombre del programa
	  Salida:       1 si error, 0 si no
		      variables globales:-
			'nf_in', 'nf_out' nombres de ficheros de entrada y
			salida e indicadores corrrespondientes actualizados
****************************************************************************/
int analiza_args(int argc, char *argv[])
{
int ppar=1;
char par[129];

*nf_in='\0';
*nf_out='\0';

while(ppar<argc) {
	/* comprueba si empieza por '/' o '-' */
	if((*argv[ppar]=='/') || (*argv[ppar]=='-')) {
		/* copia argumento sin car cter inicial y pasa a may£sculas */
		strcpy(par,argv[ppar]+1);
		may_str(par);

		/* mira si es alguno de los par metros v lidos */
		if(!strcmp(par,"MEM")) msg_memoria=TRUE;
		if(!strcmp(par,"EDS")) entorno=TRUE;
	}
	else {
		/* si no hay nombre de fichero de entrada, toma el */
		/* par metro actual */
		if(*nf_in=='\0') {
			strcpy(nf_in,argv[ppar]);
			may_str(nf_in);
		}
		else if(*nf_out=='\0') {
			strcpy(nf_out,argv[ppar]);
			may_str(nf_out);
		}
	}

	ppar++;
}

return(0);
}

/****************************************************************************
	INPUT_NSAL: recoge el nombre del fichero de salida. Se usa 'nin'
	  para formar el nombre por defecto del fichero de salida.
	  Entrada:	'nin' cadena con el nombre del fich. entrada
	  		'nout' cadena que contendr  nombre del fich. salida
****************************************************************************/
void input_nsal(char *nin, char *nout)
{
char drive_n[_MAX_DRIVE], dir_n[_MAX_DIR], fname_n[_MAX_FNAME],
  ext_n[_MAX_EXT], ns[129];

_splitpath(nin,drive_n,dir_n,fname_n,ext_n);

strcpy(nout,drive_n);		/* construye path */
strcat(nout,dir_n);
strcat(nout,fname_n);		/* construye ruta, sin ext */
strcat(nout,".DAT");		/* ruta con extensi¢n */

/* recoge el nombre del fichero de salida */
/* si se teclea alguno coge ese, si no coge el nombre por defecto */
sprintf(buff,"\nFichero de salida [%s]: ",nout);
w_imps(buff,&wcomp);

w_input(ns,128,W_CURSOR,&wcomp);

if(*ns!='\0') strcpy(nout,ns);
may_str(nout);

}

/****************************************************************************
	LEE_LINEA: lee una linea de un fichero de entrada.
	  Entrada:	'f_in' puntero al fichero de entrada
			'lin' buffer donde se guardar  la linea le¡da la
			cual ser  de una longitud igual o mayor a LONG_LIN
			'nlin' puntero a n£mero de linea actual en fichero
			de entrada
	  Salida:       puntero a 'lin'
			'nlin' actualizada
****************************************************************************/
char *lee_linea(FILE *f_in, char *lin, unsigned long *nlin)
{

/* lee una l¡nea del fichero de entrada y comprueba si hubo error */
/* en cuyo caso finaliza el proceso */
if((fgets(lin,LONG_LIN,f_in))==NULL)
  if(ferror(f_in)) imp_error(_E_LFIN,*nlin);

/* incrementa n£mero de l¡nea */
(*nlin)++;

return(lin);
}

/****************************************************************************
	TIPO_LIN: devuelve el tipo de linea.
	  Entrada:	'lin' puntero a la linea
	  Salida:	tipo de linea (NOTA: blanco=espacio o tabulaci¢n)
			L_COMENT si el primer car cter no blanco es indicador
			de comentario
			L_CONST si los dos primeros car cteres no blancos
			son indicadores de marca de secci¢n
			L_MARCA si el primer car cter no blanco es indicador
			de marca de secci¢n (y no es L_CONST)
			L_NULA si la linea s¢lo contiene blancos
			L_NORM en cualquier otro caso
****************************************************************************/
int tipo_lin(char *lin)
{
char *aux_lin;

/* apunta al inicio de la linea a comprobar y salta blancos iniciales */
aux_lin=lin;
aux_lin=salta_espacios(aux_lin);
switch(*aux_lin) {
	case CHR_COMENT :
		return(L_COMENT);
	case MARCA_S :
		if(*(aux_lin+1)==MARCA_S) return(L_CONST);
		return(L_MARCA);
	case '\n' :
		return(L_NULA);
	case '\0' :
		return(L_NULA);

}

return(L_NORM);
}

/****************************************************************************
	CHEQUEA_CONX: chequea la validez de la tabla de conexiones buscando
	  alguna conexi¢n a una localidad no v lida.
	  Entrada:      'num_loc' n£mero de localidades compiladas
			'conx' puntero a inicio tabla de conexiones
			'desp_conx' puntero a tabla de desplazamientos
			de las conexiones
			'err_conx' puntero a variable d¢nde dejar n£mero de
			localidad
			'num_conx' puntero a variable d¢nde dejar n£mero de
			orden de conexi¢n
	  Salida:       _E_NERR si no hay errores
			_E_CLOC si los hubo
			'err_conx' el n£mero de localidad en la que se
			detect¢ el error, 'num_conx' el n£mero de orden
			de la conexi¢n que produjo error
****************************************************************************/
COD_ERR chequea_conx(BYTE num_loc, BYTE *conx, unsigned *desp_conx,
  BYTE *err_conx, int *num_conx)
{
BYTE *pc;
int i, j;

for(i=0; i<(int)num_loc; i++) {
	pc=conx+desp_conx[i];
	j=0;
	while(*pc) {
		/* salta el verbo de movimiento */
		pc++;
		/* n£mero de orden de conexi¢n */
		j++;
		/* si detecta un conexi¢n a una localidad no definida */
		/* almacena su n£mero de localidad y sale indicando error */
		if((*pc++>=num_loc)) {
			*err_conx=(BYTE)i;
			*num_conx=j;
			return(_E_CLOC);
		}
	}
}

return(_E_NERR);
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
	BORRA_SCR: borra la pantalla.
	  Entrada:      'color' color de relleno
****************************************************************************/
void borra_scr(BYTE color)
{

_asm {
	mov ax,0600h            ; funci¢n desplazar ventana hacia arriba
	mov bh,color
	mov cx,0                ; esquina superior izquierda
	mov dx,184fh            ; esquina inferior derecha
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

/****************************************************************************
	LEE_CONFIG: lee fichero de configuraci¢n (si existe) y asigna
	  colores.
****************************************************************************/
void lee_config(void)
{
STC_CFG cfg;
FILE *fcfg;

if((fcfg=fopen(NF_CFG,"rb"))==NULL) return;
if(fread(&cfg,sizeof(STC_CFG),1,fcfg)!=1) {
	fclose(fcfg);
	return;
}
fclose(fcfg);

/* asigna colores */
color_cab=cfg.color_cab;
color_wcomp=cfg.color_wcomp;
color_werr=cfg.color_werr;

}
