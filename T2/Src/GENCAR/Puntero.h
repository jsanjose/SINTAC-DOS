/************************************
	Fichero de cabecera para
	las funciones de manejo de
	puntero de PUNTERO.C
************************************/

#if !defined (PUNTERO_H)
#define PUNTERO_H

/*** Constantes ***/
/* constantes para comprobar botones pulsados mediante funci¢n AND (&) */
#define PULSADO_IZQ 0x01        /* 0000 0010 bot¢n izquierdo pulsado */
#define PULSADO_DER 0x02        /* 0000 1000 bot¢n derecho pulsado */
#define PULSADO_CEN 0x04        /* 0010 0000 bot¢n central pulsado */

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
int coge_pos_puntero(STC_PUNTERO _far *puntero);
int vis_puntero(enum PTRVIS pv);

#endif  /* PUNTERO_H */
