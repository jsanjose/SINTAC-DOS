/*****************************************
	Fichero de cabecera principal
	del generador de caracteres
	SINTAC.

	Prototipos de funciones,
	constantes y variables globales
	usadas por el generador de
	caracteres.
*****************************************/

/*** Constantes ***/
#define NUM_BARR      3         /* n£mero de barras de botones */
#define CUAD_ANCHO    12        /* anchura de cuadro de la cuadr¡cula */
#define CUAD_ALTO     16        /* altura de cuadro de la cuadr¡cula */
#define ALTURA_CUAD   14        /* altura de la cuadr¡cula */
#define COLOR_F_CUAD  0         /* color de fondo de cuadr¡cula */
#define COLOR_B_CUAD  7         /* color de borde de cuadr¡cula */
#define COLOR_R_CUAD  15        /* color de relleno de cuadr¡cula */
#define COLOR_L_ASCII 15        /* color de los car cteres de tabla ASCII */
#define COLOR_R_ASCII 12        /* color de marcador de tabla ASCII */
#define FILA_TABLA    4         /* fila de origen de tabla ASCII */
#define COLUMNA_TABLA 2         /* columna de origen de tabla ASCII */
#define TAB_ROM8x14   2         /* tabla de 8x14 de la ROM */

/*** Variables globales ***/
STC_BBOTON barra[NUM_BARR]={
{
	0, 0,                   /* barra de formato especial */
	BBOTON_INDF,            /* contiene botones distribuidos por */
	7, 0, 15, 8,            /* toda la pantalla */
	0, 0,
	8,
	{
		/* ocupa toda la pantalla */
		{"", TAMTXT_GRA, OFF, NO_ACT, 0, 0, 640, 350},
		/* encabezamiento */
		{"", TAMTXT_PEQ, ON, NO_ACT, 5, 4, 629, 21},
		/* bot¢n de salida del programa */
		{"&Salir", TAMTXT_PEQ, ON, ACT, 602, 303, 33, 41},
		/* zona de dibujo */
		{"", TAMTXT_MED, OFF, ACT, 428, 39, 6+(8*CUAD_ANCHO),
		  6+(ALTURA_CUAD*CUAD_ALTO)},
		/* zona de tabla ASCII */
		{"", TAMTXT_MED, OFF, ACT, 5, 39, 6+(33*8), 6+(16*ALTURA_CUAD)},
		/* recuadro para dibujar car cter a tama¤o real */
		/* adem s servir  como bot¢n de acci¢n */
		{"", TAMTXT_MED, OFF, ACT, 280, 39, 22, 14+ALTURA_CUAD},
		/* para entradas por teclado */
		{"", TAMTXT_MED, OFF, NO_ACT, 5, 303, 593, 20},
		/* para mensajes */
		{"", TAMTXT_MED, OFF, NO_ACT, 5, 324, 593, 20},
	}
},
{
	5, 24,
	BBOTON_HORZ,
	7, 0, 15, 8,
	135, 16,
	2,
	{
		{"&Cargar fuente", TAMTXT_PEQ, ON, ACT},
		{"&Grabar fuente", TAMTXT_PEQ, ON, ACT},
	}
},
{
	554, 39,
	BBOTON_VERT,
	7, 0, 15, 8,
	80, 24,
	7,
	{
		{"&Almacena", TAMTXT_PEQ, ON, ACT},
		{"&Borra rejilla", TAMTXT_PEQ, ON, ACT},
		{"&Llena rejilla", TAMTXT_PEQ, ON, ACT},
		{"<A&rriba>", TAMTXT_PEQ, ON, ACT},
		{"<Aba&jo>", TAMTXT_PEQ, ON, ACT},
		{"<&Izquierda>", TAMTXT_PEQ, ON, ACT},
		{"<&Derecha>", TAMTXT_PEQ, ON, ACT},
	}
},
};

/*** Prototipos de funciones de GCS.C ***/
void _far int24_hnd(unsigned deverror, unsigned errcode,
  unsigned _far *devhdr);

void dibuja_cuadricula(short x, short y, int alt);
void cambia_cuadricula(short x, short y, short orgx, short orgy, int alt);
void dibuja_tabla_ascii(short fila, short columna);
void cambia_caracter(BYTE *car, short x, short y);
void limpia_caracter(BYTE *car, int alt, BYTE byte);
int pulsa_en_ascii(short x, short y);
void dibuja_marcador_ascii(int ascii);
void copia_tabla_rom(BYTE tabla, BYTE _far *tabla_car);
void copia_def_car(BYTE _far *origen, BYTE _far *destino);
void dibuja_en_cuadricula(short orgx, short orgy, BYTE *def_car);
void actualiza_tabla(short fila, short columna, int ascii);
void dibuja_caracter(short x, short y, short color, BYTE *caracter);
int graba_def(char *nombre, BYTE *fuente);
int carga_def(char *nombre, BYTE *fuente);
void cuadricula(void);
void tab_ascii(void);
void def_caracter(void);
void almacena_definicion(void);
void borra_rejilla(void);
void llena_rejilla(void);
void scroll_def_arr(void);
void scroll_def_abj(void);
void scroll_def_izq(void);
void scroll_def_der(void);
int pide_nombre_fich(char *nombre, int longitud);
short espera_tecla(void);
void cargar_fuente(void);
void grabar_fuente(void);
