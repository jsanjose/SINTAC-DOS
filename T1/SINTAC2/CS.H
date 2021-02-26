/*****************************************
	Fichero de cabecera principal
	del compilador SINTAC.

	Prototipos de funciones,
	constantes y variables globales
	usadas por el compilador.
*****************************************/

/***  Prototipos de funciones de CS.C  **/
void fin_prg(int codigo);
void imp_error(unsigned cod_err, unsigned long nlin);
int may_str(char *cadena);
void input_nsal(char *nin, char *nout);
char *lee_linea(FILE *f_in, char *buff, unsigned long *nlin);
int tipo_lin(char *lin);
COD_ERR chequea_conx(BYTE num_loc, BYTE *conx, ptrdiff_t *desp_conx,
  BYTE *err_conx);
void codifica(BYTE *mem, ptrdiff_t bytes_mem);

/*************  CONSTANTES  *************/
/* constantes para tipo_lin() */
#define L_COMENT	0
#define L_CONST         1
#define L_MARCA         2
#define L_NULA          3
#define L_NORM          4

/*********  VARIABLES GLOBALES  *********/
/* identificadores de las secciones */
/* NOTA: el orden viene determinado por las constantes */
/* definidas en SINTAC.H */
char id_secc[N_SECCS][4]={"MSY", "MSG", "VOC", "LOC", "OBJ", "PRO"};
/* nombres de las secciones */
char *nomb_secc[N_SECCS]={
	"Mensajes del sistema",
	"Mensajes",
	"Vocabulario",
	"Localidades",
	"Objetos",
	"Procesos"
};

/**********  Mensajes de error  *********/
/* NOTA: el tipo de dato COD_ERR definido en SINTAC.H */
/* es una enumeraci¢n con los c¢digos de error correspondientes */
/* a cada uno de los mensajes */
const char *men_error[]={
	"",                                             /* no error */
	"Error de indirecci¢n",                         /* indirecci¢n */
	"Error de apertura de fichero temporal",
	"Error de escritura en fichero temporal",
	"Error de lectura de fichero temporal",
	"Error de apertura de fichero de entrada",
	"Error de lectura en fichero de entrada",
	"Fin de fichero de entrada",
	"Error de apertura de fichero de salida",
	"Error de escritura en fichero de salida",
	"Secci¢n no v lida",
	"Secci¢n repetida",
	"Marca de secci¢n no encontrada",
	"Marca de fin de secci¢n (\\END) no encontrada",
	"Faltan secciones",
	"Car cter no v lido en palabra de vocabulario",
	"Faltan campos",
	"N£mero de palabra no v lido",
	"Campo num‚rico no v lido",
	"Tipo de palabra no v lido",
	"Palabra repetida",
	"Vocabulario lleno",
	"Falta \'@\'",
	"N£mero fuera de secuencia",
	"Se rebas¢ la memoria reservada para la secci¢n actual",
	"N£mero no v lido",
	"Palabra no est  en vocabulario",
	"Nombre no est  en vocabulario",
	"Adjetivo no est  en vocabulario",
	"La palabra no es verbo de movimiento",
	"El primer campo debe ser un Nombre",
	"El segundo campo debe ser un Adjetivo",
	"Localidad inicial no v lida",
	"Banderas de objeto no v lidas",
	"Falta texto de Objeto",
	"Comienzo de entrada no v lido",
	"Tabla de etiquetas llena",
	"Etiqueta repetida",
	"Nombre de etiqueta no v lido",
	"Primera palabra en entrada no es verbo ni nombre convertible",
	"Segunda palabra en entrada no es nombre",
	"Condacto no v lido",
	"Un proceso no puede llamarse a s¡ mismo",
	"Rebasado m ximo n£mero de llamadas 'forward' a procesos",
	"N£mero de ventana no v lido",
	"Columna de inicio de ventana fuera de pantalla",
	"Fila de inicio de ventana fuera de pantalla",
	"Dimensiones de ventana demasiado grandes",
	"N£mero de mensaje no v lido",
	"N£mero de localidad no v lido",
	"N£mero de objeto no v lido",
	"Campo de etiqueta no v lido",
	"Rebasado m ximo n£mero de saltos 'forward' con SKIP",
	"Salto fuera de rango",
	"Adjetivo no v lido",
	"Nombre no v lido",
	"Bandera de usuario no v lida",
	"Par metro de CHANCE debe estar entre 0 y 100",
	"Banco de memoria no v lido",
	"N£mero de primer mensaje del sistema no v lido",
	"N£mero de segundo mensaje del sistema no v lido",
	"Par metro de EXIT no v lido, debe ser 0 o 1",
	"Fila fuera de pantalla",
	"Columna fuera de pantalla",
	"N£mero de color incorrecto",
	"Conexi¢n a localidad no v lida",
	"Etiqueta no definida",
	"Salto 'forward' fuera de rango",
	"Llamada a proceso inexistente",
	"Tabla de constantes llena",
	"Primer car cter de nombre de constante no v lido",
	"Falta valor de la constante",
	"Constante no definida",
	"Nombre de constante no v lido",
	"Error en valor de constante",
};
