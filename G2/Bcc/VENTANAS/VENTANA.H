/***************************************
	Fichero de cabecera de
	las funciones de manejo
	de ventanas de VENTANA.C
***************************************/

#if !defined (VENTANA_H)
#define VENTANA_H

/*** Constantes ***/
#define V_LINEA_LINEA   0       /* imprimir l¡nea a l¡nea */
#define V_PASA_LINEA    1       /* pasa de una l¡nea a otra */
#define V_NORELLENA     0       /* no rellena con espacios imprimir cadena */
#define V_RELLENA       1       /* rellena con espacios al imprimir cadena */

/*** Tipos de datos y estructuras ***/
#if !defined (_U_CHAR_BYTE)
typedef unsigned char BYTE;
#define _U_CHAR_BYTE
#endif

typedef struct {
	short fil, col;         /* posici¢n de la ventana en pantalla */
	short ancho, alto;      /* dimensiones de la ventana */
	BYTE attr_princ;        /* atributo principal de la ventana */
	BYTE attr_s1;           /* atributo para sombra 1 */
	BYTE attr_s2;           /* atributo para sombra 2 */
	char *borde;            /* puntero a cadena con caracteres de borde */
	char *titulo;           /* puntero a cadena con texto encabezamiento */
	BYTE far *fondo;        /* puntero a buffer donde guardar fondo */
	int modo_texto;         /* modo de impresi¢n de texto */
	short filc, colc;       /* posici¢n de impresi¢n dentro de ventana */
	BYTE attr_texto;        /* color del texto de la ventana */
} STC_VENTANA;

/*** Prototipos ***/
void v_impcar(short fil, short col, BYTE car, BYTE attr);
void v_crea(STC_VENTANA *ventana, short fil, short col, short ancho,
  short alto, BYTE attr_princ, BYTE attr_s1, BYTE attr_s2, char *titulo);
void v_dibuja(STC_VENTANA *ventana, int rellena);
void v_abre(STC_VENTANA *ventana);
void v_cierra(STC_VENTANA *ventana);
void v_pon_cursor(STC_VENTANA *ventana, short fil, short col);
void v_impc(STC_VENTANA *ventana, char car);
void v_impcad(STC_VENTANA *ventana, char *cad, int rellena);
void v_modo_texto(STC_VENTANA *ventana, int modo);
void v_borra(STC_VENTANA *ventana);
void v_color(STC_VENTANA *ventana, BYTE attr);
void v_borde(STC_VENTANA *ventana, char *borde);
void v_scroll_arr(STC_VENTANA *ventana, int n);
void v_scroll_abj(STC_VENTANA *ventana, int n);
void v_scroll_izq(STC_VENTANA *ventana, int n);
void v_scroll_der(STC_VENTANA *ventana, int n);

#endif  /* VENTANA_H */
