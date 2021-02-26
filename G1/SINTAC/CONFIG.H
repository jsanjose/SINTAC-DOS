/*****************************************
	Fichero de cabecera principal
	del programa de configuraci¢n.

	Prototipos de funciones y
	constantes.
*****************************************/

/*** Constantes ***/

#define N_OPC        21         /* n£mero de opciones de men£ */
#define LNG_OPC      40         /* longitud texto de opciones */
#define COLOR_MENUC  0x07       /* color de opciones de men£ */
#define COLOR_OPCC   0x70       /* color de opci¢n seleccionada */
#define COLOR_INPUT  0x70       /* color de ventana de INPUT */
#define VCOLOR_FIL   5          /* fila ventana selecc. colores */
#define VCOLOR_COL   42         /* columna ventana selecc. colores */
#define MARCA1       0x07       /* marca 1 de selecci¢n de color */
#define MARCA2       '*'        /* marca 2 de selecci¢n de color */
#define COLOR_MARCA  0x0f       /* color de marca de selecci¢n de color */
/* texto de prueba de color */
#define TXT_VCOLOR   "  -- Prueba --  "

/* c¢digos de scan de teclas especiales */
#define TCUR_IZQ     0x4b       /* cursor izquierda */
#define TCUR_DER     0x4d       /* cursor derecha */
#define TCUR_ABJ     0x50       /* cursor abajo */
#define TCUR_ARR     0x48       /* cursor arriba */
#define TRETURN      0x1c       /* RETURN */

/*** Prototipos ***/
void _far int24_hnd(unsigned deverror, unsigned errcode,
  unsigned _far *devhdr);
void inicializa_cfg(void);
void ajusta_ndir(char *dir);
void pon_cursor(BYTE fil, BYTE col);
void imp_linea_scr(short fil, short col, BYTE attr, char *lin, int ncar);
void borra_linea_scr(short linea, BYTE attr);
void borra_scr(BYTE attr);
int lee_input(BYTE fil, BYTE col, char *cadena, int longitud, int ancho,
  BYTE attr);
int menu_cfg(void);
BYTE elige_color(BYTE fila, BYTE columna, BYTE color_act);
