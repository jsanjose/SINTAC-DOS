/****************************************************************************
			 INTERPRETE-DEBUGGER SINTAC
			    (c)1993 JSJ Soft Ltd.

	NOTA: mediante la constante DEBUGGER se controla si se genera
	  el c¢digo del int‚rprete o del int‚rprete-debugger, mediante
	  la constante RUNTIME se controla si se genera m¢dulo "runtime".
	  Con: DEBUGGER=0 se genera el c¢digo del int‚rprete-debugger
	       DEBUGGER=1 se genera el c¢digo del int‚rprete
	       RUNTIME=0 genera c¢digo normal
	       RUNTIME=1 se genera m¢dulo "rutime"
	  El valor de estas constantes se pueden modificar usando el men£
	  Options|Make|Compiler Flags|Defines.
****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <time.h>
#include <dos.h>
#include <bios.h>
#include <graph.h>
#include "version.h"
#include "sintac.h"
#include "wing.h"
#include "condacto.h"
#include "ds.h"

#if DEBUGGER==1
	#pragma message("Int‚rprete-debugger...")
#else
	#pragma message("Int‚rprete...")
#endif

#if RUNTIME==1
	#pragma message("M¢dulo 'runtime'...")
#endif

/*** Variables externas ***/
extern BYTE loc_obj[MAX_OBJ];   /* tabla de localidades act. de objetos */
extern BYTE objs_cogidos;       /* n£mero de objetos cogidos */
extern STC_WINDOW w[N_VENT];    /* tabla para guardar par metros de ventanas */
extern int ptrp;                /* puntero de pila */
extern STC_BANCORAM ram[BANCOS_RAM];    /* para RAMSAVE y RAMLOAD */
extern STC_CONDACTO cd[];       /* tabla de funci¢n-tipo condactos */

/*** Variables globales ***/
#if DEBUGGER==1 && RUNTIME==0
#include "tabcond.h"            /* tabla de nombre-tipo de condactos */
BOOLEAN entorno=FALSE;          /* indica si se ejecuta desde entorno */
STC_WINDOW w_jsj;               /* ventana de presentaci¢n */
STC_WINDOW w_deb;               /* ventana de debugger */
BOOLEAN debugg=TRUE;            /* TRUE si paso a paso activado */
BOOLEAN pra_lin=FALSE;          /* TRUE si en primera l¡nea de una entrada */
char _huge *img_debug;          /* puntero buffer para fondo ventana debug. */
#endif

#if RUNTIME==1
long lng_runtime=0;             /* longitud (bytes) de m¢dulo 'runtime' */
#endif

/* cabecera de fichero de base de datos */
CAB_SINTAC cab;

/* nombre de fichero de base de datos */
char nf_base_datos[_MAX_PATH];

/* variables para Vocabulario */
struct palabra vocabulario[NUM_PAL];    /* para almacenar vocabulario */

/* variables para Mensajes de Sistema */
unsigned tab_desp_msy[MAX_MSY];         /* tabla de desplaz. mens. sist. */
char *tab_msy;                          /* puntero a inicio zona mens. sist. */

/* variables para Mensajes */
unsigned tab_desp_msg[MAX_MSG];         /* tabla de desplaz. de mensajes */
char *tab_msg;                          /* puntero a inicio zona de mensajes */
BYTE tabla_msg=0;                       /* tabla de mensajes cargada */

/* variables para Localidades */
unsigned tab_desp_loc[MAX_LOC];         /* tabla desplaz. textos de locs. */
char *tab_loc;                          /* puntero a inicio de texto de locs. */
/* variables para Conexiones */
unsigned tab_desp_conx[MAX_LOC];        /* tabla desplaz. lista conexiones */
char *tab_conx;                         /* puntero inicio zona de conexiones */

/* variables para Objetos */
unsigned tab_desp_obj[MAX_OBJ];         /* tabla de desplaz.lista de objetos */
char *tab_obj;                          /* puntero a inicio zona de objetos */

/* variables para Procesos */
unsigned tab_desp_pro[MAX_PRO];         /* tabla desplazamiento de procesos */
BYTE *tab_pro;                          /* puntero a inicio zona de procesos */

STC_WINDOW w_err;               /* ventana para mensajes de error */
BYTE var[VARS];                 /* variables del sistema (8 bits) */
BYTE flag[BANDS];               /* banderas del sistema, 256 banderas */
BYTE pro_act;                   /* n£mero de proceso actual */
BYTE *ptr_proc;                 /* puntero auxiliar */
unsigned sgte_ent;              /* desplazamiento de sgte. entrada */
BOOLEAN resp_act;               /* RESP (=1) o NORESP (=0) */
BOOLEAN nueva_ent;              /* indica que no debe ajustar ptr_proc para */
				/* saltar a siguiente entrada */

BYTE ruptura;                   /* indicador de ruptura (BREAK) */
struct videoconfig vid;         /* informaci¢n del sistema de video */

/* tablas con las definiciones de los caracteres */
unsigned char _far cardef8x16[256][16];
unsigned char _far cardef8x8[256][8];

/*** Programa principal ***/
void main(int argc, char *argv[])
{
#if DEBUGGER==1
BYTE lin_deb=WDEB_FIL;
BYTE max_lindeb, *pro_d;
long tam_img_debug;
#endif
BYTE i, indir, msc_indir, ncondacto, par[8], npar;
BOOLEAN res_pro;

/* establece modo de pantalla */
modo_video(0);

#if RUNTIME==0
if(argc<2) m_err(7,"Falta nombre de fichero",1);
#endif

#if DEBUGGER==1
/* detr s del nombre del fichero espera /lxx o /Lxx (o -lxx o -Lxx) */
if(argc==3) {
	/* si introdujo /l o /L (o -l o -L) recoge los dos siguientes */
	/* d¡gitos y calcula l¡nea de la ventana del debugger */
	if(((argv[2][0]=='/') || (argv[2][0]=='-')) && ((argv[2][1]=='l') ||
	  (argv[2][1]=='L'))) {
		lin_deb=(BYTE)(((argv[2][2]-'0')*10)+(argv[2][3]-'0'));
		max_lindeb=(BYTE)(vid.numtextrows-WDEB_ALTO);
		if(lin_deb>max_lindeb) lin_deb=max_lindeb;
	}
}
/* como tercer par metro espera /e o /E (o -E o -e) */
if(argc==4) {
	if(((argv[3][0]=='/') || (argv[3][0]=='-')) && ((argv[3][1]=='e') ||
	  (argv[3][1]=='E'))) entorno=TRUE;
}

/* crea la ventana del debugger */
w_crea(lin_deb,WDEB_COL,WDEB_ANCHO,WDEB_ALTO,WDEB_COLORF,WDEB_COLOR,NO_BORDE,
  &w_deb);

/* reserva buffer para guardar fondo */
tam_img_debug=_imagesize(0,0,(w_deb.lx*8)-1,(w_deb.ly*w_deb.chralt)-1);
img_debug=halloc(tam_img_debug,1);

#endif

/* instala 'handler' de errores cr¡ticos */
_harderr(int24_hnd);

/* carga base de datos e inicializa variables */
cls();

#if RUNTIME==0
carga_bd(argv[1]);
#else
carga_bd(argv[0]);
#endif

inic();

_asm {
	mov ax,3300h            ; obtener indicador de ruptura
	int 21h
	mov ruptura,dl          ; guarda indicador de ruptura
	mov ax,3301h            ; establecer indicador de ruptura
	mov dl,0                ; lo desactiva
	int 21h
}

#if DEBUGGER==1
/* presentaci¢n */
cls();
if(entorno==FALSE) {
	w_crea(WJSJ_FIL,WJSJ_COL,WJSJ_ANCHO,WJSJ_ALTO,1,15,BORDE_3,&w_jsj);
	/* centra ventana */
	w_jsj.wx=(BYTE)((vid.numtextcols-w_jsj.lx)/2);
	w_jsj.wxi=(BYTE)(w_jsj.wx+1);
	w_cls(&w_jsj);
	w_imps("\n  Int‚rprete-Debugger versi¢n "VERSION_IS"\n"
	       "\n  "COPYRIGHT"\n\n"
	       "\n           Pulsa una tecla",&w_jsj);
	w_lee_tecla();
	cls();
}
#endif

/* inicializa puntero a proceso actual */
ptr_proc=tab_pro+tab_desp_pro[pro_act];

while(1) {
	/* si no es fin de proceso */
	if(*ptr_proc) {
		/* si 'res_pro' es FALSE no debe ejecutar entrada */
		res_pro=!resp_act || (resp_act && ((*ptr_proc==NO_PAL) ||
		  (*ptr_proc==var[2])) && ((*(ptr_proc+1)==NO_PAL) ||
		  (*(ptr_proc+1)==var[3])));

		/* salta verbo-nombre */
		ptr_proc+=2;

		/* calcula el desplazamiento de la siguiente entrada */
		sgte_ent=(*(ptr_proc+1) << 8) | *ptr_proc;
		ptr_proc+=2;

		#if DEBUGGER==1
		/* indica que es primera l¡nea de entrada */
		pra_lin=TRUE;
		#endif

	}
	/* si fin de proceso */
	else {
		res_pro=done();
		ptr_proc++;     /* ajustamos ptr_proc */

		#if DEBUGGER==1
		pra_lin=FALSE;  /* no es primera l¡nea de entrada */
		#endif

	}
	if(nueva_ent==TRUE) nueva_ent=FALSE;
	/* si res_pro es TRUE y no fin de entrada ejecuta esta entrada */
	/* si no, salta a la siguiente */
	while(res_pro && *ptr_proc) {
		#if DEBUGGER==1
		/* guarda direcci¢n de condacto en curso */
		pro_d=ptr_proc;
		#endif

		/* si hay indirecci¢n */
		if(*ptr_proc==INDIR) {
			ptr_proc++;     /* salta prefijo indirecci¢n */
			indir=*ptr_proc++;
		}
		else indir=0;

		ncondacto=*ptr_proc;
		npar=cd[ncondacto].npar;

		msc_indir=0x01;
		for(i=0; i<npar; i++) {
			if(indir & msc_indir) par[i]=var[*(ptr_proc+i+1)];
			else par[i]=*(ptr_proc+i+1);
			msc_indir <<= 1;
		}

		#if DEBUGGER==1
		/* si est  activado el paso a paso */
		if(debugg==TRUE) debugger(indir,npar,pro_d);
		/* sino, activa paso a paso */
		else if(_bios_keybrd(_KEYBRD_READY)==F10) {
			_bios_keybrd(_KEYBRD_READ);
			debugg=TRUE;
			/* indicamos que no es primera l¡nea de entrada para */
			/* que imp_condacto() no imprima el verbo-nombre ya */
			/* que el puntero al condacto no est  ajustado */
			pra_lin=FALSE;
		}
		#endif

		/* ejecuta condacto seg£n n£mero de par metros */
		switch(npar) {
			case 0 :
				res_pro=cd[ncondacto].cond();
				ptr_proc++;
				break;
			case 1 :
				res_pro=cd[ncondacto].cond(par[0]);
				ptr_proc+=2;
				break;
			case 2 :
				res_pro=cd[ncondacto].cond(par[0],par[1]);
				ptr_proc+=3;
				break;
			case 3 :
				res_pro=cd[ncondacto].cond(par[0],par[1],
				  par[2]);
				ptr_proc+=4;
				break;
			case 4 :
				res_pro=cd[ncondacto].cond(par[0],par[1],
				  par[2],par[3]);
				ptr_proc+=5;
				break;
			case 7 :
				res_pro=cd[ncondacto].cond(par[0],par[1],
				  par[2],par[3],par[4],par[5],par[6]);
				ptr_proc+=8;
				break;
		}
	}
	/* si fin entrada, pasa a la siguiente */
	if(!*ptr_proc) ptr_proc++;
	else if(nueva_ent==FALSE) {
		ptr_proc=tab_pro+tab_desp_pro[pro_act]+sgte_ent;
	}
}

}

/****************************************************************************
	INT24_HND: rutina de manejo de errores cr¡ticos de hardware.
****************************************************************************/
void _far int24_hnd(unsigned deverror, unsigned errcode,
  unsigned _far *devhdr)
{

_hardresume(_HARDERR_FAIL);

}

#if DEBUGGER==1
/****************************************************************************
	SACA_PAL: devuelve el n£mero de la primera entrada en el vocabulario
	  que se corresponda con el n£mero y tipo de palabra dado.
	  Entrada:      'num_pal' n£mero de la palabra a buscar
			'tipo_pal' tipo de la palabra a buscar
	  Salida:       n£mero dentro de la tabla de vocabulario o
			(NUM_PAL+1) si no se encontr¢
****************************************************************************/
int saca_pal(BYTE num_pal, BYTE tipo_pal)
{
int i;

for(i=0; i<cab.pal_voc; i++) {
	if((vocabulario[i].num==num_pal) && (vocabulario[i].tipo==tipo_pal))
	  return(i);
}

return(NUM_PAL+1);      /* n£mero de palabra no existe */
}

/****************************************************************************
	IMP_CONDACTO: imprime condacto en curso en la ventana de debug.
	  Entrada:      'indir' indicadores de indirecci¢n
			'npar' n£mero de par metros
			'pro_d' direcci¢n del condacto en curso
		      variables globales:-
			'pra_lin' TRUE si es 1era l¡nea de entrada
			'ptr_proc' puntero a byte de condacto + par metros
****************************************************************************/
void imp_condacto(BYTE indir, BYTE npar, BYTE *pro_d)
{
BYTE i, *pcp, msc_indir;
int j, dirrel;
unsigned dir;
char lin_cond[LNG_LINDEB+1], par[7];
char *Pal_Nula="-     ";

/* si es la primera l¡nea de la entrada pone el puntero apuntando al */
/* campo verbo, si no apunta al condacto */
if(pra_lin==TRUE) dir=(unsigned)(pro_d-tab_pro-4);
else dir=(unsigned)(pro_d-tab_pro);

/* imprime direcci¢n del condacto */
sprintf(lin_cond,"%5u: ",dir);
w_imps(lin_cond,&w_deb);

if(pra_lin==TRUE) {
	pcp=ptr_proc-4;                 /* apunta a verbo-nombre */
	if(indir) pcp-=2;               /* ajuste por indirecci¢n */
	if(*pcp==NO_PAL) sprintf(lin_cond,"%s  ",Pal_Nula);
	else {
		j=saca_pal(*pcp,_VERB);
		/* si no es verbo, quiz  sea nombre */
		if(j==(NUM_PAL+1)) j=saca_pal(*pcp,_NOMB);
		sprintf(lin_cond,"%s  ",vocabulario[j].p);
	}
	w_imps(lin_cond,&w_deb);
	pcp++;

	if(*pcp==NO_PAL) sprintf(lin_cond,"%s",Pal_Nula);
	else sprintf(lin_cond,"%s",vocabulario[saca_pal(*pcp,_NOMB)].p);
	w_imps(lin_cond,&w_deb);
}

/* siguiente l¡nea de ventana */
w_deb.cwx=1;
w_deb.cwy++;

/* imprime condacto */
sprintf(lin_cond,"%s ",condacto[*ptr_proc].cnd);
w_imps(lin_cond,&w_deb);

/* imprime par metros seg£n tipo de condacto */
switch(condacto[*ptr_proc].tipo) {
	case 0 :
		sprintf(lin_cond," ");
		break;
	case 11 :
		dirrel=(*(ptr_proc+2) << 8) | *(ptr_proc+1);
		if(pra_lin==TRUE) dirrel+=4;
		sprintf(lin_cond,"%5i",dirrel);
		break;
	case 13 :
		sprintf(lin_cond,"%s",
		  vocabulario[saca_pal(*(ptr_proc+1),_ADJT)].p);
		break;
	case 14 :
		sprintf(lin_cond,"%s",
		  vocabulario[saca_pal(*(ptr_proc+1),_NOMB)].p);
		break;
	case 16 :
		if(*(ptr_proc+1)==NO_PAL) sprintf(lin_cond,"%s ",Pal_Nula);
		else {
			j=saca_pal(*(ptr_proc+1),_VERB);
			/* si no es vebro, quiz  sea nombre convertible */
			if(j==(NUM_PAL+1)) j=saca_pal(*(ptr_proc+1),_NOMB);
			sprintf(lin_cond,"%s ",vocabulario[j].p);
		}
		w_imps(lin_cond,&w_deb);
		if(*(ptr_proc+2)==NO_PAL) sprintf(lin_cond,"%s",Pal_Nula);
		else sprintf(lin_cond,"%s",vocabulario[saca_pal(*(ptr_proc+2),
		  _NOMB)].p);
		break;
	default :
		*lin_cond='\0';
		msc_indir=0x01;
		for(i=0; i<npar; i++) {
			if(indir & msc_indir) sprintf(par,"[%u] ",
			  *(ptr_proc+i+1));
			else sprintf(par,"%u ",*(ptr_proc+i+1));
			strcat(lin_cond,par);
			msc_indir <<= 1;
		}
		break;
}

/* imprime par metros */
w_imps(lin_cond,&w_deb);

/* si estaba en primera l¡nea de entrada indica que ya no est  */
pra_lin=FALSE;

}

/****************************************************************************
	INP_BAND: rutina de introducci¢n por teclado de n£meros para debugger.
	  Salida:       n£mero introducido en el rango 0-255 (BYTE).
****************************************************************************/
BYTE inp_band(void)
{
unsigned k;
char numero[4];
int i;
BYTE oldcwx;

/* guarda antigua posici¢n cursor */
oldcwx=w_deb.cwx;

/* repite hasta que sea un n£mero v lido */
do {
	i=0;
	/* restaura posici¢n cursor */
	w_deb.cwx=oldcwx;

	/* repite mientras no introduzca 3 d¡gitos o no pulse RETURN */
	do {
		w_impc(CUR_DEBUG,&w_deb);
		w_deb.cwx--;
		k=w_lee_tecla();
		if((k>='0') && (k<='9')) {
			numero[i]=(char)k;
			i++;
			w_impc((char)k,&w_deb);
		}
	} while((i<3) && (k!=RETURN));
	numero[i]='\0';

	/* pasa cadena ASCII a n£mero */
	i=atoi(numero);
} while((i<0) || (i>255));

return((BYTE)i);
}

/****************************************************************************
	IMP_VARBAND: imprime en la l¡nea del debugger la variable y bandera
	  actuales.
	  Entrada:      'variable' n£mero de variable a imprimir
			'bandera' n£mero de bandera a imprimir
****************************************************************************/
void imp_varband(BYTE variable, BYTE bandera)
{
char lin_deb[LNG_LINDEB+1];
int palabra;

/* imprime variable */
sprintf(lin_deb,"Var %3u=%3u       ",variable,var[variable]);
/* posiciona cursor */
w_deb.cwx=9;
w_deb.cwy=0;
w_imps(lin_deb,&w_deb);

/* si es una variable de sentencia l¢gica imprime adem s la */
/* palabra del vocabulario correspondiente */
if(variable==2) {
	palabra=saca_pal(var[2],_VERB);
	/* puede ser nombre convertible */
	if(palabra==(NUM_PAL+1)) palabra=saca_pal(var[2],_NOMB);
}
else if(variable==3) palabra=saca_pal(var[3],_NOMB);
else if(variable==4) palabra=saca_pal(var[4],_ADJT);
else if(variable==5) palabra=saca_pal(var[5],_NOMB);
else if(variable==6) palabra=saca_pal(var[6],_ADJT);
else palabra=NUM_PAL+1;

if(palabra==(NUM_PAL+1)) sprintf(lin_deb,"------");
else sprintf(lin_deb,"%s",vocabulario[palabra].p);

/* posiciona cursor e imprime palabra de vocabulario */
w_deb.cwx=21;
w_imps(lin_deb,&w_deb);

/* imprime bandera */
sprintf(lin_deb,"Band %3u=%1u",bandera,notzero(bandera));
w_deb.cwx=28;
w_deb.cwy=0;
w_imps(lin_deb,&w_deb);

}

/****************************************************************************
	GUARDA_DEBUGGER: guarda la zona de pantalla sobreimpresa por la
	  ventana del debugger.
****************************************************************************/
void guarda_debugger(void)
{

if(img_debug!=NULL) _getimage(w_deb.wx*8,w_deb.wy*w_deb.chralt,
  ((w_deb.wx+w_deb.lx)*8)-1,((w_deb.wy+w_deb.ly)*w_deb.chralt)-1,img_debug);

}

/****************************************************************************
	RECUPERA_DEBUGGER: recupera la zona de pantalla sobreimpresa por la
	  ventana del debugger.
****************************************************************************/
void recupera_debugger(void)
{

if(img_debug!=NULL) _putimage(w_deb.wx*8,w_deb.wy*w_deb.chralt,img_debug,
  _GPSET);

}

/****************************************************************************
	IMP_DEBUGGER: imprime la ventana del debugger.
	  Entrada:      'indir' indicadores de indirecci¢n
			'npar' n£mero de par metros
			'pro_d' direcci¢n del condacto en curso
			'variable', 'bandera' n£mero de variable y bandera
			que se mostrar n
			'txt_deb' texto a imprimir en la £ltima l¡nea
****************************************************************************/
void imp_debugger(BYTE indir, BYTE npar, BYTE *pro_d, BYTE variable,
  BYTE bandera, char *txt_deb)
{
char lin_deb[LNG_LINDEB+1];

/* borra la ventana del debugger e imprime informaci¢n */
w_cls(&w_deb);

/* dibuja laterales de la ventana */
_setcolor(WDEB_COLOR);
_moveto(w_deb.wx*8,w_deb.wy*w_deb.chralt);
_lineto(w_deb.wx*8,((w_deb.wy+w_deb.ly)*w_deb.chralt)-1);
_moveto(((w_deb.wx+w_deb.lx)*8)-1,w_deb.wy*w_deb.chralt);
_lineto(((w_deb.wx+w_deb.lx)*8)-1,((w_deb.wy+w_deb.ly)*w_deb.chralt)-1);

w_deb.cwx=1;
w_deb.cwy=0;
sprintf(lin_deb,"PRO %3u",(unsigned)pro_act);
w_imps(lin_deb,&w_deb);
imp_varband(variable,bandera);

w_deb.cwx=1;
w_deb.cwy=1;
imp_condacto(indir,npar,pro_d);

w_deb.cwx=1;
w_deb.cwy=3;
w_imps(txt_deb,&w_deb);

}

/****************************************************************************
	DEBUGGER: funci¢n principal del debugger.
	  Entrada:      'indir' indicadores de indirecci¢n
			'npar' n£mero de par metros
			'pro_d' direcci¢n del condacto en curso
****************************************************************************/
void debugger(BYTE indir, BYTE npar, BYTE *pro_d)
{
unsigned key;
static BYTE variable=0;
static BYTE bandera=0;
BYTE valor;
char lin_deb[LNG_LINDEB+1];
char *txt_deb1="Var.  Band.  Pant.  Desact.  Salir";
char *txt_deb2="Otra  Modificar  Fin              ";

guarda_debugger();

/* imprime ventana del debugger */
imp_debugger(indir,npar,pro_d,variable,bandera,txt_deb1);

key=mayuscula((char)w_lee_tecla());

while(esta_en("VBPDS",(char)key)) {
	switch(key) {
		case 'V' :      /* variables */
			w_deb.cwx=1;
			w_deb.cwy=3;
			w_imps(txt_deb2,&w_deb);
			do {
				imp_varband(variable,bandera);
				key=mayuscula((char)w_lee_tecla());
				switch((BYTE)key) {
				case 'M' :              /* modificar */
					w_deb.cwx=17;
					w_deb.cwy=0;
					valor=inp_band();
					var[variable]=valor;
					break;
				case 'O' :              /* otra variable */
					w_deb.cwx=13;
					w_deb.cwy=0;
					variable=inp_band();
					break;
				case COD_ARR :
					variable--;
					break;
				case COD_ABJ :
					variable++;
					break;
				}
			} while(key!='F');
			break;
		case 'B' :      /* banderas */
			w_deb.cwx=1;
			w_deb.cwy=3;
			w_imps(txt_deb2,&w_deb);
			do {
				imp_varband(variable,bandera);
				key=mayuscula((char)w_lee_tecla());
				switch((BYTE)key) {
				case 'M' :              /* modificar */
					w_deb.cwx=37;
					w_deb.cwy=0;
					w_impc(CUR_DEBUG,&w_deb);
					do {
						key=w_lee_tecla();
					} while((key!='0') && (key!='1'));
					if(key=='0') clear(bandera);
					else set(bandera);
					break;
				case 'O' :      /* otra bandera */
					w_deb.cwx=33;
					w_deb.cwy=0;
					bandera=inp_band();
					break;
				case COD_ARR :
					bandera--;
					break;
				case COD_ABJ :
					bandera++;
					break;
				}
			} while(key!='F');
			break;
		case 'P' :      /* pantalla */
			recupera_debugger();
			w_lee_tecla();
			imp_debugger(indir,npar,pro_d,variable,bandera,
			  txt_deb1);
			break;
		case 'D' :      /* desactivar */
			/* desactiva paso a paso */
			debugg=FALSE;
			recupera_debugger();
			return;
		case 'S' :      /* salir */
			m_err(0,"",2);
			break;
	}
	w_deb.cwx=1;
	w_deb.cwy=3;
	w_imps(txt_deb1,&w_deb);
	key=mayuscula((char)w_lee_tecla());
}

recupera_debugger();

}

#endif

/****************************************************************************
	MODO_VIDEO: establece el modo de pantalla y almacena informaci¢n
	  sobre el sistema de v¡deo.
	  Entrada:      'modo' modo a seleccionar, 0 para 640x480x16, 1 para
			320x200x256
	  Salida:       variable global 'vid' con informaci¢n sobre sistema
			de v¡deo
****************************************************************************/
void modo_video(int modo)
{

/* coge informaci¢n del sistema de v¡deo */
_getvideoconfig(&vid);

/* si no tiene tarjeta VGA, sale con error */
if(vid.adapter!=_VGA) {
	fprintf(stderr,"\nEste programa requiere tarjeta gr fica VGA.\n");
	exit(1);
}

/* selecciona modo de v¡deo y actualiza variables 14 y 15 */
if(modo==0) {
	/* modo de 640x480x16, 80 columnas, 30 filas */
	_setvideomode(_VRES16COLOR);
	var[14]=MODO0_COL;
	var[15]=MODO0_FIL;
	clear(5);
}
else {
	/* modo de 320x200x256, 40 columnas, 25 filas */
	_setvideomode(_MRES256COLOR);
	var[14]=MODO1_COL;
	var[15]=MODO1_FIL;
	set(5);
}

/* actualiza informaci¢n sistema de v¡deo de acuerdo al modo seleccionado */
_getvideoconfig(&vid);

}

#if RUNTIME==0
/****************************************************************************
	CARGA_BD: carga la base de datos.
	  Entrada:      'nombre' nombre de fichero de base de datos
****************************************************************************/
void carga_bd(char *nombre)
{
FILE *fbd;
char *errmem="No hay suficiente memoria";
char *srecon=SRECON;
unsigned bytes_msg;
int i;

if((fbd=fopen(nombre,"rb"))==NULL)
  m_err(1,"Error de apertura fichero de entrada",1);

/* guarda nombre de fichero de base de datos */
strcpy(nf_base_datos,nombre);

/* lee cabecera */
frd(fbd,&cab,sizeof(CAB_SINTAC),1);

/* comprueba que la versi¢n de la base de datos sea correcta */
if((cab.srecon[L_RECON-2]!=srecon[L_RECON-2]) ||
  (cab.srecon[L_RECON-1]!=srecon[L_RECON-1]))
  m_err(5,"Fichero de entrada no v lido",1);

/* Reserva de memoria para las distintas secciones */
/* Mensajes del Sistema */
if((tab_msy=(char *)malloc((size_t)cab.bytes_msy))==NULL) m_err(6,errmem,1);

/* Mensajes */
/* reserva memoria para tabla de mensajes m s grande */
bytes_msg=0;
for(i=0; i<MAX_TMSG; i++) {
	if(cab.fpos_msg[i]!=(fpos_t)0) {
		if(cab.bytes_msg[i]>bytes_msg) bytes_msg=cab.bytes_msg[i];
	}
}
if((tab_msg=(char *)malloc((size_t)bytes_msg))==NULL) m_err(6,errmem,1);

/* Localidades */
if((tab_loc=(char *)malloc((size_t)cab.bytes_loc))==NULL) m_err(6,errmem,1);
/* Conexiones */
if((tab_conx=(char *)malloc((size_t)cab.bytes_conx))==NULL) m_err(6,errmem,1);

/* Objetos */
if((tab_obj=(char *)malloc((size_t)cab.bytes_obj))==NULL) m_err(6,errmem,1);

/* Procesos */
if((tab_pro=(BYTE *)malloc((size_t)cab.bytes_pro))==NULL) m_err(6,errmem,1);

fseek(fbd,cab.fpos_voc,SEEK_SET);
frd(fbd,vocabulario,sizeof(struct palabra),cab.pal_voc);

fseek(fbd,cab.fpos_msy,SEEK_SET);
frd(fbd,tab_desp_msy,sizeof(unsigned),(size_t)MAX_MSY);
frd(fbd,tab_msy,sizeof(char),cab.bytes_msy);

/* busca primera tabla de mensajes disponible y la carga */
for(i=0; i<MAX_TMSG; i++) if(cab.fpos_msg[i]!=(fpos_t)0) break;
tabla_msg=(BYTE)i;
fseek(fbd,cab.fpos_msg[i],SEEK_SET);
frd(fbd,tab_desp_msg,sizeof(unsigned),(size_t)MAX_MSG);
frd(fbd,tab_msg,sizeof(char),cab.bytes_msg[i]);

fseek(fbd,cab.fpos_loc,SEEK_SET);
frd(fbd,tab_desp_loc,sizeof(unsigned),(size_t)MAX_LOC);
frd(fbd,tab_loc,sizeof(char),cab.bytes_loc);
frd(fbd,tab_desp_conx,sizeof(unsigned),(size_t)MAX_LOC);
frd(fbd,tab_conx,sizeof(BYTE),cab.bytes_conx);

fseek(fbd,cab.fpos_obj,SEEK_SET);
frd(fbd,tab_desp_obj,sizeof(unsigned),(size_t)MAX_OBJ);
frd(fbd,tab_obj,sizeof(char),cab.bytes_obj);

fseek(fbd,cab.fpos_pro,SEEK_SET);
frd(fbd,tab_desp_pro,sizeof(unsigned),(size_t)MAX_PRO);
frd(fbd,tab_pro,sizeof(BYTE),cab.bytes_pro);

fclose(fbd);

/* decodifica las secciones */
codifica((BYTE *)tab_msy,cab.bytes_msy);
codifica((BYTE *)tab_msg,cab.bytes_msg[tabla_msg]);
codifica((BYTE *)tab_loc,cab.bytes_loc);
codifica(tab_conx,cab.bytes_conx);
codifica((BYTE *)tab_obj,cab.bytes_obj);
codifica(tab_pro,cab.bytes_pro);

}
#else
/****************************************************************************
	CARGA_BD: carga la base de datos (m¢dulo runtime).
	  Entrada:      'nombre' nombre de fichero EXE con base de
			datos 'linkada'
****************************************************************************/
void carga_bd(char *nombre)
{
FILE *fbd;
char *errmem="No hay suficiente memoria";
char *srecon=SRECON;
unsigned bytes_msg;
long pos;
int i;

if((fbd=fopen(nombre,"rb"))==NULL)
  m_err(1,"Error de apertura fichero de entrada",1);

/* guarda nombre de fichero de base de datos */
strcpy(nf_base_datos,nombre);

/* al final de fichero 'linkado' (runtime+base de datos) debe estar */
/* la longitud de m¢dulo 'runtime' */
pos=0L-sizeof(long);
fseek(fbd,pos,SEEK_END);
frd(fbd,&lng_runtime,sizeof(long),1);

/* posiciona puntero de fichero en inicio de base de datos */
fseek(fbd,lng_runtime,SEEK_SET);

/* lee cabecera */
frd(fbd,&cab,sizeof(CAB_SINTAC),1);

/* comprueba que la versi¢n de la base de datos sea correcta */
if((cab.srecon[L_RECON-2]!=srecon[L_RECON-2]) ||
  (cab.srecon[L_RECON-1]!=srecon[L_RECON-1]))
  m_err(5,"Fichero de entrada no v lido",1);

/* Reserva de memoria para las distintas secciones */
/* Mensajes del Sistema */
if((tab_msy=(char *)malloc((size_t)cab.bytes_msy))==NULL) m_err(6,errmem,1);

/* Mensajes */
/* reserva memoria para tabla de mensajes m s grande */
bytes_msg=0;
for(i=0; i<MAX_TMSG; i++) {
	if(cab.fpos_msg[i]!=(fpos_t)0) {
		if(cab.bytes_msg[i]>bytes_msg) bytes_msg=cab.bytes_msg[i];
	}
}
if((tab_msg=(char *)malloc((size_t)bytes_msg))==NULL) m_err(6,errmem,1);

/* Localidades */
if((tab_loc=(char *)malloc((size_t)cab.bytes_loc))==NULL) m_err(6,errmem,1);
/* Conexiones */
if((tab_conx=(char *)malloc((size_t)cab.bytes_conx))==NULL) m_err(6,errmem,1);

/* Objetos */
if((tab_obj=(char *)malloc((size_t)cab.bytes_obj))==NULL) m_err(6,errmem,1);

/* Procesos */
if((tab_pro=(BYTE *)malloc((size_t)cab.bytes_pro))==NULL) m_err(6,errmem,1);

fseek(fbd,cab.fpos_voc+lng_runtime,SEEK_SET);
frd(fbd,vocabulario,sizeof(struct palabra),cab.pal_voc);

fseek(fbd,cab.fpos_msy+lng_runtime,SEEK_SET);
frd(fbd,tab_desp_msy,sizeof(unsigned),(size_t)MAX_MSY);
frd(fbd,tab_msy,sizeof(char),cab.bytes_msy);

/* busca primera tabla de mensajes disponible y la carga */
for(i=0; i<MAX_TMSG; i++) if(cab.fpos_msg[i]!=(fpos_t)0) break;
tabla_msg=(BYTE)i;
fseek(fbd,cab.fpos_msg[i]+lng_runtime,SEEK_SET);
frd(fbd,tab_desp_msg,sizeof(unsigned),(size_t)MAX_MSG);
frd(fbd,tab_msg,sizeof(char),cab.bytes_msg[i]);

fseek(fbd,cab.fpos_loc+lng_runtime,SEEK_SET);
frd(fbd,tab_desp_loc,sizeof(unsigned),(size_t)MAX_LOC);
frd(fbd,tab_loc,sizeof(char),cab.bytes_loc);
frd(fbd,tab_desp_conx,sizeof(unsigned),(size_t)MAX_LOC);
frd(fbd,tab_conx,sizeof(BYTE),cab.bytes_conx);

fseek(fbd,cab.fpos_obj+lng_runtime,SEEK_SET);
frd(fbd,tab_desp_obj,sizeof(unsigned),(size_t)MAX_OBJ);
frd(fbd,tab_obj,sizeof(char),cab.bytes_obj);

fseek(fbd,cab.fpos_pro+lng_runtime,SEEK_SET);
frd(fbd,tab_desp_pro,sizeof(unsigned),(size_t)MAX_PRO);
frd(fbd,tab_pro,sizeof(BYTE),cab.bytes_pro);

fclose(fbd);

/* decodifica las secciones */
codifica((BYTE *)tab_msy,cab.bytes_msy);
codifica((BYTE *)tab_msg,cab.bytes_msg[tabla_msg]);
codifica((BYTE *)tab_loc,cab.bytes_loc);
codifica(tab_conx,cab.bytes_conx);
codifica((BYTE *)tab_obj,cab.bytes_obj);
codifica(tab_pro,cab.bytes_pro);

}
#endif

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
	INIC: inicializa diversas tablas y variables.
	  Entrada:      variables globales:-
			  'vid' con informaci¢n de sistema de v¡deo
****************************************************************************/
void inic(void)
{
int i;
char *po;

/* n£mero de objetos cogidos */
objs_cogidos=0;

/* inicializa tabla de localidades actuales de los objetos */
for(i=0; i<(int)cab.num_obj; i++) {
	po=tab_obj+tab_desp_obj[i];
	/* coge localidad inicial y la guarda en tabla */
	loc_obj[i]=(BYTE)*(po+2);
	/* si lleva objeto de inicio, incrementa contador objs. cogidos */
	if((loc_obj[i]==PUESTO) || (loc_obj[i]==COGIDO)) objs_cogidos++;
}

/* inicializa variables */
for(i=0; i<VARS; i++) {
	/* variables de sentencia l¢gica inicializadas a NO_PAL */
	if((i>1) && (i<7)) var[i]=NO_PAL;
	else var[i]=0;
}

/* inicializa banderas */
for(i=0; i<BANDS; i++) flag[i]=0;

/* inicializa ventanas */
for(i=0; i<N_VENT; i++) w_crea(0,0,80,25,0,7,NO_BORDE,&w[i]);

/* inicializa bancos de RAMSAVE y RAMLOAD */
for(i=0; i<BANCOS_RAM; i++) ram[i].usado=FALSE;

pro_act=0;                      /* n£mero de proceso actual */
ptrp=0;                         /* puntero de pila */
resp_act=FALSE;                 /* NORESP */
nueva_ent=FALSE;

/* como siempre se deber  ejecutar en VGA pone a 1 la bandera 4 */
/* por compatibilidad con versiones anteriores */
set(4);

/* inicializa variables y banderas relacionadas con el sistema de v¡deo */
if(vid.mode==_VRES16COLOR) {
	/* modo de 640x480x16, 80 columnas, 30 filas */
	_setvideomode(_VRES16COLOR);
	var[14]=MODO0_COL;
	var[15]=MODO0_FIL;
	clear(5);
}
else {
	/* modo de 320x200x256, 40 columnas, 25 filas */
	_setvideomode(_MRES256COLOR);
	var[14]=MODO1_COL;
	var[15]=MODO1_FIL;
	set(5);
}

/* tabla de mensajes cargada inicialmente */
var[17]=tabla_msg;

}

/****************************************************************************
	FRD: controla la entrada de datos desde el fichero de entrada
	  mediante la funci¢n fread.
	  Entrada:      'fbd' puntero a fichero de base de datos
			'buff' puntero a buffer donde dejar datos leidos
			'tam' tama¤o de datos a leer
			'cant' cantidad de datos a leer de tama¤o 'tam'
****************************************************************************/
void frd(FILE *fbd, void *buff, size_t tam, size_t cant)
{

if(fread(buff,tam,cant,fbd)!=cant) {
	if(feof(fbd)) return;
	if(ferror(fbd)) {
		fclose(fbd);
		m_err(5,"Error en fichero de entrada",1);
	}
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
	M_ERR: imprime mensajes de error en una ventana de pantalla.
	  Entrada:      'x' columna de inicio del texto dentro de la ventana
			de errores
			'm' puntero a mensaje a imprimir
			'flag' si distinto de 0 sale al sistema operativo
			con exit(flag);
****************************************************************************/
void m_err(BYTE x, char *m, int flag)
{

/* borra la pantalla */
cls();

/* crea ventana de errores */
w_crea(WERR_FIL,WERR_COL,WERR_ANCHO,WERR_ALTO,WERR_COLORF,WERR_COLOR,BORDE_2,
  &w_err);

/* centra ventana */
w_err.wx=(BYTE)((vid.numtextcols-w_err.lx)/2);
w_err.wxi=(BYTE)(w_err.wx+1);

/* si no es mensaje vac¡o lo imprime */
if(*m) {
	w_cls(&w_err);          /* borra ventana para mensajes de error */
	w_err.cwx=x;            /* coloca cursor */
	w_err.cwy=0;
	w_imps(m,&w_err);       /* imprime mensaje */
	w_lee_tecla();          /* espera a que se pulse una tecla */
	cls();                  /* borra la pantalla */
}

/* si flag es distinto de 0 */
if(flag) {
	_setvideomode(_DEFAULTMODE);
	_asm {
		mov ax,3301h            ; establecer indicador ruptura
		mov dl,ruptura          ; lo restaura
		int 21h
	}
	/* libera memoria */
	free(tab_msy);
	free(tab_msg);
	free(tab_loc);
	free(tab_conx);
	free(tab_obj);
	free(tab_pro);

	#if DEBUGGER==1
	if(img_debug!=NULL) hfree(img_debug);
	#endif

	exit(flag);             /* sale al sistema operativo */
}

}
