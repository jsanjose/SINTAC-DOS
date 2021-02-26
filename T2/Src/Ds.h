/*****************************************
	Fichero de cabecera principal
	del int‚rprete-debugger SINTAC.

	Prototipos de funciones,
	constantes y variables globales
	usadas por el int‚rprete-debugger.
*****************************************/

/*** Constantes ***/
#define OFF 0
#define ON  1

#define VARS        256         /* n£mero de variables */
#define BANDS       32          /* n£mero de banderas/8 */

#define MAXLONG     128         /* m xima longitud de la l¡nea de entrada */
#define STK         100         /* profundidad de la pila del int‚rprete */

#define SEPARADOR   0
#define FIN_FRASE   1
#define PALABRA     2
#define TERMINACION 3
#define NO_PALABRA  4

/*** Variables globales ***/
const char CAR_PAL[]="ABCDEFGHIJKLMN¥OPQRSTUVWXYZ0123456789";
const char C_NO_SIG[]=" !­?¨";      /* caracteres no significativos */
const char C_SEPAR[]=".,;:\"\'";    /* separadores */

/*** Prototipos de funciones de DS.C ***/
void _far int24_hnd(unsigned deverror, unsigned errcode,
  unsigned _far *devhdr);

#if DEBUGGER==ON
int saca_pal(BYTE num_pal,BYTE tipo_pal);
void imp_condacto(BOOLEAN indir1, BOOLEAN indir2, BYTE *pro_d);
BYTE inp_band(void);
void guarda_screen(void);
void recupera_screen(void);
void imp_varband(BYTE variable, BYTE bandera);
void imp_debugger(BOOLEAN indir1, BOOLEAN indir2, BYTE *pro_d, BYTE variable,
  BYTE bandera, char *txt_deb);
void debugger(BOOLEAN indir1, BOOLEAN indir2, BYTE *pro_d);
#endif

void modo_video(void);
void w_imps2(char *s, STC_WINDOW *w);
BOOLEAN esta_en(const char *s, char c);
void pausa(clock_t wait);
void load_db(char *nombre);
void codifica(BYTE *mem, unsigned bytes_mem);
void inic(void);
void frd(void *buff, size_t tam, size_t cant);
int analiza(char *(*pfrase), BYTE *tipo, BYTE *num);
char mayuscula(char c);
int esta_en_voc(char *pal);
BOOLEAN parse1(void);
void m_err(BYTE x, char *m, int flag);
int carga_def(char *nombre);

BOOLEAN process(BYTE prc);
BOOLEAN done(void);
BOOLEAN notdone(void);
BOOLEAN resp(void);
BOOLEAN noresp(void);
BOOLEAN defwin(BYTE nw, BYTE cw, BYTE wx, BYTE wy, BYTE lx, BYTE ly);
BOOLEAN window(BYTE nw);
BOOLEAN clw(BYTE nw);
BOOLEAN let(BYTE nv,BYTE val);
BOOLEAN eq(BYTE nv, BYTE val);
BOOLEAN noteq(BYTE nv, BYTE val);
BOOLEAN lt(BYTE nv, BYTE val);
BOOLEAN gt(BYTE nv, BYTE val);
BOOLEAN mes(BYTE nm);
BOOLEAN newline(void);
BOOLEAN message(BYTE nm);
BOOLEAN sysmess(BYTE nm);
BOOLEAN desc(BYTE nl);
BOOLEAN add(BYTE nv, BYTE val);
BOOLEAN sub(BYTE nv, BYTE val);
BOOLEAN inc(BYTE nv);
BOOLEAN dec(BYTE nv);
BOOLEAN set(BYTE nf);
BOOLEAN clear(BYTE nf);
BOOLEAN zero(BYTE nf);
BOOLEAN notzero(BYTE nf);
BOOLEAN place(BYTE nobj, BYTE nloc);
BOOLEAN get(BYTE nobj);
BOOLEAN drop(BYTE nobj);
BOOLEAN input(void);
BOOLEAN parse(void);
BOOLEAN skip(BYTE lsb, BYTE hsb);
BOOLEAN at(BYTE locno);
BOOLEAN notat(BYTE locno);
BOOLEAN atgt(BYTE locno);
BOOLEAN atlt(BYTE locno);
BOOLEAN adject1(BYTE adj);
BOOLEAN noun2(BYTE nomb);
BOOLEAN adject2(BYTE adj);
BOOLEAN listat(BYTE locno);
BOOLEAN isat(BYTE nobj, BYTE locno);
BOOLEAN isnotat(BYTE nobj, BYTE locno);
BOOLEAN present(BYTE nobj);
BOOLEAN absent(BYTE nobj);
BOOLEAN worn(BYTE nobj);
BOOLEAN notworn(BYTE nobj);
BOOLEAN carried(BYTE nobj);
BOOLEAN notcarr(BYTE nobj);
BOOLEAN wear(BYTE nobj);
BOOLEAN remove1(BYTE nobj);
BOOLEAN create(BYTE nobj);
BOOLEAN destroy(BYTE nobj);
BOOLEAN swap(BYTE nobj1, BYTE nobj2);
BOOLEAN restart(void);
BOOLEAN whato(void);
BOOLEAN move(BYTE nv);
BOOLEAN ismov(void);
BOOLEAN goto1(BYTE locno);
BOOLEAN print(BYTE nv);
BOOLEAN dprint(BYTE nv);
BOOLEAN cls(void);
BOOLEAN anykey(void);
BOOLEAN pause(BYTE pau);
BOOLEAN listobj(void);
BOOLEAN firsto(void);
BOOLEAN nexto(BYTE locno);
BOOLEAN synonym(BYTE verb, BYTE nomb);
BOOLEAN hasat(BYTE val);
BOOLEAN hasnat(BYTE val);
BOOLEAN light(void);
BOOLEAN nolight(void);
BOOLEAN random(BYTE varno, BYTE rnd);
BOOLEAN seed(BYTE seed);
BOOLEAN puto(BYTE nloc);
BOOLEAN inkey(void);
BOOLEAN copyov(BYTE nobj, BYTE varno);
BOOLEAN chance(BYTE rnd);
BOOLEAN ramsave(BYTE banco);
BOOLEAN ramload(BYTE banco, BYTE vtop, BYTE ftop);
BOOLEAN ability(BYTE nobjs);
BOOLEAN autog(void);
BOOLEAN autod(void);
BOOLEAN autow(void);
BOOLEAN autor(void);
BOOLEAN isdoall(void);
BOOLEAN ask(BYTE smess1, BYTE smess2, BYTE varno);
BOOLEAN quit(void);
BOOLEAN save(void);
BOOLEAN load(BYTE vtop, BYTE ftop);
BOOLEAN exit1(BYTE ex);
BOOLEAN end1(void);
BOOLEAN printat(BYTE y, BYTE x);
BOOLEAN saveat(void);
BOOLEAN backat(void);
BOOLEAN newtext(void);
BOOLEAN printc(BYTE car);
BOOLEAN ink(BYTE color);
BOOLEAN paper(BYTE color);
BOOLEAN bright(BYTE b);
BOOLEAN blink(BYTE b);
BOOLEAN color(BYTE col);
BOOLEAN debug(BYTE modo);
BOOLEAN wborder(BYTE nw, BYTE borde);
BOOLEAN charset(BYTE set);
BOOLEAN extern1(BYTE prg, BYTE par);
