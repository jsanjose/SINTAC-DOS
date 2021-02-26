/***************************************
	Fichero de cabecera principal
	del listador SINTAC
***************************************/

/*** Constantes ***/
#define CURSOR          0x0607  /* tipo de cursor */

/*** Prototipos ***/
void _far int24_hnd(unsigned deverror, unsigned errcode,
  unsigned _far *devhdr);
void lee_cfg(char *argv0);
void acerca_de(void);
void codifica(BYTE *mem, unsigned bytes_mem);
int saca_pal(BYTE num_pal, BYTE tipo_pal);
void fprint_bin(FILE *f, BYTE b);
char mayuscula(char c);
int npar_condacto(BYTE cond);
int carga_bd(char *nombre_fich);
void info_bd(void);
void lista_voc(void);
void lista_msy(void);
void lista_msg(void);
void lista_loc(void);
void lista_obj(void);
void lista_pro(void);
