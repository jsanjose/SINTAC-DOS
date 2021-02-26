/***************************************
	Fichero de cabecera del
	programa de instalaci¢n
	del sistema SINTAC
***************************************/

/*** Constantes ***/
#define MENU_FIL        5       /* fila men£ principal */
#define MENU_COL        21      /* columna "     "     */
#define CURSOR          0x0607  /* tipo de cursor */
#define CDIR_ANCHO      68      /* anchura cuadro directorios */
#define CDIR_ALTO       17      /* altura cuadro directorios */
#define MCOLOR_FIL      5       /* fila men£ selecci¢n color */
#define MCOLOR_COL      21      /* columna "    "        "   */
#define VCOLOR_FIL      6       /* fila ventana prueba de color */
#define VCOLOR_COL      10      /* columna  "     "    "    "   */
#define VCOLOR_ANCHO    30      /* anchura  "     "    "    "   */
#define VCOLOR_ALTO     9       /* altura   "     "    "    "   */
#define CCOLOR_FIL      6       /* fila cuadro cambio de colores */
#define CCOLOR_COL      40      /* columna "     "    "     "    */
#define CCOLOR_ANCHO    30      /* anchura "     "    "     "    */
#define CCOLOR_ALTO     14      /* altura  "     "    "     "    */
#define VSELCOL_FIL     6       /* fila ventana selecci¢n color */
#define VSELCOL_COL     26      /* columna  "       "       "   */
#define VSELCOL_ANCHO   27      /* anchura  "       "       "   */
#define VSELCOL_ALTO    8       /* altura   "       "       "   */
#define VSELCOL_CHR1    'Û'     /* car cter para paleta de color */
#define VSELCOL_CHR2    0x18    /*    "     marcador */
#define ERR_ANCHO       38      /* anchura cuadro de errores */
#define ERR_ALTO        8       /* altura    "    "    "     */
#define AV_ANCHO        40      /* anchura cuadro de aviso */
#define AV_ALTO         9       /* altura    "    "    "   */
#define NUM_FICH_INST   100     /* m ximo n£mero de ficheros para instalar */
#define LNG_NOMBRE      41      /* longitud m xima de nombre de fichero + 1 */

/*** Tipos de datos y estructuras ***/
typedef struct {
	char nombre[LNG_NOMBRE];        /* nombre del fichero */
	int tipo;                       /* tipo de fichero */
	int disco;                      /* n§ disco de instalaci¢n */
} STC_DATFICH;

/*** Prototipos ***/
void _far int24_hnd(unsigned deverror, unsigned errcode,
  unsigned _far *devhdr);
void inicializa_cfg(void);
void esconde_cursor(void);
void ajusta_ndir(char *dir);
void directorios(void);
void colores(void);
void elige_colores(char *titulo, BYTE *attr_princ, BYTE *attr_s1,
  BYTE *attr_s2, BYTE *attr_tecla, BYTE *attr_sel);
BYTE selecciona_color(BYTE c);
BYTE cambia_tinta(BYTE attr);
BYTE cambia_fondo(BYTE attr);
void error(char *msg);
void lee_datos_fich(char *nf_datfich);
int copia(char *origen, char *destino);
void beep(void);
void instalar(void);
void aviso(void);
