#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>
#include <stddef.h>
#include <conio.h>
#include <io.h>
#include <bios.h>
#include "version.h"
#include "sintac.h"

/****************************************/
/************** Constantes **************/
/****************************************/

#define COLUMNAS 80     /* columnas en pantalla */
#define FILAS    25     /* filas en pantalla */

#define NS 7            /* n£mero de secciones */
#define L_MAX 129       /* longitud m xima de la linea de entrada + 1 */
#define V_MOV  14       /* m ximo n£m. para verbos de movimiento */
#define N_CONV 20       /*    "    "     "  nombres convertibles */
#define N_PROP 50       /*    "    "     "     "    propios */
#define SYS_MESS 32     /* m¡nimo n£mero de mensajes del sistema */

#define _VOC 0
#define _OBJ 1
#define _LOC 2
#define _MSG 3
#define _MSY 4
#define _PRO 5
#define _END 6

/****************************************/
/***** Tipos de datos y estructuras *****/
/****************************************/

struct coords {
	BYTE fila;
	BYTE columna;
};

/****************************************/
/********** Variables globales **********/
/****************************************/

FILE *f_in;
FILE *f_sal;
/* para guardar nombre del fichero de salida */
char nombres[_MAX_DRIVE+_MAX_DIR+_MAX_FNAME+_MAX_EXT];
char lin[L_MAX];    /* buffer para guardar l¡nea de entrada */
char *l;            /* puntero a l¡nea de entrada */
unsigned long numlin=0;
int compiled[NS]={0,0,0,0,0,0};
struct coords c;    /* para guardar coordenadas del cursor */
const char n_secc[][4]={"VOC","OBJ","LOC","MSG","MSY","PRO","END"};
const char *mem_err="Memoria insuficiente para %s\n";
const char *fsal_err="Error de escritura en fichero de salida [%s].\n";
const char *c_errores[]={
	"--- Fin de fichero ---\n",
	"Error en lectura del fichero de entrada.",
	"Secci¢n no v lida.",
	"Secci¢n repetida.",
	"Marca de secci¢n no encontrada.",
	"Marca de fin de secci¢n (\\END) no encontrada.",
	"Marca \\END repetida.",
	"Car cter no v lido en palabra de vocabulario.",
	"Faltan campos.",
	"N£mero de palabra no v lido.",
	"Campo num‚rico no v lido.",
	"Tipo de palabra no v lido.",
	"Palabra repetida.",
	"Vocabulario lleno.",
	"Falta \'@\'.",
	"N£mero fuera de secuencia.",
	"Se rebas¢ el l¡mite de memoria para la secci¢n actual.",
	"Falta \'@\' de fin de texto.",
	"N£mero no v lido.",
	"Error en puntero a fichero de entrada.",
	"Palabra no est  en vocabulario.",
	"Secci¢n de Vocabulario debe aparecer antes que esta secci¢n.",
	"La palabra no es verbo de movimiento.",
	"Se rebas¢ el l¡mite de memoria para las Conexiones.",
	"Conexi¢n a localidad no v lida.",
	"Secci¢n de Localidades debe aparecer antes que esta secci¢n.",
	"El primer campo debe ser un Nombre.",
	"El segundo campo debe ser un Adjetivo.",
	"N£mero de Localidad no v lido.",
	"N£mero de Objeto no v lido.",
	"Banderas no v lidas.",
	"Falta texto de Objeto.",
	"Secci¢n de Objetos debe aparecer antes que esta secci¢n.",
	"Secci¢n de Mensajes debe aparecer antes que esta secci¢n.",
	"Secci¢n de Mensajes del Sistema debe aparecer antes que esta secci¢n.",
	"Primera palabra en entrada no es verbo ni nombre convertible.",
	"Segunda palabra en entrada no es nombre.",
	"Condacto no v lido.",
	"N£mero de ventana no v lido.",
	"Columna de inicio de ventana fuera de pantalla.",
	"Fila de inicio de ventana fuera de pantalla.",
	"Dimensiones de ventana demasiado grandes.",
	"N£mero de mensaje no v lido.",
	"N£mero de localidad no v lido.",
	"N£mero de objeto no v lido.",
	"Comienzo de entrada no v lido.",
	"Un proceso no puede llamarse a s¡ mismo.",
	"Rebasado m ximo n£mero de llamadas 'forward' a procesos.",
	"Llamada a proceso inexistente.",
	"Campo de etiqueta no v lido.",
	"Rebasado m ximo n£mero de saltos 'forward' con SKIP.",
	"Salto fuera de rango.",
	"Rebasado m ximo n£mero de etiquetas.",
	"Etiqueta no definida.",
	"Salto 'forward' fuera de rango.",
	"Etiqueta repetida.",
	"Adjetivo no v lido.",
	"Nombre no v lido.",
	"Bandera de usuario no v lida.",
	"Par metro de CHANCE debe estar entre 0 y 100.",
	"Banco de memoria no v lido.",
	"N£mero de primer mensaje del sistema no v lido.",
	"N£mero de segundo mensaje del sistema no v lido.",
	"Par metro de EXIT err¢neo, debe ser 0 o 1.",
	"Fila fuera de pantalla.",
	"Columna fuera de pantalla.",
	"N£mero de color incorrecto.",
};
/* c¢digos de error */
enum cod_error {
	_E_NERR=0,
	_E_LECF,
	_E_SCCI,
	_E_SCRP,
	_E_MSCC,
	_E_MEND,
	_E_REND,
	_E_CVOC,
	_E_FCAM,
	_E_NPAL,
	_E_CNUM,
	_E_TPAL,
	_E_PREP,
	_E_MVOC,
	_E_FALT,
	_E_NFSC,
	_E_FMEM,
	_E_NFTX,
	_E_NVAL,
	_E_FENT,
	_E_NPVC,
	_E_SVOC,
	_E_NMOV,
	_E_NMCN,
	_E_CLOC,
	_E_SLOC,
	_E_NNOM,
	_E_NADJ,
	_E_LNVL,
	_E_NONV,
	_E_FLAG,
	_E_OTEX,
	_E_SOBJ,
	_E_SMSG,
	_E_SMSY,
	_E_NVNC,
	_E_NNNN,
	_E_NCND,
	_E_WPNW,
	_E_WPWX,
	_E_WPWY,
	_E_WPFP,
	_E_MPNM,
	_E_MPNL,
	_E_OPNV,
	_E_NENT,
	_E_PRPR,
	_E_PRRB,
	_E_PRLL,
	_E_NLAB,
	_E_RBTS,
	_E_LBFR,
	_E_RBTL,
	_E_RFFW,
	_E_LFFR,
	_E_LREP,
	_E_NCAD,
	_E_NNMB,
	_E_NVHT,
	_E_CHNV,
	_E_RBNV,
	_E_MPNP,
	_E_MPNS,
	_E_NOEX,
	_E_EFIL,
	_E_ECOL,
	_E_COLR,
};
int err_conx=0; /* variable usada por la funci¢n chequea_conx */

const char *nomb_seccs[]={
	"Vocabulario",
	"Objetos",
	"Localidades",
	"Mensajes",
	"Mensajes del Sistema",
	"Procesos"
};

const char *ABECEDARIO="ABCDEFGHIJKLMN¥OPQRSTUVWXYZ";
const char *NUMEROS="0123456789";

#define N_CONDACTOS 101
const char condactos[][8]={
	"",
	"PROCESS",
	"DONE   ",
	"NOTDONE",
	"RESP   ",
	"NORESP ",
	"DEFWIN ",
	"WINDOW ",
	"CLW    ",
	"LET    ",
	"EQ     ",
	"NOTEQ  ",
	"LT     ",
	"GT     ",
	"MES    ",
	"NEWLINE",
	"MESSAGE",
	"SYSMESS",
	"DESC   ",
	"ADD    ",
	"SUB    ",
	"INC    ",
	"DEC    ",
	"SET    ",
	"CLEAR  ",
	"ZERO   ",
	"NOTZERO",
	"PLACE  ",
	"GET    ",
	"DROP   ",
	"INPUT  ",
	"PARSE  ",
	"SKIP   ",
	"AT     ",
	"NOTAT  ",
	"ATGT   ",
	"ATLT   ",
	"ADJECT1",
	"NOUN2  ",
	"ADJECT2",
	"LISTAT ",
	"ISAT   ",
	"ISNOTAT",
	"PRESENT",
	"ABSENT ",
	"WORN   ",
	"NOTWORN",
	"CARRIED",
	"NOTCARR",
	"WEAR   ",
	"REMOVE ",
	"CREATE ",
	"DESTROY",
	"SWAP   ",
	"RESTART",
	"WHATO  ",
	"MOVE   ",
	"ISMOV  ",
	"GOTO   ",
	"PRINT  ",
	"DPRINT ",
	"CLS    ",
	"ANYKEY ",
	"PAUSE  ",
	"LISTOBJ",
	"FIRSTO ",
	"NEXTO  ",
	"SYNONYM",
	"HASAT  ",
	"HASNAT ",
	"LIGHT  ",
	"NOLIGHT",
	"RANDOM ",
	"SEED   ",
	"PUTO   ",
	"INKEY  ",
	"COPYOV ",
	"CHANCE ",
	"RAMSAVE",
	"RAMLOAD",
	"ABILITY",
	"AUTOG  ",
	"AUTOD  ",
	"AUTOW  ",
	"AUTOR  ",
	"ISDOALL",
	"ASK    ",
	"QUIT   ",
	"SAVE   ",
	"LOAD   ",
	"EXIT   ",
	"END    ",
	"PRINTAT",
	"SAVEAT ",
	"BACKAT ",
	"NEWTEXT",
	"PRINTC ",
	"INK    ",
	"PAPER  ",
	"BRIGHT ",
	"BLINK  ",
	"COLOR  ",
};

/** variables para Vocabulario **/
struct palabra vocabulario[NUM_PAL];    /* para almacenar vocabulario */
int pvoc=0;         /* n£mero de £ltima palabra de vocabulario */
BYTE v_mov=V_MOV;   /* guarda m x. n£m. de verbo de movimiento */
BYTE n_conv=N_CONV; /* guarda m x. n£m. de nombre convertible */
BYTE n_prop=N_PROP; /* guarda m x. n£m. de nombre propio */

/** variables para Mensajes **/
ptrdiff_t mens[MAX_MSG];    /* tabla de desplaz. de mensajes */
char far *msgs;             /* puntero a inicio de zona de mensajes */
char far *act_msg;          /*    "    " mensaje actual */
char far *ult_msg;          /*    "    " £ltimo byte de zona mensajes */
BYTE m_act=0;               /* n£mero de mensaje actual */

/** variables para Mensajes de Sistema **/
ptrdiff_t mensys[MAX_MSY];  /* tabla de desplaz. de mensajes */
char far *msgsys;           /* puntero a inicio de zona de mensajes */
char far *act_msgsys;       /*    "    " mensaje actual */
char far *ult_msgsys;       /*    "    " £ltimo byte de mensajes sist. */
BYTE m_actsys=0;            /* n£mero de mensaje actual */

/** variables para Localidades **/
BYTE n_loc=0;               /* n£mero de localidad actual */
ptrdiff_t locs[MAX_LOC];    /* tabla de desplaz. de texto locs. */
char far *localidades;      /* puntero a inicio de texto de locs. */
char far *act_loc;          /*    "    " texto loc. actual */
char far *ult_loc;          /*    "    "   "   £ltimo byte de locs. */
/** variables para Conexiones **/
ptrdiff_t conxs[MAX_LOC];   /* tabla de desplaz. de lista de conexiones */
char far *conx;             /* puntero a inicio de zona de conexiones */
char far *act_conx;         /*    "    " conexiones de loc. actual */
char far *ult_conx;         /*    "    " £ltimo byte de conexiones */

/** variables para Objetos **/
BYTE objact=0;              /* n£mero de objeto actual */
ptrdiff_t objs[MAX_OBJ];    /* tabla de desplaz. de lista de objetos */
char far *obj;              /* puntero a inicio de zona de objetos */
char far *act_obj;          /*    "    " objeto actual */
char far *ult_obj;          /*    "    " £ltimo byte de objetos */

/** variables para Procesos **/
BYTE far *proc;             /* puntero a inicio de zona de Procesos */
BYTE far *proc_act;         /*    "    " proceso actual */
BYTE far *proc_ult;         /*    "    " £ltimo byte de procesos */
ptrdiff_t pro[MAX_PRO];     /* tabla de desplaz. de procesos */
BYTE proact=0;              /* num. de proceso actual */

/** tabla y variables para comprobar llamadas a procesos forward **/
#define MAXPROCESS 2000 /* m ximo n£mero de llamadas a procesos forward */
struct pro_comp {
	BYTE numpro;        /* num. de proceso al que se llama con PROCESS */
	unsigned long nl;   /* num. de l¡nea de archivo de entrada d¢nde llama */
} prc[MAXPROCESS];
int pprc=0;             /* puntero dentro de la tabla */

/** tabla y variables para condacto SKIP y etiquetas **/
#define LABELS 100      /* m ximo n£mero de etiquetas */
#define LONG_LAB 15     /* longitud m xima+1 de etiqueta */
#define FSKIP 100       /* m ximo n£mero de llamadas forward */
struct lab {
	char label[LONG_LAB];   /* para guardar nombre etiqueta */
	BYTE far *plab;         /*   "     "    direcci¢n etiqueta */
} etiqueta[LABELS];
int ptlab;              /* puntero a tabla de etiquetas */
struct skp {
	char label[LONG_LAB];   /* nombre etiqueta a sustituir */
	BYTE far *fsk;          /* d¢nde sustituir etiqueta */
	unsigned long nl;       /* num. l¡nea archivo de entrada */
} fskip[FSKIP];
int ptskip;             /* puntero a tabla dir. etiquetas forward */

/****************************************/
/************** Prototipos **************/
/****************************************/

size_t ajusta_block(size_t tam,char *params);
void fwrt2(const void *buff,size_t tam,size_t cant);
int wrt_fich(char far *buff,unsigned tam,FILE *fp);
void estadisticas(void);
int si_o_no(char *preg);
char *salta_espacios(char *s);
int tipo_lin(char *l0);
int lee_linea(void);
char mayuscula(char c);
int esta_en(const char *l0,char c);
char *hasta_espacio(char *s);
int fin_linea(char *s);
int comp_voc(void);
int coge_pal(char *pal);
int coge_num(BYTE *n,char *l0);
int mete_palabra(struct palabra *p);
int compara_pal(struct palabra *p1,struct palabra *p2);
int comp_txt(char *l0,ptrdiff_t *mens,char far *(*tmsg), char far *(*actm),
  char far *(*ultm),BYTE *act,BYTE maximo);
int recoge_texto(char far *(*act),char far *ult,char *l0);
int comprueba_linea(char *l0);
int comp_loc(void);
int esta_en_voc(char *pal);
int chequea_conx(void);
int comp_obj(void);
BYTE es_flag1(char c);
int comp_pro(void);
void warning(char *w);
int chequea_pro(void);
int check_mem(ptrdiff_t tam);
int coge_num2(BYTE *n, char *l0);
int sgte_campo(void);
int mete_fsk(char *etq,BYTE far *dir);
int sust_labels(void);
struct coords coge_cur(void);
void pon_cur(struct coords c);

int process(void);
int done(void);
int notdone(void);
int resp(void);
int noresp(void);
int defwin(void);
int window(void);
int clw(void);
int let(void);
int eq(void);
int noteq(void);
int lt(void);
int gt(void);
int mes(void);
int newline(void);
int message(void);
int sysmess(void);
int desc(void);
int add(void);
int sub(void);
int inc(void);
int dec(void);
int set(void);
int clear(void);
int zero(void);
int notzero(void);
int place(void);
int get(void);
int drop(void);
int input(void);
int parse(void);
int skip(void);
int at(void);
int notat(void);
int atgt(void);
int atlt(void);
int adject1(void);
int noun2(void);
int adject2(void);
int listat(void);
int isat(void);
int isnotat(void);
int present(void);
int absent(void);
int worn(void);
int notworn(void);
int carried(void);
int notcarr(void);
int wear(void);
int remove1(void);
int create(void);
int destroy(void);
int swap(void);
int restart(void);
int whato(void);
int move(void);
int ismov(void);
int goto1(void);
int print(void);
int dprint(void);
int cls(void);
int anykey(void);
int pause(void);
int listobj(void);
int firsto(void);
int nexto(void);
int synonym(void);
int hasat(void);
int hasnat(void);
int light(void);
int nolight(void);
int random(void);
int seed(void);
int puto(void);
int inkey(void);
int copyov(void);
int chance(void);
int ramsave(void);
int ramload(void);
int ability(void);
int autog(void);
int autod(void);
int autow(void);
int autor(void);
int isdoall(void);
int ask(void);
int quit(void);
int save(void);
int load(void);
int exit1(void);
int end1(void);
int printat(void);
int saveat(void);
int backat(void);
int newtext(void);
int printc(void);
int ink(void);
int paper(void);
int bright(void);
int blink(void);
int color(void);

/*** Tabla de funciones para compilar condactos ***/
static int (*cd[])()={
	0       ,           /* condacto 0, reservado para fin entrada */
	process ,
	done    ,
	notdone ,
	resp    ,
	noresp  ,
	defwin  ,
	window  ,
	clw     ,
	let     ,
	eq      ,
	noteq   ,
	lt      ,
	gt      ,
	mes     ,
	newline ,
	message ,
	sysmess ,
	desc    ,
	add     ,
	sub     ,
	inc     ,
	dec     ,
	set     ,
	clear   ,
	zero    ,
	notzero ,
	place   ,
	get     ,
	drop    ,
	input   ,
	parse   ,
	skip    ,
	at      ,
	notat   ,
	atgt    ,
	atlt    ,
	adject1 ,
	noun2   ,
	adject2 ,
	listat  ,
	isat    ,
	isnotat ,
	present ,
	absent  ,
	worn    ,
	notworn ,
	carried ,
	notcarr ,
	wear    ,
	remove1 ,
	create  ,
	destroy ,
	swap    ,
	restart ,
	whato   ,
	move    ,
	ismov   ,
	goto1   ,
	print   ,
	dprint  ,
	cls     ,
	anykey  ,
	pause   ,
	listobj ,
	firsto  ,
	nexto   ,
	synonym ,
	hasat   ,
	hasnat  ,
	light   ,
	nolight ,
	random  ,
	seed    ,
	puto    ,
	inkey   ,
	copyov  ,
	chance  ,
	ramsave ,
	ramload ,
	ability ,
	autog   ,
	autod   ,
	autow   ,
	autor   ,
	isdoall ,
	ask     ,
	quit    ,
	save    ,
	load    ,
	exit1   ,
	end1    ,
	printat ,
	saveat  ,
	backat  ,
	newtext ,
	printc  ,
	ink     ,
	paper   ,
	bright  ,
	blink   ,
	color   ,
};

/****************************************/
/********** Programa principal **********/
/****************************************/

void main(int argc,char *argv[])
{
int i, sin_compilar=0;
enum cod_error error=_E_NERR;
enum {VOC=0,OBJ,LOC,MSG,MSY,PRO,END} analiza, ant_secc;
static char seccion[4];
char ns[129];
char nombre[129], drive_n[_MAX_DRIVE], dir_n[_MAX_DIR],
  fname_n[_MAX_FNAME], ext_n[_MAX_EXT];
unsigned t_msg, t_msy, t_loc, t_con, t_obj, t_pro;
size_t block=65000;     /* tama¤o bloques de memoria */
int nombre1=0, nombre2=0;

printf("\n"COPYRIGHT);
printf("Compilador Versi¢n "VERSION_CS);

if(argc>=2) {   /* si tecle¢ al menos 1 par metro de entrada */
	if(argv[1][0]=='/') block=ajusta_block(block,argv[1]);
	else {  /* si no empieza con '/' supone que es nombre fichero */
		for(i=0;argv[1][i];i++) nombre[i]=mayuscula(argv[1][i]);
		nombre[i]='\0';
		nombre1=1;      /* indica que cogi¢ nombre fich. entrada */
	}
}

if(argc>=3) {   /* si tecle¢ al menos 2 par metros de entrada */
	if(!nombre1) {
		for(i=0;argv[2][i];i++) nombre[i]=mayuscula(argv[2][i]);
		nombre[i]='\0';
		nombre1=1;      /* indica que cogi¢ nombre fich. entrada */
	}
	else {
		for(i=0;argv[2][i];i++) nombres[i]=mayuscula(argv[2][i]);
		nombres[i]='\0';
		nombre2=1;      /* indica que cogi¢ nomb. fich. de salida */
	}
}

/* si tecle¢ 3 o m s par metros de entrada y no cogi¢ nombre fich. de salida*/
if((argc>=4) && !nombre2) {
	for(i=0;argv[3][i];i++) nombres[i]=mayuscula(argv[3][i]);
	nombres[i]='\0';
	nombre2=1;      /* indica que cogi¢ nomb. fich. de salida */
}

if(!nombre1) {
	printf("Nombre del fichero a compilar: ");
	gets(nombre);   /* coge nombre de fichero de entrada */
	for(i=0;nombre[i];i++) nombre[i]=mayuscula(nombre[i]);
}
if ((f_in=fopen(nombre,"r"))==NULL) {
	printf("Error de apertura de fichero de entrada [%s].\n",nombre);
	exit(1);
}

if(!nombre2) {
	_splitpath(nombre,drive_n,dir_n,fname_n,ext_n);
	strcpy(nombres,drive_n);                /* construye path */
	strcat(nombres,dir_n);
	strcat(nombres,fname_n);                /* construye ruta, sin ext */
	strcat(nombres,".DAT");                 /* ruta con extensi¢n */
	printf("Fichero de salida [%s]: ",nombres);
	gets(ns);   /* coge nombre de fichero de salida */
	for(i=0;ns[i];i++) ns[i]=mayuscula(ns[i]);
	if(*ns!='\0') strcpy(nombres,ns); /* si introduce nombre, coge ese */
}

if(!access(nombres,0)) {    /* comprueba si fich. existe */
	printf("Fichero %s ya existe.",nombres);
	if(!si_o_no(" ¨Continuar? ")) exit(1);
}
printf("\n");

/* Reserva de memoria para las distintas secciones */
/* -- Mensajes -- */
printf("Memoria reservada para...\n");
if((msgs=(char far *)_fmalloc(block))==NULLF) {
	printf(mem_err,nomb_seccs[_MSG]);
	exit(1);
}
act_msg=msgs;
ult_msg=msgs+block-1;
printf("MENSAJES:\t\t%u\n",block);

/* -- Mensajes del Sistema -- */
if((msgsys=(char far *)_fmalloc(block))==NULLF) {
	printf(mem_err,nomb_seccs[_MSY]);
	exit(1);
}
act_msgsys=msgsys;
ult_msgsys=msgsys+block-1;
printf("MENSAJES DEL SISTEMA:\t%u\n",block);

/* -- Localidades -- */
if((localidades=(char far *)_fmalloc(block))==NULLF) {
	printf(mem_err,nomb_seccs[_LOC]);
	exit(1);
}
act_loc=localidades;
ult_loc=localidades+block-1;
printf("LOCALIDADES:\t\t%u\n",block);
/* -- Conexiones -- */
if((conx=(char far *)_fmalloc(block/8))==NULLF) {
	printf(mem_err,"Tabla de Conexiones");
	exit(1);
}
act_conx=conx;
ult_conx=conx+(block/8)-1;
printf("TABLA DE CONEXIONES:\t%u\n",block/8);

/* -- Objetos -- */
if((obj=(char far *)_fmalloc(block/2))==NULLF) {
	printf(mem_err,nomb_seccs[_OBJ]);
	exit(1);
}
act_obj=obj;
ult_obj=obj+(block/2)-1;
printf("OBJETOS:\t\t%u\n",block/2);

/* -- Procesos -- */
if((proc=(BYTE far *)_fmalloc(block))==NULLF) {
	printf(mem_err,nomb_seccs[_PRO]);
	exit(1);
}
proc_act=proc;
proc_ult=proc+block-1;
printf("PROCESOS:\t\t%u\n\n",block);

/* Compilador */
analiza=END+1;    /* inicializa secci¢n a analizar */

do {
	if(error=lee_linea()) break;  /* error de lectura o fin de fichero */
	if(*l=='\\') {      /* ¨inicio de secci¢n? */
		l++;
		for(i=0;i<3;i++) seccion[i]=*l++;
		seccion[i]='\0';
		error=_E_SCCI;  /* marca error de secci¢n no v lida */
		for(i=0;i<NS;i++) {
			if(!strcmp(seccion,n_secc[i])) {
				/* si encuentra marca de secci¢n */
				/* guarda la secci¢n anterior */
				/* y almacena secci¢n nueva */
				error=_E_NERR;
				ant_secc=analiza;
				analiza=i;
				if(analiza!=END)
				  if(analiza==PRO) {
					if(proact==0) c=coge_cur();
					pon_cur(c);
					printf
					  ("--- Compilando Proceso %u ---",
					  (unsigned)proact);
				  }
				  else printf
				    ("--- Compilando secci¢n de %s ---\n",
				    nomb_seccs[analiza]);
				break;
			}
		}
		if(!error) {
			if((ant_secc<PRO) && (analiza!=END)) {
				error=_E_MEND;
				break;
			}
			else {
				/* secci¢n repetida */
				if((analiza!=END) && (analiza!=PRO) &&
				  compiled[analiza]) {
					error=_E_SCRP;
					break;
				}
			}
		}
		else break;
	}
	switch(analiza) {
	case VOC :
		error=comp_voc();
		break;
	case OBJ :
		error=comp_obj();
		break;
	case LOC :
		error=comp_loc();
		break;
	case MSG :
		error=comp_txt(l,mens,&msgs,&act_msg,&ult_msg,&m_act,
		  (BYTE)(MAX_MSG-1));
		break;
	case MSY :
		error=comp_txt(l,mensys,&msgsys,&act_msgsys,&ult_msgsys,
		  &m_actsys,(BYTE)(MAX_MSY-1));
		break;
	case PRO :
		error=comp_pro();
		if(error) break;
		/* sustituye etiquetas forward */
		else error=sust_labels();
		break;
	case END :
		switch(ant_secc) {
		case END :
			error=_E_REND;  /* marca END repetida */
			break;
		default :
			if(!compiled[ant_secc]) {       /* si no compilada */
				/* marca compilada y desactiva error */
				compiled[ant_secc]=1;
				error=_E_NERR;
				/* ajustes finales tras compilar */
				switch(ant_secc) {
				case LOC :
					error=chequea_conx();
					break;
				}
			}
			/* marca de secci¢n no encontrada */
			else error=_E_MSCC;
			break;
		}
		break;
	default :
		error=_E_MSCC;  /* marca de secci¢n no encontrada */
		break;
	}
} while(!error);

if(error==_E_LECF) {
	if(feof(f_in)) {    /* fin de fichero */
		/* acaba de compilar proc. */
		if(ant_secc==PRO) error=chequea_pro();
		else printf(c_errores[0]);
	}
	else if(ferror(f_in)) {  /* error de lectura */
		printf("\n*** ERROR 1 ***\n%s\n",c_errores[1]);
		exit(1);
	}
}
if(error!=_E_NERR) {
	printf("\n*** ERROR %u ***\n%s\n",error,c_errores[error]);
	if(error==_E_CLOC) printf("Localidad: %i",err_conx);
	else if((error==_E_PRLL) || (error==_E_RFFW) || (error==_E_LFFR))
	  printf("L¡nea %lu",numlin);
	else printf("L¡nea %lu: %s",numlin,lin);
}
if(!error || (error==_E_LECF)) {
/* si no hubo error o es fin de fich. mira si hay alguna secc. sin compilar */
	for(i=0;i<(NS-1);i++) {
		if(!compiled[i]) {
		  printf("Secci¢n de %s sin compilar, rev¡sala\n",
		    nomb_seccs[i]);
		  /* marca que hay alguna secci¢n sin compilar */
		  sin_compilar+=1;
		}
	}
	/* comprueba si hay suficientes mensajes del sistema */
	if(compiled[_MSY] && (m_actsys<SYS_MESS))
	  printf("\n*** Aviso: faltan mensajes del sistema.\n");
	if(!sin_compilar) { /* si se compilaron todas sin error */
		if((f_sal=fopen(nombres,"wb"))==NULL) {
			printf("\nError de apertura de fichero de "
			  "salida [%s].\n",nombres);
			exit(1);
		}
		printf("\nEscribiendo en fichero de salida %s.\n",nombres);

		/* escribe marca de reconocimiento */
		if(fputs(SRECON,f_sal)) {
			printf(fsal_err,nombres);
			exit(1);
		}

		/* calcula y escribe datos de las diferentes secciones */
		t_msg=(unsigned)(act_msg-msgs);
		t_msy=(unsigned)(act_msgsys-msgsys);
		t_loc=(unsigned)(act_loc-localidades);
		t_con=(unsigned)(act_conx-conx);
		t_obj=(unsigned)(act_obj-obj);
		t_pro=(unsigned)(proc_act-proc);

		fwrt2((BYTE *)&v_mov,sizeof(BYTE),1);
		fwrt2((BYTE *)&n_conv,sizeof(BYTE),1);
		fwrt2((BYTE *)&n_prop,sizeof(BYTE),1);
		fwrt2((int *)&pvoc,sizeof(int),1);
		fwrt2((BYTE *)&m_act,sizeof(BYTE),1);
		fwrt2((unsigned *)&t_msg,sizeof(unsigned),1);
		fwrt2((BYTE *)&m_actsys,sizeof(BYTE),1);
		fwrt2((unsigned *)&t_msy,sizeof(unsigned),1);
		fwrt2((BYTE *)&n_loc,sizeof(BYTE),1);
		fwrt2((unsigned *)&t_loc,sizeof(unsigned),1);
		fwrt2((unsigned *)&t_con,sizeof(unsigned),1);
		fwrt2((BYTE *)&objact,sizeof(BYTE),1);
		fwrt2((unsigned *)&t_obj,sizeof(unsigned),1);
		fwrt2((BYTE *)&proact,sizeof(BYTE),1);
		fwrt2((unsigned *)&t_pro,sizeof(unsigned),1);

		fwrt2((struct palabra *)vocabulario,sizeof(struct palabra),
		  (size_t)pvoc);

		fwrt2((ptrdiff_t *)mens,sizeof(ptrdiff_t),(size_t)MAX_MSG);
		if(wrt_fich(msgs,t_msg,f_sal)) {
			printf(fsal_err,nombres);
			exit(1);
		}

		fwrt2((ptrdiff_t *)mensys,sizeof(ptrdiff_t),(size_t)MAX_MSY);
		if(wrt_fich(msgsys,t_msy,f_sal)) {
			printf(fsal_err,nombres);
			exit(1);
		}

		fwrt2((ptrdiff_t *)locs,sizeof(ptrdiff_t),(size_t)MAX_LOC);
		if(wrt_fich(localidades,t_loc,f_sal)) {
			printf(fsal_err,nombres);
			exit(1);
		}

		fwrt2((ptrdiff_t *)conxs,sizeof(ptrdiff_t),(size_t)MAX_LOC);
		if(wrt_fich(conx,t_con,f_sal)) {
			printf(fsal_err,nombres);
			exit(1);
		}

		fwrt2((ptrdiff_t *)objs,sizeof(ptrdiff_t),(size_t)MAX_OBJ);
		if(wrt_fich(obj,t_obj,f_sal)) {
			printf(fsal_err,nombres);
			exit(1);
		}

		fwrt2((ptrdiff_t *)pro,sizeof(ptrdiff_t),(size_t)MAX_PRO);
		if(wrt_fich(proc,t_pro,f_sal)) {
			printf(fsal_err,nombres);
			exit(1);
		}

		printf("OK.\n");
		estadisticas();
		exit(0);    /* fin de compilaci¢n */
	}
}                            
/* aqu¡ llega si hubo alg£n error durante la compilaci¢n */
exit(1);
}


/****************************************/
/*************** Funciones **************/
/****************************************/

/*****************************************************************************
	AJUSTA_BLOCK: ajusta el tama¤o del bloque de memoria que se
	  reservar  para las distintas secciones de acuerdo a los par me-
	  tros dados en la l¡nea de entrada.
	  Entrada: tam     tama¤o del bloque (mirar def. de variable 'block')
		   *params puntero a par metros de entrada '/x'
*****************************************************************************/
size_t ajusta_block(size_t tam,char *params)
{
switch(*(params+1)) {
case 'r' :
case 'R' :
	tam/=2;         /* deja tama¤o en la mitad */
	break;
case 's' :
case 'S' :
	tam/=4;         /* deja el tama¤o en la cuarta parte */
	break;
default :
	printf("Par metro %s, no v lido; se ignora.\n",params);
}
return(tam);
}

/*****************************************************************************
	FWRT2: controla salida de datos hacia el fichero de salida, usando
	  la funci¢n fwrite.
*****************************************************************************/
void fwrt2(const void *buff,size_t tam,size_t cant)
{

if(fwrite(buff,tam,cant,f_sal)!=cant) {
	printf(fsal_err,nombres);
	exit(1);
}

}

/*****************************************************************************
	WRT_FICH: escribe en el fichero de salida fp, un bloque de BYTES
	  almacenados en un buffer de memoria far.
	  Devuelve 1 si se encontr¢ error o 0 si no.
*****************************************************************************/
int wrt_fich(char far *buff,unsigned tam,FILE *fp)
{
BYTE cod;

while(tam) {
	cod=(BYTE)0xff-(BYTE)*buff;     /* BYTE a escribir, codificado */
	if(fputc((int)cod,fp)==EOF) {   /* escribe BYTE */
		if(ferror(fp)) return(1);   /* si error devuelve 1 */
	}
	buff++; /* siguiente BYTE */
	tam--;
}
return(0);  /* indica que no hubo error */
}

/*****************************************************************************
	ESTADISTICAS: imprime diversa informaci¢n sobre la base de datos
	  compilada.
*****************************************************************************/
void estadisticas(void)
{
static char *formato="%-20s : %5i %5u\n";
unsigned t_voc, t_msg, t_msy, t_loc, t_con, t_obj, t_pro;
unsigned long t_tot;

t_voc=pvoc*sizeof(struct palabra);
t_msg=(unsigned)(act_msg-msgs);
t_msy=(unsigned)(act_msgsys-msgsys);
t_loc=(unsigned)(act_loc-localidades);
t_con=(unsigned)(act_conx-conx);
t_obj=(unsigned)(act_obj-obj);
t_pro=(unsigned)(proc_act-proc);
t_tot=(unsigned long)(t_voc+t_msg+t_msy+t_loc+t_con+t_obj+t_pro);

printf("\nSECCION                 NUM. BYTES\n");
printf(formato,nomb_seccs[_VOC],pvoc,t_voc);
printf(formato,nomb_seccs[_MSG],(int)m_act,t_msg);
printf(formato,nomb_seccs[_MSY],(int)m_actsys,t_msy);
printf(formato,nomb_seccs[_LOC],(int)n_loc,t_loc);
printf(formato,"Conexiones",(int)n_loc,t_con);
printf(formato,nomb_seccs[_OBJ],(int)objact,t_obj);
printf(formato,nomb_seccs[_PRO],(int)proact,t_pro);
printf("----------------------------------\n");
printf("TOTAL:%28lu\n",t_tot);
printf("----------------------------------\n");

}

/*****************************************************************************
	SI_O_NO: imprime una pregunta y devuelve 1 si se contest¢ afirmativa-
	  mente o 0 si no.
*****************************************************************************/
int si_o_no(char *preg)
{
printf("%s",preg);
if((mayuscula((char)getche()))=='N') return(0);
putchar('\n');
return(1);
}

/*****************************************************************************
	SALTA_ESPACIOS: salta los espacios y tabulaciones iniciales de una linea
*****************************************************************************/
char *salta_espacios(char *s)
{
while((*s==' ') || (*s=='\t')) s++;
return(s);
}

/*****************************************************************************
	TIPO_LIN: devuelve 0 si la linea de entrada es un comentario o
	  est  en blanco.
*****************************************************************************/
int tipo_lin(char *l0)
{
if((*l0=='\n') || (*l0==';')) return(0);
return(1);
}

/*****************************************************************************
	LEE_LINEA: lee lineas del fichero de entrada hasta encontrar una que
	  no sea ni comentario, ni est‚ en blanco.
	  Devuelve 1 si hubo error de lectura o fin de fichero.
	  Usa la variable global *l, que es un puntero al buffer de entrada,
	  lin[]
*****************************************************************************/
int lee_linea(void)
{
do {
	l=lin;
	if(fgets(l,L_MAX,f_in)==NULL) return(1);
	++numlin;   /* incrementa contador de lineas le¡das */
	if(comprueba_linea(l)) {
		warning("l¡nea rebosa el buffer de entrada.");
		while(fgetc(f_in)!='\n');   /* desprecia cars. finales */
	}
	l=salta_espacios(l);    /* salta blancos iniciales */
} while(!tipo_lin(l));
return(0);  /* no ha habido error */
}

/*****************************************************************************
	MAYUSCULA: convierte una letra en may£scula
*****************************************************************************/
char mayuscula(char c)
{
if((c>='a') && (c<='z')) return(c-'a'+'A');
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

/*****************************************************************************
	ESTA_EN: comprueba si un car cter est  en una cadena.
	  Devuelve 0 si el car cter no est  en la cadena.
*****************************************************************************/
int esta_en(const char *l0, char c)
{
while(*l0) {
	if(*l0++==c) return(1);
}
return(0);
}

/*****************************************************************************
	HASTA_ESPACIO: salta los caracteres de una linea, hasta encontar un
	  espacio o el fin de la linea.
*****************************************************************************/
char *hasta_espacio(char *s)
{
while((*s!=' ') && (*s!='\t') && (*s!='\n') && (*s!='\0')) s++;
return(s);
}

/*****************************************************************************
	FIN_LINEA: comprueba si un car cter es indicador de fin de linea.
	  Devuelve 0 si no lo es.
*****************************************************************************/
int fin_linea(char *s)
{
if((*s=='\n') || (*s=='\0')) return(1);
return(0);
}

/*****************************************************************************
	COMP_VOC: compila la secci¢n de Vocabulario.
	  Devuelve un valor distinto de 0 si hubo error.
*****************************************************************************/
int comp_voc(void)
{
struct palabra p;
int palerr;

if(*(l-4)=='\\') return(_E_NERR); /* retorna si ley¢ marca secci¢n */

/* CAMPO PALABRA */

if(palerr=coge_pal(p.p)) return(palerr);

/* CAMPO VALOR DE PALABRA */

l=hasta_espacio(l); /* desprecia cars. hasta espacio o fin de linea */
if(fin_linea(l)) return(_E_FCAM); /* error, faltan campos */
l=salta_espacios(l);    /* desprecia espacios */
if(coge_num(&p.num,l)) return(_E_CNUM);  /* si error, sale */
if((p.num<1) || (p.num>254)) return(_E_NPAL); /* valor de pal. no v lido */

/* CAMPO TIPO DE PALABRA */

l=hasta_espacio(l); /* desprecia cars. hasta espacio o fin de linea */
if(fin_linea(l)) return(_E_FCAM); /* error, faltan campos */
l=salta_espacios(l);    /* desprecia espacios */
switch(mayuscula(*l)) {
case 'V' :
	p.tipo=_VERB;
	break;
case 'N' :
	p.tipo=_NOMB;
	break;
case 'A' :
	p.tipo=_ADJT;
	break;
case 'C' :
	p.tipo=_CONJ;
	break;
default :
	return(_E_TPAL);
	break;
}

if(palerr=mete_palabra(&p)) return(palerr);   /* intenta meter palabra */

return(_E_NERR);
}

/*****************************************************************************
	COGE_PAL: recoge una palabra de vocabulario de la linea de entrada l
	  y la mete en *pal (buffer de al menos LONGPAL caracteres).
	  Devuelve:
		_E_FCAM si encontr¢ un car. fin de linea (no hay m s campos
		  detr s de la palabra).
		_E_CVOC si encontr¢ car cter no v lido.
		0       si no hubo error.
*****************************************************************************/
int coge_pal(char *pal)
{
int i;
for(i=0;i<LONGPAL-1;i++) {
	if(fin_linea(l)) {      /* no hay m s campos detr s */
		for(;i<LONGPAL-1;i++) *(pal+i)=' '; /* rellena con espacios */
		*(pal+i)='\0';
		return(_E_FCAM);
	}
	if((*l==' ') || (*l=='\t')) {   /* si encuentra espacio a tab. */
		for(;i<LONGPAL-1;i++) *(pal+i)=' '; /* rellena con espacios */
		break;
	}
	else {
		*(pal+i)=mayuscula(*l);
		l++;
		/* comprueba si es car cter v lido de palabra */
		if(!esta_en(ABECEDARIO,*(pal+i)) &&
		  !esta_en(NUMEROS,*(pal+i))) return(_E_CVOC);
	}
}
*(pal+i)='\0';
return(_E_NERR);
}

/*****************************************************************************
	COGE_NUM: recoge un campo num‚rico de la linea apuntada por l y
	  lo devuelve en formato BYTE en n.
	  Devuelve: 0 si no hubo error
		    1 si lo hubo
*****************************************************************************/
int coge_num(BYTE *n, char *l0)
{
char num[4];
int i;

i=0;
while(esta_en(NUMEROS,*l0) && (i<3)) num[i++]=*l0++;
/* si 1er car. no es n£mero, o el ult. no es blanco ni fin linea, error */
if((!i) || (!fin_linea(l0) && (*l0!=' ') && (*l0!='\t'))) return(1);
for(;i<3;i++) num[i]=' ';   /* rellena con espacios */
num[i]='\0';    /* marca fin de cadena */
i=atoi(num);
if((i<0) || (i>255)) return(1); /* n£mero fuera de rango */
*n=(BYTE)i;
return(0);  /* vuelve sin error */
}

/*****************************************************************************
	METE_PALABRA: introduce una palabra en el vocabulario.
	  Devuelve:
		_E_PREP: si la palabra est  repetida
		_E_MVOC: si no caben m s palabras en vocabulario.
		0        si no hubo error
*****************************************************************************/
int mete_palabra(struct palabra *p)
{
int i;
if(pvoc>(NUM_PAL-1)) return(_E_MVOC);   /* vocabulario lleno */
if(!pvoc) {     /* si vocabulario est  vac¡o mete palabra sin m s */
	for(i=0;i<LONGPAL;i++) vocabulario[pvoc].p[i]=p->p[i];
	vocabulario[pvoc].num=p->num;
	vocabulario[pvoc].tipo=p->tipo;
}
else {
	i=0;
	do {
		if(!compara_pal(p,&vocabulario[i])) return(_E_PREP);
	} while(++i<pvoc);
	/* si no encontr¢ una igual, la mete */
	for(i=0;i<LONGPAL;i++) vocabulario[pvoc].p[i]=p->p[i];
	vocabulario[pvoc].num=p->num;
	vocabulario[pvoc].tipo=p->tipo;
}
pvoc++;
return(_E_NERR);  /* indica que la palabra no estaba */
}

/*****************************************************************************
	COMPARA_PAL: compara dos palabras de vocabulario y devuelve 1 si son
	  distintas o 0 si son iguales.
*****************************************************************************/
int compara_pal(struct palabra *p1, struct palabra *p2)
{
if(strcmp(p1->p,p2->p)) return(1);
else return(0);
}

/*****************************************************************************
	COMP_TXT: compila texto de secciones de Mensajes, Mensajes del
	  Sistema, Localidades...
	  Devuelve un valor distinto de 0 si hubo error.
*****************************************************************************/
int comp_txt(char *l0,ptrdiff_t *mens,char far *(*tmsg), char far *(*actm),
  char far *(*ultm),BYTE *act,BYTE maximo)
{
BYTE n;
int result;
int elin;

if(*(l0-4)=='\\') return(_E_NERR); /* retorna si ley¢ marca secci¢n */

if(*l0!='@') return(_E_FALT);
l0++;
if(coge_num(&n,l0)) return(_E_CNUM);
if(n!=*act) return(_E_NFSC);    /* comprueba si est  dentro de la secuencia */
if(n>maximo) return(_E_NVAL);
l0=hasta_espacio(l0);
l0++;    /* salta espacio */

*(mens+(*act))=(*actm)-(*tmsg); /* mete desplz. relativo de mensaje */
do {
	result=recoge_texto(actm,*ultm,l0);
	if(result) break; /* si ley¢ fin de mensaje o error fuera memoria */
	/* error de lectura o fin de fichero */
	if(elin=lee_linea()) return(elin);
	l0=lin;
	if(*l0=='\\') return(_E_NFTX);  /* no se encontr¢ fin de texto */
} while(!result);
if(result==2) return(_E_FMEM);
(*act)++;   /* incrementa para siguiente mensaje */

return(_E_NERR);
}

/*****************************************************************************
	RECOGE_TEXTO: coge el texto de la linea de entrada.
	  Devuelve:
		0: no error
		1: fin de texto encontrado (@)
		2: fuera de memoria
*****************************************************************************/
int recoge_texto(char far *(*act), char far *ult, char *l0)
{
while(*l0) {
	if(*act==ult) return(2);    /* fuera de memoria */
	if(*l0=='@') {
		*(*act)++='\0';
		return(1);  /* fin de texto */
	}
	if(*l0=='|') *(*act)++='\n'; /* avance de linea */
	else if(*l0=='\t') {
		*(*act)++=' ';   /* tabulaci¢n */
		warning("tabulaci¢n transformada en espacio");
	}
	else if(*l0!='\n') *(*act)++=*l0;
	l0++;
}
return(0);
}

/*****************************************************************************
	COMPRUEBA_LINEA: comprueba si la linea es m s larga que el buffer de
	  entrada. Si lo es devuelve 0. En otro caso devuelve 1.
*****************************************************************************/
int comprueba_linea(char *l0)
{
while(*l0) {
	/* si encuentra \n la linea cupo en buffer */
	if(*l0++=='\n') return(0);
}
return(1);
}

/*****************************************************************************
	COMP_LOC: compila la secci¢n de Localidades.
*****************************************************************************/
int comp_loc(void)
{
int errct, palfound;
BYTE nloc, num, tipo;
fpos_t *pos;
static char con[LONGPAL];
unsigned long numlin0;

if(!compiled[_VOC]) return(_E_SVOC);
if(*(l-4)=='\\') return(_E_NERR); /* retorna si ley¢ marca secci¢n */

if(errct=comp_txt(l,locs,&localidades,&act_loc,&ult_loc,&n_loc,
  (BYTE)(MAX_LOC-1))) return(errct); /* compila texto localidad */

conxs[n_loc-1]=act_conx-conx;   /* guarda desplazamiento */
do {
	numlin0=numlin; /* guarda n£mero de l¡nea actual */
	/* almacena posici¢n en fichero */
	if(fgetpos(f_in,pos)) return(_E_FENT);
	if(errct=lee_linea()) return(errct);
	if(*l!='#') {   /* si no es marca de conexiones */
		/* restaura pos. en fichero */
		if(fsetpos(f_in,pos)) return(_E_FENT);
		numlin=numlin0; /* restaura n£mero de l¡nea */
		/* indica fin de conexiones para localidad */
		*act_conx++=(BYTE)0;
		if(act_conx>=ult_conx) return(_E_NMCN);
		return(_E_NERR);
	}
	else {
		l=hasta_espacio(l);
		l=salta_espacios(l);
		/* coge el campo palabra */
		if(errct=coge_pal(con)) return(errct);
		if((palfound=esta_en_voc(con))==(NUM_PAL+1)) return(_E_NPVC);
		num=vocabulario[palfound].num;
		tipo=vocabulario[palfound].tipo;
		if(((tipo!=_VERB) && (tipo!=_NOMB)) || (num>=v_mov))
		  return(_E_NMOV);
		/* desprecia cars. hasta 1er espacio o tab. */
		l=hasta_espacio(l);
		l=salta_espacios(l);
		/* error campo num‚rico */
		if(coge_num(&nloc,l)) return(_E_CNUM);
		/* n£m. loc. no v lido */
		if(nloc>(MAX_LOC-1)) return(_E_NVAL);
		*act_conx++=num;    /* guarda verbo de movimiento */
		if(act_conx>=ult_conx) return(_E_NMCN);
		*act_conx++=nloc;   /* y localidad a la que lleva */
		if(act_conx>=ult_conx) return(_E_NMCN);
	}
} while(1);
}

/*****************************************************************************
	ESTA_EN_VOC: comprueba si una palabra est  en el vocabulario.
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
	CHEQUEA_CONX: chuequea la validez de la tabla de conexiones buscando
	  alguna conexi¢n a una localidad no v lida.
	  Devuelve _E_NERR si no hay errores o _E_CLOC si los hubo.
	  Adem s deja en la variable global err_conx el n£mero de localidad
	  en la que se detect¢ el error.
*****************************************************************************/
int chequea_conx(void)
{
char far *pc;
int i;

for(i=0;i<(int)n_loc;i++) {
	pc=conx+conxs[i];
	while(*pc) {
		pc++;   /* salta el verbo de movimiento */
		if((*pc++>=n_loc)) {
			err_conx=i; /* almacena n£mero de localidad */
			return(_E_CLOC); /* comprueba num. de localidad */
		}
	}
}
return(_E_NERR);
}

/*****************************************************************************
	COMP_OBJ: compila la secci¢n de Objetos.
*****************************************************************************/
int comp_obj(void)
{
BYTE n, tipo, nombre, adjetivo, loc_ini, flags1, flag, flags21, flags22;
unsigned flags2, flag2;
int palfound, i, errct;
char c;
static char palabra[LONGPAL];

if(!compiled[_VOC]) return(_E_SVOC);    /* si no se compil¢ vocabulario */
if(!compiled[_LOC]) return(_E_SLOC);    /* si no se compil¢ localidades */
if(*(l-4)=='\\') return(_E_NERR); /* retorna si ley¢ marca secci¢n */

if(*l!='@') return(_E_FALT);
l++;
if(coge_num(&n,l)) return(_E_CNUM);
if(n!=objact) return(_E_NFSC);   /* comprueba si est  dentro de secuencia */
if(n==255) return(_E_NONV); /* 255 no es num. de obj. v lido */

l=hasta_espacio(l);
l=salta_espacios(l);
if(errct=coge_pal(palabra)) return(errct); /* coge el campo palabra */
if((palfound=esta_en_voc(palabra))==(NUM_PAL+1)) return(_E_NPVC);
tipo=vocabulario[palfound].tipo;
if(tipo!=_NOMB) return(_E_NNOM);    /* palabra no es un nombre */
nombre=vocabulario[palfound].num;   /* guarda n£m. de nombre */
l=hasta_espacio(l);
l=salta_espacios(l);
if(*l!='_') {   /* si no es signo de cualquier palabra */
	if(errct=coge_pal(palabra)) return(errct); /* coge el campo palabra */
	if((palfound=esta_en_voc(palabra))==(NUM_PAL+1)) return(_E_NPVC);
	tipo=vocabulario[palfound].tipo;
	if(tipo!=_ADJT) return(_E_NADJ);    /* palabra no es adjetivo */
	adjetivo=vocabulario[palfound].num;   /* guarda n£m. de adjetivo */
}
else {
	adjetivo=NO_PAL;  /* es cualquier palabra */
	l++;    /* salta a siguiente posici¢n */
}

l=hasta_espacio(l);
l=salta_espacios(l);
if(coge_num(&loc_ini,l)) return(_E_CNUM);
if(((loc_ini<NO_CREADO) && (loc_ini>=n_loc)) || loc_ini==LOC_ACTUAL)
  return(_E_LNVL);

flags1=0;
do {
	l=hasta_espacio(l);
	l=salta_espacios(l);
	flag=es_flag1(*l);
	flags1 |= flag; /* activa flag correspondiente */
	if(flag) l++;   /* salta car cter si fue flag v lido*/
} while(flag);

flags2=0;
flag2=0x8000;    /* bit de mayor peso a 1 */
for(i=0;i<16;i++) {
	c=mayuscula(*l);
	if((c!='O') && (c!='X')) return(_E_FLAG);
	if(c=='X') flags2 |= flag2;
	flag2 >>= 1; /* desplaza hacia la derecha */
	l++;
}
flags22=(BYTE)(flags2 & 0x00ff);    /* separa la parte baja */
flags21=(BYTE)(flags2 >> 8);        /* de la parte alta */

/* guardamos los datos recogidos */
objs[objact++]=act_obj-obj; /* desplazamiento */
*act_obj++=nombre;      /* NOMBRE */
if(act_obj==ult_obj) return(_E_FMEM);    /* comprueba si se sale de memoria */
*act_obj++=adjetivo;    /* ADJETIVO */
if(act_obj==ult_obj) return(_E_FMEM);    /* comprueba si se sale de memoria */
*act_obj++=loc_ini;     /* LOCALIDAD INICIAL */
if(act_obj==ult_obj) return(_E_FMEM);    /* comprueba si se sale de memoria */
*act_obj++=flags1;      /* FLAGS 1 */
if(act_obj==ult_obj) return(_E_FMEM);    /* comprueba si se sale de memoria */
*act_obj++=flags21;     /* FLAGS 2 */
if(act_obj==ult_obj) return(_E_FMEM);    /* comprueba si se sale de memoria */
*act_obj++=flags22;
if(act_obj==ult_obj) return(_E_FMEM);    /* comprueba si se sale de memoria */

if(errct=lee_linea()) return(errct);
if((*l=='@') || (*l=='\\')) return(_E_OTEX); /* falta texto de objeto */

while(*l) { /* compila el texto del objeto */
	if(act_obj==ult_obj) return(_E_FMEM);    /* fuera de memoria */
	if(*l=='\n') {
		*act_obj++='\0';
		break;  /* fin de texto */
	}
	else if(*l=='\t') {
		warning("tabulaci¢n transformada en espacio");
		*act_obj++=' ';         /* transforma tab. en espacio */
		l++;    /* salta caracter */
	}
	else *act_obj++=*l++;
}
return(0);
}

/*****************************************************************************
	ES_FLAG1: comprueba si el car cter dado es correspondiente a un flag
	  de objeto. Da lo mismo si es may£scula o min£scula.
	  Devuelve:
		car cter P : 1  (prenda)
		car cter L : 2  (fuente de luz)
		otros      : 0  (no v lido)
*****************************************************************************/
BYTE es_flag1(char c)
{
switch(mayuscula(c)) {
case 'P' :
	return(1);
	break;
case 'L' :
	return(2);
	break;
default :
	return(0);
	break;
}
}

/*****************************************************************************
	COMP_PRO: compila la secci¢n de Procesos.
*****************************************************************************/
int comp_pro(void)
{
BYTE p;
char s[4], pal[LONGPAL], cond[8], etq[LONG_LAB];
int i, errct;
int pra_lin_entr;       /* indicador de que est  en 1era. linea de entrada */
BYTE far *sgte_entr;    /* puntero d¢nde guardar desplz. sgte. entrada */
ptrdiff_t se;           /* variable auxiliar para desplz. sgte. entr. */

if(!compiled[_VOC]) return(_E_SVOC);    /* si no se compil¢ vocabulario */
if(!compiled[_LOC]) return(_E_SLOC);    /* si no se compil¢ localidades */
if(!compiled[_OBJ]) return(_E_SOBJ);    /* si no se compil¢ objetos */
if(!compiled[_MSG]) return(_E_SMSG);    /* si no se compil¢ mensajes */
if(!compiled[_MSY]) return(_E_SMSY);    /* si no se compil¢ mens. sist. */

/* espera que despu‚s del \PRO est‚ num. de proceso separado con espacios */
l=hasta_espacio(l);
l=salta_espacios(l);
if(coge_num(&p,l)) return(_E_CNUM);     /* error en campo num‚rico */
if(p!=proact) return(_E_NFSC);          /* num. de proceso fuera de sec. */

/* las etiquetas son locales, es decir, s¢lo valen dentro del proceso */
/* en el  que fueron definidas */
ptlab=0;    /* inicializa puntero a tabla etiquetas */
ptskip=0;   /*     "         "    " tabla referencias forward */
pro[proact]=proc_act-proc;  /* guarda desplaz. de proceso */
if(errct=lee_linea()) {     /* lee linea de entrada */
	if(feof(f_in)) return(_E_MEND);     /* END final no encontrado */
	return(errct);
}
do {
	if(*l=='\\') {  /* si encuentra marca de secci¢n */
		l++;
		for(i=0;i<3;i++) s[i]=*l++;     /* recoge marca de secci¢n */
		s[i]='\0';
		/* END no encontrado */
		if(strcmp(s,n_secc[_END])) return(_E_MEND);
		else {                  /* si encontr¢ fin de proceso */
			proact++;           /* siguiente n£mero de proceso */
			*proc_act++=0;      /* pone marca de fin de proceso */
			/* fuera de memoria */
			if(proc_act==proc_ult) return(_E_FMEM);
			*proc_act++=0;
			/* fuera de memoria */
			if(proc_act==proc_ult) return(_E_FMEM);
			/* marca de compilado al menos 1 proc. */
			compiled[_PRO]=1;
			return(_E_NERR);  /* sin error-->proc. xx compilado */
		}
	}
	/* el 1er car. de l¡nea no debe ser un blanco */
	/* ya que si no, la entrada no es v lida */
	if((lin[0]==' ') || (lin[0]=='\t')) return(_E_NENT);
	if(lin[0]=='$') {   /* si es una etiqueta */
		if(ptlab>=LABELS) return(_E_RBTL); /* tabla etiquetas llena */
		l++;
		for(i=0;i<LONG_LAB-1;i++) {   /* recoge etiqueta */
			if(fin_linea(l) || (*l==' ') || (*l=='\t')) break;
			etq[i]=mayuscula(*l);
			l++;
		}
		for(;i<LONG_LAB-1;i++) etq[i]=' ';
		etq[i]='\0';
		if(!ptlab) {    /* si tabla etiquetas vac¡a */
			for(i=0;i<LONG_LAB;i++) etiqueta[ptlab].label[i]=etq[i];
			/* direcci¢n de etiqueta */
			etiqueta[ptlab].plab=proc_act;
		}
		else {  /* si no mira si la etiqueta ya est  definida */
			for(i=0;i<ptlab;i++) if(!strcmp(etq,etiqueta[i].label))
			  return(_E_LREP);  /* error si etiqueta ya definida */
			for(i=0;i<LONG_LAB;i++) etiqueta[ptlab].label[i]=etq[i];
			etiqueta[ptlab].plab=proc_act; /* direcci¢n de etiqueta */
		}
		ptlab++;    /* sgte. entrada en la tabla */
		if(errct=lee_linea()) {     /* lee linea de entrada */
			/* END final no encontrado */
			if(feof(f_in)) return(_E_MEND);
			return(errct);
		}
	}
	/* espera encontrar un verbo-nomb. conv. y un nombre */
	errct=coge_pal(pal);    /* recoge campo verbo */
	if(*pal=='_') {     /* si es no-palabra */
		*proc_act++=NO_PAL;
	}
	else {
		/* si hubo error al coger pal., sale */
		if(errct) return(errct);
		if((i=esta_en_voc(pal))==NUM_PAL+1) return(_E_NPVC); /* no en voc. */
		/* si no es verbo ni nombre convertible */
		if((vocabulario[i].tipo!=_VERB) &&
		  ((vocabulario[i].tipo==_NOMB) &&
		  (vocabulario[i].num>=n_conv))) return(_E_NVNC);
		/* guarda num. de verbo-nomb.conv. */
		*proc_act++=vocabulario[i].num;
	}
	if(proc_act==proc_ult) return(_E_FMEM); /* comprueba fuera de mem. */
	l=hasta_espacio(l);
	l=salta_espacios(l);
	errct=coge_pal(pal);    /* recoge campo palabra */
	if(*pal=='_') {     /* si es no-palabra */
		*proc_act++=NO_PAL;
	}
	else {
		if(errct) return(errct); /* sale si hubo palabra no v lida */
		/* guarda num. de verbo-nomb.conv. */
		if((i=esta_en_voc(pal))==NUM_PAL+1) return(_E_NPVC);
		/* no es nombre */
		if(vocabulario[i].tipo!=_NOMB) return(_E_NNNN);
		*proc_act++=vocabulario[i].num; /* guarda num. de nombre */
	}
	if(proc_act==proc_ult) return(_E_FMEM); /* comprueba fuera de mem. */
	sgte_entr=proc_act; /* guarda ptro. d¢nde meter desplz. sgte. entr. */
	proc_act+=2;        /* salta 2 bytes para empezar compilar entrada */
	if(proc_act>=proc_ult) return(_E_FMEM); /* comprueba fuera de mem. */
	/* espera encontrar un condacto */
	pra_lin_entr=1; /* estamos en primera linea de entrada */
	do {
		if(pra_lin_entr) {
			/* necesario por si es 1era lin. entrada */
			l=hasta_espacio(l);
			/* deja ptro. al inicio de condacto */
			l=salta_espacios(l);
			pra_lin_entr=0;
		}
		else {  /* si no es 1era lin. de entrada */
			if(errct=lee_linea()) {     /* lee linea de entrada */
				/* END final no encontrado */
				if(feof(f_in)) return(_E_MEND);
				return(errct);
			}
			/* si 1er car. no es espacio o encuentra */
			/* marca de secci¢n */
			if(((lin[0]!=' ') && (lin[0]!='\t')) || (*l=='\\')) {
				*proc_act++=0;  /* marca fin de entrada */
				/* fuera de mem. */
				if(proc_act==proc_ult) return(_E_FMEM);
				/* guarda desplz. sgte. entrada respecto */
				/* a inicio proc. */
				se=(proc_act-proc)-pro[proact];
				/* byte bajo */
				*sgte_entr=(BYTE)(se & 0x00ff);
				/* byte alto */
				*(sgte_entr+1)=(BYTE)((se >> 8) & 0x00ff);
				break;    /* fin de entrada */
			}
		}
		i=0;
		while((i<7) && (esta_en(ABECEDARIO,mayuscula(*l)) ||
		  esta_en(NUMEROS,mayuscula(*l)))) {
			cond[i]=mayuscula(*l);  /* recoge condacto */
			l++;
			i++;
		}
		if((*l!=' ') && (*l!='\t') && (*l!='\n')) return(_E_NCND);
		for(;i<7;i++) cond[i]=' ';  /* rellena con espacios */
		cond[i]='\0';   /* a¤ade fin de cadena */
		for(i=0;i<N_CONDACTOS+1;i++) {  /* busca condacto */
			/* si lo encuentra sale */
			if(!strcmp(cond,condactos[i])) break;
		}
		/* retorna con error si condacto no encontrado o no v lido */
		if(i==N_CONDACTOS+1) return(_E_NCND);
		/* si no, compila comprobando par metros, indirecci¢n,... */
		if(errct=cd[i]()) return(errct);    /* compila condacto */
		l=hasta_espacio(l);
		l=salta_espacios(l);
		if(!fin_linea(l) && *l!=';') {
			putchar('\n');
			warning("demasiados campos, se ignoran los sobrantes");
			c=coge_cur();     /* guarda posici¢n cursor */
		}
	} while(1);
} while(1);
}

/*****************************************************************************
	WARNING: imprime un mensaje de aviso.
	  Entrada: w mensaje a imprimir
*****************************************************************************/
void warning(char *w)
{
printf("*** Aviso: %s. L¡nea %lu.\n",w,numlin);
}

/*****************************************************************************
	CHEQUEA_PRO: comprueba si hay alguna llamada a un proceso inexistente.
	  Devuelve: _E_NERR si no hubo errores
		    _E_PRLL si llam¢ a un proceso inexistente (deja en la
		      variable numlin el num. de l¡nea del error)
*****************************************************************************/
int chequea_pro(void)
{
int i;

for(i=0;i<pprc;i++) {
	if(prc[i].numpro>=proact) {
		numlin=prc[i].nl;
		return(_E_PRLL);
	}
}
return(_E_NERR);
}

/*****************************************************************************
	CHECK_MEM: comprueba si hay memoria suficiente para compilar
	  un condacto.
	  Devuelve: 1 si no hay memoria suficiente
		    0 si la hay
*****************************************************************************/
int check_mem(ptrdiff_t tam)
{
if(proc_act+tam>=proc_ult) return(1);   /* si no hay mem. suficiente */
return(0);
}

/*****************************************************************************
	COGE_NUM2: recoge un campo num‚rico de la linea apuntada por l y
	  lo devuelve en formato BYTE en n.
	  Devuelve: 0 si no hubo error
				1 si indirecci¢n (num. entre [])
				2 si hubo error
*****************************************************************************/
int coge_num2(BYTE *n, char *l0)
{
char num[4];
int i, indir;
indir=0;
i=0;

if(*l0=='[') {
	indir=1;    /* indica indirecci¢n */
	l0++;
}
while(esta_en(NUMEROS,*l0) && (i<3)) num[i++]=*l0++;
/* si 1er. car cter no es n£mero */
if(!i) return(2);
if(indir) {
	if (*l0!=']') return(2); /* si indirecci¢n y £ltimo car. no es ']' */
}
/* si no indirecci¢n y el £ltimo car. no es blanco ni fin linea, error */
else if(!fin_linea(l0) && (*l0!=' ') && (*l0!='\t')) return(2);
for(;i<3;i++) num[i]=' ';   /* rellena con espacios */
num[i]='\0';    /* marca fin de cadena */
i=atoi(num);
if((i<0) || (i>255)) return(2); /* n£mero fuera de rango */
*n=(BYTE)i;
return(indir);  /* vuelve sin error */
}

/*****************************************************************************
	SGTE_CAMPO: desplaza puntero de l¡nea al siguiente campo.
	  Devuelve: 1 si no hay m s campos
		    0 si hay m s campos
*****************************************************************************/
int sgte_campo(void)
{
l=hasta_espacio(l);
l=salta_espacios(l);
if(fin_linea(l)) return(1);    /* si no hay m s campos */
return(0);
}

/*****************************************************************************
	METE_FSK: introduce etiqueta y dir. de sustituci¢n para una llamada
	  forward con SKIP.
	  Devuelve: 1 si la tabla de sustituci¢n de SKIP est  llena
		    0 si no hubo error
*****************************************************************************/
int mete_fsk(char *etq,BYTE far *dir)
{
int i;

if(ptskip>=FSKIP) return(1);    /* rebosamiento tabla */
for(i=0;i<LONG_LAB;i++) fskip[ptskip].label[i]=etq[i];  /* etiqueta */
fskip[ptskip].fsk=dir;      /* direcci¢n */
fskip[ptskip].nl=numlin;    /* num. de l¡nea de archivo de entrada */
ptskip++;
return(0);      /* sin error */
}

/*****************************************************************************
	SUST_LABELS: sutituye referencias forward en saltos SKIP.
	  Devuelve: _E_NERR si no hubo error
		    _E_RFFW si etiqueta no definida y 'numlin' n£mero
		      de l¡nea del error
		    _E_LFFR si salto fuera de rango
*****************************************************************************/
int sust_labels(void)
{
int i, j;
unsigned desplu;

for(i=0;i<ptskip;i++) {
	for(j=0;j<ptlab;j++) if(!strcmp(fskip[i].label,etiqueta[j].label))
	  break;
	if(j==ptlab) {  /* si no encontr¢ la etiqueta */
		numlin=fskip[i].nl; /* num. de lin. d¢nde est  error */
		return(_E_RFFW);
	}
	/* si la etiqueta est  definida */
	/* desplazamiento */
	desplu=(unsigned)(etiqueta[j].plab-(fskip[i].fsk-1));
	if(desplu>(unsigned)32767) return(_E_LFFR);
	*(fskip[i].fsk)=(BYTE)(desplu & 0x00ff);            /* byte bajo */
	*(fskip[i].fsk+1)=(BYTE)((desplu >> 8) & 0x00ff);   /* byte alto */
}
return(_E_NERR);
}

/*****************************************************************************
	COGE_CUR: devuelve en una estructura las coordenadas del cursor.
*****************************************************************************/
struct coords coge_cur(void)
{
struct coords cr;
union REGS reg;

reg.h.ah=0x03;      /* funci¢n buscar posici¢n cursor */
reg.h.bh=0;         /* p gina 0 */
int86(0x10,&reg,&reg);
cr.fila=reg.h.dh;
cr.columna=reg.h.dl;
return(cr);
}


/*****************************************************************************
	PON_CUR: coloca el cursor en unas coordenadas definidas.
*****************************************************************************/
void pon_cur(struct coords c)
{
union REGS reg;

reg.h.ah=0x02;  /* funci¢n posicionar cursor */
reg.h.bh=0;     /* p gina 0 */
reg.h.dh=c.fila;
reg.h.dl=c.columna;
int86(0x10,&reg,&reg);
}

/*****************************************************************************
	Funciones para compilar cada uno de los condactos.
*****************************************************************************/
int process(void)
{
BYTE par1;
int indir;

if(check_mem(3)) return(_E_FMEM);
if(sgte_campo()) return(_E_FCAM);
if((indir=coge_num2(&par1,l))==2) return(_E_CNUM);
if(!indir && (par1==proact)) return(_E_PRPR);
if(indir) *proc_act++=INDIR1;
*proc_act++=1;
*proc_act++=par1;

if(!indir && (par1>proact)) { /* si se llama a un proceso no definido */
	if(pprc>=MAXPROCESS) return(_E_PRRB);   /* rebosa tabla */
	prc[pprc].numpro=par1;  /* guarda num. proc. al que se llama */
	prc[pprc].nl=numlin;    /* y el num. de l¡nea en archivo de entrada */
	pprc++;     /* apunta a sgte. elemento */
}
return(_E_NERR);
}

int done(void)
{
if(check_mem(1)) return(_E_FMEM);
*proc_act++=2;
return(_E_NERR);
}

int notdone(void)
{
if(check_mem(1)) return(_E_FMEM);
*proc_act++=3;
return(_E_NERR);
}

int resp(void)
{
if(check_mem(1)) return(_E_FMEM);
*proc_act++=4;
return(_E_NERR);
}

int noresp(void)
{
if(check_mem(1)) return(_E_FMEM);
*proc_act++=5;
return(_E_NERR);
}

int defwin(void)
{
BYTE nw, cw, wx, wy, lx, ly;
int indir1, indir2;

if(check_mem(8)) return(_E_FMEM);
if(sgte_campo()) return(_E_FCAM);
if((indir1=coge_num2(&nw,l))==2) return(_E_CNUM);
if(!indir1 && (nw>N_VENT-1)) return(_E_WPNW);
if(sgte_campo()) return(_E_FCAM);
if((indir2=coge_num2(&cw,l))==2) return(_E_CNUM);
if(sgte_campo()) return(_E_FCAM);
if(coge_num(&wy,l)) return(_E_CNUM);
if(sgte_campo()) return(_E_FCAM);
if(coge_num(&wx,l)) return(_E_CNUM);
if(wx>COLUMNAS) return(_E_WPWX);
if(wy>FILAS) return(_E_WPWY);
if(sgte_campo()) return(_E_FCAM);
if(coge_num(&lx,l)) return(_E_CNUM);
if(wx+lx>COLUMNAS) return(_E_WPFP);
if(sgte_campo()) return(_E_FCAM);
if(coge_num(&ly,l)) return(_E_CNUM);
if(wy+ly>FILAS) return(_E_WPFP);

if(indir1 && !indir2) *proc_act++=INDIR1;
else if(!indir1 && indir2) *proc_act++=INDIR2;
else if(indir1 && indir2) *proc_act++=INDIR12;
*proc_act++=6;
*proc_act++=nw;
*proc_act++=cw;
*proc_act++=wy;
*proc_act++=wx;
*proc_act++=lx;
*proc_act++=ly;
return(_E_NERR);
}

int window(void)
{
BYTE nw;
int indir;

if(check_mem(3)) return(_E_FMEM);
if(sgte_campo()) return(_E_FCAM);
if((indir=coge_num2(&nw,l))==2) return(_E_CNUM);
if(!indir && nw>N_VENT-1) return(_E_WPNW);
if(indir) *proc_act++=INDIR1;
*proc_act++=7;
*proc_act++=nw;
return(_E_NERR);
}

int clw(void)
{
BYTE nw;
int indir;

if(check_mem(3)) return(_E_FMEM);
if(sgte_campo()) return(_E_FCAM);
if((indir=coge_num2(&nw,l))==2) return(_E_CNUM);
if(!indir && nw>N_VENT-1) return(_E_WPNW);
if(indir) *proc_act++=INDIR1;
*proc_act++=8;
*proc_act++=nw;
return(_E_NERR);
}

int let(void)
{
BYTE varno, val;
int indir1, indir2;

if(check_mem(4)) return(_E_FMEM);
if(sgte_campo()) return(_E_FCAM);
if((indir1=coge_num2(&varno,l))==2) return(_E_CNUM);
if(sgte_campo()) return(_E_FCAM);
if((indir2=coge_num2(&val,l))==2) return(_E_CNUM);
if(indir1 && !indir2) *proc_act++=INDIR1;
else if(!indir1 && indir2) *proc_act++=INDIR2;
else if(indir1 && indir2) *proc_act++=INDIR12;
*proc_act++=9;
*proc_act++=varno;
*proc_act++=val;
return(_E_NERR);
}

int eq(void)
{
BYTE varno, val;
int indir1, indir2;

if(check_mem(4)) return(_E_FMEM);
if(sgte_campo()) return(_E_FCAM);
if((indir1=coge_num2(&varno,l))==2) return(_E_CNUM);
if(sgte_campo()) return(_E_FCAM);
if((indir2=coge_num2(&val,l))==2) return(_E_CNUM);
if(indir1 && !indir2) *proc_act++=INDIR1;
else if(!indir1 && indir2) *proc_act++=INDIR2;
else if(indir1 && indir2) *proc_act++=INDIR12;
*proc_act++=10;
*proc_act++=varno;
*proc_act++=val;
return(_E_NERR);
}

int noteq(void)
{
BYTE varno, val;
int indir1, indir2;

if(check_mem(4)) return(_E_FMEM);
if(sgte_campo()) return(_E_FCAM);
if((indir1=coge_num2(&varno,l))==2) return(_E_CNUM);
if(sgte_campo()) return(_E_FCAM);
if((indir2=coge_num2(&val,l))==2) return(_E_CNUM);
if(indir1 && !indir2) *proc_act++=INDIR1;
else if(!indir1 && indir2) *proc_act++=INDIR2;
else if(indir1 && indir2) *proc_act++=INDIR12;
*proc_act++=11;
*proc_act++=varno;
*proc_act++=val;
return(_E_NERR);
}

int lt(void)
{
BYTE varno, val;
int indir1, indir2;

if(check_mem(4)) return(_E_FMEM);
if(sgte_campo()) return(_E_FCAM);
if((indir1=coge_num2(&varno,l))==2) return(_E_CNUM);
if(sgte_campo()) return(_E_FCAM);
if((indir2=coge_num2(&val,l))==2) return(_E_CNUM);
if(indir1 && !indir2) *proc_act++=INDIR1;
else if(!indir1 && indir2) *proc_act++=INDIR2;
else if(indir1 && indir2) *proc_act++=INDIR12;
*proc_act++=12;
*proc_act++=varno;
*proc_act++=val;
return(_E_NERR);
}

int gt(void)
{
BYTE varno, val;
int indir1, indir2;

if(check_mem(4)) return(_E_FMEM);
if(sgte_campo()) return(_E_FCAM);
if((indir1=coge_num2(&varno,l))==2) return(_E_CNUM);
if(sgte_campo()) return(_E_FCAM);
if((indir2=coge_num2(&val,l))==2) return(_E_CNUM);
if(indir1 && !indir2) *proc_act++=INDIR1;
else if(!indir1 && indir2) *proc_act++=INDIR2;
else if(indir1 && indir2) *proc_act++=INDIR12;
*proc_act++=13;
*proc_act++=varno;
*proc_act++=val;
return(_E_NERR);
}

int mes(void)
{
BYTE mesno;
int indir;

if(check_mem(3)) return(_E_FMEM);
if(sgte_campo()) return(_E_FCAM);
if((indir=coge_num2(&mesno,l))==2) return(_E_CNUM);
if(!indir && (mesno>=m_act)) return(_E_MPNM);
if(indir) *proc_act++=INDIR1;
*proc_act++=14;
*proc_act++=mesno;
return(_E_NERR);
}

int newline(void)
{
if(check_mem(1)) return(_E_FMEM);
*proc_act++=15;
return(_E_NERR);
}

int message(void)
{
BYTE mesno;
int indir;

if(check_mem(3)) return(_E_FMEM);
if(sgte_campo()) return(_E_FCAM);
if((indir=coge_num2(&mesno,l))==2) return(_E_CNUM);
if(!indir && (mesno>=m_act)) return(_E_MPNM);
if(indir) *proc_act++=INDIR1;
*proc_act++=16;
*proc_act++=mesno;
return(_E_NERR);
}

int sysmess(void)
{
BYTE mesno;
int indir;

if(check_mem(3)) return(_E_FMEM);
if(sgte_campo()) return(_E_FCAM);
if((indir=coge_num2(&mesno,l))==2) return(_E_CNUM);
if(!indir && (mesno>=m_actsys)) return(_E_MPNM);
if(indir) *proc_act++=INDIR1;
*proc_act++=17;
*proc_act++=mesno;
return(_E_NERR);
}

int desc(void)
{
BYTE locno;
int indir;

if(check_mem(3)) return(_E_FMEM);
if(sgte_campo()) return(_E_FCAM);
if((indir=coge_num2(&locno,l))==2) return(_E_CNUM);
if(!indir && (locno>=n_loc)) return(_E_MPNL);
if(indir) *proc_act++=INDIR1;
*proc_act++=18;
*proc_act++=locno;
return(_E_NERR);
}

int add(void)
{
BYTE varno, val;
int indir1, indir2;

if(check_mem(4)) return(_E_FMEM);
if(sgte_campo()) return(_E_FCAM);
if((indir1=coge_num2(&varno,l))==2) return(_E_CNUM);
if(sgte_campo()) return(_E_FCAM);
if((indir2=coge_num2(&val,l))==2) return(_E_CNUM);
if(indir1 && !indir2) *proc_act++=INDIR1;
else if(!indir1 && indir2) *proc_act++=INDIR2;
else if(indir1 && indir2) *proc_act++=INDIR12;
*proc_act++=19;
*proc_act++=varno;
*proc_act++=val;
return(_E_NERR);
}

int sub(void)
{
BYTE varno, val;
int indir1, indir2;

if(check_mem(4)) return(_E_FMEM);
if(sgte_campo()) return(_E_FCAM);
if((indir1=coge_num2(&varno,l))==2) return(_E_CNUM);
if(sgte_campo()) return(_E_FCAM);
if((indir2=coge_num2(&val,l))==2) return(_E_CNUM);
if(indir1 && !indir2) *proc_act++=INDIR1;
else if(!indir1 && indir2) *proc_act++=INDIR2;
else if(indir1 && indir2) *proc_act++=INDIR12;
*proc_act++=20;
*proc_act++=varno;
*proc_act++=val;
return(_E_NERR);
}

int inc(void)
{
BYTE varno;
int indir;

if(check_mem(3)) return(_E_FMEM);
if(sgte_campo()) return(_E_FCAM);
if((indir=coge_num2(&varno,l))==2) return(_E_CNUM);
if(indir) *proc_act++=INDIR1;
*proc_act++=21;
*proc_act++=varno;
return(_E_NERR);
}

int dec(void)
{
BYTE varno;
int indir;

if(check_mem(3)) return(_E_FMEM);
if(sgte_campo()) return(_E_FCAM);
if((indir=coge_num2(&varno,l))==2) return(_E_CNUM);
if(indir) *proc_act++=INDIR1;
*proc_act++=22;
*proc_act++=varno;
return(_E_NERR);
}

int set(void)
{
BYTE flagno;
int indir;

if(check_mem(3)) return(_E_FMEM);
if(sgte_campo()) return(_E_FCAM);
if((indir=coge_num2(&flagno,l))==2) return(_E_CNUM);
if(indir) *proc_act++=INDIR1;
*proc_act++=23;
*proc_act++=flagno;
return(_E_NERR);
}

int clear(void)
{
BYTE flagno;
int indir;

if(check_mem(3)) return(_E_FMEM);
if(sgte_campo()) return(_E_FCAM);
if((indir=coge_num2(&flagno,l))==2) return(_E_CNUM);
if(indir) *proc_act++=INDIR1;
*proc_act++=24;
*proc_act++=flagno;
return(_E_NERR);
}

int zero(void)
{
BYTE flagno;
int indir;

if(check_mem(3)) return(_E_FMEM);
if(sgte_campo()) return(_E_FCAM);
if((indir=coge_num2(&flagno,l))==2) return(_E_CNUM);
if(indir) *proc_act++=INDIR1;
*proc_act++=25;
*proc_act++=flagno;
return(_E_NERR);
}

int notzero(void)
{
BYTE flagno;
int indir;

if(check_mem(3)) return(_E_FMEM);
if(sgte_campo()) return(_E_FCAM);
if((indir=coge_num2(&flagno,l))==2) return(_E_CNUM);
if(indir) *proc_act++=INDIR1;
*proc_act++=26;
*proc_act++=flagno;
return(_E_NERR);
}

int place(void)
{
BYTE objno, locno;
int indir1, indir2;

if(check_mem(4)) return(_E_FMEM);
if(sgte_campo()) return(_E_FCAM);
if((indir1=coge_num2(&objno,l))==2) return(_E_CNUM);
if(!indir1 && (objno>=objact)) return(_E_OPNV);
if(sgte_campo()) return(_E_FCAM);
if((indir2=coge_num2(&locno,l))==2) return(_E_CNUM);
if(!indir2 && (locno>=n_loc) && (locno<NO_CREADO)) return(_E_MPNL);
if(indir1 && !indir2) *proc_act++=INDIR1;
else if(!indir1 && indir2) *proc_act++=INDIR2;
else if(indir1 && indir2) *proc_act++=INDIR12;
*proc_act++=27;
*proc_act++=objno;
*proc_act++=locno;
return(_E_NERR);
}

int get(void)
{
BYTE objno;
int indir;

if(check_mem(3)) return(_E_FMEM);
if(sgte_campo()) return(_E_FCAM);
if((indir=coge_num2(&objno,l))==2) return(_E_CNUM);
if(!indir && (objno>=objact)) return(_E_OPNV);
if(indir) *proc_act++=INDIR1;
*proc_act++=28;
*proc_act++=objno;
return(_E_NERR);
}

int drop(void)
{
BYTE objno;
int indir;

if(check_mem(3)) return(_E_FMEM);
if(sgte_campo()) return(_E_FCAM);
if((indir=coge_num2(&objno,l))==2) return(_E_CNUM);
if(!indir && (objno>=objact)) return(_E_OPNV);
if(indir) *proc_act++=INDIR1;
*proc_act++=29;
*proc_act++=objno;
return(_E_NERR);
}

int input(void)
{
if(check_mem(1)) return(_E_FMEM);
*proc_act++=30;
return(_E_NERR);
}

int parse(void)
{
if(check_mem(1)) return(_E_FMEM);
*proc_act++=31;
return(_E_NERR);
}

int skip(void)
{
char lb[LONG_LAB];
int i;
unsigned desplu;
int despli;

if(check_mem(3)) return(_E_FMEM);
if(sgte_campo()) return(_E_FCAM);
if(*l!='$') return(_E_NLAB);    /* etiqueta no v lida */
l++;
for(i=0;i<LONG_LAB-1;i++) {     /* recoge etiqueta */
	if(fin_linea(l) || (*l==' ') || (*l=='\t')) break;
	lb[i]=mayuscula(*l);
	l++;
}
for(;i<LONG_LAB-1;i++) lb[i]=' ';   /* rellena con espacios */
lb[i]='\0';
if(!ptlab) {  /* si no hay ninguna etiqueta definida */
	if(mete_fsk(lb,proc_act+1)) return(_E_RBTS); /* guarda refer. forward */
		*proc_act++=32;
		*proc_act++=0;  /* inicializa a 0, pero luego ser  sustituido */
		*proc_act++=0;  /* por su desplazamiento correspondiente */
}
else {  /* si hay alguna definida */
	for(i=0;i<ptlab;i++) if(!strcmp(lb,etiqueta[i].label)) break;
	if(i==ptlab) {  /* si no encontr¢ la etiqueta */
		/* guarda ref. forward */
		if(mete_fsk(lb,proc_act+1)) return(_E_RBTS);
		*proc_act++=32;
		*proc_act++=0;  /* inicializa a 0, luego ser  sustituido */
		*proc_act++=0;  /* por su desplazamiento correspondiente */
	}
	else {  /* si la etiqueta est  definida */
		*proc_act++=32;
		/* desplaz. atr s */
		desplu=(unsigned)(proc_act-1-etiqueta[i].plab);
		if(desplu>(unsigned)32768) return(_E_LBFR);
		despli=-desplu;
		*proc_act++=(BYTE)(despli & 0x00ff);        /* byte bajo */
		*proc_act++=(BYTE)((despli >> 8) & 0x00ff); /* byte alto */
	}
}
return(_E_NERR);
}

int at(void)
{
BYTE locno;
int indir;

if(check_mem(3)) return(_E_FMEM);
if(sgte_campo()) return(_E_FCAM);
if((indir=coge_num2(&locno,l))==2) return(_E_CNUM);
if(indir) *proc_act++=INDIR1;
*proc_act++=33;
*proc_act++=locno;
return(_E_NERR);
}

int notat(void)
{
BYTE locno;
int indir;

if(check_mem(3)) return(_E_FMEM);
if(sgte_campo()) return(_E_FCAM);
if((indir=coge_num2(&locno,l))==2) return(_E_CNUM);
if(indir) *proc_act++=INDIR1;
*proc_act++=34;
*proc_act++=locno;
return(_E_NERR);
}

int atgt(void)
{
BYTE locno;
int indir;

if(check_mem(3)) return(_E_FMEM);
if(sgte_campo()) return(_E_FCAM);
if((indir=coge_num2(&locno,l))==2) return(_E_CNUM);
if(indir) *proc_act++=INDIR1;
*proc_act++=35;
*proc_act++=locno;
return(_E_NERR);
}

int atlt(void)
{
BYTE locno;
int indir;

if(check_mem(3)) return(_E_FMEM);
if(sgte_campo()) return(_E_FCAM);
if((indir=coge_num2(&locno,l))==2) return(_E_CNUM);
if(indir) *proc_act++=INDIR1;
*proc_act++=36;
*proc_act++=locno;
return(_E_NERR);
}

int adject1(void)
{
int adjetivo;
char adj[LONGPAL];

if(check_mem(2)) return(_E_FMEM);
if(sgte_campo()) return(_E_FCAM);
if(coge_pal(adj)==_E_CVOC) return(_E_CVOC); /* coge el campo palabra */
if((adjetivo=esta_en_voc(adj))==(NUM_PAL+1)) return(_E_NPVC);
if(vocabulario[adjetivo].tipo!=_ADJT) return(_E_NCAD);
*proc_act++=37;
*proc_act++=vocabulario[adjetivo].num;
return(_E_NERR);
}

int noun2(void)
{
int nombre;
char nmb[LONGPAL];

if(check_mem(2)) return(_E_FMEM);
if(sgte_campo()) return(_E_FCAM);
if(coge_pal(nmb)==_E_CVOC) return(_E_CVOC); /* coge el campo palabra */
if((nombre=esta_en_voc(nmb))==(NUM_PAL+1)) return(_E_NPVC);
if(vocabulario[nombre].tipo!=_NOMB) return(_E_NNMB);
*proc_act++=38;
*proc_act++=vocabulario[nombre].num;
return(_E_NERR);
}

int adject2(void)
{
int adjetivo;
char adj[LONGPAL];

if(check_mem(2)) return(_E_FMEM);
if(sgte_campo()) return(_E_FCAM);
if(coge_pal(adj)==_E_CVOC) return(_E_CVOC); /* coge el campo palabra */
if((adjetivo=esta_en_voc(adj))==(NUM_PAL+1)) return(_E_NPVC);
if(vocabulario[adjetivo].tipo!=_ADJT) return(_E_NCAD);
*proc_act++=39;
*proc_act++=vocabulario[adjetivo].num;
return(_E_NERR);
}

int listat(void)
{
BYTE locno;
int indir;

if(check_mem(3)) return(_E_FMEM);
if(sgte_campo()) return(_E_FCAM);
if((indir=coge_num2(&locno,l))==2) return(_E_CNUM);
if(!indir && (locno>=n_loc) && (locno<NO_CREADO)) return(_E_MPNL);
if(indir) *proc_act++=INDIR1;
*proc_act++=40;
*proc_act++=locno;
return(_E_NERR);
}

int isat(void)
{
BYTE objno, locno;
int indir1, indir2;

if(check_mem(4)) return(_E_FMEM);
if(sgte_campo()) return(_E_FCAM);
if((indir1=coge_num2(&objno,l))==2) return(_E_CNUM);
if(!indir1 && (objno>=objact)) return(_E_OPNV);
if(sgte_campo()) return(_E_FCAM);
if((indir2=coge_num2(&locno,l))==2) return(_E_CNUM);
if(!indir2 && (locno>=n_loc) && (locno<NO_CREADO)) return(_E_MPNL);
if(indir1 && !indir2) *proc_act++=INDIR1;
else if(!indir1 && indir2) *proc_act++=INDIR2;
else if(indir1 && indir2) *proc_act++=INDIR12;
*proc_act++=41;
*proc_act++=objno;
*proc_act++=locno;
return(_E_NERR);
}

int isnotat(void)
{
BYTE objno, locno;
int indir1, indir2;

if(check_mem(4)) return(_E_FMEM);
if(sgte_campo()) return(_E_FCAM);
if((indir1=coge_num2(&objno,l))==2) return(_E_CNUM);
if(!indir1 && (objno>=objact)) return(_E_OPNV);
if(sgte_campo()) return(_E_FCAM);
if((indir2=coge_num2(&locno,l))==2) return(_E_CNUM);
if(!indir2 && (locno>=n_loc) && (locno<NO_CREADO)) return(_E_MPNL);
if(indir1 && !indir2) *proc_act++=INDIR1;
else if(!indir1 && indir2) *proc_act++=INDIR2;
else if(indir1 && indir2) *proc_act++=INDIR12;
*proc_act++=42;
*proc_act++=objno;
*proc_act++=locno;
return(_E_NERR);
}

int present(void)
{
BYTE objno;
int indir;

if(check_mem(3)) return(_E_FMEM);
if(sgte_campo()) return(_E_FCAM);
if((indir=coge_num2(&objno,l))==2) return(_E_CNUM);
if(!indir && (objno>=objact)) return(_E_OPNV);
if(indir) *proc_act++=INDIR1;
*proc_act++=43;
*proc_act++=objno;
return(_E_NERR);
}

int absent(void)
{
BYTE objno;
int indir;

if(check_mem(3)) return(_E_FMEM);
if(sgte_campo()) return(_E_FCAM);
if((indir=coge_num2(&objno,l))==2) return(_E_CNUM);
if(!indir && (objno>=objact)) return(_E_OPNV);
if(indir) *proc_act++=INDIR1;
*proc_act++=44;
*proc_act++=objno;
return(_E_NERR);
}

int worn(void)
{
BYTE objno;
int indir;

if(check_mem(3)) return(_E_FMEM);
if(sgte_campo()) return(_E_FCAM);
if((indir=coge_num2(&objno,l))==2) return(_E_CNUM);
if(!indir && (objno>=objact)) return(_E_OPNV);
if(indir) *proc_act++=INDIR1;
*proc_act++=45;
*proc_act++=objno;
return(_E_NERR);
}

int notworn(void)
{
BYTE objno;
int indir;

if(check_mem(3)) return(_E_FMEM);
if(sgte_campo()) return(_E_FCAM);
if((indir=coge_num2(&objno,l))==2) return(_E_CNUM);
if(!indir && (objno>=objact)) return(_E_OPNV);
if(indir) *proc_act++=INDIR1;
*proc_act++=46;
*proc_act++=objno;
return(_E_NERR);
}

int carried(void)
{
BYTE objno;
int indir;

if(check_mem(3)) return(_E_FMEM);
if(sgte_campo()) return(_E_FCAM);
if((indir=coge_num2(&objno,l))==2) return(_E_CNUM);
if(!indir && (objno>=objact)) return(_E_OPNV);
if(indir) *proc_act++=INDIR1;
*proc_act++=47;
*proc_act++=objno;
return(_E_NERR);
}

int notcarr(void)
{
BYTE objno;
int indir;

if(check_mem(3)) return(_E_FMEM);
if(sgte_campo()) return(_E_FCAM);
if((indir=coge_num2(&objno,l))==2) return(_E_CNUM);
if(!indir && (objno>=objact)) return(_E_OPNV);
if(indir) *proc_act++=INDIR1;
*proc_act++=48;
*proc_act++=objno;
return(_E_NERR);
}

int wear(void)
{
BYTE objno;
int indir;

if(check_mem(3)) return(_E_FMEM);
if(sgte_campo()) return(_E_FCAM);
if((indir=coge_num2(&objno,l))==2) return(_E_CNUM);
if(!indir && (objno>=objact)) return(_E_OPNV);
if(indir) *proc_act++=INDIR1;
*proc_act++=49;
*proc_act++=objno;
return(_E_NERR);
}

int remove1(void)
{
BYTE objno;
int indir;

if(check_mem(3)) return(_E_FMEM);
if(sgte_campo()) return(_E_FCAM);
if((indir=coge_num2(&objno,l))==2) return(_E_CNUM);
if(!indir && (objno>=objact)) return(_E_OPNV);
if(indir) *proc_act++=INDIR1;
*proc_act++=50;
*proc_act++=objno;
return(_E_NERR);
}

int create(void)
{
BYTE objno;
int indir;

if(check_mem(3)) return(_E_FMEM);
if(sgte_campo()) return(_E_FCAM);
if((indir=coge_num2(&objno,l))==2) return(_E_CNUM);
if(!indir && (objno>=objact)) return(_E_OPNV);
if(indir) *proc_act++=INDIR1;
*proc_act++=51;
*proc_act++=objno;
return(_E_NERR);
}

int destroy(void)
{
BYTE objno;
int indir;

if(check_mem(3)) return(_E_FMEM);
if(sgte_campo()) return(_E_FCAM);
if((indir=coge_num2(&objno,l))==2) return(_E_CNUM);
if(!indir && (objno>=objact)) return(_E_OPNV);
if(indir) *proc_act++=INDIR1;
*proc_act++=52;
*proc_act++=objno;
return(_E_NERR);
}

int swap(void)
{
BYTE objno1, objno2;
int indir1, indir2;

if(check_mem(4)) return(_E_FMEM);
if(sgte_campo()) return(_E_FCAM);
if((indir1=coge_num2(&objno1,l))==2) return(_E_CNUM);
if(!indir1 && (objno1>=objact)) return(_E_OPNV);
if(sgte_campo()) return(_E_FCAM);
if((indir2=coge_num2(&objno2,l))==2) return(_E_CNUM);
if(!indir2 && (objno2>=objact)) return(_E_OPNV);
if(indir1 && !indir2) *proc_act++=INDIR1;
else if(!indir1 && indir2) *proc_act++=INDIR2;
else if(indir1 && indir2) *proc_act++=INDIR12;
*proc_act++=53;
*proc_act++=objno1;
*proc_act++=objno2;
return(_E_NERR);
}

int restart(void)
{
if(check_mem(1)) return(_E_FMEM);
*proc_act++=54;
return(_E_NERR);
}

int whato(void)
{
if(check_mem(1)) return(_E_FMEM);
*proc_act++=55;
return(_E_NERR);
}

int move(void)
{
BYTE varno;
int indir;

if(check_mem(3)) return(_E_FMEM);
if(sgte_campo()) return(_E_FCAM);
if((indir=coge_num2(&varno,l))==2) return(_E_CNUM);
if(indir) *proc_act++=INDIR1;
*proc_act++=56;
*proc_act++=varno;
return(_E_NERR);
}

int ismov(void)
{
if(check_mem(1)) return(_E_FMEM);
*proc_act++=57;
return(_E_NERR);
}

int goto1(void)
{
BYTE locno;
int indir;

if(check_mem(3)) return(_E_FMEM);
if(sgte_campo()) return(_E_FCAM);
if((indir=coge_num2(&locno,l))==2) return(_E_CNUM);
if(!indir && (locno>=n_loc)) return(_E_MPNL);
if(indir) *proc_act++=INDIR1;
*proc_act++=58;
*proc_act++=locno;
return(_E_NERR);
}

int print(void)
{
BYTE varno;
int indir;

if(check_mem(3)) return(_E_FMEM);
if(sgte_campo()) return(_E_FCAM);
if((indir=coge_num2(&varno,l))==2) return(_E_CNUM);
if(indir) *proc_act++=INDIR1;
*proc_act++=59;
*proc_act++=varno;
return(_E_NERR);
}

int dprint(void)
{
BYTE varno;
int indir;

if(check_mem(3)) return(_E_FMEM);
if(sgte_campo()) return(_E_FCAM);
if((indir=coge_num2(&varno,l))==2) return(_E_CNUM);
if(indir) *proc_act++=INDIR1;
*proc_act++=60;
*proc_act++=varno;
return(_E_NERR);
}

int cls(void)
{
if(check_mem(1)) return(_E_FMEM);
*proc_act++=61;
return(_E_NERR);
}

int anykey(void)
{
if(check_mem(1)) return(_E_FMEM);
*proc_act++=62;
return(_E_NERR);
}

int pause(void)
{
BYTE pau;
int indir;

if(check_mem(3)) return(_E_FMEM);
if(sgte_campo()) return(_E_FCAM);
if((indir=coge_num2(&pau,l))==2) return(_E_CNUM);
if(indir) *proc_act++=INDIR1;
*proc_act++=63;
*proc_act++=pau;
return(_E_NERR);
}

int listobj(void)
{
if(check_mem(1)) return(_E_FMEM);
*proc_act++=64;
return(_E_NERR);
}

int firsto(void)
{
if(check_mem(1)) return(_E_FMEM);
*proc_act++=65;
return(_E_NERR);
}

int nexto(void)
{
BYTE locno;
int indir;

if(check_mem(3)) return(_E_FMEM);
if(sgte_campo()) return(_E_FCAM);
if((indir=coge_num2(&locno,l))==2) return(_E_CNUM);
if(!indir && (locno>=n_loc) && (locno<NO_CREADO)) return(_E_MPNL);
if(indir) *proc_act++=INDIR1;
*proc_act++=66;
*proc_act++=locno;
return(_E_NERR);
}

int synonym(void)
{
BYTE verb, nomb;
char pal[LONGPAL];
int i, errct;

if(check_mem(3)) return(_E_FMEM);
if(sgte_campo()) return(_E_FCAM);
errct=coge_pal(pal);    /* recoge campo verbo */
if(*pal=='_') {         /* si es no-palabra */
	verb=NO_PAL;
}
else {
	if(errct) return(errct);    /* si hubo error al coger pal., sale */
	if((i=esta_en_voc(pal))==NUM_PAL+1) return(_E_NPVC); /* no en voc. */
	/* si no es verbo ni nombre convertible */
	if((vocabulario[i].tipo!=_VERB) &&
	  ((vocabulario[i].tipo==_NOMB) && (vocabulario[i].num>=n_conv)))
	  return(_E_NVNC);
	verb=vocabulario[i].num;    /* guarda num. de verbo-nomb.conv. */
}
if(sgte_campo()) return(_E_FCAM);
errct=coge_pal(pal);    /* recoge campo palabra */
if(*pal=='_') {         /* si es no-palabra */
	nomb=NO_PAL;
}
else {
	if(errct==_E_CVOC) return(errct);   /* sale si hubo pal. no v lida */
	if((i=esta_en_voc(pal))==NUM_PAL+1) return(_E_NPVC); /* no en voc. */
	if(vocabulario[i].tipo!=_NOMB) return(_E_NNNN); /* no es nombre */
	nomb=vocabulario[i].num;    /* guarda num. de nombre */
}
*proc_act++=67;
*proc_act++=verb;
*proc_act++=nomb;
return(_E_NERR);
}

int hasat(void)
{
BYTE val;
int indir;

if(check_mem(3)) return(_E_FMEM);
if(sgte_campo()) return(_E_FCAM);
if((indir=coge_num2(&val,l))==2) return(_E_CNUM);
if(!indir && (val>17)) return(_E_NVHT);
if(indir) *proc_act++=INDIR1;
*proc_act++=68;
*proc_act++=val;
return(_E_NERR);
}

int hasnat(void)
{
BYTE val;
int indir;

if(check_mem(3)) return(_E_FMEM);
if(sgte_campo()) return(_E_FCAM);
if((indir=coge_num2(&val,l))==2) return(_E_CNUM);
if(!indir && (val>17)) return(_E_NVHT);
if(indir) *proc_act++=INDIR1;
*proc_act++=69;
*proc_act++=val;
return(_E_NERR);
}

int light(void)
{
if(check_mem(1)) return(_E_FMEM);
*proc_act++=70;
return(_E_NERR);
}

int nolight(void)
{
if(check_mem(1)) return(_E_FMEM);
*proc_act++=71;
return(_E_NERR);
}

int random(void)
{
BYTE varno, rnd;
int indir1, indir2;

if(check_mem(4)) return(_E_FMEM);
if(sgte_campo()) return(_E_FCAM);
if((indir1=coge_num2(&varno,l))==2) return(_E_CNUM);
if(sgte_campo()) return(_E_FCAM);
if((indir2=coge_num2(&rnd,l))==2) return(_E_CNUM);
if(indir1 && !indir2) *proc_act++=INDIR1;
else if(!indir1 && indir2) *proc_act++=INDIR2;
else if(indir1 && indir2) *proc_act++=INDIR12;
*proc_act++=72;
*proc_act++=varno;
*proc_act++=rnd;
return(_E_NERR);
}

int seed(void)
{
BYTE seed;
int indir;

if(check_mem(3)) return(_E_FMEM);
if(sgte_campo()) return(_E_FCAM);
if((indir=coge_num2(&seed,l))==2) return(_E_CNUM);
if(indir) *proc_act++=INDIR1;
*proc_act++=73;
*proc_act++=seed;
return(_E_NERR);
}

int puto(void)
{
BYTE locno;
int indir;

if(check_mem(3)) return(_E_FMEM);
if(sgte_campo()) return(_E_FCAM);
if((indir=coge_num2(&locno,l))==2) return(_E_CNUM);
if(!indir && (locno>=n_loc) && (locno<NO_CREADO)) return(_E_MPNL);
if(indir) *proc_act++=INDIR1;
*proc_act++=74;
*proc_act++=locno;
return(_E_NERR);
}

int inkey(void)
{
if(check_mem(1)) return(_E_FMEM);
*proc_act++=75;
return(_E_NERR);
}

int copyov(void)
{
BYTE objno, varno;
int indir1, indir2;

if(check_mem(4)) return(_E_FMEM);
if(sgte_campo()) return(_E_FCAM);
if((indir1=coge_num2(&objno,l))==2) return(_E_CNUM);
if(!indir1 && (objno>=objact)) return(_E_OPNV);
if(sgte_campo()) return(_E_FCAM);
if((indir2=coge_num2(&varno,l))==2) return(_E_CNUM);
if(indir1 && !indir2) *proc_act++=INDIR1;
else if(!indir1 && indir2) *proc_act++=INDIR2;
else if(indir1 && indir2) *proc_act++=INDIR12;
*proc_act++=76;
*proc_act++=objno;
*proc_act++=varno;
return(_E_NERR);
}

int chance(void)
{
BYTE rnd;
int indir;

if(check_mem(3)) return(_E_FMEM);
if(sgte_campo()) return(_E_FCAM);
if((indir=coge_num2(&rnd,l))==2) return(_E_CNUM);
if(!indir && (rnd>100)) return(_E_CHNV);
if(indir) *proc_act++=INDIR1;
*proc_act++=77;
*proc_act++=rnd;
return(_E_NERR);
}

int ramsave(void)
{
BYTE banco;
int indir;

if(check_mem(3)) return(_E_FMEM);
if(sgte_campo()) return(_E_FCAM);
if((indir=coge_num2(&banco,l))==2) return(_E_CNUM);
if(!indir && (banco>1)) return(_E_RBNV);
if(indir) *proc_act++=INDIR1;
*proc_act++=78;
*proc_act++=banco;
return(_E_NERR);
}

int ramload(void)
{
BYTE banco, vtop, ftop;
int indir1, indir2;

if(check_mem(5)) return(_E_FMEM);
if(sgte_campo()) return(_E_FCAM);
if((indir1=coge_num2(&banco,l))==2) return(_E_CNUM);
if(!indir1 && (banco>1)) return(_E_RBNV);
if(sgte_campo()) return(_E_FCAM);
if((indir2=coge_num2(&vtop,l))==2) return(_E_CNUM);
if(sgte_campo()) return(_E_FCAM);
if(coge_num(&ftop,l)) return(_E_CNUM);
if(indir1 && !indir2) *proc_act++=INDIR1;
else if(!indir1 && indir2) *proc_act++=INDIR2;
else if(indir1 && indir2) *proc_act++=INDIR12;
*proc_act++=79;
*proc_act++=banco;
*proc_act++=vtop;
*proc_act++=ftop;
return(_E_NERR);
}

int ability(void)
{
BYTE nobjs;
int indir;

if(check_mem(3)) return(_E_FMEM);
if(sgte_campo()) return(_E_FCAM);
if((indir=coge_num2(&nobjs,l))==2) return(_E_CNUM);
if(indir) *proc_act++=INDIR1;
*proc_act++=80;
*proc_act++=nobjs;
return(_E_NERR);
}

int autog(void)
{
if(check_mem(1)) return(_E_FMEM);
*proc_act++=81;
return(_E_NERR);
}

int autod(void)
{
if(check_mem(1)) return(_E_FMEM);
*proc_act++=82;
return(_E_NERR);
}

int autow(void)
{
if(check_mem(1)) return(_E_FMEM);
*proc_act++=83;
return(_E_NERR);
}

int autor(void)
{
if(check_mem(1)) return(_E_FMEM);
*proc_act++=84;
return(_E_NERR);
}

int isdoall(void)
{
if(check_mem(1)) return(_E_FMEM);
*proc_act++=85;
return(_E_NERR);
}

int ask(void)
{
BYTE smess1, smess2, varno;
int indir1, indir2;

if(check_mem(5)) return(_E_FMEM);
if(sgte_campo()) return(_E_FCAM);
if((indir1=coge_num2(&smess1,l))==2) return(_E_CNUM);
if(!indir1 && (smess1>=m_actsys)) return(_E_MPNP);
if(sgte_campo()) return(_E_FCAM);
if((indir2=coge_num2(&smess2,l))==2) return(_E_CNUM);
if(!indir2 && (smess2>=m_actsys)) return(_E_MPNS);
if(sgte_campo()) return(_E_FCAM);
if(coge_num(&varno,l)) return(_E_CNUM);
if(indir1 && !indir2) *proc_act++=INDIR1;
else if(!indir1 && indir2) *proc_act++=INDIR2;
else if(indir1 && indir2) *proc_act++=INDIR12;
*proc_act++=86;
*proc_act++=smess1;
*proc_act++=smess2;
*proc_act++=varno;
return(_E_NERR);
}

int quit(void)
{
if(check_mem(1)) return(_E_FMEM);
*proc_act++=87;
return(_E_NERR);
}

int save(void)
{
if(check_mem(1)) return(_E_FMEM);
*proc_act++=88;
return(_E_NERR);
}

int load(void)
{
BYTE vtop, ftop;
int indir1, indir2;

if(check_mem(4)) return(_E_FMEM);
if(sgte_campo()) return(_E_FCAM);
if((indir1=coge_num2(&vtop,l))==2) return(_E_CNUM);
if(sgte_campo()) return(_E_FCAM);
if((indir2=coge_num2(&ftop,l))==2) return(_E_CNUM);
if(indir1 && !indir2) *proc_act++=INDIR1;
else if(!indir1 && indir2) *proc_act++=INDIR2;
else if(indir1 && indir2) *proc_act++=INDIR12;
*proc_act++=89;
*proc_act++=vtop;
*proc_act++=ftop;
return(_E_NERR);
}

int exit1(void)
{
BYTE ex;
int indir;

if(check_mem(3)) return(_E_FMEM);
if(sgte_campo()) return(_E_FCAM);
if((indir=coge_num2(&ex,l))==2) return(_E_CNUM);
if(!indir && (ex>1)) return(_E_NOEX);
if(indir) *proc_act++=INDIR1;
*proc_act++=90;
*proc_act++=ex;
return(_E_NERR);
}

int end1(void)
{
if(check_mem(1)) return(_E_FMEM);
*proc_act++=91;
return(_E_NERR);
}

int printat(void)
{
BYTE y, x;
int indir1, indir2;

if(check_mem(4)) return(_E_FMEM);
if(sgte_campo()) return(_E_FCAM);
if((indir1=coge_num2(&y,l))==2) return(_E_CNUM);
if(!indir1 && y>FILAS-1) return(_E_EFIL);
if(sgte_campo()) return(_E_FCAM);
if((indir2=coge_num2(&x,l))==2) return(_E_CNUM);
if(!indir2 && x>COLUMNAS-1) return(_E_ECOL);
if(indir1 && !indir2) *proc_act++=INDIR1;
else if(!indir1 && indir2) *proc_act++=INDIR2;
else if(indir1 && indir2) *proc_act++=INDIR12;
*proc_act++=92;
*proc_act++=y;
*proc_act++=x;
return(_E_NERR);
}

int saveat(void)
{
if(check_mem(1)) return(_E_FMEM);
*proc_act++=93;
return(_E_NERR);
}

int backat(void)
{
if(check_mem(1)) return(_E_FMEM);
*proc_act++=94;
return(_E_NERR);
}

int newtext(void)
{
if(check_mem(1)) return(_E_FMEM);
*proc_act++=95;
return(_E_NERR);
}

int printc(void)
{
BYTE car;
int indir;

if(check_mem(3)) return(_E_FMEM);
if(sgte_campo()) return(_E_FCAM);
if((indir=coge_num2(&car,l))==2) return(_E_CNUM);
if(indir) *proc_act++=INDIR1;
*proc_act++=96;
*proc_act++=car;
return(_E_NERR);
}

int ink(void)
{
BYTE color;
int indir;

if(check_mem(3)) return(_E_FMEM);
if(sgte_campo()) return(_E_FCAM);
if((indir=coge_num2(&color,l))==2) return(_E_CNUM);
if(!indir && (color>7)) return(_E_COLR);
if(indir) *proc_act++=INDIR1;
*proc_act++=97;
*proc_act++=color;
return(_E_NERR);
}

int paper(void)
{
BYTE color;
int indir;

if(check_mem(3)) return(_E_FMEM);
if(sgte_campo()) return(_E_FCAM);
if((indir=coge_num2(&color,l))==2) return(_E_CNUM);
if(!indir && (color>7)) return(_E_COLR);
if(indir) *proc_act++=INDIR1;
*proc_act++=98;
*proc_act++=color;
return(_E_NERR);
}

int bright(void)
{
BYTE b;
int indir;

if(check_mem(3)) return(_E_FMEM);
if(sgte_campo()) return(_E_FCAM);
if((indir=coge_num2(&b,l))==2) return(_E_CNUM);
if(!indir && (b>1)) return(_E_COLR);
if(indir) *proc_act++=INDIR1;
*proc_act++=99;
*proc_act++=b;
return(_E_NERR);
}

int blink(void)
{
BYTE b;
int indir;

if(check_mem(3)) return(_E_FMEM);
if(sgte_campo()) return(_E_FCAM);
if((indir=coge_num2(&b,l))==2) return(_E_CNUM);
if(!indir && (b>1)) return(_E_COLR);
if(indir) *proc_act++=INDIR1;
*proc_act++=100;
*proc_act++=b;
return(_E_NERR);
}

int color(void)
{
BYTE col;
int indir;

if(check_mem(3)) return(_E_FMEM);
if(sgte_campo()) return(_E_FCAM);
if((indir=coge_num2(&col,l))==2) return(_E_CNUM);
if(indir) *proc_act++=INDIR1;
*proc_act++=101;
*proc_act++=col;
return(_E_NERR);
}

