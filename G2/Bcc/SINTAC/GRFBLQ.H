/***************************************
	Fichero de cabecera de
	las funciones de manejo de
	bloques gr ficos de GRFBLQ.C
***************************************/

#if !defined (GRFBLQ_H)
#define GRFBLQ_H

/*** Prototipos ***/
unsigned long blq_tam(short x0, short y0, short x1, short y1);
void blq_coge(short x0, short y0, short x1, short y1, unsigned char far *blq);
void blq_pon(short x, short y, unsigned char far *blq);

#endif  /* GRFBLQ_H */