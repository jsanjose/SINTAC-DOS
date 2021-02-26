/****************************************************************************
			  INTERPRETE-DEBUGGER SINTAC
			    (c)1993 JSJ Soft Ltd.

	NOTA: mediante la constante DEBUGGER se controla si se genera
	  el c¢digo del int‚rprete o del int‚rprete-debugger.
	  Con: DEBUGGER=ON se genera el c¢digo del int‚rprete-debugger
	       DEBUGGER=OFF se genera el c¢digo del int‚rprete
	  El valor de esta constante se puede modificar usando el men£
	  Options|Make|Compiler Flags|Defines.
****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <graph.h>
#include <bios.h>
#include <dos.h>
#include <time.h>
#include <fcntl.h>
#include <io.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <process.h>
#include "win.h"
#include "version.h"
#include "sintac.h"
#include "ds.h"

#if DEBUGGER==ON
#include "tabcond.h"    /* tabla de nombre-tipo de condactos */
BYTE buf_scr[800];      /* buffer para guardar zona pantalla */
STC_WINDOW w_jsj;       /* ventana de presentaci¢n */
STC_WINDOW w_deb;       /* ventana de debugger */
BOOLEAN debugg=TRUE;    /* TRUE si paso a paso activado */
BOOLEAN pra_lin=FALSE;  /* TRUE si en primera l¡nea de una entrada */
#endif

/* cabecera de fichero */
CAB_SINTAC cab;

/* variables para Vocabulario */
struct palabra vocabulario[NUM_PAL];    /* para almacenar vocabulario */

/* variables para Mensajes */
unsigned tab_desp_msg[MAX_MSG];         /* tabla de desplaz. de mensajes */
char *tab_msg;                          /* puntero a inicio zona de mensajes */

/* variables para Mensajes de Sistema */
unsigned tab_desp_msy[MAX_MSY];         /* tabla de desplaz. mens. sist. */
char *tab_msy;                          /* puntero a inicio zona mens. sist. */

/* variables para Localidades */
unsigned tab_desp_loc[MAX_LOC];         /* tabla desplaz. textos de locs. */
char *tab_loc;                          /* puntero a inicio de texto de locs. */
/** variables para Conexiones **/
unsigned tab_desp_conx[MAX_LOC];        /* tabla desplaz. lista conexiones */
char *tab_conx;                         /* puntero inicio zona de conexiones */

/* variables para Objetos */
unsigned tab_desp_obj[MAX_OBJ];         /* tabla de desplaz.lista de objetos */
char *tab_obj;                          /* puntero a inicio zona de objetos */

/* variables para Procesos */
unsigned tab_desp_pro[MAX_PRO];         /* tabla desplazamiento de procesos */
BYTE *tab_pro;                          /* puntero a inicio zona de procesos */

FILE *f_in;
STC_WINDOW w_err;               /* ventana para mensajes de error */
char frase[MAXLONG];            /* buffer para guardar frase tecleada */
char f_sl[MAXLONG];             /*    "     "     "    nombre fichero */
char *lin;                      /* puntero auxiliar a 'frase', usado por */
				/* parse1() y parse() */
BOOLEAN ini_inp=FALSE;          /* se pone a TRUE para indicar a parse() */
				/* el inicio de frase */
BOOLEAN mas_texto=FALSE;        /* usada por parse1() para frases encadenadas */
STC_WINDOW w[N_VENT];           /* tabla para guardar par metros de ventanas */
BYTE var[VARS];                 /* variables del sistema (8 bits) */
BYTE loc_obj[MAX_OBJ];          /* tabla de localidades act. de objetos */
BYTE flag[BANDS];               /* banderas del sistema, 256 banderas */
BYTE objs_cogidos;              /* n£mero de objetos cogidos */
BYTE pro_act;                   /* n£mero de proceso actual */
BYTE *pila1[STK];               /* para guardar direcciones de retorno */
unsigned pila2[STK];            /*   "     "    deplz. sgte. entrada */
BYTE pila3[STK];                /*   "     "    num. de proc. de llamada */
int ptrp;                       /* puntero de pila */
BYTE *ptr_proc;                 /* puntero auxiliar */
unsigned sgte_ent;              /* desplazamiento de sgte. entrada */
BOOLEAN resp_act;               /* RESP (=1) o NORESP (=0) */
BOOLEAN nueva_ent;              /* indica que no debe ajustar ptr_proc para */
				/* saltar a siguiente entrada */
int ptr_nexto=-1;               /* puntero NEXTO a tabla de objetos */
BOOLEAN doall=FALSE;            /* indicador de bucle doall */
struct {                        /* bancos para RAMSAVE y RAMLOAD */
	BYTE bram[VARS+BANDS+MAX_OBJ];
	BOOLEAN usado;
} ram[BANCOS_RAM];

BYTE ruptura;                   /* indicador de ruptura (BREAK) */
BYTE _far tabla_car[256][14];   /* buffer usado por carga_def() */
struct videoconfig vid;         /* informaci¢n del sistema de video */

struct condacto {
	int (*cond)();          /* puntero a funci¢n del condacto */
	BYTE tipo;              /* tipo: 0= void    1= BYTE   2= BYTE,BYTE */
				/*       3= BYTE,BYTE,BYTE */
};                              /*       6= BYTE,BYTE,BYTE,BYTE,BYTE,BYTE */
struct condacto cd[]={          /* tabla funci¢n-tipo de condacto */
	0       , 0,            /* condacto 0, reservado para fin entrada */
	process , 1,
	done    , 0,
	notdone , 0,
	resp    , 0,
	noresp  , 0,
	defwin  , 6,
	window  , 1,
	clw     , 1,
	let     , 2,
	eq      , 2,
	noteq   , 2,
	lt      , 2,
	gt      , 2,
	mes     , 1,
	newline , 0,
	message , 1,
	sysmess , 1,
	desc    , 1,
	add     , 2,
	sub     , 2,
	inc     , 1,
	dec     , 1,
	set     , 1,
	clear   , 1,
	zero    , 1,
	notzero , 1,
	place   , 2,
	get     , 1,
	drop    , 1,
	input   , 0,
	parse   , 0,
	skip    , 2,
	at      , 1,
	notat   , 1,
	atgt    , 1,
	atlt    , 1,
	adject1 , 1,
	noun2   , 1,
	adject2 , 1,
	listat  , 1,
	isat    , 2,
	isnotat , 2,
	present , 1,
	absent  , 1,
	worn    , 1,
	notworn , 1,
	carried , 1,
	notcarr , 1,
	wear    , 1,
	remove1 , 1,
	create  , 1,
	destroy , 1,
	swap    , 2,
	restart , 0,
	whato   , 0,
	move    , 1,
	ismov   , 0,
	goto1   , 1,
	print   , 1,
	dprint  , 1,
	cls     , 0,
	anykey  , 0,
	pause   , 1,
	listobj , 0,
	firsto  , 0,
	nexto   , 1,
	synonym , 2,
	hasat   , 1,
	hasnat  , 1,
	light   , 0,
	nolight , 0,
	random  , 2,
	seed    , 1,
	puto    , 1,
	inkey   , 0,
	copyov  , 2,
	chance  , 1,
	ramsave , 1,
	ramload , 3,
	ability , 1,
	autog   , 0,
	autod   , 0,
	autow   , 0,
	autor   , 0,
	isdoall , 0,
	ask     , 3,
	quit    , 0,
	save    , 0,
	load    , 2,
	exit1   , 1,
	end1    , 0,
	printat , 2,
	saveat  , 0,
	backat  , 0,
	newtext , 0,
	printc  , 1,
	ink     , 1,
	paper   , 1,
	bright  , 1,
	blink   , 1,
	color   , 1,
	debug   , 1,
	wborder , 2,
	charset , 1,
	extern1 , 2,
};

/*** Programa principal ***/
void main(int argc, char *argv[])
{
#if DEBUGGER==ON
BYTE lin_deb=0;
#endif
BYTE par1, par2;
BOOLEAN res_pro;        /* guarda salida de llamada a condacto */
BYTE *pro_d;            /* almacena direcci¢n de condacto en curso */
BOOLEAN indir1, indir2; /* indicadores de indirecci¢n de condacto en curso */

/* desactiva el cursor */
_displaycursor(_GCURSOROFF);

if(argc<2) m_err(13,"Falta nombre de fichero",1);

#if DEBUGGER==ON

/* si tecle¢ algo detr s del nombre del fichero */
if(argc==3) {
	/* si introdujo /l o /L recoge los dos siguiente d¡gitos y */
	/* calcula l¡nea de la ventana del debugger */
	if((argv[2][0]=='/') && ((argv[2][1]=='l') || (argv[2][1]=='L'))) {
		lin_deb=(BYTE)(((argv[2][2]-'0')*10)+(argv[2][3]-'0'));
		/* como la ventana del debugger ocupa 4 l¡neas */
		/* no puede ser colocada m s abajo de la 21 */
		if(lin_deb>21) lin_deb=21;
	}
}

/* crea la ventana del debugger */
w_crea(lin_deb,0,80,5,23,BORDE_1,&w_deb);

#endif

_harderr(int24_hnd);    /* instala 'handler' de errores cr¡ticos */

cls();
load_db(argv[1]);       /* carga base de datos */
inic();                 /* inicializa variables */

/* establece modo de pantalla */
modo_video();

_asm {
	mov ax,3300h            ; obtener indicador de ruptura
	int 21h
	mov ruptura,dl          ; guarda indicador de ruptura
	mov ax,3301h            ; establecer indicador de ruptura
	mov dl,0                ; lo desactiva
	int 21h
}

#if DEBUGGER==ON
/* presentaci¢n */
cls();
w_crea(9,19,40,9,48,BORDE_3,&w_jsj);
w_cls(&w_jsj);
w_imps("\n  "COPYRIGHT
       "\n  Int‚rprete-Debugger versi¢n "VERSION_IS"\n\n"
       "\n           Pulsa una tecla",&w_jsj);
w_lee_tecla();
cls();
#endif

/* inicializa puntero a proceso actual */
ptr_proc=tab_pro+tab_desp_pro[pro_act];

while(1) {
	/* si no es fin de proceso */
	if(*ptr_proc) {
		/* si res_pro es FALSE no debe ejecutar entrada */
		res_pro=!resp_act || (resp_act && ((*ptr_proc==NO_PAL) ||
		  (*ptr_proc==var[2])) && ((*(ptr_proc+1)==NO_PAL) ||
		  (*(ptr_proc+1)==var[3])));

		ptr_proc+=2;    /* salta verbo-nombre */

		/* calcula el desplazamiento de la siguiente entrada */
		sgte_ent=(*(ptr_proc+1) << 8) | *ptr_proc;
		ptr_proc+=2;

		#if DEBUGGER==ON
		pra_lin=TRUE;   /* indica que es primera l¡nea de entrada */
		#endif

	}
	/* si fin de proceso */
	else {
		res_pro=done();
		ptr_proc++;     /* ajustamos ptr_proc */

		#if DEBUGGER==ON
		pra_lin=FALSE;  /* no es primera l¡nea de entrada */
		#endif

	}
	if(nueva_ent) nueva_ent=FALSE;
	/* si res_pro es TRUE y no fin de entrada ejecuta esta entrada */
	/* si no, salta a la siguiente */
	while(res_pro && *ptr_proc) {
		/* supone que no hay indirecci¢n */
		par1=*(ptr_proc+1);
		par2=*(ptr_proc+2);

		#if DEBUGGER==ON
		indir1=FALSE;
		indir2=FALSE;
		/* guarda direcci¢n de condacto en curso */
		pro_d=ptr_proc;
		#endif

		/* indirecci¢n en primer par metro */
		if(*ptr_proc==INDIR1) {
			ptr_proc++;     /* salta prefijo indirecci¢n */
			par1=var[*(ptr_proc+1)];
			par2=*(ptr_proc+2);

			#if DEBUGGER==ON
			indir1=TRUE;
			#endif

		}
		/* indirecci¢n en segundo par metro */
		else if(*ptr_proc==INDIR2) {
			ptr_proc++;     /* salta prefijo indirecci¢n */
			par1=*(ptr_proc+1);
			par2=var[*(ptr_proc+2)];

			#if DEBUGGER==ON
			indir2=1;
			#endif

		}
		/* indirecci¢n en ambos par metros */
		else if(*ptr_proc==INDIR12) {
			ptr_proc++;     /* salta prefijo indirecci¢n */
			par1=var[*(ptr_proc+1)];
			par2=var[*(ptr_proc+2)];

			#if DEBUGGER==ON
			indir1=TRUE;
			indir2=TRUE;
			#endif

		}

	#if DEBUGGER==ON
	/* si est  activado el paso a paso */
	if(debugg) debugger(indir1,indir2,pro_d);
	/* sino, activa paso a paso */
	else if(_bios_keybrd(_KEYBRD_READY)==F10) {
		_bios_keybrd(_KEYBRD_READ);
		debugg=TRUE;
		/* indicamos que no es primera l¡nea de entrada para que */
		/* imp_condacto() no imprima el verbo-nombre ya que el */
		/* puntero al condacto no est  ajustado */
		pra_lin=FALSE;
	}
	#endif

		/* ejecuta condacto seg£n tipo */
		switch(cd[*ptr_proc].tipo) {
			case 0 :        /* sin par metros */
				res_pro=cd[*ptr_proc].cond();
				ptr_proc++;
				break;
			case 1 :        /* BYTE */
				res_pro=cd[*ptr_proc].cond(par1);
				ptr_proc+=2;
				break;
			case 2 :        /* BYTE,BYTE */
				res_pro=cd[*ptr_proc].cond(par1,par2);
				ptr_proc+=3;
				break;
			case 3 :        /* BYTE,BYTE,BYTE */
				res_pro=cd[*ptr_proc].cond(par1,par2,
				  *(ptr_proc+3));
				ptr_proc+=4;
				break;
			case 6 :        /* BYTE,BYTE,BYTE,BYTE,BYTE,BYTE */
				res_pro=cd[*ptr_proc].cond(par1,par2,
				  *(ptr_proc+3),*(ptr_proc+4),*(ptr_proc+5),
				  *(ptr_proc+6));
				ptr_proc+=7;
				break;
		}
	}
	/* si fin entrada, pasa a la siguiente */
	if(!*ptr_proc) ptr_proc++;
	else if(!nueva_ent) ptr_proc=tab_pro+tab_desp_pro[pro_act]+sgte_ent;
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

#if DEBUGGER==ON
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
	  Entrada:      'indir1' e 'indir2' indicadores de indirecci¢n
			'pro_d' direcci¢n del condacto en curso
		    variables globales:-
			'pra_lin' TRUE si es 1era l¡nea de entrada
			'ptr_proc' puntero a byte de condacto + par metros
****************************************************************************/
void imp_condacto(BOOLEAN indir1, BOOLEAN indir2, BYTE *pro_d)
{
BYTE *pcp;
int j, dirrel;
unsigned dir;
char lin_cond[81];
char *Pal_Nula="-       ";

/* si es la primera l¡nea de la entrada pone el puntero apuntando al */
/* campo verbo, si no apunta al condacto */
if(pra_lin) dir=(unsigned)(pro_d-tab_pro-4);
else dir=(unsigned)(pro_d-tab_pro);

/* imprime direcci¢n del condacto */
sprintf(lin_cond,"  %5u: ",dir);
w_imps(lin_cond,&w_deb);

if(pra_lin) {
	pcp=ptr_proc-4;                 /* apunta a verbo-nombre */
	if(indir1 || indir2) pcp--;     /* ajuste por indirecci¢n */
	if(*pcp==NO_PAL) sprintf(lin_cond,Pal_Nula);
	else {
		j=saca_pal(*pcp,_VERB);
		/* si no es verbo, quiz  sea nombre */
		if(j==(NUM_PAL+1)) j=saca_pal(*pcp,_NOMB);
		sprintf(lin_cond,"%s  ",vocabulario[j].p);
	}
	w_imps(lin_cond,&w_deb);
	pcp++;

	if(*pcp==NO_PAL) sprintf(lin_cond,Pal_Nula);
	else sprintf(lin_cond,"%s  ",vocabulario[saca_pal(*pcp,_NOMB)].p);
	w_imps(lin_cond,&w_deb);
}
else w_imps("                ",&w_deb);

/* imprime condacto */
sprintf(lin_cond,"%s  ",condacto[*ptr_proc].cnd);
w_imps(lin_cond,&w_deb);

/* imprime par metros seg£n tipo de condacto */
switch(condacto[*ptr_proc].tipo) {
	case 0 :
		sprintf(lin_cond," ");
		break;
	case 1 :
	case 3 :
	case 5 :
	case 6 :
	case 7 :
	case 8 :
	case 10 :
	case 12 :
	case 17 :
	case 19 :
	case 20 :
	case 23 :
	case 25 :
	case 26 :
		if(indir1) sprintf(lin_cond,"[%3u]",*(ptr_proc+1));
		else sprintf(lin_cond,"%5u",*(ptr_proc+1));
		break;
	case 4 :
	case 9 :
	case 15 :
	case 18 :
	case 22 :
	case 24 :
	case 27 :
		if(indir1 && indir2)
		  sprintf(lin_cond,"[%3u] [%3u]",*(ptr_proc+1),*(ptr_proc+2));
		else if(indir1) sprintf(lin_cond,"[%3u] %5u",*(ptr_proc+1),
		  *(ptr_proc+2));
		else if(indir2) sprintf(lin_cond,"%5u [%3u]",*(ptr_proc+1),
		  *(ptr_proc+2));
		else sprintf(lin_cond,"%5u %5u",*(ptr_proc+1),*(ptr_proc+2));
		break;
	case 11 :
		dirrel=(*(ptr_proc+2) << 8) | *(ptr_proc+1);
		if(pra_lin) dirrel+=4;  /* ajusta si 1era l¡nea */
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
	case 2 :
		if(indir1 && indir2) sprintf(lin_cond,"[%3u] [%3u] ",
		  *(ptr_proc+1),*(ptr_proc+2));
		else if(indir1) sprintf(lin_cond,"[%3u] %5u ",
		  *(ptr_proc+1),*(ptr_proc+2));
		else if(indir2) sprintf(lin_cond,"%5u [%3u] ",*(ptr_proc+1),
		  *(ptr_proc+2));
		else sprintf(lin_cond,"%5u %5u ",*(ptr_proc+1),*(ptr_proc+2));
		w_imps(lin_cond,&w_deb);
		sprintf(lin_cond,"%5u %5u %5u %5u",*(ptr_proc+3),
		  *(ptr_proc+4),*(ptr_proc+5),*(ptr_proc+6));
		break;
	case 16 :
		if(*(ptr_proc+1)==NO_PAL) sprintf(lin_cond,Pal_Nula);
		else {
			j=saca_pal(*(ptr_proc+1),_VERB);
			/* si no es vebro, quiz  sea nombre convertible */
			if(j==(NUM_PAL+1)) j=saca_pal(*(ptr_proc+1),_NOMB);
			sprintf(lin_cond,"%s  ",vocabulario[j].p);
		}
		w_imps(lin_cond,&w_deb);
		if(*(ptr_proc+2)==NO_PAL) sprintf(lin_cond,"-");
		else sprintf(lin_cond,"%s",vocabulario[saca_pal(*(ptr_proc+2),
		  _NOMB)].p);
		break;
	case 21 :
		if(indir1 && indir2) sprintf(lin_cond,"[%3u] [%3u]",
		  *(ptr_proc+1),*(ptr_proc+2));
		else if(indir1) sprintf(lin_cond,"[%3u] %5u",*(ptr_proc+1),
		  *(ptr_proc+2));
		else if(indir2) sprintf(lin_cond,"%5u [%3u]",*(ptr_proc+1),
		  *(ptr_proc+2));
		else sprintf(lin_cond,"%5u %5u",*(ptr_proc+1),*(ptr_proc+2));
		  w_imps(lin_cond,&w_deb);
		sprintf(lin_cond," %5u",*(ptr_proc+3));
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
		w_impc((char)'Û',&w_deb);
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
	GUARDA_SCREEN: guarda en un buffer la zona de pantalla que
	  ser  sobreescrita por la ventana del debugger.
****************************************************************************/
void guarda_screen(void)
{
unsigned i;
/* puntero a buffer v¡deo modo texto */
BYTE _far *pscr=(BYTE _far *)0xb8000000;

/* posiciona puntero en primer byte de zona a guardar */
pscr+=((w_deb.wy*80)+w_deb.wx)*2;

/* recoge las zona que ocupa la ventana del debugger */
for(i=0; i<(unsigned)(w_deb.lx*w_deb.ly*2); i++) buf_scr[i]=*pscr++;

}

/****************************************************************************
	RECUPERA_SCREEN: recupera del buffer la zona de pantalla.
****************************************************************************/
void recupera_screen(void)
{
unsigned i;
/* puntero a buffer v¡deo modo texto */
BYTE _far *pscr=(BYTE _far *)0xb8000000;

/* posiciona puntero en primer byte de zona a guardar */
pscr+=((w_deb.wy*80)+w_deb.wx)*2;

/* recoge las dos filas que ocupa la ventana del debugger */
for(i=0; i<(unsigned)(w_deb.lx*w_deb.ly*2); i++) *pscr++=buf_scr[i];

}

/****************************************************************************
	IMP_VARBAND: imprime en la l¡nea del debugger la variable y bandera
	  actuales.
	  Entrada:      'variable' n£mero de variable a imprimir
			'bandera' n£mero de bandera a imprimir
****************************************************************************/
void imp_varband(BYTE variable, BYTE bandera)
{
char lin_deb[81];
int palabra;

/* imprime variable */
sprintf(lin_deb,"Variable: %3u=%3u  ......",variable,var[variable]);
/* posiciona cursor */
w_deb.cwx=17;
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
w_deb.cwx=36;
w_imps(lin_deb,&w_deb);

/* imprime bandera */
sprintf(lin_deb,"Bandera: %3u=%1u",bandera,notzero(bandera));
w_deb.cwx=45;
w_deb.cwy=0;
w_imps(lin_deb,&w_deb);

}

/****************************************************************************
	IMP_DEBUGGER: imprime la ventana del debugger.
	  Entrada:      'indir1', 'indir2' indicadores de indirecci¢n
			'pro_d' direcci¢n del condacto en curso
			'variable', 'bandera' n£mero de variable y bandera
			que se mostrar n
			'tex_deb' texto a imprimir en la £ltima l¡nea
****************************************************************************/
void imp_debugger(BOOLEAN indir1, BOOLEAN indir2, BYTE *pro_d, BYTE variable,
  BYTE bandera, char *txt_deb)
{
char lin_deb[81];

/* borra la ventana del debugger e imprime informaci¢n */
w_cls(&w_deb);

w_deb.cwx=2;
w_deb.cwy=0;
sprintf(lin_deb,"Proceso: %3u",(unsigned)pro_act);
w_imps(lin_deb,&w_deb);

imp_varband(variable,bandera);
w_deb.cwx=0;
w_deb.cwy=1;
imp_condacto(indir1,indir2,pro_d);

w_deb.cwx=2;
w_deb.cwy=2;
w_imps(txt_deb,&w_deb);

}

/****************************************************************************
	DEBUGGER: funci¢n principal del debugger.
	  Entrada:      'indir1', 'indir2' indicadores de indirecci¢n
			'pro_d' direcci¢n del condacto en curso
****************************************************************************/
void debugger(BOOLEAN indir1, BOOLEAN indir2, BYTE *pro_d)
{
unsigned key;
static BYTE variable=0;
static BYTE bandera=0;
BYTE valor;
char lin_deb[81];
char *txt_deb1="Variables  Banderas  Pantalla  Desactivar  Salir";
char *txt_deb2="Otra  Modificar  Fin                            ";

/* guarda la zona de pantalla d¢nde se imprimir  */
/* la ventana del debugger */
guarda_screen();

/* imprime ventana del debugger */
imp_debugger(indir1,indir2,pro_d,variable,bandera,txt_deb1);

key=mayuscula((char)w_lee_tecla());

/* desactiva paso a paso */
if(key=='D') debugg=FALSE;

while(esta_en("SPVB",(char)key)) {
	switch(key) {
		case 'S' :      /* salir */
			m_err(17,"Pulsa una tecla",2);
			break;
		case 'P' :      /* mostrar pantalla */
			recupera_screen();
			/* espera hasta que se pulse una tecla */
			w_lee_tecla();
			imp_debugger(indir1,indir2,pro_d,variable,bandera,
			  txt_deb1);
			break;
		case 'V' :      /* variables */
			w_deb.cwx=2;
			w_deb.cwy=2;
			w_imps(txt_deb2,&w_deb);
			do {
				imp_varband(variable,bandera);
				key=mayuscula((char)w_lee_tecla());
				switch((BYTE)key) {
				case 'M' :              /* modificar */
					w_deb.cwx=31;
					w_deb.cwy=0;
					valor=inp_band();
					var[variable]=valor;
					break;
				case 'O' :              /* otra variable */
					w_deb.cwx=27;
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
		case 'b' :      /* banderas */
		case 'B' :
			w_deb.cwx=2;
			w_deb.cwy=2;
			w_imps(txt_deb2,&w_deb);
			do {
				imp_varband(variable,bandera);
				key=mayuscula((char)w_lee_tecla());
				switch((BYTE)key) {
				case 'M' :              /* modificar */
					w_deb.cwx=58;
					w_deb.cwy=0;
					w_impc((char)'Û',&w_deb);
					do {
						key=w_lee_tecla();
					} while((key!='0') && (key!='1'));
					if(key=='0') clear(bandera);
					else set(bandera);
					break;
				case 'O' :      /* otra bandera */
					w_deb.cwx=54;
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
		}
	w_deb.cwx=2;
	w_deb.cwy=2;
	w_imps(txt_deb1,&w_deb);
	key=mayuscula((char)w_lee_tecla());
}

recupera_screen();

}

#endif

/****************************************************************************
	MODO_VIDEO: establece el modo de v¡deo.
	  Entrada:      variable global 'vid' con informaci¢n sobre el
			sistema de v¡deo activo
****************************************************************************/
void modo_video(void)
{
/* si es VGA selecciona 350 l¡neas de barrido para usar */
/* caracteres de 8x14 en modo texto 80x25 */
if(vid.adapter==_VGA) {
	_asm {
		mov ax,1201h    ; selecci¢n de 350 l¡neas de barrido
		mov bl,30h
		int 10h
	}
}

/* pantalla modo texto 80x25 color */
_asm {
	mov ax,0003h            ; modo de v¡deo texto 80x25 color
	int 10h
}

/* desactiva el cursor */
_displaycursor(_GCURSOROFF);

}

/****************************************************************************
	W_IMPS2: imprime una cadena en una ventana. Las palabras de final de
	  l¡nea que no quepan dentro de la ventana se pasan a la l¡nea
	  siguiente. Esta versi¢n de w_imps tiene en cuenta el car cter de
	  subrayado que puede aparecer en los mensajes de la base de datos y
	  lo sustituye por el objeto actual.
	  Entrada:      's' cadena a imprimir
			'w' puntero a ventana
****************************************************************************/
void w_imps2(char *s, STC_WINDOW *w)
{
char b[MAX_PAL];                /* buffer para guardar palabras */
BYTE cuenta=0, i;
char *pto;

while(1) {
	/* si se encontr¢ fin de frase, espacio o avance de l¡nea */
	if((*s=='\0') || (*s==' ') || (*s=='\n')) {
		/* si cuenta==0, no hay palabra almacenada */
		if(!cuenta && (*s!='\0')) {
			w_impc(*s,w);
			s++;
		}
		else {
			/* si la palabra almacenada no cabe en lo que */
			/* queda de l¡nea pero cabe en la siguiente, */
			/* la imprime en la siguiente l¡nea, si no la */
			/* imprime sin m s */
			if((cuenta>(BYTE)(w->lxi-w->cwx-1)) &&
			  (cuenta<=w->lxi)) w_impc('\n',w);
			for(i=0; i<cuenta; i++) w_impc(b[i],w);
			cuenta=0;
			/* si fin de frase, sale */
			if(*s=='\0') break;
		}
	}
	else {
		if(*s!='_') {
			/* si letra no es espacio ni avance de l¡nea */
			/* la guarda */
			b[cuenta]=*s++;
			cuenta++;
			/* si se llena buffer */
			if(cuenta==MAX_PAL) {
				/* imprime lo que hay almacenado */
				for(i=0; i<MAX_PAL; i++) w_impc(b[i],w);
				cuenta=0;
				/* imprime resto de palabra */
				while((*s!='\0') && (*s!=' ') && (*s!='\n')) {
					w_impc(*s,w);
					s++;
				}
			}
		}
		/* si encontr¢ s¡mbolo de subrayado */
		else {
			/* se salta el s¡mbolo de subrayado */
			s++;
			/* puntero a texto objeto */
			pto=tab_obj+tab_desp_obj[var[8]]+6;
			while(*pto) {
				/* almacena texto objeto */
				b[cuenta]=*pto++;
				cuenta++;
				 /* si se llena el buffer */
				if(cuenta==MAX_PAL) {
					/* imprime lo que hay almacenado */
					for(i=0; i<MAX_PAL; i++) w_impc(b[i],w);
					cuenta=0;
					/* imprime resto de pal. de objeto */
					while((*pto!='\0') && (*pto!=' ') &&
					  (*pto!='\n')) {
						w_impc(*pto,w);
						pto++;
					}
				}
			}
		}
	}
}

}

/****************************************************************************
	ESTA_EN: comprueba si un car cter est  en una cadena.
	  Entrada:      's' cadena con la que se comprobar  el car cter
			'c' car cter a comprobar
	  Salida:       TRUE si el car cter est  en la cadena
			FALSE si no
****************************************************************************/
BOOLEAN esta_en(const char *s, char c)
{

while(*s) {
	if(*s++==c) return(TRUE);
}

return(FALSE);
}

/****************************************************************************
	PAUSA: realiza una pausa. Si se pulsa una tecla, sale de la pausa,
	  pero no saca la tecla del buffer de teclado.
	  Entrada:      'wait' tiempo de la pausa en milisegundos, si el
			es 0, se espera a que pulse una tecla
****************************************************************************/
void pausa(clock_t wait)
{
clock_t t1, t2;

/* si pausa es 0, espera que se pulse una tecla */
if(!wait) {
	w_lee_tecla();
}

t1=wait+clock();
do {
	t2=clock();
} while((t2<t1) && !_bios_keybrd(_KEYBRD_READY));

}

/****************************************************************************
	LOAD_DB: carga la base de datos.
****************************************************************************/
void load_db(char *nombre)
{
char *errmem="No hay suficiente memoria";
char *srecon=SRECON;

if((f_in=fopen(nombre,"rb"))==NULL)
  m_err(6,"Error de apertura fichero de entrada",1);

/* lee cabecera */
frd(&cab,sizeof(CAB_SINTAC),1);

/* comprueba que la versi¢n de la base de datos sea correcta */
if((cab.srecon[L_RECON-2]!=srecon[L_RECON-2]) ||
  (cab.srecon[L_RECON-1]!=srecon[L_RECON-1]))
  m_err(10,"Fichero de entrada no v lido",1);


/* Reserva de memoria para las distintas secciones */
/* -- Mensajes -- */
if((tab_msg=(char *)malloc((size_t)cab.bytes_msg))==NULL) m_err(12,errmem,1);

/* -- Mensajes del Sistema -- */
if((tab_msy=(char *)malloc((size_t)cab.bytes_msy))==NULL) m_err(12,errmem,1);

/* -- Localidades -- */
if((tab_loc=(char *)malloc((size_t)cab.bytes_loc))==NULL) m_err(12,errmem,1);
/* -- Conexiones -- */
if((tab_conx=(char *)malloc((size_t)cab.bytes_conx))==NULL) m_err(12,errmem,1);

/* -- Objetos -- */
if((tab_obj=(char *)malloc((size_t)cab.bytes_obj))==NULL) m_err(12,errmem,1);

/* -- Procesos -- */
if((tab_pro=(BYTE *)malloc((size_t)cab.bytes_pro))==NULL) m_err(12,errmem,1);

frd(vocabulario,sizeof(struct palabra),cab.pal_voc);

frd(tab_desp_msg,sizeof(unsigned),(size_t)MAX_MSG);
frd(tab_msg,sizeof(char),cab.bytes_msg);

frd(tab_desp_msy,sizeof(unsigned),(size_t)MAX_MSY);
frd(tab_msy,sizeof(char),cab.bytes_msy);

frd(tab_desp_loc,sizeof(unsigned),(size_t)MAX_LOC);
frd(tab_loc,sizeof(char),cab.bytes_loc);

frd(tab_desp_conx,sizeof(unsigned),(size_t)MAX_LOC);
frd(tab_conx,sizeof(BYTE),cab.bytes_conx);

frd(tab_desp_obj,sizeof(unsigned),(size_t)MAX_OBJ);
frd(tab_obj,sizeof(char),cab.bytes_obj);

frd(tab_desp_pro,sizeof(unsigned),(size_t)MAX_PRO);
frd(tab_pro,sizeof(BYTE),cab.bytes_pro);

/* decodifica las secciones */
codifica((BYTE *)tab_msg,cab.bytes_msg);
codifica((BYTE *)tab_msy,cab.bytes_msy);
codifica((BYTE *)tab_loc,cab.bytes_loc);
codifica(tab_conx,cab.bytes_conx);
codifica((BYTE *)tab_obj,cab.bytes_obj);
codifica(tab_pro,cab.bytes_pro);
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
	INIC: inicializa diversas tablas y variables.
****************************************************************************/
void inic(void)
{
int i;
char *po;

objs_cogidos=0;                         /* n£mero de objetos cogidos */

/* inicializa tabla de localidades actuales de los objetos */
for(i=0; i<(int)cab.num_obj; i++) {
	po=tab_obj+tab_desp_obj[i];
	/* coge localidad inicial y la guarda en tabla */
	loc_obj[i]=(BYTE)*(po+2);
	/* si lleva objeto de inicio, incrementa contador objs. cogidos */
	if((loc_obj[i]==PUESTO) || (loc_obj[i]==COGIDO)) objs_cogidos++;
}

/* inicializa variables */
for(i=0; i<256; i++) {
	/* variables de sentencia l¢gica inicializadas a NO_PAL */
	if((i>1) && (i<7)) var[i]=NO_PAL;
	else var[i]=0;
}

/* inicializa banderas */
for(i=0; i<32; i++) flag[i]=0;

/* inicializa ventanas */
for(i=0; i<N_VENT; i++) w_crea(0,0,80,25,7,NO_BORDE,&w[i]);

/* inicializa bancos de RAMSAVE y RAMLOAD */
for(i=0; i<BANCOS_RAM; i++) ram[i].usado=FALSE;

pro_act=0;                      /* n£mero de proceso actual */
ptrp=0;                         /* puntero de pila */
resp_act=FALSE;                 /* NORESP */
nueva_ent=FALSE;

/* coge y guarda informaci¢n del sistema de video */
/* la cual puede ser usada por varias rutinas */
_getvideoconfig(&vid);

/* si la tarjeta es EGA o VGA pone a 1 la bandera 4 */
if((vid.adapter==_EGA) || (vid.adapter==_VGA)) set(4);

}

/****************************************************************************
	FRD: controla la entrada de datos desde el fichero de entrada
	  mediante la funci¢n fread.
****************************************************************************/
void frd(void *buff, size_t tam, size_t cant)
{
if(fread(buff,tam,cant,f_in)!=cant) {
	if(feof(f_in)) return;
	if(ferror(f_in)) m_err(12,"Error en fichero de entrada",1);
}

}

/****************************************************************************
	ANALIZA: analiza una palabra.
	  < !­?¨>   caracteres no significativos
	  <.,;:"'>  separadores
	  Entrada:      'pfrase' puntero a puntero a frase a analizar
	  Salida:       'tipo', 'num' tipo y n£mero de la palabra en vocabul.
			SEPARADOR si encontr¢ un separador
			FIN_FRASE si encontr¢ final de la frase
			PALABRA si encontr¢ palabra v lida de vocabulario
			TERMINACION como PALABRA pero si adem s encontr¢ una
			terminaci¢n en LA, LE o LO
			NO_PAL si encontr¢ palabra pero no est  en
			vocabulario
****************************************************************************/
int analiza(char *(*pfrase), BYTE *tipo, BYTE *num)
{
int i=0;
char palabra[LONGPAL+1];

/* salta caracteres no significativos */
for(; esta_en(C_NO_SIG,*(*pfrase)); (*pfrase)++);

/* si es un separador lo salta */
if(esta_en(C_SEPAR,*(*pfrase))) {
	(*pfrase)++;
	/* salta caracteres no significativos */
	for(; esta_en(C_NO_SIG,*(*pfrase)); (*pfrase)++);

	/* si hay otro separador detr s lo salta tambi‚n */
	while(esta_en(C_SEPAR,*(*pfrase))) {
		(*pfrase)++;
		/* salta caracteres no significativos */
		for(; esta_en(C_NO_SIG,*(*pfrase)); (*pfrase)++);
	}
	/* si detr s encuentra final de la frase, sale indic ndolo */
	/* si no, sale indicando que encontr¢ separador */
	if(!*(*pfrase)) return(FIN_FRASE);
	else return(SEPARADOR);
}

/* si encontr¢ fin de frase, lo indica */
if(!*(*pfrase)) return(FIN_FRASE);

/* salta espacios anteriores a palabra */
for(; *(*pfrase)==' '; (*pfrase)++);
/* al llegar aqu¡ *(*pfrase) ser  [A-Z]+¥+[0-9] */
/* repite mientras no llene palabra y encuentre car cter v lido de palabra */
do {
	/* mete car cter en palabra */
	palabra[i]=*(*pfrase);
	(*pfrase)++;
	i++;
} while((i<LONGPAL) && (esta_en(CAR_PAL,*(*pfrase))));

/* rellena con espacios y marca fin de palabra */
for(; i<LONGPAL; i++) palabra[i]=' ';
palabra[i]='\0';

/* desprecia resto de palabra */
while(esta_en(CAR_PAL,*(*pfrase))) (*pfrase)++;

/* comprobamos si la palabra est  en vocabulario */
*num=0;
*tipo=0;
if((i=esta_en_voc(palabra))==NUM_PAL+1) return(NO_PALABRA);

/* si est  en vocabulario coge su n£mero y su tipo */
*num=vocabulario[i].num;
*tipo=vocabulario[i].tipo;

/* si es una conjunci¢n indica que encontr¢ un separador */
if(*tipo==_CONJ) return(SEPARADOR);

/* si termina en LA, LE o LO indica palabra con terminaci¢n */
if((*((*pfrase)-2)=='L') && ((*((*pfrase)-1)=='A') || (*((*pfrase)-1)=='E') ||
  (*((*pfrase)-1)=='O'))) return(TERMINACION);
/* lo mismo si termina en LAS, LES o LOS */
else if((*((*pfrase)-1)=='S') && (*((*pfrase)-3)=='L') &&
  ((*((*pfrase)-1)=='A') || (*((*pfrase)-1)=='E') || (*((*pfrase)-1)=='O')))
  return(TERMINACION);
/* en cualquier otro caso indica que encontr¢ palabra */
else return(PALABRA);

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
	ESTA_EN_VOC: comprueba si una palabra est  en el vocabulario.
	  Entrada:      'vocab' puntero a tabla de vocabulario
			'pal_voc' n£mero de palabras en vocabulario
			'pal' puntero a palabra a buscar
	  Salida:       posici¢n dentro del vocabulario si se encontr¢, si no
			devuelve (NUM_PAL+1) que puede considerarse como
			palabra no encontrada
****************************************************************************/
int esta_en_voc(char *pal)
{
int i;

for(i=0; i<cab.pal_voc; i++) {
	if(!strcmp(pal,vocabulario[i].p)) return(i);
}

return(NUM_PAL+1);
}

/****************************************************************************
	PARSE1: analiza la l¡nea principal de entrada hasta un separador o
	  hasta el fin de l¡nea.
	  Entrada:      'frase' conteniendo la l¡nea a analizar
	  Salida:       TRUE si no hay m s que analizar
			FALSE si queda texto por analizar
			Variables 2 a 6 actualizadas convenientemente
****************************************************************************/
BOOLEAN parse1(void)
{
BYTE num, tipo;
BYTE nombrelo, adjtlo, verbo;
int res, f[3], i;

/* inicializa banderas de palabras */
for(i=0; i<3; i++) f[i]=0;

nombrelo=var[3];        /* guarda nombre para verbo con terminaci¢n */
adjtlo=var[4];          /* guarda adjetivo para verbo con terminaci¢n */
verbo=var[2];           /* guarda verbo por si teclea frase sin ‚l */

/* inicializa variables de sentencia l¢gica */
for(i=2; i<7; i++) var[i]=NO_PAL;

/* repite hasta fin de frase o separador */
do {
	/* analiza una palabra */
	res=analiza(&lin,&tipo,&num);

	/* si es palabra sin terminaci¢n */
	if(res==PALABRA) {
		/* si es verbo y no cogi¢ ninguno, lo almacena */
		if((tipo==_VERB) && f[_VERB]!=1) {
			var[2]=num;
			f[_VERB]=1;
		}
		/* s¢lo coge los dos primeros nombres */
		if((tipo==_NOMB) && (f[_NOMB]<2)) {
			/* almacena nombre 1 en variable 3 */
			if(!f[_NOMB]) {
				var[3]=num;
				/* si es nombre convertible tambi‚n lo */
				/* almacena en verbo si no cogi¢ antes */
				/* ning£n verbo ni otro nombre conv. */
				if((num<cab.n_conv) && !f[_VERB]) {
					var[2]=num;
					/* indica que hay nombre convert. */
					f[_VERB]=2;
					/* luego lo incrementar  */
					f[_NOMB]--;
				}
			}
			/* almacena nombre 2 en variable 5 */
			if(f[_NOMB]==1) var[5]=num;
			/* incrementa n£mero de nombres cogidos */
			f[_NOMB]++;
		}
		/* s¢lo coge los dos primeros adjetivos */
		if((tipo==_ADJT) && (f[_ADJT]<2)) {
			/* almacena adjetivo 1 en variable 4 */
			if(!f[_ADJT]) var[4]=num;
			/* almacena adjetivo 2 en variable 6 */
			if(f[_ADJT]==1) var[6]=num;
			/* incrementa n£mero de adjetivos cogidos */
			f[_ADJT]++;
		}
	}
	/* si es palabra con terminaci¢n */
	if(res==TERMINACION) {
		if((tipo==_VERB) && f[_VERB]!=1) {
			var[2]=num; /* almacena n£mero verbo en variable 2 */
			f[_VERB]=1; /* indica que ya ha cogido el verbo */
			/* si nombre anterior no era Propio */
			/* recupera el nombre anterior y su adjetivo */
			if(nombrelo>=cab.n_prop) {
				var[3]=nombrelo;
				var[4]=adjtlo;
				f[_NOMB]++;
				f[_ADJT]++;
			}
		}
		/* s¢lo coge los dos primeros nombres */
		if((tipo==_NOMB) && (f[_NOMB]<2)) {
			/* almacena nombre 1 en variable 3 */
			if(!f[_NOMB]) {
				var[3]=num;
				/* si es nombre convertible tambi‚n lo */
				/* almacena en verbo si no cogi¢ antes */
				/* ning£n verbo ni otro nombre conv. */
				if((num<cab.n_conv) && !f[_VERB]) {
					var[2]=num;
					/* indica que hay nombre convertible */
					f[_VERB]=2;
					/* luego lo incrementar  */
					f[_NOMB]--;
				}
			}
			/* almacena nombre 2 en variable 5 */
			if(f[_NOMB]==1) var[5]=num;
			/* incrementa n£mero de nombres cogidos */
			f[_NOMB]++;
		}
		/* s¢lo coge los dos primeros adjetivos */
		if((tipo==_ADJT) && (f[_ADJT]<2)) {
			/* almacena adjetivo 1 en variable 4 */
			if(!f[_ADJT]) var[4]=num;
			/* almacena adjetivo 2 en variable 6 */
			if(f[_ADJT]==1) var[6]=num;
			/* incrementa n£mero de adjetivos cogidos */
			f[_ADJT]++;
		}
	}
} while((res!=FIN_FRASE) && (res!=SEPARADOR));

/* si tecle¢ una frase sin verbo pero con nombre, pone el verbo anterior */
if(!f[_VERB] && f[_NOMB]) var[2]=verbo;

/* si es fin de frase mira si analiz¢ o no lo £ltimo cogido */
if(res==FIN_FRASE) {
	if(!mas_texto) {
		mas_texto=TRUE;         /* indicador para siguiente llamada */
		return(FALSE);          /* indica que analice lo £ltimo cogido */
	}
	else {
		/* si analiz¢ lo £ltimo cogido antes de fin frase */
		/* indica que no queda m s por analizar */
		mas_texto=FALSE;
		return(TRUE);
	}
}

/* si es separador, supone que hay m s texto detr s */
return(FALSE);
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
w_crea(10,14,50,3,79,BORDE_2,&w_err);

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
	free(tab_msg);
	free(tab_msy);
	free(tab_loc);
	free(tab_conx);
	free(tab_obj);
	free(tab_pro);

	exit(flag);             /* sale al sistema operativo */
}

}

/****************************************************************************
	CARGA_DEF: carga de un fichero las definiciones de los
	  caracteres de 8x14 y actualiza el generador de caracteres.
	  Entrada:      'nombre' nombre del fichero
	  Salida:       0 si hubo error o un valor distinto de 0 en
			otro caso
	  NOTA: usa un buffer global llamado 'tabla_car'
****************************************************************************/
int carga_def(char *nombre)
{
FILE *ffuente;
char cad_recon[LONG_RECON_F+1];
char *recon_fuente=RECON_FUENTE;

/* abre el fichero para lectura */
ffuente=fopen(nombre,"rb");
/* sale si hubo error */
if(ffuente==NULL) return(0);

/* lee cadena de reconocimiento */
if(fread(cad_recon,sizeof(char),LONG_RECON_F+1,ffuente)<LONG_RECON_F+1) {
	fclose(ffuente);
	return(0);
}

/* comprueba la versi¢n del fichero */
if(cad_recon[LONG_RECON_F-1]!=recon_fuente[LONG_RECON_F-1]) {
	fclose(ffuente);
	return(0);
}

/* si la versi¢n ha sido v lida lee las definiciones de los caracteres */
if(fread(tabla_car,sizeof(BYTE),256*14,ffuente)<256*14) {
	fclose(ffuente);
	return(0);
}

fclose(ffuente);

/* actualiza el generador de caracteres */
_asm {
	push bp
	mov ax,1100h            ; cargar definiciones (sin programar CRT)
	mov bh,14               ; 14 bytes por car cter
	mov bl,0                ; tabla 0 del generador de caracteres
	mov cx,256              ; 256 caracteres definidos
	mov dx,SEG tabla_car
	mov es,dx
	mov bp,OFFSET tabla_car ; ES:BP direcci¢n definiciones caracteres
	mov dx,0                ; c¢digo ASCII del 1er car cter definido
	int 10h
	pop bp
}

return(1);
}

/****************************************************************************
	PROCESS: ejecuta una llamada a un proceso.
	  Entrada:      'pro' n£mero de proceso
****************************************************************************/
BOOLEAN process(BYTE prc)
{
/* si se rebasa la capacidad de pila interna */
if(ptrp==STK) m_err(9,"Rebosamiento de la pila interna",1);

pila1[ptrp]=ptr_proc+2; /* guarda dir. sgte. condacto en proc. actual */
pila2[ptrp]=sgte_ent;   /*   "    desplazamiento de sgte. entrada */
pila3[ptrp]=pro_act;    /*   "    n£mero de proceso actual */
ptrp++;                 /* incrementa puntero de pila */
pro_act=prc;

/* direcci¢n del proceso llamado - 2 (que se sumar ) */
ptr_proc=tab_pro+tab_desp_pro[pro_act]-2;

/* indica que no debe ajustar ptr_proc para siguiente entrada */
nueva_ent=TRUE;

/* saltar  a inicio nueva entrada (la primera del proceso llamado) */
return(FALSE);
}

/****************************************************************************
	DONE: finaliza un proceso retornando como TRUE.
****************************************************************************/
BOOLEAN done(void)
{
if(!ptrp) m_err(0,"",1);
ptrp--;
/* recupera dir. sgte condacto en proc. que llam¢ - 1 (que se sumar  luego) */
ptr_proc=pila1[ptrp]-1;

sgte_ent=pila2[ptrp];   /* desplazamiento de la siguiente entrada */
pro_act=pila3[ptrp];    /* n£mero de proceso que llam¢ */

/* indica que no debe ajustar ptr_proc para siguiente entrada */
nueva_ent=TRUE;

return(TRUE);
}

/****************************************************************************
	NOTDONE: finaliza un proceso retornando como FALSE.
****************************************************************************/
BOOLEAN notdone(void)
{
done();                 /* ejecuta un DONE */
nueva_ent=FALSE;        /* para que ajuste a siguiente entrada */

return(FALSE);          /* pero sale como FALSE */
}

/****************************************************************************
	RESP: activa comprobaci¢n de verbo-nombre al inicio de cada entrada.
****************************************************************************/
BOOLEAN resp(void)
{
resp_act=TRUE;

return(TRUE);
}

/****************************************************************************
	NORESP: desactiva comprobaci¢n de verbo-nombre.
****************************************************************************/
BOOLEAN noresp(void)
{
resp_act=FALSE;

return(TRUE);
}

/****************************************************************************
	DEFWIN: define una ventana.
	  Entrada:      'nw' n£mero de ventana
			'cw' color
			'wy', 'wx' posici¢n de esquina superior izquierda
			(fila,columna) de la ventana
			'lx', 'ly' tama¤o (ancho y alto) de la ventana
****************************************************************************/
BOOLEAN defwin(BYTE nw, BYTE cw, BYTE wy, BYTE wx, BYTE lx, BYTE ly)
{
w_crea(wy,wx,lx,ly,cw,NO_BORDE,&w[nw]);

return(TRUE);
}

/****************************************************************************
	WINDOW: selecciona la ventana activa.
	  Entrada:      'nw' n£mero de ventana
****************************************************************************/
BOOLEAN window(BYTE nw)
{
var[0]=nw;

return(TRUE);
}

/****************************************************************************
	CLW: borra/inicializa una ventana.
	  Entrada:      'nw' n£mero de ventana
****************************************************************************/
BOOLEAN clw(BYTE nw)
{
w_cls(&w[nw]);

return(TRUE);
}

/****************************************************************************
	LET: asigna un valor a una variable.
	  Entrada:      'nv' n£mero de variable
			'val' valor a asignar
****************************************************************************/
BOOLEAN let(BYTE nv, BYTE val)
{
var[nv]=val;

return(TRUE);
}

/****************************************************************************
	EQ: comprueba si una variable es igual a un valor.
	  Entrada:      'nv' n£mero de variable
			'val' valor
	  Salida:       TRUE si la variable es igual al valor
			FALSE si es distinta de valor
****************************************************************************/
BOOLEAN eq(BYTE nv, BYTE val)
{
if(var[nv]==val) return(TRUE);

return(FALSE);
}

/****************************************************************************
	NOTEQ: comprueba si una variable distinta de un valor.
	  Entrada:      'nv' n£mero de variable
			'val' valor
	  Salida:       TRUE si la variable es distinta del valor
			FALSE si es igual al valor
****************************************************************************/
BOOLEAN noteq(BYTE nv, BYTE val)
{
return(TRUE-eq(nv,val));
}

/****************************************************************************
	LT: comprueba si una variable es menor que un valor.
	  Entrada:      'nv' n£mero de variable
			'val' valor
	  Salida:       TRUE si la variable es menor que valor
			FALSE si es mayor o igual que valor
****************************************************************************/
BOOLEAN lt(BYTE nv, BYTE val)
{
if(var[nv]<val) return(TRUE);

return(FALSE);
}

/****************************************************************************
	GT: comprueba si una variable es mayor que un valor.
	  Entrada:      'nv' n£mero de variable
			'val' valor
	  Salida:       TRUE si la variable es mayor que valor
			FALSE si es menor o igual que valor
****************************************************************************/
BOOLEAN gt(BYTE nv, BYTE val)
{
if(var[nv]>val) return(TRUE);

return(FALSE);
}

/****************************************************************************
	NEWLINE: imprime un avance de l¡nea.
****************************************************************************/
BOOLEAN newline(void)
{
w_impc('\n',&w[var[0]]);

return(TRUE);
}

/****************************************************************************
	MES: imprime un mensaje.
	  Entrada:      'nm' n£mero de mensaje
****************************************************************************/
BOOLEAN mes(BYTE nm)
{
char *pm;

pm=tab_msg+tab_desp_msg[nm];    /* puntero a mensaje */
w_imps2(pm,&w[var[0]]);         /* imprime mensaje en ventana activa */

return(TRUE);
}

/****************************************************************************
	MESSAGE: imprime un mensaje con avance de l¡nea.
	  Entrada:      'nm' n£mero de mensaje
****************************************************************************/
BOOLEAN message(BYTE nm)
{
mes(nm);                        /* imprime mensaje */
newline();                      /* y avance de l¡nea */

return(TRUE);
}

/****************************************************************************
	SYSMESS: imprime un mensaje del sistema.
	  Entrada: 'nm' n£mero de mensaje
****************************************************************************/
BOOLEAN sysmess(BYTE nm)
{
char *pm;

pm=tab_msy+tab_desp_msy[nm];    /* puntero a mensaje */
w_imps2(pm,&w[var[0]]);         /* imprime mensaje en ventana activa */

return(TRUE);
}

/****************************************************************************
	DESC: imprime la descripci¢n de una localidad y salta al inicio del
	  Proceso 0. Si est  oscuro imprime el mensaje del sistema 23.
	  Entrada:      'nl' n£mero de localidad
****************************************************************************/
BOOLEAN desc(BYTE nl)
{
char *pl;

/* puntero a descripci¢n de localidad */
pl=tab_loc+tab_desp_loc[nl];

/* si no est  oscuro o hay una fuente de luz imprime descripci¢n */
if(zero(0) || light()) w_imps2(pl,&w[var[0]]);
else sysmess(23);       /* 'Est  oscuro. No puedes ver nada.' */

/* indica que hay que inicializar: borrar ventana, listar objetos... */
clear(2);

restart();              /* salta al inicio de Proceso 0 */

/* -1 a ptr_proc (y -1 del restart), luego se sumar n 2 */
ptr_proc--;

return(FALSE);
}

/****************************************************************************
	ADD: suma un valor a una variable.
	  Entrada:      'nv' n£mero de variable
			'val' valor a sumar
****************************************************************************/
BOOLEAN add(BYTE nv, BYTE val)
{
var[nv]+=val;

return(TRUE);
}

/****************************************************************************
	SUB: resta un valor a una variable.
	  Entrada:      'nv' n£mero de variable
			'val' valor a restar
****************************************************************************/
BOOLEAN sub(BYTE nv, BYTE val)
{
var[nv]-=val;

return(TRUE);
}

/****************************************************************************
	INC: incrementa en 1 el valor de una variable.
	  Entrada:      'nv' n£mero de variable
****************************************************************************/
BOOLEAN inc(BYTE nv)
{
var[nv]++;

return(TRUE);
}

/****************************************************************************
	DEC: decrementa en 1 el valor de una variable.
	  Entrada:      'nv' n£mero de variable
****************************************************************************/
BOOLEAN dec(BYTE nv)
{
var[nv]--;

return(TRUE);
}

/****************************************************************************
	SET: pone a 1 una bandera.
	  Entrada:      'nf' n£mero de bandera
****************************************************************************/
BOOLEAN set(BYTE nf)
{
BYTE mascara_set=0x80;
int nbyte, nbit;

nbyte=nf/8;                     /* n£mero de byte */
nbit=nf%8;                      /* bit dentro del byte */

mascara_set>>=nbit;
flag[nbyte]|=mascara_set;

return(TRUE);
}

/****************************************************************************
	CLEAR: pone a 0 una bandera.
	  Entrada:      'nf' n£mero de bandera
****************************************************************************/
BOOLEAN clear(BYTE nf)
{
BYTE mascara_clr=0x80;
int nbyte, nbit;

nbyte=nf/8;                     /* n£mero de byte */
nbit=nf%8;                      /* bit dentro del byte */

mascara_clr>>=nbit;                     /* desplaza hacia derecha (a¤ade 0) */
mascara_clr=(BYTE)0xff-mascara_clr;     /* complementa mascara_clr */
flag[nbyte]&=mascara_clr;

return(TRUE);
}

/****************************************************************************
	ZERO: comprueba si una bandera es 0.
	  Entrada:      'nf' n£mero de bandera
	  Salida:       TRUE si la bandera es 0
			FALSE si la bandera es 1
****************************************************************************/
BOOLEAN zero(BYTE nf)
{
BYTE mascara=0x80;
int nbyte, nbit;

nbyte=nf/8;
nbit=nf%8;
mascara>>=nbit;

if(!(flag[nbyte] & mascara)) return(TRUE);

return(FALSE);
}

/****************************************************************************
	NOTZERO: comprueba si una bandera es 1.
	  Entrada:      'nf' n£mero de bandera
	  Salida:       TRUE si la bandera es 1
			FALSE si la bandera es 0
****************************************************************************/
BOOLEAN notzero(BYTE nf)
{
return(TRUE-zero(nf));
}

/****************************************************************************
	PLACE: coloca un objeto en una localidad.
	  Entrada:      'nobj' n£mero de objeto
			'nloc' n£mero de localidad
****************************************************************************/
BOOLEAN place(BYTE nobj,BYTE nloc)
{
BYTE locobj;

/* coge la localidad actual del objeto */
locobj=loc_obj[nobj];

/* si se refiere a localidad actual sustituye por su valor */
if(nloc==LOC_ACTUAL) nloc=var[1];

/* si localidad actual de objeto es igual a la de destino, no hace nada */
if(nloc==locobj) return(TRUE);

/* si pasa un objeto a cogido o puesto y no estaba ni cogido ni puesto */
/* incrementa el n£mero de objetos que lleva */
if(((nloc==COGIDO) || (nloc==PUESTO)) && (locobj!=COGIDO) &&
  (locobj!=PUESTO)) objs_cogidos++;

/* si el objeto estaba cogido o puesto y no lo pasa a cogido ni a puesto */
/* decrementa el n£mero de objetos que lleva */
if(((locobj==COGIDO) || (locobj==PUESTO)) && (nloc!=COGIDO) &&
  (nloc!=PUESTO)) objs_cogidos--;

/* actualiza posici¢n del objeto */
loc_obj[nobj]=nloc;

return(TRUE);
}

/****************************************************************************
	GET: coge un objeto.
	  Entrada:      'nobj' n£mero de objeto
	  Salida:       TRUE si se pudo coger el objeto
			FALSE si no se pudo coger
			NOTA: la variable 8 contendr  el n£mero del objeto
			a la salida
****************************************************************************/
BOOLEAN get(BYTE nobj)
{
/* si el objeto no existe */
if(nobj>=cab.num_obj) {
	sysmess(1);             /* 'Aqu¡ no est  eso.' */
	return(FALSE);
}

/* coloca el n£mero del objeto en la variable 8 */
var[8]=nobj;

/* si ya ten¡a el objeto */
if(carried(nobj) || worn(nobj)) {
	sysmess(3);             /* 'Ya tienes eso.' */
	return(FALSE);          /* debe hacer un DONE */
}

/* si el objeto no est  presente */
if(loc_obj[nobj]!=var[1]) {
	sysmess(1);             /* 'Aqu¡ no est  eso.' */
	return(FALSE);          /* debe hacer un DONE */
}

/* si lleva muchas cosas */
if((objs_cogidos>=var[7]) && (var[7]!=0)) {
	sysmess(2);     /* 'No puedes coger _. Llevas demasiadas cosas.' */
	return(FALSE);  /* debe hacer un DONE */
}

/* coge el objeto */
place(nobj,COGIDO);
sysmess(0);             /* 'Has cogido _.' */

return(TRUE);
}

/****************************************************************************
	DROP: deja un objeto.
	  Entrada:      'nobj' n£mero de objeto
	  Salida:       TRUE si se pudo dejar el objeto
			FALSE si no se pudo dejar
			NOTA: la variable 8 contendr  el n£mero del objeto
			a la salida
****************************************************************************/
BOOLEAN drop(BYTE nobj)
{
/* si el objeto no existe */
if(nobj>=cab.num_obj) {
	sysmess(5);                     /* 'No tienes eso.' */
	return(FALSE);
}

/* coloca el n£mero del objeto en la variable 8 */
var[8]=nobj;

/* si no cogido ni puesto */
if(notcarr(nobj) && notworn(nobj)) {
	sysmess(5);                     /* 'No tienes eso.' */
	return(FALSE);                  /* debe hacer un DONE */
}

/* si lleva el objeto cogido o puesto, lo deja en loc. actual */
place(nobj,var[1]);
sysmess(4);                             /* 'Dejas _.' */

return(TRUE);
}

/****************************************************************************
	INPUT: recoge texto del jugador.
	  Salida:       TRUE si no tecle¢ nada
			FALSE si tecle¢ algo
			NOTA: la variable 12 contendr  el c¢digo de la tecla
			de funci¢n que se puls¢ durante el INPUT
****************************************************************************/
BOOLEAN input(void)
{
int i;
unsigned k;

/* inicializa sentencia l¢gica */
for(i=2; i<7; i++) var[i]=NO_PAL;

ini_inp=TRUE;           /* indica a parse() inicio de frase */
mas_texto=FALSE;        /* usada por parse1() para frases encadenadas */

/* NOTA: el cursor ser  el primer car cter del mensaje del sistema 7 */
k=w_input(frase,MAXLONG,*(tab_msy+tab_desp_msy[7]),&w[var[0]]);

/* guarda c¢digo de tecla conque termin¢ INPUT */
var[12]=(BYTE)k;

/* si no tecle¢ nada */
if(*frase=='\0') return(TRUE);

return(FALSE);
}

/****************************************************************************
	PARSE: analiza la frase tecleada por jugador.
	  Entrada:      variables globales.-
			  'frase' conteniendo frase tecleada
			  'ini_inp' TRUE si hay que analizar desde principio,
			  FALSE si sigue donde lo dej¢ en £ltima llamada
	  Salida:       TRUE si se analiz¢ toda la frase
			FALSE si queda m s por analizar
****************************************************************************/
BOOLEAN parse(void)
{
BOOLEAN par;

/* si inicio de frase */
if(ini_inp) {
	/* coloca puntero al principio de l¡nea tecleada */
	lin=frase;
	ini_inp=FALSE;
}

/* analiza hasta separador o fin l¡nea */
par=parse1();

return(par);
}

/****************************************************************************
	SKIP: salto relativo dentro de un proceso.
	  Entrada:      'lsb', 'hsb' bytes bajo y alto del desplazamiento del
			salto (-32768 a 32767)
****************************************************************************/
BOOLEAN skip(BYTE lsb, BYTE hsb)
{
int despli;

/* calcula desplazamiento */
despli=(hsb << 8) | lsb;

/* ptr_por - 3 que se sumar n luego */
ptr_proc=ptr_proc+despli-3;

/* para que no ajuste ptr_proc a siguiente entrada */
nueva_ent=TRUE;

return(FALSE);          /* saltar  a inicio de entrada */
}

/****************************************************************************
	AT: comprueba si est  en una localidad.
	  Entrada:      'locno' n£mero de localidad
	  Salida:       TRUE si est  en esa localidad
			FALSE si no est  en esa localidad
****************************************************************************/
BOOLEAN at(BYTE locno)
{
if(var[1]==locno) return(TRUE);

return(FALSE);
}

/****************************************************************************
	NOTAT: comprueba que no est  en una localidad.
	  Entrada:      'locno' n£mero de localidad
	  Salida:       TRUE si no est  en esa localidad
			FALSE si est  en esa localidad
****************************************************************************/
BOOLEAN notat(BYTE locno)
{
return(TRUE-at(locno));
}

/****************************************************************************
	ATGT: comprueba si est  en una localidad superior a la dada.
	  Entrada:      'locno' n£mero de localidad
	  Salida:       TRUE si est  en una localidad superior
			FALSE si est  en una localidad inferior o igual
****************************************************************************/
BOOLEAN atgt(BYTE locno)
{
if(var[1]>locno) return(TRUE);

return(FALSE);
}

/****************************************************************************
	ATLT: comprueba si est  en una localidad inferior a la dada.
	  Entrada:      'locno' n£mero de localidad
	  Salida:       TRUE si est  en una localidad inferior
			FALSE si est  en una localidad superior o igual
****************************************************************************/
BOOLEAN atlt(BYTE locno)
{
if(var[1]<locno) return(TRUE);

return(FALSE);
}

/****************************************************************************
	ADJECT1: comprueba el primer adjetivo de la sentencia l¢gica.
	  Entrada:      'adj' n£mero de adjetivo
	  Salida:       TRUE si el adjetivo 1 en la sentencia l¢gica (var[4])
			es el dado
			FALSE si no
****************************************************************************/
BOOLEAN adject1(BYTE adj)
{
if(var[4]==adj) return(TRUE);

return(FALSE);
}

/****************************************************************************
	NOUN2: comprueba el segundo nombre de la sentencia l¢gica.
	  Entrada:      'nomb' n£mero de nombre
	  Salida:       TRUE si el nombre 2 en la sentencia l¢gica (var[5])
			es el dado
			FALSE si no
****************************************************************************/
BOOLEAN noun2(BYTE nomb)
{
if(var[5]==nomb) return(TRUE);

return(FALSE);
}

/****************************************************************************
	ADJECT2: comprueba el segundo adjetivo de la sentencia l¢gica.
	  Entrada:      'adj' n£mero de adjetivo
	  Salida:       TRUE si el adjetivo 2 en la sentencia l¢gica (var[6])
			es el dado
			FALSE si no
****************************************************************************/
BOOLEAN adject2(BYTE adj)
{
if(var[6]==adj) return(TRUE);

return(FALSE);
}

/****************************************************************************
	LISTAT: lista los objetos presentes en una localidad.
	  Entrada:      'locno' n£mero de localidad
****************************************************************************/
BOOLEAN listat(BYTE locno)
{
BYTE i, j=0;
char *po;
BYTE obl[MAX_OBJ];      /* tabla para guardar n£meros de objetos a listar */

/* si se trata de localidad actual sustituye por su valor */
if(locno==LOC_ACTUAL) locno=var[1];

/* recorre toda la tabla de objetos */
for(i=0; i<cab.num_obj; i++) {
	/* almacena n£meros de objetos en localidad especificada */
	if(loc_obj[i]==locno) {
		obl[j]=i;
		j++;
	}
}

/* si no se encontr¢ ning£n objeto */
if(!j) sysmess(10);                     /* ' nada.' */
else {
	/* recupera objetos guardados */
	for(i=0; i<j; i++) {
		/* puntero a texto objeto */
		po=tab_obj+tab_desp_obj[obl[i]]+6;

		/* si bandera 1 est  activada imprime en formato columna */
		if(notzero(1)) {
			w_imps(po,&w[var[0]]);
			w_impc('\n',&w[var[0]]);
		}
		else {
			/* imprime objeto */
			w_imps(po,&w[var[0]]);
			/* si final de la lista */
			if(i==(BYTE)(j-1)) sysmess(13);
			/* uni¢n entre los dos £ltimos objetos de lista */
			else if(i==(BYTE)(j-2)) sysmess(12);
			/* separaci¢n entre objs. */
			else sysmess(11);
		}
	}
}

return(TRUE);
}

/****************************************************************************
	ISAT: comprueba si un objeto est  en una localidad.
	  Entrada:      'nobj' n£mero de objeto
			'locno' n£mero de localidad
	  Salida:       TRUE si el objeto est  en la localidad
			FALSE si no est  en la localidad
****************************************************************************/
BOOLEAN isat(BYTE nobj, BYTE locno)
{
/* si se trata de localidad actual sustituye por su valor */
if(locno==LOC_ACTUAL) locno=var[1];

if(loc_obj[nobj]==locno) return(TRUE);

return(FALSE);
}

/****************************************************************************
	ISNOTAT: comprueba si un objeto no est  en una localidad.
	  Entrada:      'nobj' n£mero de objeto
			'locno' n£mero de localidad
	  Salida:       TRUE si el objeto no est  en la localidad
			FALSE si est  en la localidad
****************************************************************************/
BOOLEAN isnotat(BYTE nobj, BYTE locno)
{
return(TRUE-isat(nobj,locno));
}

/****************************************************************************
	PRESENT: comprueba si un objeto est  presente (en localidad actual,
	  cogido o puesto).
	  Entrada:      'nobj' n£mero de objeto
	  Salida:       TRUE si el objeto est  presente
			FALSE si no est  presente
****************************************************************************/
BOOLEAN present(BYTE nobj)
{
if(isat(nobj,LOC_ACTUAL) || isat(nobj,COGIDO) || isat(nobj,PUESTO))
  return(TRUE);

return(FALSE);
}

/****************************************************************************
	ABSENT: comprueba si un objeto no est  presente.
	  Entrada:      'nobj' n£mero de objeto
	  Salida:       TRUE si el objeto no est  presente
			FALSE si est  presente
****************************************************************************/
BOOLEAN absent(BYTE nobj)
{
return(TRUE-present(nobj));
}

/****************************************************************************
	WORN: comprueba si un objeto est  puesto.
	  Entrada:      'nobj' n£mero de objeto
	  Salida:       TRUE si el objeto est  puesto
			FALSE si no est  puesto
****************************************************************************/
BOOLEAN worn(BYTE nobj)
{
if(isat(nobj,PUESTO)) return(TRUE);

return(FALSE);
}

/****************************************************************************
	NOTWORN: comprueba si un objeto no est  puesto.
	  Entrada:      'nobj' n£mero de objeto
	  Salida:       TRUE si el objeto no est  puesto
			FALSE si est  puesto
****************************************************************************/
BOOLEAN notworn(BYTE nobj)
{
return(TRUE-worn(nobj));
}

/****************************************************************************
	CARRIED: comprueba si un objeto est  cogido.
	  Entrada:      'nobj' n£mero de objeto
	  Salida:       TRUE si el objeto est  cogido
			FALSE si no est  cogido
****************************************************************************/
BOOLEAN carried(BYTE nobj)
{
if(isat(nobj,COGIDO)) return(TRUE);

return(FALSE);
}

/****************************************************************************
	NOTCARR: comprueba si un objeto no est  cogido.
	  Entrada:      'nobj' n£mero de objeto
	  Salida:       TRUE si el objeto no est  cogido
			FALSE si est  cogido
****************************************************************************/
BOOLEAN notcarr(BYTE nobj)
{
return(TRUE-carried(nobj));
}

/****************************************************************************
	WEAR: pone un objeto que sea una prenda.
	  Entrada:      'nobj' n£mero de objeto
	  Salida:       TRUE si se pudo poner el objeto
			FALSE si no se pudo poner
			NOTA: la variable 8 contendr  el n£mero del objeto
			a la salida
****************************************************************************/
BOOLEAN wear(BYTE nobj)
{
char *po;

/* si el objeto no existe */
if(nobj>=cab.num_obj) {
	sysmess(5);                     /* 'No tienes eso.' */
	return(FALSE);
}

/* coloca el n£mero del objeto en la variable 8 */
var[8]=nobj;

/* puntero a banderas1 de objeto */
po=tab_obj+tab_desp_obj[nobj]+3;

/* si el objeto est  puesto */
if(worn(nobj)) {
	sysmess(16);                    /* 'Ya llevas puesto _.' */
	return(FALSE);                  /* debe hacer un DONE */
}

/* si el objeto no est  aqu¡ */
if(absent(nobj)) {
	sysmess(1);                     /* 'Aqu¡ no est  eso.' */
	return(FALSE);                  /* debe hacer un DONE */
}

/* si el objeto no est  cogido */
if(notcarr(nobj)) {
	sysmess(5);                     /* 'No tienes eso.' */
	return(FALSE);
}

/* si no es una prenda */
if(!(*po & 0x01)) {
	sysmess(17);                    /* 'No puedes ponerte _.' */
	return(FALSE);                  /* debe hacer un DONE */
}

/* se pone la prenda */
place(nobj,PUESTO);
sysmess(18);                            /* 'Te pones _.' */

return(TRUE);
}

/****************************************************************************
	REMOVE: quita un objeto que sea una prenda.
	  Entrada:      'nobj' n£mero de objeto
	  Salida:       TRUE si se pudo quitar el objeto
			FALSE si no se pudo quitar
			NOTA: la variable 8 contendr  el n£mero del objeto
			a la salida
****************************************************************************/
BOOLEAN remove1(BYTE nobj)
{

/* si el objeto no existe */
if(nobj>=cab.num_obj) {
	sysmess(19);                    /* 'No llevas puesto eso.' */
	return(FALSE);
}

/* coloca el n£mero del objeto en la variable 8 */
var[8]=nobj;

/* si no lo lleva puesto */
if(notworn(nobj)) {
	sysmess(19);                    /* 'No llevas puesto eso.' */
	return(FALSE);                  /* debe hacer un DONE */
}

/* pasa el objeto a cogido */
place(nobj,COGIDO);
sysmess(20);                            /* 'Te quitas _.' */

return(TRUE);
}

/****************************************************************************
	CREATE: pasa un objeto no creado a la localidad actual.
	  Entrada:      'nobj' n£mero de objeto
****************************************************************************/
BOOLEAN create(BYTE nobj)
{
if(isat(nobj,NO_CREADO)) place(nobj,LOC_ACTUAL);

return(TRUE);
}

/****************************************************************************
	DESTROY: pasa un objeto a no creado.
	  Entrada:      'nobj' n£mero de objeto
****************************************************************************/
BOOLEAN destroy(BYTE nobj)
{
place(nobj,NO_CREADO);

return(TRUE);
}

/****************************************************************************
	SWAP: intercambia dos objetos.
	  Entrada:      'nobj1' n£mero de objeto 1
			'nobj2' n£mero de objeto 2
****************************************************************************/
BOOLEAN swap(BYTE nobj1,BYTE nobj2)
{
BYTE locobj2;

locobj2=loc_obj[nobj2];         /* guarda localidad de 2§ objeto */
place(nobj2,loc_obj[nobj1]);    /* pasa 2§ a localidad del 1§ */
place(nobj1,locobj2);           /* pasa 1§ a localidad del 2§ */

return(TRUE);
}

/****************************************************************************
	RESTART: salta al inicio del proceso 0.
****************************************************************************/
BOOLEAN restart(void)
{
/* inicializa puntero de pila */
ptrp=0;

/* proceso actual 0 */
pro_act=0;

/* puntero a inicio Proceso 0, -1 que luego se suma */
ptr_proc=tab_pro+tab_desp_pro[0]-1;

/* no debe ajustar ptr_proc para siguiente entrada */
nueva_ent=TRUE;

return(FALSE);
}

/****************************************************************************
	WHATO: devuelve el n£mero de objeto que se corresponde con el nombre1,
	  adjetivo1 de la sentencia l¢gica actual.
	  Entrada:      var[3] nombre
			var[4] adjetivo
	  Salida:       var[8] n£mero de objeto
****************************************************************************/
BOOLEAN whato(void)
{
BYTE i;
char *po;

/* inicializa a n£mero de objeto no v lido */
var[8]=255;

for(i=0; i<cab.num_obj; i++) {
	/* puntero a nombre-adjetivo objeto */
	po=tab_obj+tab_desp_obj[i];

	/* si encuentra el objeto, coloca su n£mero en var[8] */
	if((var[3]==(BYTE)*po) && ((var[4]==NO_PAL) ||
	  (var[4]==(BYTE)*(po+1)))) {
		var[8]=i;
		break;
	}
}

return(TRUE);
}

/****************************************************************************
	MOVE: actualiza el contenido de una variable de acuerdo a su
	  contenido actual, a la sentencia l¢gica y a la tabla de conexiones,
	  para que contenga el n£mero de localidad con la que conecta una
	  dada por medio de la palabra de movimiento de la sentencia l¢gica.
	  Entrada:      'nv' n£mero de variable (cuyo contenido es el n£mero
			de una localidad v lida)
			var[2] y var[3] sentencia l¢gica
	  Salida:       TRUE si hay conexi¢n y 'nv' modificada para que
			contenga un n£mero de localidad que conecta con la
			dada por medio de la tabla de conexiones
			FALSE si no hay conexi¢n en esa direcci¢n y 'nv' sin
			modificar
****************************************************************************/
BOOLEAN move(BYTE nv)
{
BYTE *pc;

/* puntero a conexiones de localidad var[nv] */
pc=tab_conx+tab_desp_conx[var[nv]];

/* mientras haya conexiones */
while(*pc) {
	if((var[2]==*pc && var[3]==NO_PAL) || (var[2]==NO_PAL && var[3]==*pc)
	  || (var[2]==*pc && var[3]==*pc)) {
		/* guarda n£mero localidad hacia la que conecta y sale */
		var[nv]=*(pc+1);
		return(TRUE);
	}
	/* siguiente conexi¢n */
	pc+=2;
}

/* si no encontr¢ conexi¢n */
return(FALSE);
}

/****************************************************************************
	ISMOV: comprueba si la sentencia l¢gica es de movimiento
	  (movim. - NO_PAL, NO_PAL - movim. o movim. - movim.).
	  Entrada:      var[2] y var[3] con nombre-verbo
	  Salida:       TRUE si es sentencia l¢gica de movimiento
			FALSE si no lo es
****************************************************************************/
BOOLEAN ismov(void)
{
if((var[2]<cab.v_mov && var[3]==NO_PAL) ||
  (var[2]==NO_PAL && var[3]<cab.v_mov) ||
  (var[2]<cab.v_mov && var[3]<cab.v_mov)) return(TRUE);

return(FALSE);
}

/****************************************************************************
	GOTO: va a una localidad especificada.
	  Entrada:      'locno' n£mero de localidad
	  Salida:       var[1] conteniendo el n£mero de nueva localidad
			(si 'locno' es localidad no v lida, el contenido de
			var[1] no se modifica)
****************************************************************************/
BOOLEAN goto1(BYTE locno)
{
if(locno<cab.num_loc) var[1]=locno;

return(TRUE);
}

/****************************************************************************
	PRINT: imprime el contenido de una variable en la posici¢n actual.
	  Entrada:      'nv' n£mero de variable
****************************************************************************/
BOOLEAN print(BYTE nv)
{
w_impn((unsigned)var[nv],&w[var[0]]);

return(TRUE);
}

/****************************************************************************
	DPRINT: imprime el contenido de dos variables consecutivas como un
	  n£mero de 16 bits en la posici¢n actual.
	  Entrada:      'nv' n£mero de la primera variable
			NOTA: si nv=255 el resultado ser  impredecible
****************************************************************************/
BOOLEAN dprint(BYTE nv)
{
unsigned num;

num=(var[nv]*256)+var[nv+1];
w_impn(num,&w[var[0]]);

return(TRUE);
}

/****************************************************************************
	CLS: borra la pantalla.
****************************************************************************/
BOOLEAN cls(void)
{
_clearscreen(_GCLEARSCREEN);

return(TRUE);
}

/****************************************************************************
	ANYKEY: espera hasta que se pulse una tecla.
****************************************************************************/
BOOLEAN anykey(void)
{
sysmess(22);                    /* 'Pulsa una tecla.' */
w_lee_tecla();

return(TRUE);
}

/****************************************************************************
	PAUSE: realiza una pausa de una duraci¢n determinada o hasta que se
	  pulse una tecla.
	  Entrada:      'pau' valor de la pausa en d‚cimas de segundo
****************************************************************************/
BOOLEAN pause(BYTE pau)
{
/* hace la pausa */
pausa((clock_t)(pau*100));

/* saca tecla del buffer */
if(_bios_keybrd(_KEYBRD_READY)) _bios_keybrd(_KEYBRD_READ);

return(TRUE);
}

/****************************************************************************
	LISTOBJ: lista los objetos de la localidad actual.
****************************************************************************/
BOOLEAN listobj(void)
{
/* si no est  oscuro o hay luz lista objetos */
if(zero(0) || light()) {
	sysmess(9);                     /* 'Tambi‚n puedes ver: ' */
	listat(var[1]);
}

return(TRUE);
}

/****************************************************************************
	FIRSTO: coloca el puntero NEXTO al principio de la tabla de objetos.
****************************************************************************/
BOOLEAN firsto(void)
{
/* coloca puntero al inicio de la tabla - 1 */
ptr_nexto=-1;
/* indica que est  activo bucle DOALL */
doall=TRUE;

return(TRUE);
}

/****************************************************************************
	NEXTO: mueve el puntero NEXTO al siguiente objeto que est‚ en la
	  localidad especificada.
	  Entrada:      'locno' n£mero de localidad
	  Salida:       variables 3 y 4 con el nombre y adjetivo del
			siguiente objeto encontrado (si el objeto encontrado
			es el £ltimo pone 'doall' a FALSE)
****************************************************************************/
BOOLEAN nexto(BYTE locno)
{
char *po;
int i;

/* si no est  en bucle DOALL sale */
if(!doall) return(TRUE);

/* si se refiere a la localidad actual sustituye por su valor */
if(locno==LOC_ACTUAL) locno=var[1];

while(1) {
	ptr_nexto++;
	/* si lleg¢ al final cancela bucle DOALL */
	if(ptr_nexto>=(int)cab.num_obj) {
		doall=FALSE;
		return(TRUE);
	}
	/* si objeto est  en localidad */
	if(loc_obj[ptr_nexto]==locno) {
		/* puntero a objeto */
		po=tab_obj+tab_desp_obj[ptr_nexto];
		/* coge nombre y adjetivo de objeto */
		var[3]=(BYTE)*po;
		var[4]=(BYTE)*(po+1);
		/* mira si es el £timo objeto en la localidad indicada */
		for(i=ptr_nexto+1; i<(int)cab.num_obj; i++) {
			if(loc_obj[i]==locno) break;
		}
		/* si es £ltimo objeto desactiva bucle DOALL */
		if(i>=(int)cab.num_obj) doall=FALSE;
		return(TRUE);
	}
}

}

/****************************************************************************
	SYNONYM: coloca el verbo-nombre dado en las variables 2 y 3,
	  si alguno es NO_PAL no realiza la sustituci¢n.
****************************************************************************/
BOOLEAN synonym(BYTE verb, BYTE nomb)
{
if(verb!=NO_PAL) var[2]=verb;
if(nomb!=NO_PAL) var[3]=nomb;

return(TRUE);
}

/****************************************************************************
	HASAT: comprueba si el objeto actual (cuyo n£mero est  en la variable
	  del sistema 8) tiene activada una bandera de usuario.
	  Entrada:      'val' n£mero de bandera de usuario a comprobar
			(0 a 17), 16 comprueba si es PRENDA y 17 si FUENTE
			DE LUZ
	  Salida:       TRUE si la bandera de usuario est  a 1
			FALSE si est  a 0
****************************************************************************/
BOOLEAN hasat(BYTE val)
{
char *po;
unsigned flags2, masc=0x8000;

/* puntero a banderas2 de objeto */
po=tab_obj+tab_desp_obj[var[8]]+4;

/* comprobar PRENDA o FUENTE LUZ */
if(val==16 || val==17) {
	/* puntero a banderas1 de objeto */
	po--;
	/* comprueba PRENDA */
	if((val==16) && (*po & 0x01)) return(TRUE);
	/* comprueba LUZ */
	if((val==17) && (*po & 0x02)) return(TRUE);
	return(FALSE);
}

/* coge las banderas de usuario */
flags2=(*po*256)+*(po+1);

/* desplaza m scara */
masc>>=val;

/* si es 1 el bit correspondiente */
if(flags2 & masc) return(TRUE);

return(FALSE);
}

/****************************************************************************
	HASNAT: comprueba si el objeto actual (cuyo n£mero est  en la
	  variable del sistema 8) no tiene activada una bandera de usuario.
	  Entrada:      'val' n£mero de bandera de usuario a comprobar
			(0 a 17), 16 comprueba si es PRENDA y 17 si FUENTE
			DE LUZ
	  Salida:       TRUE si la bandera de usuario est  a 0
			FALSE si est  a 1
****************************************************************************/
BOOLEAN hasnat(BYTE val)
{
return(TRUE-hasat(val));
}

/****************************************************************************
	LIGHT: comprueba si hay presente una fuente de luz.
	  Salida:       TRUE si hay presente una fuente de luz
			FALSE si no
****************************************************************************/
BOOLEAN light(void)
{
BYTE i;
char *po;

/* recorre tabla de objetos */
for(i=0; i<cab.num_obj; i++) {
	/* puntero a banderas1 de objeto */
	po=tab_obj+tab_desp_obj[i]+3;
	/* si es fuente de luz y adem s est  presente, sale con TRUE */
	if(*po & 0x02) if(present(i)) return(TRUE);
}

/* si no hay presente ninguna fuente de luz */
return(FALSE);
}

/****************************************************************************
	NOLIGHT: comprueba si no hay presente ninguna fuente de luz.
	  Salida:       TRUE si no hay presente ninguna fuente de luz
			FALSE si hay presente al menos una fuente de luz
****************************************************************************/
BOOLEAN nolight(void)
{
return(TRUE-light());
}

/****************************************************************************
	RANDOM: genera n£meros aleatorios.
	  Entrada:      'varno' n£mero de variable que contendr  el n£mero
			aleatorio
			'rnd' l¡mite de n£mero aleatorio
	  Salida:       'varno' conteniendo un n£mero aleatorio entre 0 y
			'rnd'-1
****************************************************************************/
BOOLEAN random(BYTE varno, BYTE rnd)
{
int x;

x=32767/rnd;
var[varno]=(BYTE)(rand()/x);

return(TRUE);
}

/****************************************************************************
	SEED: coloca el punto de inicio del generador de n£meros aleatorios.
	  Entrada:      'seed' punto de inicio dentro de la serie de n£meros
			aleatorios (si es 1 reinicializa el generador)
****************************************************************************/
BOOLEAN seed(BYTE seed)
{
srand((unsigned)seed);

return(TRUE);
}

/****************************************************************************
	PUTO: coloca el objeto actual cuyo n£mero est  en la variable 8 en
	  una localidad.
	  Entrada:      'nloc' n£mero de localidad
****************************************************************************/
BOOLEAN puto(BYTE nloc)
{
place(var[8],nloc);

return(TRUE);
}

/****************************************************************************
	INKEY: coloca en las variables 9 y 10 el par de c¢digos ASCII IBM
	  de la £ltima tecla pulsada (si se puls¢ alguna).
	  Salida:       TRUE si se puls¢ una tecla y adem s...
			 -variable 9 conteniendo 1er c¢digo ASCII IBM (c¢digo
			  ASCII del car cter, si es distinto de 0)
			 -variable 10 conteniendo 2o c¢digo ASCII IBM (c¢digo
			  de scan de la tecla pulsada)
			FALSE si no se puls¢ ninguna tecla (deja sin
			modificar las variables 9 y 10)
****************************************************************************/
BOOLEAN inkey(void)
{
unsigned tecla;

/* si hay tecla esperando */
if(_bios_keybrd(_KEYBRD_READY)) {
	/* recoge c¢digos tecla pulsada */
	tecla=_bios_keybrd(_KEYBRD_READ);
	/* c¢digo ASCII, byte bajo */
	var[9]=(BYTE)(tecla & 0x00ff);
	/* c¢digo scan, byte alto */
	var[10]=(BYTE)(tecla >> 8);
	return(TRUE);
}

return(FALSE);
}

/****************************************************************************
	COPYOV: copia el n£mero de localidad en la que est  el objeto
	  referenciado en una variable dada.
	  Entrada:      'nobj' n£mero de objeto
			'varno' n£mero de variable
****************************************************************************/
BOOLEAN copyov(BYTE nobj, BYTE varno)
{
var[varno]=loc_obj[nobj];

return(TRUE);
}

/****************************************************************************
	CHANCE: comprueba una probabilidad en tanto por ciento.
	  Entrada:      'rnd' probabilidad de 0 a 100
	  Salida:       TRUE si el n£mero aleatorio generado internamente es
			menor o igual que rnd
			FALSE si es mayor que rnd
****************************************************************************/
BOOLEAN chance(BYTE rnd)
{
BYTE chc;

/* n£mero aleatorio entre 0 y 100 */
chc=(BYTE)(rand()/327);

if(chc>rnd) return(FALSE);

return(TRUE);
}

/****************************************************************************
	RAMSAVE: graba en uno de los bancos de ram disponibles el estado
	  actual (variables, banderas y posici¢n actual de objetos).
	  Entrada:      'banco' n£mero de banco de memoria a usar (0 o 1)
****************************************************************************/
BOOLEAN ramsave(BYTE banco)
{
int i;

/* marca banco usado */
ram[banco].usado=TRUE;

/* guarda variables */
for(i=0; i<VARS; i++) ram[banco].bram[i]=var[i];
/* guarda banderas */
for(i=0; i<BANDS; i++) ram[banco].bram[VARS+i]=flag[i];
/* guarda localidades actuales de los objetos */
for(i=0; i<MAX_OBJ; i++) ram[banco].bram[VARS+BANDS+i]=loc_obj[i];

return(TRUE);
}

/****************************************************************************
	RAMLOAD: recupera una posici¢n guardada con RAMSAVE.
	  Entrada:      'banco' n£mero de banco de memoria a usar (0 o 1)
			'vtop'  m ximo n£mero de variable a recuperar (se
			recuperar  desde la 0 hasta 'vtop' inclusive)
			'ftop' m ximo n£mero de bandera a recuperar (se
			recuperar  desde la 0 hasta ftop inclusive)
	  Salida:       TRUE si se pudo ejecutar RAMLOAD
			FALSE si el banco indicado no fu‚ usado antes por
			RAMSAVE
****************************************************************************/
BOOLEAN ramload(BYTE banco, BYTE vtop, BYTE ftop)
{
int i, nbyte, nbit;
BYTE mascara;

/* mira si el banco ha sido usado */
if(!ram[banco].usado) return(FALSE);

/* recupera variables */
for(i=0; i<=(int)vtop; i++) var[i]=ram[banco].bram[i];
/* recupera banderas */
for(i=0; i<=(int)ftop; i++) {
	nbyte=i/8;
	nbit=i%8;
	mascara=0x80;
	mascara>>=nbit;
	if(ram[banco].bram[VARS+nbyte] & mascara) set((BYTE)i);
	else clear((BYTE)i);
}
/* recupera localidades actuales de los objetos */
for(i=0; i<MAX_OBJ; i++) loc_obj[i]=ram[banco].bram[VARS+BANDS+i];

return(TRUE);
}

/****************************************************************************
	ABILITY: designa el n£mero de objetos m ximo que puede ser llevado.
	  Entrada:      'nobjs' n£mero de objetos m ximo (0 ilimitados)
****************************************************************************/
BOOLEAN ability(BYTE nobjs)
{
/* m ximo n£mero de objetos en variable 7 */
let(7,nobjs);

return(TRUE);
}

/****************************************************************************
	AUTOG: coge el objeto cuyo nombre-adjetivo est n en las variables
	  3 y 4 respectivamente (es como WHATO seguido de GET [8]).
	  Salida:       TRUE si se pudo coger el objeto
			FALSE si no se pudo coger
****************************************************************************/
BOOLEAN autog(void)
{
whato();

return(get(var[8]));
}

/****************************************************************************
	AUTOD: deja el objeto cuyo nombre-adjetivo est n en las variables
	  3 y 4 respectivamente (es como WHATO seguido de DROP [8]).
	  Salida:       TRUE si se pudo dejar el objeto
			FALSE si no se pudo dejar
****************************************************************************/
BOOLEAN autod(void)
{
whato();

return(drop(var[8]));
}

/****************************************************************************
	AUTOW: pone el objeto cuyo nombre-adjetivo est n en las variables
	  3 y 4 respectivamente (es como WHATO seguido de WEAR [8]).
	  Salida:       TRUE si se pudo poner el objeto
			FALSE si no se pudo poner
****************************************************************************/
BOOLEAN autow(void)
{
whato();

return(wear(var[8]));
}

/****************************************************************************
	AUTOR: quita el objeto cuyo nombre-adjetivo est n en las variables
	  3 y 4 respectivamente (es como WHATO seguido de REMOVE [8]).
	  Salida:       TRUE si se pudo quitar el objeto
			FALSE si no se pudo quitar
****************************************************************************/
BOOLEAN autor(void)
{
whato();

return(remove1(var[8]));
}

/****************************************************************************
	ISDOALL: comprueba si se est  ejecutando un bucle DOALL (FIRSTO ha
	  sido ejecutado y NEXTO no ha alcanzado todav¡a £ltimo objeto).
	  Salida:       TRUE si se est  ejecutando un bucle DOALL
			FALSE si no
****************************************************************************/
BOOLEAN isdoall(void)
{
if(doall) return(TRUE);

return(FALSE);
}

/****************************************************************************
	ASK: realiza una pregunta y espera hasta que se introduce la
	  respuesta.
	  Entrada:      'smess1' mensaje del sistema que contiene la pregunta
			'smess2'    "     "     "    con las posibles
			respuestas (cada una de una sola letra y seguidas,
			no importa si en may£sculas o min£sculas), el m ximo
			n£mero de caracteres permitidos es 256 (0 a 255), si
			son m s, la variable 'varno' contendr  resultados
			imprevisibles
			'varno' n£mero de variable que contendr  la respuesta
	  Salida:       'varno'=0 si se tecle¢ 1er car cter de 'smess2', 1 si
			el 2o, 2 si el 3ero, etc...
****************************************************************************/
BOOLEAN ask(BYTE smess1, BYTE smess2, BYTE varno)
{
char *pm;
unsigned key;
BYTE i;
BYTE oldx, oldy;

/* imprime la pregunta */
sysmess(smess1);
/* guarda posici¢n actual del cursor */
oldx=w[var[0]].cwx;
oldy=w[var[0]].cwy;
while(1) {
	/* recupera coordenadas */
	w[var[0]].cwx=oldx;
	w[var[0]].cwy=oldy;

	/* imprime cursor */
	w_impc(*(tab_msy+tab_desp_msy[7]),&w[var[0]]);

	/* recupera coordenadas */
	w[var[0]].cwx=oldx;
	w[var[0]].cwy=oldy;

	key=mayuscula((char)w_lee_tecla());
	/* imprime tecla pulsada */
	w_impc((char)key,&w[var[0]]);

	/* comienza b£squeda */
	i=0;
	/* puntero a mensaje con las respuestas */
	pm=tab_msy+tab_desp_msy[smess2];

	/* analiza hasta final de cadena */
	while(*pm) {
		if(mayuscula(*pm)==(char)key) {
			/* almacena n£mero de car cter de smess2 y sale */
			var[varno]=i;
			return(TRUE);
		}
		/* pasa al siguiente car cter de smess2 */
		pm++;
		i++;
	}
}
}

/****************************************************************************
	QUIT: presenta el mensaje del sistema 24 (¨Est s seguro?) y pregunta
	  para abandonar.
	  Salida:       TRUE si se responde con el 1er car cter del mensaje
			del sistema 25
			FALSE si se responde con el 2o car cter del mensaje
			del sistema 25
			NOTA: se usa la variable 11
****************************************************************************/
BOOLEAN quit(void)
{
/* hace pregunta '¨Est s seguro?' */
ask(24,25,11);
newline();

/* si respondi¢ con 1er car cter */
if(!var[11]) return(TRUE);

return(FALSE);
}

/****************************************************************************
	SAVE: guarda la posici¢n actual en disco (variables, banderas y
	  posici¢n actual de objetos).
****************************************************************************/
BOOLEAN save(void)
{
int h_save;

sysmess(26);                    /* 'Nombre del fichero: ' */
w_input(f_sl,MAXLONG,*(tab_msy+tab_desp_msy[7]),&w[var[0]]);
newline();

/* si el fichero ya existe */
if(!access(f_sl,0)) {
	ask(27,25,11);          /* 'Fichero ya existe. ¨Quieres continuar? ' */
	newline();
	/* si respondi¢ con 2o car cter del mensaje del sistema 25 */
	if(var[11]) return(TRUE);
}

h_save=open(f_sl,O_CREAT|O_TRUNC|O_BINARY|O_RDWR,S_IREAD|S_IWRITE);

/* si ocurri¢ un error en apertura */
if(h_save==-1) {
	sysmess(28);            /* 'Error de apertura de fichero.' */
	return(TRUE);
}

/* escribe cadena de reconocimiento */
if(write(h_save,SRECON,L_RECON+1)==-1) {
	sysmess(29);            /* 'Error de entrada/salida en fichero.' */
	close(h_save);
	return(TRUE);
}

/* guarda variables */
if(write(h_save,var,VARS)==-1) {
	sysmess(29);            /* 'Error de entrada/salida en fichero.' */
	close(h_save);
	return(TRUE);
}

/* guarda banderas */
if(write(h_save,flag,BANDS)==-1) {
	sysmess(29);            /* 'Error de entrada/salida en fichero.' */
	close(h_save);
	return(TRUE);
}

/* guarda localidades actuales de los objetos */
if(write(h_save,loc_obj,MAX_OBJ)==-1) {
	sysmess(29);            /* 'Error de entrada/salida en fichero.' */
	close(h_save);
	return(TRUE);
}

close(h_save);

return(TRUE);
}

/****************************************************************************
	LOAD: recupera de disco una posici¢n grabada con SAVE.
	  Entrada:      'vtop' m ximo n£mero de variable a recuperar (se
			recuperar  desde la 0 hasta 'vtop' inclusive)
			'ftop' m ximo n£mero de bandera a recuperar (se
			recuperar  desde la 0 hasta 'ftop' inclusive)
****************************************************************************/
BOOLEAN load(BYTE vtop, BYTE ftop)
{
int h_load, i, nbyte, nbit;
BYTE mascara;
char rec_ld[L_RECON+1];
BYTE var_l[VARS];
BYTE flag_l[BANDS];

sysmess(26);                    /* 'Nombre del fichero: ' */
w_input(f_sl,MAXLONG,*(tab_msy+tab_desp_msy[7]),&w[var[0]]);
newline();

h_load=open(f_sl,O_BINARY|O_RDONLY);

/* si curri¢ un error en apertura */
if(h_load==-1) {
	sysmess(28);            /* 'Error de apertura de fichero.' */
	return(TRUE);
}

/* recoge cadena de reconocimiento */
if(read(h_load,rec_ld,L_RECON+1)==-1) {
	sysmess(29);            /* 'Error de entrada/salida en fichero.' */
	close(h_load);
	return(TRUE);
}

/* comprueba cadena de reconocimiento */
if(strcmp(rec_ld,SRECON)) {
	sysmess(30);            /* 'Fichero no v lido.' */
	close(h_load);
	return(TRUE);
}

/* recoge variables */
if(read(h_load,var_l,VARS)==-1) {
	sysmess(29);            /* 'Error de entrada/salida en fichero.' */
	close(h_load);
	return(TRUE);
}

/* recoge banderas */
if(read(h_load,flag_l,BANDS)==-1) {
	sysmess(29);            /* 'Error de entrada/salida en fichero.' */
	close(h_load);
	return(TRUE);
}

/* recoge localidad actual de los objetos */
if(read(h_load,loc_obj,MAX_OBJ)==-1) {
	sysmess(29);            /* 'Error de entrada/salida en fichero.' */
	close(h_load);
	return(TRUE);
}

/* recupera variables */
for(i=0; i<=(int)vtop; i++) var[i]=var_l[i];

/* recupera banderas */
for(i=0; i<=(int)ftop; i++) {
	nbyte=i/8;
	nbit=i%8;
	mascara=0x80;
	mascara>>=nbit;
	if(flag_l[nbyte] & mascara) set((BYTE)i);
	else clear((BYTE)i);
}
close(h_load);

return(TRUE);
}

/****************************************************************************
	EXIT: sale al sistema operativo o reinicia.
	  Entrada:      'ex' si es 0 reinicia, si es 1 sale al sistema
			operativo
****************************************************************************/
BOOLEAN exit1(BYTE ex)
{
/* sale al sistema operativo si 'ex' es 1 */
if(ex==1) m_err(0,"",1);

/* reinicializaci¢n */
/* borra la pantalla */
cls();
/* reinicializa variables */
inic();
/* se pone al inicio de proceso 0 */
restart();
/* -1 a ptr_proc (y -1 del restart), luego se sumar n 2 */
ptr_proc--;

return(FALSE);
}

/****************************************************************************
	END: pregunta para otra partida o salir. Si se responde con el primer
	  car cter del mensaje del sistema 25, ejecuta un EXIT 0 (reiniciali-
	  zaci¢n). Si no sale al sistema operativo.
****************************************************************************/
BOOLEAN end1(void)
{
ask(31,25,11);                          /* '¨Lo intentas de nuevo? ' */
newline();

/* si respondi¢ con 1er car cter del mensaje del sistema 25 */
if(!var[11]) {
	/* reinicializaci¢n */
	exit1(0);
	/* +1 a ptr_proc (-2 del exit1), luego se sumar  1 */
	ptr_proc++;
	return(FALSE);
}

/* sale al sistema operativo */
exit1(1);
}

/****************************************************************************
	PRINTAT: coloca el cursor en una posici¢n dada de la ventana actual.
	  Entrada:      'y', 'x' coordenada del cursor (fila, columna); si
			salen de los l¡mites de la ventana el cursor se
			colocar  en la esquina superior izquierda (0,0)
****************************************************************************/
BOOLEAN printat(BYTE y, BYTE x)
{
/* si se sale de ventana coloca cursor en esquina superior izquierda */
if((y>(BYTE)(w[var[0]].lyi-1)) || (x>(BYTE)(w[var[0]].lxi-1))) y=x=0;

w[var[0]].cwy=y;
w[var[0]].cwx=x;

return(TRUE);
}

/****************************************************************************
	SAVEAT: almacena la posici¢n de impresi¢n de la ventana actual. Cada
	  ventana tiene sus propias posiciones de impresi¢n almacenadas.
****************************************************************************/
BOOLEAN saveat(void)
{
w[var[0]].cwxs=w[var[0]].cwx;
w[var[0]].cwys=w[var[0]].cwy;

return(TRUE);
}

/****************************************************************************
	BACKAT: recupera la posici¢n de impresi¢n guardada por el £ltimo
	  SAVEAT en la ventana actual.
	  NOTA: si no se ejecut¢ ning£n SAVEAT, la posici¢n de impresi¢n
	  recuperada ser  la (0,0).
****************************************************************************/
BOOLEAN backat(void)
{
w[var[0]].cwx=w[var[0]].cwxs;
w[var[0]].cwy=w[var[0]].cwys;

return(TRUE);
}

/****************************************************************************
	NEWTEXT: deshecha el resto de la l¡nea de input que a£n queda por
	  analizar y coloca el puntero al final de la misma.
****************************************************************************/
BOOLEAN newtext(void)
{
/* coloca puntero al final de la frase */
for(; *lin; lin++);
/* indica que no queda m s por analizar */
mas_texto=TRUE;

return(TRUE);
}

/****************************************************************************
	PRINTC: imprime un car cter en la posici¢n actual del cursor y dentro
	  de la ventana activa.
	  Entrada:      'car' c¢digo ASCII del car cter a imprimir
****************************************************************************/
BOOLEAN printc(BYTE car)
{
/* imprime el car cter en la ventana activa */
w_impc(car,&w[var[0]]);

return(TRUE);
}

/****************************************************************************
	INK: selecciona el color temporal de la tinta en la ventana activa.
	  Todos los textos de esa ventana se imprimir n con ese color de
	  tinta.
	  Entrada:      'color' n£mero del color 0 a 7
****************************************************************************/
BOOLEAN ink(BYTE color)
{
/* borra el antiguo color de tinta y pone el nuevo */
w[var[0]].colort &= 0xf8;
w[var[0]].colort |= color;

return(TRUE);
}

/****************************************************************************
	PAPER: selecciona el color temporal del fondo en la ventana activa.
	  Todos los textos de esa ventana se imprimir n con ese color de
	  fondo.
	  Entrada:      'color' n£mero del color 0 a 7
****************************************************************************/
BOOLEAN paper(BYTE color)
{
/* borra el antiguo color del fondo y pone el nuevo */
w[var[0]].colort &= 0x8f;
w[var[0]].colort |= (color<<4);

return(TRUE);
}

/****************************************************************************
	BRIGHT: selecciona el atributo de brillo temporal para la ventana
	  activa. Todos los textos de esa ventana se imprimir n con ese
	  brillo.
	  Entrada:      'b' brillo (0=sin brillo, 1=con brillo)
****************************************************************************/
BOOLEAN bright(BYTE b)
{
if(b) w[var[0]].colort |= 0x08;
else w[var[0]].colort &= 0xf7;

return(TRUE);
}

/****************************************************************************
	BLINK: selecciona el atributo de parpadeo temporal para la ventana
	  activa. Todos los textos de esa ventana se imprimir n con ese
	  parpadeo.
	  Entrada:      'b' parpadeo (0=sin parpadeo, 1=con parpadeo)
****************************************************************************/
BOOLEAN blink(BYTE b)
{
if(b) w[var[0]].colort |= 0x80;
else w[var[0]].colort &= 0x7f;

return(TRUE);
}

/****************************************************************************
	COLOR: selecciona un color (fondo, tinta, brillo y parpadeo) temporal
	  para la ventana activa.
	  Entrada:      'col' color
****************************************************************************/
BOOLEAN color(BYTE col)
{
w[var[0]].colort=col;

return(TRUE);
}

/****************************************************************************
	DEBUG: activa o desctiva el paso a paso.
	  Entrada:      'modo' indica si el paso a paso est  activado (1)
			o desactivado (0)
	  NOTA: este condacto s¢lo es activo en el int‚rprete-debugger
****************************************************************************/
BOOLEAN debug(BYTE modo)
{
#if DEBUGGER==ON
if(modo) {
	debugg=TRUE;
	/* indicamos que no es primera l¡nea de */
	/* entrada para que imp_condacto() no */
	/* imprima el verbo-nombre ya que el puntero */
	/* al condacto puede no estar ajustado */
	pra_lin=FALSE;
}
else debugg=FALSE;
#endif

return(TRUE);
}

/****************************************************************************
	WBORDER: define tipo de borde de una ventana.
	  Entrada:      'nw' n£mero de ventana
			'borde' tipo de borde de la ventana
****************************************************************************/
BOOLEAN wborder(BYTE nw, BYTE borde)
{
w_crea(w[nw].wy,w[nw].wx,w[nw].lx,w[nw].ly,w[nw].color,borde,&w[nw]);

return(TRUE);
}

/****************************************************************************
	CHARSET: carga y selecciona un nuevo juego de caracteres.
	  Entrada:      'set' n£mero del juego de caracteres (0-255), si se
			introduce 0, se seleccionar  el juego de caracteres
			por defecto.
	  Salida:       TRUE si se carg¢ la fuente con ‚xito
			FALSE si hubo alg£n error o la tarjeta gr fica no
			soporta cambio del juego de caracteres
	  NOTA: los juegos de caracteres deben estar en ficheros cuyo
	    nombre sea ??????FS.ext siendo 'ext' el n£mero identificador de
	    1 a 255, por ejemplo: SIMPLEFS.1, EPICO1FS.2, ...
	    Este condacto s¢lo tiene efecto en tarjetas gr ficas EGA y VGA
****************************************************************************/
BOOLEAN charset(BYTE set)
{
struct find_t info;
char nombre[13]="??????FS.";
char num[4];

/* si no es tarjeta EGA o VGA no admite cambio de juego de caracteres */
if((vid.adapter!=_EGA) && (vid.adapter!=_VGA)) return(FALSE);

/* si selecciona el juego de caracteres 0, activa las definiciones del BIOS */
if(!set) {
	_asm {
		mov ax,1101h            ; carga definiciones 8x14 del BIOS
		mov bl,0                ; tabla 0 del generador de caracteres
		int 10h
	}
}

/* construye nombre de fichero a¤adiendo extensi¢n */
itoa((int)set,num,10);
strcat(nombre,num);

/* busca el fichero y sale si no lo encontr¢ */
if(_dos_findfirst(nombre,_A_NORMAL,&info)) return(FALSE);

/* el nombre verdadero del fichero estar  en 'info.name' */
/* lo carga y actualiza generador de caracteres */
if(!carga_def(info.name)) return(FALSE);

return(TRUE);
}

/****************************************************************************
	EXTERN: ejecuta un programa externo.
	  Entrada:      'prg' n£mero identificativo del programa a ejecutar
			'par' par metro que se pasar  al programa externo
			(se pasar  el n£mero 0-255 como una cadena de carac-
			teres)
	  Salida:       TRUE si se ejecut¢ con ‚xito
			FALSE si no se pudo ejecutar
			NOTA: la variable 13 contendr  el c¢digo de salida
			del proceso externo que se ejecut¢ (0 si no pudo
			ejecutarse)
	  NOTA: este condacto ejecutar  un fichero cuyo nombre sea
	    'EXTERn' siendo 'n' un n£mero de 0 a 255; as¡ una llamada a
	    EXTERN 0 ejecutar  el fichero EXTER0, EXTERN 128 ejecutar  el
	    fichero EXTER128,...
	    Se buscar  primero un fichero con extensi¢n .COM, si no se
	    encuentra se buscar  un .EXE y finalmente un .BAT
	    Se pasar  el par metro 'par' como si se hubiese tecleado en la
	    l¡nea del DOS: EXTERn par
****************************************************************************/
BOOLEAN extern1(BYTE prg, BYTE par)
{
char nombre[9]="EXTER";
char num[4];
int cod;

/* construye nombre de fichero a¤adiendo n£mero identificativo */
itoa((int)prg,num,10);
strcat(nombre,num);

/* convierte par metro en cadena ASCII */
itoa((int)par,num,10);

/* intenta ejecutar el programa */
cod=spawnl(P_WAIT,nombre,nombre,num,NULL);

/* restaura el modo de v¡deo por si result¢ modificado */
modo_video();

/* pone a cero la variable 13 */
var[13]=0;

if(cod==-1) return(FALSE);
else {
	var[13]=(BYTE)cod;
	return(TRUE);
}
}

