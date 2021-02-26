/***************************************
	Fichero de cabecera de
	las funciones de manejo
	de men£s de MENU.C
***************************************/

#if !defined (MENU_H)
#define MENU_H

/*** Constantes ***/
#define MAX_NUMOPCIONES 20      /* m ximo n§ de opciones de un men£ */
#define MENU_HORZ       0       /* tipo de men£ horizontal */
#define MENU_VERT       1       /* tipo de men£ vertical */
#define MENU_FIJO       0x80    /* activa men£ fijo, que no se cierra */
#define MENU_NFIJO      0x7f    /* para comprobaciones de tipo de men£ */
#define CAR_FINOPC      ':'     /* fin de texto de opci¢n */
#define CAR_SEPAR       '|'     /* separador de opciones */
#define CAR_TECLA       '^'     /* marca tecla de activaci¢n */
#define SEPARADOR       'Ä'     /* car cter separador */

/* c¢digos de acciones para men£s */
#define MENU_NULA       -1      /* acci¢n nula */
#define MENU_SALIDA     -2      /* salida de un men£ (ESCAPE) */
#define MENU_IZQUIERDA  -3      /* movimiento de cursor izquierda */
#define MENU_DERECHA    -4      /* movimiento de cursor derecha */
#define MENU_ARRIBA     -5      /* movimiento de cursor arriba */
#define MENU_ABAJO      -6      /* movimiento de cursor abajo */
#define MENU_SELECCIONA -7      /* selecci¢n de opci¢n */
#define MENU_TECLA      -8      /* env¡a tecla */
#define MENU_RATON      -9      /* env¡a acci¢n de rat¢n */

/*** Tipos de datos y estructuras ***/
#if !defined (_U_CHAR_BYTE)
typedef unsigned char BYTE;
#define _U_CHAR_BYTE
#endif

typedef struct {
	char *opcion;           /* puntero a texto de la opci¢n */
				/* "" si £ltima opci¢n */
	char tecla;             /* c¢digo ASCII de la tecla de activaci¢n */
	short fil, col;         /* posici¢n de la opci¢n en la ventana */
	int lng_opcion;         /* longitud del texto de la opci¢n */
} STC_OPCION;

typedef struct {
	STC_VENTANA v;           /* ventana del men£ */
	STC_OPCION opc[MAX_NUMOPCIONES];        /* opciones del men£ */
	BYTE attr_tecla;        /* atributo de tecla activaci¢n */
	BYTE attr_opcion;       /* atributo de opci¢n resaltada */
	BYTE tipo;              /* tipo de men£: MENU_HORZ o MENU_VERT */
	int num_opciones;       /* n£mero de opciones */
	short separ_opc;        /* separaci¢n entre opciones */
				/* s¢lo MENU_HORZ */
	int opcion;             /* £ltima opci¢n seleccionada */
} STC_MENU;

typedef struct {
	int accion;             /* c¢digo de acci¢n */
	char tecla;             /* tecla pulsada (MENU_TECLA) */
	short fil, col;         /* posici¢n del rat¢n (MENU_RATON) */
} STC_ACCION;

/*** Prototipos ***/
STC_MENU *m_crea(BYTE tipo, char *titulo, char *opciones, short fil,
  short col, BYTE attr_princ, BYTE attr_s1, BYTE attr_s2, BYTE attr_tecla,
  BYTE attr_opcion, short separ_opciones);
void m_abre(STC_MENU *menu);
void m_cierra(STC_MENU *menu);
void m_elimina(STC_MENU *menu);
void m_resalta_opcion(STC_MENU *menu, int resalta);
int m_accion(STC_MENU *menu, STC_ACCION *acc);
int m_elige_opcion(STC_MENU *menu);

#endif  /* MENU_H */

