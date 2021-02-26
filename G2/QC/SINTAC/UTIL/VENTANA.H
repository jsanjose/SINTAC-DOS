/************************************
	Fichero de cabecera de
	las funciones de manejo
	de ventanas de VENTANA.C
************************************/

#if !defined (VENTANA_H)
#define VENTANA_H

/*** Constantes ***/
#define V_CENT      9999        /* para centrar la ventana en X o en Y */

/* constantes para cuadro de selecci¢n de ficheros */
#define LNG_NFICH   13          /* m x. longitud + 1 de nombre de fichero */
#define CFICH_NBOT  11          /* n£mero de botones de cuadro sel. fich. */
#define CFICH_ALTB  20          /* altura de botones de cuadro sel. fich. */
#define CFICH_ANCHB 60          /* anchura de botones de cuadro sel. fich. */
#define CFICH_ALTC  120         /* altura de zona para lista de ficheros */
#define CFICH_ANCHC 176         /* anchura de zona para lista de ficheros */
#define CFICH_MARG  7           /* m rgenes del cuadro de selecc. de fich. */
#define CFICH_SEPAR 2           /* separaci¢n entre botones */
#define CFICH_SEPLF 10          /* separaci¢n entre elementos lista fich. */
/* anchura zona entrada de nombre fichero y ruta */
#define CFICH_ANCHE CFICH_ANCHC+(CFICH_SEPAR*2)+(CFICH_ANCHB/3)
/* dimensiones de cuadro de selecci¢n de ficheros */
#define CFICH_ANCHO (CFICH_MARG*2)+CFICH_ANCHB+CFICH_ANCHE
#define CFICH_ALTO  (CFICH_MARG*2)+(CFICH_SEPAR*2)+(CFICH_ALTB*2)+CFICH_ALTC

#define CF_ACCION   1           /* si se puls¢ en bot¢n de acci¢n */
#define CF_SALIDA   2           /* si se puls¢ en bot¢n de salida */

#define CF_NELEM    11          /* n§ de elementos que caben en lista fich. */
#define CF_CHRDIR1  '['         /* car cter inicio de nombre de directorio */
#define CF_CHRDIR2  ']'         /* car cter fin de nombre de directorio */
#define CF_STRDIR2  "]"         /* ¡dem pero en cadena para strcat() */

#define AV_ALTOBOT  16          /* altura bot¢n cuadro aviso */
#define AV_ANCHOBOT 48          /* anchura bot¢n cuadro aviso */

/* valores devueltos por la funci¢n cuadro_siono() */
#define SIONO_SI    1
#define SIONO_NO    2

/*** Tipos de datos y estructuras ***/
/* definici¢n de una ventana */
typedef struct {
	short x, y;             /* origen de la ventana */
				/* poner V_CENT en X o Y para centrar */
	short ancho, alto;      /* dimensiones (pixels) de la ventana */
	short color_fondo;      /* color de fondo */
	short color_pplano;     /* color de los textos */
	short color_s1;         /* color de sombra d‚bil */
	short color_s2;         /* color de sombra fuerte */
	char _huge *buff;       /* puntero a buffer para guardar fondo */
} STC_VENTANA;

/* definici¢n de un cuadro de selecci¢n de ficheros */
typedef struct {
	STC_VENTANA v;          /* ventana que contendr  al cuadro */
	STC_BBOTON b;           /* barra de botones del cuadro */
} STC_CUADROFICH;

/* entrada de lista de ficheros */
typedef struct _lista_ficheros {
	char fich[LNG_NFICH+2];                 /* nombre de fichero */
	struct _lista_ficheros *sgte_fich;      /* ptro. a sgte. entrada */
} STC_LISTA_FICH;

/*** Prototipos ***/
int abre_ventana(STC_VENTANA *ventana);
int cierra_ventana(STC_VENTANA *ventana);
int cuadro_fich(short x, short y, short color_fondo,short color_pplano,
  short color_s1, short color_s2, short color_tecla,char *texto,
  char *nomb_fich);
int cuadro_aviso(char *aviso, short x, short y, short color_fondo,
  short color_pplano, short color_s1, short color_s2, short color_tecla);
int cuadro_siono(char *mensaje, short x, short y, short color_fondo,
  short color_pplano, short color_s1, short color_s2, short color_tecla);

#endif  /* VENTANA_H */


