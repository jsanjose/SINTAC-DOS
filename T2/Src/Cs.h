/*****************************************
	Fichero de cabecera principal
	del compilador SINTAC.

	Prototipos de funciones,
	constantes y variables globales
	usadas por el compilador.
*****************************************/

/*** Prototipos de funciones de CS.C **/
void fin_prg(int codigo);
void imp_error(unsigned cod_err, unsigned long nlin);
int may_str(char *cadena);
void input_nsal(char *nin, char *nout);
char *lee_linea(FILE *f_in, char *buff, unsigned long *nlin);
int tipo_lin(char *lin);
COD_ERR chequea_conx(BYTE num_loc, BYTE *conx, unsigned *desp_conx,
  BYTE *err_conx, int *num_conx);
void codifica(BYTE *mem, unsigned bytes_mem);

/*** Constantes ***/
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
char id_secc[N_SECCS][4]={"VOC", "MSG", "MSY", "LOC", "OBJ", "PRO"};
/* nombres de las secciones */
char *nomb_secc[N_SECCS]={
	"Vocabulario",
	"Mensajes",
	"Mensajes del sistema",
	"Localidades",
	"Objetos",
	"Procesos",
};

/** Mensajes de error **/
/* NOTA: el tipo de dato COD_ERR definido en SINTAC.H */
/* es una enumeraci¢n con los c¢digos de error correspondientes */
/* a cada uno de los mensajes */
const char *men_error[]={
	"",                                             /* no error */
	"error de indirecci¢n",                         /* indirecci¢n */
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
	"rebosado m ximo n£mero de llamadas 'forward' a procesos",
	"n£mero de ventana no v lido",
	"columna de inicio de ventana fuera de pantalla",
	"fila de inicio de ventana fuera de pantalla",
	"dimensiones de ventana demasiado grandes",
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
	"fila fuera de pantalla",
	"columna fuera de pantalla",
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
};
