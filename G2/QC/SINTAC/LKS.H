/***************************************
	Fichero de cabecera principal
	del linkador SINTAC
***************************************/

/*** Constantes ***/
#define CURSOR          0x0607  /* tipo de cursor */
#define ERR_ANCHO       42      /* anchura cuadro de errores */
#define ERR_ALTO        8       /* altura    "    "    "     */

/*** Prototipos ***/
void _far int24_hnd(unsigned deverror, unsigned errcode,
  unsigned _far *devhdr);
void lee_cfg(char *argv0);
void esconde_cursor(void);
int analiza_args(int argc, char *argv[]);
void construye_nfexe(void);
void imprime_error(char *msg);
void copia_fichero(int horg, int hdest);
void linkar(void);
