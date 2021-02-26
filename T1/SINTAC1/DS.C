#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <stddef.h>
#include <bios.h>
#include <dos.h>
#include <graph.h>
#include <time.h>
#include <signal.h>
#include <fcntl.h>
#include <io.h>
#include <sys\types.h>
#include <sys\stat.h>
#include "version.h"
#include "sintac.h"

/****************************************/
/************** Constantes **************/
/****************************************/

#define OFF 0
#define ON  1
/* #define DEBUGGER ON */

#define IZQ 0x4b00
#define DER 0x4d00
#define ARR 0x4800
#define ABJ 0x5000
#define F1  0x3b00
#define F2  0x3c00
#define F3  0x3d00
#define F4  0x3e00
#define F5  0x3f00
#define F6  0x4000
#define F7  0x4100
#define F8  0x4200
#define F9  0x4300
#define F10 0x4400
#define COD_IZQ 0xfe
#define COD_DER 0xff
#define COD_ARR 0xfc
#define COD_ABJ 0xfd
#define COD_F1  0xfb
#define COD_F2  0xfa
#define COD_F3  0xf9
#define COD_F4  0xf8
#define COD_F5  0xf7
#define COD_F6  0xf6
#define COD_F7  0xf5
#define COD_F8  0xf4
#define COD_F9  0xf3
#define COD_F10 0xf2

#if DEBUGGER==ON
#define SHIFT_F1 0x5400
#define COD_SHIFT_F1 0xf1
#endif

#define RETURN '\r'
#define BACKSPACE '\b'
#define MAXLONG 128     /* m†xima longitud de la linea de entrada */
#define SCROLL_PAU 500L /* pausa para s°mbolo scroll */
#define CAR_SCROLL (BYTE)'\x1f' /* caracter cuando hay que scrollear */
#define STK 10          /* profundidad de la pila del intÇrprete */

#define SEPARADOR   0
#define FIN_FRASE   1
#define PALABRA     2
#define TERMINACION 3
#define NO_PALABRA  4

#define TRUE  1
#define FALSE 0

/****************************************/
/***** Tipos de datos y estructuras ****/
/****************************************/
struct window {
	BYTE wx,wy;         /* posici¢n de la ventana (esquina sup. izq.) */
	BYTE lx,ly;         /* tama§o de la ventana (ancho, alto) */
	BYTE cwx,cwy;       /* posici¢n actual cursor dentro de ventana */
	BYTE color;         /* color global de la ventana */
	BYTE colort;        /* color temporal de la ventana */
	BYTE scroll;        /* indicador de scroll */
	BYTE cwxs,cwys;     /* pos. cursor guardada con SAVEAT */
};

struct condacto {
	int (*cond)();      /* puntero a funci¢n del condacto */
	BYTE tipo;          /* tipo: 0= void    1=BYTE   2=BYTE,BYTE */
};                          /*       6= BYTE,BYTE,BYTE,BYTE,BYTE,BYTE */

/****************************************/
/********** Variables globales **********/
/****************************************/

#if DEBUGGER==ON
#include "condact.h"    /* tabla de nombre-tipo de condactos */
char lin_cond[81];      /* buffer para impresi¢n de l°neas debug */
char txt_deb1[]="  Proceso: %3u   ";
char txt_deb2[]="## V=variables  B=banderas  S=salir  Otra=ejecutar ##"
		"   \n";
BYTE buf_scr[320];      /* buffer para guardar zona pantalla */
struct window w_jsj={   /* ventana de presentaci¢n */
	19,10,
	40,9,
	0,0,
	48,
	48,
	0,
	0,0,
};
struct window w_deb={   /* ventana de debugger */
	0,0,
	80,2,
	0,0,
	23,
	23,
	0,
	0,0,
};
BYTE far *pro_d;
int indir1, indir2; /* indicadores de indirecci¢n condacto en curso */
int pra_lin=0;      /* =1 si en 1era lin. de una entrada */
unsigned key;
BYTE variable=0;
BYTE bandera=0;
BYTE valor;
int palabra;
int debugg=1;       /* =1 si paso a paso activado */
#endif

FILE *f_in;
struct window w_err={       /* ventana para mensajes de error */
	19,10,
	40,3,
	0,0,
	79,
	79,
	0,
	0,0,
};
char frase[MAXLONG];    /* buffer para guardar frase tecleada */
char f_sl[MAXLONG];     /*    "     "     "    nombre fichero */
char *lin;  /* ptro. auxiliar a 'frase', usado por PARSE1 y PARSE */
int ini_inp=0;  /* se pone a 1 para indicar a PARSE inicio de frase */
int mas_texto=0;    /* usada por parse1 para frases encadenadas */
const char TECLAS[]="ABCDEFGHIJKLMN•OPQRSTUVWXYZ"
		    "abcdefghijklmn§opqrstuvwxyz"
		    "0123456789†Ç°¢£Å.,;:!≠?®\"\'\r\b "
		    "\xfe\xff\xfc\xfd\xfb\xfa\xf9\xf8\xf7\xf6\xf5"
		    "\xf4\xf3\xf2"
#if DEBUGGER==ON
		   "\xf1";      /* si debugger inserta para Shift-F1 */
#else
		   "";          /* si no, no lo inserta */
#endif

const char C_NO_SIG[]=" !≠?®";      /* cars. no significativos */
const char C_SEPAR[]=".,;:\"\'";    /* separadores */
const char CAR_PAL[]="ABCDEFGHIJKLMN•OPQRSTUVWXYZ0123456789";

/** variables para Vocabulario **/
struct palabra vocabulario[NUM_PAL];    /* para almacenar vocabulario */
int pvoc=0;     /* n£mero de £ltima palabra de vocabulario */
BYTE v_mov;     /* guarda m†x. n£m. de verbo de movimiento */
BYTE n_conv;    /* guarda m†x. n£m. de nombre convertible */
BYTE n_prop;    /* guarda m†x. n£m. de nombre propio */

/** variables para Mensajes **/
ptrdiff_t mens[MAX_MSG];    /* tabla de desplaz. de mensajes */
char far *msgs;             /* puntero a inicio de zona de mensajes */
BYTE m_act=0;               /* n£mero de mensajes+1 */

/** variables para Mensajes de Sistema **/
ptrdiff_t mensys[MAX_MSY];  /* tabla de desplaz. de mensajes */
char far *msgsys;           /* puntero a inicio de zona de mensajes */
BYTE m_actsys=0;            /* n£mero de mensajes sistema+1 */

/** variables para Localidades **/
BYTE n_loc=0;               /* n£mero de localidades+1 */
ptrdiff_t locs[MAX_LOC];    /* tabla de desplaz. de texto locs. */
char far *localidades;      /* puntero a inicio de texto de locs. */
/** variables para Conexiones **/
ptrdiff_t conxs[MAX_LOC];   /* tabla de desplaz. de lista de conexiones */
char far *conx;             /* puntero a inicio de zona de conexiones */

/** variables para Objetos **/
BYTE objact=0;              /* n£mero de objetos+1 */
ptrdiff_t objs[MAX_OBJ];    /* tabla de desplaz. de lista de objetos */
char far *obj;              /* puntero a inicio de zona de objetos */

/** variables para Procesos **/
BYTE far *proc;             /* puntero a inicio de zona de procesos */
ptrdiff_t pro[MAX_PRO];     /* tabla de desplaz. de procesos */
BYTE proact=0;              /* num. de proceso actual */

/** variables y tablas **/
#define VARS 256            /* n£mero de variables */
#define BANDS 32            /* n£mero de banderas/8 */
struct window w[N_VENT];    /* tabla para guardar par†metros de ventanas */
BYTE var[VARS];             /* variables del sistema (8 bits) */
BYTE loc_obj[MAX_OBJ];      /* tabla de localidades act. de objetos */
BYTE flag[BANDS];           /* flags del sistema, 256 flags */
BYTE objs_cogidos;          /* num. de objetos cogidos */
BYTE pro_act;               /* num. de proceso actual */
BYTE far *pila1[STK];       /* para guardar direcciones de retorno */
ptrdiff_t pila2[STK];       /*   "     "    deplz. sgte. entrada */
BYTE pila3[STK];            /*   "     "    num. de proc. de llamada */
int ptrp;                   /* puntero de pila */
BYTE far *ptr_proc;         /* puntero auxiliar */
ptrdiff_t sgte_ent;         /* desplazamiento de sgte. entrada */
int resp_act;               /* RESP (=1) o NORESP (=0) */
int nueva_ent;              /* indica que no debe ajustar ptr_proc para */
							/* saltar a sgte. entrada */
int ptr_nexto=-1;           /* puntero NEXTO a tabla de objetos */
int doall=0;                /* indicador de bucle doall */
BYTE ram0_b[VARS+BANDS+MAX_OBJ];    /* banco 0 para RAMSAVE y RAMLOAD */
BYTE ram1_b[VARS+BANDS+MAX_OBJ];    /* banco 1 para RAMSAVE y RAMLOAD */
int ram0=0;                 /* flags de comprobaci¢n para RAMLOAD */
int ram1=0;
BYTE ruptura;               /* indicador de ruptura */

/****************************************/
/************** Prototipos **************/
/****************************************/

#if DEBUGGER==ON
int saca_pal(BYTE num_pal,BYTE tipo_pal);
void imp_condacto(void);
BYTE inp_band(void);
void guarda_screen(void);
void recupera_screen(void);
#endif

void modo_pant(void);
void w_cls(struct window *w);
void w_impc(char c,struct window *w);
void w_imps(char far *s,struct window *w);
void w_impn(unsigned n,struct window *w);
void pon_cur(BYTE fila, BYTE columna);
void scrollw(BYTE f,BYTE c,BYTE ancho,BYTE alto,BYTE color,BYTE lineas);
void impc(BYTE c,BYTE color);
void w_input(char *plin,unsigned maxlong,char cursor,struct window *w);
unsigned lee_tecla(void);
int esta_en(const char *cadena,unsigned c);
void pausa(clock_t wait);
void load_db(char *nombre);
void inic(void);
void frd2(void *buff,size_t tam,size_t cant);
int rd_fich(char far *buff,unsigned tam);
int analiza(char *(*pfrase),BYTE *tipo,BYTE *num);
char mayuscula(char c);
int esta_en_voc(char *pal);
int parse1(void);
void m_err(BYTE x,BYTE y,char *m,int flag);

int process(BYTE prc);
int done(void);
int notdone(void);
int resp(void);
int noresp(void);
int defwin(BYTE nw,BYTE cw,BYTE wx,BYTE wy,BYTE lx,BYTE ly);
int window(BYTE nw);
int clw(BYTE nw);
int let(BYTE nv,BYTE val);
int eq(BYTE nv,BYTE val);
int noteq(BYTE nv,BYTE val);
int lt(BYTE nv,BYTE val);
int gt(BYTE nv,BYTE val);
int mes(BYTE nm);
int newline(void);
int message(BYTE nm);
int sysmess(BYTE nm);
int desc(BYTE nl);
int add(BYTE nv,BYTE val);
int sub(BYTE nv,BYTE val);
int inc(BYTE nv);
int dec(BYTE nv);
int set(BYTE nf);
int clear(BYTE nf);
int zero(BYTE nf);
int notzero(BYTE nf);
int place(BYTE nobj,BYTE nloc);
int get(BYTE nobj);
int drop(BYTE nobj);
int input(void);
int parse(void);
int skip(BYTE lsb,BYTE hsb);
int at(BYTE locno);
int notat(BYTE locno);
int atgt(BYTE locno);
int atlt(BYTE locno);
int adject1(BYTE adj);
int noun2(BYTE nomb);
int adject2(BYTE adj);
int listat(BYTE locno);
int isat(BYTE nobj,BYTE locno);
int isnotat(BYTE nobj,BYTE locno);
int present(BYTE nobj);
int absent(BYTE nobj);
int worn(BYTE nobj);
int notworn(BYTE nobj);
int carried(BYTE nobj);
int notcarr(BYTE nobj);
int wear(BYTE nobj);
int remove1(BYTE nobj);
int create(BYTE nobj);
int destroy(BYTE nobj);
int swap(BYTE nobj1,BYTE nobj2);
int restart(void);
int whato(void);
int move(BYTE nv);
int ismov(void);
int goto1(BYTE locno);
int print(BYTE nv);
int dprint(BYTE nv);
int cls(void);
int anykey(void);
int pause(BYTE pau);
int listobj(void);
int firsto(void);
int nexto(BYTE locno);
int synonym(BYTE verb,BYTE nomb);
int hasat(BYTE val);
int hasnat(BYTE val);
int light(void);
int nolight(void);
int random(BYTE varno,BYTE rnd);
int seed(BYTE seed);
int puto(BYTE nloc);
int inkey(void);
int copyov(BYTE nobj,BYTE varno);
int chance(BYTE rnd);
int ramsave(BYTE banco);
int ramload(BYTE banco,BYTE vtop,BYTE ftop);
int ability(BYTE nobjs);
int autog(void);
int autod(void);
int autow(void);
int autor(void);
int isdoall(void);
int ask(BYTE smess1,BYTE smess2,BYTE varno);
int quit(void);
int save(void);
int load(BYTE vtop,BYTE ftop);
int exit1(BYTE ex);
int end1(void);
int printat(BYTE y,BYTE x);
int saveat(void);
int backat(void);
int newtext(void);
int printc(BYTE car);
int ink(BYTE color);
int paper(BYTE color);
int bright(BYTE b);
int blink(BYTE b);
int color(BYTE col);

void far int24_hnd(unsigned deverror,unsigned errcode,unsigned far *devhdr);

/****************************************/
/********** Programa principal **********/
/****************************************/

void main(int argc,char *argv[])
{

#if DEBUGGER==ON
BYTE lin_deb;
#endif

BYTE par1, par2;        /* 2 primeros par†metros en llamada a condacto */
int res_pro;            /* resultado (TRUE, FALSE) de llamada a proc. */
union REGS dreg;
struct SREGS dsreg;
static struct condacto cd[]={   /* tabla funci¢n-tipo de condacto */
	0       , 0,        /* condacto 0, reservado para fin entrada */
	process , 1,        /* tipo 1 = par†metro de 1 byte */
	done    , 0,        /* tipo 0 = sin par†metros */
	notdone , 0,
	resp    , 0,
	noresp  , 0,
	defwin  , 6,        /* tipo 6 = 6 par†metros de 1 byte */
	window  , 1,
	clw     , 1,
	let     , 2,        /* tipo 2 = 2 par†metros de 1 byte */
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
	ramload , 3,        /* tipo 3 = 3 par†metros de 1 byte */
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
};

if(argc<2) m_err(7,1,"Falta nombre de fichero.",1);

#if DEBUGGER==ON
if(argc==3) {       /* si tecle¢ algo detr†s del nombre del fichero */
	if((argv[2][0]=='/') && ((argv[2][1]=='l') ||
	  (argv[2][1]=='L'))) {
	/* si introdujo /l o /L recoge los dos siguiente d°gitos y */
	/* calcula l°nea de la ventana del debugger */
		lin_deb=((argv[2][2]-'0')*10)+(argv[2][3]-'0');
		if(lin_deb>23) lin_deb=23;
		w_deb.wy=lin_deb;
	}
}
#endif

_harderr(int24_hnd);    /* instala handler de errores cr°ticos */

cls();
pon_cur(0,0);
load_db(argv[1]);           /* carga base de datos */
inic();                     /* inicializa variables */
ram0=0;                 /* indica que los bancos de memoria de RAMSAVE y */
ram1=0;                 /* RAMLOAD no han sido utilizados */
modo_pant();
dreg.h.ah=0x33;             /* obtener/establecer indicador de ruptura */
dreg.h.al=0;                /* obtenerlo */
intdosx(&dreg,&dreg,&dsreg);
ruptura=dreg.h.dl;          /* guarda indicador de ruptura */
dreg.h.ah=0x33;
dreg.h.al=1;                /* establecerlo */
dreg.h.dl=0;                /* lo desactiva */
intdosx(&dreg,&dreg,&dsreg);

#if DEBUGGER==ON
/* presentaci¢n */
cls();
w_cls(&w_jsj);
w_imps("\n   "COPYRIGHT,&w_jsj);
w_imps("   IntÇrprete-Debugger Versi¢n "VERSION_IS"\n\n"
       "      ATENCION: Este programa es de\n"
       "            DOMINIO PUBLICO.\n\n"
       "            Pulsa una tecla.",&w_jsj);
lee_tecla();
cls();
pon_cur(0,0);
#endif

ptr_proc=proc+pro[pro_act]; /* ptro. a proceso actual */
do {
	if(*ptr_proc) {     /* si no es fin de proceso */
		/* res_pro=FALSE si no debe ejec. entrada */
		res_pro=!resp_act || (resp_act && ((*ptr_proc==NO_PAL) ||
		  (*ptr_proc==var[2])) && ((*(ptr_proc+1)==NO_PAL) ||
		  (*(ptr_proc+1)==var[3])));
		ptr_proc+=2;    /* salta verbo-nombre */
		/* calcula el desplazamiento de la sgte. entrada */
		sgte_ent=(ptrdiff_t)((*(ptr_proc+1) << 8) | *ptr_proc);
		ptr_proc+=2;

#if DEBUGGER==ON
		pra_lin=1;  /* indica primera l°nea de entrada */
#endif

	}
	else {      /* fin de proceso */
		res_pro=done();
		ptr_proc++;     /* ajustamos ptr_proc */

#if DEBUGGER==ON
		pra_lin=0;      /* no es primera l°nea de entrada */
#endif

	}
	if(nueva_ent) nueva_ent=0;
	/* si res_pro es TRUE y no fin de entrada ejecuta esta entrada */
	/* si no, salta a la siguiente */
	while(res_pro && *ptr_proc) {
		par1=*(ptr_proc+1); /* supone no hay indirecci¢n */
		par2=*(ptr_proc+2);

#if DEBUGGER==ON
		indir1=0;
		indir2=0;
		pro_d=ptr_proc; /* guarda dir. condacto en curso */
#endif

		/* indirecci¢n en 1er par†metro */
		if(*ptr_proc==INDIR1) {
			ptr_proc++; /* salta prefijo */
			par1=var[*(ptr_proc+1)];
			par2=*(ptr_proc+2);

#if DEBUGGER==ON
			indir1=1;
#endif

		}
		/* indirecci¢n en 2ß par†metro */
		else if(*ptr_proc==INDIR2) {
			ptr_proc++; /* salta prefijo */
			par1=*(ptr_proc+1);
			par2=var[*(ptr_proc+2)];

#if DEBUGGER==ON
			indir2=1;
#endif

		}
		/* indirecci¢n en ambos */
		else if(*ptr_proc==INDIR12) {
			ptr_proc++; /* salta prefijo */
			par1=var[*(ptr_proc+1)];
			par2=var[*(ptr_proc+2)];

#if DEBUGGER==ON
			indir1=1;
			indir2=1;
#endif

		}

#if DEBUGGER==ON
	if(debugg) {    /* si est† activado paso a paso */
		guarda_screen();
		w_cls(&w_deb);
		sprintf(lin_cond,txt_deb1,(unsigned)pro_act);
		w_imps(lin_cond,&w_deb);
		w_imps(txt_deb2,&w_deb);
		imp_condacto();         /* imprime condacto en curso */
		key=lee_tecla();        /* espera que se pulse tecla */
		if(key==COD_SHIFT_F1) debugg=0; /* desactiva paso a paso */
		while(esta_en("vVbBsS",key)) {
			switch(key) {
			case 's' :      /* salir */
			case 'S' :
				m_err(11,1,"Pulsa una tecla.",2);
				break;
			case 'v' :      /* variables */
			case 'V' :
			  do {
				sprintf(lin_cond, "Variable: %3u=%3u"
				  "  ..... ## O=otra  M=modificar F=fin ##",
				  variable,var[variable]);
				/* posiciona cursor */
				w_deb.cwy=0;
				w_deb.cwx=17;
				w_imps(lin_cond,&w_deb);
				if(variable==2) {
				  palabra=saca_pal(var[2],_VERB);
				  /* puede ser nombre convertible */
				  if(palabra==(NUM_PAL+1))
				    palabra=saca_pal(var[2],_NOMB);
				}
				else if(variable==3)
				  palabra=saca_pal(var[3],_NOMB);
				else if(variable==4)
				  palabra=saca_pal(var[4],_ADJT);
				else if(variable==5)
				  palabra=saca_pal(var[5],_NOMB);
				else if(variable==6)
				  palabra=saca_pal(var[6],_ADJT);
				else palabra=NUM_PAL+1;
				if(palabra==(NUM_PAL+1))
				  sprintf(lin_cond,"-----");
				else sprintf(lin_cond,"%s",
				  vocabulario[palabra].p);
				w_deb.cwx=36;           /* posiciona cursor */
				w_imps(lin_cond,&w_deb);
				key=lee_tecla();        /* espera tecla */
				switch(key) {
				case 'm' :              /* modificar */
				case 'M' :
					w_deb.cwx=31;
					valor=inp_band();
					var[variable]=valor;
					break;
				case 'o' :      /* otra variable */
				case 'O' :
					w_deb.cwx=27;
					variable=inp_band();
					break;
				case COD_ARR :
					variable--;
					break;
				case COD_ABJ :
					variable++;
					break;
				}
			  } while((key!='f') && (key!='F'));
			  break;
			case 'b' :      /* banderas */
			case 'B' :
			  do {
				sprintf(lin_cond,"Bandera: %3u=%1u  "
				  "##  O=otra  M=modificar  F=fin  ##   ",
				  bandera,notzero(bandera));
				w_deb.cwy=0;    /* posiciona cursor */
				w_deb.cwx=17;
				w_imps(lin_cond,&w_deb);
				key=lee_tecla();        /* espera tecla */
				switch(key) {
				case 'm' :      /* modificar */
				case 'M' :
					w_deb.cwx=30;
					w_impc((char)'€',&w_deb);
					do {
						key=lee_tecla();
					} while((key!='0') && (key!='1'));
					if(key=='0') clear(bandera);
					else set(bandera);
					break;
				case 'o' :      /* otra bandera */
				case 'O' :
					w_deb.cwx=26;
					bandera=inp_band();
					break;
				case COD_ARR :
					bandera--;
					break;
				case COD_ABJ :
					bandera++;
					break;

				}
			  } while((key!='f') && (key!='F'));
			  break;
			}
		w_deb.cwy=0;    /* posiciona cursor */
		w_deb.cwx=17;
		w_imps(txt_deb2,&w_deb);
		key=lee_tecla();
		}
	recupera_screen();
	}
	/* activa-desactiva paso a paso */
	else if(_bios_keybrd(_KEYBRD_READY)==SHIFT_F1) {
		_bios_keybrd(_KEYBRD_READ);
		debugg=1;
	}
#endif

	/* ejecuta condacto seg£n tipo */
	switch(cd[*ptr_proc].tipo) {
	case 0 :    /* sin par†metros */
		res_pro=cd[*ptr_proc].cond();
		ptr_proc++;     /* sgte. condacto */
		break;
	case 1 :    /* BYTE */
		res_pro=cd[*ptr_proc].cond(par1);
		ptr_proc+=2;    /* sgte. condacto */
		break;
	case 2 :    /* BYTE,BYTE */
		res_pro=cd[*ptr_proc].cond(par1,par2);
		ptr_proc+=3;    /* sgte. condacto */
		break;
	case 3 :    /* BYTE,BYTE,BYTE */
		res_pro=cd[*ptr_proc].cond(par1,par2,
		  *(ptr_proc+3));
		ptr_proc+=4;    /* sgte. condacto */
		break;
	case 6 :    /* BYTE,BYTE,BYTE,BYTE,BYTE,BYTE */
		res_pro=cd[*ptr_proc].cond(par1,par2,*(ptr_proc+3),
		  *(ptr_proc+4),*(ptr_proc+5),*(ptr_proc+6));
		ptr_proc+=7;    /* sgte. condacto */
		break;
	}
	}
	if(!*ptr_proc) ptr_proc++;  /* si fin entrada, pasa a la siguiente */
	else if(!nueva_ent) ptr_proc=proc+pro[pro_act]+sgte_ent;
} while(1);
}

#if DEBUGGER==ON
/*****************************************************************************
	SACA_PAL: devuelve el n£mero de la primera entrada en el vocabulario
	  que se corresponda con el n£mero y tipo de palabra dado.
	  Si no se encontr¢ ninguna entrada se devuelve (NUM_PAL+1).
*****************************************************************************/
int saca_pal(BYTE num_pal,BYTE tipo_pal)
{
int i;
for(i=0;i<pvoc;i++)
  if((vocabulario[i].num==num_pal) && (vocabulario[i].tipo==tipo_pal))
  return(i);
return(NUM_PAL+1);  /* n£mero de palabra no existe */
}

/*****************************************************************************
	IMP_CONDACTO: imprime condacto en curso en la ventana de debug.
	  Entrada: 'indir1' e 'indir2' indicadores de indirecci¢n
		   'pro_d' direcci¢n del condacto en curso
		   'ptr_proc' puntero a byte de condacto + par†metros
		   'pra_lin' =1 si es 1era lin. de entrada
*****************************************************************************/
void imp_condacto(void)
{
BYTE far *pcp;
int j, dirrel;
unsigned dir;

if(pra_lin) dir=(unsigned)(pro_d-proc-4);
else dir=(unsigned)(pro_d-proc);
sprintf(lin_cond,"  %5u: ",dir);  /* calcula dir. cond. */
w_imps(lin_cond,&w_deb);
if(pra_lin) {
	pcp=ptr_proc-4;     /* apunta a verbo-nombre */
	if(indir1 || indir2) pcp--;     /* ajuste por indirecci¢n */
	if(*pcp==NO_PAL) sprintf(lin_cond,"-      ");
	else {
		j=saca_pal(*pcp,_VERB); /* si no es verbo, quiz† sea nombre */
		if(j==(NUM_PAL+1)) j=saca_pal(*pcp,_NOMB);
		sprintf(lin_cond,"%s  ",vocabulario[j].p);
	}
	w_imps(lin_cond,&w_deb);
	pcp++;
	if(*pcp==NO_PAL) sprintf(lin_cond,"-      ");
	else sprintf(lin_cond,"%s  ",vocabulario[saca_pal(*pcp,_NOMB)].p);
	w_imps(lin_cond,&w_deb);
}
else w_imps("              ",&w_deb);
sprintf(lin_cond,"%s  ",c[*ptr_proc].cnd);  /* imprime condacto */
w_imps(lin_cond,&w_deb);
switch(c[*ptr_proc].tipo) {
case 0 :
	sprintf(lin_cond," ");
	break;
case 1 :
	if(indir1) sprintf(lin_cond,"[%3u]",*(ptr_proc+1));
	else sprintf(lin_cond,"%5u",*(ptr_proc+1));
	break;
case 2 :
	if(indir1 && indir2)
	  sprintf(lin_cond,"[%3u] [%3u]",*(ptr_proc+1),*(ptr_proc+2));
	else if(indir1) sprintf(lin_cond,"[%3u] %5u",*(ptr_proc+1),
	  *(ptr_proc+2));
	else if(indir2) sprintf(lin_cond,"%5u [%3u]",*(ptr_proc+1),
	  *(ptr_proc+2));
	else sprintf(lin_cond,"%5u %5u",*(ptr_proc+1),*(ptr_proc+2));
	break;
case 3 :
	dirrel=(*(ptr_proc+2) << 8) | *(ptr_proc+1);
	if(pra_lin) dirrel+=4;  /* ajusta si 1era l°nea */
	sprintf(lin_cond,"%5i",dirrel);
	break;
case 4 :
	sprintf(lin_cond,"%s",vocabulario[saca_pal(*(ptr_proc+1),_ADJT)].p);
	break;
case 5 :
	sprintf(lin_cond,"%s",vocabulario[saca_pal(*(ptr_proc+1),_NOMB)].p);
	break;
case 6 :
	if(indir1 && indir2) sprintf(lin_cond,"[%3u] [%3u] ",*(ptr_proc+1),
	  *(ptr_proc+2));
	else if(indir1) sprintf(lin_cond,"[%3u] %5u ",*(ptr_proc+1),
	  *(ptr_proc+2));
	else if(indir2) sprintf(lin_cond,"%5u [%3u] ",*(ptr_proc+1),
	  *(ptr_proc+2));
	else sprintf(lin_cond,"%5u %5u ",*(ptr_proc+1),*(ptr_proc+2));
	w_imps(lin_cond,&w_deb);
	sprintf(lin_cond,"%5u %5u %5u %5u",*(ptr_proc+3),*(ptr_proc+4),
	  *(ptr_proc+5),*(ptr_proc+6));
	break;
case 7 :
	if(*(ptr_proc+1)==NO_PAL) sprintf(lin_cond,"-      ");
	else {
		/* quiz† sea nomb conv */
		j=saca_pal(*(ptr_proc+1),_VERB);
		if(j==(NUM_PAL+1)) j=saca_pal(*(ptr_proc+1),_NOMB);
		sprintf(lin_cond,"%s  ",vocabulario[j].p);
	}
	w_imps(lin_cond,&w_deb);
	if(*(ptr_proc+2)==NO_PAL) sprintf(lin_cond,"-");
	else sprintf(lin_cond,"%s",vocabulario[saca_pal(*(ptr_proc+2),
	  _NOMB)].p);
	break;
case 8 :
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
w_imps(lin_cond,&w_deb);    /* imprime par†metros */
pra_lin=0;
}

/*****************************************************************************
	INP_BAND: rutina de introducci¢n por teclado de n£meros para debugger
	  Salida: n£mero introducido en el rango 0-255 (BYTE).
*****************************************************************************/
BYTE inp_band(void)
{
unsigned k;
char numero[4];
int i;
BYTE oldcwx;

oldcwx=w_deb.cwx;   /* guarda antigua posici¢n cursor */
do {
	i=0;
	w_deb.cwx=oldcwx;   /* restaura posici¢n cursor */
	do {
		w_impc((char)'€',&w_deb);
		w_deb.cwx--;
		k=lee_tecla();
		if((k>='0') && (k<='9')) {
			numero[i]=(char)k;
			i++;
			w_impc((char)k,&w_deb);
		}
	} while((i<3) && (k!=RETURN));
	numero[i]='\0';
	i=atoi(numero); /* pasa cadena ASCII a n£mero */
} while((i<0) || (i>255));
return((BYTE)i);
}

/*****************************************************************************
	GUARDA_SCREEN: guarda en un buffer la zona de pantalla que
	  ser† sobreescrita por la ventana del debugger.
*****************************************************************************/
void guarda_screen(void)
{
int i, j, k=0;
BYTE fil, col;
union REGS reg;

fil=w_deb.wy;   /* coge la fila de inicio de  la ventana del debugger */
for(j=0;j<2;j++) {              /* recoge 2 filas */
	col=w_deb.wx;   /* columna de inicio de la ventana del debugger */
	for(i=0;i<80;i++) {     /* 80 columnas cada fila */
		pon_cur(fil,col);
		reg.h.ah=0x08;  /* leer car†cter y attr. de pos. cursor */
		reg.h.bh=0;     /* p†gina 0 */
		int86(0x10,&reg,&reg);
		buf_scr[k]=reg.h.ah;         /* guarda atributo */
		buf_scr[k+1]=reg.h.al;       /* guarda car†cter */
		k+=2;
		col++;
	}
	fil++;          /* siguiente fila */
}
}

/*****************************************************************************
	RECUPERA_SCREEN: recupera del buffer la zona de pantalla.
*****************************************************************************/
void recupera_screen(void)
{
int i, j, k=0;
BYTE fil, col;

fil=w_deb.wy;   /* coge la fila de inicio de  la ventana del debugger */
for(j=0;j<2;j++) {              /* recupera 2 filas */
	col=w_deb.wx;   /* columna de inicio de la ventana del debugger */
	for(i=0;i<80;i++) {     /* 80 columnas cada fila */
		pon_cur(fil,col);
		impc(buf_scr[k+1],buf_scr[k]);
		k+=2;
		col++;
	}
	fil++;          /* siguiente fila */
}
}
#endif

/****************************************/
/*************** Funciones **************/
/****************************************/

/*****************************************************************************
	MODO_PANT: inicializa el modo de pantalla.
*****************************************************************************/
void modo_pant(void)
{
union REGS reg;

reg.h.ah=0x01;      /* funci¢n definir cursor */
reg.x.cx=0x2000;    /* esconde cursor */
int86(0x10,&reg,&reg);
}

/*****************************************************************************
	W_CLS: borra una ventana.
*****************************************************************************/
void w_cls(struct window *w)
{
scrollw(w->wy,w->wx,w->lx,w->ly,w->color,0);    /* borra ventana */
w->cwx=0;       /* posiciona el cursor al inicio de la ventana */
w->cwy=0;
w->colort=w->color;     /* resetea el color temporal */
w->scroll=0;    /* borra indicador de scroll */
}

/*****************************************************************************
	W_IMPC: imprime un car†cter en una ventana. S¢lo tiene en cuenta como
	  car†cter especial el c¢digo 13 (avance de l°nea).
*****************************************************************************/
void w_impc(char c,struct window *w)
{
union REGS reg;

if(c!='\n') {   /* si no es avance de linea */
	pon_cur(w->wy+w->cwy,w->wx+w->cwx);
	impc((BYTE)c,w->colort);
	w->cwx++;    /* incrementa columna */
}
else w->cwx=w->lx;   /* si es avance de linea, fuerza paso a sgte. lin. */

if(w->cwx>(w->lx-1)) {  /* si llega a la £ltima columna */
	w->cwy++;    /* pasa a siguiente linea */
	w->cwx=0;
	if(w->cwy>(w->ly-1)) {
		w->cwy=w->ly-1; /* coloca en £ltima linea */
		w->scroll++;    /* incrementa indicador de scroll */
		if(w->scroll>w->ly) w->scroll=1;
		if(w->scroll==1) {  /* espera tecla antes de scroll */
			 /* esq. inf. der. */
			pon_cur(w->wy+w->ly-1,w->wx+w->lx-1);
			reg.h.ah=0x08;  /* coger car y attr de pos. cursor */
			reg.h.bh=0;     /* p†gina 0 */
			int86(0x10,&reg,&reg);  /* AH=atributo, AL=car†cter */
			do {
				/* imp. car. de espera scroll */
				impc(CAR_SCROLL,w->colort);
				pausa(SCROLL_PAU);
				/* borra car. de espera */
				impc(reg.h.al,w->colort);
				pausa(SCROLL_PAU);
			} while(!_bios_keybrd(_KEYBRD_READY));
#if DEBUGGER==ON
			/* activa-desactiva paso a paso */
			if(_bios_keybrd(_KEYBRD_READY)==SHIFT_F1) debugg=1;
#endif
			_bios_keybrd(_KEYBRD_READ); /* saca tecla de buffer */
		}
		if(w->scroll) scrollw(w->wy,w->wx,w->lx,w->ly,w->color,1);
	}
}
}

/*****************************************************************************
	W_IMPS: imprime una cadena en una ventana. Las palabras de final de
	  linea que no quepan dentro de la ventana se pasan a la linea sgte.
*****************************************************************************/
void w_imps(char far *s,struct window *w)
{
#define MAX_PAL 80
static char b[MAX_PAL]; /* buffer para guardar palabras */
BYTE cuenta=0, i;
int fin=0;
char far *pto;

do {
	if((*s=='\0') || (*s==' ') || (*s=='\n')) {
	/* si se encontr¢ fin de frase, espacio o av. linea */
		if(!cuenta && (*s!='\0')) {
		/* si cuenta==0, no hay palabra almacenada */
			w_impc(*s,w);
			s++;
		}
		else {
			/* si la palabra almacenada no cabe en lo que */
			/* queda de linea pero cabe en la siguiente, */
			/* la imprime en la sgte linea si no, la */
			/* imprime sin m†s */
			if((cuenta>w->lx-w->cwx-1) && (cuenta<=w->lx))
			  w_impc('\n',w);
			for(i=0;i<cuenta;i++) w_impc(b[i],w);
			cuenta=0;
			if(*s=='\0') fin=1; /* indica fin de frase */
		}
	}
	else {
		if(*s!='_') {
			/* si letra no es espacio ni av. lin. la guarda */
			b[cuenta]=*s++;
			cuenta++;
			if(cuenta==MAX_PAL) { /* si se llena buffer */
				/* imprime lo que hay almacenado */
				for(i=0;i<MAX_PAL;i++) w_impc(b[i],w);
				cuenta=0;
				/* imprime resto de palabra */
				while((*s!='\0') && (*s!=' ') && (*s!='\n')) {
					w_impc(*s,w);
					s++;
				}
			}
		}
		else {  /* si encontr¢ s°mbolo de subrayado */
			s++;    /* se lo salta */
			pto=obj+objs[var[8]]+6; /* ptro a texto objeto */
			while(*pto) {
				b[cuenta]=*pto++; /* almacena texto objeto */
				cuenta++;
				if(cuenta==MAX_PAL) { /* si se llena buffer */
					/* imprime lo que hay almacenado */
					for(i=0;i<MAX_PAL;i++) w_impc(b[i],w);
					cuenta=0;
					/* imprime resto de pal. de objeto*/
					while((*pto!='\0') && (*pto!=' ') &&
					  (*pto!='\n')) {
						w_impc(*pto,w);
						pto++;
					}
				}
			}
		}
	}
} while(!fin);
}

/*****************************************************************************
	W_IMPN: imprime un n£mero en una ventana.
*****************************************************************************/
void w_impn(unsigned n,struct window *w)
{
int i=0;
char d[5];

do {
	d[i]=(char)(n%10)+'0';  /* coge el resto de n/10 y lo pasa a ASCII */
	n/=10;
	i++;
} while(n);
while(i--) w_impc(d[i],w);
}

/*****************************************************************************
	PON_CUR: coloca el cursor en una posici¢n de pantalla.
*****************************************************************************/
void pon_cur(BYTE fila, BYTE columna)
{
union REGS reg;

reg.h.ah=0x02;  /* funci¢n posicionar cursor */
reg.h.bh=0;     /* p†gina 0 */
reg.h.dh=fila;
reg.h.dl=columna;
int86(0x10,&reg,&reg);
}

/*****************************************************************************
	SCROLLW: realiza el scroll de una ventana dada.
*****************************************************************************/
void scrollw(BYTE f,BYTE c,BYTE ancho,BYTE alto,BYTE color,BYTE lineas)
{
union REGS reg;

reg.h.ah=0x06;      /* funci¢n scroll de ventana */
reg.h.al=lineas;    /* si 0, borra la ventana */
reg.h.bh=color;
reg.h.ch=f;
reg.h.cl=c;
reg.h.dh=f+alto-1;
reg.h.dl=c+ancho-1;
int86(0x10,&reg,&reg);
}

/*****************************************************************************
	IMPC: imprime un caracter en pantalla, en la posici¢n del cursor.
*****************************************************************************/
void impc(BYTE c,BYTE color)
{
union REGS reg;

reg.h.ah=0x09;  /* funci¢n escribir caracter */
reg.h.al=c;
reg.h.bh=0;
reg.h.bl=color;
reg.x.cx=1;
int86(0x10,&reg,&reg);
}

/*****************************************************************************
  W_INPUT: permite introducir por teclado una linea de longitud m†xima
	'maxlong', que se almacenar† en 'plin'.
	S¢lo admite los caracteres que contenga la cadena TECLAS (global)
*****************************************************************************/
void w_input(char *plin,unsigned maxlong,char cursor,struct window *w)
{
unsigned c, long_lin;
char *pcur, *pfinl, *i;
BYTE oldx, oldy;

oldx=w->cwx;
oldy=w->cwy;
w->scroll=1;
pcur=plin;
pfinl=plin;
long_lin=1;
*pfinl='\0';

w_impc(cursor,w);
while((c=lee_tecla())!=RETURN) {
	switch(c) {
	case BACKSPACE : /* borrado */
		/* comprueba que no est† a la izquierda */
		if(pcur!=plin) {
			pcur--;
			for(i=pcur;i<pfinl;i++) *i=*(i+1);
			pfinl--;
			long_lin--;
		}
		break;
	case COD_IZQ :  /* mover cursor izquierda */
		/* comprueba que no estÇ a la izquierda */
		if(pcur!=plin) pcur--;
		break;
	case COD_DER :  /* mover cursor derecha */
		/* comprueba que no estÇ a la derecha */
		if(pcur!=pfinl) pcur++;
		break;
	case COD_ARR :  /* ignora cursor arriba-abajo */
	case COD_ABJ :
		break;
#if DEBUGGER==ON
	case COD_SHIFT_F1 :   /* si F1, activa-desactiva paso a paso */
		if(debugg) debugg=0;
		else debugg=1;
		break;
#endif
	case COD_F1 :
	case COD_F2 :
	case COD_F3 :
	case COD_F4 :
	case COD_F5 :
	case COD_F6 :
	case COD_F7 :
	case COD_F8 :
	case COD_F9 :
	case COD_F10 :
		break;
	default :
		if(long_lin==maxlong) break;    /* inserta caracter */
		if(pcur!=pfinl) for(i=pfinl;i>pcur;i--) *i=*(i-1);
		*pcur++=(char)c;
		*(++pfinl)='\0';
		long_lin++;
		break;
	}
	if(w->scroll!=1) w->cwy=oldy-(w->scroll-1);
	else w->cwy=oldy;
	w->cwx=oldx;    /* posiciona cursor para imprimir */
	i=plin;
	do {
		 /* si es posici¢n cursor, lo imprime */
		if(i==pcur) w_impc(cursor,w);
		if(*i) w_impc(*i,w);
		i++;
	} while(i<=pfinl);
	w_impc(' ',w);
}
/* reimprime la linea de input, para borrar el cursor */
if(w->scroll!=1) w->cwy=oldy-(w->scroll-1);
else w->cwy=oldy;
w->cwx=oldx;    /* posiciona cursor para imprimir */
i=plin;
do {
	if(*i) w_impc(*i,w);
	i++;
} while(i<=pfinl);
w_impc(' ',w);

while(*plin) {
	*plin=mayuscula(*plin); /* convierte linea a mayusculas */
	plin++;
}
if((w->scroll==1) && (w->cwy<(w->ly-1))) w->scroll=0;
}

/*****************************************************************************
  LEE_TECLA: devuelve el c¢digo ASCII de la tecla pulsada. Si se puls¢
	una tecla de cursor devuelve COD_IZQ, COD_DER, COD_ARR o COD_ABJ
	Espera hasta que se pulse una tecla, o hasta que la tecla pulsada
	sea una de las contenidas en la cadena TECLAS (global).
*****************************************************************************/
unsigned lee_tecla(void)
{
unsigned tecla;
do {
	/* byte bajo=ASCII, byte alto=CODIGO*/
	tecla=_bios_keybrd(_KEYBRD_READ);
	if(tecla==IZQ) tecla=COD_IZQ;       /* comprueba teclas especiales */
	else if(tecla==DER) tecla=COD_DER;  /* y pone ASCII=c¢digo */
	else if(tecla==ARR) tecla=COD_ARR;
	else if(tecla==ABJ) tecla=COD_ABJ;
	else if(tecla==F1) tecla=COD_F1;
	else if(tecla==F2) tecla=COD_F2;
	else if(tecla==F3) tecla=COD_F3;
	else if(tecla==F4) tecla=COD_F4;
	else if(tecla==F5) tecla=COD_F5;
	else if(tecla==F6) tecla=COD_F6;
	else if(tecla==F7) tecla=COD_F7;
	else if(tecla==F8) tecla=COD_F8;
	else if(tecla==F9) tecla=COD_F9;
	else if(tecla==F10) tecla=COD_F10;

#if DEBUGGER==ON
	else if(tecla==SHIFT_F1) tecla=COD_SHIFT_F1;
#endif

	tecla &= 0x00ff;
} while(!esta_en(TECLAS,tecla));
return(tecla);
}

/*****************************************************************************
  ESTA_EN: comprueba si el caracter 'c' est† contenido en la cadena
	'cadena'. Si est† devuelve 1, y si no 0.
*****************************************************************************/
int esta_en(const char *cadena,unsigned c)
{
while(*cadena) if((char)c==*cadena++) return(1);
return(0);
}

/*****************************************************************************
	PAUSA: realiza una pausa de milisegundos.
	  Si se pulsa una tecla, sale de la pausa, pero no saca la tecla
	  del buffer de teclado.
	  Si el valor de pausa es 0, se espera a que pulse una tecla.
*****************************************************************************/
void pausa(clock_t wait)
{
clock_t t1, t2;

if(!wait) {     /* si pausa es 0, espera que se pulse una tecla */
	lee_tecla();
}
t1=wait+clock();
do {
	t2=clock();
} while((t2<t1) && !_bios_keybrd(_KEYBRD_READY));
}

/*****************************************************************************
	LOAD_DB: carga la base de datos.
*****************************************************************************/
void load_db(char *nombre)
{
static char *errmem="No hay suficiente memoria.";
static char recon[L_RECON];
unsigned t_msg, t_msy, t_loc, t_con, t_obj, t_pro;

if((f_in=fopen(nombre,"rb"))==NULL)
  m_err(0,1,"Error de apertura de fichero de entrada",1);

frd2((char *)recon,sizeof(char),L_RECON-1);
recon[L_RECON-1]='\0';  /* a§ade el fin de cadena */
if(strcmp(SRECON,recon))
  m_err(4,1,"Fichero de entrada no v†lido.",1);

frd2((BYTE *)&v_mov,sizeof(BYTE),1);
frd2((BYTE *)&n_conv,sizeof(BYTE),1);
frd2((BYTE *)&n_prop,sizeof(BYTE),1);
frd2((int *)&pvoc,sizeof(int),1);
frd2((BYTE *)&m_act,sizeof(BYTE),1);
frd2((unsigned *)&t_msg,sizeof(unsigned),1);
frd2((BYTE *)&m_actsys,sizeof(BYTE),1);
frd2((unsigned *)&t_msy,sizeof(unsigned),1);
frd2((BYTE *)&n_loc,sizeof(BYTE),1);
frd2((unsigned *)&t_loc,sizeof(unsigned),1);
frd2((unsigned *)&t_con,sizeof(unsigned),1);
frd2((BYTE *)&objact,sizeof(BYTE),1);
frd2((unsigned *)&t_obj,sizeof(unsigned),1);
frd2((BYTE *)&proact,sizeof(BYTE),1);
frd2((unsigned *)&t_pro,sizeof(unsigned),1);

/* Reserva de memoria para las distintas secciones */
/* -- Mensajes -- */
if((msgs=(char far *)_fmalloc((size_t)t_msg))==NULLF)
  m_err(6,1,errmem,1);

/* -- Mensajes del Sistema -- */
if((msgsys=(char far *)_fmalloc((size_t)t_msy))==NULLF)
  m_err(6,1,errmem,1);

/* -- Localidades -- */
if((localidades=(char far *)_fmalloc((size_t)t_loc))==NULLF)
  m_err(6,1,errmem,1);
/* -- Conexiones -- */
if((conx=(char far *)_fmalloc((size_t)t_con))==NULLF)
  m_err(6,1,errmem,1);

/* -- Objetos -- */
if((obj=(char far *)_fmalloc((size_t)t_obj))==NULLF)
  m_err(6,1,errmem,1);

/* -- Procesos -- */
if((proc=(BYTE far *)_fmalloc((size_t)t_pro))==NULLF)
  m_err(6,1,errmem,1);

frd2((struct palabra *)vocabulario,sizeof(struct palabra),pvoc);

frd2((ptrdiff_t *)mens,sizeof(ptrdiff_t),(size_t)MAX_MSG);
rd_fich(msgs,t_msg);

frd2((ptrdiff_t *)mensys,sizeof(ptrdiff_t),(size_t)MAX_MSY);
rd_fich(msgsys,t_msy);

frd2((ptrdiff_t *)locs,sizeof(ptrdiff_t),(size_t)MAX_LOC);
rd_fich(localidades,t_loc);

frd2((ptrdiff_t *)conxs,sizeof(ptrdiff_t),(size_t)MAX_LOC);
rd_fich(conx,t_con);

frd2((ptrdiff_t *)objs,sizeof(ptrdiff_t),(size_t)MAX_OBJ);
rd_fich(obj,t_obj);

frd2((ptrdiff_t *)pro,sizeof(ptrdiff_t),(size_t)MAX_PRO);
rd_fich(proc,t_pro);

}

/*****************************************************************************
	INIC: inicializa diversas tablas y variables.
*****************************************************************************/
void inic(void)
{
int i;
char far *po;

objs_cogidos=0;                 /* num. de objetos cogidos */
for(i=0;i<(int)objact;i++) {    /* inicializa tabla de localidades actuales */
	po=obj+objs[i];             /* de los objetos */
	loc_obj[i]=(BYTE)*(po+2);   /* coge loc. inicial y guarda en tabla */
	if((loc_obj[i]==PUESTO) || (loc_obj[i]==COGIDO))
	  objs_cogidos++;       /* si lleva objeto de inicio, inc. contador */
}
for(i=0;i<256;i++) {            /* inicializa variables */
	if((i>1) && (i<7)) var[i]=NO_PAL;   /* vars. de sentencia l¢gica */
	else var[i]=0;
}
for(i=0;i<32;i++) flag[i]=0;    /* inicializa flags */
for(i=0;i<N_VENT;i++) {         /* inicializa ventanas */
	w[i].wx=0;      /* origen ventana en (0,0) */
	w[i].wy=0;
	w[i].lx=80;     /* tama§o 80x25 (toda la pantalla) */
	w[i].ly=25;
	w[i].cwx=0;
	w[i].cwy=0;
	w[i].color=7;   /* color blanco en modo texto */
	w[i].colort=7;
	w[i].scroll=0;
	w[i].cwxs=0;
	w[i].cwys=0;
}
pro_act=0;          /* num. de proceso actual */
ptrp=0;             /* puntero de pila */
resp_act=0;         /* NORESP */
nueva_ent=0;
}

/*****************************************************************************
	FRD2: controla la entrada de datos desde el fichero de entrada
	  mediante la funci¢n fread.
*****************************************************************************/
void frd2(void *buff,size_t tam,size_t cant)
{
if(fread(buff,tam,cant,f_in)!=cant) {
	if(feof(f_in)) return;
	if(ferror(f_in))
	  m_err(5,1,"Error en fichero de entrada.",1);
}
}

/*****************************************************************************
	RD_FICH: lee del fichero de entrada f_in, un bloque de BYTES y los
	  almacena en un buffer de memoria far.
*****************************************************************************/
int rd_fich(char far *buff,unsigned tam)
{
BYTE cod;
int c;

while(tam) {
	if((c=fgetc(f_in))==EOF) {    /* lee dato */
		if(feof(f_in))
		  m_err(8,1,"--- Fin de fichero ---",1);
		if(ferror(f_in))
		  m_err(5,1,"Error en fichero de entrada.",1);
	}
	cod=(BYTE)0xff-(BYTE)c;     /* decodifica BYTE */
	*buff=cod;                  /* almacena BYTE */
	buff++;                     /* siguiente BYTE */
	tam--;
}
return(0);  /* indica que no hubo error */
}

/*****************************************************************************
	ANALIZA: analiza una palabra.
	  < !≠?®>   caracteres no significativos
	  <.,;:"'>  separadores
	  Entrada: char *(*pfrase), ptro. a frase a analizar.
	  Salida:  actualiza tipo y num de acuerdo al vocabulario.
		Devuelve SEPARADOR si encontr¢ un separador
			 FIN_FRASE si encontr¢ final de la frase
			 PALABRA si encontr¢ palabra v†lida de vocabulario
			 TERMINACION como PALABRA pero si adem†s encontr¢ una
			   terminaci¢n en LA, LE o LO
			 NO_PAL si encontr¢ palabra pero no est† en
			   vocabulario
*****************************************************************************/
int analiza(char *(*pfrase),BYTE *tipo,BYTE *num)
{
int i=0;
static char palabra[6];

for(;esta_en(C_NO_SIG,*(*pfrase));(*pfrase)++); /* salta cars. no signif. */
if(esta_en(C_SEPAR,*(*pfrase))) {       /* si es un separador */
	(*pfrase)++;                        /* y lo salta */
	for(;esta_en(C_NO_SIG,*(*pfrase));(*pfrase)++); /* salta cars. */
	while(esta_en(C_SEPAR,*(*pfrase))) { /* por si hay otro sep. detr†s */
		(*pfrase)++;    /* salta si es otro separador */
		/* salta cars. */
		for(;esta_en(C_NO_SIG,*(*pfrase));(*pfrase)++);
	}
	if(!*(*pfrase)) return(FIN_FRASE);  /* si detr†s hay un \0 */
	else return(SEPARADOR);             /* si no, habr† m†s frase */
}

if(!*(*pfrase)) return(FIN_FRASE);  /* fin de frase */
for(;*(*pfrase)==' ';(*pfrase)++);  /* salta espacios anteriores a pal. */
/* al llegar aqu° *(*pfrase) ser† A-Z+•+0-9 */
do {
	palabra[i]=*(*pfrase);  /* mete caracter en palabra */
	(*pfrase)++;
	i++;
} while((i<5) && (esta_en(CAR_PAL,*(*pfrase))));    /* mientras car. v†lido */
for(;i<5;i++) palabra[i]=' ';   /* rellena con espacios */
palabra[i]='\0';    /* a§ade car. nulo al final */
while(esta_en(CAR_PAL,*(*pfrase))) (*pfrase)++; /* desprecia resto palabra */

/* comprobamos si la palabra est† en vocabulario */
*num=0;
*tipo=0;
if((i=esta_en_voc(palabra))==NUM_PAL+1) return(NO_PALABRA);
*num=vocabulario[i].num;    /* coge su n£mero */
*tipo=vocabulario[i].tipo;  /* y su tipo */
if(*tipo==_CONJ) return(SEPARADOR); /* si es una conjunci¢n */
if((*((*pfrase)-2)=='L') && ((*((*pfrase)-1)=='A') || (*((*pfrase)-1)=='E') ||
  (*((*pfrase)-1)=='O'))) return(TERMINACION); /* si termina en LA, LE o LO */
else if((*((*pfrase)-1)=='S') && (*((*pfrase)-3)=='L') &&
  ((*((*pfrase)-1)=='A') || (*((*pfrase)-1)=='E') || (*((*pfrase)-1)=='O')))
  return(TERMINACION);  /* si termina en LAS, LES o LOS */
else return(PALABRA);
}

/*****************************************************************************
	MAYUSCULA: convierte una letra en may£scula
*****************************************************************************/
char mayuscula(char c)
{
if((c>='a') && (c<='z')) return(c-'a'+'A');
switch(c) {
case (char)'§' :
	c=(char)'•';
	break;
case (char)'†' :
	c='A';
	break;
case (char)'Ç' :
	c='E';
	break;
case (char)'°' :
	c='I';
	break;
case (char)'¢' :
	c='O';
	break;
case (char)'£' :
case (char)'Å' :
	c='U';
	break;
}
return(c);
}

/*****************************************************************************
	ESTA_EN_VOC: comprueba si una palabra est† en el vocabulario.
	  Devuelve (NUM_PAL+1) si no se encontr¢.
	  Si se encontr¢ devuelve su posici¢n en el vocabulario
*****************************************************************************/
int esta_en_voc(char *pal)
{
int i;

for(i=0;i<pvoc;i++)     /* comprueba si es palabra de vocabulario */
  if(!strcmp(pal,vocabulario[i].p)) return(i);    /* palabra encontrada */

return(NUM_PAL+1);  /* palabra no encontrada */
}

/*****************************************************************************
	PARSE1: analiza la linea principal de entrada hasta un separador o
	  hasta el fin de linea.
	  Entrada: 'frase' conteniendo la linea a analizar
	  Salida:  TRUE si no hay m†s que analizar
		   FALSE si queda texto por analizar
		   Variables 2 a 6 actualizadas convenientemente
*****************************************************************************/
int parse1(void)
{
BYTE num, tipo;
BYTE nombrelo, adjtlo, verbo;
int res, f[3], i;

for(i=0;i<3;i++) f[i]=0;    /* inic. flags de palabras */
nombrelo=var[3];    /* guarda nombre para verbo con terminaci¢n */
adjtlo=var[4];      /* guarda adjetivo para verbo con terminaci¢n */
verbo=var[2];       /* guarda verbo por si teclea frase sin Çl */
for(i=2;i<7;i++) var[i]=NO_PAL; /* inic. variables */

do {
	res=analiza(&lin,&tipo,&num);  /* analiza una palabra */
	if(res==PALABRA) {  /* si es palabra sin terminaci¢n */
		if((tipo==_VERB) && f[_VERB]!=1) {
			var[2]=num; /* almacena num. verbo en variable 2 */
			f[_VERB]=1; /* indica que ya ha cogido verbo */
		}
		/* s¢lo coge los dos 1ß nombres */
		if((tipo==_NOMB) && (f[_NOMB]<2)) {
			if(!f[_NOMB]) {  /* almacena nombre 1 en var. 3 */
				var[3]=num;
				/* si es nomb. conv. tambiÇn lo almacena */
				/* en verbo si no cogi¢ antes ning£n */
				/* verbo ni otro nomb. conv. */
				if((num<n_conv) && !f[_VERB]) {
					var[2]=num;
					/* indica que hay nombre convert. */
					f[_VERB]=2;
					/* luego lo incrementar† */
					f[_NOMB]--;
				}
			}
			/* almacena nombre 2 en var. 5 */
			if(f[_NOMB]==1) var[5]=num;
			f[_NOMB]++; /* increm. num. nombres cogidos */
		}
		/* s¢lo coge los dos 1ß adjet. */
		if((tipo==_ADJT) && (f[_ADJT]<2)) {
			/* almacena adjetivo 1 en var. 4 */
			if(!f[_ADJT]) var[4]=num;
			/* almacena adjetivo 2 en var. 6 */
			if(f[_ADJT]==1) var[6]=num;
			f[_ADJT]++; /* increm. num. adjetivos cogidos */
		}
	}
	if(res==TERMINACION) {  /* si es palabra con terminaci¢n */
		if((tipo==_VERB) && f[_VERB]!=1) {
			var[2]=num; /* almacena num. verbo en variable 2 */
			f[_VERB]=1; /* indica que ya ha cogido el verbo */
			/* si nombre anterior no era Propio */
			if(nombrelo>=n_prop) {
				var[3]=nombrelo;
				var[4]=adjtlo;
				f[_NOMB]++; /* deja el nombre anterior */
				f[_ADJT]++; /* y su adjetivo */
			}
		}
		/* s¢lo coge los dos 1ß nombres */
		if((tipo==_NOMB) && (f[_NOMB]<2)) {
			if(!f[_NOMB]) {  /* almacena nombre 1 en var. 3 */
				var[3]=num;
				/* si es nomb. conv. tambiÇn lo almacena */
				/* en verbo si no cogi¢ antes ning£n */
				/* verbo ni otro nomb. conv. */
				if((num<n_conv) && !f[_VERB]) {
					var[2]=num;
					/* indica que hay nombre convert. */
					f[_VERB]=2;
					/* luego lo incrementar† */
					f[_NOMB]--;
				}
			}
			/* almacena nombre 2 en var. 5 */
			if(f[_NOMB]==1) var[5]=num;
			f[_NOMB]++; /* increm. num. nombres cogidos */
		}
		/* s¢lo coge los dos 1ß adjet. */
		if((tipo==_ADJT) && (f[_ADJT]<2)) {
			/* almacena adjetivo 1 en var. 4 */
			if(!f[_ADJT]) var[4]=num;
			/* almacena adjetivo 2 en var. 6 */
			if(f[_ADJT]==1) var[6]=num;
			f[_ADJT]++; /* increm. num. adjetivos cogidos */
		}
	}
} while((res!=FIN_FRASE) && (res!=SEPARADOR));
/* si tecle¢ una frase sin verbo pero con nombre, poner el verbo anterior */
if(!f[_VERB] && f[_NOMB]) var[2]=verbo;
if(res==FIN_FRASE) {
	if(!mas_texto) {
		mas_texto=1;    /* indicador para siguiente llamada */
		return(FALSE);  /* indica que analice lo £ltimo cogido */
	}
	else {
		/* si analiz¢ lo £ltimo cogido antes de fin frase */
		mas_texto=0;
		return(TRUE);   /* indica que no queda m†s por analizar */
	}
}
return(FALSE);  /* si es separador, supone que hay m†s texto detr†s */
}

/*****************************************************************************
	M_ERR: imprime mensajes de error en una ventana de pantalla.
	  Entrada: x,y  coordenada de inicio de texto dentro de ventana error
		   m    puntero a mensaje a imprimir
		   flag si no es 0 sale al sistema operativo con exit(flag);
*****************************************************************************/
void m_err(BYTE x,BYTE y,char *m,int flag)
{
union REGS dreg;
struct SREGS dsreg;

cls();              /* borra la pantalla */
if(*m) {            /* si no es mensaje vac°o */
	w_cls(&w_err);      /* borra ventana para mensajes de error */
	w_err.cwx=x;        /* coloca cursor */
	w_err.cwy=y;
	w_imps(m,&w_err);   /* imprime mensaje */
	lee_tecla();        /* espera a que se pulsa una tecla */
	cls();              /* borra la pantalla */
}
if(flag) {          /* si flag es distinto de 0 */
	_setvideomode(_DEFAULTMODE);
	dreg.h.ah=0x33;             /* obtener/establecer indicador ruptura */
	dreg.h.al=1;                /* establecerlo */
	dreg.h.dl=ruptura;          /* lo restaura */
	intdosx(&dreg,&dreg,&dsreg);
	exit(flag);     /* sale al sistema operativo */
}
}

/*****************************************************************************
	PROCESS: ejecuta una llamada a un proceso.
	  Entrada: pro num. de proceso
*****************************************************************************/
int process(BYTE prc)
{
if(ptrp==STK)   /* si se rebasa la capacidad de pila interna */
  m_err(3,1,"Rebosamiento de la pila interna.",1);

pila1[ptrp]=ptr_proc+2; /* guarda dir. sgte. condacto en proc. actual */
pila2[ptrp]=sgte_ent;   /*   "    desplazamiento de sgte. entrada */
pila3[ptrp]=pro_act;    /*   "    num. de proceso actual */
ptrp++;                 /* incrementa puntero de pila */
pro_act=prc;
ptr_proc=proc+pro[pro_act]-2;   /* dir. proceso llamado - 2 que se sumar† */
nueva_ent=1;    /* indica que no debe ajustar ptr_proc para sgte. entr. */
return(FALSE);  /* saltar† a inicio nueva entrada (la 1era del proc.) */
}

/*****************************************************************************
	DONE: finaliza un proceso retornando como TRUE.
*****************************************************************************/
int done(void)
{
if(!ptrp) m_err(0,0,"",1);
ptrp--;
ptr_proc=pila1[ptrp]-1; /* recupera dir. sgte condacto en proc. que llam¢ */
						/* -1 que se sumar† luego */
sgte_ent=pila2[ptrp];   /* desplazamiento de la sgte. entrada */
pro_act=pila3[ptrp];    /* num. de proceso que llam¢ */
nueva_ent=1;    /* indica que no debe ajustar ptr_proc para sgte. entr. */
return(TRUE);
}

/*****************************************************************************
	NOTDONE: finaliza un proceso retornando como FALSE.
*****************************************************************************/
int notdone(void)
{
done();             /* ejecuta un DONE */
nueva_ent=0;        /* para que ajuste a sgte. entrada */
return(FALSE);      /* pero sale como FALSE */
}

/*****************************************************************************
	RESP: activa comprobaci¢n de verbo-nombre al inicio de cada entrada.
*****************************************************************************/
int resp(void)
{
resp_act=1;
return(TRUE);
}

/*****************************************************************************
	NORESP: desactiva comprobaci¢n de verbo-nombre.
*****************************************************************************/
int noresp(void)
{
resp_act=0;
return(TRUE);
}

/*****************************************************************************
	DEFWIN: define una ventana.
	  Entrada: nw    num. de ventana
		   cw    color
		   wy,wx posici¢n de esquina superior izquierda (fila,columna)
		   lx,ly tama§o (ancho y alto) de la ventana
*****************************************************************************/
int defwin(BYTE nw,BYTE cw,BYTE wy,BYTE wx,BYTE lx,BYTE ly)
{
w[nw].wx=wx;
w[nw].wy=wy;
w[nw].lx=lx;
w[nw].ly=ly;
w[nw].color=cw;
w[nw].colort=cw;
w[nw].scroll=0;
w[nw].cwxs=0;
w[nw].cwys=0;
return(TRUE);
}

/*****************************************************************************
	WINDOW: selecciona la ventana activa.
	  Entrada: nw num. de ventana
*****************************************************************************/
int window(BYTE nw)
{
var[0]=nw;
return(TRUE);
}

/*****************************************************************************
	CLW: borra una ventana.
	  Entrada: nw num. de ventana
*****************************************************************************/
int clw(BYTE nw)
{
w_cls(&w[nw]);
return(TRUE);
}

/*****************************************************************************
	LET: asigna un valor a una variable.
	  Entrada: nv  num. de variable
			   val valor a asignar
*****************************************************************************/
int let(BYTE nv,BYTE val)
{
var[nv]=val;
return(TRUE);
}

/*****************************************************************************
	EQ: comprueba si una variable es igual a un valor.
	  Entrada: nv  num. de variable
		   val valor
	  Salida:  TRUE si la variable es igual al valor
		   FALSE si es distinta de valor
*****************************************************************************/
int eq(BYTE nv,BYTE val)
{
if(var[nv]==val) return(TRUE);
return(FALSE);
}

/*****************************************************************************
	NOTEQ: comprueba si una variable distinta de un valor.
	  Entrada: nv  num. de variable
		   val valor
	  Salida:  TRUE si la variable es distinta del valor
		   FALSE si es igual al valor
*****************************************************************************/
int noteq(BYTE nv,BYTE val)
{
return(TRUE-eq(nv,val));
}

/*****************************************************************************
	LT: comprueba si una variable es menor que un valor.
	  Entrada: nv  num. de variable
		   val valor
	  Salida:  TRUE si la variable es menor que valor
		   FALSE si es mayor o igual que valor
*****************************************************************************/
int lt(BYTE nv,BYTE val)
{
if(var[nv]<val) return(TRUE);
return(FALSE);
}

/*****************************************************************************
	GT: comprueba si una variable es mayor que un valor.
	  Entrada: nv  num. de variable
		   val valor
	  Salida:  TRUE si la variable es mayor que valor
		   FALSE si es menor o igual que valor
*****************************************************************************/
int gt(BYTE nv,BYTE val)
{
if(var[nv]>val) return(TRUE);
return(FALSE);
}

/*****************************************************************************
	NEWLINE: imprime un avance de linea.
*****************************************************************************/
int newline(void)
{
w_impc('\n',&w[var[0]]);
return(TRUE);
}

/*****************************************************************************
	MES: imprime un mensaje.
	  Entrada: nm num. de mensaje
*****************************************************************************/
int mes(BYTE nm)
{
char far *pm;

pm=msgs+mens[nm];   /* puntero a mensaje */
w_imps(pm,&w[var[0]]);  /* imprime mensaje en ventana activa */
return(TRUE);
}

/*****************************************************************************
	MESSAGE: imprime un mensaje con avance de linea.
	  Entrada: nm num. de mensaje
*****************************************************************************/
int message(BYTE nm)
{
mes(nm);    /* imprime mensaje */
newline();  /* y avance de linea */
return(TRUE);
}

/*****************************************************************************
	SYSMESS: imprime un mensaje del sistema.
	  Entrada: nm num. de mensaje
*****************************************************************************/
int sysmess(BYTE nm)
{
char far *pm;

pm=msgsys+mensys[nm];   /* puntero a mensaje */
w_imps(pm,&w[var[0]]);  /* imprime mensaje en ventana activa */
return(TRUE);
}

/*****************************************************************************
	DESC: imprime la descripci¢n de una localidad y salta al inicio del
	  Proceso 0. Si est† oscuro imprime el mensaje del sistema 23.
	  Entrada: nl num. de localidad
*****************************************************************************/
int desc(BYTE nl)
{
char far *pl;

pl=localidades+locs[nl];        /* puntero a descripci¢n de localidad */
/* si no est† oscuro o hay una fuente de luz imprime descripci¢n */
if(zero(0) || light()) w_imps(pl,&w[var[0]]);
else sysmess(23);       /* Est† oscuro. No puedes ver nada. */
clear(2);               /* indica que hay que inicializar */
			/* es decir, borrar ventana, listar objetos... */
restart();              /* salta al inicio de Proceso 0 */
ptr_proc--;     /* -1 a ptr_proc (y -1 del restart), luego se sumar†n 2 */
return(FALSE);
}

/*****************************************************************************
	ADD: suma un valor a una variable.
	  Entrada: nv  num. de variable
		   val valor a sumar
*****************************************************************************/
int add(BYTE nv,BYTE val)
{
var[nv]+=val;
return(TRUE);
}

/*****************************************************************************
	SUB: resta un valor a una variable.
	  Entrada: nv  num. de variable
		   val valor a restar
*****************************************************************************/
int sub(BYTE nv,BYTE val)
{
var[nv]-=val;
return(TRUE);
}

/*****************************************************************************
	INC: incrementa en 1 el valor de una variable.
	  Entrada: nv num. de variable
*****************************************************************************/
int inc(BYTE nv)
{
var[nv]++;
return(TRUE);
}

/*****************************************************************************
	DEC: decrementa en 1 el valor de una variable.
	  Entrada: nv num. de variable
*****************************************************************************/
int dec(BYTE nv)
{
var[nv]--;
return(TRUE);
}

/*****************************************************************************
	SET: pone a 1 un flag.
	  Entrada: nf num. de flag
*****************************************************************************/
int set(BYTE nf)
{
BYTE mascara_set=0x80;
int nbyte, nbit;

nbyte=nf/8;     /* num. de byte */
nbit=nf%8;      /* bit dentro del byte */
mascara_set>>=nbit;
flag[nbyte]|=mascara_set;
return(TRUE);
}

/*****************************************************************************
	CLEAR: pone a 0 un flag.
	  Entrada: nf num. de flag
*****************************************************************************/
int clear(BYTE nf)
{
BYTE mascara_clr=0x80;
int nbyte, nbit;
nbyte=nf/8;     /* num. de byte */
nbit=nf%8;      /* bit dentro del byte */
mascara_clr>>=nbit;                 /* desplaza hacia der. (a§ade 0) */
mascara_clr=(BYTE)0xff-mascara_clr; /* complementa mascara_clr */
flag[nbyte]&=mascara_clr;
return(TRUE);
}

/*****************************************************************************
	ZERO: comprueba si un flag es 0.
	  Entrada: nf num. de flag
	  Salida:  TRUE si el flag es 0
		   FALSE si el flag es 1
*****************************************************************************/
int zero(BYTE nf)
{
BYTE mascara=0x80;
int nbyte, nbit;

nbyte=nf/8;
nbit=nf%8;
mascara>>=nbit;
if(!(flag[nbyte] & mascara)) return(TRUE);
return(FALSE);
}

/*****************************************************************************
	NOTZERO: comprueba si un flag es 1.
	  Entrada: nf num. de flag
	  Salida:  TRUE si el flag es 1
		   FALSE si el flag es 0
*****************************************************************************/
int notzero(BYTE nf)
{
return(TRUE-zero(nf));
}

/*****************************************************************************
	PLACE: coloca un objeto en una localidad.
	  Entrada: nobj num. de objeto
		   nloc num. de localidad
*****************************************************************************/
int place(BYTE nobj,BYTE nloc)
{
BYTE locobj;

locobj=loc_obj[nobj];  /* coge la localidad actual del objeto */
/* si se refiere a loc. actual, sutituye el valor de loc. */
if(nloc==LOC_ACTUAL) nloc=var[1];
/* si loc. de obj. es igual a loc. de destino, no hace nada */
if(nloc==locobj) return(TRUE);
/* si pasa un objeto a cogido o puesto y no estaba ni cogido ni puesto */
/* incrementa el num. de objetos que lleva */
if(((nloc==COGIDO) || (nloc==PUESTO)) && (locobj!=COGIDO) &&
  (locobj!=PUESTO)) objs_cogidos++;
/* si el objeto estaba cogido o puesto y no lo pasa a cogido ni a puesto */
/* decrementa el num. de objetos que lleva */
if(((locobj==COGIDO) || (locobj==PUESTO)) && (nloc!=COGIDO) &&
  (nloc!=PUESTO)) objs_cogidos--;
loc_obj[nobj]=nloc;    /* actualiza pos. de objeto */
return(TRUE);
}

/*****************************************************************************
	GET: coge un objeto.
	  Entrada: nobj num. de objeto
	  Salida:  TRUE si se pudo coger el objeto
		   FALSE si no se pudo coger
	  NOTA: la variable 8 contendr† el n£mero del objeto a la salida
*****************************************************************************/
int get(BYTE nobj)
{
if(nobj>=objact) {  /* si el objeto no existe */
	sysmess(1);     /* Aqu° no est† eso. */
	return(FALSE);
}
var[8]=nobj;    /* coloca el n£mero del objeto en la variable 8 */
if(carried(nobj) || worn(nobj)) {   /* si ya ten°a el objeto */
	sysmess(3);     /* Ya tienes eso */
	return(FALSE);  /* debe hacer un DONE */
}
if(loc_obj[nobj]!=var[1]) {         /* si el objeto no est† presente */
	sysmess(1);     /* Aqu° no est† eso */
	return(FALSE);  /* debe hacer un DONE */
}
if((objs_cogidos>=var[7]) && (var[7]!=0)) { /* si lleva muchas cosas */
	sysmess(2);     /* No puedes coger _. Llevas demasiadas cosas. */
	return(FALSE);  /* debe hacer un DONE */
}
place(nobj,COGIDO); /* coge el objeto */
sysmess(0);         /* Has cogido _. */
return(TRUE);
}

/*****************************************************************************
	DROP: deja un objeto.
	  Entrada: nobj num. de objeto
	  Salida:  TRUE si se pudo dejar el objeto
		   FALSE si no se pudo dejar
	  NOTA: la variable 8 contendr† el n£mero del objeto a la salida
*****************************************************************************/
int drop(BYTE nobj)
{
if(nobj>=objact) {  /* si el objeto no existe */
	sysmess(5);     /* No tienes eso. */
	return(FALSE);
}
var[8]=nobj;    /* coloca el n£mero del objeto en la variable 8 */
if(notcarr(nobj) && notworn(nobj)) {    /* si no cogido ni puesto */
	sysmess(5);     /* No tienes eso. */
	return(FALSE);  /* debe hacer un DONE */
}
/* si lleva el objeto cogido o puesto, lo deja en loc. actual */
place(nobj,var[1]);
sysmess(4);     /* Dejas _. */
return(TRUE);
}

/*****************************************************************************
	INPUT: recoge texto del jugador.
	  Entrada: ninguna
	  Salida:  TRUE si no tecle¢ nada
		   FALSE si tecle¢ algo
*****************************************************************************/
int input(void)
{
int i;

for(i=2;i<7;i++) var[i]=NO_PAL; /* inicializa sentencia l¢gica */
ini_inp=1;      /* indica a PARSE inicio de frase */
mas_texto=0;    /* variable usada por parse1 para frases encadenadas */
/* NOTA: el cursor ser† el 1er caracter del mens. del sistema 7 */
w_input(frase,MAXLONG,*(msgsys+mensys[7]),&w[var[0]]); /* recoge frase */
if(*frase=='\0') return(TRUE);  /* si no tecle¢ nada */
return(FALSE);
}

/*****************************************************************************
	PARSE: analiza la frase tecleada por jugador.
	  Entrada: 'frase' conteniendo frase tecleada
		   'ini_inp' =1 si hay que analizar desde principio
			     =0 si sigue donde lo dej¢ en la ult. llamada
	  Salida:  TRUE si se analiz¢ toda la frase
		   FALSE si queda m†s por analizar
*****************************************************************************/
int parse(void)
{
int par;

if(ini_inp) {   /* si inicio de frase */
	lin=frase;  /* coloca ptro al principio de linea tecleada */
	ini_inp=0;
}
par=parse1();   /* analiza hasta separador o fin linea */
return(par);
}

/*****************************************************************************
	SKIP: salto relativo dentro de un proceso.
	  Entrada: lsb,hsb bytes bajo y alto del desplazamiento del
	  salto (-32768 a 32767)
*****************************************************************************/
int skip(BYTE lsb,BYTE hsb)
{
int despli;

despli=(hsb << 8) | lsb;    /* calcula desplazamiento */
ptr_proc=ptr_proc+despli-3; /* -3 que se sumar†n luego */
nueva_ent=1;    /* para que no ajuste ptr_proc a sgte. entrada */
return(FALSE);  /* saltar† a inicio de entrada */
}

/*****************************************************************************
	AT: comprueba si est† en una localidad.
	  Entrada: locno num. de localidad
	  Salida:  TRUE si est† en esa localidad
		   FALSE si no est† en esa localidad
*****************************************************************************/
int at(BYTE locno)
{
if(var[1]==locno) return(TRUE);
else return(FALSE);
}

/*****************************************************************************
	NOTAT: comprueba que no est† en una localidad.
	  Entrada: locno num. de localidad
	  Salida:  TRUE si no est† en esa localidad
		   FALSE si est† en esa localidad
*****************************************************************************/
int notat(BYTE locno)
{
return(TRUE-at(locno));
}

/*****************************************************************************
	ATGT: comprueba si est† en una localidad superior a la dada.
	  Entrada: locno num. de localidad
	  Salida:  TRUE si est† en una localidad superior
		   FALSE si est† en una localidad inferior o igual
*****************************************************************************/
int atgt(BYTE locno)
{
if(var[1]>locno) return(TRUE);
else return(FALSE);
}

/*****************************************************************************
	ATLT: comprueba si est† en una localidad inferior a la dada.
	  Entrada: locno num. de localidad
	  Salida:  TRUE si est† en una localidad inferior
		   FALSE si est† en una localidad superior o igual
*****************************************************************************/
int atlt(BYTE locno)
{
if(var[1]<locno) return(TRUE);
else return(FALSE);
}

/*****************************************************************************
	ADJECT1: comprueba el 1er adjetivo de la sentencia l¢gica.
	  Entrada: adj num. de adjetivo
	  Salida:  TRUE si el adjet. 1 en la sentencia l¢gica (var[4]) es el dado
		   FALSE si no
*****************************************************************************/
int adject1(BYTE adj)
{
if(var[4]==adj) return(TRUE);
else return(FALSE);
}

/*****************************************************************************
	NOUN2: comprueba el 2o nombre de la sentencia l¢gica.
	  Entrada: nomb num. de nombre
	  Salida:  TRUE si el nombre 2 en la sentencia l¢gica (var[5]) es el dado
		   FALSE si no
*****************************************************************************/
int noun2(BYTE nomb)
{
if(var[5]==nomb) return(TRUE);
else return(FALSE);
}

/*****************************************************************************
	ADJECT2: comprueba el 2o adjetivo de la sentencia l¢gica.
	  Entrada: adj num. de adjetivo
	  Salida:  TRUE si el adjet. 2 en la sentencia l¢gica (var[6]) es el dado
		   FALSE si no
*****************************************************************************/
int adject2(BYTE adj)
{
if(var[6]==adj) return(TRUE);
else return(FALSE);
}

/*****************************************************************************
	LISTAT: lista los objetos presentes en una localidad.
	  Entrada: locno num. de localidad
*****************************************************************************/
int listat(BYTE locno)
{
BYTE i, j=0;
char far *po;
BYTE obl[MAX_OBJ];      /* tabla para guardar n£meros de objs. a listar */

if(locno==LOC_ACTUAL) locno=var[1];
/* recorre toda la tabla de objetos */
for(i=0;i<objact;i++) {
	if(loc_obj[i]==locno) {
		obl[j]=i;       /* almacena num. de objeto */
		j++;
	}
}
if(!j) sysmess(10);     /* si no se list¢ ning£n objeto */
else {
	for(i=0;i<j;i++) {          /* recupera objetos guardados */
		po=obj+objs[obl[i]]+6;  /* puntero a texto objeto */
		if(notzero(1)) {        /* si flag 1=1, columna */
			w_imps(po,&w[var[0]]);      /* imprime £ltimo objeto */
			w_impc('\n',&w[var[0]]);    /* sgte. l°nea */
		}
		else {
			w_imps(po,&w[var[0]]);    /* imprime objeto */
			if(i==(j-1)) sysmess(13); /* final de la lista */
			else if(i==(j-2)) sysmess(12);  /* uni¢n 2 £ltimos */
			else sysmess(11); /* separaci¢n entre objs. */
		}
	}
}
return(TRUE);
}

/*****************************************************************************
	ISAT: comprueba si un objeto est† en una localidad.
	  Entrada: nobj  num. de objeto
		   locno num. de localidad
	  Salida:  TRUE si el objeto est† en la localidad
		   FALSE si no est† en la localidad
*****************************************************************************/
int isat(BYTE nobj,BYTE locno)
{
if(locno==LOC_ACTUAL) locno=var[1];
if(loc_obj[nobj]==locno) return(TRUE);
else return(FALSE);
}

/*****************************************************************************
	ISNOTAT: comprueba si un objeto no est† en una localidad.
	  Entrada: nobj  num. de objeto
		   locno num. de localidad
	  Salida:  TRUE si el objeto no est† en la localidad
		   FALSE si est† en la localidad
*****************************************************************************/
int isnotat(BYTE nobj,BYTE locno)
{
return(TRUE-isat(nobj,locno));
}

/*****************************************************************************
	PRESENT: comprueba si un objeto est† presente (en loc. actual, cogido
	  o puesto).
	  Entrada: nobj num. de objeto
	  Salida:  TRUE si el objeto est† presente
		   FALSE si no est† presente
*****************************************************************************/
int present(BYTE nobj)
{
if(isat(nobj,LOC_ACTUAL) || isat(nobj,COGIDO) || isat(nobj,PUESTO))
  return(TRUE);
else return(FALSE);
}

/*****************************************************************************
	ABSENT: comprueba si un objeto no est† presente.
	  Entrada: nobj num. de objeto
	  Salida:  TRUE si el objeto no est† presente
		   FALSE si est† presente
*****************************************************************************/
int absent(BYTE nobj)
{
return(TRUE-present(nobj));
}

/*****************************************************************************
	WORN: comprueba si un objeto est† puesto.
	  Entrada: nobj num. de objeto
	  Salida:  TRUE si el objeto est† puesto
		   FALSE si no est† puesto
*****************************************************************************/
int worn(BYTE nobj)
{
if(isat(nobj,PUESTO)) return(TRUE);
else return(FALSE);
}

/*****************************************************************************
	NOTWORN: comprueba si un objeto no est† puesto.
	  Entrada: nobj num. de objeto
	  Salida:  TRUE si el objeto no est† puesto
		   FALSE si est† puesto
*****************************************************************************/
int notworn(BYTE nobj)
{
return(TRUE-worn(nobj));
}

/*****************************************************************************
	CARRIED: comprueba si un objeto est† cogido.
	  Entrada: nobj num. de objeto
	  Salida:  TRUE si el objeto est† cogido
		   FALSE si no est† cogido
*****************************************************************************/
int carried(BYTE nobj)
{
if(isat(nobj,COGIDO)) return(TRUE);
else return(FALSE);
}

/*****************************************************************************
	NOTCARR: comprueba si un objeto no est† cogido.
	  Entrada: nobj num. de objeto
	  Salida:  TRUE si el objeto no est† cogido
		   FALSE si est† cogido
*****************************************************************************/
int notcarr(BYTE nobj)
{
return(TRUE-carried(nobj));
}

/*****************************************************************************
	WEAR: pone un objeto que sea una prenda.
	  Entrada: nobj num. de objeto
	  Salida:  TRUE si se pudo poner el objeto
		   FALSE si no se pudo poner
	  NOTA: la variable 8 contendr† el n£mero del objeto a la salida
*****************************************************************************/
int wear(BYTE nobj)
{
char far *po;

if(nobj>=objact) {  /* si el objeto no existe */
	sysmess(5);     /* No tienes eso. */
	return(FALSE);
}
var[8]=nobj;    /* coloca el n£mero del objeto en la variable 8 */
po=obj+objs[nobj]+3;    /* puntero a flags1 de objeto */
if(worn(nobj)) {    /* si el objeto est† puesto */
	sysmess(16);    /* Ya llevas puesto _. */
	return(FALSE);  /* debe hacer un DONE */
}
if(absent(nobj)) {  /* si el objeto no est† aqu° */
	sysmess(1);     /* Aqu° no est† eso */
	return(FALSE);  /* debe hacer un DONE */
}
if(notcarr(nobj)) { /* si el objeto no est† cogido */
	sysmess(5);     /* No tienes eso. */
	return(FALSE);
}
if(!(*po & 0x01)) { /* si no es una prenda */
	sysmess(17);    /* No puedes ponerte _. */
	return(FALSE);  /* debe hacer un DONE */
}
place(nobj,PUESTO); /* se pone la prenda */
sysmess(18);        /* Te pones _. */
return(TRUE);
}

/*****************************************************************************
	REMOVE: quita un objeto que sea una prenda.
	  Entrada: nobj num. de objeto
	  Salida:  TRUE si se pudo quitar el objeto
		   FALSE si no se pudo quitar
	  NOTA: la variable 8 contendr† el n£mero del objeto a la salida
*****************************************************************************/
int remove1(BYTE nobj)
{
if(nobj>=objact) {  /* si el objeto no existe */
	sysmess(19);    /* No llevas puesto eso. */
	return(FALSE);
}
var[8]=nobj;    /* coloca el n£mero del objeto en la variable 8 */
if(notworn(nobj)) {     /* si no lo lleva puesto */
	sysmess(19);    /* No llevas puesto eso. */
	return(FALSE);  /* debe hacer un DONE */
}
place(nobj,COGIDO); /* pasa el objeto a cogido */
sysmess(20);        /* Te quitas _. */
return(TRUE);
}

/*****************************************************************************
	CREATE: pasa un objeto no creado a la localidad actual.
	  Entrada: nobj num. de objeto
*****************************************************************************/
int create(BYTE nobj)
{
if(isat(nobj,NO_CREADO)) place(nobj,LOC_ACTUAL); /* pasa obj. a loc. actual */
return(TRUE);
}

/*****************************************************************************
	DESTROY: pasa un objeto a no creado.
	  Entrada: nobj num. de objeto
*****************************************************************************/
int destroy(BYTE nobj)
{
place(nobj,NO_CREADO);  /* pasa el objeto a no creado */
return(TRUE);
}

/*****************************************************************************
	SWAP: intercambia dos objetos.
	  Entrada: nobj1 num. de objeto 1
			   nobj2 num. de objeto 2
*****************************************************************************/
int swap(BYTE nobj1,BYTE nobj2)
{
BYTE locobj2;

locobj2=loc_obj[nobj2];         /* guarda localidad de 2ß objeto */
place(nobj2,loc_obj[nobj1]);    /* pasa 2ß a localidad del 1ß */
place(nobj1,locobj2);           /* pasa 1ß a localidad del 2ß */
return(TRUE);
}

/*****************************************************************************
	RESTART: salta al inicio del proceso 0.
*****************************************************************************/
int restart(void)
{
ptrp=0;                 /* inicializa puntero de pila */
pro_act=0;              /* proceso actual 0 */
ptr_proc=proc+pro[0]-1; /* ptro. a inicio Proceso 0, -1 que luego se suma */
nueva_ent=1;            /* no debe ajustar ptr_proc para sgte. entrada */
return(FALSE);
}

/*****************************************************************************
	WHATO: devuelve el num. de objeto que se corresponde con el nombre1,
	  adjetivo1 de la sentencia l¢gica actual.
	  Entrada: var[3] nombre
		   var[4] adjetivo
	  Salida:  var[8] num. de objeto
*****************************************************************************/
int whato(void)
{
BYTE i;
char far *po;

var[8]=255;     /* n£mero de objeto no v†lido */
for(i=0;i<objact;i++) {
	po=obj+objs[i];     /* puntero a nombre-adjetivo objeto */
	/* si encuentra el objeto, coloca su n£mero en var[8] */
	if((var[3]==(BYTE)*po) && ((var[4]==NO_PAL) || 
	  (var[4]==(BYTE)*(po+1)))) {
		var[8]=i;
		break;
	}
}
return(TRUE);
}

/*****************************************************************************
	MOVE: actualiza el contenido de una variable de acuerdo a su contenido
	  actual, a la sentencia l¢gica y a la tabla de conexiones, para que
	  contenga el num. de localidad con la que conecta una dada por medio
	  de la palabra de movimiento de la sentencia l¢gica.
	  Entrada: nv num. de variable (cuyo contenido es el num. de una loca-
		     lidad v†lida)
		   var[2] y var[3] sentencia l¢gica
	  Salida:  TRUE si hay conexi¢n y nv modificada para que contenga un
		     num. de localidad que conecta con la dada por medio de
		     la tabla de conexiones
		   FALSE si no hay conexi¢n en esa direcci¢n y nv sin
		     modificar
*****************************************************************************/
int move(BYTE nv)
{
char far *pc;

pc=conx+conxs[var[nv]];     /* ptro. a conexiones de loc. var[nv] */
while(*pc) {
	if((var[2]==*pc && var[3]==NO_PAL) ||
	  (var[2]==NO_PAL && var[3]==*pc) || (var[2]==*pc && var[3]==*pc)) {
		var[nv]=*(pc+1); /* guarda num. loc. hacia la que conecta */
		return(TRUE);    /* y sale */
	}
	pc+=2;                  /* siguiente conexi¢n */
}
return(FALSE);  /* si no encontr¢ conexi¢n */
}

/*****************************************************************************
	ISMOV: comprueba si la sentencia l¢gica es de movimiento 
	  (movim. - NO_PAL, NO_PAL - movim. o movim. - movim.).
	  Entrada: var[2] y var[3] con nombre-verbo
	  Salida:  TRUE si es sentencia l¢gica de movimiento
		   FALSE si no lo es
*****************************************************************************/
int ismov(void)
{
if((var[2]<v_mov && var[3]==NO_PAL) || (var[2]==NO_PAL && var[3]<v_mov) ||
  (var[2]<v_mov && var[3]<v_mov)) return(TRUE);
else return(FALSE);
}

/*****************************************************************************
	GOTO: va a una localidad especificada.
	  Entrada: locno  num. de localidad
	  Salida:  var[1] conteniendo el num. de nueva localidad (si locno es
	    localidad no v†lida, el contenido de var[1] no se modifica)
*****************************************************************************/
int goto1(BYTE locno)
{
if(locno<n_loc) var[1]=locno;
return(TRUE);
}

/*****************************************************************************
	PRINT: imprime el contenido de una variable en la posici¢n actual.
	  Entrada: nv num. de variable
*****************************************************************************/
int print(BYTE nv)
{
w_impn((unsigned)var[nv],&w[var[0]]);
return(TRUE);
}

/*****************************************************************************
	DPRINT: imprime el contenido de dos variables consecutivas como un
	  n£mero de 16 bits en la posici¢n actual.
	  Entrada: nv num. de la primera variable
	  NOTA: si nv=255 el resultado ser† impredecible
*****************************************************************************/
int dprint(BYTE nv)
{
unsigned num;

num=(var[nv]*256)+var[nv+1];
w_impn(num,&w[var[0]]);
return(TRUE);
}

/*****************************************************************************
	CLS: borra la pantalla dej†ndola con los atributos por defecto (tinta
	  blanca, papel negro, sin brillo ni parpadeo).
*****************************************************************************/
int cls(void)
{
scrollw(0,0,80,25,7,0);     /* borra pantalla */
return(TRUE);
}

/*****************************************************************************
	ANYKEY: espera hasta que se pulse una tecla.
*****************************************************************************/
int anykey(void)
{
sysmess(22);    /* Pulsa una tecla. */
lee_tecla();
return(TRUE);
}

/*****************************************************************************
	PAUSE: realiza una pausa de una duraci¢n determinada o hasta que se
	  pulse una tecla.
	  Entrada: pau valor de la pausa en dÇcimas de segundo
*****************************************************************************/
int pause(BYTE pau)
{
pausa((clock_t)(pau*100));  /* hace la pausa */
/* saca tecla del buffer */
if(_bios_keybrd(_KEYBRD_READY)) _bios_keybrd(_KEYBRD_READ);
return(TRUE);
}

/*****************************************************************************
	LISTOBJ: lista los objetos de la localidad actual.
*****************************************************************************/
int listobj(void)
{
if(zero(0) || light()) {    /* si no est† oscuro o hay luz lista objetos */
	sysmess(9);
	listat(var[1]);
}
return(TRUE);
}

/*****************************************************************************
	FIRSTO: coloca el puntero NEXTO al principio de la tabla de objetos.
*****************************************************************************/
int firsto(void)
{
ptr_nexto=-1;       /* coloca puntero al inicio de la tabla-1 */
doall=1;            /* indica bucle DOALL */
return(TRUE);
}

/*****************************************************************************
	NEXTO: mueve el puntero NEXTO al siguiente objeto que estÇ en la loca-
	  lidad especificada.
	  Entrada: locno num. de localidad
	  Salida:  variables 3 y 4 con el nombre y adjetivo del siguiente
		     objeto encontrado (si el objeto encontrado es el £ltimo
		     pone a 0 doall)
*****************************************************************************/
int nexto(BYTE locno)
{
char far *po;
int i;

if(!doall) return(TRUE);
if(locno==LOC_ACTUAL) locno=var[1];
do {
	ptr_nexto++;
	if(ptr_nexto>=(int)objact) {                /* si lleg¢ al final */
		doall=0;                            /* cancela bucle DOALL */
		return(TRUE);
	}
	if(loc_obj[ptr_nexto]==locno) {             /* si objeto est† en loc. */
		po=obj+objs[ptr_nexto];             /* puntero a objeto */
		var[3]=(BYTE)*po;                   /* pone nombre */
		var[4]=(BYTE)*(po+1);               /*   "  adjetivo */
		/* mira si es el £timo objeto en la localidad indicada */
		for(i=ptr_nexto+1;i<(int)objact;i++) {
			if(loc_obj[i]==locno) break;
		}
		/* si es ult. objeto desactiva bucle DOALL */
		if(i>=(int)objact) doall=0;
		return(TRUE);
	}
} while(1);
}

/*****************************************************************************
	SYNONYM: coloca el verbo-nombre dado en las variables 2 y 3, si alguno
	  es NO_PAL no realiza la sustituci¢n.
*****************************************************************************/
int synonym(BYTE verb,BYTE nomb)
{
if(verb!=NO_PAL) var[2]=verb;
if(nomb!=NO_PAL) var[3]=nomb;
return(TRUE);
}

/*****************************************************************************
	HASAT: comprueba si el objeto actual (cuyo num. est† en la variable
	  del sistema 8) tiene activada una bandera de usuario.
	  Entrada: val num. de bandera de usuario a comprobar (0 a 17)
		     16 comprueba si es PRENDA y 17 si FUENTE DE LUZ
	  Salida:  TRUE si la bandera de usuario est† a 1
		   FALSE si est† a 0
*****************************************************************************/
int hasat(BYTE val)
{
char far *po;
unsigned flags2, masc=0x8000;

po=obj+objs[var[8]]+4;      /* puntero a flags2 de objeto */
if(val==16 || val==17) {    /* comprobar PRENDA o FUENTE LUZ */
	po--;       /* puntero a flags1 de objeto */
	if((val==16) && (*po & 0x01)) return(TRUE);     /* comprueba PRENDA */
	if((val==17) && (*po & 0x02)) return(TRUE);     /* comprueba LUZ */
	return(FALSE);
}
flags2=(*po*256)+*(po+1);   /* coge las banderas de usuario */
masc>>=val;                 /* desplaza m†scara */
if(flags2 & masc) return(TRUE);     /* si es 1 el bit correspondiente */
return(FALSE);
}

/*****************************************************************************
	HASNAT: comprueba si el objeto actual (cuyo num. est† en la variable
	  del sistema 8) no tiene activada una bandera de usuario.
	  Entrada: val num. de bandera de usuario a comprobar (0 a 17)
		     16 comprueba si es PRENDA y 17 si FUENTE DE LUZ
	  Salida:  TRUE si la bandera de usuario est† a 0
		   FALSE si est† a 1
*****************************************************************************/
int hasnat(BYTE val)
{
return(TRUE-hasat(val));
}

/*****************************************************************************
	LIGHT: comprueba si hay presente una fuente de luz.
	  Salida: TRUE si hay presente una fuente de luz
		  FALSE si no
*****************************************************************************/
int light(void)
{
BYTE i;
char far *po;

for(i=0;i<objact;i++) {     /* recorre tabla de objetos */
	po=obj+objs[i]+3;       /* puntero a flags1 de objeto */
	/* si es fuente de luz y adem†s est† presente, sale con TRUE */
	if(*po & 0x02) if(present(i)) return(TRUE);
}
return(FALSE);              /* si no hay presente ninguna fuente de luz */
}

/*****************************************************************************
	NOLIGHT: comprueba si no hay presente ninguna fuente de luz.
	  Salida: TRUE si no hay presente ninguna fuente de luz
		  FALSE si hay presente al menos una fuente de luz
*****************************************************************************/
int nolight(void)
{
return(TRUE-light());
}

/*****************************************************************************
	RANDOM: genera n£meros aleatorios.
	  Entrada: varno num. de variable que contendr† el num. aleatorio
		   rnd   l°mite de num. aleatorio
	  Salida:  varno conteniendo un num. aleatorio entre 0 y rnd-1
*****************************************************************************/
int random(BYTE varno,BYTE rnd)
{
int x;

x=32767/rnd;
var[varno]=(BYTE)(rand()/x);
return(TRUE);
}

/*****************************************************************************
	SEED: coloca el punto de inicio del generador de nums. aleatorios
	  Entrada: seed punto de inicio dentro de la serie de nums. aleato-
		     rios (si es 1 reinicializa el generador)
*****************************************************************************/
int seed(BYTE seed)
{
srand((unsigned)seed);
return(TRUE);
}

/*****************************************************************************
	PUTO: coloca el objeto actual cuyo num. est† en la variable 8 en
	  una localidad
	  Entrada: nloc num. de localidad
*****************************************************************************/
int puto(BYTE nloc)
{
place(var[8],nloc);
return(TRUE);
}

/*****************************************************************************
	INKEY: coloca en las variables 9 y 10 el par de c¢digos ASCII IBM
	  de la £ltima tecla pulsada (si se puls¢ alguna).
	  Salida: TRUE si se puls¢ una tecla y adem†s...
		    -variable 9 conteniendo 1er c¢digo ASCII IBM (c¢digo
		    ASCII del caracter, si es distinto de 0)
		    -variable 10 conteniendo 2o c¢digo ASCII IBM (c¢digo
		    de scan de la tecla pulsada)
		  FALSE si no se puls¢ ninguna tecla (deja sin modificar las
		    variables 9 y 10)
*****************************************************************************/
int inkey(void)
{
unsigned tecla;

if(_bios_keybrd(_KEYBRD_READY)) {   /* si hay tecla esperando */
	tecla=_bios_keybrd(_KEYBRD_READ);   /* recoge c¢digos tecla pulsada */
	var[9]=(BYTE)(tecla & 0x00ff);      /* c¢digo ASCII, byte bajo */
	var[10]=(BYTE)(tecla >> 8);         /* c¢digo scan, byte alto */
	return(TRUE);
}
return(FALSE);
}

/*****************************************************************************
	COPYOV: copia el n£mero de localidad en la que est† el objeto
	  referenciado en una variable dada.
	  Entrada: nobj  num. de objeto
		   varno num. de variable
*****************************************************************************/
int copyov(BYTE nobj,BYTE varno)
{
var[varno]=loc_obj[nobj];
return(TRUE);
}

/*****************************************************************************
	CHANCE: comprueba una probabilidad en tanto por ciento.
	  Entrada: rnd  probabilidad de 0 a 100
	  Salida:  TRUE si el num. aleatorio generado internamente es menor o
		     igual que rnd
		   FALSE si es mayor que rnd
*****************************************************************************/
int chance(BYTE rnd)
{
BYTE chc;

chc=(BYTE)(rand()/327);     /* n£mero aleatorio entre 0 y 100 */
if(chc>rnd) return(FALSE);
return(TRUE);
}

/*****************************************************************************
	RAMSAVE: graba en uno de los bancos de ram disponibles el estado
	  actual (variables, banderas y posici¢n actual de objetos).
	  Entrada: banco n£m de banco de memoria a usar (0 o 1)
*****************************************************************************/
int ramsave(BYTE banco)
{
int i;
BYTE *pram;

if(banco==0) {      /* selecci¢n de banco de memoria */
	pram=ram0_b;
	ram0=1;         /* banco usado */
}
else if(banco==1) {
	pram=ram1_b;
	ram1=1;         /* banco usado */
}
for(i=0;i<VARS;i++) pram[i]=var[i];                     /* guarda variables */
for(i=0;i<BANDS;i++) pram[VARS+i]=flag[i];              /* guarda banderas */
for(i=0;i<MAX_OBJ;i++) pram[VARS+BANDS+i]=loc_obj[i];   /* guarda loc. obj. */
return(TRUE);
}

/*****************************************************************************
	RAMLOAD: recupera una posici¢n guardada con RAMSAVE.
	  Entrada: banco n£m. de banco de memoria a usar (0 o 1)
		   vtop  m†ximo n£m. de variable a recuperar (se recuperar†
		     desde la 0 hasta vtop inclusive)
		   ftop  m†ximo n£m. de bandera a recuperar (se recuperar†
		     desde la 0 hasta ftop inclusive)
	  Salida:  TRUE si se pudo ejecutar RAMLOAD
		   FALSE si el banco indicado no fuÇ usado antes por RAMSAVE
*****************************************************************************/
int ramload(BYTE banco,BYTE vtop,BYTE ftop)
{
int i, nbyte, nbit;
BYTE *pram, mascara;

if(banco==0) {                  /* selecci¢n del banco de memoria */
	if(!ram0) return(FALSE);    /* si el banco 0 no ha sido usado */
	pram=ram0_b;
}
else if(banco==1) {
	if(!ram1) return(FALSE);    /* si el banco 1 no ha sido usado */
	pram=ram1_b;
}
for(i=0;i<=(int)vtop;i++) var[i]=pram[i];   /* recupera variables */
for(i=0;i<=(int)ftop;i++) {                 /* recupera banderas */
	nbyte=i/8;
	nbit=i%8;
	mascara=0x80;
	mascara>>=nbit;
	if(pram[VARS+nbyte] & mascara) set((BYTE)i);
	else clear((BYTE)i);
}
for(i=0;i<MAX_OBJ;i++) loc_obj[i]=pram[VARS+BANDS+i];
return(TRUE);
}

/*****************************************************************************
	ABILITY: designa el n£mero de objetos m†ximo que puede ser llevado.
	  Entrada: nobjs n£mero de objetos m†ximo (0 ilimitados)
*****************************************************************************/
int ability(BYTE nobjs)
{
let(7,nobjs);       /* m†ximo n£mero de objetos en variable 7 */
return(TRUE);
}

/*****************************************************************************
	AUTOG: coge el objeto cuyo nombre-adjetivo est†n en las variables
	  3 y 4 respectivamente (es como WHATO seguido de GET [8]).
	  Salida: TRUE si se pudo coger el objeto
		  FALSE si no se pudo coger
*****************************************************************************/
int autog(void)
{
whato();
return(get(var[8]));
}

/*****************************************************************************
	AUTOD: deja el objeto cuyo nombre-adjetivo est†n en las variables
	  3 y 4 respectivamente (es como WHATO seguido de DROP [8]).
	  Salida: TRUE si se pudo dejar el objeto
		  FALSE si no se pudo dejar
*****************************************************************************/
int autod(void)
{
whato();
return(drop(var[8]));
}

/*****************************************************************************
	AUTOW: pone el objeto cuyo nombre-adjetivo est†n en las variables
	  3 y 4 respectivamente (es como WHATO seguido de WEAR [8]).
	  Salida: TRUE si se pudo poner el objeto
		  FALSE si no se pudo poner
*****************************************************************************/
int autow(void)
{
whato();
return(wear(var[8]));
}

/*****************************************************************************
	AUTOR: quita el objeto cuyo nombre-adjetivo est†n en las variables
	  3 y 4 respectivamente (es como WHATO seguido de REMOVE [8]).
	  Salida: TRUE si se pudo quitar el objeto
		  FALSE si no se pudo quitar
*****************************************************************************/
int autor(void)
{
whato();
return(remove1(var[8]));
}

/*****************************************************************************
	ISDOALL: comprueba si se est† ejecutando un bucle DOALL (FIRSTO ha
	  sido ejecutado y NEXTO no ha alcanzado todav°a £ltimo objeto).
	  Salida: TRUE si se est† ejecutando un bucle DOALL
		  FALSE si no
*****************************************************************************/
int isdoall(void)
{
if(doall) return(TRUE);
return(FALSE);
}

/*****************************************************************************
	ASK: realiza una pregunta y espera hasta que se introduce la
	  respuesta.
	  Entrada: smess1 mensaje del sistema que contiene la pregunta
		   smess2    "     "     "    con las posibles respuestas
		     (cada una de una sola letra y seguidas, no importa si
		     en may£sculas o min£sculas), el m†ximo n£mero de carac-
		     teres permitidos es 256 (0 a 255), si son m†s la varia-
		     ble varno contendr† resultados imprevisibles
	  Salida:  varno  n£m. de variable que contendr† la respuesta (0 si
		     se tecle¢ 1er car†cter de smess2, 1 si el 2o, 2 si el
		     3ero, etc...)
	  NOTA: s¢lo se admiten teclas almacenadas en la cadena TECLAS global
*****************************************************************************/
int ask(BYTE smess1,BYTE smess2,BYTE varno)
{
char far *pm;
unsigned key;
BYTE i;
BYTE oldx, oldy;

sysmess(smess1);                /* imprime la pregunta */
oldx=w[var[0]].cwx;             /* guarda posici¢n actual del cursor */
oldy=w[var[0]].cwy;
do {
	w[var[0]].cwx=oldx;         /* recupera coordenadas */
	w[var[0]].cwy=oldy;
	w_impc(*(msgsys+mensys[7]),&w[var[0]]);     /* imprime cursor */
	w[var[0]].cwx=oldx;         /* recupera coordenadas */
	w[var[0]].cwy=oldy;
	key=mayuscula((char)lee_tecla());
	w_impc((char)key,&w[var[0]]);   /* imprime tecla pulsada */
	i=0;                        /* comienza b£squeda */
	pm=msgsys+mensys[smess2];   /* puntero a mensaje con las respuestas */
	while(*pm) {
		if(mayuscula(*pm)==(char)key) {
			var[varno]=i;   /* almacena num. de car. de smess2 */
			return(TRUE);   /* y sale */
		}
		pm++;               /* pasa al siguiente car†cter de smess2 */
		i++;
	}
} while(1);
}

/*****************************************************************************
	QUIT: presenta el mensaje del sistema 24 (®Est†s seguro?) y pregunta
	  para abandonar.
	  Salida: TRUE si se responde con el 1er caracter del mensaje del
		    sistema 25
		  FALSE si se responde con el 2o caracter del mensaje del
		    sistema 25
	  NOTA: se usa la variable 11
*****************************************************************************/
int quit(void)
{
ask(24,25,11);              /* hace pregunta ®Est†s seguro? */
newline();
if(!var[11]) return(TRUE);  /* si respondi¢ con 1er car†cter */
return(FALSE);
}

/*****************************************************************************
	SAVE: guarda la posici¢n actual en disco (variables, banderas y
	  posici¢n actual de objetos).
*****************************************************************************/
int save(void)
{
int h_save;

sysmess(26);                /* Nombre del fichero: */
w_input(f_sl,MAXLONG,*(msgsys+mensys[7]),&w[var[0]]);   /* nombre fichero */
newline();
if(!access(f_sl,0)) {       /* si fichero ya existe */
	ask(27,25,11);      /* Fichero ya existe. ®Quieres continuar? */
	newline();
	if(var[11]) return(TRUE);  /* si respondi¢ con 2o car. mens. sist. 25 */
}
h_save=open(f_sl,O_CREAT|O_TRUNC|O_BINARY|O_RDWR,S_IREAD|S_IWRITE);
if(h_save==-1) {            /* Ocurri¢ un error en apertura */
	sysmess(28);        /* Error de apertura de fichero. */
	return(TRUE);
}
/* escribe cadena de reconocimiento */
if(write(h_save,SRECON,L_RECON)==-1) {
	sysmess(29);            /* Error de entrada/salida en fichero. */
	close(h_save);
	return(TRUE);
}
if(write(h_save,(char *)var,VARS)==-1) {            /* guarda variables */
	sysmess(29);            /* Error de entrada/salida en fichero. */
	close(h_save);
	return(TRUE);
}
if(write(h_save,(char *)flag,BANDS)==-1) {          /* guarda banderas */
	sysmess(29);            /* Error de entrada/salida en fichero. */
	close(h_save);
	return(TRUE);
}
if(write(h_save,(char *)loc_obj,MAX_OBJ)==-1) {     /* guarda loc. objetos */
	sysmess(29);            /* Error de entrada/salida en fichero. */
	close(h_save);
	return(TRUE);
}
close(h_save);
return(TRUE);
}

/*****************************************************************************
	LOAD: recupera de disco una posici¢n grabada con SAVE.
	  Entrada: vtop  m†ximo n£m. de variable a recuperar (se recuperar†
		     desde la 0 hasta vtop inclusive)
		   ftop  m†ximo n£m. de bandera a recuperar (se recuperar†
		     desde la 0 hasta ftop inclusive)
*****************************************************************************/
int load(BYTE vtop,BYTE ftop)
{
int h_load, i, nbyte, nbit;
BYTE mascara;
static char rec_ld[L_RECON];
BYTE var_l[VARS];
BYTE flag_l[BANDS];

sysmess(26);                    /* Nombre del fichero: */
w_input(f_sl,MAXLONG,*(msgsys+mensys[7]),&w[var[0]]);   /* nombre fichero */
newline();
h_load=open(f_sl,O_BINARY|O_RDONLY);
if(h_load==-1) {                /* Ocurri¢ un error en apertura */
	sysmess(28);            /* Error de apertura de fichero. */
	return(TRUE);
}
/* recoge cadena de reconocimiento */
if(read(h_load,rec_ld,L_RECON)==-1) {
	sysmess(29);            /* Error de entrada/salida en fichero. */
	close(h_load);
	return(TRUE);
}
if(strcmp(SRECON,rec_ld)) {     /* comprueba cadena de reconocimiento */
	sysmess(30);                /* Fichero no v†lido. */
	close(h_load);
	return(TRUE);
}
if(read(h_load,(char *)var_l,VARS)==-1) {           /* recoge variables */
	sysmess(29);            /* Error de entrada/salida en fichero. */
	close(h_load);
	return(TRUE);
}
if(read(h_load,(char *)flag_l,BANDS)==-1) {         /* recoge banderas */
	sysmess(29);            /* Error de entrada/salida en fichero. */
	close(h_load);
	return(TRUE);
}
if(read(h_load,(char *)loc_obj,MAX_OBJ)==-1) {      /* recoge loc. objetos */
	sysmess(29);            /* Error de entrada/salida en fichero. */
	close(h_load);
	return(TRUE);
}
for(i=0;i<=(int)vtop;i++) var[i]=var_l[i];          /* recupera variables */
for(i=0;i<=(int)ftop;i++) {                         /* recupera banderas */
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

/*****************************************************************************
	EXIT: sale al sistema operativo o reinicia.
	  Entrada: ex si es 0 reinicia, si es 1 sale al sistema operativo
*****************************************************************************/
int exit1(BYTE ex)
{
if(ex==1) m_err(0,0,"",1);
/* reinicializaci¢n */
cls();          /* borra la pantalla */
inic();         /* reinicializa variables */
restart();      /* se pone al inicio de proceso 0 */
ptr_proc--;     /* -1 a ptr_proc (y -1 del restart), luego se sumar†n 2 */
return(FALSE);
}

/*****************************************************************************
	END: pregunta para otra partida o salir. Si se responde con el primer
	  car†cter del mensaje del sistema 25, ejecuta un EXIT 0 (reiniciali-
	  zaci¢n). Si no sale al sistema operativo.
*****************************************************************************/
int end1(void)
{
ask(31,25,11);          /* ®Lo intentas de nuevo? */
newline();
if(!var[11]) {          /* si respondi¢ con 1er car. de mens. sist. 25 */
	exit1(0);       /* reinicializaci¢n */
	ptr_proc++;     /* +1 a ptr_proc (-2 del exit1), luego se sumar† 1 */
	return(FALSE);
}
exit1(1);               /* sale al sistema operativo */
}

/*****************************************************************************
	PRINTAT: coloca el cursor en una posici¢n dada de la ventana actual.
	  Entrada: y,x coordenada del cursor (fila, columna); si salen de los
		     l°mites de la ventana el cursor se colocar† en la
		     esquina superior izquierda (0,0)
*****************************************************************************/
int printat(BYTE y,BYTE x)
{
/* si se sale de ventana */
if((y>(w[var[0]].ly-1)) || (x>(w[var[0]].lx-1))) y=x=0;
w[var[0]].cwy=y;
w[var[0]].cwx=x;
return(TRUE);
}

/*****************************************************************************
	SAVEAT: almacena la posici¢n de impresi¢n de la ventana actual. Cada
	  ventana tiene sus propias posiciones de impresi¢n almacenadas.
*****************************************************************************/
int saveat(void)
{
w[var[0]].cwxs=w[var[0]].cwx;
w[var[0]].cwys=w[var[0]].cwy;
return(TRUE);
}

/*****************************************************************************
	BACKAT: recupera la posici¢n de impresi¢n guardada por el £ltimo
	  SAVEAT en la ventana actual.
	  NOTA: si no se ejecut¢ ning£n SAVEAT, la posici¢n de impresi¢n
	  recuperada ser† la (0,0).
*****************************************************************************/
int backat(void)
{
w[var[0]].cwx=w[var[0]].cwxs;
w[var[0]].cwy=w[var[0]].cwys;
return(TRUE);
}

/*****************************************************************************
	NEWTEXT: deshecha el resto de la l°nea de input que a£n queda por ana-
	  lizar y coloca el puntero al final de la misma.
*****************************************************************************/
int newtext(void)
{
for(;*lin;lin++);   /* coloca puntero al final de la frase */
mas_texto=1;        /* indica que no queda m†s por analizar */
return(TRUE);
}

/*****************************************************************************
	PRINTC: imprime un caracter en la posici¢n actual del cursor y dentro
	  de la ventana activa.
	  Entrada: car c¢digo ASCII del caracter a imprimir
*****************************************************************************/
int printc(BYTE car)
{
w_impc(car,&w[var[0]]);     /* imprime el caracter en la ventana activa */
return(TRUE);
}

/*****************************************************************************
	INK: selecciona el color temporal de la tinta en la ventana activa.
	  Todos los textos de esa ventana se imprimir†n con ese color de
	  tinta.
	  Entrada: color n£mero del color 0 a 7
*****************************************************************************/
int ink(BYTE color)
{
w[var[0]].colort &= 0xf8;   /* borra el antiguo color de tinta */
w[var[0]].colort |= color;  /* y pone el nuevo */
return(TRUE);
}

/*****************************************************************************
	PAPER: selecciona el color temporal del fondo en la ventana activa.
	  Todos los textos de esa ventana se imprimir†n con ese color de
	  fondo.
	  Entrada: color n£mero del color 0 a 7
*****************************************************************************/
int paper(BYTE color)
{
w[var[0]].colort &= 0x8f;       /* borra el antiguo color del fondo */
w[var[0]].colort |= (color<<4); /* y pone el nuevo */
return(TRUE);
}

/*****************************************************************************
	BRIGHT: selecciona el atributo de brillo temporal para la ventana
	  activa. Todos los textos de esa ventana se imprimir†n con ese
	  brillo.
	  Entrada: b brillo (0=sin brillo, 1=con brillo)
*****************************************************************************/
int bright(BYTE b)
{
if(b) w[var[0]].colort |= 0x08; /* activar brillo */
else w[var[0]].colort &= 0xf7;  /* desactivar brillo */
return(TRUE);
}

/*****************************************************************************
	BLINK: selecciona el atributo de parpadeo temporal para la ventana
	  activa. Todos los textos de esa ventana se imprimir†n con ese
	  parpadeo.
	  Entrada: b parpadeo (0=sin parpadeo, 1=con parpadeo)
*****************************************************************************/
int blink(BYTE b)
{
if(b) w[var[0]].colort |= 0x80; /* activar parpadeo */
else w[var[0]].colort &= 0x7f;  /* desactivar parpadeo */
return(TRUE);
}

/*****************************************************************************
	COLOR: selecciona un color (fondo, tinta, brillo y parpadeo) temporal
	  para la ventana activa.
	  Entrada: col color
*****************************************************************************/
int color(BYTE col)
{
w[var[0]].colort=col;
return(TRUE);
}

/*****************************************************************************
	INT24_HND: rutina de manejo de errores cr°ticos de hardware.
*****************************************************************************/
void far int24_hnd(unsigned deverror,unsigned errcode,unsigned far *devhdr)
{
_hardresume(_HARDERR_FAIL);
}
