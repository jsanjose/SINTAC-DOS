/****************************************************************************
				    PCXHEAD.C
			      (c)1993 JSJ Soft Ltd.

	Este programa visualiza la informaci¢n contenida en la cabecera
	de un fichero PCX.
	Se ejecuta desde la l¡nea del DOS mediante:

	PCXHEAD [fichero.pcx]

	Despu‚s de introducir el nombre del fichero (si no se introdujo en
	la l¡nea del DOS) se proceder  a leer el mismo y presentar la
	informaci¢n correspondiente.

	C¢digos de salida:
		0 - sin error
		1 - error de apertura de fichero .PCX
		2 - error de lectura de fichero .PCX
****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <string.h>
#include <conio.h>

typedef unsigned char BYTE;

typedef struct _PCXHEADER {
	BYTE propietario;       /* deber¡a ser siempre 0x0A */
	BYTE version;           /* version del fichero PCX */
	BYTE metodo_codif;      /* 1=codificaci¢n PCX 'run-lenght' */
	BYTE bits_pixel;        /* n£mero de bits por pixel */
	int ventana[4];         /* tama¤o dibujo (izq., arr., der., abajo) */
	int resh, resv;         /* resoluci¢n horizontal y vertical */
	BYTE paleta[16][3];     /* informaci¢n de paleta para la imagen */
	BYTE reserv1;
	BYTE num_planos;        /* n£mero de planos de color */
	int bytes_scan;         /* n£mero de bytes por l¡nea de scan */
	BYTE reserv2[60];
} PCXHEADER;

void tecla(void);

void main(int argc, char *argv[], char *envp[])
{
char nombre[81];
FILE *fichpcx;
PCXHEADER inf_pcx;
int i, j;

printf("\nPCXHEAD - (c)1993 JSJ Soft Ltd.\n\n");

if(argc<2) {
	printf("Nombre del fichero PCX: ");
	gets(nombre);
}
else strcpy(nombre,argv[1]);

if((fichpcx=fopen(nombre,"rb"))==NULL) {
	printf("\nError de apertura.\n");
	exit(1);
}

if(fread(&inf_pcx,sizeof(PCXHEADER),1,fichpcx)!=1) {
	printf("\nError de lectura.\n");
	exit(2);
}

fclose(fichpcx);

printf("\n--- Informaci¢n del fichero %s\n",nombre);
printf("\nPropietario\t\t\t%u\n",inf_pcx.propietario);
printf("Versi¢n\t\t\t\t%u\n",inf_pcx.version);
printf("M‚todo de codificaci¢n\t\t%u\n",inf_pcx.metodo_codif);
printf("Bits por pixel\t\t\t%u\n",inf_pcx.bits_pixel);
printf("Ventana\t\tizquierda\t%i\n",inf_pcx.ventana[0]);
printf("\t\tarriba\t\t%i\n",inf_pcx.ventana[1]);
printf("\t\tderecha\t\t%i\n",inf_pcx.ventana[2]);
printf("\t\tabajo\t\t%i\n",inf_pcx.ventana[3]);
printf("Resoluci¢n\thorizontal\t%i\n",inf_pcx.resh);
printf("\t\tvertical\t%i\n",inf_pcx.resv);
printf("N£mero de planos\t\t%u\n",inf_pcx.num_planos);
printf("Bytes/l¡nea scan\t\t%i\n",inf_pcx.bytes_scan);

tecla();

for(i=0; i<16; i++) {
	printf("\nPaleta [%i]\t",i);
	for(j=0; j<3; j++) printf("%3u   ",inf_pcx.paleta[i][j]);
}
printf("\nReservado 1 = %u\n",inf_pcx.reserv1);
printf("Reservado 2 =\n");
for(i=0; i<60; i++) printf("%3u ",inf_pcx.reserv2[i]);
printf("\n");

exit(0);
}

void tecla(void)
{
printf("\nPulsa una tecla.\n");
getch();
}

