/************************************
	Fichero de cabecera para
	las funciones de manejo de
	puntero de PUNTERO.C
************************************/

#if !defined (PUNTERO_H)
#define PUNTERO_H

/*** Constantes ***/
/* constantes para comprobar botones pulsados mediante funci¢n AND (&) */
#define PULSADO_IZQ 0x01        /* bot¢n izquierdo pulsado */
#define PULSADO_DER 0x02        /* bot¢n derecho pulsado */
#define PULSADO_CEN 0x04        /* bot¢n central pulsado */

#define CUR_IZQ 0x4b            /* c¢digo scan de cursor izquierda */
#define CUR_DER 0x4d            /*   "     "   "    "    derecha */
#define CUR_ARR 0x48            /*   "     "   "    "    arriba */
#define CUR_ABJ 0x50            /*   "     "   "    "    abajo */
#define TEC_ESP 0x39            /*   "     "   "  barra espaciadora */

/* contantes del punmtero cuando se controla por teclado */
#define MULT_VEL 2              /* multiplicador de velocidad, indica */
				/* cada cuantas pulsaciones aumenta veloc. */
#define VEL_INIC 1*MULT_VEL     /* velocidad inicial (pixels) */
#define MAX_VEL  8*MULT_VEL     /* m xima velocidad (pixels) */

/*** Tipos de datos y estructuras ***/
/* estructura para informaci¢n sobre puntero */
typedef struct {
    short x, y;
    unsigned botones;
} STC_PUNTERO;

/* valores para la funci¢n vis_puntero() */
typedef enum {MUESTRA=1, ESCONDE} PTRVIS;

/*** Prototipos ***/
int inic_puntero(void);
int coge_pos_puntero(STC_PUNTERO *puntero);
int vis_puntero(enum PTRVIS pv);
int hay_raton(void);

#endif  /* PUNTERO_H */
