/****************************************************************************
				    PCXVIDEO
			      (c)1993 JSJ Soft Ltd.

	Este programa permite visualizar ficheros gr ficos en formato
	PCX, formato ampliamente soportado por los programas de dibujo
	para ordenadores PC y compatibles.
	Esta versi¢n soporta ficheros con 16 y 256 colores.
	El programa se invoca desde la l¡nea del DOS:

	PCXVIDEO fichero.pcx [x] [y] [ancho] [alto] [modo_vis]

	Los par metros entre corchetes son opcionales. Estos par metros son:

		- x, y: indican las coordenadas de pantalla d¢nde se visuali-
		zar  el gr fico. El origen de la pantalla (0,0) est  en la
		esquina superior izquierda. La coordenada x se redondear  a un
		valor m£ltiplo de 8 en im genes de 16 colores.
		- ancho, alto: especifican la porci¢n del gr fico a visua-
		lizar. El par metro ancho se redondear  a un m£ltiplo de 8 en
		im genes de 16 colores.
		Si se especifica 0 en alguno de estos par metros se visuali-
		zar  el gr fico en su totalidad en la dimensi¢n correspon-
		diente. As¡ ancho=0, alto=0 visualiza el gr fico completo,
		mientras que ancho=0, alto=50 visualizar  s¢lo las 50 primeras
		l¡neas del gr fico.
		- modo_vis: puede ser uno de estos valores:
			0 - lee el gr fico del disco pero no lo visualiza
			hasta el final.
			1 - visualiza el gr fico seg£n lo lee del disco
		Cualquier otro valor producir  el mismo efecto que 0.

		C¢digos de salida:
			 0 - sin error
			 1 - error de apertura de fichero PCX
			 2 - error de lectura de fichero PCX
			 3 - fichero no v lido
			 4 - modo de pantalla incorrecto (no se deber¡a
			     llegar a producir)
			 5 - falta memoria para la imagen (no se deber¡a
			     llegar a producir)
			11 - no se encontr¢ VGA
			12 - n£mero de colores de imagen no soportado (debe
			     ser 16 o 256)
****************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <conio.h>
#include <process.h>
#include <graph.h>
#include "pcx.h"

/*** Variables globales ***/
char *MErr_ApFich="Error de apertura de fichero";
char *MErr_LectFich="Error de lectura de fichero";
char *MErr_FormNVal="Formato de fichero no v lido";
char *MErr_NoVGA="Tarjeta gr fica VGA no disponible";
char *MErr_NumCol="N£mero de colores no soportado";

/*** Prototipos ***/
void error_pcx(int err);

void main(int argc, char *argv[], char *envp[])
{
STC_CABPCX pcx;
FILE *fichpcx;
char nombre_fich[256];
int i, num_colores, par[5];

/* inicializa par metros a 0 */
for(i=0; i<5; par[i++]=0);

if(argc<2) {
	printf("\nVisualizador de im genes PCX, 16 y 256 colores."
	       "\n(c)1993 JSJ Soft Ltd.");
	printf("\n\nNombre del fichero PCX: ");
	gets(nombre_fich);
	printf("Coordenada X: ");
	scanf("%d",&par[0]);
	printf("Coordenada Y: ");
	scanf("%d",&par[1]);
	printf("Ancho (0=m ximo): ");
	scanf("%d",&par[2]);
	printf("Alto (0=m ximo): ");
	scanf("%d",&par[3]);
	printf("Modo (0 o 1): ");
	scanf("%d",&par[4]);
}
else {
	strcpy(nombre_fich,argv[1]);
	/* s¢lo admite 5 par metros */
	argc=(argc>7 ? 7 : argc);
	/* recoge los par metros que pueda de la l¡nea del DOS */
	for(i=2; i<argc; i++) par[i-2]=atoi(argv[i]);
}

/* lee cabecera de fichero PCX */
if((fichpcx=fopen(nombre_fich,"rb"))==NULL) error_pcx(1);
if(fread(&pcx,sizeof(STC_CABPCX),1,fichpcx)!=1) {
	fclose(fichpcx);
	error_pcx(2);
}
fclose(fichpcx);

/* ajusta modo de pantalla de acuerdo al n§ de colores de la imagen */
num_colores=1 << (pcx.bits_pixel*pcx.num_planos);
if(num_colores==16) {
	if(!_setvideomode(_VRES16COLOR)) error_pcx(11);
}
else if(num_colores==256) {
	if(!_setvideomode(_MRES256COLOR)) error_pcx(11);
}
else error_pcx(12);

/* lee imagen y la dibuja en pantalla */
if(num_colores==16) {
	if(par[4]==1) i=pcx_16p(nombre_fich,(short)par[0],(short)par[1],
	  par[2],par[3],0);
	else i=pcx_16(nombre_fich,(short)par[0],(short)par[1],par[2],par[3],
	  0);
}
else {
	if(par[4]==1) i=pcx_256p(nombre_fich,(short)par[0],(short)par[1],
	  par[2],par[3],0);
	else i=pcx_256(nombre_fich,(short)par[0],(short)par[1],par[2],par[3],
	  0);
}

/* si no hubo error hace pausa para poder ver la imagen */
if(!i) getch();

_setvideomode(_DEFAULTMODE);

error_pcx(i);
}

/****************************************************************************
	ERROR_PCX: imprime un mensaje de error y sale al sistema operativo.
	  Entrada:      'err' c¢digo de error
****************************************************************************/
void error_pcx(int err)
{
char *merr;

switch(err) {
	case 1 :
		merr=MErr_ApFich;
		break;
	case 2 :
		merr=MErr_LectFich;
		break;
	case 3 :
		merr=MErr_FormNVal;
		break;
	case 11 :
		merr=MErr_NoVGA;
		break;
	case 12 :
		merr=MErr_NumCol;
		break;
	default :
		merr=NULL;
}

if(merr!=NULL) printf("\nError: %s.\n",merr);

exit(err);

}
