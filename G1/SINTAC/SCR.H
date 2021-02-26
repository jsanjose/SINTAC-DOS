/*****************************************
	Fichero de cabecera para las
	funciones de impresi¢n r pida
	en pantalla.
*****************************************/

/*** Constantes ***/
#define LNG_NFICH   13          /* m x. longitud + 1 de nombre de fichero */
#define CF_CHRDIR1  '['         /* car cter inicio de nombre de directorio */
#define CF_CHRDIR2  ']'         /* car cter fin de nombre de directorio */
#define CF_STRDIR2  "]"         /* ¡dem pero en cadena para strcat() */
#define CF_ANCHO    40          /* anchura del cuadro selecci¢n de ficheros */
#define CF_ALTO     13          /* altura del cuadro selecci¢n de ficheros */

/*** Tipos de datos y estructuras ***/
#if !defined (_U_CHAR_BYTE)
typedef unsigned char BYTE;
#define _U_CHAR_BYTE
#endif

/* entrada de lista de ficheros */
typedef struct _lista_ficheros {
	char fich[LNG_NFICH+2];                 /* nombre de fichero */
	struct _lista_ficheros *sgte_fich;      /* ptro. a sgte. entrada */
} STC_LISTA_FICH;

/*** Prototipos ***/
void pon_cursor(BYTE fil, BYTE col);
void imp_linea_scr(short fil, short col, BYTE attr, char *lin, int ncar);
void borra_linea_scr(short linea, BYTE attr);
void borra_scr(BYTE attr);
void dibuja_ventana(short fil, short col, short ancho, short alto, BYTE color);
int lee_input(BYTE fil, BYTE col, char *cadena, int longitud, int ancho,
  BYTE attr);
int cuadro_fich(short f, short c, BYTE attr1, BYTE attr2, char *ruta,
  char *mascara, char *mensaje, char *fich);

