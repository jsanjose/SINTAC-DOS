#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <malloc.h>
#include <string.h>
#include <conio.h>
#include "version.h"
#include "sintac.h"

/****************************************/
/********** Variables globales **********/
/****************************************/

FILE *f_in;
#include "condact.h"    /* tabla de nombre-tipo de condactos */

/** variables para Vocabulario **/
struct palabra vocabulario[NUM_PAL];    /* para almacenar vocabulario */
int pvoc=0;     /* n£mero de £ltima palabra de vocabulario */
BYTE v_mov;     /* guarda m x. n£m. de verbo de movimiento */
BYTE n_conv;    /* guarda m x. n£m. de nombre convertible */
BYTE n_prop;    /* guarda m x. n£m. de nombre propio */

/** variables para Mensajes **/
ptrdiff_t mens[MAX_MSG];    /* tabla de desplaz. de mensajes */
char far *msgs;             /* puntero a inicio de zona de mensajes */
BYTE m_act=0;               /* n£mero de mensaje actual */

/** variables para Mensajes de Sistema **/
ptrdiff_t mensys[MAX_MSY];  /* tabla de desplaz. de mensajes */
char far *msgsys;           /* puntero a inicio de zona de mensajes */
BYTE m_actsys=0;            /* n£mero de mensaje actual */

/** variables para Localidades **/
BYTE n_loc=0;               /* n£mero de localidad actual */
ptrdiff_t locs[MAX_LOC];    /* tabla de desplaz. de texto locs. */
char far *localidades;      /* puntero a inicio de texto de locs. */
/** variables para Conexiones **/
ptrdiff_t conxs[MAX_LOC];   /* tabla de desplaz. de lista de conexiones */
char far *conx;             /* puntero a inicio de zona de conexiones */

/** variables para Objetos **/
BYTE objact=0;              /* n£mero de objeto actual */
ptrdiff_t objs[MAX_OBJ];    /* tabla de desplaz. de lista de objetos */
char far *obj;              /* puntero a inicio de zona de objetos */

/** variables para Procesos **/
BYTE far *proc;             /* puntero a inicio de zona de procesos */
ptrdiff_t pro[MAX_PRO];     /* tabla de desplaz. de procesos */
BYTE proact=0;              /* num. de proceso actual */

/****************************************/
/************** Prototipos **************/
/****************************************/

int si_o_no(char *preg);
int rd_fich(char far *buff,unsigned tam);
void frd2(void *buff,size_t tam,size_t cant);
int saca_pal(BYTE num_pal,BYTE tipo_pal);
void print_bin(BYTE b);
char mayuscula(char c);

/****************************************/
/********** Programa principal **********/
/****************************************/

void main(int argc,char *argv[])
{
static char *formato="%-20s : %5i %5u\n";
static char *errmem="No hay suficiente memoria.\n";
char nombre[_MAX_PATH];
char recon[L_RECON];
BYTE v_mov, n_conv, n_prop;
BYTE m_act, m_actsys, n_loc, objact;
unsigned t_msg, t_msy, t_loc, t_con, t_obj, t_pro;
char far *pc;
int i, j;
BYTE far *pcp;
int cd, indir1, indir2, pralin, dirrel;

printf("\n"COPYRIGHT);
printf("Listador Versi¢n "VERSION_LS);

if(argc<2) {
	printf("Nombre de fichero a listar: ");
	gets(nombre);
	for(i=0;nombre[i];i++) nombre[i]=mayuscula(nombre[i]);
}
else {
	for(i=0;argv[1][i];i++) nombre[i]=mayuscula(argv[1][i]);
	nombre[i]='\0';
}
if ((f_in=fopen(nombre,"rb"))==NULL) {
	printf("Error de apertura de fichero de entrada [%s].\n",nombre);
	exit(1);
}

frd2((char *)recon,sizeof(char),L_RECON-1);
recon[L_RECON-1]='\0';  /* a¤ade el fin de cadena */
if(strcmp(SRECON,recon)) {
	printf("Fichero de entrada no v lido.\n");
	exit(1);
}
frd2((BYTE *)&v_mov,sizeof(BYTE),1);
frd2((BYTE *)&n_conv,sizeof(BYTE),1);
frd2((BYTE *)&n_prop,sizeof(BYTE),1);
frd2((int *)&pvoc,sizeof(int),1);
frd2((BYTE *)&m_act,sizeof(BYTE),1);
frd2((unsigned *)&t_msg,sizeof(unsigned),1);
frd2((BYTE *)&m_actsys,sizeof(BYTE),1);
frd2((unsigned *)&t_msy,sizeof(unsigned),1);
frd2((BYTE *)&n_loc,sizeof(BYTE),1);
frd2((unsigned *)&t_loc,sizeof(unsigned),1);
frd2((unsigned *)&t_con,sizeof(unsigned),1);
frd2((BYTE *)&objact,sizeof(BYTE),1);
frd2((unsigned *)&t_obj,sizeof(unsigned),1);
frd2((BYTE *)&proact,sizeof(BYTE),1);
frd2((unsigned *)&t_pro,sizeof(unsigned),1);

printf("\nSECCION                 NUM. BYTES\n");
printf(formato,"Vocabulario",pvoc,(unsigned)(pvoc*sizeof(struct palabra)));
printf(formato,"Mensajes",(int)m_act,t_msg);
printf(formato,"Mensajes del Sistema",(int)m_actsys,t_msy);
printf(formato,"Localidades",(int)n_loc,t_loc);
printf(formato,"Conexiones",(int)n_loc,t_con);
printf(formato,"Objetos",(int)objact,t_obj);
printf(formato,"Procesos",(int)proact,t_pro);
printf("Palabras de movimiento < %3u\n",(unsigned)v_mov);
printf("Nombres convertibles   < %3u\n",(unsigned)n_conv);
printf("Nombres propios        < %3u\n",(unsigned)n_prop);

/* Reserva de memoria para las distintas secciones */
/* -- Mensajes -- */
if((msgs=(char far *)_fmalloc((size_t)t_msg))==NULLF) {
	printf(errmem);
	exit(1);
}

/* -- Mensajes del Sistema -- */
if((msgsys=(char far *)_fmalloc((size_t)t_msy))==NULLF) {
	printf(errmem);
	exit(1);
}

/* -- Localidades -- */
if((localidades=(char far *)_fmalloc((size_t)t_loc))==NULLF) {
	printf(errmem);
	exit(1);
}
/* -- Conexiones -- */
if((conx=(char far *)_fmalloc((size_t)t_con))==NULLF) {
	printf(errmem);
	exit(1);
}

/* -- Objetos -- */
if((obj=(char far *)_fmalloc((size_t)t_obj))==NULLF) {
	printf(errmem);
	exit(1);
}

/* -- Procesos -- */
if((proc=(BYTE far *)_fmalloc((size_t)t_pro))==NULLF) {
	fprintf(stderr,errmem);
	exit(1);
}

frd2((struct palabra *)vocabulario,sizeof(struct palabra),pvoc);

frd2((ptrdiff_t *)mens,sizeof(ptrdiff_t),(size_t)MAX_MSG);
rd_fich(msgs,t_msg);

frd2((ptrdiff_t *)mensys,sizeof(ptrdiff_t),(size_t)MAX_MSY);
rd_fich(msgsys,t_msy);

frd2((ptrdiff_t *)locs,sizeof(ptrdiff_t),(size_t)MAX_LOC);
rd_fich(localidades,t_loc);

frd2((ptrdiff_t *)conxs,sizeof(ptrdiff_t),(size_t)MAX_LOC);
rd_fich(conx,t_con);

frd2((ptrdiff_t *)objs,sizeof(ptrdiff_t),(size_t)MAX_OBJ);
rd_fich(obj,t_obj);

frd2((ptrdiff_t *)pro,sizeof(ptrdiff_t),(size_t)MAX_PRO);
rd_fich(proc,t_pro);

if(si_o_no("\n\n¨Ver Vocabulario? "))
{
	printf("\n\tPALABRA\tNUMERO\tTIPO");
	for(i=0;i<pvoc;i++) {
		printf("\n%3i:\t%s\t%i\t",i,vocabulario[i].p,vocabulario[i].num);
		switch(vocabulario[i].tipo) {
			case _VERB :
				if(vocabulario[i].num<v_mov) printf("verbo movimiento");
				else printf("verbo");
				break;
			case _NOMB :
				if(vocabulario[i].num<v_mov) printf("movimiento");
				else if(vocabulario[i].num<n_conv)
				  printf("nombre convertible");
				else if(vocabulario[i].num<n_prop) printf("nombre propio");
				else printf("nombre");
				break;
			case _ADJT :
				printf("adjetivo");
				break;
			case _CONJ :
				printf("conjunci¢n");
				break;
		}
	}
}

if(si_o_no("\n\n¨Ver Mensajes? "))
{
	for(i=0;i<(int)m_act;i++) {
		printf("\n%3i: ",i);
		pc=msgs+mens[i];
		while(*pc) {    /* comprobamos antes por si es mensaje nulo */
			printf("%c",*pc++);
		}

	}
}

if(si_o_no("\n\n¨Ver Mensajes del Sistema? "))
{
	for(i=0;i<(int)m_actsys;i++) {
		printf("\n%3i: ",i);
		pc=msgsys+mensys[i];
		while(*pc) {    /* comprobamos antes por si es mensaje nulo */
			printf("%c",*pc++);
		}
	}
}

if(si_o_no("\n\n¨Ver Localidades? "))
{
	for(i=0;i<(int)n_loc;i++) {
		printf("\n%3i: ",i);
		pc=localidades+locs[i];
		while(*pc) {    /* comprobamos antes por si es mensaje nulo */
			printf("%c",*pc++);
		}
		printf("\nCONEXIONES:\n");
		pc=conx+conxs[i];
		if(*pc==0) printf("Ninguna.\n");
		else do {
			j=saca_pal(*pc,_VERB);  /* si no es verbo, quiz  sea nombre */
			if(j==(NUM_PAL+1)) j=saca_pal(*pc,_NOMB);
			printf(" %s a %3u\n",vocabulario[j].p,*(pc+1));
			pc+=2;
		} while(*pc);
	}
}

if(si_o_no("\n\n¨Ver Objetos? "))
{
	for(i=0;i<(int)objact;i++) {
		printf("\n%3i: ",i);
		pc=obj+objs[i];
		printf("%s",vocabulario[saca_pal(*pc,_NOMB)].p);
		pc++;
		if(*pc==(char)NO_PAL) printf(" _    ");
		else printf(" %5s",vocabulario[saca_pal(*pc,_ADJT)].p);
		pc++;
		printf(" %3i ",(int)*pc);
		pc++;
		print_bin(*pc);
		printf(" ");
		pc++;
		print_bin(*pc);
		print_bin(*(pc+1));
		printf(" ");
		pc+=2;
		while(*pc) {    /* comprobamos antes por si es mensaje nulo */
			printf("%c",*pc++);
		}
	}
}

if(si_o_no("\n\n¨Ver Procesos? "))
{
	for(i=0;i<(int)proact;i++) {
		printf("\nProceso %3i\n",i);
		pcp=proc+pro[i];
		while(*pcp) {
			/* imprime verbo-nombre entrada */
			printf("\n%5u: ",pcp-proc);
			if(*pcp==NO_PAL) printf("_      ");
			else {
				j=saca_pal(*pcp,_VERB); /* si no es verbo, quiz  sea nombre */
				if(j==(NUM_PAL+1)) j=saca_pal(*pcp,_NOMB);
				printf("%s  ",vocabulario[j].p);
			}
			pcp++;
			if(*pcp==NO_PAL) printf("_      ");
			else printf("%s  ",vocabulario[saca_pal(*pcp,_NOMB)].p);
			pcp+=3; /* desprecia desplz. sgte. entrada */
			pralin=1;   /* indica que es 1era lin. de entrada */
			while(*pcp) {
				if(!pralin) printf("%5u:               ",pcp-proc);
				indir1=0;
				indir2=0;
				switch(*pcp) {
					case INDIR1 :
						indir1=1;
						pcp++;
						break;
					case INDIR2 :
						indir2=1;
						pcp++;
						break;
					case INDIR12 :
						indir1=1;
						indir2=1;
						pcp++;
						break;
				}
				cd=*pcp++;   /* coge n£mero de condacto */
				printf("%s   ",c[cd].cnd);   /* imprime condacto */
				switch(c[cd].tipo) {
					case 0 :
						printf("\n");
						break;
					case 1 :
						if(indir1) printf("[%3u]\n",*pcp);
						else printf("%5u\n",*pcp);
						pcp++;
						break;
					case 2 :
						if(indir1 && indir2)
						  printf("[%3u] [%3u]\n",*pcp,*(pcp+1));
						else if(indir1) printf("[%3u] %5u\n",*pcp,*(pcp+1));
						else if(indir2) printf("%5u [%3u]\n",*pcp,*(pcp+1));
						else printf("%5u %5u\n",*pcp,*(pcp+1));
						pcp+=2;
						break;
					case 3 :
						dirrel=(*(pcp+1) << 8) | *pcp;
						if(pralin) dirrel+=4;   /* ajusta si 1era lin. */
						printf("%5i\n",dirrel);
						pcp+=2;
						break;
					case 4 :
						printf("%s\n",vocabulario[saca_pal(*pcp,_ADJT)].p);
						pcp++;
						break;
					case 5 :
						printf("%s\n",vocabulario[saca_pal(*pcp,_NOMB)].p);
						pcp++;
						break;
					case 6 :
						if(indir1 && indir2)
						  printf("[%3u] [%3u] ",*pcp,*(pcp+1));
						else if(indir1) printf("[%3u] %5u ",*pcp,*(pcp+1));
						else if(indir2) printf("%5u [%3u] ",*pcp,*(pcp+1));
						else printf("%5u %5u ",*pcp,*(pcp+1));
						printf("%5u %5u %5u %5u\n",*(pcp+2),*(pcp+3),
						  *(pcp+4),*(pcp+5));
						pcp+=6;
						break;
					case 7 :
						if(*pcp==NO_PAL) printf("_      ");
						else {
							j=saca_pal(*pcp,_VERB); /* quiz  sea nomb conv */
							if(j==(NUM_PAL+1)) j=saca_pal(*pcp,_NOMB);
							printf("%s  ",vocabulario[j].p);
						}
						pcp++;
						if(*pcp==NO_PAL) printf("_    \n");
						else
						  printf("%s\n",vocabulario[saca_pal(*pcp,_NOMB)].p);
						pcp++;
						break;
					case 8 :
						if(indir1 && indir2)
						  printf("[%3u] [%3u] ",*pcp,*(pcp+1));
						else if(indir1) printf("[%3u] %5u ",*pcp,*(pcp+1));
						else if(indir2) printf("%5u [%3u] ",*pcp,*(pcp+1));
						else printf("%5u %5u ",*pcp,*(pcp+1));
						printf("%5u\n",*(pcp+2));
						pcp+=3;
						break;

				}
			pralin=0;
			}
		pcp++;  /* salta fin de entrada */
		}
	}
}
}

/****************************************/
/*************** Funciones **************/
/****************************************/

/*****************************************************************************
	SI_O_NO: imprime una pregunta y devuelve 1 si se contest¢ afirmativa-
	  mente o 0 si no.
*****************************************************************************/
int si_o_no(char *preg)
{
printf("%s",preg);
if((mayuscula((char)getche()))=='N') return(0);
return(1);
}

/*****************************************************************************
	RD_FICH: lee del fichero de entrada f_in, un bloque de BYTES y los
	  almacena en un buffer de memoria far.
*****************************************************************************/
int rd_fich(char far *buff,unsigned tam)
{
BYTE cod;
int c;

while(tam) {
	if((c=fgetc(f_in))==EOF) {    /* lee dato */
		if(feof(f_in)) {
			printf("--- Fin de fichero ---\n");
			exit(0);
		}
		if(ferror(f_in)) {
			printf("Error en fichero de entrada.\n");
			exit(1);
		}
	}
	cod=(BYTE)0xff-(BYTE)c;     /* decodifica BYTE */
	*buff=cod;                  /* almacena BYTE */
	buff++;                     /* siguiente BYTE */
	tam--;
}
return(0);  /* indica que no hubo error */
}

/*****************************************************************************
	FRD2: controla la entrada de datos desde el fichero de entrada
	  mediante la funci¢n fread.
*****************************************************************************/
void frd2(void *buff,size_t tam,size_t cant)
{
if(fread(buff,tam,cant,f_in)!=cant) {
	if(feof(f_in)) {
		printf("--- Fin de fichero ---\n");
		return;
	}
	if(ferror(f_in)) {
		printf("Error en fichero de entrada.\n");
		exit(1);
	}
}
}

/*****************************************************************************
	SACA_PAL: devuelve el n£mero de la primera entrada en el vocabulario
	  que se corresponda con el n£mero y tipo de palabra dado.
	  Si no se encontr¢ ninguna entrada se devuelve (NUM_PAL+1).
*****************************************************************************/
int saca_pal(BYTE num_pal,BYTE tipo_pal)
{
int i;
for(i=0;i<pvoc;i++)
  if((vocabulario[i].num==num_pal) && (vocabulario[i].tipo==tipo_pal))
  return(i);
return(NUM_PAL+1);  /* n£mero de palabra no existe */
}

/*****************************************************************************
	PRINT_BIN: imprime un BYTE en forma binaria.
*****************************************************************************/
void print_bin(BYTE b)
{
int i;
BYTE mascara=0x80;

for(i=0;i<8;i++) {
	if(b & mascara) printf("1");
	else printf("0");
	mascara >>= 1;
}
}

/*****************************************************************************
	MAYUSCULA: convierte una letra en may£scula
*****************************************************************************/
char mayuscula(char c)
{
if((c>='a') && (c<='z')) return(c-'a'+'A');
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
