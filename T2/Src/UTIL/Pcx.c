/****************************************************************************
				     PCX.C

	Conjunto de funciones para visualizar ficheros gr ficos en
	formato PCX.

			    (c)1992 JSJ Soft Ltd.

	Las siguientes funciones son p£blicas:
		- pcx_decodif16: decodifica y dibuja en pantalla un fichero
		    PCX de 16 colores (EGA, VGA)
		- pcx_decodif256: decodifica y dibuja en pantalla un fichero
		    PCX de 256 colores (VGA)

	Las siguientes estructuras est n definidas en PCX.H:
		PCXHEADER: cabecera de un fichero PCX
****************************************************************************/

#include <stdio.h>
#include <graph.h>
#include "pcx.h"

/* buffer interno para decodificar l¡neas de scan */
/* aqu¡ se guardar n los 4 planos que componen una l¡nea de scan */
/* uno detr s de otro, sin separaciones */
BYTE _far lin_scan[TAMLINPCX16*4];

/* aqu¡ se guardar  la l¡nea de scan de 256 colores, antes de */
/* imprimir la porci¢n correspondiente en pantalla */
BYTE _far lin_scan256[TAMLINPCX256];

/*** Prototipos de funciones internas ***/
static void carga_paleta(const BYTE _far *nueva_paleta, int num_colores);

/****************************************************************************
	CARGA_PALETA: carga la paleta con colores dados.
	  Entrada:      'nueva_paleta' puntero a una tabla con los nuevos
			colores; cada entrada de la tabla contiene los 3
			componentes de color (rojo, verde y azul en este
			orden)
			'num_colores' n£mero de colores en la paleta
****************************************************************************/
void carga_paleta(const BYTE _far *nueva_paleta, int num_colores)
{

_asm {
	mov ax,1012h            ; actualizar bloque de registros del DAC
	mov bx,0                ; primer registro
	mov cx,num_colores      ; n£mero de registros a actualizar
	les dx,nueva_paleta     ; direcci¢n tabla rojo-verde-azul
	int 10h
}

}

/****************************************************************************
	PCX_DECODIF16: decodifica un fichero PCX de 16 colores (EGA, VGA) y
	  dibuja un trozo de imagen en la pantalla.
	  Entrada:      'nombre_pcx' nombre del fichero PCX
			'x', 'y' coordenadas de pantalla d¢nde se dibujar 
			el gr fico
			'anchura', 'altura' dimensiones (en pixels) de la
			zona del gr fico que se dibujar , si alguno de los
			valores es 0, se coger n las dimensiones dadas por
			la cabecera del fichero PCX
			'modo' modo de dibujo en pantalla de la imagen:
			PCX_PINTA va dibujando la imagen seg£n se lee del
			fichero, PCX_OCULTA dibuja la imagen pero no se
			visualiza hasta que est  completa, PCX_PALETA no
			modifica la paleta de colores
	  Salida:       0 si no hubo ning£n error o un valor distinto de 0
			si se produjo alg£n error
			Errores:
			  E_PCX_APER - error de apertura de fichero PCX
			  E_PCX_LECT - error de lectura de fichero PCX
			  E_PCX_FORM - formato de imagen incorrecto
			  E_PCX_MODO - modo de pantalla incorrecto
****************************************************************************/
int pcx_decodif16(char *nombre_pcx, short x, short y, int anchura, int altura,
  int modo)
{
FILE *fichpcx;
PCXHEADER pcx;
struct videoconfig vid;
long color[16];
int i, j, bytes_decodif, cuenta, ancho_scan, totbytes;
BYTE _far *pscan, _far *dirvideo;
BYTE bpcx;

/* obtiene informaci¢n de modo de v¡deo */
_getvideoconfig(&vid);

/* mira si es una EGA o VGA en modo de 16 colores y resoluci¢n */
/* horizontal de 640 pixels */
if(((vid.adapter!=_EGA) && (vid.adapter!=_VGA)) || (vid.numcolors!=16) ||
  (vid.numxpixels!=640)) return(E_PCX_MODO);

if((fichpcx=fopen(nombre_pcx,"rb"))==NULL) return(E_PCX_APER);

/* establece buffer para lectura de fichero */
setvbuf(fichpcx,NULL,_IOFBF,PCX_FICH_BUF);

/* lee la cabecera */
if(fread(&pcx,sizeof(PCXHEADER),1,fichpcx)!=1){
	fclose(fichpcx);
	return(E_PCX_LECT);
}

/* comprueba que sea una imagen con 4 planos de bits */
if(pcx.num_planos!=4) {
	fclose(fichpcx);
	return(E_PCX_FORM);
}

/* coge y convierte la informaci¢n de paleta, si es EGA la formatea */
/* para la funci¢n _remapallpalette(), si es VGA la dimensiona */
if(vid.adapter==_EGA) {
	for(i=0; i<vid.numcolors; i++) color[i]=RGB(pcx.paleta[i][0]/64,
	  pcx.paleta[i][1]/64,pcx.paleta[i][2]/64);
}
else {
	for(i=0; i<vid.numcolors; i++) {
		pcx.paleta[i][0]>>=2;
		pcx.paleta[i][1]>>=2;
		pcx.paleta[i][2]>>=2;
	}
}

/* si quiere dibujar la imagen mientras la lee, actualiza ahora la paleta */
/* para EGA usa la funci¢n _remapallpalette(), para VGA usa la funci¢n */
/* carga_paleta() con la informaci¢n de paleta de cabecera (dimensionada) */
if(modo==PCX_PINTA) {
	if(vid.adapter==_EGA) _remapallpalette(color);
	else carga_paleta(&pcx.paleta[0][0],vid.numcolors);
}
/* si quiere que la imagen no se vea, pone a 0 toda la paleta */
else if(modo==PCX_OCULTA) {
	for(i=0; i<vid.numcolors; i++) _remappalette(i,0L);
}

/* si la altura dada es 0, la calcula de acuerdo a la cabecera */
if(!altura) altura=pcx.ventana[3]-pcx.ventana[1]+1;
/* idem con la anchura */
if(!anchura) anchura=pcx.ventana[2]-pcx.ventana[0]+1;

/* si el gr fico a dibujar no cabe en la pantalla */
/* ajusta las dimensiones para que quepa */
if((y+altura)>vid.numypixels) altura=vid.numypixels-y;
if((x+anchura)>vid.numxpixels) anchura=vid.numxpixels-x;

/* calcula la anchura en bytes */
anchura/=8;

/* n£mero de bytes a decodificar por l¡nea de scan */
totbytes=pcx.bytes_scan*4;

/* direcci¢n en buffer de v¡deo de primer pixel a dibujar */
/* le suma el desplazamiento correspondiente */
dirvideo=(BYTE _far *)0xa0000000+((x/8)+(y*TAMLINPCX16));

/* inicializa puntero a buffer de l¡nea */
pscan=lin_scan;

/* bucle para dibujar la parte de la imagen PCX correspondiente */
for(j=0; j<altura; j++, dirvideo+=TAMLINPCX16) {
	/* inicializa contador de bytes decodificados */
	bytes_decodif=0;

	/* bucle para decodificar 4 planos de bits de una l¡nea */
	do {
		/* lee 1 byte del fichero PCX */
		if(fread(&bpcx,sizeof(BYTE),1,fichpcx)!=1) {
			fclose(fichpcx);
			return(E_PCX_LECT);
		}

		/* si los 2 bits altos est n a 1, recoge los 6 bits bajos */
		/* y lee el siguiente byte */
		if((bpcx & 0xc0)==0xc0) {
			cuenta=(int)(bpcx & 0x3f);
			if(fread(&bpcx,sizeof(BYTE),1,fichpcx)!=1) {
				fclose(fichpcx);
				return(E_PCX_LECT);
			}
		}
		else cuenta=1;

		/* expande byte */
		for(i=0; i<cuenta; i++) {
			*pscan++=bpcx;
			/* incrementa n£mero de bytes decodificados */
			bytes_decodif++;
			/* si se completa la l¡nea de scan, sale */
			if(bytes_decodif==totbytes) break;
		}
	} while(bytes_decodif<totbytes);

	/* dibuja la l¡nea de scan decodificada en las coordenadas */
	/* de pantalla (x,y1) */
	/* NOTA: la coordenada 'x' queda redondeada al byte m s cercano */

	ancho_scan=pcx.bytes_scan;
	/* inicializa puntero a buffer de l¡nea */
	pscan=lin_scan;

	_asm {
		push di
		push si
		push ds

		mov dx,03ceh            ; DX = puerto controlador de gr ficos
		mov ax,0005h            ; modo lectura 0, escritura 0
		out dx,ax
		mov ax,0003h            ; selecciona funci¢n 'Replace'
		out dx,ax
		mov ax,0ff08h           ; M scara de bit = 0FFH
		out dx,ax
		mov dl,0c4h             ; DX = puerto de secuenciador (03C4H)

		les di,dirvideo         ; ES:DI = A000:xxxx direcci¢n v¡deo

		mov ax,0102h            ; AH = 0001B, selecciona 1er plano
		lds si,pscan            ; DS:SI = direcci¢n buffer de l¡nea
	planobit:
		out dx,ax               ; selecciona plano de bits
		push di
		push si
		mov cx,anchura          ; CX = n£mero de bytes a transferir
		rep movsb               ; transfiere datos a plano activo
		pop si
		pop di
		add si,ancho_scan       ; siguiente plano en buffer de l¡nea
		shl ah,1                ; siguiente plano de bits
		cmp ah,16
		jnz planobit            ; bucle a trav‚s de los planos de bits

		; aqu¡ DX debe valer 03C4H
		mov ax,0f02h            ; valor por defecto de M scara de mapa
		out dx,ax

		pop ds
		pop si
		pop di
	}

}

fclose(fichpcx);

/* ahora actualiza la paleta para que se vea la imagen */
if(modo==PCX_OCULTA) {
	if(vid.adapter==_EGA) _remapallpalette(color);
	else carga_paleta(&pcx.paleta[0][0],vid.numcolors);
}

return(0);
}

/****************************************************************************
	PCX_DECODIF256: decodifica un fichero PCX de 256 colores (VGA) y
	  dibuja un trozo de imagen en la pantalla.
	  Entrada:      'nombre_pcx' nombre del fichero PCX
			'x', 'y' coordenadas de pantalla d¢nde se dibujar 
			el gr fico
			'anchura', 'altura' dimensiones (en pixels) de la
			zona del gr fico que se dibujar , si alguno de los
			valores es 0, se coger n las dimensiones dadas por
			la cabecera del fichero PCX
			'modo' modo de dibujo en pantalla de la imagen:
			PCX_PINTA va dibujando la imagen seg£n se lee del
			fichero, PCX_OCULTA dibuja la imagen pero no se
			visualiza hasta que est  completa, PCX_PALETA no
			modifica la paleta de colores
	  Salida:       0 si no hubo ning£n error o un valor distinto de 0
			si se produjo alg£n error
			Errores:
			  E_PCX_APER - error de apertura de fichero PCX
			  E_PCX_LECT - error de lectura de fichero PCX
			  E_PCX_FORM - formato de imagen incorrecto
			  E_PCX_MODO - modo de pantalla incorrecto
****************************************************************************/
int pcx_decodif256(char *nombre_pcx, short x, short y, int anchura,
  int altura, int modo)
{
FILE *fichpcx;
PCXHEADER pcx;
struct videoconfig vid;
BYTE color[256][3];
fpos_t fpos;
BYTE bpcx;
BYTE _far *pscan, _far *dirvideo, _far *pvideo;
int i, j, bytes_decodif, cuenta;

/* obtiene informaci¢n de modo de v¡deo */
_getvideoconfig(&vid);

/* mira si es una VGA en modo de 256 colores y resoluci¢n */
/* horizontal de 320 pixels */
if((vid.adapter!=_VGA) || (vid.numcolors!=256) || (vid.numxpixels!=320))
  return(E_PCX_MODO);

if((fichpcx=fopen(nombre_pcx,"rb"))==NULL) return(E_PCX_APER);

/* establece buffer para lectura de fichero */
setvbuf(fichpcx,NULL,_IOFBF,PCX_FICH_BUF);

/* lee la cabecera */
if(fread(&pcx,sizeof(PCXHEADER),1,fichpcx)!=1){
	fclose(fichpcx);
	return(E_PCX_LECT);
}

/* comprueba que sea una imagen con 8 bits por pixel */
/* y que contenga informaci¢n de paleta de 256 colores (versi¢n=5) */
if((pcx.bits_pixel!=8) || (pcx.version!=5)) {
	fclose(fichpcx);
	return(E_PCX_FORM);
}

/* guarda el puntero del fichero */
if(fgetpos(fichpcx,&fpos)) {
	fclose(fichpcx);
	return(E_PCX_LECT);
}

/* pone el puntero del fichero al comienzo de la informaci¢n de paleta */
if(fseek(fichpcx,-769L,SEEK_END)) {
	fclose(fichpcx);
	return(E_PCX_LECT);
}

/* lee 1er byte de informaci¢n de cabecera y comprueba que sea 12 */
if(fread(&bpcx,sizeof(BYTE),1,fichpcx)!=1) {
	fclose(fichpcx);
	return(E_PCX_LECT);
}
if(bpcx!=12) {
	fclose(fichpcx);
	return(E_PCX_FORM);
}

/* lee y convierte la informaci¢n de paleta */
for(i=0; i<vid.numcolors; i++) {
	/* componente rojo */
	if(fread(&color[i][0],sizeof(BYTE),1,fichpcx)!=1) {
		fclose(fichpcx);
		return(E_PCX_LECT);
	}
	/* componente verde */
	if(fread(&color[i][1],sizeof(BYTE),1,fichpcx)!=1) {
		fclose(fichpcx);
		return(E_PCX_LECT);
	}
	/* componente azul */
	if(fread(&color[i][2],sizeof(BYTE),1,fichpcx)!=1) {
		fclose(fichpcx);
		return(E_PCX_LECT);
	}
	/* dimensiona componentes */
	color[i][0]>>=2;
	color[i][1]>>=2;
	color[i][2]>>=2;
}

/* recupera el puntero del fichero */
if(fsetpos(fichpcx,&fpos)) {
	fclose(fichpcx);
	return(E_PCX_LECT);
}

/* si quiere dibujar la imagen mientras la lee, actualiza ahora la paleta */
if(modo==PCX_PINTA) carga_paleta(&color[0][0],256);

/* si quiere que la imagen no se vea, pone a 0 toda la paleta */
else if(modo==PCX_OCULTA) {
	for(i=0; i<vid.numcolors; i++) {
		_asm {
			mov ax,1010h    ; actualiza un registro del DAC
			mov bx,i        ; n£mero de registro
			mov cx,0        ; componente verde-azul
			mov dh,0        ; componente rojo
			int 10h
		}
	}
}

/* si la altura dada es 0, la calcula de acuerdo a la cabecera */
if(!altura) altura=pcx.ventana[3]-pcx.ventana[1]+1;
/* idem con la anchura */
if(!anchura) anchura=pcx.ventana[2]-pcx.ventana[0]+1;

/* si el gr fico a dibujar no cabe en la pantalla */
/* ajusta las dimensiones para que quepa */
if((y+altura)>vid.numypixels) altura=vid.numypixels-y;
if((x+anchura)>vid.numxpixels) anchura=vid.numxpixels-x;

/* inicializa puntero al inicio de primer pixel en buffer de v¡deo */
dirvideo=(BYTE _far *)0xa0000000+(x+(y*TAMLINPCX256));

/* bucle para dibujar la parte de la imagen PCX correspondiente */
for(j=0; j<altura; j++, dirvideo+=TAMLINPCX256) {
	/* inicializa puntero a posici¢n de buffer de l¡nea */
	/* d¢nde se colocar  l¡nea de scan */
	pscan=lin_scan256;
	/* inicializa contador de bytes decodificados */
	bytes_decodif=0;

	/* bucle para decodificar una l¡nea de scan */
	do {
		/* lee 1 byte del fichero PCX */
		if(fread(&bpcx,sizeof(BYTE),1,fichpcx)!=1) {
			fclose(fichpcx);
			return(E_PCX_LECT);
		}

		/* si los 2 bits altos est n a 1, recoge los 6 bits bajos */
		/* y lee el siguiente byte */
		if((bpcx & 0xc0)==0xc0) {
			cuenta=(int)(bpcx & 0x3f);
			if(fread(&bpcx,sizeof(BYTE),1,fichpcx)!=1) {
				fclose(fichpcx);
				return(E_PCX_LECT);
			}
		}
		else cuenta=1;

		/* expande byte */
		for(i=0; i<cuenta; i++) {
			*pscan++=bpcx;
			/* incrementa n£mero de bytes decodificados */
			bytes_decodif++;
			/* si se completa la l¡nea de scan, sale */
			if(bytes_decodif==pcx.bytes_scan) break;
		}
	} while(bytes_decodif<pcx.bytes_scan);

	/* imprime porci¢n correspondiente de la l¡nea en pantalla */
	/* inicializa puntero a buffer de l¡nea de scan */
	pscan=lin_scan256;

	/* dibuja el ancho correspondiente */
	for(i=0, pvideo=dirvideo; i<anchura; i++) *pvideo++=*pscan++;
}

fclose(fichpcx);

/* ahora actualiza la paleta para que se vea la imagen */
if(modo==PCX_OCULTA) carga_paleta(&color[0][0],vid.numcolors);

return(0);
}

