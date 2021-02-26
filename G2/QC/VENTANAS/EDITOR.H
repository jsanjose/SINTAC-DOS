/***************************************
	Fichero de cabecera de
	las funciones de manejo
	de editor de texto
	de EDITOR.C
***************************************/

#if !defined (EDITOR_H)
#define EDITOR_H

/*** Constantes ***/
#define E_MAXNUMLIN  5000       /* m ximo n£mero de l¡neas del editor */
#define E_MAXLNGLIN  256        /* m xima longitud de una l¡nea */
#define RETURN       0x0d       /* c¢digo ASCII de RETURN */
#define TAB          0x09       /* c¢digo ASCII de tabulaci¢n */
#define E_FLECHARR   0x18       /* car cter de flecha arriba */
#define E_FLECHABJ   0x19       /* car cter de flecha abajo */
#define E_FLECHIZQ   0x1b       /* car cter de flecha izquierda */
#define E_FLECHDER   0x1a       /* car cter de flecha derecha */
#define E_CARRELL1   0xb2       /* car cter de relleno 1 */
#define E_CARRELL2   0xfe       /* car cter de relleno 2 */
#define E_CHR1F      '['        /* 1er car. de zona nombre de fichero */
#define E_CHR2F      ']'        /* £ltimo car. de zona nombre de fichero */
#define E_CHRMOD     0x0f       /* car cter de texto modificado */
#define E_CHRNOMOD   ' '        /* car cter de texto no modificado */
#define E_EDITA      0          /* se permite editar el texto */
#define E_SOLOLECT   1          /* s¢lo se permite leer el texto */
#define E_PAUSA1     200        /* pausa para primera pulsaci¢n rat¢n */
#define E_PAUSA2     50         /* pausa para sucesivas pulsaciones rat¢n */

#define CB_FIL       7          /* fila del cuadro de b£squeda */
#define CB_COL       14         /* columna del cuadro de b£squeda */
#define CB_ANCHO     50         /* anchura del cuadro de b£squeda */
#define CB_ALTO      8          /* altura del cuadro de b£squeda */

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
#define TSUP         0x53       /* borrar */
#define TINS         0x52       /* INSERT */
#define TCTR_INS     0x92       /* CTRL + INSERT */
#define TCTR_SUP     0x93       /* CTRL + borrar */
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
#define TSF1         0x54       /* Shift+F1 */

/*** Tipos de datos y estructuras ***/
#if !defined (_U_CHAR_BYTE)
typedef unsigned char BYTE;
#define _U_CHAR_BYTE
#endif

/* estructura de l¡neas de bloque de texto */
typedef struct _bloque {
	char *lin;              /* puntero a texto de l¡nea actual */
	struct _bloque *sgte;   /* puntero a siguiente l¡nea */
} STC_BLOQUE;

/*** Prototipos ***/
void e_inicializa(char *fich, short fil, short col, short ancho, short alto,
  BYTE attr_princ, BYTE attr_s1, BYTE attr_s2, BYTE attr_blq, BYTE attr_princc,
  BYTE attr_s1c, BYTE attr_s2c, BYTE attr_teclac, BYTE attr_selc, int modo_ed);
void e_borra_texto(void);
int e_editor(void);
void e_dibuja_editor(void);
int e_carga_texto(char *fich);
int e_graba_texto(char *fich);
void e_nombre_fichero(char *nf);
int e_inicia_busqueda(void);
int e_continua_busqueda(void);
int e_modificado(void);
void e_cambia_modo(int modo_ed);
int e_carga_textox(char *fich, long desplz);
void e_vacia_bloque(void);
int e_graba_bloque(char *nfich);
int e_inserta_bloque(void);
void e_pon_cursor(int lin, int col);
char *e_palabra_cursor(void);

#endif  /* EDITOR_H */
