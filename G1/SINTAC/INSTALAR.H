/*****************************************
	Fichero de cabecera principal
	del programa de instalaci¢n.

	Prototipos de funciones y
	constantes.
*****************************************/

/*** Constantes ***/

#define N_OPC        6          /* n£mero de opciones de men£ */
#define LNG_OPC      40         /* longitud texto de opciones */
#define COLOR_MENUC  0x07       /* color de opciones de men£ */
#define COLOR_OPCC   0x70       /* color de opci¢n seleccionada */
#define COLOR_INPUT  0x70       /* color de ventana de INPUT */
#define COLOR_MSGI   0x03       /* color de mensajes de instalaci¢n */
#define LIN_INST     10         /* l¡nea de inicio de mensajes de instal. */

/* directorios por defecto para instalar el sistema */
#define DIRDEF_SINTAC   "C:\\SINTACG1\\"
#define DIRDEF_GENCAR   "C:\\SINTACG1\\GENCAR\\"
#define DIRDEF_DATOS    "C:\\SINTACG1\\DATOS\\"
#define DIRDEF_DOC      "C:\\SINTACG1\\MANUAL\\"

/* mensaje de inicio de la instalaci¢n */
#define MSG_INSTL       "Instalando SINTAC G1..."

/* nombre de fichero ASCII con los nombres de ficheros en los */
/* discos de instalaci¢n, as¡ como otros datos */
#define NF_DATFICH   "INSTALAR.DAT"
#define NUM_FICH     25         /* m ximo n£mero de ficheros para instalar */
#define LNG_NOMBRE   41         /* longitud m xima de nombre de fichero + 1 */

/* c¢digos de scan de teclas especiales */
#define TCUR_IZQ     0x4b       /* cursor izquierda */
#define TCUR_DER     0x4d       /* cursor derecha */
#define TCUR_ABJ     0x50       /* cursor abajo */
#define TCUR_ARR     0x48       /* cursor arriba */
#define TRETURN      0x1c       /* RETURN */

/*** Tipos de datos y estructuras ***/
typedef struct {
	char nombre[LNG_NOMBRE];        /* nombre del fichero */
	int tipo;                       /* tipo de fichero */
} STC_DATFICH;

/*** Prototipos ***/
void _far int24_hnd(unsigned deverror, unsigned errcode,
  unsigned _far *devhdr);
void inicializa_cfg(void);
void ajusta_ndir(char *dir);
void beep(void);
void pon_cursor(BYTE fil, BYTE col);
void imp_linea_scr(short fil, short col, BYTE attr, char *lin, int ncar);
void borra_linea_scr(short linea, BYTE attr);
void borra_scr(BYTE attr);
int lee_input(BYTE fil, BYTE col, char *cadena, int longitud, int ancho,
  BYTE attr);
int menu_instal(void);
int lee_datos_fich(char *nf_datfich);
int copia(char *origen, char *destino);
int instalar(int n);

