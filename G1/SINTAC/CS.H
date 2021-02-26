/*****************************************
	Fichero de cabecera principal
	del compilador SINTAC.

	Prototipos de funciones,
	constantes y variables globales
	usadas por el compilador.
*****************************************/

/*** Constantes ***/
#define LIN_WCOMP       1               /* l¡nea pantalla de ventana compil. */
#define ALT_WCOMP       14              /* altura de ventana de compilador */
#define LIN_WERR        (LIN_WCOMP+ALT_WCOMP)   /* l¡nea de ventana errores */
#define ALT_WERR        25-LIN_WERR     /* altura de ventana de errores */
#define MARCA_C         (char)'®'       /* marca la secci¢n que se compila */
#define COL_ERROR       62              /* columna para contador errores */
#define FIL_ERROR       4               /* fila para contador errores */

/* constantes para tipo_lin() */
#define L_COMENT	0
#define L_CONST         1
#define L_MARCA         2
#define L_NULA          3
#define L_NORM          4

/*** Variables globales ***/
/* identificadores de las secciones */
/* NOTA: el orden viene determinado por las constantes */
/* definidas en SINTAC.H */
char id_secc[N_SECCS][4]={"VOC", "MSY", "MSG", "LOC", "OBJ", "PRO"};
/* nombres de las secciones */
char *nomb_secc[N_SECCS]={
	"Vocabulario",
	"Mensajes del sistema",
	"Mensajes",
	"Localidades",
	"Objetos",
	"Procesos",
};

/* mensajes de error */
/* NOTA: el tipo de dato COD_ERR definido en CSERR.H es una */
/* enumeraci¢n con todos los c¢digos de error */
const char *men_error[]={
	"",                                             /* no error */
	"error de indirecci¢n",                         /* indirecci¢n */
	"TABLA DE CONDACTOS ERRONEA",
	"error de apertura de fichero temporal",
	"error de escritura en fichero temporal",
	"error de lectura de fichero temporal",
	"error de apertura de fichero de entrada",
	"error de lectura en fichero de entrada",
	"fin de fichero de entrada",
	"error de apertura de fichero de salida",
	"error de escritura en fichero de salida",
	"no hay suficiente memoria para ejecutar el programa",
	"secci¢n no v lida",
	"secci¢n repetida",
	"marca de secci¢n no encontrada",
	"marca de fin de secci¢n (\\END) no encontrada",
	"faltan secciones",
	"car cter no v lido en palabra de vocabulario",
	"faltan campos",
	"n£mero de palabra no v lido",
	"campo num‚rico no v lido",
	"tipo de palabra no v lido",
	"palabra repetida",
	"vocabulario lleno",
	"falta \'@\'",
	"n£mero fuera de secuencia",
	"se rebas¢ la memoria reservada para la secci¢n actual",
	"n£mero no v lido",
	"palabra no est  en vocabulario",
	"nombre no est  en vocabulario",
	"adjetivo no est  en vocabulario",
	"la palabra no es verbo de movimiento",
	"el primer campo debe ser un Nombre",
	"el segundo campo debe ser un Adjetivo",
	"localidad inicial no v lida",
	"banderas de objeto no v lidas",
	"falta texto de Objeto",
	"comienzo de entrada no v lido",
	"tabla de etiquetas llena",
	"etiqueta repetida",
	"nombre de etiqueta no v lido",
	"primera palabra en entrada no es verbo ni nombre convertible",
	"segunda palabra en entrada no es nombre",
	"condacto no v lido",
	"un proceso no puede llamarse a s¡ mismo",
	"rebasado m ximo n£mero de llamadas 'forward' a procesos",
	"n£mero de ventana no v lido",
	"n£mero de mensaje no v lido",
	"n£mero de localidad no v lido",
	"n£mero de objeto no v lido",
	"campo de etiqueta no v lido",
	"rebasado m ximo n£mero de saltos 'forward' con SKIP",
	"salto fuera de rango",
	"adjetivo no v lido",
	"nombre no v lido",
	"bandera de usuario no v lida",
	"par metro de CHANCE debe estar entre 0 y 100",
	"banco de memoria no v lido",
	"n£mero de primer mensaje del sistema no v lido",
	"n£mero de segundo mensaje del sistema no v lido",
	"par metro debe ser 0 o 1",
	"n£mero de color incorrecto",
	"conexi¢n a localidad no v lida",
	"etiqueta no definida",
	"salto 'forward' fuera de rango",
	"llamada a proceso inexistente",
	"tabla de constantes llena",
	"primer car cter de nombre de constante no v lido",
	"falta valor de la constante",
	"constante no definida",
	"nombre de constante no v lido",
	"error en valor de constante",
	"n£mero de borde de ventana no v lido",
	"segundo par metro debe ser 0 o 1",
	"componente RGB debe estar entre 0 y 63",
};

/*** Prototipos de funciones de CS.C **/
void _far int24_hnd(unsigned deverror, unsigned errcode,
  unsigned _far *devhdr);
void fin_prg(int codigo);
void imp_error(unsigned cod_err, unsigned long nlin);
int may_str(char *cadena);
int analiza_args(int argc, char *argv[]);
void input_nsal(char *nin, char *nout);
char *lee_linea(FILE *f_in, char *lin, unsigned long *nlin);
int tipo_lin(char *lin);
COD_ERR chequea_conx(BYTE num_loc, BYTE *conx, unsigned *desp_conx,
  BYTE *err_conx, int *num_conx);
void codifica(BYTE *mem, unsigned bytes_mem);
void borra_scr(BYTE color);
void pon_cursor(BYTE fil, BYTE col);
void lee_config(void);
