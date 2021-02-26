/*****************************************
	Fichero de cabecera principal
	del editor.

	Prototipos de funciones y
	constantes usadas por el editor.
*****************************************/

/*** Constantes ***/
#define NUM_OPC      16         /* n£mero de opciones de men£ */
#define LNG_OPC      18         /* m x. longitud de opci¢n de men£ */
#define CTECLA       (char)'&'  /* car. indicador de tecla de activaci¢n */
#define LINEAS       2000       /* m ximo n£mero de l¡neas del editor */
#define CAR_LIN      80         /* n£mero de caracteres por l¡nea */
#define LIN_SCR      2          /* l¡nea inicial de zona edici¢n */
#define NUM_LIN_SCR  18         /* n£mero de l¡neas de zona de edici¢n */
#define RETURN       0x0d       /* c¢digo ASCII de RETURN */
#define TAB          0x09       /* c¢digo ASCII de tabulaci¢n */
#define REGLETA      "ðúúúúúúúþúúúúúúúþúúúúúúúþúúúúúúúþúúúúúúúþúúúúúúú" \
		     "þúúúúúúúþúúúúúúúþúúúúúúúþúúúúúúú"
#define FBD_TEMP     "EDS$DAT.TMP"      /* fichero temporal base datos */
#define FBD_INICIO   "INICIO.BD"        /* fichero con base datos inicio */
#define FBD_SECCION  "EDS$SEC.TMP"      /* fichero temporal de secci¢n */
#define FBD_TEMP2    "EDS$AUX.TMP"      /* fichero temporal auxiliar */
#define MAX_MSGERR   50         /* m ximo n§ mensaje de error admitidos */
#define FIL_ERR      7          /* fila ventana mensajes de error */
#define COL_ERR      4          /* columna ventana mensajes de error */
#define ANCH_ERR     70         /* anchura ventana mensajes de error */
#define ALT_ERR      10         /* altura ventana mensajes de error */
#define BUFF_AYUDA   64000      /* tama¤o de buffer para texto de ayuda */
#define NF_AYUDA     "EDS.AYD"  /* nombre del fichero de texto de ayuda */
#define MAX_APARTAD  300        /* m ximo n£mero de apartados en ayuda */
#define MAXL_APTDO   21         /* m x. longitud + 1 de nombre de apartado */
#define CAY_FIN      (char)'þ'  /* car cter de fin de ayuda */
#define CAY_APTDO    (char)'¯'  /* car cter de inicio nombre apartado */
#define CAY_FINAPTDO (char)'®'  /* car cter de fin de apartado */
#define CAY_REF      (char)'ð'  /* car cter de referencia a apartado */
#define NLIN_APTDO   200        /* m ximo n£mero de l¡neas por apartado */
#define LLIN_APTDO   80         /* m x. longitud de l¡neas de un apartado */
#define FIL_AY       3          /* fila ventana de ayuda */
#define COL_AY       9          /* columna ventana de ayuda */
#define ANCH_AY      60         /* anchura ventana de ayuda */
#define ALT_AY       16         /* altura ventana de ayuda */
#define FIL_CUADROF  3          /* fila para cuadro selecci¢n ficheros */
#define COL_CUADROF  19         /* columna para cuadro selecci¢n ficheros */

/* c¢digos de scan de teclas especiales */
#define TCUR_IZQ     0x4b       /* cursor izquierda */
#define TCUR_DER     0x4d       /* cursor derecha */
#define TCUR_ABJ     0x50       /* cursor abajo */
#define TCUR_ARR     0x48       /* cursor arriba */
#define TPAG_ABJ     0x51       /* p gina abajo */
#define TPAG_ARR     0x49       /* p gina arriba */
#define TFIN         0x4f       /* fin */
#define TORG         0x47       /* origen */
#define TCTR_FIN     0x75       /* CTRL + fin */
#define TCTR_ORG     0x77       /* CTRL + origen */
#define TESC         0x01       /* ESCAPE */
#define TDEL         0x0e       /* DELETE */
#define TSUP         0x53       /* borrado */
#define TF1          0x3b       /* F1 */
#define TF2          0x3c       /* F2 */
#define TF3          0x3d       /* F3 */
#define TF4          0x3e       /* F4 */
#define TF5          0x3f       /* F5 */
#define TF6          0x40       /* F6 */
#define TF7          0x41       /* F7 */
#define TF8          0x42       /* F8 */
#define TF9          0x43       /* F9 */
#define TF10         0x44       /* F10 */

/*** Tipos de datos y estructuras ***/
/* estructura de mensaje de error del compilador */
typedef struct {
	unsigned long nlin;     /* n£mero de l¡nea del error */
	char msg[81];           /* mensaje de error */
} STC_MERRC;

/*** Prototipos ***/
void _far int24_hnd(unsigned deverror, unsigned errcode,
  unsigned _far *devhdr);
void error(char *merr, int cod);
int editor(char *fich, int linea_inic, BOOLEAN *modificado);
void inicializa_pantalla(void);
int inicializa_texto(void);
void libera_texto(void);
void borra_todo_texto(void);
int carga_fichero(char *nfich);
void ajusta_linea(char *lin);
int graba_fichero(char *nfich);
void expande_tabs(char *lin);
void imp_texto_scr(void);
int mueve_cursor_abajo(int nlin);
int mueve_cursor_arriba(int nlin);
void scroll_arr_scr(void);
void scroll_abj_scr(void);
void beep(void);
int inserta_caracter(char car);
int borra_caracter(int borra);
int inserta_linea(int linea);
int borra_linea(int linea);
int pasa_linea_a(int linea);
char mayuscula(char c);
int busca_texto(char *cad, int lin);
void cabecera(void);
void linea_estado(void);
void regleta(void);
int menu(void);
void imp_opcion_menu(char *opcion, short fil, short col, BYTE color1,
  BYTE color2);
int copia_ficherot(char *nfichorg, char *nfichdest);
int abre_fichero_bd(char *nfich);
int cierra_fichero_bd(char *nfich);
int crea_nueva_bd(char *nfich);
int nuevo_fichero_bd(char *nfich);
int separa_seccion(char *csecc);
int busca_marca(char *marca, char *l);
int edita_secc(char *msecc, int lin);
int sustituye_secc(char *msecc);
int pide_numero(char *cad);
BOOLEAN es_marca_secc(char *lin);
int edita_constantes(int lin);
int crea_proceso(int pro);
int ejecuta(char *ruta, char *prg, char **arg);
void lee_config(void);
void haz_fich_sal(char *fich, char *fsal);
void errores_compilador(void);
int busca_linea_secc(unsigned long nlin, char *secc, unsigned long *lin_secc);
int lee_ayuda(void);
int ayuda(void);

