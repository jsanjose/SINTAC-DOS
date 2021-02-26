/************************************
	Fichero de cabecera de
	las funciones de manejo
	de ficheros PCX de PCX.C
************************************/

#if !defined (PCX_H)
#define PCX_H

/*** Macros ***/
#define RGB(r,g,b) (((long)((b) << 8 | (g)) << 8) | (r))

/*** Constantes ***/
/* tama¤o en bytes de cada plano de una l¡nea de pantalla en los */
/* modos de alta resoluci¢n a 16 colores de EGA y VGA */
#define TAMLINPCX16  80

/* tama¤o en byte de una l¡nea de pantalla en el modo de 256 colores */
#define TAMLINPCX256 320

/* tama¤o de buffer de entrada/salida de ficheros */
#define PCX_FICH_BUF 20480

/* c¢digos de error de las funciones de decodificaci¢n de ficheros PCX */
#define E_PCX_APER  1           /* error de apertura de fichero */
#define E_PCX_LECT  2           /* error de lectura de fichero */
#define E_PCX_FORM  3           /* formato de fichero no v lido */
#define E_PCX_MODO  4           /* modo de pantalla incorrecto */

/* modos de dibujado de la imagen */
#define PCX_PINTA   0           /* va dibujando la imagen seg£n la lee */
#define PCX_OCULTA  1           /* dibuja la imagen pero no se ve hasta fin */
#define PCX_PALETA  2           /* conserva la paleta de colores actual */

/*** Tipos de datos y estructuras ***/
#if !defined (_U_CHAR_BYTE)
typedef unsigned char BYTE;
#define _U_CHAR_BYTE
#endif

typedef struct {
	BYTE propietario;       /* deber¡a ser siempre 0x0A */
	BYTE version;           /* version del fichero PCX */
	BYTE metodo_codif;      /* 1=codificaci¢n PCX 'run-lenght' */
	BYTE bits_pixel;        /* n£mero de bits por pixel */
	int ventana[4];         /* tama¤o dibujo (izq., arr., der., abajo) */
	int resh, resv;         /* resoluci¢n horizontal y vertical */
	BYTE paleta[16][3];     /* informaci¢n de paleta para la imagen */
	BYTE reserv1;
	BYTE num_planos;        /* n£mero de planos de color */
	int bytes_scan;         /* n£mero de bytes por l¡nea de scan */
	BYTE reserv2[60];
} PCXHEADER;

/*** Prototipos ***/
int pcx_decodif16(char *nombre_pcx, short x, short y, int anchura, int altura,
  int modo);
int pcx_decodif256(char *nombre_pcx, short x, short y, int anchura,
  int altura, int modo);

#endif  /* PCX_H */
