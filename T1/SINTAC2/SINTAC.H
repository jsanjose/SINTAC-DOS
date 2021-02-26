/*****************************************
      Fichero de cabecera principal
      del sistema SINTAC.

      Definici¢n de constantes y
      tipos de datos usados por
      los m¢dulos del SINTAC.
*****************************************/

#if !defined (SINTAC_H)
#define SINTAC_H

#define COLUMNAS 80             /* columnas de la pantalla */
#define FILAS    25             /* filas de la pantalla */

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
#define LONGETQ    15           /* longitud m xima+1 de etiqueta */
#define FSKIP      100          /* m ximo n£mero de saltos (SKIP) 'forward' */
#define FPROCESS   2000         /* m ximo n£mero de llamadas 'forward' */
#define NCONST     1000         /* m ximo n£mero de constantes */
#define LNGCONST   15           /* longitud m xima+1 de constantes */

/* valores por defecto */
#define V_MOV   14      /* m ximo n£m. para verbos de movimiento */
#define N_CONV  20      /*    "    "     "  nombres convertibles */
#define N_PROP  50      /*    "    "     "     "    propios */

#define LONGPAL 6       /* longitud de palabra de vocabulario+1 */
#define NUM_PAL 2500	/* m ximo n£mero de palabras en vocabulario */
#define MAX_MSG 256	/*   "      "    de Mensajes */
#define MAX_MSY 256	/*   "      "    de Mensajes del Sistema */
#define NUM_MSY 32      /* n£mero m¡nimo de mensajes del sistema */
#define MAX_LOC 252	/*   "      "    de Localidades */
#define MAX_OBJ 255	/*   "      "    de Objetos */
#define MAX_PRO 256	/*   "      "    de Procesos */

#define BANCOS_RAM 2    /* n£mero de bancos de memoria para RAMSAVE/RAMLOAD */

/* tama¤o m ximo del bloque de bytes */
#if defined NDEBUG
#define TAM_MEM (unsigned)65500
#else
#define TAM_MEM (unsigned)10000
#endif

/* orden de las secciones */
#define MSY 0
#define MSG 1
#define VOC 2
#define LOC 3
#define OBJ 4
#define PRO 5

#define _VERB  0        /* valor para verbo */
#define _NOMB  1        /*   "     "  nombre */
#define _ADJT  2        /*   "     "  adjetivo */
#define _CONJ  3        /*   "     "  conjunci¢n */
#define NO_PAL 255	/* valor de palabra nula */

#define NO_CREADO  252  /* n£m. de loc. para objs. no creados */
#define PUESTO     253  /*  "   "   "     "   "    puestos */
#define COGIDO     254  /*  "   "   "     "   "    cogidos */
#define LOC_ACTUAL 255	/*  "   "   "   que equivale a loc. actual */

/* cadena de reconocimiento */
#define SRECON "Base de datos creada con: SINTAC T2 --- (c)1992 JSJ Soft Ltd.\x1a"
/* longitud cadena reconocimiento + 1 */
#define L_RECON 63
#define COPYRIGHT "S.I.N.T.A.C. (c)1992 JSJ Soft Ltd.\n"

#define N_VENT 10	/* m ximo num. de ventanas */

/* prefijos de indirecci¢n */
#define INDIR1  255	/* indirecci¢n s¢lo en 1er par metro */
#define INDIR2  254	/*      "        "  en 2§      "     */
#define INDIR12 253	/*      "      en los dos primeros par metros */

/* macro para codificar BYTES */
#define CODIGO(b) (BYTE)0xff-(BYTE)b

/****  TIPOS DE DATOS Y ESTRUCTURAS  ****/
typedef unsigned char BYTE;
typedef enum {FALSE=0, TRUE} BOOLEAN;
/* estructura de palabra de vocabulario */
struct palabra {
	char p[LONGPAL];	/* palabra */
	BYTE num;		/* valor */
	BYTE tipo;		/* tipo */
};
/* estructura de c¢digo de retorno de error */
typedef struct {
	int codigo;
	unsigned long linea;
} STC_ERR;
/* estructura de etiqueta */
typedef struct {
	char etq[LONGETQ];      /* para guardar nombre etiqueta */
	BYTE *petq;             /*   "     "    direcci¢n etiqueta */
} STC_ETIQUETA;
/* estructura para saltos (SKIP) 'forward' */
typedef struct {
	char etq[LONGETQ];      /* nombre etiqueta a sustituir */
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
	char cnst[LNGCONST];    /* nombre de la constante */
	BYTE valor;             /* valor de la constante */
} STC_CONSTANTE;

/***  c¢digos de error del compilador ***/
typedef enum {
	_E_NERR=0,      /* c¢digo de no error */
	_E_CIND,        /* c¢digo de indirecci¢n */
	_E_FTMP,        /* error apertura fichero temporal */
	_E_ETMP,        /* error de escritura fichero temporal */
	_E_LTMP,        /* error de lectura fichero temporal */
	_E_AFIN,        /* error apertura fichero de entrada */
	_E_LFIN,        /* error de lectura fichero de entrada */
	_E_EOFI,        /* fin de fichero de entrada */
	_E_AFOU,        /* error de apertura de fichero de salida */
	_E_EFOU,        /* error de escritura en fichero de salida */
	_E_SCCI,        /* secci¢n no v lida */
	_E_SCRP,        /* secci¢n repetida */
	_E_MSCC,        /* marca de secci¢n no encontrada */
	_E_MEND,        /* marca \END no encontrada */
	_E_NOSC,        /* faltan secciones */
	_E_CVOC,        /* car cter no v lido en palabra */
	_E_FCAM,        /* faltan campos */
	_E_NPAL,        /* n£mero de palabra no v lido */
	_E_CNUM,        /* campo num‚rico no v lido */
	_E_TPAL,        /* tipo de palabra no v lido */
	_E_PREP,        /* palabra repetida */
	_E_MVOC,        /* vocabulario lleno */
	_E_FALT,        /* falta '@' */
	_E_NFSC,        /* n£mero fuera de secuencia */
	_E_FMEM,        /* rebasada memoria reservada para secci¢n actual */
	_E_NVAL,        /* n£mero no v lido */
	_E_NPVC,        /* palabra no est  en vocabulario */
	_E_NNVC,        /* nombre no est  en vocabulario */
	_E_NAVC,        /* adjetivo no est  en vocabulario */
	_E_NMOV,        /* la palabra no es verbo de movimiento */
	_E_NNOM,        /* el primer campo debe ser un nombre */
	_E_NADJ,        /* el segundo campo debe ser un adjetivo */
	_E_LINC,        /* localidad inicial no v lida */
	_E_BAND,        /* banderas de objeto no v lidas */
	_E_OTEX,        /* falta texto de objeto */
	_E_NENT,        /* comienzo de entrada no v lido */
	_E_RBTL,        /* tabla de etiquetas llena */
	_E_EREP,        /* etiqueta repetida */
	_E_ENVL,        /* nombre de etiqueta no v lido */
	_E_NVNC,        /* primera palabra entrada no es verbo ni nomb.conv. */
	_E_NNNN,        /* segunda palabra entrada no es nombre */
	_E_NCND,        /* condacto no v lido */
	_E_PRPR,        /* un proceso no puede llamarse a s¡ mismo */
	_E_PRRB,        /* rebasado m ximo n£mero de llamadas 'forward' */
	_E_WPNW,        /* n£mero de ventana no v lido */
	_E_WPWX,        /* columna de inicio de ventana fuera de pantalla */
	_E_WPWY,        /* fila de inicio de ventana fuera de pantalla */
	_E_WPFP,        /* dimensiones de ventana demasiado grandes */
	_E_MPNM,        /* n£mero de mensaje no v lido */
	_E_MPNL,        /* n£mero de localidad no v lido */
	_E_OPNV,        /* n£mero de objeto no v lido */
	_E_NLAB,        /* campo de etiqueta no v lido */
	_E_RBTS,        /* rebasado m ximo num. saltos 'forward' SKIP */
	_E_LBFR,        /* salto fuera de rango */
	_E_NCAD,        /* adjetivo no v lido */
	_E_NNMB,        /* nombre no v lido */
	_E_NVHT,        /* bandera de usuario no v lida */
	_E_CHNV,        /* par metro CHANCE debe estar entre 0 y 100 */
	_E_RBNV,        /* banco de memoria no v lido */
	_E_MPNP,        /* n£mero de 1er mensaje del sistema no v lido */
	_E_MPNS,        /* n£mero de 2§ mensaje del sistema no v lido */
	_E_NOEX,        /* par metro de EXIT no v lido */
	_E_EFIL,        /* fila fuera de pantalla */
	_E_ECOL,        /* columna fuera de pantalla */
	_E_COLR,        /* color incorrecto */
	_E_CLOC,        /* conexi¢n a localidad no v lida */
	_E_RFFW,        /* etiqueta no definida */
	_E_LFFR,        /* salto 'forward' fuera de rango */
	_E_PRLL,        /* llamada a proceso inexistente */
	_E_TCCN,        /* tabla de constantes llena */
	_E_PCCN,        /* primer car cter de nombre de constante no v lido */
	_E_FVCN,        /* falta valor de constante */
	_E_CTND,        /* constante no definida */
	_E_NNCT,        /* nombre de constante no v lido */
	_E_VNCT,        /* error en valor de constante */
} COD_ERR;

#endif  /* SINTAC_H */
