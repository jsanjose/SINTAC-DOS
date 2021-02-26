/************************************
	Fichero de cabecera de
	las funciones de manejo
	de ventanas de WIN.H
************************************/

#if !defined (WIN_H)
#define WIN_H

/*** Constantes ***/
#define SCROLL_PAU 500L         /* pausa para s¡mbolo scroll */
#define CAR_SCROLL (BYTE)'\x1f' /* car cter cuando hay que 'scrollear' */
#define MAX_PAL    80           /* tama¤o buffer para w_imps() */
#define W_CURSOR   (char)'Û'    /* cursor que puede usarse con w_input() */

/* c¢digos de teclas especiales */
#define IZQ     0x4b00
#define DER     0x4d00
#define ARR     0x4800
#define ABJ     0x5000
#define ORG     0x4700
#define FIN     0x4f00
#define SUP     0x5300
#define F1      0x3b00
#define F2      0x3c00
#define F3      0x3d00
#define F4      0x3e00
#define F5      0x3f00
#define F6      0x4000
#define F7      0x4100
#define F8      0x4200
#define F9      0x4300
#define F10     0x4400

/* valores devueltos por w_lee_tecla() para las teclas de cursor y funci¢n */
#define COD_F1  0xff
#define COD_F2  0xfe
#define COD_F3  0xfd
#define COD_F4  0xfc
#define COD_F5  0xfb
#define COD_F6  0xfa
#define COD_F7  0xf9
#define COD_F8  0xf8
#define COD_F9  0xf7
#define COD_F10 0xf6
#define COD_DER 0xf5
#define COD_IZQ 0xf4
#define COD_ABJ 0xf3
#define COD_ARR 0xf2
#define COD_ORG 0xf1
#define COD_FIN 0xf0
#define COD_SUP 0xef

#define RETURN     '\r'
#define BACKSPACE  '\b'

/* tipos de borde para las ventanas */
#define NO_BORDE   0    /* sin borde */
#define BORDE_1    1    /* borde simple */
#define BORDE_2    2    /* borde doble */
#define BORDE_3    3    /* borde doble arriba-abajo, simple lados */
#define BORDE_4    4    /* borde simple arriba-abajo, doble lados */
#define BORDE_5    5    /* borde relleno 1 */
#define BORDE_6    6    /* borde relleno 2 */
#define BORDE_7    7    /* borde relleno 3 */
#define BORDE_8    8    /* borde relleno 4 */

/*** Tipos de datos y estructuras ***/
#if !defined (_U_CHAR_BYTE)
typedef unsigned char BYTE;
#define _U_CHAR_BYTE
#endif

typedef struct {
	BYTE wx, wy;            /* posici¢n de la ventana (esquina sup. izq.) */
	BYTE lx, ly;            /* tama¤o de la ventana (ancho, alto) */
	BYTE cwx, cwy;          /* posici¢n actual cursor dentro de ventana */
	BYTE color;             /* color global de la ventana */
	BYTE colort;            /* color temporal de la ventana */
	BYTE scroll;            /* indicador de scroll */
	BYTE cwxs, cwys;        /* posici¢n cursor guardada */
	BYTE borde;             /* tipo de borde de la ventana */
	BYTE wxi, wyi;          /* origen del interior de la ventana */
	BYTE lxi, lyi;          /* dimensiones del interior de la ventana */
} STC_WINDOW;

/*** Prototipos ***/
void w_impcar(BYTE fil, BYTE col, BYTE car, BYTE attr);
void w_scroll(BYTE lineas, STC_WINDOW *w);
void w_crea(BYTE f, BYTE c, BYTE ancho, BYTE alto, BYTE color, BYTE borde,
  STC_WINDOW *w);
void w_cls(STC_WINDOW *w);
void w_impc(char c, STC_WINDOW *w);
void w_imps(char *s, STC_WINDOW *w);
void w_impn(unsigned n, STC_WINDOW *w);
unsigned w_input(char *plin, unsigned maxlong, char cursor, STC_WINDOW *w);
unsigned w_lee_tecla(void);

#endif  /* WIN_H */
