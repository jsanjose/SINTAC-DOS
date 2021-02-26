/*****************************************
      Fichero de cabecera principal
      del sistema SINTAC.

      Definici¢n de constantes y
      tipos de datos usados por
      los m¢dulos del SINTAC.
*****************************************/

#if !defined (SINTAC_H)
#define SINTAC_H

/*** Constantes ***/
#define MODO0_COL  80           /* columnas en el modo 0 (640x480x16) */
#define MODO0_FIL  30           /* filas en el modo 0 */
#define MODO1_COL  40           /* columnas en el modo 1 (320x200x256) */
#define MODO1_FIL  25           /* filas en el modo 1 */

#define N_SECCS    6            /* n£mero de secciones en base de datos */
#define LONG_LIN   129          /* longitud m xima de l¡nea de entrada */
#define MARCA_S    (char)'\\'   /* marca de inicio de secci¢n */
#define CHR_COMENT (char)';'    /* indicador de comentario */
#define CHR_DELIM  (char)'@'    /* delimitador de texto */
#define MARCA_CNX  (char)'#'    /* marca de inicio de conexi¢n */
#define CHR_NOPAL  (char)'_'    /* car cter indicador 'cualquier palabra' */
#define MARCA_ETQ  (char)'$'    /* marca de etiqueta */
#define BAND_0     (char)'O'    /* car cter para band. usuario desactivada */
#define BAND_1     (char)'X'    /* car cter para band. usuario activada */
#define LNGCOND    7            /* longitud de un condacto */
#define LABELS     100          /* m ximo n£mero de etiquetas */
#define LONGETQ    14           /* longitud m xima de etiqueta */
#define FSKIP      100          /* m ximo n£mero de saltos (SKIP) 'forward' */
#define FPROCESS   2000         /* m ximo n£mero de llamadas 'forward' */
#define NCONST     1000         /* m ximo n£mero de constantes */
#define LNGCONST   14           /* longitud m xima de constantes */
#define VARS       256          /* n£mero de variables */
#define BANDS      32           /* n£mero de banderas / 8 */
#define MAXLONG    128          /* m xima longitud de la l¡nea de entrada */
				/* en el int‚rprete */

/* valores por defecto */
#define V_MOV   14      /* m ximo n£m. para verbos de movimiento */
#define N_CONV  20      /*    "    "     "  nombres convertibles */
#define N_PROP  50      /*    "    "     "     "    propios */

#define LONGPAL 6       /* longitud de palabra de vocabulario */
#define NUM_PAL 2500	/* m ximo n£mero de palabras en vocabulario */
#define MAX_MSY 256     /*   "      "    de Mensajes del Sistema */
#define MAX_MSG 256     /*   "      "    de Mensajes */
#define MAX_LOC 252     /*   "      "    de Localidades */
#define MAX_OBJ 255	/*   "      "    de Objetos */
#define MAX_PRO 256	/*   "      "    de Procesos */
#define NUM_MSY 32      /* n£mero m¡nimo de Mensajes del Sistema */

#define BANCOS_RAM 2    /* n£mero de bancos de memoria para RAMSAVE/RAMLOAD */

#define TAM_MEM     0xffdc      /* tama¤o del bloque de bytes */
#define MIN_TAM_MEM 0x1000      /* m¡nimo tama¤o del bloque de bytes */

/* orden de las secciones */
#define VOC 0
#define MSY 1
#define MSG 2
#define LOC 3
#define OBJ 4
#define PRO 5

#define _VERB  0        /* valor para verbo */
#define _NOMB  1        /*   "     "  nombre */
#define _ADJT  2        /*   "     "  adjetivo */
#define _CONJ  3        /*   "     "  conjunci¢n */
#define NO_PAL 255	/* valor de palabra nula */

#define NO_CREADO  252  /* n£mero de localidad para objs. no creados */
#define PUESTO     253  /*   "    "      "      "     "   puestos */
#define COGIDO     254  /*   "    "      "      "     "   cogidos */
#define LOC_ACTUAL 255  /*   "    "      "     que equivale a loc. actual */

/* cadena de reconocimiento de fichero de base de datos */
/* los 2 caracteres finales indican la versi¢n de la base de datos */
/* las versiones disponibles son: */
/*      'T1' primera versi¢n s¢lo modo texto */
/*      'T2' segunda versi¢n modo texto, condactos a¤adidos */
/*           palabras del vocabulario de 6 caracteres */
/*      'G1' primera versi¢n modo gr fico */
#define SRECON "JSJ SINTAC G1"
/* longitud de la cadena de reconocimiento */
#define L_RECON 13

/* cadena de reconocimiento de fichero de fuente */
/* el £ltimo car cter indica la versi¢n de fichero */
/* versiones disponibles: */
/*      "1"     fuente de 8x14 */
/*      "2"     fuentes de 8x16 y 8x8 */
#define RECON_FUENTE  "JSJ SINTAC FNT2"
/* longitud de la cadena de reconocimiento */
#define LONG_RECON_F  15

#define N_VENT 10       /* m ximo n£mero de ventanas */
#define N_BORD 9        /* m ximo n£mero de tipos de borde */

/* prefijos de indirecci¢n */
#define INDIR1  255	/* indirecci¢n s¢lo en 1er par metro */
#define INDIR2  254	/*      "        "  en 2§      "     */
#define INDIR12 253	/*      "      en los dos primeros par metros */

/* nombre de fichero donde se guardar n los errores del compilador */
#define NF_ERR  "$SINT$.ERR"

/* nombre de fichero de configuraci¢n */
#define NF_CFG  "SINTAC.CFG"

/* macro para codificar BYTES */
#define CODIGO(b) (BYTE)0xff-(BYTE)b

/*** Tipos de datos y estructuras ***/
#if !defined (_U_CHAR_BYTE)
typedef unsigned char BYTE;
#define _U_CHAR_BYTE
#endif

typedef enum {FALSE=0, TRUE} BOOLEAN;

/* estructura de palabra de vocabulario */
struct palabra {
	char p[LONGPAL+1];      /* palabra */
	BYTE num;		/* valor */
	BYTE tipo;		/* tipo */
};

/* estructura de cabecera de fichero de base de datos */
typedef struct {
	char srecon[L_RECON+1];         /* cadena de reconocimiento */
	BYTE v_mov;                     /* m x. n£m. de verbo de movimiento */
	BYTE n_conv;                    /*  "    "    " nombre convertible */
	BYTE n_prop;                    /*  "    "    " nombre propio */
	int pal_voc;                    /* n£mero de palabras en vocabulario */
	BYTE num_msy;                   /* n£m. de mensajes del sistema */
	unsigned bytes_msy;             /* memoria para mensajes del sistema */
	BYTE num_msg;                   /* n£mero de mensajes */
	unsigned bytes_msg;             /* memoria para mensajes */
	BYTE num_loc;                   /* n£mero de localidades */
	unsigned bytes_loc;             /* memoria para localidades */
	unsigned bytes_conx;            /*   "      "   conexiones */
	BYTE num_obj;                   /* n£mero de objetos */
	unsigned bytes_obj;             /* memoria para objetos */
	BYTE num_pro;                   /* n£mero de procesos */
	unsigned bytes_pro;             /* memoria para procesos */
	BYTE reservado[1002];           /* RESERVADO */
} CAB_SINTAC;

/* estructura de c¢digo de retorno de error */
typedef struct {
	int codigo;
	unsigned long linea;
} STC_ERR;

/* estructura de etiqueta */
typedef struct {
	char etq[LONGETQ+1];    /* para guardar nombre etiqueta */
	BYTE *petq;             /*   "     "    direcci¢n etiqueta */
} STC_ETIQUETA;

/* estructura para saltos (SKIP) 'forward' */
typedef struct {
	char etq[LONGETQ+1];    /* nombre etiqueta a sustituir */
	BYTE *fsk;              /* d¢nde sustituir etiqueta */
	unsigned long nl;       /* n£mero de l¡nea en archivo de entrada */
} STC_SKPFORWARD;

/* estructura para llamadas a procesos 'forward' */
typedef struct {
	BYTE numpro;            /* n£mero de proceso al que se llama */
	unsigned long nl;       /* l¡nea de archivo de entrada d¢nde llama */
} STC_PRCFORWARD;

/* estructura para constantes */
typedef struct {
	char cnst[LNGCONST+1];  /* nombre de la constante */
	BYTE valor;             /* valor de la constante */
} STC_CONSTANTE;

/* estructura del fichero de configuraci¢n */
typedef struct {
	char dir_sintac[_MAX_PATH];     /* dir. del sistema SINTAC */
	char dir_gcs[_MAX_PATH];        /* dir. generador caracteres */
	char dir_bd[_MAX_PATH];         /* dir. bases de datos */
	BYTE color_scr;                 /* color general de pantalla */
	BYTE color_editor;              /* color de zona de edici¢n */
	BYTE color_regla;               /* color de la regleta */
	BYTE color_cab;                 /* color de la cabecera */
	BYTE color_linst;               /* color de l¡nea de estado */
	BYTE color_menu;                /* color del men£ de opciones */
	BYTE color_tecla;               /* color de tecla de activaci¢n */
	BYTE color_opcion;              /* color opci¢n resaltada */
	BYTE color_wcomp;               /* color de ventana del compilador */
	BYTE color_werr;                /* color de ventana de errores */
	BYTE color_err;                 /* color ventana errores entorno */
	BYTE color_errsel;              /* color de error resaltado */
	BYTE color_ay;                  /* color de ventana de ayuda */
	BYTE color_aptdo;               /* color de cabecera de apartado */
	BYTE color_ref;                 /* color de referencia de ayuda */
} STC_CFG;

#endif  /* SINTAC_H */
