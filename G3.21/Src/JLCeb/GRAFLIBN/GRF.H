/*

	Librer¡a gr fica SINTAC

	(c) 1995 Javier San Jos‚ y Jos‚ Luis Cebri n
								*/

/*** Constantes ***/
/* modos de v¡deo */
#define G_MV_T40	0x00	/* texto 40x25, 16 colores sin r faga color */
#define G_MV_T40C	0x01	/* texto 40x25, 16 colores */
#define G_MV_T80      	0x02    /* texto 80x25, 16 colores sin r faga color */
#define G_MV_T80C      	0x03	/* texto 80x25, 16 colores */
#define G_MV_G1C16	0x0e	/* gr ficos 640x200, 16 colores */
#define G_MV_G2C16	0x10	/* gr ficos 640x350, 16 colores */
#define G_MV_G3C16	0x12	/* gr ficos 640x480, 16 colores */
#define G_MV_G1C256	0x13	/* gr ficos 320x200, 256 colores */

#define G_MV_G4C16      0xf0    /* gr ficos 720x480, 16 colores */
#define G_MV_G2C256	0xff	/* gr ficos 360x480, 256 colores */
#define G_MV_G3C256     0xfe    /* gr ficos 360x240. 256 colores */

#define G_MV_GV0C256	0x100	/* gr ficos 640x400, 256 colores */
#define G_MV_GV1C256	0x101	/* gr ficos 640x480, 256 colores */
#define G_MV_GV2C256	0x103	/* gr ficos 800x600, 256 colores */
#define G_MV_GV3C256    0x105   /* gr ficos 1024x768, 256 colores */

/* definici¢n alternativa de modos de v¡deo */
#define G_640x200x16    0x0e
#define G_640x350x16    0x10
#define G_640x480x16	0x12
#define G_720x480x16	0xf0
#define G_320x200x256	0x13
#define G_360x240x256   0xfe
#define G_360x480x256	0xff
#define G_640x400x256   0x100
#define G_640x480x256   0x101
#define G_800x600x256   0x103
#define G_1024x768x256  0x105

/* estados del rat¢n */
#define RG_NOVAL	9999	/* valor para rat¢n no inicializado */
#define RG_MUESTRA      1       /* para mostrar puntero de rat¢n */
#define RG_OCULTA      	2       /* para ocultar puntero de rat¢n */

/* modos de dibujo */
#define G_NORM   0x00
#define G_AND    0x08
#define G_OR     0x10
#define G_XOR    0x18
#define G_NOBACK 0x20		/* para imp_chr */

/* modos de memoria para g_memmodel */
#define G_MM_16C	1
#define G_MM_256C	2
#define G_MM_256C_X	3
#define G_MM_256C_V   	4

/* c¢digos de error de las funciones de decodificaci¢n de ficheros PCX */
#define E_PCX_APER  1           /* error de apertura de fichero */
#define E_PCX_LECT  2           /* error de lectura de fichero */
#define E_PCX_FORM  3           /* formato de fichero no v lido */
#define E_PCX_MODO  4           /* modo de pantalla incorrecto */
#define E_PCX_MEM   5           /* no hay memoria suficiente para imagen */

/*** Tipos de datos y estructuras ***/
typedef struct {
	int boton1, boton2;     /* estado de los botones del rat¢n */
				/* 1 pulsado, 0 suelto */
	int xv, yv;           	/* coordenadas virtuales del rat¢n */
	int x, y;		/* coordenadas reales del rat¢n */
	int fil, col;         	/* fila y columna del rat¢n */
} STC_RATONG;

/* imagen decodificada */
typedef struct {
	int ancho, alto;        /* dimensiones en pixels */
	int bits_pixel ;        /* n£mero de colores (4=16, 8=256) */
	int num_bloques ;	/* n£mero de bloques (1-6) */
	char *bloques[6] ;	/* punteros a los bloques */
	BYTE paleta[256][3];    /* paleta RGB de colores */
} STC_IMAGEN;

extern STC_RATONG raton ;

/* variables p£blicas */
extern int g_bytesfila ;
extern long g_bytespantalla ;
extern int g_max_x ;
extern int g_max_y ;
extern int g_memmodel ;
extern int g_modo ;

extern int g_chr_alto ;
extern int g_chr_ancho ;
extern int g_chr_maxfil ;
extern int g_chr_maxcol ;

extern int g_actuando ;

extern int ult_x, ult_y ;

extern int g_raton_resx, g_raton_resy, g_default_pointer ;
extern int g_autoraton ;
extern int g_rg_x, g_rg_y, g_rg_x2, g_rg_y2 ;
extern int g_raton_color ;

/*** funciones ***/
int g_modovideo (int m) ;

int g_maxx(void) ;
int g_maxy(void) ;

void g_setwindow (int zone) ;
void g_incwindow () ;

void g_borra_pantalla (void) ;
unsigned char g_coge_pixel (int x, int y) ;
void g_punto (int x, int y, unsigned char color, unsigned char modo) ;
void g_linea (int x0, int y0, int x1, int y1, unsigned char color, unsigned char modo) ;
void g_rectangulo (int x0, int y0, int x1, int y1, unsigned char color, unsigned char modo, int relleno) ;

void def_chr_set(unsigned char *ptr_set1, unsigned char *ptr_set2,
		 unsigned char *ptr_anch1, unsigned char *ptr_anch2) ;
int chr_anchura(unsigned char chr) ;
void imp_chr_pos(int x, int y) ;
int imp_chr(unsigned char chr, unsigned char colorf, unsigned char color, unsigned char modo) ;
void g_scroll_arr (int l, int c, int an, int al, unsigned char color) ;
void g_scroll_abj (int l, int c, int an, int al, unsigned char color) ;
int chr_altura(void) ;
int chr_maxfil(void) ;
int chr_maxcol(void) ;

unsigned long blq_tam(int x0, int y0, int x1, int y1) ;
void blq_coge(int x0, int y0, int x1, int y1, unsigned char far *blq) ;
void blq_pon(int x, int y, unsigned char far *blq) ;
void blq_clip(int x, int y, int ancho, int alto, unsigned char far *blq) ;

int blq_alto (char *b) ;
int blq_ancho (char *b) ;

/* funciones (rat¢n) */
int rg_inicializa(void);
int rg_puntero(int modo);
int rg_estado(STC_RATONG *raton);
int rg_pon_puntero(int x, int y);
void rg_rango (int x1, int y1, int x2, int y2) ;
void rg_desconecta (void) ;

void rg_setpointer(int) ;
void rg_activar(void) ;
void rg_desactivar(void) ;
void rg_actualizar(void) ;
