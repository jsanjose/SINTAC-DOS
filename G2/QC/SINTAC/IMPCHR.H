/***************************************
	Fichero de cabecera de
	las funciones de impresi¢n
	de caracteres de IMPCHR.C
***************************************/

#if !defined (IMPCHR_H)
#define IMPCHR_H

/*** Constantes ***/
/* altura de las definiciones de caracteres */
#define CHR_ALT   16

/* modos de impresi¢n de caracteres con la funci¢n imp_chr() */
#define CHR_NORM  0x00
#define CHR_AND   0x08
#define CHR_OR    0x10
#define CHR_XOR   0x18

/*** Prototipos ***/
void imp_chr(int chr, short colorf, short color, unsigned char modo);
void def_chr_set(unsigned char _far *ptr_set1, unsigned char _far *ptr_set2);

#endif  /* IMPCHR_H */

