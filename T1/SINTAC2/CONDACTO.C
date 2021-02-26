#include <stdlib.h>     /* atoi() */
#include <stddef.h>     /* ptrdiff_t */
#include <string.h>     /* strcmp() */
#include "sintac.h"
#include "condacto.h"

/*** Prototipos de funciones auxiliares externas a este m¢dulo ***/
extern char mayuscula(char c);
extern BOOLEAN fin_linea(char c);
extern char *salta_espacios(char *s);
extern char *hasta_espacio(char *s);
extern BOOLEAN esta_en(const char *s, char c);
extern COD_ERR coge_pal(char *l, char *pal);
extern COD_ERR coge_num(char *l, BYTE *n);
extern int esta_en_voc(struct palabra *vocab, int pvoc, char *pal);
extern BOOLEAN coge_nombre_etq(char *(*l), char *etq, int lngbuff);

/*** Prototipos de funciones internas ***/
BOOLEAN sgte_campo(char *(*l));
int mete_fsk(char *etq,BYTE *dir);
COD_ERR compila_par1(char *(*l), BYTE *(*act_pro), BYTE codigo, BYTE lim1,
  BYTE lim2, COD_ERR coderr);
COD_ERR compila_par1_pal(char *(*l), BYTE *(*act_pro), BYTE codigo,
  BYTE tipopal, COD_ERR coderr);
COD_ERR compila_par2(char *(*l), BYTE *(*act_pro), BYTE codigo, BYTE lim11,
  BYTE lim21, BYTE lim22, COD_ERR coderr1, COD_ERR coderr2);
COD_ERR compila_par3(char *(*l), BYTE *(*act_pro), BYTE codigo, BYTE lim1,
  BYTE lim2, COD_ERR coderr1, COD_ERR coderr2);
COD_ERR compila_par6(char *(*l), BYTE *(*act_pro), BYTE codigo, BYTE lim1,
  BYTE lim2, COD_ERR coderr1, COD_ERR coderr2);

/* Variables externas */
extern const char *Abecedario;
extern const char *Numeros;
extern const char *Signos;
extern int pt_etiq;
extern int pt_skip;
extern int pprc;
extern STC_ETIQUETA etiqueta[LABELS];
extern STC_SKPFORWARD fskip[FSKIP];
extern STC_PRCFORWARD prc[FPROCESS];
extern BYTE num_msg;
extern BYTE num_msy;
extern BYTE num_loc;
extern BYTE num_obj;
extern struct palabra vocabulario[NUM_PAL];
extern int pal_voc;
extern BYTE n_conv;
extern unsigned long numlin_fich_entr;
extern BYTE num_pro_act;

/*****************************************************************************
	SGTE_CAMPO: desplaza puntero de l¡nea al siguiente campo.
	  Entrada:      'l' puntero a l¡nea de entrada (quedar  actualizado)
	  Salida:       TRUE si no hay m s campos
			FALSE si hay m s campos
*****************************************************************************/
BOOLEAN sgte_campo(char *(*l))
{
*l=hasta_espacio(*l);
*l=salta_espacios(*l);

/* comprueba si hay m s campos */
if(fin_linea(*(*l))) return(TRUE);

return(FALSE);
}

/*****************************************************************************
	METE_FSK: introduce etiqueta y su direcci¢n de sustituci¢n para una
	  llamada 'forward' con SKIP.
	  Entrad:       'etq' etiqueta a introducir
			'dir' direcci¢n d¢nde meter el valor de la etiqueta
			cuando se calcule
	  Salida:       TRUE si la tabla de sustituci¢n de SKIP est  llena
			FALSE si no hubo error
*****************************************************************************/
int mete_fsk(char *etq,BYTE *dir)
{
int i;

/* comprueba si se rebosa la tabla */
if(pt_skip>=FSKIP) return(TRUE);

/* guarda la etiqueta, su direcci¢n y la l¡nea dentro de fichero de entrada */
for(i=0; i<LONGETQ; i++) fskip[pt_skip].etq[i]=etq[i];
fskip[pt_skip].fsk=dir;
fskip[pt_skip].nl=numlin_fich_entr;
pt_skip++;

return(FALSE);
}

/*****************************************************************************
	COMPILA_PAR1: compila condactos con un par metro.
	  Entrada:      'l' puntero al puntero a la l¡nea con el par metro
			del condacto
			'act_pro' puntero a d¢nde dejar condacto compilado
			'codigo' c¢digo num‚rico del condacto
			'lim1', 'lim2' l¡mites para el par metro (NOTA: meter
			255 y 0 respectivamente para saltar comprobaci¢n)
			'coderr' c¢digo de error en caso de que el par metro
			sea mayor que 'lim1' y menor que 'lim2'
	  Salida:       'l' y 'act_pro' actualizados.
			c¢digo de error
*****************************************************************************/
COD_ERR compila_par1(char *(*l), BYTE *(*act_pro), BYTE codigo, BYTE lim1,
  BYTE lim2, COD_ERR coderr)
{
BYTE par;
COD_ERR indir;

if(sgte_campo(l)) return(_E_FCAM);
indir=coge_num(*l,&par);
/* si hubo error al coger par metro 1, sale */
if((indir!=_E_NERR) && (indir!=_E_CIND)) return(indir);

/* comprueba que sea mayor que 'lim1' y menor que 'lim2' */
if((indir!=_E_CIND) && (par>lim1) && (par<lim2)) return(coderr);
if(indir==_E_CIND) *(*act_pro)++=INDIR1;

*(*act_pro)++=codigo;
*(*act_pro)++=par;

return(_E_NERR);
}

/*****************************************************************************
	COMPILA_PAR1_PAL: compila condactos con un par metro, el cual es
	  una palabra del vocabulario.
	  Entrada:      'l' puntero al puntero a la l¡nea con el par metro
			del condacto
			'act_pro' puntero a d¢nde dejar condacto compilado
			'codigo' c¢digo num‚rico del condacto
			'tipopal' tipo de palabra esperada (_VERB, _NOMB,
			_ADJT, _CONJ)
			'coderr' c¢digo de error en caso de que la palabra
			encontrada no sea del tipo especificado por 'tipopal'
	  Salida:       'l' y 'act_pro' actualizados.
			c¢digo de error
*****************************************************************************/
COD_ERR compila_par1_pal(char *(*l), BYTE *(*act_pro), BYTE codigo,
  BYTE tipopal, COD_ERR coderr)
{
int numpal;
char palabra[LONGPAL];

if(sgte_campo(l)) return(_E_FCAM);
if(coge_pal(*l,palabra)==_E_CVOC) return(_E_CVOC);

/* saca la palabra del vocabulario */
if((numpal=esta_en_voc(vocabulario,pal_voc,palabra))==(NUM_PAL+1))
  return(_E_NPVC);

/* comprueba que sea del tipo especificado */
if(vocabulario[numpal].tipo!=tipopal) return(coderr);

*(*act_pro)++=codigo;
*(*act_pro)++=vocabulario[numpal].num;

return(_E_NERR);
}

/*****************************************************************************
	COMPILA_PAR2: compila condactos con dos par metros.
	  Entrada:      'l' puntero al puntero a la l¡nea con el par metro
			del condacto
			'act_pro' puntero a d¢nde dejar condacto compilado
			'codigo' c¢digo num‚rico del condacto
			'lim11' l¡mite para el par metro 1 (NOTA: meter
			255 para saltar comprobaci¢n)
			'lim21', 'lim22' l¡mites para el par metro 2 (NOTA:
			meter 255 y 0 respectivamente para saltar
			comprobaci¢n)
			'coderr1' c¢digo de error en caso de que el par metro
			1 sea mayor que 'lim11'
			'coderr2' c¢digo de error en caso de que el par metro
			2 sea mayor que 'lim21' y menor que 'lim22'
	  Salida:       'l' y 'act_pro' actualizados.
			c¢digo de error
*****************************************************************************/
COD_ERR compila_par2(char *(*l), BYTE *(*act_pro), BYTE codigo, BYTE lim11,
  BYTE lim21, BYTE lim22, COD_ERR coderr1, COD_ERR coderr2)
{
BYTE par1, par2;
COD_ERR indir1, indir2;

if(sgte_campo(l)) return(_E_FCAM);
indir1=coge_num(*l,&par1);
/* si hubo error al coger el par metro 1, sale */
if((indir1!=_E_NERR) && (indir1!=_E_CIND)) return(indir1);
/* comprueba que sea mayor que 'lim11' */
if((indir1!=_E_CIND) && (par1>lim11)) return(coderr1);

if(sgte_campo(l)) return(_E_FCAM);
indir2=coge_num(*l,&par2);
/* si hubo error al coger el par metro 2, sale */
if((indir2!=_E_NERR) && (indir2!=_E_CIND)) return(indir2);
/* comprueba que sea mayor que 'lim21' y menor que 'lim22' */
if((indir2!=_E_CIND) && (par2>lim21) && (par2<lim22)) return(coderr2);

if((indir1==_E_CIND) && (indir2!=_E_CIND)) *(*act_pro)++=INDIR1;
else if((indir1!=_E_CIND) && (indir2==_E_CIND)) *(*act_pro)++=INDIR2;
else if((indir1==_E_CIND) && (indir2==_E_CIND)) *(*act_pro)++=INDIR12;

*(*act_pro)++=codigo;
*(*act_pro)++=par1;
*(*act_pro)++=par2;

return(_E_NERR);
}

/*****************************************************************************
	COMPILA_PAR3: compila condactos con tres par metros.
	  Entrada:      'l' puntero al puntero a la l¡nea con el par metro
			del condacto
			'act_pro' puntero a d¢nde dejar condacto compilado
			'codigo' c¢digo num‚rico del condacto
			'lim1' l¡mite para el par metro 1 (NOTA: meter
			255 para saltar comprobaci¢n)
			'lim2' l¡mite para el par metro 2 (NOTA: meter
			255 para saltar comprobaci¢n)
			'coderr1' c¢digo de error en caso de que el par metro
			1 sea mayor que 'lim1'
			'coderr2' c¢digo de error en caso de que el par metro
			2 sea mayor que 'lim2'
	  Salida:       'l' y 'act_pro' actualizados.
			c¢digo de error
*****************************************************************************/
COD_ERR compila_par3(char *(*l), BYTE *(*act_pro), BYTE codigo, BYTE lim1,
  BYTE lim2, COD_ERR coderr1, COD_ERR coderr2)
{
COD_ERR par3err;
BYTE par3;

/* compila con los dos primeros par metros */
par3err=compila_par2(l,act_pro,codigo,lim1,lim2,0,coderr1,coderr2);
/* si hubo error al compilarlos, sale */
if(par3err) return(par3err);

if(sgte_campo(l)) return(_E_FCAM);
/* coge el tercer par metro */
par3err=coge_num(*l,&par3);
/* si hubo error al cogerlo, sale */
if(par3err) return(par3err);

*(*act_pro)++=par3;

return(_E_NERR);
}

/*****************************************************************************
	COMPILA_PAR6: compila condactos con seis par metros.
	  Entrada:      'l' puntero al puntero a la l¡nea con el par metro
			del condacto
			'act_pro' puntero a d¢nde dejar condacto compilado
			'codigo' c¢digo num‚rico del condacto
			'lim1' l¡mite para el par metro 1 (NOTA: meter
			255 para saltar comprobaci¢n)
			'lim2' l¡mite para el par metro 2 (NOTA: meter
			255 para saltar comprobaci¢n)
			'coderr1' c¢digo de error en caso de que el par metro
			1 sea mayor que 'lim1'
			'coderr2' c¢digo de error en caso de que el par metro
			2 sea mayor que 'lim2'
	  Salida:       'l' y 'act_pro' actualizados.
			c¢digo de error
*****************************************************************************/
COD_ERR compila_par6(char *(*l), BYTE *(*act_pro), BYTE codigo, BYTE lim1,
  BYTE lim2, COD_ERR coderr1, COD_ERR coderr2)
{
COD_ERR par6err;
BYTE par3, par4, par5, par6;

/* compila con los dos primeros par metros */
par6err=compila_par2(l,act_pro,codigo,lim1,lim2,0,coderr1,coderr2);
/* si hubo error al compilarlos, sale */
if(par6err) return(par6err);

if(sgte_campo(l)) return(_E_FCAM);
/* coge el tercer par metro */
par6err=coge_num(*l,&par3);
/* si hubo error al cogerlo, sale */
if(par6err) return(par6err);

if(sgte_campo(l)) return(_E_FCAM);
/* coge el cuarto par metro */
par6err=coge_num(*l,&par4);
/* si hubo error al cogerlo, sale */
if(par6err) return(par6err);

if(sgte_campo(l)) return(_E_FCAM);
/* coge el quinto par metro */
par6err=coge_num(*l,&par5);
/* si hubo error al cogerlo, sale */
if(par6err) return(par6err);

if(sgte_campo(l)) return(_E_FCAM);
/* coge el sexto par metro */
par6err=coge_num(*l,&par6);
/* si hubo error al cogerlo, sale */
if(par6err) return(par6err);

*(*act_pro)++=par3;
*(*act_pro)++=par4;
*(*act_pro)++=par5;
*(*act_pro)++=par6;

return(_E_NERR);
}

/*****************************************************************************
	Funciones para compilar cada uno de los condactos.
*****************************************************************************/
COD_ERR process(char *(*l), BYTE *(*act_pro))
{
BYTE par1;
COD_ERR indir;

if(sgte_campo(l)) return(_E_FCAM);
indir=coge_num(*l,&par1);
if((indir!=_E_NERR) && (indir!=_E_CIND)) return(indir);
if((indir!=_E_CIND) && (par1==num_pro_act)) return(_E_PRPR);
if(indir==_E_CIND) *(*act_pro)++=INDIR1;
*(*act_pro)++=1;
*(*act_pro)++=par1;

/* si se llama a un proceso no definido guardar  informaci¢n sobre esa */
/* llamada en una tabla para resolverla al final de la compilaci¢n */
if((indir!=_E_CIND) && (par1>num_pro_act)) {
	/* comprueba si rrebosa la tabla */
	if(pprc>=FPROCESS) return(_E_PRRB);
	/* si no, guarda el n£mero del proceso al que se llama */
	/* y el n£mero de l¡nea en el archivo de entrada */
	prc[pprc].numpro=par1;
	prc[pprc].nl=numlin_fich_entr;
	pprc++;
}

return(_E_NERR);
}

COD_ERR done(char *(*l), BYTE *(*act_pro))
{
*(*act_pro)++=2;
return(_E_NERR);
}

COD_ERR notdone(char *(*l), BYTE *(*act_pro))
{
*(*act_pro)++=3;
return(_E_NERR);
}

COD_ERR resp(char *(*l), BYTE *(*act_pro))
{
*(*act_pro)++=4;
return(_E_NERR);
}

COD_ERR noresp(char *(*l), BYTE *(*act_pro))
{
*(*act_pro)++=5;
return(_E_NERR);
}

COD_ERR defwin(char *(*l), BYTE *(*act_pro))
{
COD_ERR err;
BYTE wy, wx, lx, ly;

/* compila y si hubo error, sale */
err=compila_par6(l,act_pro,6,N_VENT-1,255,_E_WPNW,_E_NERR);
if(err) return(err);

/* recoge las dimensiones de la ventana */
wy=*(*act_pro-4);
wx=*(*act_pro-3);
lx=*(*act_pro-2);
ly=*(*act_pro-1);

/* comprueba las dimensiones de la ventana */
if(wx>COLUMNAS) return(_E_WPWX);
if(wy>FILAS) return(_E_WPWY);
if(wx+lx>COLUMNAS) return(_E_WPFP);
if(wy+ly>FILAS) return(_E_WPFP);

return(err);
}

COD_ERR window(char *(*l), BYTE *(*act_pro))
{
COD_ERR err;

err=compila_par1(l,act_pro,7,N_VENT-1,0,_E_WPNW);
return(err);
}

COD_ERR clw(char *(*l), BYTE *(*act_pro))
{
COD_ERR err;

err=compila_par1(l,act_pro,8,N_VENT-1,0,_E_WPNW);
return(err);
}

COD_ERR let(char *(*l), BYTE *(*act_pro))
{
COD_ERR err;

err=compila_par2(l,act_pro,9,255,255,0,_E_NERR,_E_NERR);
return(err);
}

COD_ERR eq(char *(*l), BYTE *(*act_pro))
{
COD_ERR err;

err=compila_par2(l,act_pro,10,255,255,0,_E_NERR,_E_NERR);
return(err);
}

COD_ERR noteq(char *(*l), BYTE *(*act_pro))
{
COD_ERR err;

err=compila_par2(l,act_pro,11,255,255,0,_E_NERR,_E_NERR);
return(err);
}

COD_ERR lt(char *(*l), BYTE *(*act_pro))
{
COD_ERR err;

err=compila_par2(l,act_pro,12,255,255,0,_E_NERR,_E_NERR);
return(err);
}

COD_ERR gt(char *(*l), BYTE *(*act_pro))
{
COD_ERR err;

err=compila_par2(l,act_pro,13,255,255,0,_E_NERR,_E_NERR);
return(err);
}

COD_ERR mes(char *(*l), BYTE *(*act_pro))
{
COD_ERR err;

err=compila_par1(l,act_pro,14,(BYTE)(num_msg-1),0,_E_MPNM);
return(err);
}

COD_ERR newline(char *(*l), BYTE *(*act_pro))
{
*(*act_pro)++=15;
return(_E_NERR);
}

COD_ERR message(char *(*l), BYTE *(*act_pro))
{
COD_ERR err;

err=compila_par1(l,act_pro,16,(BYTE)(num_msg-1),0,_E_MPNM);
return(err);
}

COD_ERR sysmess(char *(*l), BYTE *(*act_pro))
{
COD_ERR err;

err=compila_par1(l,act_pro,17,(BYTE)(num_msy-1),0,_E_MPNM);
return(err);
}

COD_ERR desc(char *(*l), BYTE *(*act_pro))
{
COD_ERR err;

err=compila_par1(l,act_pro,18,(BYTE)(num_loc-1),0,_E_MPNL);
return(err);
}

COD_ERR add(char *(*l), BYTE *(*act_pro))
{
COD_ERR err;

err=compila_par2(l,act_pro,19,255,255,0,_E_NERR,_E_NERR);
return(err);
}

COD_ERR sub(char *(*l), BYTE *(*act_pro))
{
COD_ERR err;

err=compila_par2(l,act_pro,20,255,255,0,_E_NERR,_E_NERR);
return(err);
}

COD_ERR inc(char *(*l), BYTE *(*act_pro))
{
COD_ERR err;

err=compila_par1(l,act_pro,21,255,0,_E_NERR);
return(err);
}

COD_ERR dec(char *(*l), BYTE *(*act_pro))
{
COD_ERR err;

err=compila_par1(l,act_pro,22,255,0,_E_NERR);
return(err);
}

COD_ERR set(char *(*l), BYTE *(*act_pro))
{
COD_ERR err;

err=compila_par1(l,act_pro,23,255,0,_E_NERR);
return(err);
}

COD_ERR clear(char *(*l), BYTE *(*act_pro))
{
COD_ERR err;

err=compila_par1(l,act_pro,24,255,0,_E_NERR);
return(err);
}

COD_ERR zero(char *(*l), BYTE *(*act_pro))
{
COD_ERR err;

err=compila_par1(l,act_pro,25,255,0,_E_NERR);
return(err);
}

COD_ERR notzero(char *(*l), BYTE *(*act_pro))
{
COD_ERR err;

err=compila_par1(l,act_pro,26,255,0,_E_NERR);
return(err);
}

COD_ERR place(char *(*l), BYTE *(*act_pro))
{
COD_ERR err;

err=compila_par2(l,act_pro,27,(BYTE)(num_obj-1),(BYTE)(num_loc-1),NO_CREADO,
  _E_OPNV,_E_MPNL);
return(err);
}

COD_ERR get(char *(*l), BYTE *(*act_pro))
{
COD_ERR err;

err=compila_par1(l,act_pro,28,(BYTE)(num_obj-1),0,_E_OPNV);
return(err);
}

COD_ERR drop(char *(*l), BYTE *(*act_pro))
{
COD_ERR err;

err=compila_par1(l,act_pro,29,(BYTE)(num_obj-1),0,_E_OPNV);
return(err);
}

COD_ERR input(char *(*l), BYTE *(*act_pro))
{
*(*act_pro)++=30;
return(_E_NERR);
}

COD_ERR parse(char *(*l), BYTE *(*act_pro))
{
*(*act_pro)++=31;
return(_E_NERR);
}

COD_ERR skip(char *(*l), BYTE *(*act_pro))
{
char etq[LONGETQ];
int i;
unsigned desplu;
int despli;

if(sgte_campo(l)) return(_E_FCAM);

/* comprueba si lo que sigue es una etiqueta */
/* para lo cual debe empezar con el car cter MARCA_ETQ */
if(*(*l)!=MARCA_ETQ) return(_E_NLAB);
(*l)++;

/* recoge la etiqueta */
if(coge_nombre_etq(l,etq,LONGETQ)) return(_E_ENVL);

/* si no hay ninguna etiqueta definida, la mete en tabla sin m s */
if(!pt_etiq) {
	mete_fsk(etq,*act_pro+1);
	*(*act_pro)++=32;
	/* inicializa a 0, pero luego ser  sustituido */
	/* por su desplazamiento correspondiente */
	*(*act_pro)++=0;
	*(*act_pro)++=0;
}
else {
	/* mira a ver si la etiqueta ya existe */
	for(i=0; i<pt_etiq; i++) {
		if(!strcmp(etq,etiqueta[i].etq)) break;
	}
	/* si la etiqueta no existe la guarda */
	if(i==pt_etiq) {
		if(mete_fsk(etq,*act_pro+1)) return(_E_RBTS);
		*(*act_pro)++=32;
		/* inicializa a 0, pero luego ser  sustituido */
		/* por su desplazamiento correspondiente */
		*(*act_pro)++=0;
		*(*act_pro)++=0;
	}
	else {
		*(*act_pro)++=32;
		/* si la etiqueta fue definida, recupera su valor */
		desplu=(unsigned)(*act_pro-1-etiqueta[i].petq);
		if(desplu>(unsigned)32768) return(_E_LBFR);
		despli=-desplu;
		/* guarda byte bajo y byte alto */
		*(*act_pro)++=(BYTE)(despli & 0x00ff);
		*(*act_pro)++=(BYTE)((despli >> 8) & 0x00ff);
	}
}
return(_E_NERR);
}

COD_ERR at(char *(*l), BYTE *(*act_pro))
{
COD_ERR err;

err=compila_par1(l,act_pro,33,255,0,_E_NERR);
return(err);
}

COD_ERR notat(char *(*l), BYTE *(*act_pro))
{
COD_ERR err;

err=compila_par1(l,act_pro,34,255,0,_E_NERR);
return(err);
}

COD_ERR atgt(char *(*l), BYTE *(*act_pro))
{
COD_ERR err;

err=compila_par1(l,act_pro,35,255,0,_E_NERR);
return(err);
}

COD_ERR atlt(char *(*l), BYTE *(*act_pro))
{
COD_ERR err;

err=compila_par1(l,act_pro,36,255,0,_E_NERR);
return(err);
}

COD_ERR adject1(char *(*l), BYTE *(*act_pro))
{
COD_ERR err;

err=compila_par1_pal(l,act_pro,37,_ADJT,_E_NCAD);
return(err);
}

COD_ERR noun2(char *(*l), BYTE *(*act_pro))
{
COD_ERR err;

err=compila_par1_pal(l,act_pro,38,_NOMB,_E_NNMB);
return(err);
}

COD_ERR adject2(char *(*l), BYTE *(*act_pro))
{
COD_ERR err;

err=compila_par1_pal(l,act_pro,39,_ADJT,_E_NCAD);
return(err);
}

COD_ERR listat(char *(*l), BYTE *(*act_pro))
{
COD_ERR err;

err=compila_par1(l,act_pro,40,(BYTE)(num_loc-1),NO_CREADO,_E_MPNL);
return(err);
}

COD_ERR isat(char *(*l), BYTE *(*act_pro))
{
COD_ERR err;

err=compila_par2(l,act_pro,41,(BYTE)(num_obj-1),(BYTE)(num_loc-1),NO_CREADO,
  _E_OPNV,_E_MPNL);
return(err);
}

COD_ERR isnotat(char *(*l), BYTE *(*act_pro))
{
COD_ERR err;

err=compila_par2(l,act_pro,42,(BYTE)(num_obj-1),(BYTE)(num_loc-1),NO_CREADO,
  _E_OPNV,_E_MPNL);
return(err);
}

COD_ERR present(char *(*l), BYTE *(*act_pro))
{
COD_ERR err;

err=compila_par1(l,act_pro,43,(BYTE)(num_obj-1),0,_E_OPNV);
return(err);
}

COD_ERR absent(char *(*l), BYTE *(*act_pro))
{
COD_ERR err;

err=compila_par1(l,act_pro,44,(BYTE)(num_obj-1),0,_E_OPNV);
return(err);
}

COD_ERR worn(char *(*l), BYTE *(*act_pro))
{
COD_ERR err;

err=compila_par1(l,act_pro,45,(BYTE)(num_obj-1),0,_E_OPNV);
return(err);
}

COD_ERR notworn(char *(*l), BYTE *(*act_pro))
{
COD_ERR err;

err=compila_par1(l,act_pro,46,(BYTE)(num_obj-1),0,_E_OPNV);
return(err);
}

COD_ERR carried(char *(*l), BYTE *(*act_pro))
{
COD_ERR err;

err=compila_par1(l,act_pro,47,(BYTE)(num_obj-1),0,_E_OPNV);
return(err);
}

COD_ERR notcarr(char *(*l), BYTE *(*act_pro))
{
COD_ERR err;

err=compila_par1(l,act_pro,48,(BYTE)(num_obj-1),0,_E_OPNV);
return(err);
}

COD_ERR wear(char *(*l), BYTE *(*act_pro))
{
COD_ERR err;

err=compila_par1(l,act_pro,49,(BYTE)(num_obj-1),0,_E_OPNV);
return(err);
}

COD_ERR remove1(char *(*l), BYTE *(*act_pro))
{
COD_ERR err;

err=compila_par1(l,act_pro,50,(BYTE)(num_obj-1),0,_E_OPNV);
return(err);
}

COD_ERR create(char *(*l), BYTE *(*act_pro))
{
COD_ERR err;

err=compila_par1(l,act_pro,51,(BYTE)(num_obj-1),0,_E_OPNV);
return(err);
}

COD_ERR destroy(char *(*l), BYTE *(*act_pro))
{
COD_ERR err;

err=compila_par1(l,act_pro,52,(BYTE)(num_obj-1),0,_E_OPNV);
return(err);
}

COD_ERR swap(char *(*l), BYTE *(*act_pro))
{
COD_ERR err;

err=compila_par2(l,act_pro,53,(BYTE)(num_obj-1),(BYTE)(num_obj-1),0,
  _E_OPNV,_E_OPNV);
return(err);
}

COD_ERR restart(char *(*l), BYTE *(*act_pro))
{
*(*act_pro)++=54;
return(_E_NERR);
}

COD_ERR whato(char *(*l), BYTE *(*act_pro))
{
*(*act_pro)++=55;
return(_E_NERR);
}

COD_ERR move(char *(*l), BYTE *(*act_pro))
{
COD_ERR err;

err=compila_par1(l,act_pro,56,255,0,_E_NERR);
return(err);
}

COD_ERR ismov(char *(*l), BYTE *(*act_pro))
{
*(*act_pro)++=57;
return(_E_NERR);
}

COD_ERR goto1(char *(*l), BYTE *(*act_pro))
{
COD_ERR err;

err=compila_par1(l,act_pro,58,(BYTE)(num_loc-1),0,_E_MPNL);
return(err);
}

COD_ERR print(char *(*l), BYTE *(*act_pro))
{
COD_ERR err;

err=compila_par1(l,act_pro,59,255,0,_E_NERR);
return(err);
}

COD_ERR dprint(char *(*l), BYTE *(*act_pro))
{
COD_ERR err;

err=compila_par1(l,act_pro,60,255,0,_E_NERR);
return(err);
}

COD_ERR cls(char *(*l), BYTE *(*act_pro))
{
*(*act_pro)++=61;
return(_E_NERR);
}

COD_ERR anykey(char *(*l), BYTE *(*act_pro))
{
*(*act_pro)++=62;
return(_E_NERR);
}

COD_ERR pause(char *(*l), BYTE *(*act_pro))
{
COD_ERR err;

err=compila_par1(l,act_pro,63,255,0,_E_NERR);
return(err);
}

COD_ERR listobj(char *(*l), BYTE *(*act_pro))
{
*(*act_pro)++=64;
return(_E_NERR);
}

COD_ERR firsto(char *(*l), BYTE *(*act_pro))
{
*(*act_pro)++=65;
return(_E_NERR);
}

COD_ERR nexto(char *(*l), BYTE *(*act_pro))
{
COD_ERR err;

err=compila_par1(l,act_pro,66,(BYTE)(num_loc-1),NO_CREADO,_E_MPNL);
return(err);
}

COD_ERR synonym(char *(*l), BYTE *(*act_pro))
{
BYTE verb, nomb;
char pal[LONGPAL];
int i, errct;

if(sgte_campo(l)) return(_E_FCAM);

/* recoge campo verbo */
errct=coge_pal(*l,pal);

/* comprueba si es palabra nula (CHR_NOPAL) */
/* si lo es mete NO_PAL */
if(*pal==CHR_NOPAL) verb=NO_PAL;
else {
	/* si hubo error al coger palabra, sale */
	if(errct) return(errct);

	/* comprueba si est  en vocabulario */
	if((i=esta_en_voc(vocabulario,pal_voc,pal))==NUM_PAL+1)
	  return(_E_NPVC);

	/* si no es verbo ni nombre convertible, error */
	if((vocabulario[i].tipo!=_VERB) &&
	  ((vocabulario[i].tipo==_NOMB) && (vocabulario[i].num>=n_conv)))
	  return(_E_NVNC);

	verb=vocabulario[i].num;
}
if(sgte_campo(l)) return(_E_FCAM);

/* recoge campo nombre */
errct=coge_pal(*l,pal);

/* comprueba si es palabra nula (CHR_NOPAL) */
/* si lo es mete NO_PAL */
if(*pal==CHR_NOPAL) nomb=NO_PAL;
else {
	/* si hubo error al coger palabra, sale */
	if(errct==_E_CVOC) return(errct);

	/* comprueba si est  en vocabulario */
	if((i=esta_en_voc(vocabulario,pal_voc,pal))==NUM_PAL+1)
	  return(_E_NPVC);

	/* si no es nombre, error */
	if(vocabulario[i].tipo!=_NOMB) return(_E_NNNN);

	nomb=vocabulario[i].num;
}
*(*act_pro)++=67;
*(*act_pro)++=verb;
*(*act_pro)++=nomb;
return(_E_NERR);
}

COD_ERR hasat(char *(*l), BYTE *(*act_pro))
{
COD_ERR err;

err=compila_par1(l,act_pro,68,17,0,_E_NVHT);
return(err);
}

COD_ERR hasnat(char *(*l), BYTE *(*act_pro))
{
COD_ERR err;

err=compila_par1(l,act_pro,69,17,0,_E_NVHT);
return(err);
}

COD_ERR light(char *(*l), BYTE *(*act_pro))
{
*(*act_pro)++=70;
return(_E_NERR);
}

COD_ERR nolight(char *(*l), BYTE *(*act_pro))
{
*(*act_pro)++=71;
return(_E_NERR);
}

COD_ERR random(char *(*l), BYTE *(*act_pro))
{
COD_ERR err;

err=compila_par2(l,act_pro,72,255,255,0,_E_NERR,_E_NERR);
return(err);
}

COD_ERR seed(char *(*l), BYTE *(*act_pro))
{
COD_ERR err;

err=compila_par1(l,act_pro,73,255,0,_E_NERR);
return(err);
}

COD_ERR puto(char *(*l), BYTE *(*act_pro))
{
COD_ERR err;

err=compila_par1(l,act_pro,74,(BYTE)(num_loc-1),NO_CREADO,_E_MPNL);
return(err);
}

COD_ERR inkey(char *(*l), BYTE *(*act_pro))
{
*(*act_pro)++=75;
return(_E_NERR);
}

COD_ERR copyov(char *(*l), BYTE *(*act_pro))
{
COD_ERR err;

err=compila_par2(l,act_pro,76,(BYTE)(num_obj-1),255,0,_E_OPNV,_E_NERR);
return(err);
}

COD_ERR chance(char *(*l), BYTE *(*act_pro))
{
COD_ERR err;

err=compila_par1(l,act_pro,77,100,0,_E_CHNV);
return(err);
}

COD_ERR ramsave(char *(*l), BYTE *(*act_pro))
{
COD_ERR err;

err=compila_par1(l,act_pro,78,BANCOS_RAM-1,0,_E_RBNV);
return(err);
}

COD_ERR ramload(char *(*l), BYTE *(*act_pro))
{
COD_ERR err;

err=compila_par3(l,act_pro,79,BANCOS_RAM-1,255,_E_RBNV,_E_NERR);
return(err);
}

COD_ERR ability(char *(*l), BYTE *(*act_pro))
{
COD_ERR err;

err=compila_par1(l,act_pro,80,255,0,_E_NERR);
return(err);
}

COD_ERR autog(char *(*l), BYTE *(*act_pro))
{
*(*act_pro)++=81;
return(_E_NERR);
}

COD_ERR autod(char *(*l), BYTE *(*act_pro))
{
*(*act_pro)++=82;
return(_E_NERR);
}

COD_ERR autow(char *(*l), BYTE *(*act_pro))
{
*(*act_pro)++=83;
return(_E_NERR);
}

COD_ERR autor(char *(*l), BYTE *(*act_pro))
{
*(*act_pro)++=84;
return(_E_NERR);
}

COD_ERR isdoall(char *(*l), BYTE *(*act_pro))
{
*(*act_pro)++=85;
return(_E_NERR);
}

COD_ERR ask(char *(*l), BYTE *(*act_pro))
{
COD_ERR err;

err=compila_par3(l,act_pro,86,(BYTE)(num_msy-1),(BYTE)(num_msy-1),_E_MPNP,
  _E_MPNS);
return(err);
}

COD_ERR quit(char *(*l), BYTE *(*act_pro))
{
*(*act_pro)++=87;
return(_E_NERR);
}

COD_ERR save(char *(*l), BYTE *(*act_pro))
{
*(*act_pro)++=88;
return(_E_NERR);
}

COD_ERR load(char *(*l), BYTE *(*act_pro))
{
COD_ERR err;

err=compila_par2(l,act_pro,89,255,255,0,_E_NERR,_E_NERR);
return(err);
}

COD_ERR exit1(char *(*l), BYTE *(*act_pro))
{
COD_ERR err;

err=compila_par1(l,act_pro,90,1,0,_E_NOEX);
return(err);
}

COD_ERR end1(char *(*l), BYTE *(*act_pro))
{
*(*act_pro)++=91;
return(_E_NERR);
}

int printat(char *(*l), BYTE *(*act_pro))
{
COD_ERR err;

err=compila_par2(l,act_pro,92,FILAS-1,COLUMNAS-1,0,_E_EFIL,_E_ECOL);
return(err);
}

COD_ERR saveat(char *(*l), BYTE *(*act_pro))
{
*(*act_pro)++=93;
return(_E_NERR);
}

COD_ERR backat(char *(*l), BYTE *(*act_pro))
{
*(*act_pro)++=94;
return(_E_NERR);
}

COD_ERR newtext(char *(*l), BYTE *(*act_pro))
{
*(*act_pro)++=95;
return(_E_NERR);
}

COD_ERR printc(char *(*l), BYTE *(*act_pro))
{
COD_ERR err;

err=compila_par1(l,act_pro,96,255,0,_E_NERR);
return(err);
}

COD_ERR ink(char *(*l), BYTE *(*act_pro))
{
COD_ERR err;

err=compila_par1(l,act_pro,97,7,0,_E_COLR);
return(err);
}

COD_ERR paper(char *(*l), BYTE *(*act_pro))
{
COD_ERR err;

err=compila_par1(l,act_pro,98,7,0,_E_COLR);
return(err);
}

COD_ERR bright(char *(*l), BYTE *(*act_pro))
{
COD_ERR err;

err=compila_par1(l,act_pro,99,1,0,_E_COLR);
return(err);
}

COD_ERR blink(char *(*l), BYTE *(*act_pro))
{
COD_ERR err;

err=compila_par1(l,act_pro,100,1,0,_E_COLR);
return(err);
}

COD_ERR color(char *(*l), BYTE *(*act_pro))
{
COD_ERR err;

err=compila_par1(l,act_pro,101,255,0,_E_NERR);
return(err);
}

