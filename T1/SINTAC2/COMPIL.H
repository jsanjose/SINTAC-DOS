/*****************************************
	Fichero de cabecera con los
	prototipos de las funciones
	de apoyo al compilador.
*****************************************/

#if !defined (COMPIL_H)
#define COMPIL_H

/*************  Prototipos  *************/
char mayuscula(char c);
BOOLEAN fin_linea(char c);
BOOLEAN es_espacio(char c);
char *salta_espacios(char *s);
char *hasta_espacio(char *s);
BOOLEAN esta_en(const char *s, char c);
unsigned long num_linea(char *(*l));
STC_ERR compila_voc(FILE *f_voc, struct palabra *vocab, int *num_pal);
COD_ERR comp_lin_voc(char *l, struct palabra *p);
COD_ERR coge_pal(char *l, char *pal);
COD_ERR coge_num(char *l, BYTE *n);
BOOLEAN compara_pal(struct palabra *p1, struct palabra *p2);
void aviso(char *men_aviso);
STC_ERR comp_txt(FILE *f_txt, char *(*txt), char *ult_txt, BYTE act,
  BYTE maximo);
STC_ERR compila_men(FILE *f_men, char *men, unsigned mem_men, BYTE max_men,
  ptrdiff_t *desp_men, BYTE *n_men, ptrdiff_t *bytes_men);
int esta_en_voc(struct palabra *vocab, int pvoc, char *pal);
STC_ERR compila_loc(FILE *f_loc, char *loc, unsigned mem_loc, BYTE max_loc,
  ptrdiff_t *desp_loc, BYTE *conx, unsigned mem_conx, ptrdiff_t *desp_conx,
  struct palabra *vocabulario, int pal_voc, BYTE v_mov, BYTE *n_loc,
  ptrdiff_t *bytes_loc, ptrdiff_t *bytes_conx);
STC_ERR compila_obj(FILE *f_obj, char *obj, unsigned mem_obj, BYTE max_obj,
  ptrdiff_t *desp_obj, struct palabra *vocabulario, int pal_voc, BYTE n_loc,
  BYTE *n_obj, ptrdiff_t *bytes_obj);
BYTE es_band1(char c);
STC_ERR compila_pro(FILE *f_pro, BYTE *pro, unsigned mem_pro, BYTE max_pro,
  ptrdiff_t *desp_pro, struct palabra *vocabulario, int pal_voc, BYTE n_loc,
  BYTE n_msy, BYTE n_msg, BYTE n_obj, BYTE n_conv, BYTE *n_pro,
  ptrdiff_t *bytes_pro);
BOOLEAN coge_nombre_etq(char *(*l), char *etq, int lngbuff);
STC_ERR sust_etiquetas(void);
STC_ERR chequea_pro(BYTE num_pro);
COD_ERR mete_const(char *l);
BOOLEAN coge_const(char *cnst, BYTE *valor);

/********  Variables p£blicas  **********/
/* tabla para guardar las constantes definidas durante la compilaci¢n */
extern STC_CONSTANTE constante[NCONST];
extern int pt_const;

#endif  /* COMPIL_H */

