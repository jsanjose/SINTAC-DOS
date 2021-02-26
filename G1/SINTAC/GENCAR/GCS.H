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
#define GCS_COPYRIGHT "GENERADOR DE CARACTERES SINTAC versi¢n 2.0"

#define COLOR_FONDO   7         /* color fondo de botones */
#define COLOR_PPLAN   0         /* color texto de botones */
#define COLOR_S1      15        /* color sombra d‚bil */
#define COLOR_S2      8         /* color sombra fuerte */
#define COLOR_TECLA   4         /* color tecla activaci¢n */
#define COLOR_F_CUAD  0         /* color de fondo de cuadr¡cula */
#define COLOR_B_CUAD  7         /* color de borde de cuadr¡cula */
#define COLOR_R_CUAD  15        /* color de relleno de cuadr¡cula */
#define COLOR_L_ASCII 15        /* color de los car cteres de tabla ASCII */
#define COLOR_R_ASCII 12        /* color de marcador de tabla ASCII */
#define COLOR_F_PRB   0         /* color de fondo de ventana prueba */
#define COLOR_P_PRB   10        /* color primer plano de ventana prueba */

#define NUM_BARR      3         /* n£mero de barras de botones */
#define CUAD_ANCHO    12        /* anchura de cuadro de la cuadr¡cula */
#define CUAD_ALTO     16        /* altura de cuadro de la cuadr¡cula */
#define TAB_ROM8x16   6         /* tabla de 8x16 de la ROM */
#define TAB_ROM8x8    3         /* tabla de 8x8 de la ROM */
#define ALT_C8x16     6+(16*16) /* altura cuadr¡cula 8x16 */
#define ALT_C8x8      6+(16*8)  /* altura cuadr¡cula 8x8 */
#define NPRB_CAR      40        /* n§ de caracteres en ventana de prueba */

#define AVISO_CF      8         /* color fondo ventana aviso */
#define AVISO_CPP     14        /* color primer plano ventana aviso */
#define AVISO_CS1     7         /* color sombra d‚bil ventana aviso */
#define AVISO_CS2     0         /* color sombra fuerte ventana aviso */
#define AVISO_CT      12        /* color tecla activaci¢n ventana aviso */

/* modos de dibujo de caracteres con rutina imp_chrdef() */
#define C_NORM        0x00
#define C_AND         0x08
#define C_OR          0x10
#define C_XOR         0x18

/*** Variables globales ***/
/* icono de flecha apuntando hacia izquierda */
#define PFLECHA_IZQ 7
#define DXFI 28
#define DYFI 3
struct xycoord flecha_izq[PFLECHA_IZQ] = {
	{0+DXFI, 6+DYFI}, {6+DXFI, 0+DYFI}, {6+DXFI, 4+DYFI},
	{12+DXFI, 4+DYFI}, {12+DXFI, 8+DYFI}, {6+DXFI, 8+DYFI},
	{6+DXFI, 12+DYFI}
};

/* icono de flecha apuntando hacia derecha */
#define PFLECHA_DER 7
#define DXFD 28
#define DYFD 3
struct xycoord flecha_der[PFLECHA_DER] = {
	{0+DXFD, 4+DYFD}, {6+DXFD, 4+DYFD}, {6+DXFD, 0+DYFD},
	{12+DXFD, 6+DYFD}, {6+DXFD, 12+DYFD}, {6+DXFD, 8+DYFD},
	{0+DXFD, 8+DYFD}
};

/* icono de flecha apuntando hacia arriba */
#define PFLECHA_ARR 7
#define DXFA 28
#define DYFA 3
struct xycoord flecha_arr[PFLECHA_ARR] = {
	{0+DXFA, 6+DYFA}, {6+DXFA, 0+DYFA}, {12+DXFA, 6+DYFA},
	{8+DXFA, 6+DYFA}, {8+DXFA, 12+DYFA}, {4+DXFA, 12+DYFA},
	{4+DXFA, 6+DYFA}
};

/* icono de flecha apuntando hacia abajo */
#define PFLECHA_ABJ 7
#define DXFB 28
#define DYFB 3
struct xycoord flecha_abj[PFLECHA_ABJ] = {
	{0+DXFB, 6+DYFB}, {4+DXFB, 6+DYFB}, {4+DXFB, 0+DYFB},
	{8+DXFB, 0+DYFB}, {8+DXFB, 6+DYFB}, {12+DXFB, 6+DYFB},
	{6+DXFB, 12+DYFB}
};

/* icono de flecha doble arriba-abajo */
#define PFLECHA_ARRABJ 10
#define DXFAB 28
#define DYFAB 1
struct xycoord flecha_arrabj[PFLECHA_ARRABJ] = {
	{0+DXFAB, 6+DYFAB}, {6+DXFAB, 0+DYFAB}, {12+DXFAB, 6+DYFAB},
	{8+DXFAB, 6+DYFAB}, {8+DXFAB, 9+DYFAB}, {12+DXFAB, 9+DYFAB},
	{6+DXFAB, 15+DYFAB}, {0+DXFAB, 9+DYFAB}, {4+DXFAB, 9+DYFAB},
	{4+DXFAB, 6+DYFAB}
};

/* icono de flecha doble izquierda-derecha */
#define PFLECHA_IZQDER 10
#define DXFID 26
#define DYFID 3
struct xycoord flecha_izqder[PFLECHA_IZQDER] = {
	{0+DXFID, 6+DYFID}, {6+DXFID, 0+DYFID}, {6+DXFID, 4+DYFID},
	{9+DXFID, 4+DYFID}, {9+DXFID, 0+DYFID}, {15+DXFID, 6+DYFID},
	{9+DXFID, 12+DYFID}, {9+DXFID, 8+DYFID}, {6+DXFID, 8+DYFID},
	{6+DXFID, 12+DYFID}
};

/* matriz con barras de botones */
STC_BBOTON barra[NUM_BARR]={
{
	0, 0,                   /* barra de formato especial, contiene */
	BBOTON_INDF,            /* botones distribuidos por toda la pantalla */
	COLOR_FONDO, COLOR_PPLAN, COLOR_S1, COLOR_S2, COLOR_TECLA,
	0, 0,
	7,
	{
		/* ocupa toda la pantalla */
		{"", 0, 0, OFF, NO_ACT, 0, 0, 640, 350},
		/* encabezamiento */
		{"", 0, 0, ON, NO_ACT, 5, 4, 629, 21},
		/* bot¢n de salida del programa */
		{"&Salir", 0, 0, ON, ACT, 581, 324, 54, 20},
		/* cuadr¡cula de dibujo */
		{"", 0, 0, OFF, ACT, 428, 39, 6+(8*CUAD_ANCHO),
		  6+(16*CUAD_ALTO)},
		/* zona de tabla ASCII */
		{"", 0, 0, OFF, ACT, 5, 39, 6+(33*8),
		  6+(16*16)},
		/* recuadro para dibujar car cter a tama¤o real */
		/* adem s servir  como bot¢n de acci¢n */
		{"", 0, 0, OFF, ACT, 280, 39, 22, 30},
		/* para imprimir c¢digo ASCII car cter actual */
		{"", 0, 0, OFF, NO_ACT, 595, 24, 38, 16},
	}
},
{
	5, 24,
	BBOTON_HORZ,
	COLOR_FONDO, COLOR_PPLAN, COLOR_S1, COLOR_S2, COLOR_TECLA,
	118, 16,
	5,
	{
		{"&Cargar fuente", 0, 0, ON, ACT},
		{"&Grabar fuente", 0, 0, ON, ACT},
		{"Fuente 8x1&6", 0, 0, OFF, NO_ACT},
		{"Fuente &8x8", 0, 0, ON, ACT},
		{"&Prueba", 0, 0, ON, ACT},
	}
},
{
	554, 39,
	BBOTON_VERT,
	COLOR_FONDO, COLOR_PPLAN, COLOR_S1, COLOR_S2, COLOR_TECLA,
	80, 24,
	10,
	{
		{"Al&macena", 0, 0, ON, ACT},
		{"&Borra rej", 0, 0, ON, ACT},
		{"&Llena rej", 0, 0, ON, ACT},
		{"&Invertir", 0, 0, ON, ACT},
		{"&W", flecha_arr, PFLECHA_ARR, ON, ACT},
		{"&X", flecha_abj, PFLECHA_ABJ, ON, ACT},
		{"&A", flecha_izq, PFLECHA_IZQ, ON, ACT},
		{"&D", flecha_der, PFLECHA_DER, ON, ACT},
		{"&Q", flecha_arrabj, PFLECHA_ARRABJ, ON, ACT},
		{"&Z", flecha_izqder, PFLECHA_IZQDER, ON, ACT},
	}
},
};

/*** Prototipos de funciones de GCS.C ***/
void _far int24_hnd(unsigned deverror, unsigned errcode,
  unsigned _far *devhdr);

void dibuja_cuadricula(short x, short y);
void cambia_cuadricula(short x, short y, short orgx, short orgy);
void imp_chrdef(BYTE _far *pchr, short colorf, short color, short alt,
  unsigned char modo);
void dibuja_tabla_ascii(short x, short y);
void cambia_caracter(BYTE *car, short x, short y);
void limpia_caracter(BYTE *car, BYTE byte);
int pulsa_en_ascii(short x, short y);
void dibuja_marcador_ascii(int ascii);
void copia_tabla_rom(BYTE tabla, BYTE _far *tabla_car, int alt);
void copia_def_car(BYTE _far *origen, BYTE _far *destino);
void dibuja_en_cuadricula(void);
void actualiza_tabla(short x, short y, int ascii);
void dibuja_caracter(void);
int graba_def(char *nombre);
int carga_def(char *nombre);
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
void invertir_def(void);
void gira180(void);
void espejo_def(void);
void cargar_fuente(void);
void grabar_fuente(void);
void pasa_a8x16(void);
void pasa_a8x8(void);
void prueba_car(void);
void imp_ascii_car(void);
