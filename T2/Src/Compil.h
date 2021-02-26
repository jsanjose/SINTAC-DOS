/*****************************************
	Fichero de cabecera con los
	prototipos de las funciones
	de apoyo al compilador.
*****************************************/

#if !defined (COMPIL_H)
#define COMPIL_H

/*** Prototipos de funciones de COMPIL.C ***/
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
  unsigned *desp_men, BYTE *n_men, unsigned *bytes_men);
int esta_en_voc(struct palabra *vocab, int pvoc, char *pal);
STC_ERR compila_loc(FILE *f_loc, char *loc, unsigned mem_loc, BYTE max_loc,
  unsigned *desp_loc, BYTE *conx, unsigned mem_conx, unsigned *desp_conx,
  struct palabra *vocabulario, int pal_voc, BYTE v_mov, BYTE *n_loc,
  unsigned *bytes_loc, unsigned *bytes_conx);
STC_ERR compila_obj(FILE *f_obj, char *obj, unsigned mem_obj, BYTE max_obj,
  unsigned *desp_obj, struct palabra *vocabulario, int pal_voc, BYTE n_loc,
  BYTE *n_obj, unsigned *bytes_obj);
BYTE es_band1(char c);
STC_ERR compila_pro(FILE *f_pro, BYTE *pro, unsigned mem_pro, BYTE max_pro,
  unsigned *desp_pro, struct palabra *vocabulario, int pal_voc, BYTE n_loc,
  BYTE n_msy, BYTE n_msg, BYTE n_obj, BYTE n_conv, BYTE *n_pro,
  unsigned *bytes_pro);
BOOLEAN coge_nombre_etq(char *(*l), char *etq, int lngbuff);
STC_ERR sust_etiquetas(void);
STC_ERR chequea_pro(BYTE num_pro);
COD_ERR mete_const(char *l, unsigned long linea);
BOOLEAN coge_const(char *cnst, BYTE *valor);

/* funciones para compilar condactos */
BOOLEAN sgte_campo(char *(*l));
int mete_fsk(char *etq, BYTE *dir);
COD_ERR compila_par1(char *(*l), BYTE *(*act_pro), BYTE *ult_pro, BYTE codigo,
  BYTE lim1, BYTE lim2, COD_ERR coderr);
COD_ERR compila_par1_pal(char *(*l), BYTE *(*act_pro), BYTE *ult_pro,
  BYTE codigo, int pal_voc, BYTE tipopal, COD_ERR coderr);
COD_ERR compila_par2(char *(*l), BYTE *(*act_pro), BYTE *ult_pro, BYTE codigo,
  BYTE lim11, BYTE lim21, BYTE lim22, COD_ERR coderr1, COD_ERR coderr2);
COD_ERR compila_par3(char *(*l), BYTE *(*act_pro), BYTE *ult_pro, BYTE codigo,
  BYTE lim1, BYTE lim2, COD_ERR coderr1, COD_ERR coderr2);
COD_ERR compila_par6(char *(*l), BYTE *(*act_pro), BYTE *ult_pro, BYTE codigo,
  BYTE lim1, BYTE lim2, COD_ERR coderr1, COD_ERR coderr2);
COD_ERR process(char *(*l), BYTE *(*act_pro), BYTE *ult_pro, BYTE condacto);
COD_ERR defwin(char *(*l), BYTE *(*act_pro), BYTE *ult_pro, BYTE condacto);
COD_ERR skip(char *(*l), BYTE *(*act_pro), BYTE *ulr_pro, BYTE condacto);
COD_ERR synonym(char *(*l), BYTE *(*act_pro), BYTE *ult_pro, BYTE condacto,
  int pal_voc, BYTE n_conv);

#endif  /* COMPIL_H */

