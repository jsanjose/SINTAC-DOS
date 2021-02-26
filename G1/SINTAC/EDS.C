/****************************************************************************
				EDITOR SINTAC
			    (c)1993 JSJ Soft Ltd.
****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <process.h>
#include <bios.h>
#include <dos.h>
#include <io.h>
#include <direct.h>
#include "scr.h"
#include "sintac.h"
#include "version.h"
#include "color.h"
#include "eds.h"

/*** Variables globales ***/
char *(ptxt[LINEAS]);           /* punteros a l¡neas de texto */
int lineas=0;                   /* n£mero total de l¡neas en buffer */
int max_lineas=0;               /* m ximo n£mero de l¡neas del buffer */
int linea_actual=0;             /* l¡nea del cursor en el buffer */
BYTE fil_cursor=LIN_SCR;        /* fila del cursor en pantalla */
BYTE col_cursor=0;              /* columna del cursor en pantalla */
char texto_estado[70];          /* texto a imprimir en l¡nea estado */

/* men£ del editor, m x. NUM_OPC elementos de LNG_OPC caracteres cada uno */
/* para presentar en 4 columnas de 4 elementos */
char menu_eds[NUM_OPC][LNG_OPC+1] = {
	"&Nuevo",
	"&Cargar",
	"&Grabar",
	"C&ompilar",
	"&Ejecutar",
	"&Vocabulario",
	"Mensajes &sistema",
	"&Mensajes",
	"&Localidades",
	"O&bjetos",
	"&Procesos",
	"Cons&tantes",
	"E&ditor caracteres",
	"E&rrores",
	"A&yuda",
	"Sal&ir",
};

/* marca de fin de secci¢n */
char *Mfin_Secc="\\END";
char *Mvoc="\\VOC";
char *Mmsy="\\MSY";
char *Mmsg="\\MSG";
char *Mloc="\\LOC";
char *Mobj="\\OBJ";
char *Mpro="\\PRO";

/* colores */
BYTE color_scr=COLOR_SCR;       /* color general de pantalla */
BYTE color_editor=COLOR_EDIT;   /* color de zona de edici¢n */
BYTE color_regla=COLOR_REGLA;   /* color de la regleta */
BYTE color_cab=COLOR_CAB;       /* color de la cabecera */
BYTE color_linst=COLOR_LINST;   /* color de l¡nea de estado */
BYTE color_menu=COLOR_MENU;     /* color del men£ de opciones */
BYTE color_tecla=COLOR_TECLA;   /* color de tecla de activaci¢n */
BYTE color_opcion=COLOR_OPC;    /* color opci¢n resaltada */
BYTE color_err=COLOR_ERR;       /* color ventana errores */
BYTE color_errsel=COLOR_ERRSEL; /* color error seleccionado */
BYTE color_ay=COLOR_AY;         /* color ventana de ayuda */
BYTE color_aptdo=COLOR_APTDO;   /* color de cabecera de apartado */
BYTE color_ref=COLOR_REF;       /* color de referencias de ayuda */

/* ficheros temporales */
char *Nfbdat=FBD_TEMP;          /* nombre fich. temporal base de datos */
BOOLEAN fbdat=FALSE;            /* indica si se abri¢ alg£n fich. base datos */
char *Nfsecc=FBD_SECCION;       /* nombre fich. temporal de secci¢n editor */
char *Nfaux=FBD_TEMP2;          /* nombre fich. temporal auxiliar */
char *Nferr=NF_ERR;             /* nombre fich. mensajes de error compil. */

/* matriz de punteros a argumentos, usada por las llamadas a ejecuta() */
char *argum[5];

/* argumentos de llamada al compilador */
char *Arg_Cs="-EDS";
/* argumentos de llamada al int‚rprete-debugger */
char *Arg_Ds1="-L00";
char *Arg_Ds2="-E";

/* cadena con n£meros */
char *CNum="0123456789";

/* mensajes */
char *MsgErr_Escr="Error de escritura en fichero.";
char *MsgErr_Lect="Error de lectura de fichero.";
char *MsgCarga_Txt="Cargando texto...";
char *MsgGraba_Txt="Guardando texto...";
char *MsgNo_Hay_Fich="No hay fichero de base de datos abierto.";
char *MsgErr_Lect_Bd="Error en lectura de base de datos.";
char *MsgErr_Aper_Ft="No se pudo abrir fichero temporal.";
char *MsgErr_Escr_Ft="Error de escritura en fichero temporal.";
char *MsgErr_Lect_Ft="Error de lectura de fichero temporal.";
char *MsgNo_Mem="No hay memoria.";
char *MsgEspera="Espera un momento...";
char *MsgGuard="Guardando cambios...";
char *MsgNo_Busca="Cadena no encontrada.";

/* directorios de trabajo */
char dir_sintac[_MAX_PATH];     /* directorio principal de SINTAC */
char dir_gcs[_MAX_PATH];	/* directorio de generador de caracteres */
char dir_bd[_MAX_PATH];		/* directorio de bases de datos */

/* m scara para listas de ficheros */
char *mascf="*.*";

/* puntero para buffer del texto de ayuda */
char *buff_ayuda=NULL;

/* lista de nombres y punteros a los apartados de la ayuda */
char *apartado[MAX_APARTAD], n_apartado[MAX_APARTAD][MAXL_APTDO];

/* n£mero de apartados de la ayuda */
int n_aptdos=0;

/*** Programa principal ***/
void main(void)
{
char nfich[13], fichero_bd[_MAX_PATH], fichero_csal[_MAX_PATH];
int opcion, p, ant_drive;
char pro[9], tecla, ant_dir[_MAX_DIR];
char drive_n[_MAX_DRIVE], dir_n[_MAX_DIR], fname_n[_MAX_FNAME],
  ext_n[_MAX_EXT];

/* instala 'handler' de errores cr¡ticos */
_harderr(int24_hnd);

/* lee fichero de configuraci¢n */
lee_config();

/* carga el fichero de ayuda */
lee_ayuda();

/* borra pantalla */
borra_scr(color_scr);

/* imprime pantalla del editor */
inicializa_pantalla();

/* indica que no hay fichero cargado */
nfich[0]='\0';

do {
	opcion=menu();

	switch(opcion) {
		case 0 :        /* Nuevo */
			/* si hay fichero de errores lo borra */
			if(!access(Nferr,0)) remove(Nferr);

			if(cuadro_fich(FIL_CUADROF,COL_CUADROF,color_menu,
			  color_opcion,dir_bd,mascf," Nuevo fichero ",
			  fichero_bd)==1) {
				  if(nuevo_fichero_bd(fichero_bd)!=1)
				    error("No se pudo crear fichero.",0);
			}
			break;
		case 1 :        /* Cargar */
			/* si hay fichero de errores lo borra */
			if(!access(Nferr,0)) remove(Nferr);

			if(cuadro_fich(FIL_CUADROF,COL_CUADROF,color_menu,
			  color_opcion,dir_bd,mascf," Cargar fichero ",
			  fichero_bd)==1) {
				if(abre_fichero_bd(fichero_bd)!=1)
				  error("No se pudo cargar fichero.",0);
			}
			break;
		case 2 :        /* Grabar */
			/* mira si hay fichero de base de datos abierto */
			if(fbdat==FALSE) {
				error(MsgNo_Hay_Fich,0);
				break;
			}

			if(cuadro_fich(FIL_CUADROF,COL_CUADROF,color_menu,
			  color_opcion,dir_bd,mascf," Grabar fichero ",
			  fichero_bd)==1) {
				if(cierra_fichero_bd(fichero_bd)==0)
				  error("No se pudo grabar fichero.",0);
			}
			break;
		case 3 :        /* Compilar */
			/* mira si hay fichero de base de datos abierto */
			if(fbdat==FALSE) {
				error(MsgNo_Hay_Fich,0);
				break;
			}

			/* si hay fichero de errores lo borra */
			if(!access(Nferr,0)) remove(Nferr);

			/* ruta completa de fichero de salida */
			haz_fich_sal(fichero_bd,fichero_csal);

			argum[1]=Nfbdat;
			argum[2]=fichero_csal;
			argum[3]=Arg_Cs;
			argum[4]=NULL;
			if(ejecuta(dir_sintac,"CS",argum)!=1)
			  error("No se pudo ejecutar CS.",0);

			/* procesa mensajes error del compilador */
			errores_compilador();

			break;
		case 4 :        /* Ejecutar */
			/* mira si hay fichero de base de datos abierto */
			if(fbdat==FALSE) {
				error(MsgNo_Hay_Fich,0);
				break;
			}

			/* ruta completa de fichero de salida */
			haz_fich_sal(fichero_bd,fichero_csal);

			/* cambia a unidad y directorio de datos */
			_splitpath(fichero_bd,drive_n,dir_n,fname_n,ext_n);
			ant_drive=_getdrive();
			getcwd(ant_dir,_MAX_DIR-1);
			_chdrive((int)(mayuscula(*drive_n)-'A'+1));
			/* anula '\' final */
			p=strlen(dir_n)-1;
			if(dir_n[p]=='\\') dir_n[p]='\0';
			chdir(dir_n);

			argum[1]=fichero_csal;
			argum[2]=Arg_Ds1;
			argum[3]=Arg_Ds2;
			argum[4]=NULL;
			if(ejecuta(dir_sintac,"DS",argum)!=1)
			  error("No se pudo ejecutar DS.",0);

			/* restaura unidad y directorio */
			_chdrive(ant_drive);
			chdir(ant_dir);

			break;
		case 5 :        /* Vocabulario */
			strcpy(texto_estado," VOCABULARIO");
			edita_secc(Mvoc,0);
			break;
		case 6 :        /* Mensajes sistema */
			strcpy(texto_estado," MENSAJES DEL SISTEMA");
			edita_secc(Mmsy,0);
			break;
		case 7 :        /* Mensajes */
			strcpy(texto_estado," MENSAJES");
			edita_secc(Mmsg,0);
			break;
		case 8 :        /* Localidades */
			strcpy(texto_estado," LOCALIDADES");
			edita_secc(Mloc,0);
			break;
		case 9 :        /* Objetos */
			strcpy(texto_estado," OBJETOS");
			edita_secc(Mobj,0);
			break;
		case 10 :       /* Procesos */
			/* si no hay base de datos cargada, sale */
			if(fbdat==FALSE) {
				error(MsgNo_Hay_Fich,0);
				break;
			}
			p=pide_numero("Proceso: ");
			if(crea_proceso(p)==0) break;
			sprintf(pro,"%s %i",Mpro,p);
			sprintf(texto_estado," PROCESO %i",p);
			edita_secc(pro,0);
			break;
		case 11 :       /* Constantes */
			strcpy(texto_estado," CONSTANTES");
			edita_constantes(0);
			break;
		case 12 :       /* Editor caracteres */
			argum[1]=NULL;
			if(ejecuta(dir_gcs,"GCS",argum)!=1)
			  error("No se pudo ejecutar GCS.",0);
			break;
		case 13 :       /* Errores */
			errores_compilador();
			break;
		case 14 :
			ayuda();
			break;
		case 15 :       /* Salir */
			/* si fichero de base de datos abierto, pregunta */
			/* para grabarlo */
			if(fbdat==TRUE) {
				do {
					imp_linea_scr(LIN_SCR+NUM_LIN_SCR,1,
					  color_linst,"¨Guardar fichero?",68);
					pon_cursor(25,0);
					tecla=(char)(_bios_keybrd(_KEYBRD_READ)
					  & 0x00ff);

					pon_cursor(0,0);
					/* borra mensaje */
					linea_estado();

					if((tecla!='s') && (tecla!='S')) break;

					if(cuadro_fich(FIL_CUADROF,COL_CUADROF,
					  color_menu,color_opcion,dir_bd,mascf,
					  " Grabar fichero ",fichero_bd)==1) {
						p=cierra_fichero_bd(fichero_bd);
						if(p==0) error("No se pudo "
						  "grabar fichero.",0);
					}
				} while(p!=1);
			}
			break;
	}

	texto_estado[0]='\0';
	linea_estado();

} while(opcion!=15);

/* si hay fichero de base de datos abierto borra fichero temporal */
if(fbdat==TRUE) remove(Nfbdat);

/* si hay ficheros temporales los borra */
if(!access(Nfsecc,0)) remove(Nfsecc);
if(!access(Nfaux,0)) remove(Nfaux);

/* si hay fichero de errores lo borra */
if(!access(Nferr,0)) remove(Nferr);

/* libera memoria */
libera_texto();

/* libera buffer de texto de ayuda */
free(buff_ayuda);

borra_scr(0x07);

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
	ERROR: rutina de control de errores.
	  Entrada:      'merr' mensaje de error
			'cod' c¢digo de error; si es 0 presenta el mensaje y
			retorna, si no presenta el mensaje y sale al sistema
			operativo devolviendo ese c¢digo
****************************************************************************/
void error(char *merr, int cod)
{

/* si c¢digo no es 0, presenta mensaje y sale al sistema operativo */
/* si no presenta mensaje y retorna */
if(cod) {
	borra_scr(0x07);
	printf("\n%s\n",merr);
	exit(cod);
}
else {
	beep();
	imp_linea_scr(LIN_SCR+NUM_LIN_SCR,1,color_linst,merr,68);

	/* esconde cursor */
	pon_cursor(25,0);

	/* espera hasta que se pulse una tecla */
	_bios_keybrd(_KEYBRD_READ);

	/* borra mensaje */
	linea_estado();

	pon_cursor(0,0);
}

}

/****************************************************************************
	EDITOR: bucle principal del editor.
	  Entrada:      'fich' nombre de fichero a editar
			'linea_inic' n£mero de l¡nea donde se colocar  el
			cursor inicialmente
			'modificado' puntero a variable que contendr  TRUE
			si el texto ha sido modificado
	  Salida:       1 si se pudo editar, 0 si no
			'modificado' conteniendo TRUE si el texto fu‚
			modificado o FALSE si no
****************************************************************************/
int editor(char *fich, int linea_inic, BOOLEAN *modificado)
{
unsigned tecla;
BOOLEAN fin_edicion=FALSE;
int i;
char car, busca[CAR_LIN+1];
int num_tab, lact;
BYTE fcur, ccur;

/* indica texto no modificado *(
*modificado=FALSE;

/* inicializa buffers */
max_lineas=inicializa_texto();

/* si no hay memoria, sale */
if(max_lineas==0) {
	error(MsgNo_Mem,0);
	return(0);
}

/* borra texto actual del editor */
borra_todo_texto();

/* carga fichero en el editor */
lineas=carga_fichero(fich);

/* si no se pudo cargar, sale con error */
if(lineas==0) {
	error(MsgErr_Lect,0);
	return(0);
}

/* imprime cabecera */
cabecera();

/* inicializa cadena de b£squeda */
busca[0]='\0';

/* coloca cursor en l¡nea dada */
if(linea_inic<=lineas) linea_actual=linea_inic;

while(fin_edicion==FALSE) {
	/* imprime texto */
	imp_texto_scr();

	/* si cursor sobrepasa £ltimo car cter de l¡nea actual */
	if(col_cursor>(BYTE)strlen(ptxt[linea_actual]))
	  col_cursor=(BYTE)strlen(ptxt[linea_actual]);

	/* imprime l¡nea de estado */
	linea_estado();

	/* imprime la regleta */
	regleta();

	/* coloca el cursor en su posici¢n de pantalla */
	pon_cursor(fil_cursor,col_cursor);

	/* espera hasta que se pulse una tecla */
	tecla=_bios_keybrd(_KEYBRD_READ);

	/* comprueba teclas especiales */
	switch(tecla >> 8) {
		case TCUR_IZQ :
			if(col_cursor>0) col_cursor--;
			break;
		case TCUR_DER :
			if(col_cursor<79) col_cursor++;
			break;
		case TCUR_ABJ :
			if(mueve_cursor_abajo(1)==1) scroll_arr_scr();
			break;
		case TCUR_ARR :
			if(mueve_cursor_arriba(1)==1) scroll_abj_scr();
			break;
		case TPAG_ABJ :
			if(linea_actual<(lineas-1)) {
				if(lineas<NUM_LIN_SCR)
				  mueve_cursor_abajo(lineas-linea_actual-1);
				else mueve_cursor_abajo(NUM_LIN_SCR-1);
			}
			break;
		case TPAG_ARR :
			if(linea_actual>0)
			  mueve_cursor_arriba(NUM_LIN_SCR-1);
			break;
		case TFIN :
			col_cursor=79;
			break;
		case TORG :
			col_cursor=0;
			break;
		case TCTR_FIN :
			if(linea_actual!=(lineas-1)) {
				linea_actual=lineas-1;
				if(lineas>NUM_LIN_SCR)
				  fil_cursor=LIN_SCR+NUM_LIN_SCR-1;
				else fil_cursor=(BYTE)(LIN_SCR+linea_actual);
				col_cursor=0;
			}
			break;
		case TCTR_ORG :
			if(linea_actual!=0) {
				linea_actual=0;
				fil_cursor=LIN_SCR;
				col_cursor=0;
			}
			break;
		case TDEL :
			*modificado=TRUE;
			/* si pudo borrar car cter retrasa el cursor */
			if((borra_caracter(1)==1) && (col_cursor>0))
			  col_cursor--;
			/* si no, pasa l¡nea actual a anterior */
			else {
				i=strlen(ptxt[linea_actual-1]);
				if(pasa_linea_a(linea_actual)==1) {
					mueve_cursor_arriba(1);
					col_cursor=(BYTE)i;
				}
			}

			/* anula el c¢digo ASCII */
			tecla &= 0xff00;
			break;
		case TSUP :
			*modificado=TRUE;
			i=strlen(ptxt[linea_actual]);

			/* si est  sobre el £ltimo car cter de la l¡nea */
			/* y la l¡nea no est  en blanco */
			if(i && (col_cursor==(BYTE)i)) {
				pasa_linea_a(linea_actual+1);
				break;
			}

			/* borra el car cter sobre el cursor y si la */
			/* l¡nea est  en blanco la borra, si no es */
			/* la £ltima */
			if((borra_caracter(0)==0) &&
			  (linea_actual!=(lineas-1))) {
				borra_linea(linea_actual);
			}
			break;
		case TESC :
			fin_edicion=TRUE;
			/* anula el c¢digo ASCII */
			tecla &= 0xff00;
			break;
		case TF1 :              /* AYUDA */
			ayuda();
			break;
		case TF2 :              /* BUSCAR */
			/* pide cadena a buscar */
			linea_estado();

			imp_linea_scr(LIN_SCR+NUM_LIN_SCR,1,color_linst,
			  "Buscar:",68);
			i=lee_input(LIN_SCR+NUM_LIN_SCR,9,busca,CAR_LIN,60,
			  color_linst);

			linea_estado();

			/* si puls¢ ESCAPE, sale */
			if(i==-2) break;

			if(busca_texto(busca,linea_actual)==0)
			  error(MsgNo_Busca,0);
			break;
		case TF3 :              /* CONTINUAR BUSQUEDA */
			/* guarda l¡nea actual y posici¢n del cursor */
			lact=linea_actual;
			fcur=fil_cursor;
			ccur=col_cursor;

			/* incrementa posici¢n de cursor por si est  */
			/* justo encima de cadena a buscar */
			col_cursor++;
			if(col_cursor>=CAR_LIN) {
				col_cursor=0;
				linea_actual++;
				if(linea_actual>=lineas) linea_actual=0;
			}

			if(busca_texto(busca,linea_actual)==0) {
				linea_actual=lact;
				fil_cursor=fcur;
				col_cursor=ccur;
				error(MsgNo_Busca,0);
			}
			break;
		case TF4 :              /* BORRAR LINEA */
			*modificado=TRUE;
			/* si es la £ltima l¡nea */
			if(linea_actual==(lineas-1)) *ptxt[linea_actual]='\0';
			else borra_linea(linea_actual);
			break;
	}

	/* mira si tecle¢ un car cter v lido */
	switch(tecla & 0x00ff) {
		case 0 :
			break;
		case RETURN :
			*modificado=TRUE;
			/* inserta una l¡nea debajo, si puede */
			if(inserta_linea(linea_actual+1)==0) break;
			/* copia texto desde cursor hasta final de l¡nea */
			/* a la nueva l¡nea */
			for(i=col_cursor;
			  i<(int)(strlen(ptxt[linea_actual])+1); i++) {
				*(ptxt[linea_actual+1]+(i-col_cursor))=
				  *(ptxt[linea_actual]+(i));
			}
			/* trunca l¡nea actual */
			*(ptxt[linea_actual]+col_cursor)='\0';

			/* mueve cursor a l¡nea siguiente */
			if(mueve_cursor_abajo(1)==1) scroll_arr_scr();
			col_cursor=0;

			break;
		case TAB :
			*modificado=TRUE;
			/* calcula n§ caracteres para siguiente tabulaci¢n */
			num_tab=8-(col_cursor % 8);
			/* si queda espacio en la l¡nea, inserta espacios */
			/* hasta tabulaci¢n */
			if(num_tab<(CAR_LIN-strlen(ptxt[linea_actual]))) {
				for(i=0; i<num_tab; i++) {
					inserta_caracter(' ');
					col_cursor++;
				}
			}
			else beep();
			break;
		default :
			*modificado=TRUE;
			/* coge car cter tecleado */
			car=(char)(tecla & 0x00ff);

			/* inserta el car cter, si cursor no est  en */
			/* la £ltima columna */
			if(col_cursor<79) {
				if(inserta_caracter(car)==1) col_cursor++;
			}
			/* si el cursor est  al final de la l¡nea */
			else  {
				/* si el £ltimo car cter es '\0' */
				/* inserta el nuevo */
				if(*(ptxt[linea_actual]+79)=='\0') {
					*(ptxt[linea_actual]+79)=car;
					*(ptxt[linea_actual]+80)='\0';
				}
				else if(inserta_linea(linea_actual+1)==1) {
					mueve_cursor_abajo(1);
					col_cursor=0;
					inserta_caracter(car);
					col_cursor++;
				}
			}
			break;
	}
}

/* guarda cambios en fichero si texto result¢ modificado */
if(*modificado==TRUE) {
	if(graba_fichero(fich)==0) {
		error(MsgErr_Escr,0);
		return(0);
	}
}

/* libera memoria ocupada por el texto del editor */
libera_texto();
_heapmin();
max_lineas=0;

return(1);
}

/****************************************************************************
	INICIALIZA_PANTALLA: imprime la pantalla del editor.
****************************************************************************/
void inicializa_pantalla(void)
{
short l;

borra_scr(color_scr);
cabecera();
regleta();
for(l=LIN_SCR; l<(LIN_SCR+NUM_LIN_SCR); l++) borra_linea_scr(l,color_editor);
texto_estado[0]='\0';
linea_estado();

}

/****************************************************************************
	INICIALIZA_TEXTO: asigna memoria para las l¡neas de texto.
	  Salida:       n£mero de l¡neas de texto reservadas
****************************************************************************/
int inicializa_texto(void)
{
int i;

/* inicializa punteros */
for(i=0; i<LINEAS; i++) ptxt[i]=NULL;

for(i=0; i<LINEAS; i++) {
	if((ptxt[i]=(char *)malloc(CAR_LIN+1))==NULL) break;
}

return(i);
}

/****************************************************************************
	LIBERA_TEXTO: libera la memoria ocupada por el texto.
****************************************************************************/
void libera_texto(void)
{
int i;

for(i=0; i<max_lineas; i++) {
	/* libera l¡neas */
	free(ptxt[i]);
	ptxt[i]=NULL;
}

}

/****************************************************************************
	BORRA_TODO_TEXTO: borra todo el texto contenido en el buffer
	  del editor, e inicializa posici¢n del cursor.
****************************************************************************/
void borra_todo_texto(void)
{
int i;

for(i=0; i<max_lineas; i++) *ptxt[i]='\0';

lineas=0;
linea_actual=0;
fil_cursor=LIN_SCR;
col_cursor=0;

}

/****************************************************************************
	CARGA_FICHERO: lee un fichero de texto y lo almacena en el buffer
	  del editor.
	  Entrada:      'nfich' nombre del fichero a leer
	  Salida:       n£mero de l¡neas del fichero, 0 si hubo error
****************************************************************************/
int carga_fichero(char *nfich)
{
FILE *f_texto;
char temp[CAR_LIN+2], *car;
int num_lin=0;

/* intenta abrir el fichero */
if((f_texto=fopen(nfich,"rt"))==NULL) return(0);

/* presenta mensaje */
imp_linea_scr(LIN_SCR+NUM_LIN_SCR,1,color_linst,MsgCarga_Txt,68);
pon_cursor(25,0);

/* lee l¡neas del fichero */
while(num_lin<max_lineas) {
	if(fgets(temp,CAR_LIN+2,f_texto)==NULL) {
		if(feof(f_texto)) break;
		else return(0);
	}

	/* elimina el '\n' si es que lo hay y lo sustituye por '\0' */
	for(car=temp; *car; car++) {
		if(*car=='\n') *car='\0';
	}

	/* expande tabulaciones */
	expande_tabs(temp);

	/* copia cadena en posici¢n correspondiente de buffer */
	strcpy(ptxt[num_lin],temp);

	num_lin++;
}

pon_cursor(0,0);
/* borra mensaje */
linea_estado();

/* cierra el fichero */
fclose(f_texto);

/* si el fichero estaba vac¡o totalmente, pone una l¡nea al menos en */
/* el buffer de texto */
if(num_lin==0) {
	*ptxt[0]='\0';
	num_lin=1;
}

return(num_lin);
}

/****************************************************************************
	AJUSTA_LINEA: ajusta l¡nea del editor antes de grabarla, eliminando
	  espacios finales y a¤adiendo un '\n'.
	  Entrada:	'lin' puntero a l¡nea
****************************************************************************/
void ajusta_linea(char *lin)
{
char *c;

/* puntero a £ltimo car cter */
c=lin+strlen(lin)-1;

/* elimina los espacios finales */
for(; ((*c==' ') || (*c=='\t')); c--);

/* pone '\n' y '\0' */
*(c+1)='\n';
*(c+2)='\0';

}

/****************************************************************************
	GRABA_FICHERO: graba el texto del buffer del editor en un fichero.
	  Entrada:      'nfich' nombre del fichero a grabar
	  Salida:       n£mero de l¡neas escritas en el fichero, 0 si hubo
			error
****************************************************************************/
int graba_fichero(char *nfich)
{
FILE *f_texto;
char temp[CAR_LIN+2];
int num_lin=0, i;

/* intenta abrir el fichero */
if((f_texto=fopen(nfich,"wt"))==NULL) return(0);

/* presenta mensaje */
imp_linea_scr(LIN_SCR+NUM_LIN_SCR,1,color_linst,MsgGraba_Txt,68);
pon_cursor(25,0);

/* escribe las l¡neas en el fichero */
for(i=0; i<lineas; i++) {
	/* copia l¡nea a buffer temporal */
	strcpy(temp,ptxt[i]);

	/* elimina espacios finales de l¡nea y a¤ade '\n' */
	ajusta_linea(temp);

	if(fputs(temp,f_texto)!=0) return(0);

	num_lin++;
}

pon_cursor(0,0);
/* borra mensaje */
linea_estado();

/* cierra el fichero */
fclose(f_texto);

return(num_lin);
}

/****************************************************************************
	EXPANDE_TABS: expande las tabulaciones de una l¡nea de texto.
	  Entrada:      'lin' puntero a la l¡nea con el texto
****************************************************************************/
void expande_tabs(char *lin)
{
int i, j;
char temp[CAR_LIN+1], *p, *pt;

/* puntero a la l¡nea */
p=lin;
/* puntero a buffer temporal */
pt=temp;

for(i=0; ((i<CAR_LIN) && *p); i++, p++) {
	/* si es tabulaci¢n, la expande */
	if(*p==TAB) {
		do {
			*pt++=' ';
			i++;
		} while(((i % 8)!=0) && (i<CAR_LIN));
		i--;
	}
	else *pt++=*p;
}

/* coloca '\0' final */
*pt='\0';

/* devuelve cadena expandida */
strcpy(lin,temp);

}

/****************************************************************************
	IMP_TEXTO_SCR: imprime el texto en pantalla de acuerdo a la
	  posici¢n del cursor.
****************************************************************************/
void imp_texto_scr(void)
{
int i, j;

/* coge la l¡nea que se imprimir  en la parte superior */
i=linea_actual-(fil_cursor-LIN_SCR);

/* imprime l¡neas */
for(j=0; j<NUM_LIN_SCR; j++) {
	imp_linea_scr(j+LIN_SCR,0,color_editor,ptxt[i],CAR_LIN);
	/* siguiente l¡nea del buffer */
	i++;
}

}

/****************************************************************************
	MUEVE_CURSOR_ABAJO: desplaza el cursor de edici¢n hacia abajo.
	  Entrada:      'nlin' n£mero de l¡neas a desplazar hacia abajo
			'ccur' 1 si quiere desplazar cursor de pantalla al
			mismo tiempo, 0 si no
	  Salida:       1 si se pas¢ de la £ltima l¡nea, 0 si no
****************************************************************************/
int mueve_cursor_abajo(int nlin)
{

/* comprueba si est  en la £ltima l¡nea del buffer */
if(linea_actual==(lineas-1)) return(0);

/* mueve hacia abajo en el buffer */
linea_actual+=nlin;
/* si se pasa de la £ltima l¡nea lo coloca en la £ltima */
if(linea_actual>(lineas-1)) linea_actual=lineas-1;

/* mueve hacia abajo en pantalla */
fil_cursor+=nlin;
/* si se pasa de la £ltima l¡nea de pantalla lo coloca en la £ltima */
if(fil_cursor>(LIN_SCR+NUM_LIN_SCR-1)) {
	fil_cursor=LIN_SCR+NUM_LIN_SCR-1;
	return(1);
}

return(0);
}

/****************************************************************************
	MUEVE_CURSOR_ARRIBA: desplaza el cursor de edici¢n hacia arriba.
	  Entrada:      'nlin' n£mero de l¡neas a desplazar hacia arriba
			'ccur' 1 si quiere desplazar cursor de pantalla al
			mismo tiempo, 0 si no
	  Salida:       1 si se pas¢ de la primera l¡nea, 0 si no
****************************************************************************/
int mueve_cursor_arriba(int nlin)
{

/* comprueba si est  en la primera l¡nea del buffer */
if(linea_actual==0) return(0);

/* mueve hacia arriba en el buffer */
linea_actual-=nlin;
/* si se pasa de la primera l¡nea lo coloca en la primera */
if(linea_actual<0) linea_actual=0;

/* mueve hacia arriba en pantalla */
if(nlin>(fil_cursor-LIN_SCR)) {
	fil_cursor=LIN_SCR;
	return(1);
}
else fil_cursor-=nlin;

return(0);
}

/****************************************************************************
	SCROLL_ARR_SCR: hace scroll de la zona de edici¢n una l¡nea hacia
	  arriba.
****************************************************************************/
void scroll_arr_scr(void)
{

_asm {
	mov ax,0601h            ; funci¢n desplazar ventana arriba 1 lin.
	mov bh,color_editor     ; BH = color de relleno
	mov ch,LIN_SCR          ; coordenada Y de esquina sup. izq.
	mov cl,0                ;     "      X "     "     "    "
	mov dh,LIN_SCR+NUM_LIN_SCR-1    ; coord. Y de esquina inf. der.
	mov dl,79               ; coordenada X de esquina inf. der.
	int 10h
}

}

/****************************************************************************
	SCROLL_ABJ_SCR: hace scroll de la zona de edici¢n una l¡nea hacia
	  abajo.
****************************************************************************/
void scroll_abj_scr(void)
{

_asm {
	mov ax,0701h            ; funci¢n desplazar ventana abajo 1 lin.
	mov bh,color_editor     ; BH = color de relleno
	mov ch,LIN_SCR          ; coordenada Y de esquina sup. izq.
	mov cl,0                ;     "      X "     "     "    "
	mov dh,LIN_SCR+NUM_LIN_SCR-1    ; coord. Y de esquina inf. der.
	mov dl,79               ; coordenada X de esquina inf. der.
	int 10h
}

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
	INSERTA_CARACTER: inserta un car cter en la l¡nea actual, en la
	  posici¢n del cursor.
	  Entrada:      'car' car cter a insertar
	  Salida:       0 si no se pudo insertar el car cter, 1 si se pudo
****************************************************************************/
int inserta_caracter(char car)
{
char temp[CAR_LIN+1];
int i;

/* si no caben m s caracteres en la l¡nea actual, produce un pitido y sale */
if(strlen(ptxt[linea_actual])==CAR_LIN) {
	beep();
	return(0);
}

/* copia los caracteres hasta el cursor en un buffer temporal */
for(i=0; i<(int)col_cursor; i++) temp[i]=*(ptxt[linea_actual]+i);

/* a¤ade el car cter al buffer temporal */
temp[i]=car;

/* copia el resto de caracteres al buffer temporal */
for(; i<CAR_LIN; i++) temp[i+1]=*(ptxt[linea_actual]+i);

/* copia el buffer temporal a la l¡nea actual */
strcpy(ptxt[linea_actual],temp);

return(1);
}

/****************************************************************************
	BORRA_CARACTER: borra el car cter de la posici¢n anterior del cursor,
	  o el que est  sobre el cursor.
	  Entrada:      'borra' 1 si quiere borrar el car cter anterior al
			cursor, 0 si quiere borrar el car cter del cursor
	  Salida:       0 si no se pudo borrar el car cter (no hay m s
			caracteres o est  al principio de l¡nea),
			1 si se pudo
****************************************************************************/
int borra_caracter(int borra)
{
char temp[CAR_LIN+1];
int i;

/* si la l¡nea est  vac¡a sale */
if(strlen(ptxt[linea_actual])==0) return(0);

/* si quiere borrar el car cter anterior al cursor y est  al inicio, sale */
if((borra==1) && (col_cursor==0)) return(0);

/* copia los caracteres hasta el que hay que borrar en un buffer temporal */
for(i=0; i<(col_cursor-borra); i++) temp[i]=*(ptxt[linea_actual]+i);

/* copia los caracteres del cursor hasta el final */
for(; i<CAR_LIN; i++) temp[i]=*(ptxt[linea_actual]+(i+1));

/* copia el buffer temporal a la l¡nea actual */
strcpy(ptxt[linea_actual],temp);

return(1);
}

/****************************************************************************
	INSERTA_LINEA: inserta una l¡nea en el texto, desplazando el resto.
	  Entrada:      'linea' n£mero de la l¡nea d¢nde se insertar  la
			nueva
	  Salida:       1 si se pudo insertar, 0 si no
****************************************************************************/
int inserta_linea(int linea)
{
int i;

/* si buffer de texto est  lleno, sale */
if(lineas==max_lineas) {
	beep();
	return(0);
}

/* desplaza desde 'linea' todas, una posici¢n hacia abajo */
for(i=lineas-1; i>=linea; i--) strcpy(ptxt[i+1],ptxt[i]);

/* vac¡a la l¡nea que se insert¢ */
*ptxt[linea]='\0';

/* incrementa n£mero de l¡neas que hay en buffer */
lineas++;

return(1);
}

/****************************************************************************
	BORRA_LINEA: borra una l¡nea en el texto, desplazando el resto.
	  Entrada:      'linea' n£mero de la l¡nea que se borrar 
	  Salida:       1 si se pudo borrar, 0 si no
****************************************************************************/
int borra_linea(int linea)
{
int i;

/* si s¢lo hay una l¡nea, no la borra */
if(lineas==1) return(0);

/* traslada hacia arriba desde la l¡nea siguiente hasta la £ltima */
for(i=linea+1; i<lineas; i++) strcpy(ptxt[i-1],ptxt[i]);

/* anula la £ltima l¡nea */
*ptxt[lineas-1]='\0';

/* decrementa n£mero de l¡neas */
lineas--;

}

/****************************************************************************
	PASA_LINEA_A: pasa una l¡nea a la anterior, si cabe; luego borra la
	  l¡nea que ha pasado.
	  Entrada:      'linea' n£mero de l¡nea en el buffer de texto
	  Salida:       1 si la pudo pasar, 0 si no
****************************************************************************/
int pasa_linea_a(int linea)
{

/* si 'linea' es 0 o est  fuera de rango, sale */
if((linea==0) || (linea>=lineas)) return(0);

/* si la l¡nea cabe en la anterior, la pasa */
if(strlen(ptxt[linea])<=(size_t)(CAR_LIN-strlen(ptxt[linea-1]))) {
	strcat(ptxt[linea-1],ptxt[linea]);
	borra_linea(linea);
}
/* si no cabe, emite pitido y sale */
else {
	beep();
	return(0);
}

return(1);
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
	BUSCA_TEXTO: busca una cadena dentro del texto del editor y si
	  la encuentra posiciona el cursor.
	  Entrada:      'cad' cadena a buscar
			'lin' l¡nea a partir de la que se realiza la b£squeda
	  Salida:       1 si la encontr¢, 0 si no
****************************************************************************/
int busca_texto(char *cad, int lin)
{
int i;
char txt[CAR_LIN+1], *p;

/* si la cadena a buscar es nula, sale */
if(*cad=='\0') {
	beep();
	return(1);
}

/* convierte a may£scula la cadena a buscar */
for(p=cad; *p; p++) *p=mayuscula(*p);

/* busca desde 'lin' hacia delante */
i=lin;
while(i<lineas) {
	/* copia y convierte a may£scula l¡nea actual */
	strcpy(txt,ptxt[i]+col_cursor);
	for(p=txt; *p; p++) *p=mayuscula(*p);

	p=strstr(txt,cad);
	if(p!=NULL) {
		linea_actual=i;
		fil_cursor=LIN_SCR;
		col_cursor+=p-txt;
		return(1);
	}

	col_cursor=0;
	i++;
}

/* busca desde primera l¡nea hasta 'lin' */
i=0;
while(i<lin) {
	/* copia y convierte a may£scula l¡nea actual */
	strcpy(txt,ptxt[i]+col_cursor);
	for(p=txt; *p; p++) *p=mayuscula(*p);

	p=strstr(txt,cad);
	if(p!=NULL) {
		linea_actual=i;
		fil_cursor=LIN_SCR;
		col_cursor+=p-txt;
		return(1);
	}

	col_cursor=0;
	i++;
}

return(0);
}

/****************************************************************************
	CABECERA: imprime la cabecera del editor.
****************************************************************************/
void cabecera(void)
{
char lc[81];

sprintf(lc,"%13c%s",' ',"EDITOR "COPYRIGHT" versi¢n "VERSION_EDS);
imp_linea_scr(LIN_SCR-2,0,color_cab,lc,80);

}

/****************************************************************************
	LINEA_ESTADO: imprime la l¡nea de estado del editor.
****************************************************************************/
void linea_estado(void)
{
char ls[81];

sprintf(ls,"%-69sº %05u:%02u",texto_estado,linea_actual+1,col_cursor+1);
imp_linea_scr(LIN_SCR+NUM_LIN_SCR,0,color_linst,ls,80);

}

#pragma check_pointer(off)
/****************************************************************************
	REGLETA: imprime la regleta, con el marcador de posici¢n del
	  cursor.
****************************************************************************/
void regleta(void)
{
BYTE _far *pscr, attr;

imp_linea_scr(LIN_SCR-1,0,color_regla,REGLETA,80);

/* calcula direcci¢n en buffer de v¡deo */
pscr=(BYTE _far *)0xb8000000+((((LIN_SCR-1)*80)+col_cursor)*2);

/* direcci¢n de atributos */
pscr++;

/* invierte atributos */
attr=*pscr;
*pscr >>= 4;
*pscr |= (attr << 4);
/* elimina parpadeo */
*pscr &= 0x7f;

}
#pragma check_pointer()

/****************************************************************************
	MENU: presenta un men£ de 4 columnas por 4 elementos cada una,
	  en la parte inferior, y deja escoger una de las opciones.
	  Salida:       n£mero de opci¢n elegida
****************************************************************************/
int menu(void)
{
int i, j;
short fil, col;
unsigned tecla;
static int opcion=0;
/* posici¢n del cursor como si se moviese por una matriz de 4x4 */
BYTE fcur, ccur;
char teclas_menu[NUM_OPC];
BOOLEAN tecla_activacion=FALSE;

/* esconde cursor */
pon_cursor(25,0);

/* inicializa posici¢n del cursor */
fcur=(BYTE)(opcion%4);
ccur=(BYTE)(opcion/4);

/* rellena tabla de teclas de activaci¢n */
for(i=0; i<NUM_OPC; i++) {
	for(j=0; j<LNG_OPC; j++) {
		if(menu_eds[i][j]==CTECLA)
		  teclas_menu[i]=mayuscula(menu_eds[i][j+1]);
	}
}

/* borra zona de men£ */
for(fil=LIN_SCR+NUM_LIN_SCR+1; fil<(LIN_SCR+NUM_LIN_SCR+5); fil++)
  borra_linea_scr(fil,color_menu);

/* origen del men£ */
fil=LIN_SCR+NUM_LIN_SCR+1;
col=1;

/* imprime men£ */
for(i=0; i<NUM_OPC; i++) {
	imp_opcion_menu(menu_eds[i],fil,col,color_menu,color_tecla);
	if(i<12) imp_linea_scr(fil,col+18,color_menu,"³",1);
	fil++;
	if(fil>24) {
		fil=LIN_SCR+NUM_LIN_SCR+1;
		col+=20;
	}
}

do {
	/* imprime opci¢n resaltada */
	imp_opcion_menu(menu_eds[opcion],fcur+LIN_SCR+NUM_LIN_SCR+1,
	  (ccur*20)+1,color_opcion,color_opcion);

	/* espera tecla */
	tecla=_bios_keybrd(_KEYBRD_READ);

	/* borra opci¢n resaltada */
	imp_opcion_menu(menu_eds[opcion],fcur+LIN_SCR+NUM_LIN_SCR+1,
	  (ccur*20)+1,color_menu,color_tecla);

	switch(tecla >> 8) {
		case TCUR_ARR :
			if(fcur>0) {
				fcur--;
				opcion--;
			}
			break;
		case TCUR_ABJ :
			if(fcur<3) {
				fcur++;
				opcion++;
			}
			break;
		case TCUR_IZQ :
			if(ccur>0) {
				ccur--;
				opcion-=4;
			}
			break;
		case TCUR_DER :
			if(ccur<3) {
				ccur++;
				opcion+=4;
			}
			break;
	}

	/* comprueba si se puls¢ alguna tecla de activaci¢n */
	for(i=0; i<NUM_OPC; i++) {
		if(teclas_menu[i]==mayuscula((char)(tecla & 0x00ff))) break;
	}
	if(i!=NUM_OPC) {
		/* borra opci¢n resaltada */
		fcur=(BYTE)(opcion%4);
		ccur=(BYTE)(opcion/4);
		imp_opcion_menu(menu_eds[opcion],fcur+LIN_SCR+NUM_LIN_SCR+1,
		  (ccur*20)+1,color_menu,color_tecla);
		opcion=i;
		tecla_activacion=TRUE;
	}
} while(((tecla & 0x00ff)!=RETURN) && (tecla_activacion==FALSE));

/* muestra cursor */
pon_cursor(0,0);

return(opcion);
}

/****************************************************************************
	IMP_OPCION_MENU: imprime una opci¢n del men£.
	  Entrada:      'opcion' opcion a imprimir
			'fil', 'col' posici¢n
			'color1' color de opci¢n
			'color2' color de tecla de activaci¢n
****************************************************************************/
void imp_opcion_menu(char *opcion, short fil, short col, BYTE color1,
  BYTE color2)
{
int i;
char *p;

/* calcula posici¢n del car cter CTECLA */
for(p=opcion, i=0; *p; p++, i++) if(*p==CTECLA) break;

imp_linea_scr(fil,col,color1,opcion,i);
imp_linea_scr(fil,col+i,color2,opcion+i+1,1);
imp_linea_scr(fil,col+i+1,color1,opcion+i+2,16-i);

}

/****************************************************************************
	COPIA_FICHEROT: copia un fichero de texto en otro.
	  Entrada:      'nfichorg' nombre de fichero de origen
			'nfichdest' nombre de fichero destino
	  Salida:       1 si no hubo errores, 0 si los hubo
****************************************************************************/
int copia_ficherot(char *nfichorg, char *nfichdest)
{
char c[CAR_LIN+2];
FILE *forg, *fdest;

/* abre ficheros */
if((forg=fopen(nfichorg,"rt"))==NULL) return(0);
if((fdest=fopen(nfichdest,"wt"))==NULL) {
	fclose(forg);
	return(0);
}

/* copia fichero origen en fichero de destino */
while(1) {
	if(fgets(c,CAR_LIN+2,forg)==NULL) {
		if(feof(forg)) break;
		else {
			fclose(forg);
			fclose(fdest);
			return(0);
		}
	}
	if(fputs(c,fdest)!=0) {
		fclose(forg);
		fclose(fdest);
		return(0);
	}
}

/* cierra ficheros de origen y de destino */
fclose(forg);
fclose(fdest);

return(1);
}

/****************************************************************************
	ABRE_FICHERO_BD: abre un fichero de base de datos, el cual debe
	  existir.
	  Entrada:      'nfich' nombre del fichero
	  Salida:       1 si se pudo abrir, 0 si no; y variable global
			'fbdat' actualizada
****************************************************************************/
int abre_fichero_bd(char *nfich)
{

/* presenta mensaje */
imp_linea_scr(LIN_SCR+NUM_LIN_SCR,1,color_linst,"Cargando...",68);
pon_cursor(25,0);

/* copia el contenido del fichero original en fichero temporal */
if(copia_ficherot(nfich,Nfbdat)!=1) {
	fbdat=FALSE;
	/* borra mensaje */
	linea_estado();
	return(0);
}

pon_cursor(0,0);

/* borra mensaje */
linea_estado();

/* indica que hay base de datos en uso */
fbdat=TRUE;

return(1);
}

/****************************************************************************
	CIERRA_FICHERO_BD: cierra un fichero de base de datos
	  Entrada:      'nfich' nombre de fichero d¢nde se volcar  el
			contenido del fichero temporal de base de datos
			'Nfbdat'
	  Salida:       1 si se pudo completar operaci¢n, 0 si no
****************************************************************************/
int cierra_fichero_bd(char *nfich)
{
char tecla;

/* si no hay fichero de base de datos abierto, sale */
if(fbdat==FALSE) return(0);

/* si el fichero de destino existe, pregunta si se quiere continuar */
if(!access(nfich,0)) {
	imp_linea_scr(LIN_SCR+NUM_LIN_SCR,1,color_linst,
	  "Fichero ya existe. ¨Reemplazar?",68);
	pon_cursor(25,0);
	tecla=(char)(_bios_keybrd(_KEYBRD_READ) & 0x00ff);

	pon_cursor(0,0);
	/* borra mensaje */
	linea_estado();

	if((tecla!='s') && (tecla!='S')) return(0);
}

/* presenta mensaje */
imp_linea_scr(LIN_SCR+NUM_LIN_SCR,1,color_linst,"Grabando...",68);
pon_cursor(25,0);

/* copia fichero temporal en fichero de destino */
if(copia_ficherot(Nfbdat,nfich)!=1) {
	/* borra mensaje */
	linea_estado();
	return(0);
}

pon_cursor(0,0);

/* borra mensaje */
linea_estado();

return(1);
}

/****************************************************************************
	CREA_NUEVA_BD: crea un nuevo fichero de base de datos vac¡o
	  Entrada:      'nfich' nombre de fichero a crear
	  Salida:       1 si se pudo completar operaci¢n, 0 si no
****************************************************************************/
int crea_nueva_bd(char *nfich)
{
FILE *fbd;
char *marca_fin="\n\\END\n\n";

if((fbd=fopen(nfich,"wt"))==NULL) return(0);

fputs("\\\\V_MOV\t\t14\n",fbd);
fputs("\\\\N_CONV\t20\n",fbd);
fputs("\\\\N_PROP\t50\n\n",fbd);

fputs("\\MSY\n",fbd);
fputs(marca_fin,fbd);

fputs("\\VOC\n",fbd);
fputs(marca_fin,fbd);

fputs("\\MSG\n",fbd);
fputs(marca_fin,fbd);

fputs("\\LOC\n",fbd);
fputs(marca_fin,fbd);

fputs("\\OBJ\n",fbd);
fputs(marca_fin,fbd);

fputs("\\PRO 0\n",fbd);
fputs(marca_fin,fbd);

fclose(fbd);

return(1);
}

/****************************************************************************
	NUEVO_FICHERO_BD: crea un nuevo fichero de base de datos
	  Entrada:      'nfich' nombre de fichero a crear
	  Salida:       1 si se pudo completar operaci¢n, 0 si no
****************************************************************************/
int nuevo_fichero_bd(char *nfich)
{
char tecla, bd_inicio[_MAX_PATH];

/* si el fichero existe */
if(!access(nfich,0)) {
	imp_linea_scr(LIN_SCR+NUM_LIN_SCR,1,color_linst,
	  "Fichero ya existe. ¨Borrarlo?",68);
	pon_cursor(25,0);
	tecla=(char)(_bios_keybrd(_KEYBRD_READ) & 0x00ff);

	pon_cursor(0,0);
	/* borra mensaje */
	linea_estado();

	if((tecla!='s') && (tecla!='S')) return(1);
}

imp_linea_scr(LIN_SCR+NUM_LIN_SCR,1,color_linst,
  "¨Usar base de datos de inicio?",68);
pon_cursor(25,0);
tecla=(char)(_bios_keybrd(_KEYBRD_READ) & 0x00ff);

pon_cursor(0,0);
/* borra mensaje */
linea_estado();

/* presenta mensaje */
imp_linea_scr(LIN_SCR+NUM_LIN_SCR,1,color_linst,MsgEspera,68);
pon_cursor(25,0);

/* si quiere usar base de datos de inicio, copia FBD_INICIO en 'nfich' */
/* si no crea uno nuevo vac¡o */
if((tecla=='s') || (tecla=='S')) {
	strcpy(bd_inicio,dir_bd);
	strcat(bd_inicio,FBD_INICIO);
	if(copia_ficherot(bd_inicio,nfich)!=1) {
		pon_cursor(0,0);
		/* borra mensaje */
		linea_estado();
		return(0);
	}
}
else if(crea_nueva_bd(nfich)!=1) return(0);

pon_cursor(0,0);
/* borra mensaje */
linea_estado();

/* si pudo crear nuevo fichero, lo carga */
if(abre_fichero_bd(nfich)!=1) return(0);

return(1);
}

/****************************************************************************
	SEPARA_SECCION: separa del fichero de base de datos temporal 'Nfbdat',
	  una secci¢n y la almacena en un fichero temporal 'Nfsecc'.
	  Entrada:      'csecc' cabecera de la secci¢n ("\VOC", "\PRO 1",
			etc...)
	  Salida:       1 si se pudo completar operaci¢n, 0 si error
****************************************************************************/
int separa_seccion(char *csecc)
{
FILE *fbd, *fsecc;
char c[CAR_LIN+2];
BOOLEAN fin_seccion=FALSE;

if((fbd=fopen(Nfbdat,"rt"))==NULL) {
	error(MsgErr_Aper_Ft,0);
	return(0);
}

if((fsecc=fopen(Nfsecc,"wt"))==NULL) {
	error(MsgErr_Aper_Ft,0);
	fclose(fbd);
	return(0);
}

/* presenta mensaje */
imp_linea_scr(LIN_SCR+NUM_LIN_SCR,1,color_linst,MsgEspera,68);
pon_cursor(25,0);

while(fin_seccion==FALSE) {
	if(fgets(c,CAR_LIN+2,fbd)==NULL) {
		if(feof(fbd)) {
			error("Secci¢n no encontrada.",0);
			fclose(fbd);
			fclose(fsecc);
			return(0);
		}
		else {
			error(MsgErr_Lect_Bd,0);
			fclose(fbd);
			fclose(fsecc);
			return(0);
		}
	}

	/* si encuentra encabezamiento de secci¢n, copia l¡neas hasta */
	/* encontrar '\END' */
	if(busca_marca(csecc,c)==1) {
		while(1) {
			if(fgets(c,CAR_LIN+2,fbd)==NULL) {
				if(feof(fbd)) {
					error("Falta marca fin secci¢n.",0);
					fclose(fbd);
					fclose(fsecc);
					return(0);
				}
				else {
					error(MsgErr_Lect_Bd,0);
					fclose(fbd);
					fclose(fsecc);
					return(0);
				}
			}

			if(busca_marca(Mfin_Secc,c)==1) {
				fin_seccion=TRUE;
				break;
			}

			if(fputs(c,fsecc)!=0) {
				error(MsgErr_Escr_Ft,0);
				fclose(fbd);
				fclose(fsecc);
				return(0);
			}
		}
	}
}

pon_cursor(0,0);
/* borra mensaje */
linea_estado();

/* cierra ficheros */
fclose(fbd);
fclose(fsecc);

return(1);
}

/****************************************************************************
	BUSCA_MARCA: busca una marca de inicio o final de secci¢n.
	  Entrada:      'marca' marca a buscar ('\MSG', '\PRO 0', '\END',
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
	EDITA_SECC: edita la secci¢n actual.
	  Entrada:      'msecc' marca de inicio de secci¢n
			'lin' l¡nea de comienzo de edici¢n
	  Salida:       1 si se pudo completar operaci¢n, 0 si no
****************************************************************************/
int edita_secc(char *msecc, int lin)
{
BOOLEAN modificado;

/* si no hay base de datos cargada, sale */
if(fbdat==FALSE) {
	error(MsgNo_Hay_Fich,0);
	return(0);
}

/* busca y separa a un fichero temporal 'Nfsecc' la secci¢n */
if(separa_seccion(msecc)!=1) return(0);

if(editor(Nfsecc,lin,&modificado)==0) return(0);

if(modificado==TRUE) {
	/* actualiza cambios en fichero 'Nfbdat' */
	if(sustituye_secc(msecc)!=1) return(0);
}

return(1);
}

/****************************************************************************
	SUSTITUYE_SECC: sustituye una secci¢n del fichero con la copia de
	  la base de datos 'Nfbdat', por el contenido del fichero temporal
	  'Nfsecc'.
	  Entrada:      'msecc' marca de inicio de la secci¢n
	  Salida:       1 si se pudo completar la operaci¢n, 0 si error
****************************************************************************/
int sustituye_secc(char *msecc)
{
FILE *fbd, *fsecc, *faux;
char c[CAR_LIN+2];

/* presenta mensaje */
imp_linea_scr(LIN_SCR+NUM_LIN_SCR,1,color_linst,MsgGuard,68);
pon_cursor(25,0);

if((fbd=fopen(Nfbdat,"rt"))==NULL) {
	error(MsgErr_Aper_Ft,0);
	return(0);
}

if((fsecc=fopen(Nfsecc,"rt"))==NULL) {
	error(MsgErr_Aper_Ft,0);
	fclose(fbd);
	return(0);
}

if((faux=fopen(Nfaux,"wt"))==NULL) {
	error(MsgErr_Aper_Ft,0);
	fclose(fbd);
	fclose(fsecc);
	return(0);
}

/* copia desde inicio de 'Nfbdat' en 'Nfaux' hasta encontrar marca secci¢n */
do {
	if(fgets(c,CAR_LIN+2,fbd)==NULL) {
		if(feof(fbd)) break;
		else {
			error(MsgErr_Lect_Ft,0);
			fclose(fbd);
			fclose(fsecc);
			fclose(faux);
			return(0);
		}
	}

	if(fputs(c,faux)!=0) {
		error(MsgErr_Escr_Ft,0);
		fclose(fbd);
		fclose(fsecc);
		fclose(faux);
		return(0);
	}
} while(busca_marca(msecc,c)!=1);

/* inserta el contenido del fichero 'Nfsecc' en 'Nfaux' */
while(1) {
	if(fgets(c,CAR_LIN+2,fsecc)==NULL) {
		if(feof(fsecc)) break;
		else {
			error(MsgErr_Lect_Ft,0);
			fclose(fbd);
			fclose(fsecc);
			fclose(faux);
			return(0);
		}
	}

	if(fputs(c,faux)!=0) {
		error(MsgErr_Escr_Ft,0);
		fclose(fbd);
		fclose(fsecc);
		fclose(faux);
		return(0);
	}
}

fclose(fsecc);

/* deshecha contenido de 'Nfbdat' hasta encontrar un '\END' y luego */
/* copia en 'Nfaux' a partir de ah¡ */
do {
	if(fgets(c,CAR_LIN+2,fbd)==NULL) {
		if(feof(fbd)) break;
		else {
			error(MsgErr_Lect_Ft,0);
			fclose(fbd);
			fclose(faux);
			return(0);
		}
	}
} while(busca_marca(Mfin_Secc,c)!=1);

while(1) {
	if(fputs(c,faux)!=0) {
		error(MsgErr_Escr_Ft,0);
		fclose(fbd);
		fclose(faux);
		return(0);
	}

	if(fgets(c,CAR_LIN+2,fbd)==NULL) {
		if(feof(fbd)) break;
		else {
			error(MsgErr_Lect_Ft,0);
			fclose(fbd);
			fclose(faux);
			return(0);
		}
	}
}

fclose(fbd);
fclose(faux);

/* borra fichero temporal de base de datos */
remove(Nfbdat);

/* renombra fichero auxiliar a fichero temporal de base de datos */
rename(Nfaux,Nfbdat);

pon_cursor(0,0);
/* borra mensaje */
linea_estado();

return(1);
}

/****************************************************************************
	PIDE_NUMERO: permite introducir por teclado un n£mero de 0 a 255.
	  Entrada:      'cad' cadena a imprimir para introducci¢n
	  Salida:       n£mero tecleado
****************************************************************************/
int pide_numero(char *cad)
{
char num[4];
int l, numero=9999;

do {
	linea_estado();

	/* inicializa buffer */
	num[0]='\0';

	/* coge input de teclado */
	imp_linea_scr(LIN_SCR+NUM_LIN_SCR,1,color_linst,cad,68);
	l=strlen(cad);
	lee_input(LIN_SCR+NUM_LIN_SCR,(BYTE)(l+1),num,3,4,color_linst);

	numero=atoi(num);
} while((numero<0) || (numero>255));

/* reimprime para borrar */
linea_estado();

return(numero);
}

/****************************************************************************
	ES_MARCA_SECC: comprueba si una l¡nea es marca de secci¢n.
	  Entrada:      'lin' l¡nea a comprobar
	  Salida:       TRUE si es marca de secci¢n, FALSE si no
****************************************************************************/
BOOLEAN es_marca_secc(char *lin)
{

/* salta blancos iniciales */
for(; ((*lin==' ') || (*lin=='\t')); lin++);

/* mira se es marca de secci¢n (y no definici¢n de constante) */
if((*lin==MARCA_S) && (*(lin+1)!=MARCA_S)) return(TRUE);

return(FALSE);
}

/****************************************************************************
	EDITA_CONSTANTES: edita las constantes; se supone que las constantes
	  estar n al inicio de la base de datos por lo que se leer  desde
	  ah¡ hasta la primera marca de secci¢n.
	  Entrada:      'lin' l¡nea de inicio de la edici¢n
	  Salida:       1 si se pudo completar operaci¢n, 0 si no
****************************************************************************/
int edita_constantes(int lin)
{
FILE *fbd, *fsecc, *faux;
char c[CAR_LIN+2];
BOOLEAN modificado;

/* si no hay base de datos cargada, sale */
if(fbdat==FALSE) {
	error(MsgNo_Hay_Fich,0);
	return(0);
}

/* presenta mensaje */
imp_linea_scr(LIN_SCR+NUM_LIN_SCR,1,color_linst,MsgEspera,68);
pon_cursor(25,0);

/* busca y separa a un fichero temporal 'Nfsecc' las constantes */
if((fbd=fopen(Nfbdat,"rt"))==NULL) {
	error(MsgErr_Aper_Ft,0);
	return(0);
}

if((fsecc=fopen(Nfsecc,"wt"))==NULL) {
	error(MsgErr_Aper_Ft,0);
	fclose(fbd);
	return(0);
}

while(1) {
	if(fgets(c,CAR_LIN+2,fbd)==NULL) {
		if(feof(fbd)) break;
		else {
			error(MsgErr_Lect_Bd,0);
			fclose(fbd);
			fclose(fsecc);
			return(0);
		}
	}

	/* comprueba si ha encontrado una marca de secci¢n */
	if(es_marca_secc(c)==TRUE) break;

	if(fputs(c,fsecc)!=0) {
		error(MsgErr_Escr_Ft,0);
		fclose(fbd);
		fclose(fsecc);
		return(0);
	}
}

fclose(fbd);
fclose(fsecc);

pon_cursor(0,0);
/* borra mensaje */
linea_estado();

if(editor(Nfsecc,lin,&modificado)==0) return(0);

if(modificado==TRUE) {
	/* guarda en copia base de datos los cambios */
	/* presenta mensaje */
	imp_linea_scr(LIN_SCR+NUM_LIN_SCR,1,color_linst,MsgGuard,68);
	pon_cursor(25,0);

	if((faux=fopen(Nfaux,"wt"))==NULL) {
		error(MsgErr_Aper_Ft,0);
		return(0);
	}

	/* copia fichero 'Nfsecc' al inicio de 'Nfaux' */
	if((faux=fopen(Nfaux,"wt"))==NULL) {
		error(MsgErr_Aper_Ft,0);
		return(0);
	}

	if((fsecc=fopen(Nfsecc,"rt"))==NULL) {
		error(MsgErr_Aper_Ft,0);
		fclose(faux);
		return(0);
	}

	while(1) {
		if(fgets(c,CAR_LIN+2,fsecc)==NULL) {
			if(feof(fsecc)) break;
			else {
				error(MsgErr_Lect_Ft,0);
				fclose(faux);
				fclose(fsecc);
				return(0);
			}
		}

		if(fputs(c,faux)!=0) {
			error(MsgErr_Escr_Ft,0);
			fclose(faux);
			fclose(fsecc);
			return(0);
		}
	}

	fclose(fsecc);

	/* copia desde primera marca de secci¢n de 'Nfbdat' hasta el final */
	/* en 'Nfaux' */
	if((fbd=fopen(Nfbdat,"rt"))==NULL) {
		error(MsgErr_Aper_Ft,0);
		fclose(faux);
		return(0);
	}

	/* busca marca de secci¢n */
	while(1) {
		if(fgets(c,CAR_LIN+2,fbd)==NULL) {
			if(feof(fbd)) break;
			else {
				error(MsgErr_Lect_Ft,0);
				fclose(faux);
				fclose(fbd);
				return(0);
			}
		}

		/* comprueba si es marca de secci¢n */
		if(es_marca_secc(c)==TRUE) break;
	}

	/* empieza escribiendo l¡nea en 'Nfaux' para copiar la de la marca */
	/* de secci¢n le¡da anteriormente */
	while(1) {
		if(fputs(c,faux)!=0) {
			error(MsgErr_Escr_Ft,0);
			fclose(faux);
			fclose(fbd);
			return(0);
		}

		if(fgets(c,CAR_LIN+2,fbd)==NULL) {
			if(feof(fbd)) break;
			else {
				error(MsgErr_Lect_Ft,0);
				fclose(faux);
				fclose(fbd);
				return(0);
			}
		}
	}

	fclose(faux);
	fclose(fbd);

	/* borra fichero temporal de base de datos */
	remove(Nfbdat);

	/* renombra fichero auxiliar a fichero temporal de base de datos */
	rename(Nfaux,Nfbdat);

	pon_cursor(0,0);
	/* borra mensaje */
	linea_estado();
}

return(1);
}

/****************************************************************************
	CREA_PROCESO: inserta un nuevo proceso en la copia de la base
	  de datos.
	  Entrada:      'pro' n£mero de proceso a insertar (0 a 255), si el
			proceso ya existe, no hace nada, si no inserta uno
			nuevo
	  Salida:       1 si se complet¢ la operaci¢n, 0 si no
****************************************************************************/
int crea_proceso(int pro)
{
FILE *fbd, *faux;
char c[CAR_LIN+2], *p, num[4], tecla;
int i, numero;

/* presenta mensaje */
imp_linea_scr(LIN_SCR+NUM_LIN_SCR,1,color_linst,MsgEspera,68);
pon_cursor(25,0);

if((fbd=fopen(Nfbdat,"rt"))==NULL) {
	error(MsgErr_Aper_Ft,0);
	return(0);
}

if((faux=fopen(Nfaux,"wt"))==NULL) {
	error(MsgErr_Aper_Ft,0);
	fclose(fbd);
	return(0);
}

while(1) {
	if(fgets(c,CAR_LIN+2,fbd)==NULL) {
		if(feof(fbd)) break;
		else {
			error(MsgErr_Lect_Ft,0);
			fclose(fbd);
			fclose(faux);
			return(0);
		}
	}

	/* mira si es marca de inicio de proceso, comprueba su n£mero */
	if(busca_marca(Mpro,c)==1) {
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

		/* si n£mero de proceso encontrado es mayor que el */
		/* que queremos insertar, sale; si es igual finaliza */
		if(numero>pro) break;

		if(numero==pro) {
			fclose(fbd);
			fclose(faux);
			return(1);
		}
	}

	if(fputs(c,faux)!=0) {
		error(MsgErr_Escr_Ft,0);
		fclose(fbd);
		fclose(faux);
		return(0);
	}
}

imp_linea_scr(LIN_SCR+NUM_LIN_SCR,1,color_linst,
  "Proceso nuevo. ¨Crear?",68);
pon_cursor(25,0);
tecla=(char)(_bios_keybrd(_KEYBRD_READ) & 0x00ff);

pon_cursor(0,0);
/* borra mensaje */
linea_estado();

if((tecla!='s') && (tecla!='S')) {
	fclose(fbd);
	fclose(faux);
	return(0);
}

/* inserta proceso nuevo */
fprintf(faux,"%s %i\n",Mpro,pro);
fprintf(faux,"%s\n",Mfin_Secc);

/* copia resto de 'Nfbdat' en 'Nfaux' */
/* pone condici¢n de que no est‚ al final del fichero 'fbd' ya que si */
/* es as¡, el nuevo proceso se insert¢ al final */
while(!feof(fbd)) {
	if(fputs(c,faux)!=0) {
		error(MsgErr_Escr_Ft,0);
		fclose(fbd);
		fclose(faux);
		return(0);
	}

	if(fgets(c,CAR_LIN+2,fbd)==NULL) {
		if(feof(fbd)) break;
		else {
			error(MsgErr_Lect_Ft,0);
			fclose(fbd);
			fclose(faux);
			return(0);
		}
	}
}

fclose(fbd);
fclose(faux);

/* borra fichero temporal de base de datos */
remove(Nfbdat);

/* renombra fichero auxiliar a fichero temporal de base de datos */
rename(Nfaux,Nfbdat);

pon_cursor(0,0);
/* borra mensaje */
linea_estado();

return(1);
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
char programa[_MAX_PATH];
int codigo;

/* reutiliza memoria */
_heapmin();

/* crea nombre completo con ruta */
strcpy(programa,ruta);
strcat(programa,prg);

/* pone puntero a nombre completo de programa en primer elemento de matriz */
arg[0]=programa;

codigo=spawnv(P_WAIT,programa,arg);

/* imprime pantalla del editor */
inicializa_pantalla();

if(codigo==-1) return(0);

return(1);
}

/****************************************************************************
	LEE_CONFIG: lee fichero de configuraci¢n (si existe) y asigna
	  directorios y colores.
****************************************************************************/
void lee_config(void)
{
STC_CFG cfg;
FILE *fcfg;

/* inicializa directorios */
getcwd(dir_sintac,_MAX_PATH-1);
getcwd(dir_gcs,_MAX_PATH-1);
getcwd(dir_bd,_MAX_PATH-1);

if((fcfg=fopen(NF_CFG,"rb"))==NULL) return;
if(fread(&cfg,sizeof(STC_CFG),1,fcfg)!=1) {
	fclose(fcfg);
	return;
}
fclose(fcfg);

/* asigna directorios y colores */
strcpy(dir_sintac,cfg.dir_sintac);
strcpy(dir_gcs,cfg.dir_gcs);
strcpy(dir_bd,cfg.dir_bd);

color_scr=cfg.color_scr;
color_editor=cfg.color_editor;
color_regla=cfg.color_regla;
color_cab=cfg.color_cab;
color_linst=cfg.color_linst;
color_menu=cfg.color_menu;
color_tecla=cfg.color_tecla;
color_opcion=cfg.color_opcion;
color_err=cfg.color_err;
color_errsel=cfg.color_errsel;
color_ay=cfg.color_ay;
color_aptdo=cfg.color_aptdo;
color_ref=cfg.color_ref;

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
char drive_n[_MAX_DRIVE], dir_n[_MAX_DIR], fname_n[_MAX_FNAME],
  ext_n[_MAX_EXT];

_splitpath(fich,drive_n,dir_n,fname_n,ext_n);

/* el nombre del fichero de salida ser  como el de entrada pero */
/* con extensi¢n .DAT */
strcpy(fsal,drive_n);
strcat(fsal,dir_n);
strcat(fsal,fname_n);
strcat(fsal,".DAT");

}

/****************************************************************************
	ERRORES_COMPILADOR: procesa los mensajes de error del compilador.
****************************************************************************/
void errores_compilador(void)
{
int n_msgerr=0, i, msg;
FILE *ferr;
char lerr[81], nl[41], *l, seccion[9];
short fcur, fil;
unsigned tecla;
unsigned long lin_seccion;
STC_MERRC msgerr_compil[MAX_MSGERR];

/* si no hay fichero de errores, sale */
if(access(Nferr,0)!=0) return;

if((ferr=fopen(Nferr,"rt"))==NULL) {
	beep();
	return;
}

/* lee y almacena mensajes de error hasta el m ximo admitido */
while(n_msgerr<MAX_MSGERR) {
	if(fgets(lerr,81,ferr)==NULL) break;

	/* elimina '\n' final, si lo hay */
	for(l=lerr; *l; l++) {
		if(*l=='\n') {
			*l='\0';
			break;
		}
	}

	/* separa n£mero de l¡nea de mensaje */
	for(l=lerr, i=0; *l!=':'; l++, i++) nl[i]=*l;
	nl[i]='\0';
	msgerr_compil[n_msgerr].nlin=atol(nl);
	strcpy(msgerr_compil[n_msgerr].msg,l+2);

	n_msgerr++;
}

fclose(ferr);

/* dibuja ventana de errores */
dibuja_ventana(FIL_ERR,COL_ERR,ANCH_ERR,ALT_ERR,color_err);
imp_linea_scr(FIL_ERR,COL_ERR+((ANCH_ERR-9)/2),color_err," ERRORES ",9);

pon_cursor(25,0);
msg=0;
fcur=FIL_ERR+1;

while(1) {
	for(i=msg-(fcur-FIL_ERR-1), fil=FIL_ERR+1; (fil<(FIL_ERR+ALT_ERR-1) &&
	  (i<n_msgerr)); i++, fil++) {
		if(i==msg) imp_linea_scr(fil,COL_ERR+1,color_errsel,
		  msgerr_compil[i].msg,ANCH_ERR-2);
		else imp_linea_scr(fil,COL_ERR+1,color_err,
		  msgerr_compil[i].msg,ANCH_ERR-2);
	}

	/* espera tecla */
	tecla=_bios_keybrd(_KEYBRD_READ);

	switch(tecla >> 8) {
		case TCUR_ARR :
			if(msg>0) {
				msg--;
				if(fcur>(FIL_ERR+1)) fcur--;
			}
			break;
		case TCUR_ABJ :
			if(msg<(n_msgerr-1)) {
				msg++;
				if(fcur<(FIL_ERR+ALT_ERR-2)) fcur++;
			}
			break;
		case TESC :
			pon_cursor(0,0);
			return;
			break;
	}

	/* si se pulsa RETURN, sale */
	if((tecla & 0x00ff)==RETURN) break;
}

if(busca_linea_secc(msgerr_compil[msg].nlin,seccion,&lin_seccion)==1) {
	if(lin_seccion==0) edita_constantes((int)(msgerr_compil[msg].nlin-1));
	else edita_secc(seccion,(int)(msgerr_compil[msg].nlin-lin_seccion-1));
}

pon_cursor(0,0);

}

/****************************************************************************
	BUSCA_LINEA_SECC: busca a que secci¢n pertenece una l¡nea, y devuelve
	  la marca identificadora de esa secci¢n.
	  Entrada:      'nlin' n£mero de l¡nea (distinto de 0)
			'secc' puntero a cadena donde se colocar  marca de
			secci¢n (debe tener capacidad para al menos 9
			caracteres)
			'lin_secc' puntero a variable donde se guardar  la
			l¡nea inicial de la secci¢n
	  Salida:       0 si error, 1 si no
****************************************************************************/
int busca_linea_secc(unsigned long nlin, char *secc, unsigned long *lin_secc)
{
FILE *fbd;
char c[CAR_LIN+2], marca[9], *l;
unsigned long lin_marca=0, lin_act=0;
int i;

/* si el n£mero de l¡nea es 0, sale con error */
if(!nlin) return(0);

if((fbd=fopen(Nfbdat,"rt"))==NULL) {
	error(MsgErr_Aper_Ft,0);
	return(0);
}

/* inicializa cadenas de marca de secci¢n */
*secc='\0';
marca[0]='\0';

while(1) {
	if(fgets(c,CAR_LIN+2,fbd)==NULL) {
		if(feof(fbd)) break;
		else {
			error(MsgErr_Lect_Bd,0);
			fclose(fbd);
			return(0);
		}
	}

	/* incrementa n£mero de l¡nea del fichero de base de datos */
	lin_act++;

	/* si encuentra una marca de secci¢n, mira si el n£mero de l¡nea es */
	/* mayor que la que buscamos, si es as¡ devuelve la marca anterior */
	/* y su n£mero de l¡nea; si no almacena la marca y su l¡nea */
	if(es_marca_secc(c)==TRUE) {
		if(lin_act>nlin) {
			strcpy(secc,marca);
			*lin_secc=lin_marca;
			fclose(fbd);
			return(1);
		}
		else {
			lin_marca=lin_act;

			/* recoge la marca de secci¢n */
			for(l=c; *l!=MARCA_S; l++);
			for(i=0; i<4; i++, l++) marca[i]=mayuscula(*l);
			marca[i]='\0';

			/* si es marca de proceso tambi‚n recoge n£mero */
			if(!strcmp(marca,Mpro)) {
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

beep();

fclose(fbd);

return(0);
}

/****************************************************************************
	LEE_AYUDA: lee el fichero de ayuda.
	  Salida:       0 si hubo error, 1 si no
****************************************************************************/
int lee_ayuda(void)
{
FILE *f_ayuda;
char nf_ayuda[_MAX_PATH], *pay;
int i;

/* reserva buffer para cargar texto de ayuda, y lo carga */
buff_ayuda=(char *)malloc((size_t)BUFF_AYUDA);
if(buff_ayuda==NULL) return(0);

/* abre y lee fichero de ayuda */
strcpy(nf_ayuda,dir_sintac);
strcat(nf_ayuda,NF_AYUDA);

if((f_ayuda=fopen(nf_ayuda,"rt"))==NULL) {
	free(buff_ayuda);
	buff_ayuda=NULL;
	return(0);
}

pay=buff_ayuda;
do {
	*pay++=(char)fgetc(f_ayuda);
} while(!feof(f_ayuda));

/* marca fin de texto de ayuda */
*(pay-1)=CAY_FIN;

fclose(f_ayuda);

/* crea lista de nombres y punteros a inicio de apartados */
for(pay=buff_ayuda, n_aptdos=0; *pay!=CAY_FIN; pay++) {
	if(*pay==CAY_APTDO) {
		/* guarda puntero y nombre de apartado */
		apartado[n_aptdos]=pay;
		for(pay++, i=0; ((i<(MAXL_APTDO-1)) && (*pay!=' ') &&
		  (*pay!='\n')); pay++, i++) n_apartado[n_aptdos][i]=*pay;
		n_apartado[n_aptdos][i]='\0';

		/* siguiente apartado */
		n_aptdos++;
	}
}

return(1);
}

/****************************************************************************
	AYUDA: sistema de ayuda en l¡nea.
	  Salida:       0 si hubo error, 1 si no
****************************************************************************/
int ayuda(void)
{
char *p, *lin;
int i, j, aptdo, lin_act, n_lin, l_cur, c_cur;
char *lin_aptdo, ref[MAXL_APTDO], c;
unsigned tecla;
BOOLEAN encontrado;

if(buff_ayuda==NULL) {
	beep();
	return(0);
}

/* reserva buffer para guardar l¡neas de apartado actual antes */
/* de imprimir en ventana */
lin_aptdo=(char *)malloc((size_t)(NLIN_APTDO*LLIN_APTDO));
if(lin_aptdo==NULL) return(0);

/* inicializa apartado actual */
aptdo=0;

while(1) {
	/* copia a buffer apartado actual */
	p=apartado[aptdo];
	lin=lin_aptdo;
	n_lin=0;
	while(*p!=CAY_FINAPTDO) {
		/* guarda una l¡nea */
		for(i=0; ((i<LLIN_APTDO) && (*p!='\n')); i++, lin++, p++) {
			*lin=*p;
		}
		/* rellena con espacios hasta completar una l¡nea */
		for(; i<LLIN_APTDO; i++, lin++) {
			*lin=' ';
		}

		/* incrementa n£mero de l¡neas del apartado */
		n_lin++;

		/* salta el '\n' final de la anterior l¡nea */
		p++;
	}

	/* imprime apartado actual */
	dibuja_ventana(FIL_AY,COL_AY,ANCH_AY,ALT_AY,color_ay);
	imp_linea_scr(FIL_AY,COL_AY+((ANCH_AY-7)/2),color_ay," AYUDA ",7);
	imp_linea_scr(FIL_AY+ALT_AY-1,COL_AY+((ANCH_AY-56)/2),color_ay,
	  " \x1b\x1a\x18\x19 PgAr PgAb - mover ú RETURN - selecc."
	  "  ESC - salir ",56);

	/* inicializa l¡nea actual (la que se presenta la primera en la */
	/* ventana de ayuda) y posici¢n de cursor (respecto origen ventana) */
	lin_act=0;
	l_cur=0;
	c_cur=0;
	encontrado=FALSE;

	while(encontrado==FALSE) {
		/* imprime desde l¡nea actual hasta final de ventana */
		/* o hasta imprimir todas la l¡neas del apartado */
		for(i=0; ((i<(ALT_AY-2)) && (i<n_lin)); i++) {
			/* si es una referencia, la imprime como */
			/* cabecera y salta a siguiente */
			c=*(lin_aptdo+((lin_act+i)*LLIN_APTDO));
			if(c==CAY_APTDO) {
				imp_linea_scr(i+FIL_AY+1,COL_AY+1,color_aptdo,
				  lin_aptdo+((lin_act+i)*LLIN_APTDO)+1,
				  ANCH_AY-2);
				continue;
			}
			/* comprueba si la l¡nea tiene referencia a */
			/* apartado y la omite */
			for(j=0; j<LLIN_APTDO; j++) {
				c=*(lin_aptdo+((lin_act+i)*LLIN_APTDO)+j);
				if(c==CAY_REF) break;
			}
			if(j==LLIN_APTDO) imp_linea_scr(i+FIL_AY+1,COL_AY+1,
			  color_ay,lin_aptdo+((lin_act+i)*LLIN_APTDO),
			  ANCH_AY-2);
			else {
				imp_linea_scr(i+FIL_AY+1,COL_AY+1,color_ref,
				  lin_aptdo+((lin_act+i)*LLIN_APTDO),j);
				imp_linea_scr(i+FIL_AY+1,COL_AY+1+j,color_ay,
				  "",ANCH_AY-2-j);
			}
		}

		/* imprime cursor y espera tecla */
		pon_cursor((BYTE)(l_cur+FIL_AY+1),(BYTE)(c_cur+COL_AY+1));

		/* espera tecla */
		tecla=_bios_keybrd(_KEYBRD_READ);

		switch(tecla >> 8) {
			case TCUR_ARR :
				if(l_cur>0) l_cur--;
				else if(lin_act>0) lin_act--;
				break;
			case TCUR_ABJ :
				if(l_cur<(ALT_AY-3)) l_cur++;
				else if(lin_act<(n_lin-(ALT_AY-2)))
				  lin_act++;
				break;
			case TCUR_IZQ :
				if(c_cur>0) c_cur--;
				break;
			case TCUR_DER :
				if(c_cur<(ANCH_AY-3)) c_cur++;
				break;
			case TPAG_ARR :
				lin_act-=(ALT_AY-2);
				if(lin_act<0) lin_act=0;
				break;
			case TPAG_ABJ :
				if(n_lin<(ALT_AY-2)) break;
				lin_act+=(ALT_AY-2);
				if(lin_act>(n_lin-(ALT_AY-2)))
				  lin_act=(n_lin-(ALT_AY-2));
				break;
			case TESC :
				/* libera memoria */
				free(lin_aptdo);
				return(1);
				break;
		}

		/* si se pulsa RETURN, comprueba si hay referencia a apartado */
		if((tecla & 0x00ff)==RETURN) {
			ref[0]='\0';
			for(i=0, lin=lin_aptdo+((lin_act+l_cur)*LLIN_APTDO);
			  i<LLIN_APTDO; i++, lin++) {
				/* si hay referencia la recoge */
				if(*lin==CAY_REF) {
					/* salta marca de referencia */
					lin++;
					/* recoge referencia */
					for(j=0; ((j<(MAXL_APTDO-1)) &&
					  (*lin!=' ')); j++, lin++)
					  ref[j]=*lin;
					ref[j]='\0';
					break;
				}
			}
			/* busca referencia y si la encuentra pasa a la */
			/* secci¢n correspondiente */
			for(i=0; i<n_aptdos; i++) {
				if(!strcmp(ref,n_apartado[i])) {
					aptdo=i;
					encontrado=TRUE;
					break;
				}
			}
		}
	}
}

}
