#include <stdio.h>
#include <string.h>
#include <process.h>	/* exit */
#include <stdlib.h>	/* exit */
#include <io.h>		/* access, remove */
#include <errno.h>	/* access, getcwd, remove */
#include <conio.h>	/* getche */
#include <direct.h>     /* getcwd */
#include <stddef.h>     /* ptrdiff_t */
#include "sintac.h"
#include "version.h"
#include "compil.h"
#include "cs.h"

/****************************************/
/******* Compilador S.I.N.T.A.C. ********/
/****************************************/

/****** VARIABLES GLOBALES *****/
/* tabla de nombres de ficheros temporales para cada secci¢n */
/* se reservan _MAX_PATH caracteres para almacenar path completo */
/* adem s hace falta una tabla adicional para indicar qu‚ ficheros */
/* temporales est n abiertos */
char tf_nombre[N_SECCS][_MAX_PATH];
BOOLEAN tf_open[N_SECCS]={FALSE, FALSE, FALSE, FALSE, FALSE, FALSE};

/* mensaje de error */
const char *Mes_Err="\n*** ERROR: %s.\n";

/* tabla para guardar el vocabulario */
struct palabra vocabulario[NUM_PAL];
int pal_voc;     /* n£mero de palabras compiladas */
/* variables relacionadas con el vocabulario */
BYTE v_mov=V_MOV;       /* guarda m ximo n£m. de verbo de movimiento */
BYTE n_conv=N_CONV;     /*   "      "     "   de nombre convertible */
BYTE n_prop=N_PROP;     /*   "      "     "   de nombre propio */

/* tablas para guardar mensajes del sistema */
char tab_msy[TAM_MEM];
ptrdiff_t tab_desp_msy[MAX_MSY];
BYTE num_msy;           /* n£mero de mensajes del sistema compilados */
ptrdiff_t bytes_msy;    /* memoria ocupada */

/* tablas para guardar mensajes */
char tab_msg[TAM_MEM];
ptrdiff_t tab_desp_msg[MAX_MSY];
BYTE num_msg;           /* n£mero de mensajes compilados */
ptrdiff_t bytes_msg;    /* memoria ocupada */

/* tablas para guardar localidades y conexiones */
char tab_loc[TAM_MEM];
ptrdiff_t tab_desp_loc[MAX_LOC];
BYTE num_loc;           /* n£mero de localidades compiladas */
ptrdiff_t bytes_loc;    /* memoria ocupada */
BYTE tab_conx[TAM_MEM/8];
ptrdiff_t tab_desp_conx[MAX_LOC];
ptrdiff_t bytes_conx;

/* tablas para guardar objetos */
char tab_obj[TAM_MEM];
ptrdiff_t tab_desp_obj[MAX_OBJ];
BYTE num_obj;           /* n£mero de objetos compilados */
ptrdiff_t bytes_obj;    /* memoria ocupada */

/* tablas para guardar procesos */
BYTE tab_pro[TAM_MEM];
ptrdiff_t tab_desp_pro[MAX_PRO];
BYTE num_pro;           /* n£mero de procesos compilados */
ptrdiff_t bytes_pro;    /* memoria ocupada */

/****************************************/
/*********  Programa principal  *********/
/****************************************/
void main(int argc, char *argv[])
{
/* tabla de punteros a ficheros temporales para cada secci¢n */
FILE *tf_file[N_SECCS];
/* puntero a fichero de entrada principal y al de salida */
FILE *f_in, *f_out;
char nf_in[129], nf_out[129];	/* nombre de ficheros de entrada y salida */
int i, seccion;
BOOLEAN secc_encontrada[N_SECCS]={FALSE, FALSE, FALSE, FALSE, FALSE, FALSE};
char linea[LONG_LIN];
char *l;	/* puntero auxiliar */
char sec[4];	/* buffer auxiliar */
/* almacena el n£mero de linea actual del fichero de entrada */
unsigned long num_lin;
char lnum[40];
BOOLEAN err_sec=FALSE;
STC_ERR err;
char buff[81];  /* buffer para operaciones varias */
const char *Str_Secc="--- %s ---\n";
BYTE err_conx;
COD_ERR err_const;

printf("\n"COPYRIGHT);
printf("Compilador Versi¢n %s *** M%u\n\n",VERSION_CS,TAM_MEM);

/* si tecle¢ al menos un argumento en la linea de entrada */
/* considera el 1er argumento como nombre de fichero de entrada */
/* despu‚s intenta abrirlo, abortando el proceso si no puede */
if(argc>1) {
	strcpy(nf_in,argv[1]);
	may_str(nf_in);
	if((f_in=fopen(nf_in,"r"))==NULL) imp_error(_E_AFIN,0);
	/* si tecle¢ 2 o m s considera el segundo como nombre fich. salida */
	if(argc>2) strcpy(nf_out,argv[2]);
	else input_nsal(nf_in,nf_out);
}
/* si no tecle¢ argumentos pide nombres de fichero de entrada y salida */
/* adem s intenta abrir el fichero de entrada */
else {
	printf("Nombre del fichero a compilar: ");
	gets(nf_in);
        may_str(nf_in);
	if((f_in=fopen(nf_in,"r"))==NULL) imp_error(_E_AFIN,0);
	input_nsal(nf_in,nf_out);
}

/* comprueba si el fichero de salida ya existe */
/* si existe, da la oportunidad de terminar el proceso */
if(!access(nf_out,0)) {
	printf("Fichero %s ya existe. ¨Continuar? ",nf_out);
	if((mayuscula((char)getche()))=='S') putchar('\n');
	else fin_prg(0);
}

/* inicializa variables */
for(i=0; i<N_SECCS; secc_encontrada[i]=FALSE, i++);
/* aunque se considera que la primera l¡nea es la n£mero 1 */
/* la variable hay que inicializarla a 0 para que al leer la primera */
/* linea se incremente a 1 */
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
		err_const=mete_const(l);
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

	/* mira si esa secci¢n ya fu‚ encontrada */
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
			err_const=mete_const(l);
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
	sprintf(buff,"secci¢n de %s no encontrada.\n",nomb_secc[i]);
	aviso(buff);
	err_sec=TRUE;
}
if(err_sec) imp_error(_E_NOSC,0);

/* mira a ver si est n definidas las constantes V_MOV, N_CONV y N_PROP */
if(coge_const("V_MOV",&v_mov)) aviso("constante V_MOV no definida.\n");
if(coge_const("N_CONV",&n_conv)) aviso("constante N_CONV no definida.\n");
if(coge_const("N_PROP",&n_prop)) aviso("constante N_PROP no definida.\n");

/* coloca puntero al inicio de los ficheros temporales */
for(i=0; i<N_SECCS; i++) rewind(tf_file[i]);
printf("\nCompilando...\n");

/* compila Vocabulario */
printf(Str_Secc,nomb_secc[VOC]);
err=compila_voc(tf_file[VOC],vocabulario,&pal_voc);
if(err.codigo) imp_error(err.codigo,err.linea);
printf("Palabras compiladas: %i\n",pal_voc);
/* elimina fichero temporal de vocabulario */
fclose(tf_file[VOC]);
tf_open[VOC]=FALSE;
remove(tf_nombre[VOC]);

/* compila Mensajes del Sistema */
printf(Str_Secc,nomb_secc[MSY]);
err=compila_men(tf_file[MSY],tab_msy,TAM_MEM,(MAX_MSY-1),tab_desp_msy,
  &num_msy,&bytes_msy);
if(err.codigo) imp_error(err.codigo,err.linea);
printf("Mensajes del sistema compilados: %u, %u bytes.\n",num_msy,bytes_msy);
/* comprueba si el n£mero de mensajes compilados es el m¡nimo requerido */
/* por el sistema, si no da un aviso */
if(num_msy<NUM_MSY)
  aviso("hay menos mensajes del sistema que los requeridos.\n");
/* elimina fichero temporal de mensajes del sistema */
fclose(tf_file[MSY]);
tf_open[MSY]=FALSE;
remove(tf_nombre[MSY]);

/* compila Mensajes */
printf(Str_Secc,nomb_secc[MSG]);
err=compila_men(tf_file[MSG],tab_msg,TAM_MEM,(MAX_MSG-1),tab_desp_msg,
  &num_msg,&bytes_msg);
if(err.codigo) imp_error(err.codigo,err.linea);
printf("Mensajes compilados: %u, %u bytes.\n",num_msg,bytes_msg);
/* elimina fichero temporal de mensajes */
fclose(tf_file[MSG]);
tf_open[MSG]=FALSE;
remove(tf_nombre[MSG]);

/* compila Localidades */
printf(Str_Secc,nomb_secc[LOC]);
err=compila_loc(tf_file[LOC],tab_loc,TAM_MEM,(MAX_LOC-1),tab_desp_loc,
  tab_conx,TAM_MEM/8,tab_desp_conx,vocabulario,pal_voc,v_mov,&num_loc,
  &bytes_loc,&bytes_conx);
if(err.codigo) imp_error(err.codigo,err.linea);
printf("Localidades compiladas: %u, %u bytes.\n",num_loc,bytes_loc);
printf("            Conexiones: %u bytes.\n",bytes_conx);
/* comprueba validez de la tabla de conexiones */
err.codigo=chequea_conx(num_loc,tab_conx,tab_desp_conx,&err_conx);
if(err.codigo) {
	printf(Mes_Err,err.codigo,men_error[err.codigo]);
	printf("Localidad: %u\n",err_conx);
	fin_prg(1);
}
/* elimina fichero temporal de localidades */
fclose(tf_file[LOC]);
tf_open[LOC]=FALSE;
remove(tf_nombre[LOC]);

/* compila Objetos */
printf(Str_Secc,nomb_secc[OBJ]);
err=compila_obj(tf_file[OBJ],tab_obj,TAM_MEM,(MAX_OBJ-1),tab_desp_obj,
  vocabulario,pal_voc,num_loc,&num_obj,&bytes_obj);
if(err.codigo) imp_error(err.codigo,err.linea);
printf("Objetos compilados: %u, %u bytes.\n",num_obj,bytes_obj);
/* elimina fichero temporal de objetos */
fclose(tf_file[OBJ]);
tf_open[OBJ]=FALSE;
remove(tf_nombre[OBJ]);

/* compila Procesos */
printf(Str_Secc,nomb_secc[PRO]);
err=compila_pro(tf_file[PRO],tab_pro,TAM_MEM,MAX_PRO-1,tab_desp_pro,
  vocabulario,pal_voc,num_loc,num_msy,num_msg,num_obj,n_conv,&num_pro,
  &bytes_pro);
if(err.codigo) imp_error(err.codigo,err.linea);
printf("Procesos compilados: %u, %u bytes.\n",num_pro,bytes_pro);
/* elimina fichero temporal de procesos */
fclose(tf_file[PRO]);
tf_open[PRO]=FALSE;
remove(tf_nombre[PRO]);

/* se graba la base de datos compilada */
/* codifica datos */
codifica((BYTE *)tab_msg,bytes_msg);
codifica((BYTE *)tab_msy,bytes_msy);
codifica((BYTE *)tab_loc,bytes_loc);
codifica(tab_conx,bytes_conx);
codifica((BYTE *)tab_obj,bytes_obj);
codifica(tab_pro,bytes_pro);

/* abrir fichero de salida */
if((f_out=fopen(nf_out,"wb"))==NULL) imp_error(_E_AFOU,0);

/* cadena de reconocimiento */
if(fputs(SRECON,f_out)) imp_error(_E_EFOU,0);

/* guarda n£mero m ximo para verbos de movimiento */
if(fwrite(&v_mov,sizeof(BYTE),1,f_out)!=1) imp_error(_E_EFOU,0);
/* guarda n£mero m ximo para nombres convertibles */
if(fwrite(&n_conv,sizeof(BYTE),1,f_out)!=1) imp_error(_E_EFOU,0);
/* guarda n£mero m ximo para nombres propios */
if(fwrite(&n_prop,sizeof(BYTE),1,f_out)!=1) imp_error(_E_EFOU,0);

/* guarda n£mero de palabras del vocabulario */
if(fwrite(&pal_voc,sizeof(int),1,f_out)!=1) imp_error(_E_EFOU,0);
/* guarda n£mero de mensajes */
if(fwrite(&num_msg,sizeof(BYTE),1,f_out)!=1) imp_error(_E_EFOU,0);
/* guarda memoria ocupada por los mensajes */
if(fwrite(&bytes_msg,sizeof(ptrdiff_t),1,f_out)!=1) imp_error(_E_EFOU,0);
/* guarda n£mero de mensajes del sistema */
if(fwrite(&num_msy,sizeof(BYTE),1,f_out)!=1) imp_error(_E_EFOU,0);
/* guarda memoria ocupada por los mensajes del sistema */
if(fwrite(&bytes_msy,sizeof(ptrdiff_t),1,f_out)!=1) imp_error(_E_EFOU,0);
/* guarda n£mero de localidades */
if(fwrite(&num_loc,sizeof(BYTE),1,f_out)!=1) imp_error(_E_EFOU,0);
/* guarda memoria ocupada por las localidades */
if(fwrite(&bytes_loc,sizeof(ptrdiff_t),1,f_out)!=1) imp_error(_E_EFOU,0);
/* guarda memoria ocupada por las conexiones */
if(fwrite(&bytes_conx,sizeof(ptrdiff_t),1,f_out)!=1) imp_error(_E_EFOU,0);
/* guarda n£mero de objetos */
if(fwrite(&num_obj,sizeof(BYTE),1,f_out)!=1) imp_error(_E_EFOU,0);
/* guarda memoria ocupada por los objetos */
if(fwrite(&bytes_obj,sizeof(ptrdiff_t),1,f_out)!=1) imp_error(_E_EFOU,0);
/* guarda n£mero de procesos */
if(fwrite(&num_pro,sizeof(BYTE),1,f_out)!=1) imp_error(_E_EFOU,0);
/* guarda memoria ocupada por los procesos */
if(fwrite(&bytes_pro,sizeof(ptrdiff_t),1,f_out)!=1) imp_error(_E_EFOU,0);

/* guarda el vocabulario */
if(fwrite(vocabulario,sizeof(struct palabra),(size_t)pal_voc,
  f_out)!=(size_t)pal_voc) imp_error(_E_EFOU,0);

/* guarda punteros a mensajes */
if(fwrite(tab_desp_msg,sizeof(ptrdiff_t),(size_t)MAX_MSG,f_out)!=MAX_MSG)
  imp_error(_E_EFOU,0);
/* guarda mensajes */
if(fwrite(tab_msg,sizeof(char),(size_t)bytes_msg,f_out)!=(size_t)bytes_msg)
  imp_error(_E_EFOU,0);

/* guarda punteros a mensajes del sistema */
if(fwrite(tab_desp_msy,sizeof(ptrdiff_t),(size_t)MAX_MSY,f_out)!=MAX_MSY)
  imp_error(_E_EFOU,0);
/* guarda mensajes del sistema */
if(fwrite(tab_msy,sizeof(char),(size_t)bytes_msy,f_out)!=(size_t)bytes_msy)
  imp_error(_E_EFOU,0);

/* guarda punteros a localidades */
if(fwrite(tab_desp_loc,sizeof(ptrdiff_t),(size_t)MAX_LOC,f_out)!=MAX_LOC)
  imp_error(_E_EFOU,0);
/* guarda localidades */
if(fwrite(tab_loc,sizeof(char),(size_t)bytes_loc,f_out)!=(size_t)bytes_loc)
  imp_error(_E_EFOU,0);
/* guarda punteros a conexiones */
if(fwrite(tab_desp_conx,sizeof(ptrdiff_t),(size_t)MAX_LOC,f_out)!=MAX_LOC)
  imp_error(_E_EFOU,0);
/* guarda conexiones */
if(fwrite(tab_conx,sizeof(char),(size_t)bytes_conx,f_out)!=(size_t)bytes_conx)
  imp_error(_E_EFOU,0);

/* guarda punteros a objetos */
if(fwrite(tab_desp_obj,sizeof(ptrdiff_t),(size_t)MAX_OBJ,f_out)!=MAX_OBJ)
  imp_error(_E_EFOU,0);
/* guarda objetos */
if(fwrite(tab_obj,sizeof(char),(size_t)bytes_obj,f_out)!=(size_t)bytes_obj)
  imp_error(_E_EFOU,0);

/* guarda punteros a procesos */
if(fwrite(tab_desp_pro,sizeof(ptrdiff_t),(size_t)MAX_PRO,f_out)!=MAX_PRO)
  imp_error(_E_EFOU,0);
/* guarda procesos */
if(fwrite(tab_pro,sizeof(char),(size_t)bytes_pro,f_out)!=(size_t)bytes_pro)
  imp_error(_E_EFOU,0);

/* cierra el fichero de salida */
fclose(f_out);

fin_prg(0);     /*** FIN DEL PROGRAMA ***/
}

/****************************************/
/*************  Funciones  **************/
/****************************************/

/****************************************************************************
	FIN_PRG: finaliza el programa cerrando ficheros y eliminando
	  los temporales (si se abrieron).
	  Entrada:	'codigo' c¢digo de retorno del proceso
****************************************************************************/
void fin_prg(int codigo)
{
int i;

fcloseall();
/* elimina los ficheros temporales que existan todav¡a tras */
/* acabar la compilaci¢n */
for(i=0; i<N_SECCS; i++) if(tf_open[i]) remove(tf_nombre[i]);
exit(codigo);
}

/****************************************************************************
	IMP_ERROR: imprime un mensaje de error y sale del programa.
	  Los c¢digos de error y sus mensajes asociados se guardan en dos
	  variables globales 'cod_error' y 'men_error' (mirar CS.H).
	  Entrada:	'cod_err' c¢digo del error
	  		'nlin' n£mero de l¡nea del fichero de entrada donde
			se detect¢ el error, si es 0 no se aplica
****************************************************************************/
void imp_error(unsigned cod_err, unsigned long nlin)
{
printf(Mes_Err,cod_err,men_error[cod_err]);
if(nlin) printf("L¡nea: %lu\n",nlin);
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
printf("Fichero de salida [%s]: ",nout);
gets(ns);
if(*ns!='\0') strcpy(nout,ns);
may_str(nout);
}

/****************************************************************************
	LEE_LINEA: lee una linea de un fichero de entrada.
	  Entrada:	'f_in' puntero al fichero de entrada
	  		'buff' buffer donde se guardar  la linea le¡da la
			cual ser  de una longitud igual o mayor a LONG_LIN
			'nlin' puntero a n£mero de linea actual en fichero
			de entrada
	  Salida:	puntero a 'buff'
	  		'nlin' actualizada
****************************************************************************/
char *lee_linea(FILE *f_in, char *buff, unsigned long *nlin)
{
int error;

/* lee una l¡nea del fichero de entrada y comprueba si hubo error */
/* en cuyo caso finaliza el proceso */
if((fgets(buff,LONG_LIN,f_in))==NULL)
  if(ferror(f_in)) imp_error(_E_LFIN,*nlin);
(*nlin)++;      /* incrementa n£mero de l¡nea */
return(buff);
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

aux_lin=lin;	/* apunta al inicio de la linea a comprobar */
aux_lin=salta_espacios(aux_lin);	/* salta blancos iniciales */
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

/*****************************************************************************
	CHEQUEA_CONX: chequea la validez de la tabla de conexiones buscando
	  alguna conexi¢n a una localidad no v lida.
	  Entrada:      'num_loc' n£mero de localidades compiladas
			'conx' puntero a inicio tabla de conexiones
			'desp_conx' puntero a tabla de desplazamientos
			de las conexiones
	  Salida:       _E_NERR si no hay errores
			_E_CLOC si los hubo.
			'err_conx' el n£mero de localidad en la que se
			detect¢ el error.
*****************************************************************************/
COD_ERR chequea_conx(BYTE num_loc, BYTE *conx, ptrdiff_t *desp_conx,
  BYTE *err_conx)
{
BYTE *pc;
int i;

for(i=0; i<(int)num_loc; i++) {
	pc=conx+desp_conx[i];
	while(*pc) {
		/* salta el verbo de movimiento */
		pc++;
		/* si detecta un conexi¢n a una localidad no definida */
		/* almacena su n£mero de localidad y sale indicando error */
		if((*pc++>=num_loc)) {
			*err_conx=(BYTE)i;
			return(_E_CLOC);
		}
	}
}
return(_E_NERR);
}

/*****************************************************************************
	CODIFICA: codifica/decodifica una tabla de secci¢n.
	  Entrada:      'mem' puntero a la tabla a codificar/decodificar
			'bytes_mem' tama¤o de la tabla
*****************************************************************************/
void codifica(BYTE *mem, ptrdiff_t bytes_mem)
{
BYTE *p, *ult_p;

p=mem;
ult_p=p+bytes_mem;

for(; p<ult_p; p++) *p=CODIGO(*p);
}

