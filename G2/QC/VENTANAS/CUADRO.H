/***************************************
	Fichero de cabecera de
	las funciones de manejo
	de cuadros de di logo
	de CUADRO.C
***************************************/

#if !defined (CUADRO_H)
#define CUADRO_H

/*** Constantes ***/
#define C_MAXELEM       10      /* m ximo n§ de elementos en un cuadro */
#define C_CENT          9999    /* valor para centrar el cuadro */
#define C_CARTECLA      '^'     /* car cter que indica tecla asociada */
#define C_FLECHARR      0x18    /* car cter flecha arriba */
#define C_FLECHABJ      0x19    /* car cter flecha abajo */
#define C_CARRELL1      0xb1    /* car cter de relleno 1 */
#define C_CARRELL2      0xfe    /* car cter de relleno 2 */
#define C_FICH_ANCHO    40      /* anchura cuadro selecci¢n de ficheros */
#define C_FICH_ALTO     15      /* altura cuadro selecci¢n de ficheros */
#define C_CHRDIR1       0x11    /* car. 1 para marcar nombres directorios */
#define C_CHRDIR2       0x10    /* car. 2 para marcar nombres directorios */
#define C_PAUSA         150     /* pausa para pulsaciones de rat¢n */

/* caracteres para cajas de comprobaci¢n */
#define CCHECK_BOR1     '['     /* lateral izquierdo para caja */
#define CCHECK_BOR2     ']'     /* lateral derecho para caja */
#define CCHECK_MARC     0x07    /* marca para caja actual */
#define CCHECK_ON       'X'     /* indicador de caja activa */
#define CCHECK_OFF      ' '     /* indicador de caja desactivada */

/* c¢digos de tipos de elementos de los cuadros de di logo */
#define C_ELEM_NULO     0       /* elemento vac¡o */
#define C_ELEM_BOTON    1       /* bot¢n */
#define C_ELEM_INPUT    2       /* ventana de entrada */
#define C_ELEM_LISTA    3       /* lista de cadenas de caracteres */
#define C_ELEM_CHECK    4       /* caja de comprobaci¢n */
#define C_ELEM_TEXTO    5       /* texto */

/* c¢digos de acciones para cuadros de di logo */
#define CUADRO_NULA      -1     /* acci¢n nula */
#define CUADRO_SALIDA    -2     /* salida de un cuadro (ESCAPE) */
#define CUADRO_SELECC    -3     /* selecci¢n del elemento actual */
#define CUADRO_SGTE_ELEM -4     /* siguiente elemento */
#define CUADRO_ANT_ELEM  -5     /* elemento anterior */
#define CUADRO_TECLA     -6     /* env¡a tecla */
#define CUADRO_RATON     -7     /* env¡a acci¢n de rat¢n */

/* modos de funcionamiento de elemento de lista */
#define C_LSTNORMAL     0       /* normal */
#define C_LSTPRIMERO    1       /* elemento seleccionado siempre primero */
/* modo de ordenaci¢n de elemento de lista */
#define C_LSTSINORDEN   0       /* lista desordenada */
#define C_LSTORDENADA   1       /* lista ordenada */

/* modos de presentaci¢n de elemento de texto */
#define C_TXTLINEA      0       /* cada l¡nea por separado */
#define C_TXTPASA       1       /* pasa l¡nea a siguiente si no cabe */
#define C_TXTNOBORDE    0       /* no imprime borde alrededor de texto */
#define C_TXTBORDE      1       /* imprime borde alrededor de texto */

/*** Tipos de datos y estructuras ***/
typedef struct {
	STC_VENTANA v;          /* ventana del elemento */
	int tipo;               /* tipo de elemento */
	void *info;             /* puntero a informaci¢n del elemento */
} STC_ELEM;

typedef struct {
	STC_VENTANA v;                  /* ventana del cuadro */
	STC_ELEM elem[C_MAXELEM];       /* elementos del cuadro */
	BYTE attr_tecla;                /* atributo de teclas de activaci¢n */
	BYTE attr_sel;                  /* atributo para elem. seleccionado */
	int elemento;                   /* elemento actual */
} STC_CUADRO;

typedef struct {
	int accion;             /* c¢digo de acci¢n */
	char tecla;             /* tecla pulsada (CUADRO_TECLA) */
	short fil, col;         /* posici¢n del rat¢n (CUADRO_RATON) */
} STC_ACCION_C;

/* elemento bot¢n */
typedef struct {
	char *cadena;           /* texto bot¢n, C_CARTECLA marca tecla asoc. */
	char tecla;             /* tecla asociada */
} STC_ELEM_BOTON;

/* elemento INPUT */
typedef struct {
	char *texto;            /* texto, C_CARTECLA marca tecla asociada */
	char tecla;             /* tecla asociada */
	char *cadena;           /* puntero a buffer para la cadena a teclear */
	int longitud;           /* m x. longitud de cadena (sin contar '\0') */
} STC_ELEM_INPUT;

/* elemento lista */
typedef struct _lista {
	char *cadena;           /* puntero a cadena de caracteres */
	struct _lista *sgte;    /* puntero a siguiente elemento */
} STC_LISTA;
typedef struct {
	char *texto;            /* texto, C_CARTECLA marca tecla asociada */
	int modo;               /* modo de desplazamiento de la lista */
				/* C_LSTNORMAL=normal */
				/* C_LSTPRIMERO=seleccionado siempre primero */
	int orden;              /* modo de ordenaci¢n de la lista */
				/* C_LSTSINORDEN=desordenada */
				/* C_LSTORDENADA=ordenada */
	char tecla;             /* tecla asociada */
	STC_LISTA *elemento;    /* puntero a primer elemento de la lista */
	char *selecc;           /* puntero a cadena de elem. seleccionado */
	int num_elementos;      /* n£mero de elementos en lista */
	int elemento_pr;        /* primer elemento a mostrar */
	int elemento_sel;       /* n§ de elemento seleccionado */
} STC_ELEM_LISTA;

/* elemento caja de comprobaci¢n */
typedef struct {
	char *texto;            /* texto, C_CARTECLA marca tecla asociada */
	char tecla;             /* tecla asociada */
	int estado;             /* estado; 1 activo, 0 inactivo */
} STC_ELEM_CHECK;

/* elemento texto */
typedef struct {
	char *texto;            /* texto */
	int modo;               /* modo de impresi¢n */
				/* C_TXTLINEA=cada l¡nea por separado */
				/* C_TXTPASA=pasa l¡nea a sgte. si no cabe */
	int borde;              /* modo de impresi¢n de borde */
				/* C_TXTNOBORDE=no imprime borde */
				/* C_TXTBORDE=borde alrededor de texto */
} STC_ELEM_TEXTO;

/*** Prototipos ***/
void c_crea_cuadro(STC_CUADRO *cuad, char *titulo, short fil, short col,
  short ancho, short alto, BYTE attr_princ, BYTE attr_s1, BYTE attr_s2,
  BYTE attr_tecla, BYTE attr_sel);
int c_crea_elemento(STC_CUADRO *cuad, int elem, short fil, short col,
  short ancho, short alto, BYTE attr_princ, BYTE attr_s1, BYTE attr_s2,
  int tipo, void *info);
int c_dibuja_elemento(STC_CUADRO *cuad, int elem);
void c_abre(STC_CUADRO *cuad);
void c_cierra(STC_CUADRO *cuad);
int c_lee_input(STC_VENTANA *v, char *cadena, int longitud);
int c_accion(STC_CUADRO *c, STC_ACCION_C *acc);
int c_gestiona(STC_CUADRO *c);
int c_elem_input(STC_VENTANA *v, STC_ELEM_INPUT *e);
int c_elem_lista(STC_VENTANA *v, STC_ELEM_LISTA *e, BYTE attr_tecla,
  BYTE attr_sel);
int c_mete_en_lista(STC_ELEM_LISTA *e, char *cadena);
void c_borra_lista(STC_ELEM_LISTA *e);
int c_elem_check(STC_VENTANA *v, STC_ELEM_CHECK *e, STC_CUADRO *cuad,
  int elem);
void c_selecc_ficheros(short fil, short col, char *titulo, BYTE attr_princ,
  BYTE attr_s1, BYTE attr_s2, BYTE attr_tecla, BYTE attr_sel, char *ruta,
  char *mascara, char *fichero);

#endif  /* CUADRO_H */

