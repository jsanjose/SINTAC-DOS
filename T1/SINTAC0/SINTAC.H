/****************************************/
/************** Constantes **************/
/****************************************/

#define LONGPAL 6       /* longitud de palabra de vocabulario+1 */
#define NUM_PAL 2500    /* m ximo n£mero de palabras en vocabulario */
#define MAX_MSG 256     /*   "      "    de Mensajes */
#define MAX_MSY 256     /*   "      "    de Mensajes del Sistema */
#define MAX_LOC 252     /*   "      "    de Localidades */
#define MAX_OBJ 255     /*   "      "    de Objetos */
#define MAX_PRO 256     /*   "      "    de Procesos */

#define _VERB 0         /* valor para verbo */
#define _NOMB 1         /*   "     "  nombre */
#define _ADJT 2         /*   "     "  adjetivo */
#define _CONJ 3         /*   "     "  conjunci¢n */
#define NO_PAL 255      /* valor de cualquier palabra */

#define NO_CREADO 252   /* n£m. de loc. para objs. no creados */
#define PUESTO 253      /*  "   "   "     "   "    puestos */
#define COGIDO 254      /*  "   "   "     "   "    cogidos */
#define LOC_ACTUAL 255  /*  "   "   "   que equivale a loc. actual */

#define NULLF 0L  /* puntero far nulo */

#define SRECON "JSJ SINTAC T1.00"
#define L_RECON 17  /* longitud de la cadena de reconocimiento */
#define COPYRIGHT "S.I.N.T.A.C. (c)1992 JSJ Soft Ltd.\n"

#define N_VENT 10   /* m ximo num. de ventanas */

/* prefijos de indirecci¢n */
#define INDIR1  255 /* indirecci¢n s¢lo en 1er par metro */
#define INDIR2  254 /*      "        "  en 2§      "     */
#define INDIR12 253 /*      "      en los dos primeros par metros */

/****************************************/
/***** Tipos de datos y estructuras *****/
/****************************************/

typedef unsigned char BYTE;
struct palabra {
	char p[LONGPAL];    /* palabra */
	BYTE num;           /* valor */
	BYTE tipo;          /* tipo */
};

