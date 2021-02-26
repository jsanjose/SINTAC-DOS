/****************************************************************************
			       LISTADOR SINTAC
			    (c)1993 JSJ Soft Ltd.
****************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <conio.h>
#include "version.h"
#include "sintac.h"
#include "ls.h"

/* tabla de nombre-tipo de condactos */
#include "tabcond.h"

/* cabecera */
CAB_SINTAC cab;

/* variables para Vocabulario */
struct palabra vocabulario[NUM_PAL];    /* para almacenar vocabulario */

/* variables para Mensajes */
unsigned tab_desp_msg[MAX_MSG];         /* tabla desplazamientos mensajes */
char *tab_msg;                          /* puntero a inicio zona mensajes */

/* variables para Mensajes de Sistema */
unsigned tab_desp_msy[MAX_MSY];         /* tabla desplazamientos mens.sist. */
char *tab_msy;                          /* ptro. inicio zona mens. sistema */

/* variables para Localidades */
unsigned tab_desp_loc[MAX_LOC];         /* tabla desplazamientos textos loc. */
char *tab_loc;                          /* puntero inicio textos localidades */
/* variables para Conexiones */
unsigned tab_desp_conx[MAX_LOC];        /* tabla desplazamientos conexiones */
BYTE *tab_conx;                         /* puntero inicio zona conexiones */

/* variables para Objetos */
unsigned tab_desp_obj[MAX_OBJ];         /* tabla desplazamientos de objetos */
char *tab_obj;                          /* puntero a inicio zona de objetos */

/* variables para Procesos */
unsigned tab_desp_pro[MAX_PRO];         /* tabla desplazamientos procesos */
BYTE *tab_pro;                          /* puntero a inicio zona de procesos */

/*** Programa principal ***/
void main(int argc, char *argv[])
{
FILE *f_in;
char *formato="%-20s : %5i %5u\n";
char *errmem="No hay suficiente memoria.\n";
char *srecon=SRECON;
char nombre[_MAX_PATH];
char *pc;
int i, j;
BYTE *pcp;
int cd, indir1, indir2, pralin, dirrel;

printf("\n"COPYRIGHT);
printf("Listador Versi¢n %s\n\n",VERSION_LS);

if(argc<2) {
	printf("Nombre del fichero a listar: ");
	gets(nombre);
	for(i=0; nombre[i]; i++) nombre[i]=mayuscula(nombre[i]);
}
else {
	for(i=0; argv[1][i]; i++) nombre[i]=mayuscula(argv[1][i]);
	nombre[i]='\0';
}
if ((f_in=fopen(nombre,"rb"))==NULL) {
	printf("Error de apertura de fichero de entrada [%s].\n",nombre);
	exit(1);
}


/* lee cabecera */
fread(&cab,sizeof(CAB_SINTAC),1,f_in);

/* comprueba que la versi¢n de la base de datos sea correcta */
if((cab.srecon[L_RECON-2]!=srecon[L_RECON-2]) ||
  (cab.srecon[L_RECON-1]!=srecon[L_RECON-1])) {
	printf("\nFichero de base de datos no v lido.\n");
	exit(1);
}

printf("\nSECCION                 NUM. BYTES\n");
printf(formato,"Vocabulario",cab.pal_voc,
  (unsigned)(cab.pal_voc*sizeof(struct palabra)));
printf(formato,"Mensajes",(int)cab.num_msg,cab.bytes_msg);
printf(formato,"Mensajes del Sistema",(int)cab.num_msy,cab.bytes_msy);
printf(formato,"Localidades",(int)cab.num_loc,cab.bytes_loc);
printf(formato,"Conexiones",(int)cab.num_loc,cab.bytes_conx);
printf(formato,"Objetos",(int)cab.num_obj,cab.bytes_obj);
printf(formato,"Procesos",(int)cab.num_pro,cab.bytes_pro);
printf("Palabras de movimiento < %3u\n",(unsigned)cab.v_mov);
printf("Nombres convertibles   < %3u\n",(unsigned)cab.n_conv);
printf("Nombres propios        < %3u\n",(unsigned)cab.n_prop);

/* Reserva de memoria para las distintas secciones */
/* -- Mensajes -- */
if((tab_msg=(char *)malloc((size_t)cab.bytes_msg))==NULL) {
	printf(errmem);
	exit(1);
}

/* -- Mensajes del Sistema -- */
if((tab_msy=(char *)malloc((size_t)cab.bytes_msy))==NULL) {
	printf(errmem);
	exit(1);
}

/* -- Localidades -- */
if((tab_loc=(char *)malloc((size_t)cab.bytes_loc))==NULL) {
	printf(errmem);
	exit(1);
}
/* -- Conexiones -- */
if((tab_conx=(BYTE *)malloc((size_t)cab.bytes_conx))==NULL) {
	printf(errmem);
	exit(1);
}

/* -- Objetos -- */
if((tab_obj=(char *)malloc((size_t)cab.bytes_obj))==NULL) {
	printf(errmem);
	exit(1);
}

/* -- Procesos -- */
if((tab_pro=(BYTE *)malloc((size_t)cab.bytes_pro))==NULL) {
	printf(errmem);
	exit(1);
}

fread(vocabulario,sizeof(struct palabra),cab.pal_voc,f_in);

fread(tab_desp_msg,sizeof(unsigned),(size_t)MAX_MSG,f_in);
fread(tab_msg,sizeof(char),cab.bytes_msg,f_in);

fread(tab_desp_msy,sizeof(unsigned),(size_t)MAX_MSY,f_in);
fread(tab_msy,sizeof(char),cab.bytes_msy,f_in);

fread(tab_desp_loc,sizeof(unsigned),(size_t)MAX_LOC,f_in);
fread(tab_loc,sizeof(char),cab.bytes_loc,f_in);

fread(tab_desp_conx,sizeof(unsigned),(size_t)MAX_LOC,f_in);
fread(tab_conx,sizeof(BYTE),cab.bytes_conx,f_in);

fread(tab_desp_obj,sizeof(unsigned),(size_t)MAX_OBJ,f_in);
fread(tab_obj,sizeof(char),cab.bytes_obj,f_in);

fread(tab_desp_pro,sizeof(unsigned),(size_t)MAX_PRO,f_in);
fread(tab_pro,sizeof(BYTE),cab.bytes_pro,f_in);

/* decodifica las secciones */
codifica((BYTE *)tab_msg,cab.bytes_msg);
codifica((BYTE *)tab_msy,cab.bytes_msy);
codifica((BYTE *)tab_loc,cab.bytes_loc);
codifica(tab_conx,cab.bytes_conx);
codifica((BYTE *)tab_obj,cab.bytes_obj);
codifica(tab_pro,cab.bytes_pro);

if(si_o_no("\n\n¨Ver Vocabulario? "))
{
	printf("\n\tPALABRA\tNUMERO\tTIPO");
	for(i=0; i<cab.pal_voc; i++) {
		printf("\n%3i:\t%s\t%i\t",i,vocabulario[i].p,
		  vocabulario[i].num);
		switch(vocabulario[i].tipo) {
		case _VERB :
			if(vocabulario[i].num<cab.v_mov)
			  printf("verbo movimiento");
			else printf("verbo");
			break;
		case _NOMB :
			if(vocabulario[i].num<cab.v_mov) printf("movimiento");
			else if(vocabulario[i].num<cab.n_conv)
			  printf("nombre convertible");
			else if(vocabulario[i].num<cab.n_prop)
			  printf("nombre propio");
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
	for(i=0; i<(int)cab.num_msg; i++) {
		printf("\n%3i: ",i);
		pc=tab_msg+tab_desp_msg[i];
		/* comprobamos antes por si es mensaje nulo */
		while(*pc) {
			printf("%c",*pc++);
		}

	}
}

if(si_o_no("\n\n¨Ver Mensajes del Sistema? "))
{
	for(i=0; i<(int)cab.num_msy; i++) {
		printf("\n%3i: ",i);
		pc=tab_msy+tab_desp_msy[i];
		/* comprobamos antes por si es mensaje nulo */
		while(*pc) {
			printf("%c",*pc++);
		}
	}
}

if(si_o_no("\n\n¨Ver Localidades? "))
{
	for(i=0; i<(int)cab.num_loc; i++) {
		printf("\n%3i: ",i);
		pc=tab_loc+tab_desp_loc[i];
		/* comprobamos antes por si es mensaje nulo */
		while(*pc) {
			printf("%c",*pc++);
		}
		printf("\nCONEXIONES:\n");
		pcp=tab_conx+tab_desp_conx[i];
		if(*pcp==0) printf("Ninguna.\n");
		else do {
			j=saca_pal(*pcp,_VERB);
			/* si no es verbo, quiz  sea nombre */
			if(j==(NUM_PAL+1)) j=saca_pal(*pcp,_NOMB);
			printf(" %s a %3u\n",vocabulario[j].p,*(pcp+1));
			pcp+=2;
		} while(*pcp);
	}
}

if(si_o_no("\n\n¨Ver Objetos? "))
{
	for(i=0; i<(int)cab.num_obj; i++) {
		printf("\n%3i: ",i);
		pc=tab_obj+tab_desp_obj[i];
		printf("%s",vocabulario[saca_pal(*pc,_NOMB)].p);
		pc++;
		if(*pc==(char)NO_PAL) printf(" _     ");
		else printf(" %s",vocabulario[saca_pal(*pc,_ADJT)].p);
		pc++;
		printf(" %3u ",(BYTE)*pc);
		pc++;
		print_bin(*pc);
		printf(" ");
		pc++;
		print_bin(*pc);
		print_bin(*(pc+1));
		printf(" ");
		pc+=2;
		/* comprobamos antes por si es mensaje nulo */
		while(*pc) {
			printf("%c",*pc++);
		}
	}
}

if(si_o_no("\n\n¨Ver Procesos? "))
{
	for(i=0; i<(int)cab.num_pro; i++) {
		printf("\nProceso %3i\n",i);
		pcp=tab_pro+tab_desp_pro[i];
		while(*pcp) {
			/* imprime verbo-nombre entrada */
			printf("\n%5u: ",pcp-tab_pro);
			if(*pcp==NO_PAL) printf("_       ");
			else {
				/* si no es verbo, quiz  sea nombre */
				j=saca_pal(*pcp,_VERB);
				if(j==(NUM_PAL+1)) j=saca_pal(*pcp,_NOMB);
				printf("%s  ",vocabulario[j].p);
			}
			pcp++;
			if(*pcp==NO_PAL) printf("_       ");
			else printf("%s  ",
			  vocabulario[saca_pal(*pcp,_NOMB)].p);
			pcp++;
			/* imprime desplazamiento siguiente entrada */
			printf("(%5u)  ",(unsigned)((*(pcp+1) << 8) | *pcp));
			/* salta desplazamiento siguiente entrada */
			pcp+=2;
			/* indica que es 1era l¡nea de entrada */
			pralin=1;
			while(*pcp) {
				if(!pralin) printf("%5u:              "
				  "            ",pcp-tab_pro);
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
				/* coge n£mero de condacto y lo imprime */
				cd=*pcp++;
				printf("%s   ",condacto[cd].cnd);
				switch(condacto[cd].tipo) {
				case 0 :
					printf("\n");
					break;
				case 1 :
				case 3 :
				case 5 :
				case 6 :
				case 7 :
				case 8 :
				case 10 :
				case 12 :
				case 17 :
				case 19 :
				case 20 :
				case 23 :
				case 25 :
				case 26 :
					if(indir1) printf("[%3u]\n",*pcp);
					else printf("%5u\n",*pcp);
					pcp++;
					break;
				case 4 :
				case 9 :
				case 15 :
				case 18 :
				case 22 :
				case 24 :
				case 27 :
					if(indir1 && indir2)
					  printf("[%3u] [%3u]\n",*pcp,
					  *(pcp+1));
					else if(indir1) printf("[%3u] %5u\n",
					  *pcp,*(pcp+1));
					else if(indir2) printf("%5u [%3u]\n",
					  *pcp,*(pcp+1));
					else printf("%5u %5u\n",*pcp,
					  *(pcp+1));
					pcp+=2;
					break;
				case 11 :
					dirrel=(*(pcp+1) << 8) | *pcp;
					/* ajusta si 1era l¡nea */
					if(pralin) dirrel+=4;
					printf("%5i\n",dirrel);
					pcp+=2;
					break;
				case 13 :
					printf("%s\n",
					  vocabulario[saca_pal(*pcp,
					  _ADJT)].p);
					pcp++;
					break;
				case 14 :
					printf("%s\n",
					  vocabulario[saca_pal(*pcp,
					  _NOMB)].p);
					pcp++;
					break;
				case 2 :
					if(indir1 && indir2)
					  printf("[%3u] [%3u] ",*pcp,
					  *(pcp+1));
					else if(indir1) printf("[%3u] %5u ",
					  *pcp,*(pcp+1));
					else if(indir2) printf("%5u [%3u] ",
					  *pcp,*(pcp+1));
					else printf("%5u %5u ",*pcp,*(pcp+1));
					printf("%5u %5u %5u %5u\n",*(pcp+2),
					  *(pcp+3),*(pcp+4),*(pcp+5));
					pcp+=6;
					break;
				case 16 :
					if(*pcp==NO_PAL) printf("_       ");
					else {
						/* quiz  sea nomb conv */
						j=saca_pal(*pcp,_VERB);
						if(j==(NUM_PAL+1))
						  j=saca_pal(*pcp,_NOMB);
						printf("%s  ",
						  vocabulario[j].p);
					}
					pcp++;
					if(*pcp==NO_PAL) printf("_     \n");
					else
					  printf("%s\n",
					    vocabulario[saca_pal(*pcp,
					    _NOMB)].p);
					pcp++;
					break;
				case 21 :
					if(indir1 && indir2)
					  printf("[%3u] [%3u] ",*pcp,
					    *(pcp+1));
					else if(indir1) printf("[%3u] %5u ",
					  *pcp,*(pcp+1));
					else if(indir2) printf("%5u [%3u] ",
					  *pcp,*(pcp+1));
					else printf("%5u %5u ",*pcp,*(pcp+1));
					printf("%5u\n",*(pcp+2));
					pcp+=3;
					break;

				}
			pralin=0;
			}
		pcp++;          /* salta fin de entrada */
		}
	}
}

fclose(f_in);
}

/****************************************************************************
	CODIFICA: codifica/decodifica una tabla de secci¢n.
	  Entrada:      'mem' puntero a la tabla a codificar/decodificar
			'bytes_mem' tama¤o de la tabla
****************************************************************************/
void codifica(BYTE *mem, unsigned bytes_mem)
{
BYTE *p, *ult_p;

p=mem;
ult_p=p+bytes_mem;

for(; p<ult_p; p++) *p=CODIGO(*p);
}

/****************************************************************************
	SI_O_NO: imprime una pregunta y espera a respuesta.
	  Entrada:      'preg' texto de la pregunta
	  Salida:       FALSE si se contest¢ 'N'
			TRUE en otro caso
****************************************************************************/
int si_o_no(char *preg)
{
printf("%s",preg);
if((mayuscula((char)getche()))=='N') return(FALSE);
return(TRUE);
}

/****************************************************************************
	SACA_PAL: devuelve el n£mero de la primera entrada en el vocabulario
	  que se corresponda con el n£mero y tipo de palabra dado.
	  Entrada:      'num_pal' n£mero de palabra de vocabulario
			'tipo_pal' tipo de palabra a buscar
	  Salida:       posici¢n dentro de la tabla de vocabulario o
			(NUM_PAL+1) si no se encontr¢
****************************************************************************/
int saca_pal(BYTE num_pal, BYTE tipo_pal)
{
int i;

for(i=0; i<cab.pal_voc; i++) {
	if((vocabulario[i].num==num_pal) && (vocabulario[i].tipo==tipo_pal))
	  return(i);
}

return(NUM_PAL+1);  /* n£mero de palabra no existe */
}

/****************************************************************************
	PRINT_BIN: imprime un BYTE en forma binaria.
	  Entrada:      'b' valor a imprimir
	  Salida:       valor impreso en binario
****************************************************************************/
void print_bin(BYTE b)
{
int i;
BYTE mascara=0x80;

for(i=0; i<8; i++) {
	if(b & mascara) printf("1");
	else printf("0");
	mascara >>= 1;
}
}

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
