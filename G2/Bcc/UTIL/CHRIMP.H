/************************************
	Fichero de cabecera de
	las funciones de impresión
	de caracteres de CHRIMP.C
************************************/

#if !defined (CHRIMP_H)
#define CHRIMP_H

/*** Constantes ***/
/* modos de dibujo de caracteres con rutina imp_chr() */
#define CHR_NORM  0x00
#define CHR_AND   0x08
#define CHR_OR    0x10
#define CHR_XOR   0x18

/*** Prototipos ***/
void imp_chr(int chr, short colorf, short color, unsigned char modo);
void imp_cad(char *cadena, short colorf, short color, unsigned char modo);
void impn_cad(char *cadena, int numcar, short colorf, short color,
  unsigned char modo);

#endif  /* CHRIMP_H */

