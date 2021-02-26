#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include "sintac.h"
#include "compil.h"
#include "condacto.h"
#include "cnd.h"
#include "tabcond.h"

/* cadenas usadas para comprobar los car cteres de una palabra */
/* de vocabulario, una etiqueta y una constante */
const char *Abecedario="ABCDEFGHIJKLMN¥OPQRSTUVWXYZ";
const char *Numeros="0123456789";
const char *Signos="_+-*";

/* variable global para almacenar el n£mero de l¡nea actual dentro */
/* del fichero de entrada (se usa en algunas funciones de compilar) */
unsigned long numlin_fich_entr=0;

/* variable global para almacenar el n£mero de proceso que est  */
/* siendo compilado (se usa en algunas funciones de compilar) */
BYTE num_pro_act;

/* mensajes usados m s de una vez */
char *Tab_Esp="tabulaci¢n transformada en espacio, ";
char *Lin_Msg="l¡nea %lu\n";
char *Men_Pro="Proceso ";

/* tabla para guardar etiquetas */
STC_ETIQUETA etiqueta[LABELS];
int pt_etiq=0;  /* puntero a tabla */

/* para guardar saltos (SKIP) 'forward' */
STC_SKPFORWARD fskip[FSKIP];
int pt_skip;    /* puntero a tabla */

/* tabla y variables para guardar llamadas a procesos 'forward' */
STC_PRCFORWARD prc[FPROCESS];
int pprc=0;     /* puntero a tabla */

/* tabla para guardar las constantes definidas durante la compilaci¢n */
STC_CONSTANTE constante[NCONST];
int pt_const=0; /* puntero a tabla */


/****************************************************************************
	MAYUSCULA: convierte una letra en may£scula.
	  Entrada:      'c' car cter a convertir
	  Salida:       may£scula del car cter
****************************************************************************/
char mayuscula(char c)
{
if((c>='a') && (c<='z')) return(c-(char)'a'+(char)'A');
switch(c) {
case (char)'¤' :
	c=(char)'¥';
	break;
case (char)' ' :
	c='A';
	break;
case (char)'‚' :
	c='E';
	break;
case (char)'¡' :
	c='I';
	break;
case (char)'¢' :
	c='O';
	break;
case (char)'£' :
case (char)'' :
	c='U';
	break;
}
return(c);
}

/*****************************************************************************
	FIN_LINEA: comprueba si un car cter es indicador de fin de l¡nea.
	  Entrada:      'c' car cter a comprobar
	  Salida:       TRUE si el car cter es fin de l¡nea (retorno de
			carro o \0
			FALSE en otro caso
*****************************************************************************/
BOOLEAN fin_linea(char c)
{
if((c=='\n') || (c=='\0')) return(TRUE);
return(FALSE);
}

/*****************************************************************************
	ES_ESPACIO: comprueba si un car cter es espacio o tabulaci¢n.
	  Entrada:      'c' car cter a comprobar
	  Salida:       TRUE si el car cter es espacio o tabulaci¢n
			FALSE en otro caso
*****************************************************************************/
BOOLEAN es_espacio(char c)
{
if((c==' ') || (c=='\t')) return(TRUE);
return(FALSE);
}

/*****************************************************************************
	SALTA_ESPACIOS: salta los espacios y tabulaciones iniciales
	  de una l¡nea.
	  Entrada:      's' puntero a la l¡nea
	  Salida:       puntero al primer car cter encontrado que no sea
			espacio ni tabulaci¢n
*****************************************************************************/
char *salta_espacios(char *s)
{
while(es_espacio(*s)) s++;
return(s);
}

/*****************************************************************************
	HASTA_ESPACIO: salta los caracteres de una l¡nea, hasta encontar un
	  espacio o el fin de la linea.
	  Entrada:      's' puntero a la l¡nea
	  Salida:       puntero al primer espacio, tabulaci¢n, retorno de
			carro o fin de l¡nea encontrado
*****************************************************************************/
char *hasta_espacio(char *s)
{
while(!es_espacio(*s) && !fin_linea(*s)) s++;
return(s);
}

/*****************************************************************************
	ESTA_EN: comprueba si un car cter est  en una cadena. Devuelve 0 si
	  el car cter no est  en la cadena.
	  Entrada:      's' cadena con la que se comprobar  el car cter
			'c' car cter a comprobar
	  Salida:       TRUE si el car cter est  en la cadena
			FALSE si no
*****************************************************************************/
BOOLEAN esta_en(const char *s, char c)
{
while(*s) {
	if(*s++==c) return(TRUE);
}
return(FALSE);
}

/*****************************************************************************
	NUM_LINEA: recoge el n£mero de l¡nea que hay en todas las l¡neas
	  de los ficheros temporales y lo convierte.
	  Entrada:      'l' direcci¢n del puntero a l¡nea de entrada
	  Salida:       n£mero de l¡nea (tambi‚n lo mete en la variable
			global 'numlin_fich_entr')
			'l' actualizado para que apunte al car cter siguiente
			a los dos puntos (:)
*****************************************************************************/
unsigned long num_linea(char *(*l))
{
int i;
char lnum[40];

for(i=0; *(*l)!=':'; lnum[i++]=*(*l)++);
lnum[i]='\0';
(*l)++;         /* salta ':' */

numlin_fich_entr=atol(lnum);

return(numlin_fich_entr);
}

/*****************************************************************************
	COMPILA_VOC: compila la secci¢n de Vocabulario.
	  Entrada:      'f_voc' fichero conteniendo el vocabulario (se
			supone el fichero ya abierto)
			'vocab' puntero a tabla de vocabulario
			'num_palc' puntero a variable donde colocar el
			n£mero de palabras compiladas
	  Salida:       c¢digo de error si lo hubo
*****************************************************************************/
STC_ERR compila_voc(FILE *f_voc, struct palabra *vocab, int *num_palc)
{
STC_ERR err={_E_NERR,0};
int i;
int ivoc=0;             /* ¡ndice a tabla de vocabulario */
char lin[LONG_LIN];     /* buffer para l¡nea de entrada */
char *l;                /* puntero a buffer */
struct palabra pal;

while(1) {
	if(fgets(lin,LONG_LIN,f_voc)==NULL) {
		/* comprueba si es error, si no lo es quiere decir */
		/* que se lleg¢ al final del fichero, en cuyo caso */
		/* sale del bucle */
		if(ferror(f_voc)) {
			err.codigo=_E_ETMP;
			return(err);
		}
		else break;
	}
	l=lin;  /* apunta al inicio de la l¡nea */

	err.linea=num_linea(&l);

	/* compila el resto de la l¡nea */
	if((err.codigo=comp_lin_voc(l,&pal))!=_E_NERR) return(err);

	/* intenta meter la palabra en la tabla de vocabulario */
	/* comprueba si el vocabulario est  lleno */
	if(ivoc>NUM_PAL) {
		err.codigo=_E_MVOC;
		return(err);
	}

	/* si el vocabulario est  vac¡o mete la palabra sin m s */
	if(!ivoc) {
		for(i=0; i<LONGPAL; i++) vocab[ivoc].p[i]=pal.p[i];
		vocab[ivoc].num=pal.num;
		vocab[ivoc].tipo=pal.tipo;
	}
	else {
		/* mira si la palabra est  repetida */
		i=0;
		do {
			if(!compara_pal(&pal,&vocab[i])) {
				err.codigo=_E_PREP;
				return(err);
			}
		} while(++i<ivoc);
		/* si no encontr¢ una igual, la mete */
		for(i=0; i<LONGPAL; i++) vocab[ivoc].p[i]=pal.p[i];
		vocab[ivoc].num=pal.num;
		vocab[ivoc].tipo=pal.tipo;
	}
	ivoc++;

}
*num_palc=ivoc;
return(err);
}

/*****************************************************************************
	COMP_LIN_VOC: compila una l¡nea de la secci¢n de Vocabulario.
	  Entrada:      'l' l¡nea a compilar
			'p' puntero a buffer d¢nde dejar la palabra
	  Salida:       c¢digo de error si lo hubo
*****************************************************************************/
COD_ERR comp_lin_voc(char *l, struct palabra *p)
{
COD_ERR palerr;

/* CAMPO PALABRA */
l=salta_espacios(l);
palerr=coge_pal(l,p->p);
if(palerr) return(palerr);

/* CAMPO VALOR DE PALABRA */
/* desprecia caracteres hasta un espacio o fin de l¡nea */
/* error se encuentra fin de l¡nea (faltan campos) */
l=hasta_espacio(l);
if(fin_linea(*l)) return(_E_FCAM);
l=salta_espacios(l);
/* recoge el n£mero y si hay error sale */
palerr=coge_num(l,&p->num);
/* no admite indirecci¢n al dar el valor de una palabra de vocabulario */
if(palerr==_E_CIND) return(_E_CNUM);
if(palerr) return(palerr);

/* comprueba que sea un valor v lido de palabra */
if((p->num<1) || (p->num>254)) return(_E_NPAL);

/* CAMPO TIPO DE PALABRA */
/* desprecia caracteres hasta un espacio o fin de l¡nea */
/* error si encuentra fin de l¡nea (faltan campos) */
l=hasta_espacio(l);
l=salta_espacios(l);
if(fin_linea(*l)) return(_E_FCAM);
switch(mayuscula(*l)) {
case 'V' :
	p->tipo=_VERB;
	break;
case 'N' :
	p->tipo=_NOMB;
	break;
case 'A' :
	p->tipo=_ADJT;
	break;
case 'C' :
	p->tipo=_CONJ;
	break;
default :
	return(_E_TPAL);
	break;
}

return(_E_NERR);
}

/*****************************************************************************
	COGE_PAL: recoge una palabra de la l¡nea de entrada y la mete en
	  'pal' (buffer de al menos LONGPAL caracteres).
	  Entrada:      'l' puntero a linea de entrada
			'pal' puntero a buffer de palabra
	  Salida:       _E_FCAM si encontr¢ un car cter fin de linea
			(no hay m s campos detr s de la palabra)
			_E_CVOC si encontr¢ car cter no v lido
			_E_NERR si no hubo error.
*****************************************************************************/
COD_ERR coge_pal(char *l, char *pal)
{
int i;

for(i=0; i<LONGPAL-1; i++) {
	/* si encuentra fin de l¡nea rellena con espacios y sale con error */
	if(fin_linea(*l)) {
		for(; i<LONGPAL-1; i++) *(pal+i)=' ';
		*(pal+i)='\0';          /* marca fin de cadena */
		return(_E_FCAM);
	}
	/* si encuentra espacio o tabulaci¢n, rellena con espacios y sale */
	if(es_espacio(*l)) {
		for(; i<LONGPAL-1; i++) *(pal+i)=' ';
		break;
	}
	else {
		*(pal+i)=mayuscula(*l);
		l++;
		/* comprueba si es car cter v lido de palabra */
		if(!esta_en(Abecedario,*(pal+i)) &&
		  !esta_en(Numeros,*(pal+i))) return(_E_CVOC);
	}
}
*(pal+i)='\0';
return(_E_NERR);
}

/*****************************************************************************
	COGE_NUM: recoge un campo num‚rico de la linea de entrada, si
	  el n£mero est  encerrado entre '[]' devuelve un c¢digo indicandolo.
	  NOTA: si encuentra una constante tratar  de devolver el valor
	  num‚rico correspondiente si es que esta ha sido definida.
	  Entrada:      'l' puntero a l¡nea de entrada
			'n' puntero a dato de tipo BYTE
	  Salida:       'n' conteniendo n£mero convertido
			_E_CNUM si hubo error al coger el campo num‚rico
			_E_NNCT nombre de constante no v lido
			_E_CTND constante no definida
			_E_CIND si n£mero est  entre corchetes
			_E_NERR si no hubo error
*****************************************************************************/
COD_ERR coge_num(char *l, BYTE *n)
{
char num[4], cnst[LNGCONST];
int i=0, val;
BOOLEAN indir=FALSE;
BYTE valor;

/* si encuentra car cter '[' indica indirecci¢n */
if(*l=='[') {
	indir=TRUE;
	l++;
}

/* intenta recoger el n£mero */
while(esta_en(Numeros,*l) && (i<3)) num[i++]=*l++;
num[i]='\0';    /* marca fin de cadena */

/* si recogi¢ alg£n n£mero */
if(i) {
	val=atoi(num);
	/* comprueba si n£mero fuera de rango 8 bits */
	if((val<0) || (val>255)) return(_E_CNUM);
	valor=(BYTE)val;
}
/* si el primer car cter no es n£mero considera que */
/* ha encontrado una constante */
else {
	if(coge_nombre_etq(&l,cnst,LNGCONST)) return(_E_NNCT);

	/* si indirecci¢n el £ltimo car cter debe ser ']' */
	if(indir && (*l!=']')) return(_E_NNCT);
	else if(!es_espacio(*l) && !fin_linea(*l)) return(_E_CNUM);

	/* recoge el valor de la constante (si existe) */
	if(coge_const(cnst,&valor)) return(_E_CTND);
}

if(indir) {
	l=salta_espacios(l);
	/* si indirecci¢n y £ltimo car cter no es ']', error */
	if(*l!=']') return(_E_CNUM);
	l++;            /* salta el ']' */
}

/* si el £ltimo car cter no es blanco ni fin de l¡nea */
/* ni inicio de comentario (CHR_COMENT) da error */
if(!fin_linea(*l) && (*l!=' ') && (*l!='\t') && (*l!=CHR_COMENT))
  return(_E_CNUM);

*n=valor;

/* si n£mero est  entre corchetes lo indica */
if(indir) return(_E_CIND);

return(_E_NERR);
}

/*****************************************************************************
	COMPARA_PAL: compara dos palabras de vocabulario.
	  Entrada:      'p1' y 'p2'     punteros a palabras de vocabulario
	  Salida:       TRUE si son distintas
			FALSE si son iguales
*****************************************************************************/
BOOLEAN compara_pal(struct palabra *p1, struct palabra *p2)
{
if(strcmp(p1->p,p2->p)) return(TRUE);
else return(FALSE);
}

/*****************************************************************************
	AVISO: da un mensaje de aviso del compilador.
	  Entrada:      'men_aviso' mensaje de aviso
*****************************************************************************/
void aviso(char *men_aviso)
{
printf("*** AVISO: %s",men_aviso);
}

/*****************************************************************************
	COMP_TXT: compila texto de secciones de Mensajes, Mensajes del
	  Sistema, Localidades... por cada llamada compila el texto
	  delimitado entre dos caracteres CHR_DELIM
	  Entrada:      'f_txt' puntero al fichero que contiene el texto
			'txt' direcci¢n del puntero a la posici¢n del buffer
			d¢nde se guardar  el texto (su contenido quedar 
			actualizado tras finalizar)
			'ult_txt' puntero a la £ltima posici¢n del buffer
			d¢nde se guarda el texto
			'num_txt' n£mero del texto que se est  compilando
			(se usa para comprobar continuidad de numeraci¢n)
			'max_txt' n£mero m ximo permitido para el texto
			(se usa para comprobar numeraci¢n fuera de rango)
	  Salida:       c¢digo de error si lo hubo
*****************************************************************************/
STC_ERR comp_txt(FILE *f_txt, char *(*txt), char *ult_txt, BYTE num_txt,
  BYTE max_txt)
{
STC_ERR err={_E_NERR,0};
char lin[LONG_LIN];     /* buffer para l¡nea de entrada */
char *l;                /* puntero a buffer de l¡nea de entrada */
BYTE n;
COD_ERR txterr;

/* coge la primera l¡nea del texto que deber  tener el */
/* formato: xxxxx:@nnn ...texto... */
/* sale si error o fin de fichero */
if(fgets(lin,LONG_LIN,f_txt)==NULL) {
	if(ferror(f_txt)) err.codigo=_E_LTMP;
	return(err);
}
l=lin;  /* apunta al inicio de la l¡nea */

err.linea=num_linea(&l);

/* si el primer car cter no es CHR_DELIM sale con error */
if(*l!=CHR_DELIM) {
	err.codigo=_E_FALT;
	return(err);
}
l++;    /* salta el delimitador */

/* coge el n£mero del mensaje */
txterr=coge_num(l,&n);
if(txterr) {
	/* si indirecci¢n indica error en campo num‚rico */
	if(txterr==_E_CIND) err.codigo=_E_CNUM;
	else err.codigo=txterr;
	return(err);
}

/* comprueba si est  dentro de la secuencia y no se sale del rango */
if(n!=num_txt) {
	err.codigo=_E_NFSC;
	return(err);
}
if(n>max_txt) {
	err.codigo=_E_NVAL;
	return(err);
}

/* salta el espacio entre el n£mero y el primer car cter del texto */
l=hasta_espacio(l);
l++;

while(1) {
	/* repite hasta encontrar marca de fin de texto o fin de l¡nea */
	while((*l!=CHR_DELIM) && !fin_linea(*l)) {
		/* coge el car cter de la l¡nea de entrada y lo guarda */
		/* si es un '|' lo transforma en '\n' */
		/* si es una tabulaci¢n la transforma en espacio y da aviso */
		if(*l=='|') *(*txt)++='\n';
		else if(*l=='\t') {
			*(*txt)++=' ';
			aviso(Tab_Esp);
			printf(Lin_Msg,err.linea);
		}
		else *(*txt)++=*l;

		l++;

		/* comprueba si rebasa la memoria */
		if(*txt>=ult_txt) {
			err.codigo=_E_FMEM;
			return(err);
		}
	}

	/* si encontr¢ final de mensaje coloca car cter '\0' */
	/* y retorna sin error */
	if(*l==CHR_DELIM) {
		*(*txt)++='\0';
		return(err);
	}

	/* si encontr¢ final de l¡nea lee la siguiente */
	if(fgets(lin,LONG_LIN,f_txt)==NULL) {
		/* si se produjo error sale indic ndolo */
		if(ferror(f_txt)) {
			err.codigo=_E_LTMP;
			return(err);
		}
		/* si no es que ha llegado al final del fichero */
		/* sale sin indicar error por lo que d¢nde se llame */
		/* a esta funci¢n se deber  comprobar el final de fichero */
		return(err);
	}
	l=lin;  /* apunta al inicio de la l¡nea */

	err.linea=num_linea(&l);
}
}

/*****************************************************************************
	COMPILA_MEN: compila la secci¢n de Mensajes y Mensajes del sistema.
	  Entrada:      'f_men' puntero al fichero con los mensajes
			'men' puntero a inicio de zona reservada para
			los mensajes
			'mem_men' cantidad de memoria (bytes) reservada
			para los mensajes
			'max_men' m ximo n£mero permitido para un mensaje
			'desp_men' puntero a inicio de tabla d¢nde se
			guardar n los desplazamientos de cada mensaje
			'n_men' puntero a variable d¢nde se guardar  el
			n£mero de mensajes compilados
			'bytes_men' puntero a variable d¢nde se guardar  la
			memoria ocupada por los mensajes
	  Salida:       c¢digo de error si lo hubo
*****************************************************************************/
STC_ERR compila_men(FILE *f_men, char *men, unsigned mem_men, BYTE max_men,
  ptrdiff_t *desp_men, BYTE *n_men, ptrdiff_t *bytes_men)
{
STC_ERR err={_E_NERR,0};
char *ult_men, *act_men;        /* punteros auxiliares */
BYTE i=0;

/* calcula £ltima posici¢n en zona reservada para mensajes del sistema */
ult_men=men+mem_men-1;
/* inicializa puntero a posici¢n d¢nde ira el mensaje actual */
act_men=men;

while(!feof(f_men)) {
	/* guarda el desplazamiento del mensaje */
	desp_men[i]=act_men-men;
	err=comp_txt(f_men,&act_men,ult_men,i,max_men);
	/* sale si se produjo un error */
	if(err.codigo) return(err);
	i++;    /* siguiente mensaje */
}

/* guarda n£mero de mensajes compilados y la memoria ocupada */
*n_men=--i;
*bytes_men=act_men-men;

return(err);
}

/*****************************************************************************
	ESTA_EN_VOC: comprueba si una palabra est  en el vocabulario.
	  Entrada:      'vocab' puntero a tabla de vocabulario
			'pvoc' n£mero de palabras en vocabulario
			'pal' puntero a palabra a buscar
	  Salida:       posici¢n dentro del vocabulario si se encontr¢, si no
			devuelve (NUM_PAL+1) que puede considerarse como
			palabra no encontrada
*****************************************************************************/
int esta_en_voc(struct palabra *vocab, int pvoc, char *pal)
{
int i;

for(i=0; i<pvoc; i++) {
	if(!strcmp(pal,vocab[i].p)) return(i);
}
return(NUM_PAL+1);
}

/*****************************************************************************
	COMPILA_LOC: compila la secci¢n de Localidades.
	  Entrada:      'f_loc' puntero al fichero con las localidades
			'loc' puntero a inicio de zona reservada para
			los textos de localidades
			'mem_loc' cantidad de memoria (bytes) reservada
			para los textos de localidades
			'max_loc' m ximo n£mero permitido para una localidad
			'desp_loc' puntero a inicio de tabla d¢nde se
			guardar n los desplazamientos de cada texto de
			localidad
			'conx' puntero a inicio de zona reservada para
			las conexiones
			'mem_conx' cantidad de memoria (bytes) reservada
			para las conexiones
			'desp_conx' puntero a inicio de tabla d¢nde se
			guardar n los desplazamientos de las conexiones
			'vocabulario' puntero a tabla de vocabulario
			'pal_voc' n£mero de palabras del vocabulario
			'v_mov' m ximo n£mero de verbo de movimiento
			'n_loc' puntero a variable d¢nde se guardar  el
			n£mero de localidades compiladas
			'bytes_loc' puntero a variable d¢nde se guardar  la
			memoria ocupada por los textos de localidades
			'bytes_conx' puntero a variable d¢nde se guardar  la
			memoria ocupada por las conexiones
	  Salida:       c¢digo de error si lo hubo
*****************************************************************************/
STC_ERR compila_loc(FILE *f_loc, char *loc, unsigned mem_loc, BYTE max_loc,
  ptrdiff_t *desp_loc, BYTE *conx, unsigned mem_conx, ptrdiff_t *desp_conx,
  struct palabra *vocabulario, int pal_voc, BYTE v_mov, BYTE *n_loc,
  ptrdiff_t *bytes_loc, ptrdiff_t *bytes_conx)
{
STC_ERR err={_E_NERR, 0};
char *ult_loc, *act_loc;        /* punteros auxiliares */
BYTE *act_conx, *ult_conx;      /*    "         "      */
BYTE nloc=0;
char lin[LONG_LIN], *l;
long pos_fich;
char pal_conx[LONGPAL];
int npal;
BYTE num, tipo, num_loc;
COD_ERR locerr;

/* puntero a £ltima posici¢n de zona reservada a textos de localidades */
ult_loc=loc+mem_loc-1;
/* inicializa puntero al texto de localidad actual */
act_loc=loc;

/* puntero a £ltima posici¢n de zona reservada a conexiones */
ult_conx=conx+mem_conx-1;
/* inicializa puntero a conexiones de localidad actual */
act_conx=conx;

while(1) {
	/* guarda desplazamiento del texto de localidad */
	desp_loc[nloc]=act_loc-loc;
	/* compila el texto de la localidad */
	err=comp_txt(f_loc,&act_loc,ult_loc,nloc,max_loc);
	if(err.codigo) return(err);

	/* sale si fin de fichero */
	if(feof(f_loc)) break;

	/* guarda desplazamiento de conexiones de localidad actual */
	desp_conx[nloc]=act_conx-conx;
	nloc++;

	while(1) {
		/* almacena posici¢n en fichero */
		pos_fich=ftell(f_loc);

		/* recoge una l¡nea del fichero de entrada */
		/* sale si hubo error */
		if(fgets(lin,LONG_LIN,f_loc)==NULL) {
			if(ferror(f_loc)) {
				err.codigo=_E_LTMP;
				return(err);
			}
			/* si fin de fichero sale de bucle */
			break;
		}
		l=lin;  /* puntero al inicio de l¡nea */

		err.linea=num_linea(&l);

		/* si no encuentra como primer car cter de la l¡nea */
		/* MARCA_CNX restaura la posici¢n en el fichero y */
		/* sale del bucle */
		if(*l!=MARCA_CNX) {
			fseek(f_loc,pos_fich,SEEK_SET);
			break;
		}
		else {
			l=hasta_espacio(l);
			l=salta_espacios(l);

			/* coge el campo palabra */
			if((err.codigo=coge_pal(l,pal_conx))!=_E_NERR)
			  return(err);

			/* comprueba si la palabra est  en el vocabulario */
			npal=esta_en_voc(vocabulario,pal_voc,pal_conx);
			if(npal==(NUM_PAL+1)){
				err.codigo=_E_NPVC;
				return(err);
			}

			/* comprueba que sea verbo o nombre de movimiento */
			num=vocabulario[npal].num;
			tipo=vocabulario[npal].tipo;
			if(((tipo!=_VERB) && (tipo!=_NOMB)) || (num>=v_mov)) {
				err.codigo=_E_NMOV;
				return(err);
			}

			l=hasta_espacio(l);
			l=salta_espacios(l);

			/* si encuentra el final de la l¡nea sale con */
			/* error porque falta el n£mero de localidad con */
			/* la que conecta */
			if(fin_linea(*l)) {
				err.codigo=_E_FCAM;
				return(err);
			}

			/* coge y convierte el n£mero de localidad */
			locerr=coge_num(l,&num_loc);
			if(locerr) {
				/* no admite indirecci¢n, la trata como */
				/* error en campo num‚rico */
				if(locerr==_E_CIND) err.codigo=_E_CNUM;
				else err.codigo=locerr;
				return(err);
			}

			/* comprueba si n£mero de localidad es v lido */
			if(num_loc>(MAX_LOC-1)) {
				err.codigo=_E_NVAL;
				return(err);
			}

			/* guarda verbo de movimiento */
			*act_conx++=num;
			/* comprueba memoria */
			if(act_conx>=ult_conx) {
				err.codigo=_E_FMEM;
				return(err);
			}

			/* guarda n£mero de localidad a la que lleva */
			*act_conx++=nloc;
			/* comprueba memoria */
			if(act_conx>=ult_conx) {
				err.codigo=_E_FMEM;
				return(err);
			}
		}
	}
	/* indica fin de conexiones de localidad actual */
	*act_conx++=0;
	/* comprueba fuera de memoria */
	if(act_conx>=ult_conx) {
		err.codigo=_E_FMEM;
		return(err);
	}
}

/* guarda n£mero de localidades compiladas y memoria ocupada por textos */
*n_loc=nloc;
*bytes_loc=act_loc-loc;
*bytes_conx=act_conx-conx;

return(err);
}

/*****************************************************************************
	ES_BAND1: comprueba si el car cter dado es correspondiente a una
	  bandera de objeto (da lo mismo si es may£scula o min£scula).
	  Entrada:      'c' car cter a comprobar
	  Salida:       1 si 'P' (prenda)
			2 si 'L' (fuente de luz)
			0 otros  (no v lido)
*****************************************************************************/
BYTE es_band1(char c)
{
switch(mayuscula(c)) {
case 'P' :
	return(1);
	break;
case 'L' :
	return(2);
	break;
default :
	return(0);
	break;
}
}

/*****************************************************************************
	COMPILA_OBJ: compila la secci¢n de Objetos.
	  Entrada:      'f_obj' puntero al fichero con las localidades
			'obj' puntero a inicio de zona reservada para
			los objetos
			'mem_obj' cantidad de memoria (bytes) reservada
			para los objetos
			'max_obj' m ximo n£mero permitido para un objeto
			'desp_obj' puntero a inicio de tabla d¢nde se
			guardar n los desplazamientos de cada objeto
			'vocabulario' puntero a tabla de vocabulario
			'pal_voc' n£mero de palabras del vocabulario
			'n_loc' n£mero de localidades compiladas
			'n_obj' puntero a variable d¢nde se guardar  el
			n£mero de objetos compilados
			'bytes_obj' puntero a variable d¢nde se guardar  la
			memoria ocupada por los objetos
	  Salida:       c¢digo de error
*****************************************************************************/
STC_ERR compila_obj(FILE *f_obj, char *obj, unsigned mem_obj, BYTE max_obj,
  ptrdiff_t *desp_obj, struct palabra *vocabulario, int pal_voc, BYTE n_loc,
  BYTE *n_obj, ptrdiff_t *bytes_obj)
{
STC_ERR err={_E_NERR, 0};
char *act_obj, *ult_obj;
BYTE nobj=0;
char lin[LONG_LIN], *l, c;
BYTE n;
char pal_obj[LONGPAL];
int npal, i;
BYTE tipo, nombre, adjetivo, loc_ini;
unsigned bands2, band2;
BYTE bands1, band, bands21, bands22;
COD_ERR objerr;

/* inicializa puntero a objeto actual */
act_obj=obj;
/* inicializa puntero a £ltimo byte de zona de memoria de objetos */
ult_obj=obj+mem_obj;

while (1) {
	/* guarda desplazamiento del objeto actual */
	desp_obj[nobj]=act_obj-obj;

	/* lee l¡nea de entrada */
	if(fgets(lin,LONG_LIN,f_obj)==NULL) {
		if(ferror(f_obj)) {
			err.codigo=_E_LTMP;
			return(err);
		}
		/* si no fue error quiere decir que fue fin de fichero */
		/* en cuyo caso sale del bucle infinito */
		break;
	}
	/* inicializa puntero a l¡nea de entrada */
	l=lin;

	err.linea=num_linea(&l);

	/* comprueba si el primer car cter es CHR_DELIM */
	if(*l!=CHR_DELIM) {
		err.codigo=_E_FALT;
		return(err);
	}
	/* salta el delimitador */
	l++;

	/* coge el n£mero del objeto */
	objerr=coge_num(l,&n);
	if(objerr) {
		/* no admite indirecci¢n, la trata como error campo num‚rico */
		if(objerr==_E_CIND) err.codigo=_E_CNUM;
		else err.codigo=objerr;
		return(err);
	}
	/* comprueba si est  dentro de secuencia y es n£mero */
	/* de objeto v lido */
	if(n!=nobj) {
		err.codigo=_E_NFSC;
		return(err);
	}
	if(n>=MAX_OBJ) {
		err.codigo=_E_NVAL;
		return(err);
	}

	l=hasta_espacio(l);
	l=salta_espacios(l);

	/* coge el campo palabra (nombre) */
	if((err.codigo=coge_pal(l,pal_obj))!=_E_NERR) return(err);

	if((npal=esta_en_voc(vocabulario,pal_voc,pal_obj))==(NUM_PAL+1)) {
		err.codigo=_E_NNVC;
		return(err);
	}

	/* comprueba que la palbara sea un nombre */
	/* si lo es guarda su n£mero */
	tipo=vocabulario[npal].tipo;
	if(tipo!=_NOMB) {
		err.codigo=_E_NNOM;
		return(err);
	}
	nombre=vocabulario[npal].num;

	l=hasta_espacio(l);
	l=salta_espacios(l);

	/* si no es indicador de cualquier palabra (CHR_NOPAL) */
	/* recoge la palabra comprobando que sea un adjetivo */
	/* si no guarda valor de NO_PAL */
	if(*l!=CHR_NOPAL) {
		/* coge el campo palabra (nombre) */
		if((err.codigo=coge_pal(l,pal_obj))!=_E_NERR) return(err);
		if((npal=esta_en_voc(vocabulario,pal_voc,
		  pal_obj))==(NUM_PAL+1)) {
			err.codigo=_E_NAVC;
			return(err);
		}
		tipo=vocabulario[npal].tipo;
		if(tipo!=_ADJT) {
			err.codigo=_E_NADJ;
			return(err);
		}
		adjetivo=vocabulario[npal].num;
	}
	else {
		adjetivo=NO_PAL;
		l++;
	}

	l=hasta_espacio(l);
	l=salta_espacios(l);

	/* coge el n£mero de localidad inicial */
	objerr=coge_num(l,&loc_ini);
	if(objerr) {
		/* no admite indirecci¢n, la trata como */
		/* error en campo num‚rico */
		if(objerr==_E_CIND) err.codigo=_E_CNUM;
		else err.codigo=objerr;
		return(err);
	}
	if(((loc_ini<NO_CREADO) && (loc_ini>=n_loc)) || loc_ini==LOC_ACTUAL) {
		err.codigo=_E_LINC;
		return(err);
	}

	/* recoge las banderas del objeto hasta que encuentre un */
	/* car cter que no se corresponda a uno v lido como bandera */
	bands1=0;
	do {
		l=hasta_espacio(l);
		l=salta_espacios(l);
		band=es_band1(*l);
		/* activa la bandera correspondiente y salta a la */
		/* siguiente posici¢n si fue v lida */
		bands1|=band;
		if(band) l++;
	} while(band);

	/* recoge las banderas de usuario del objeto */
	bands2=0;
	/* bit de mayor peso de variable auxiliar a 1 */
	band2=0x8000;
	for(i=0; i<16; i++) {
		c=mayuscula(*l);
		if((c!=BAND_0) && (c!=BAND_1)) {
			err.codigo=_E_BAND;
			return(err);
		}
		if(c==BAND_1) bands2|=band2;
		/* desplaza hacia la derecha */
		band2>>=1;
		l++;
	}
	/* separa la parte baja de la parte alta */
	bands22=(BYTE)(bands2 & 0x00ff);
	bands21=(BYTE)(bands2 >> 8);

	/* guardamos los datos recogidos */
	/* comprobando primero si hay memoria disponible */
	if(act_obj+5>=ult_obj) {
		err.codigo=_E_FMEM;
		return(err);
	}

	*act_obj++=nombre;      /* NOMBRE */
	*act_obj++=adjetivo;    /* ADJETIVO */
	*act_obj++=loc_ini;     /* LOCALIDAD INICIAL */
	*act_obj++=bands1;      /* BANDERAS */
	*act_obj++=bands21;     /* BANDERAS DE USUARIO */
	*act_obj++=bands22;

	/* lee l¡nea de entrada */
	if(fgets(lin,LONG_LIN,f_obj)==NULL) {
		if(ferror(f_obj)) {
			err.codigo=_E_LTMP;
			return(err);
		}
		/* si no fue error de lectura es que se lleg¢ al final */
		/* del fichero por lo que da error 'falta texto de objeto' */
		else {
			err.codigo=_E_OTEX;
			return(err);
		}
	}
	/* inicializa puntero a l¡nea de entrada */
	l=lin;

	err.linea=num_linea(&l);

	/* si encuentra inicio de otro objeto es que falta texto */
	if(*l==CHR_DELIM) {
		err.codigo=_E_OTEX;
		return(err);
	}

	/* compila texto de objeto */
	while(*l) {
		/* comprueba fuera de memoria */
		if(act_obj>=ult_obj) {
			err.codigo=_E_FMEM;
			return(err);
		}
		/* si encuentra un '\n' coloca final de texto */
		/* y sale del bucle */
		if(*l=='\n') {
			*act_obj++='\0';
			break;
		}
		/* si encuentra una tabulaci¢n la transforma en espacio */
		/* y avisa */
		else if(*l=='\t') {
			aviso(Tab_Esp);
			printf(Lin_Msg,err.linea);
			*act_obj++=' ';
			l++;
		}
		/* en otro caso guarda el car cter encontrado */
		else *act_obj++=*l++;
	}

	/* siguiente objeto */
	nobj++;

}

/* devuelve el n£mero de objetos compilados y la memoria ocupada */
*n_obj=nobj;
*bytes_obj=act_obj-obj;

return(err);
}

/*****************************************************************************
	COMPILA_PRO: compila la secci¢n de Procesos.
	  Entrada:      'f_pro' puntero al fichero con los procesos
			'pro' puntero a inicio de zona reservada para
			los procesos
			'mem_pro' cantidad de memoria (bytes) reservada
			para los procesos
			'max_pro' m ximo n£mero permitido para un proceso
			'desp_pro' puntero a inicio de tabla d¢nde se
			guardar n los desplazamientos de cada proceso
			'vocabulario' puntero a tabla de vocabulario
			'pal_voc' n£mero de palabras del vocabulario
			'n_loc' n£mero de localidades compiladas
			'n_msy' n£mero de mensajes del sistema compilados
			'n_msg' n£mero de mensajes compilados
			'n_obj' n£mero de objetos compilados
			'n_conv' m ximo n£mero de nombre convertible
			'n_pro' puntero a variable d¢nde se guardar  el
			n£mero de procesos compilados
			'bytes_pro' puntero a variable d¢nde se guardar  la
			memoria ocupada por los procesos
	  Salida:       c¢digo de error si lo hubo
*****************************************************************************/
STC_ERR compila_pro(FILE *f_pro, BYTE *pro, unsigned mem_pro, BYTE max_pro,
  ptrdiff_t *desp_pro, struct palabra *vocabulario, int pal_voc, BYTE n_loc,
  BYTE n_msy, BYTE n_msg, BYTE n_obj, BYTE n_conv, BYTE *n_pro,
  ptrdiff_t *bytes_pro)
{
STC_ERR err={_E_NERR, 0};
char lin[LONG_LIN], *l;
BYTE *act_pro, *ult_pro;
BYTE npro=0, p;
long pos_fich;
int i, npal;
char etq[LONGETQ], pal[LONGPAL], cond[LNGCOND+1];
BYTE *sgte_entr;
BOOLEAN pra_lin_entr;
ptrdiff_t se;
COD_ERR proerr;

/* inicializa puntero a zona de memoria de proceso actual */
act_pro=pro;
/* puntero a £ltimo byte de zona reservada para procesos */
ult_pro=pro+mem_pro;

printf(Men_Pro);

while(1) {
	/* lee l¡nea de entrada; se supone que es la primera */
	/* del proceso con el formato 'num_lin:\nnn...' */
	if(fgets(lin,LONG_LIN,f_pro)==NULL) {
		if(ferror(f_pro)) {
			err.codigo=_E_LTMP;
			return(err);
		}
		/* si no fue error quiere decir que fue fin de fichero */
		/* en cuyo caso sale del bucle infinito */
		break;
	}
	/* inicializa puntero a l¡nea de entrada */
	l=lin;

	err.linea=num_linea(&l);
	l++;    /* salta '\' */

	/* recoge n£mero de proceso y sale si n£mero no v lido */
	/* o fuera de secuencia */
	proerr=coge_num(l,&p);
	if(proerr) {
		/* no admite indirecci¢n, la trata como */
		/* error en campo num‚rico */
		if(proerr==_E_CIND) err.codigo=_E_CNUM;
		else err.codigo=proerr;
		return(err);
	}
	if(p!=npro) {
		err.codigo=_E_NFSC;
		return(err);
	}

	printf("%3u\b\b\b",p);

	/* actualiza variable global 'num_pro_act' */
	num_pro_act=p;

	/* las etiquetas son locales, es decir, s¢lo valen dentro del */
	/* proceso en el que fueron definidas */
	/* inicializa puntero a tabla de etiquetas */
	pt_etiq=0;
	/* inicializa puntero a tabla de referencias forward */
	pt_skip=0;

	/* guarda desplazamiento de proceso */
	desp_pro[npro]=act_pro-pro;

	/* guarda posici¢n en fichero */
	pos_fich=ftell(f_pro);
	/* lee l¡nea de entrada */
	if(fgets(lin,LONG_LIN,f_pro)==NULL) {
		if(ferror(f_pro)) {
			err.codigo=_E_LTMP;
			return(err);
		}
		/* si no fue error quiere decir que fue fin de fichero */
		/* en cuyo caso sale del bucle infinito */
		break;
	}
	/* inicializa puntero a l¡nea de entrada */
	l=lin;
	err.linea=num_linea(&l);

	while(1) {
		/* si encuentra marca de inicio de proceso '\' */
		/* quiere decir que empieza un nuevo proceso y por */
		/* tanto acaba el actual */
		if(*l=='\\') {
			/* siguiente n£mero de proceso */
			npro++;

			/* comprueba fuera de memoria */
			if(act_pro+1>=ult_pro) {
				err.codigo=_E_FMEM;
				return(err);
			}
			/* pone marca de fin de proceso */
			*act_pro++=0;
			*act_pro++=0;

			/* recupera posici¢n en fichero */
			fseek(f_pro,pos_fich,SEEK_SET);

			/* sale del bucle para compilar siguiente proceso */
			break;
		}

		/* el primer car cter de la l¡nea no debe ser un blanco */
		/* ya que si no la entrada no es v lida */
		if(es_espacio(*l)) {
			err.codigo=_E_NENT;
			return(err);
		}

		/*** ETIQUETAS ***/
		if(*l==MARCA_ETQ) {
			/* comprueba si la tabla de etiquetas est  llena */
			if(pt_etiq>=LABELS) {
				err.codigo=_E_RBTL;
				return(err);
			}
			l++;

			/* recoge etiqueta */
			if(coge_nombre_etq(&l,etq,LONGETQ)) {
				err.codigo=_E_ENVL;
				return(err);
			}

			/* si el £ltimo car cter no es fin de l¡nea */
			/* ni espacio, error en nombre de etiqueta */
			if(!fin_linea(*l) && !es_espacio(*l)) {
				err.codigo=_E_ENVL;
				return(err);
			}

			/* si la tabla de etiquetas est  vac¡a, mete la */
			/* etiqueta sin m s, si no mira si est  ya definida */
			if(!pt_etiq) {
				strcpy(etiqueta[pt_etiq].etq,etq);
				etiqueta[pt_etiq].petq=act_pro;
			}
			else {
				for(i=0; i<pt_etiq; i++) {
					/* error si etiqueta ya definida */
					if(!strcmp(etq,etiqueta[i].etq)) {
					      err.codigo=_E_EREP;
					      return(err);
					}
				}
				/* guarda etiqueta y su direcci¢n */
				strcpy(etiqueta[pt_etiq].etq,etq);
				etiqueta[pt_etiq].petq=act_pro;
			}
			/* siguiente entrada en la tabla de etiquetas */
			pt_etiq++;

			/* lee l¡nea de entrada */
			if(fgets(lin,LONG_LIN,f_pro)==NULL) {
				if(ferror(f_pro)) {
					err.codigo=_E_LTMP;
					return(err);
				}
				/* si fin de fichero sale de bucle */
				break;
			}
		}

		/* inicializa puntero a l¡nea de entrada */
		l=lin;
		err.linea=num_linea(&l);

		/* espera encontrar un verbo o nombre convertible */
		/* y un nombre (o la palabra nula CHR_NOPAL) */

		/* si es palabra nula (CHR_NOPAL) guarda NO_PAL */
		if(*l==CHR_NOPAL) *act_pro++=NO_PAL;
		else {
			/* recoge el campo verbo */
			if((err.codigo=coge_pal(l,pal))!=_E_NERR) return(err);

			/* comprueba si est  en el vocabulario */
			npal=esta_en_voc(vocabulario,pal_voc,pal);
			if(npal==NUM_PAL+1) {
				err.codigo=_E_NPVC;
				return(err);
			}
			/* si no es verbo ni nombre convertible */
			if((vocabulario[npal].tipo!=_VERB) &&
			  ((vocabulario[npal].tipo==_NOMB) &&
			  (vocabulario[npal].num>=n_conv))) {
				err.codigo=_E_NVNC;
				return(err);
			}
			/* guarda n£mero de verbo o nombre convertible */
			*act_pro++=vocabulario[npal].num;
		}
		/* comprueba fuera de memoria */
		if(act_pro>=ult_pro) {
			err.codigo=_E_FMEM;
			return(err);
		}

		l=hasta_espacio(l);
		l=salta_espacios(l);

		/* si es palabra nula (CHR_NOPAL) guarda NO_PAL */
		if(*l==CHR_NOPAL) *act_pro++=NO_PAL;
		else {
			/* recoge el campo nombre */
			if((err.codigo=coge_pal(l,pal))!=_E_NERR) return(err);

			/* comprueba si est  en el vocabulario */
			npal=esta_en_voc(vocabulario,pal_voc,pal);
			if(npal==NUM_PAL+1) {
				err.codigo=_E_NNVC;
				return(err);
			}
			/* comprueba si es nombre */
			if(vocabulario[npal].tipo!=_NOMB) {
				err.codigo=_E_NNNN;
				return(err);
			}
			/* guarda n£mero de nombre */
			*act_pro++=vocabulario[npal].num;
		}
		/* comprueba fuera de memoria */
		if(act_pro>=ult_pro) {
			err.codigo=_E_FMEM;
			return(err);
		}

		/* guarda posici¢n d¢nde meter el desplazamiento de */
		/* la siguiente entrada */
		sgte_entr=act_pro;
		/* salta 2 bytes para empezar a compilar la entrada */
		act_pro+=2;
		/* comprueba fuera de memoria */
		if(act_pro>=ult_pro) {
			err.codigo=_E_FMEM;
			return(err);
		}

		/*** COMPILA CONDACTOS ***/
		/* espera encontrar un condacto */
		/* indica que estamos en primera linea de entrada */
		pra_lin_entr=TRUE;
		while(1) {
			/* si es primera linea entrada debe saltar */
			/* el campo nombre y dejar el puntero al inicio */
			/* del condacto */
			if(pra_lin_entr) {
				l=hasta_espacio(l);
				pra_lin_entr=FALSE;
			}
			else {
				/* guarda la posici¢n en el fichero */
				pos_fich=ftell(f_pro);

				/* lee l¡nea de entrada */
				if(fgets(lin,LONG_LIN,f_pro)==NULL) {
					if(ferror(f_pro)) {
						err.codigo=_E_LTMP;
						return(err);
					}
					/* si fin de fichero sale de bucle */
					break;
				}
				l=lin;
				err.linea=num_linea(&l);

				/* si el primer car cter no es espacio */
				/* finaliza el proceso actual */
				if((*l!=' ') && (*l!='\t')) {
					/* marca fin de entrada */
					*act_pro++=0;
					/* comprueba fuera de memoria */
					if(act_pro>=ult_pro) {
						err.codigo=_E_FMEM;
						return(err);
					}

					/* guarda el desplazamiento de la */
					/* siguiente entrada respecto al */
					/* inicio del proceso */
					se=(act_pro-pro)-desp_pro[npro];
					/* byte bajo */
					*sgte_entr=(BYTE)(se & 0x00ff);
					/* byte alto */
					*(sgte_entr+1)=(BYTE)((se >> 8)
					  & 0x00ff);

					break;  /* fin de entrada */
				}
			}

			l=salta_espacios(l);

			/* recoge el condacto */
			for(i=0; i<LNGCOND; i++, l++) {
				cond[i]=mayuscula(*l);
				if(!esta_en(Abecedario,cond[i]) &&
				  !esta_en(Numeros,cond[i])) break;
			}
			if((*l!=' ') && (*l!='\t') && (*l!='\n')) {
				err.codigo=_E_NCND;
				return(err);
			}

			/* rellena con espacios y a¤ade fin de cadena */
			for(; i<LNGCOND; cond[i++]=' ');
			cond[i]='\0';

			/* busca el condacto */
			for(i=1; i<N_CONDACTOS+1; i++) {
				if(!strcmp(cond,condacto[i].cnd)) break;
			}

			/* retorna con error si condacto no encontrado */
			/* o no v lido */
			if(i==N_CONDACTOS+1) {
				err.codigo=_E_NCND;
				return(err);
			}

			/* comprueba memoria ocupada */
			if(act_pro+condacto[i].mem>=ult_pro) {
				err.codigo=_E_FMEM;
				return(err);
			}

			/* si no, compila comprobando par metros, */
			/* indirecci¢n,... */
			if((err.codigo=cnd[i](&l,&act_pro))!=_E_NERR)
			  return(err);

			/* si quedan caracteres al final de la l¡nea */
			/* dar  un mensaje de aviso */
			l=hasta_espacio(l);
			l=salta_espacios(l);
			if(!fin_linea(*l) && *l!=';') {
				printf("\n");
				aviso("demasiados campos, se ignoran los sobrantes, ");
				printf(Lin_Msg,err.linea);
				printf(Men_Pro);
			}
		}

	if(feof(f_pro)) break;
	}
/* ahora sustituye las etiquetas */
err=sust_etiquetas();
if(err.codigo) return(err);
}
/* indica fin de £ltimo proceso */
*act_pro++=0;
*act_pro++=0;

/* ahora procesa llamadas a procesos 'forward' */
err=chequea_pro(npro);

/* devuelve el n£mero de procesos compilados y la memoria ocupada */
*n_pro=++npro;
*bytes_pro=act_pro-pro;

printf("\n");

return(err);
}

/*****************************************************************************
	COGE_NOMBRE_ETQ: recoge el nombre de una etiqueta o una constante
	  Entrada:      'l' puntero a puntero a la posici¢n de la l¡nea d¢nde
			comienza el nombre de la etiqueta o constante
			'etq' puntero a buffer d¢nde se colocar  el
			nombre de la etiqueta o constante
			'lngbuff' tama¤o del buffer
	  Salida:       TRUE si hubo error y adem s deja 'l' apuntando al
			primer car cter que no se corresponda con uno v lido
			para nombre de etiqueta o constante
			FALSE si no
*****************************************************************************/
BOOLEAN coge_nombre_etq(char *(*l), char *etq, int lngbuff)
{
int i;

for(i=0; i<lngbuff-1; i++, (*l)++) {
	/* sale si encuentra fin de l¡nea o espacio */
	if(fin_linea(*(*l)) || es_espacio(*(*l))) break;
	etq[i]=mayuscula(*(*l));
	/* si encuentra un car cter no v lido como nombre de etiqueta */
	/* o constante sale con error */
	if(!esta_en(Abecedario,etq[i]) && !esta_en(Numeros,etq[i]) &&
	  !esta_en(Signos,etq[i])) return(TRUE);
}
/* marca fin de etiqueta o constante */
etq[i]='\0';

/* si no se pudo recoger nombre de etiqueta o constante */
/* es que no es v lido, si no deja 'l' apuntando al primer car cter */
/* que no sea v lido para nombre de etiqueta o constante */
if(!i) return(TRUE);
else while(esta_en(Abecedario,mayuscula(*(*l))) && esta_en(Numeros,*(*l))
  && esta_en(Signos,*(*l))) (*l)++;

return(FALSE);
}

/*****************************************************************************
	SUST_ETIQUETAS: sutituye referencias 'forward' en saltos SKIP.
	  Salida:       c¢digo de error
*****************************************************************************/
STC_ERR sust_etiquetas(void)
{
STC_ERR err={_E_NERR, 0};
int i, j;
unsigned desplu;

for(i=0; i<pt_skip; i++) {
	/* busca la etiqueta a sustituir (fskip[i].etq) en la */
	/* tabla de etiquetas definidas (etiqueta[j].etq) */
	for(j=0; j<pt_etiq; j++) {
		if(!strcmp(fskip[i].etq,etiqueta[j].etq)) break;
	}
	/* si no encontr¢ la etiqueta pone en la variable global */
	/* 'numlin_fich_entr' el n£mero de l¡nea del error */
	if(j==pt_etiq) {
		err.codigo=(_E_RFFW);
		err.linea=fskip[i].nl;
		return(err);
	}

	/* si la etiqueta est  definida calcula el desplazamiento */
	/* comprueba que no se salga de rango y lo introduce en el c¢digo */
	desplu=(unsigned)(etiqueta[j].petq-(fskip[i].fsk-1));
	if(desplu>(unsigned)32767) {
		err.codigo=_E_LFFR;
		err.linea=fskip[i].nl;
		return(err);
	}
	*(fskip[i].fsk)=(BYTE)(desplu & 0x00ff);
	*(fskip[i].fsk+1)=(BYTE)((desplu >> 8) & 0x00ff);
}
return(err);
}

/*****************************************************************************
	CHEQUEA_PRO: comprueba si hay alguna llamada a un proceso inexistente.
	  Entrada:      'num_pro' n£mero de procesos compilados
	  Salida:       c¢digo de error
*****************************************************************************/
STC_ERR chequea_pro(BYTE num_pro)
{
STC_ERR err={_E_NERR, 0};
int i;

for(i=0; i<pprc; i++) {
	/* mira a ver si se llama a un proceso no definido */
	if(prc[i].numpro>num_pro) {
		err.codigo=_E_PRLL;
		err.linea=prc[i].nl;
		return(err);
	}
}

return(err);
}

/*****************************************************************************
	METE_CONST: mete una constante en la tabla de constantes.
	  Entrada:      'l' puntero a l¡nea de entrada conteniendo
			la constante
	  Salida:       c¢digo de error
*****************************************************************************/
COD_ERR mete_const(char *l)
{
char cnst[LNGCONST], num[4];
BYTE val;
int i, vali;

/* mira si la tabla est  llena */
if(pt_const>=NCONST) return(_E_TCCN);

/* el primer car cter del nombre de la constante no puede ser n£mero */
if(esta_en(Numeros,*l)) return(_E_PCCN);

if(coge_nombre_etq(&l,cnst,LNGCONST)) return(_E_NNCT);

/* si lo que sigue al nombre de la constante no es espacio, da error */
if(!es_espacio(*l)) return(_E_NNCT);

l=salta_espacios(l);

/* si encuentra fin de l¡nea o inicio de comentario (CHR_COMENT) */
/* da error ya que falta valor de constante */
if(fin_linea(*l) || (*l==CHR_COMENT)) return(_E_FVCN);

/* coge el valor de la constante */
i=0;
while(esta_en(Numeros,*l) && (i<3)) num[i++]=*l++;
num[i]='\0';    /* marca fin de cadena */

/* si recogi¢ alg£n n£mero */
if(i) {
	vali=atoi(num);
	/* comprueba si n£mero fuera de rango 8 bits */
	if((vali<0) || (vali>255)) return(_E_VNCT);
	val=(BYTE)vali;
}
/* si no pudo recoger ning£n n£mero sale con error */
else return(_E_VNCT);

strcpy(constante[pt_const].cnst,cnst);
constante[pt_const].valor=val;

/* siguiente entrada en la tabla */
pt_const++;

return(_E_NERR);
}

/*****************************************************************************
	COGE_CONST: recoge una constante de la tabla de constantes.
	  Entrada:      'cnst' nombre de la constante a recoger
			'valor' puntero a variable d¢nde se dejar 
			el valor de la constante (quedar  sin modificar
			si no se encontr¢ la constante)
	  Salida:       FALSE si no hubo error
			TRUE si la constante no fue definida
*****************************************************************************/
BOOLEAN coge_const(char *cnst, BYTE *valor)
{
int i;

/* si no se defini¢ ninguna constante sale con error */
if(!pt_const) return(TRUE);

/* busca la constante */
for(i=0; i<pt_const; i++) {
	if(!strcmp(constante[i].cnst,cnst)) break;
}

/* si no la encontr¢ sale con error */
if(i==pt_const) return(TRUE);

/* coge el valor de la constante y lo devuelve */
*valor=constante[i].valor;

return(FALSE);
}

