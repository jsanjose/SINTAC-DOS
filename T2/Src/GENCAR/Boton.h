/************************************
	Fichero de cabecera de
	las funciones de manejo
	de botones de BOTON.C
************************************/

#if !defined (BOTON_H)
#define BOTON_H

/*** Constantes ***/
#define MAX_BOT         16      /* m ximo n£mero de botones en un men£ */
#define LNG_TXT_BOT     21      /* m xima longitud + 1 del texto de un bot¢n */
#define ON              1       /* bot¢n se dibuja sin pulsar */
#define OFF             0       /* bot¢n se dibuja pulsado */
#define ACT             1       /* bot¢n activado (puede pulsarse) */
#define NO_ACT          0       /* bot¢n desactivado */
#define IGUAL           -1      /* para dejar modo o tipo sin modificar */
#define INV             -2      /* para invertir modo o tipo */
#define BBOTON_INDF     0       /* barra de botones indefinida */
#define BBOTON_HORZ     1       /* barra de botones horizontal */
#define BBOTON_VERT     2       /* barra de botones vertical */
#define TAMTXT_PEQ      1       /* tama¤o de texto de bot¢n peque¤o */
#define TAMTXT_MED      2       /* tama¤o de texto de bot¢n mediano */
#define TAMTXT_GRA      3       /* tama¤o de texto de bot¢n grande */
#define LONG_MAX_INPUT  200     /* m ximo n£m. de caracteres en input */

/*** Tipos de datos y estructuras ***/
/* definici¢n de un bot¢n */
typedef struct {
	char texto_bot[LNG_TXT_BOT];    /* texto del bot¢n */
	int tam_texto;          /* tama¤o del texto */
	int modo;               /* modo dibujo (ON=pulsado, OFF=sin pulsar) */
	int tipo;               /* NO_ACT=inactivo, ACT=activo */
	short xb, yb;           /* coordenadas esquina superior izquierda */
	short ancho, alto;      /* dimensiones del bot¢n */
	unsigned tecla;         /* tecla de activaci¢n */
} STC_BOTON;

/* definici¢n de una barra de botones */
typedef struct {
	short x, y;             /* coordenadas de la barra */
	int tipo;               /* tipo de la barra de botones */
	short color_fondo;      /* ¡ndice de color de fondo */
	short color_pplano;     /* ¡ndice de color de texto bot¢n */
	short color_s1;         /* color sombra d‚bil */
	short color_s2;         /* color sombra fuerte */
	short ancho_bot;        /* anchura de los botones */
	short alto_bot;         /* altura de los botones */
	int num_botones;        /* n£mero de botones en la barra */
	STC_BOTON bot[MAX_BOT]; /* datos sobre los botones */
} STC_BBOTON;

/* c¢digo de identificaci¢n para un bot¢n */
typedef struct {
	int boton;              /* n£mero de bot¢n */
	int barra;              /* n£mero de barra a la que pertenece */
	short x, y;             /* coordenada relativas a origen bot¢n */
} STC_CODBOTON;

/*** Prototipos ***/
void dibuja_cuadro_boton(short x0, short y0, short ancho, short alto,
  short colorf, short colors1, short colors2, int modo, int fondo);
void escribe_texto_boton(char *texto, STC_BOTON *boton, short colorp,
  short colort);
void dibuja_boton(STC_BOTON *boton, short colorf, short colorp, short colors1,
  short colors2);
void crea_barra_boton(STC_BBOTON *bb);
void dibuja_barra_boton(STC_BBOTON *bb);
STC_CODBOTON pulsa_boton(short x, short y, STC_BBOTON *barras,
  int num_barras);
void cambia_boton(STC_BBOTON *bb, int boton, int modo, int tipo);
STC_CODBOTON esta_pulsado(STC_BBOTON *barras, int nbarr);
int lee_input(short x, short y, char *cadena, int longitud, short color,
  short colorf);

#endif  /* BOTON_H */

