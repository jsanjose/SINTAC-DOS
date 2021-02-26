/***************************************
	Fichero de cabecera principal
	del entorno de desarrollo
	SINTAC
***************************************/

/*** Constantes ***/
#define AYD_ANCHO       76      /* anchura de ventana de ayuda */
#define AYD_ALTO        20      /* altura  "    "     "    "   */
#define INF_ANCHO       52      /* anchura de ventana de informaci¢n */
#define INF_ALTO        10      /* altura  "    "     "       "      */
#define ERR_ANCHO       33      /* anchura de ventana de errores */
#define ERR_ALTO        8       /* altura  "    "     "    "     */
#define DOS_ANCHO       40      /* anchura de cuadro de comandos DOS */
#define DOS_ALTO        8       /* altura  "    "    "     "      "  */
#define FIL_SECC        3       /* fila para mensajes de secciones */
#define COL_SECC        15      /* columna      "     "      "     */
#define PRO_ANCHO       76      /* anchura cuadro de selecci¢n de procesos */
#define PRO_ALTO        15      /* altura    "    "      "     "     "     */
#define MSG_ANCHO       76      /* anchura cuadro de selecci¢n tablas mens. */
#define MSG_ALTO        15      /* altura    "    "      "       "     "    */
#define AV_ANCHO        56      /* anchura de cuadro de avisos */
#define AV_ALTO         9       /* altura  "    "    "    "    */
#define CERR_ANCHO      76      /* anchura de cuadro de errores */
#define CERR_ALTO       15      /* altura  "    "    "     "    */
#define AYT_ANCHO       40      /* anchura de cuadro de ayuda sobre... */
#define AYT_ALTO        12      /* altura  "    "    "    "     "      */
#define NF_BDINIC       "INICIO.BD"     /* base datos de inicio */
#define MAX_NUTILS      10      /* m x. n£mero de utilidades en men£ */

/*** Prototipos ***/
int int24_hnd(int errval, int ax, int bp, int si);
void lee_cfg(char *argv0);
void acerca_de(void);
void imprime_error(char *msg);
void ayuda(char *tema);
void comando_dos(void);
void graba_bloque(void);
void esconde_cursor(void);
unsigned long max_mem(void);
int copia_fichero(char *nfichorg, char *nfichdest);
int crea_bd(char *bd);
int abre_bd(char *bd);
int es_marca_secc(char *lin);
int busca_marca(char *marca, char *l);
int salta_hasta_marca(FILE *fbd);
void coge_marca(char *l, char *marca);
int pon_extensionsecc(char *csecc, char *nf);
int copia_fichsecc(char *nfsecc, FILE *fbd, char *msecc);
int copia_secc(FILE *fbd, FILE *faux, char *msecc);
int actualiza_bdtemp(void);
int graba_bd(char *bd);
int graba_bd_modif(void);
void borra_fichtemp(void);
int separa_seccion(char *csecc, char *nfsecc);
int edita_seccion(char *csecc, int nlin);
int crea_seccmult(int ns, char *msecc, char *merr_ex, BOOLEAN *secc_existe);
int borra_seccmult(int ns, char *msecc, char *merr_noex, BOOLEAN *secc_existe);
int pregunta_siono(char *preg);
int elige_pro(void);
int elige_msg(void);
int ejecuta(char *ruta, char *prg, char **arg);
void haz_fich_sal(char *fich, char *fsal);
int compila_bd(void);
void haz_fich_exe(char *fich, char *fexe);
int linka_bd(void);
int ejecuta_bd(void);
int busca_linea_secc(unsigned long nlin, char *secc, int *lin_secc);
int errores_compil(void);
void ayuda_sobre(void);
void utilidades(void);
