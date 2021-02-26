/****************************************************************************
				    PCXVIDEO
			      (c)1993 JSJ Soft Ltd.

	Este programa permite visualizar ficheros gr ficos en formato
	PCX, formato ampliamente soportado por los programas de dibujo
	para ordenadores PC y compatibles.
	Esta versi¢n soporta ficheros con 16 colores de EGA y VGA, adem s
	de ficheros de 256 de VGA.
	El programa se invoca desde la l¡nea del DOS:

	PCXVIDEO fichero.pcx [modo_pantalla] [x] [y] [ancho] [alto] [modo_vis]

	Los par metros entre corchetes son opcionales. Estos par metros son:

		- modo_pantalla: admite 3 valores E, V o M. Con E se sele-
		cciona un modo de visualizaci¢n con una resoluci¢n de 640x350
		de 16 colores, apto tanto para EGA como VGA. La opci¢n V
		selecciona un modo de 640x480 de 16 colores, apto s¢lo para
		VGA; si no se pudiese seleccionar este modo se intentar¡a con
		el de 640x350 de 16 colores.
		Por £ltimo la opci¢n M selecciona un modo de 320x200 de 256
		colores, apto s¢lo para VGA.
		Cualquier otro valor en este par metro causa que se seleccione
		el modo de mayor resoluci¢n permitido entre el de 640x350 y
		640x480 de 16 colores.
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
			0 - visualiza el gr fico seg£n lo lee del disco
			1 - lee el gr fico del disco pero no lo visualiza
			hasta el final. La pantalla permanecer  en blanco
			hasta que se haya le¡do la imagen.
			2 - visualiza el gr fico pero sin modificar la paleta
			actual de colores.
		Cualquier otro valor producir  el mismo efecto que 2.

		C¢digos de salida:
			 0 - sin error
			 1 - error de apertura de fichero PCX
			 2 - error de lectura de fichero PCX
			 3 - fichero no v lido
			 4 - modo de pantalla incorrecto (no se deber¡a
			     llegar a producir)
			10 - falta nombre de fichero PCX
			11 - no se encontr¢ EGA o VGA
****************************************************************************/

#include <stdlib.h>
#include <conio.h>
#include <process.h>
#include <graph.h>
#include "pcx.h"

void main(int argc, char *argv[], char *envp[])
{
int i, par[5], modo256=0;

if(argc<2) exit(10);

/* si hay al menos 2 par metros, el segundo ser  un car cter */
/* que identifique el modo de pantalla a usar para visualizar la imagen */
/* 'E' para EGA 16 colores,  'V' para VGA 16 colores o 'M' para VGA */
/* 256 colores */
if(argc>2) {
	switch(argv[2][0]) {
		case 'v' :
		case 'V' :
			/* si falla al seleccionar VGA, prueba con EGA */
			if(_setvideomode(_VRES16COLOR)) break;
		case 'e' :
		case 'E' :
			if(!_setvideomode(_ERESCOLOR)) exit(11);
			break;
		case 'm' :
		case 'M' :
			if(!_setvideomode(_MRES256COLOR)) exit(11);
			/* indica que se seleccion¢ el modo de 256 colores */
			modo256=1;
			break;
		default :
			/* cualquier otro par metro selecciona el modo */
			/* de mayor resoluci¢n permitido entre el de */
			/* 640x480 a 16 colores y el de 640x350 a 16 colores */
			if(!_setvideomode(_VRES16COLOR)) {
				if(!_setvideomode(_ERESCOLOR)) exit(11);
			}
	}
}
/* si no, coge el modo de resoluci¢n m s alto posible */
else {
	if(!_setvideomode(_VRES16COLOR)) {
		if(!_setvideomode(_ERESCOLOR)) exit(11);
	}
}

/* inicializa par metros a 0 */
for(i=0; i<5; par[i++]=0);

/* s¢lo admite 6 par metros */
argc=(argc>8 ? 8 : argc);

/* recoge los par metros que pueda de la l¡nea del DOS */
for(i=3; i<argc; i++) par[i-3]=atoi(argv[i]);

/* lee imagen y la dibuja en pantalla */
/* seg£n el modo activo (16 o 256 colores) */
if(!modo256)
  i=pcx_decodif16(argv[1],(short)par[0],(short)par[1],par[2],par[3],par[4]);
else
  i=pcx_decodif256(argv[1],(short)par[0],(short)par[1],par[2],par[3],par[4]);

/* si no hubo error hace pausa para poder ver la imagen */
if(!i) getch();

_setvideomode(_DEFAULTMODE);

exit(i);
}

