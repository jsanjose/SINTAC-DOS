/****************************************************************************
				     PCX.C

	Conjunto de funciones para visualizar ficheros gr ficos en
	formato PCX (s¢lo para VGA).

			    (c)1993 JSJ Soft Ltd.

	Las siguientes funciones son p£blicas:
		- pcx_decodifica: decodifica una imagen PCX
		- pcx_256: decodifica una imagen PCX de 256 colores; almacena
		    la imagen en memoria y la dibuja al final
		- pcx_256p: decodifica una imagen PCX de 256 colores; va
		    dibujando la imagen seg£n la decodifica
		- pcx_16: decodifica una imagen PCX de 16 colores; almacena
		    la imagen en memoria y la dibuja al final
		- pcx_16p: decodifica una imagen PCX de 16 colores; va
		    dibujando la imagen seg£n la decodifica

	Las siguientes estructuras est n definidas en PCX.H:
		STC_CABPCX: cabecera de un fichero PCX
		STC_IMAGEN: imagen decodificada
****************************************************************************/

#include <stdio.h>
#include <malloc.h>
#include <memory.h>
#include <graph.h>
#include "pcx.h"

/*** Variables globales internas ***/
static BYTE lin_scan16[TAMLINPCX16*4];
static BYTE lin_scan256[TAMLINPCX256];
static BYTE color[256][3];

/*** Prototipos de funciones internas ***/
static void carga_paleta(BYTE _far *nueva_paleta, int num_colores);
static void libera_mem_imagen(STC_IMAGEN *imagen);
static void dibuja_lin16(BYTE _far *lin, BYTE _far *dirvideo, int anchura,
  int bytes_plano);

/****************************************************************************
	CARGA_PALETA: carga la paleta con colores dados.
	  Entrada:      'nueva_paleta' puntero a una tabla con los nuevos
			colores; cada entrada de la tabla contiene los 3
			componentes de color (rojo, verde y azul en este
			orden)
			'num_colores' n£mero de colores en la paleta
****************************************************************************/
void carga_paleta(BYTE _far *nueva_paleta, int num_colores)
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
	LIBERA_MEM_IMAGEN: libera la memoria ocupada por una imagen.
	  Entrada:      'imagen' puntero a imagen.
****************************************************************************/
void libera_mem_imagen(STC_IMAGEN *imagen)
{
int i;

for(i=0; i<imagen->alto; i++) _ffree(imagen->lineas[i]);
free(imagen->lineas);
imagen->lineas=NULL;

}

/****************************************************************************
	DIBUJA_LIN16: dibuja una l¡nea compuesta de 4 planos, en el modo de
	  16 colores.
	  Entrada:      'lin' puntero a l¡nea (los 4 planos deben estar
			seguidos en orden)
			'dirvideo' puntero a memoria de video donde se
			transferir  la l¡nea
			'anchura' porci¢n de la l¡nea que se dibujar 
			'bytes_plano' bytes por plano de la l¡nea
****************************************************************************/
void dibuja_lin16(BYTE _far *lin, BYTE _far *dirvideo, int anchura,
  int bytes_plano)
{

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
	lds si,lin              ; DS:SI = direcci¢n buffer de l¡nea
planobit:
	out dx,ax               ; selecciona plano de bits
	push di
	push si
	mov cx,anchura          ; CX = n£mero de bytes a transferir
	rep movsb               ; transfiere datos a plano activo
	pop si
	pop di
	add si,bytes_plano      ; siguiente plano en buffer de l¡nea
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

/****************************************************************************
	PCX_DECODIFICA: decodifica una imagen PCX.
	  Entrada:      'nombre_pcx' nombre del fichero PCX
			'imagen' puntero a estructura para la imagen
	  Salida:       0 si no hubo ning£n error o un valor distinto de 0
			si se produjo alg£n error
			Errores:
			  E_PCX_APER - error de apertura de fichero PCX
			  E_PCX_LECT - error de lectura de fichero PCX
			  E_PCX_FORM - formato de imagen incorrecto
			  E_PCX_MEM  - no hay memoria suficiente
****************************************************************************/
int pcx_decodifica(char *nombre_pcx, STC_IMAGEN *imagen)
{
FILE *fichpcx;
fpos_t fpos;
STC_CABPCX pcx;
int i, j, bpcx, num_colores, ancho, alto, tam_linea, bytes_decodif, cuenta,
  totbytes;
BYTE _far *plin;

if((fichpcx=fopen(nombre_pcx,"rb"))==NULL) return(E_PCX_APER);

/* lee la cabecera */
if(fread(&pcx,sizeof(STC_CABPCX),1,fichpcx)!=1) {
	fclose(fichpcx);
	return(E_PCX_LECT);
}

/* comprueba que sea formato PCX */
if(pcx.propietario!=0x0a) {
	fclose(fichpcx);
	return(E_PCX_FORM);
}

/* n£mero de colores de la imagen */
num_colores=1 << (pcx.bits_pixel*pcx.num_planos);
imagen->num_colores=num_colores;

/* recoge informaci¢n de paleta */
if(num_colores<=16) {
	for(i=0; i<num_colores; i++) {
		imagen->paleta[i][0]=(BYTE)(pcx.paleta[i][0] >> 2);
		imagen->paleta[i][1]=(BYTE)(pcx.paleta[i][1] >> 2);
		imagen->paleta[i][2]=(BYTE)(pcx.paleta[i][2] >> 2);
	}
}
else {
	/* guarda el puntero del fichero */
	if(fgetpos(fichpcx,&fpos)) {
		fclose(fichpcx);
		return(E_PCX_LECT);
	}

	/* pone puntero del fichero al comienzo de informaci¢n de paleta */
	if(fseek(fichpcx,-769L,SEEK_END)) {
		fclose(fichpcx);
		return(E_PCX_LECT);
	}

	/* lee 1er byte de informaci¢n de cabecera y comprueba que sea 12 */
	if((bpcx=fgetc(fichpcx))==EOF) {
		fclose(fichpcx);
		return(E_PCX_LECT);
	}
	if(bpcx!=12) {
		fclose(fichpcx);
		return(E_PCX_FORM);
	}

	/* lee y convierte la informaci¢n de paleta */
	for(i=0; i<num_colores; i++) {
		/* componente rojo */
		if((bpcx=fgetc(fichpcx))==EOF) {
			fclose(fichpcx);
			return(E_PCX_LECT);
		}
		imagen->paleta[i][0]=(BYTE)(bpcx >> 2);
		/* componente verde */
		if((bpcx=fgetc(fichpcx))==EOF) {
			fclose(fichpcx);
			return(E_PCX_LECT);
		}
		imagen->paleta[i][1]=(BYTE)(bpcx >> 2);
		/* componente azul */
		if((bpcx=fgetc(fichpcx))==EOF) {
			fclose(fichpcx);
			return(E_PCX_LECT);
		}
		imagen->paleta[i][2]=(BYTE)(bpcx >> 2);
	}

	/* recupera el puntero del fichero */
	if(fsetpos(fichpcx,&fpos)) {
		fclose(fichpcx);
		return(E_PCX_LECT);
	}
}

/* tama¤o de la imagen */
ancho=pcx.ventana[2]-pcx.ventana[0]+1;
alto=pcx.ventana[3]-pcx.ventana[1]+1;
imagen->ancho=ancho;
imagen->alto=alto;

/* reserva memoria para tabla de l¡neas */
imagen->lineas=NULL;

if((imagen->lineas=(BYTE _far **)malloc(alto*sizeof(BYTE _far *)))==NULL)
  return(E_PCX_MEM);

/* tama¤o de una l¡nea (con todos sus planos) en bytes */
tam_linea=(ancho*pcx.bits_pixel*pcx.num_planos)/8;

/* reserva memoria para las l¡neas */
for(i=0; i<alto; i++) {
	if((imagen->lineas[i]=(BYTE _far *)_fmalloc(tam_linea))==NULL) {
		for(j=0; j<(i+1); j++) _ffree(imagen->lineas[i]);
		free(imagen->lineas);
		imagen->lineas=NULL;
		return(E_PCX_MEM);
	}
}

/* n£mero de bytes por l¡nea */
totbytes=pcx.bytes_scan*pcx.num_planos;

/* decodifica imagen */
for(i=0; i<alto; i++) {
	/* inicializa puntero a posici¢n de buffer de l¡nea */
	/* d¢nde se colocar  l¡nea de scan */
	plin=imagen->lineas[i];

	/* inicializa contador de bytes decodificados */
	bytes_decodif=0;

	/* bucle para decodificar una l¡nea de scan */
	do {
		/* lee 1 byte del fichero PCX */
		if((bpcx=fgetc(fichpcx))==EOF) {
			fclose(fichpcx);
			return(E_PCX_LECT);
		}

		/* si los 2 bits altos est n a 1, recoge los 6 bits bajos */
		/* y lee el siguiente byte */
		if((bpcx & 0xc0)==0xc0) {
			cuenta=bpcx & 0x3f;
			if((bpcx=fgetc(fichpcx))==EOF) {
				fclose(fichpcx);
				return(E_PCX_LECT);
			}
		}
		else cuenta=1;

		/* expande byte */
		for(j=0; j<cuenta; j++) {
			*plin++=(BYTE)bpcx;
			/* incrementa n£mero de bytes decodificados */
			bytes_decodif++;
			/* si se completa la l¡nea de scan, sale */
			if(bytes_decodif==totbytes) break;
		}
	} while(bytes_decodif<totbytes);
}

fclose(fichpcx);

return(0);
}

/****************************************************************************
	PCX_256: decodifica una imagen PCX de 256 colores y la dibuja al
	  final; si no hay memoria para la imagen se llama a una rutina
	  alternativa que la va dibujando seg£n la decodifica.
	  Entrada:      'nombre_pcx' nombre del fichero PCX
			'x', 'y' coordenadas de pantalla d¢nde se dibujar 
			el gr fico
			'anchura', 'altura' dimensiones (en pixels) de la
			zona del gr fico que se dibujar , si alguno de los
			valores es 0, se coger n las dimensiones dadas por
			la cabecera del fichero PCX
			'paleta' 0 si se debe coger la paleta de la imagen,
			1 si se debe respetar la paleta actual
	  Salida:       0 si no hubo ning£n error o un valor distinto de 0
			si se produjo alg£n error
			Errores:
			  E_PCX_APER - error de apertura de fichero PCX
			  E_PCX_LECT - error de lectura de fichero PCX
			  E_PCX_FORM - formato de imagen incorrecto
			  E_PCX_MODO - modo de pantalla incorrecto
****************************************************************************/
int pcx_256(char *nombre_pcx, short x, short y, int anchura, int altura,
  int paleta)
{
STC_IMAGEN imagpcx;
struct videoconfig vid;
register int i;
int ret;
BYTE _far *dirvideo, _far *plin;

ret=pcx_decodifica(nombre_pcx,&imagpcx);

/* si no hubo memoria para la imagen, usa rutina alternativa */
if(ret==E_PCX_MEM) {
	pcx_256p(nombre_pcx,x,y,anchura,altura,paleta);
	return(0);
}
else if(ret) return(ret);

_getvideoconfig(&vid);

/* mira si es una VGA en modo de 256 colores y resoluci¢n */
/* horizontal de 320 pixels */
if((vid.adapter!=_VGA) || (vid.numcolors!=256) || (vid.numxpixels!=320))
  return(E_PCX_MODO);

/* mira si la imagen es de 256 colores */
if(imagpcx.num_colores!=256) {
	libera_mem_imagen(&imagpcx);
	return(E_PCX_FORM);
}

if(paleta==0) carga_paleta(&imagpcx.paleta[0][0],256);

/* si alguna dimensi¢n es 0 o mayor que la imagen, coge la de la imagen */
if(!altura || (altura>imagpcx.alto)) altura=imagpcx.alto;
if(!anchura || (anchura>imagpcx.ancho)) anchura=imagpcx.ancho;

/* si el gr fico a dibujar no cabe en la pantalla */
/* ajusta las dimensiones para que quepa */
if((y+altura)>vid.numypixels) altura=vid.numypixels-y;
if((x+anchura)>vid.numxpixels) anchura=vid.numxpixels-x;

/* puntero al primer pixel a dibujar en buffer de v¡deo */
dirvideo=(BYTE _far *)0xa0000000+(x+(y*TAMLINPCX256));

/* dibuja imagen en pantalla */
for(i=0; i<altura; i++) {
	plin=imagpcx.lineas[i];
	_fmemcpy(dirvideo,plin,anchura);
	dirvideo+=TAMLINPCX256;
}

libera_mem_imagen(&imagpcx);

return(0);
}

/****************************************************************************
	PCX_256P: decodifica una imagen PCX de 256 colores y la va dibujando
	  en pantalla.
	  Entrada:      'nombre_pcx' nombre del fichero PCX
			'x', 'y' coordenadas de pantalla d¢nde se dibujar 
			el gr fico
			'anchura', 'altura' dimensiones (en pixels) de la
			zona del gr fico que se dibujar , si alguno de los
			valores es 0, se coger n las dimensiones dadas por
			la cabecera del fichero PCX
			'paleta' 0 si se debe coger la paleta de la imagen,
			1 si se debe respetar la paleta actual
	  Salida:       0 si no hubo ning£n error o un valor distinto de 0
			si se produjo alg£n error
			Errores:
			  E_PCX_APER - error de apertura de fichero PCX
			  E_PCX_LECT - error de lectura de fichero PCX
			  E_PCX_FORM - formato de imagen incorrecto
			  E_PCX_MODO - modo de pantalla incorrecto
****************************************************************************/
int pcx_256p(char *nombre_pcx, short x, short y, int anchura, int altura,
  int paleta)
{
FILE *fichpcx;
STC_CABPCX pcx;
struct videoconfig vid;
fpos_t fpos;
BYTE _far *plin, _far *dirvideo;
register int i, j;
int img_ancho, img_alto, bytes_decodif, cuenta, bpcx, totbytes;

/* obtiene informaci¢n de modo de v¡deo */
_getvideoconfig(&vid);

/* mira si es una VGA en modo de 256 colores y resoluci¢n */
/* horizontal de 320 pixels */
if((vid.adapter!=_VGA) || (vid.numcolors!=256) || (vid.numxpixels!=320))
  return(E_PCX_MODO);

if((fichpcx=fopen(nombre_pcx,"rb"))==NULL) return(E_PCX_APER);

/* lee la cabecera */
if(fread(&pcx,sizeof(STC_CABPCX),1,fichpcx)!=1) {
	fclose(fichpcx);
	return(E_PCX_LECT);
}

/* comprueba que sea una imagen con 8 bits por pixel */
if(pcx.bits_pixel!=8) {
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
if((bpcx=fgetc(fichpcx))==EOF) {
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
	if((bpcx=fgetc(fichpcx))==EOF) {
		fclose(fichpcx);
		return(E_PCX_LECT);
	}
	color[i][0]=(BYTE)(bpcx >> 2);
	/* componente verde */
	if((bpcx=fgetc(fichpcx))==EOF) {
		fclose(fichpcx);
		return(E_PCX_LECT);
	}
	color[i][1]=(BYTE)(bpcx >> 2);
	/* componente azul */
	if((bpcx=fgetc(fichpcx))==EOF) {
		fclose(fichpcx);
		return(E_PCX_LECT);
	}
	color[i][2]=(BYTE)(bpcx >> 2);
}

/* recupera el puntero del fichero */
if(fsetpos(fichpcx,&fpos)) {
	fclose(fichpcx);
	return(E_PCX_LECT);
}

if(paleta==0) carga_paleta(&color[0][0],256);

/* si alguna dimensi¢n es 0 o mayor que la imagen, coge la de la imagen */
img_alto=pcx.ventana[3]-pcx.ventana[1]+1;
img_ancho=pcx.ventana[2]-pcx.ventana[0]+1;
if(!altura || (altura>img_alto)) altura=img_alto;
if(!anchura || (anchura>img_ancho)) anchura=img_ancho;

/* si el gr fico a dibujar no cabe en la pantalla */
/* ajusta las dimensiones para que quepa */
if((y+altura)>vid.numypixels) altura=vid.numypixels-y;
if((x+anchura)>vid.numxpixels) anchura=vid.numxpixels-x;

/* n£mero de bytes por l¡nea */
totbytes=pcx.bytes_scan*pcx.num_planos;

/* puntero al primer pixel a dibujar en buffer de v¡deo */
dirvideo=(BYTE _far *)0xa0000000+(x+(y*TAMLINPCX256));

/* bucle para decodificar/dibujar la imagen */
for(i=0; i<altura; i++) {
	/* inicializa puntero a posici¢n de buffer de l¡nea */
	/* d¢nde se colocar  l¡nea de scan */
	plin=lin_scan256;

	/* inicializa contador de bytes decodificados */
	bytes_decodif=0;

	/* bucle para decodificar una l¡nea de scan */
	do {
		/* lee 1 byte del fichero PCX */
		if((bpcx=fgetc(fichpcx))==EOF) {
			fclose(fichpcx);
			return(E_PCX_LECT);
		}

		/* si los 2 bits altos est n a 1, recoge los 6 bits bajos */
		/* y lee el siguiente byte */
		if((bpcx & 0xc0)==0xc0) {
			cuenta=bpcx & 0x3f;
			if((bpcx=fgetc(fichpcx))==EOF) {
				fclose(fichpcx);
				return(E_PCX_LECT);
			}
		}
		else cuenta=1;

		/* expande byte */
		for(j=0; j<cuenta; j++) {
			*plin++=(BYTE)bpcx;
			/* incrementa n£mero de bytes decodificados */
			bytes_decodif++;
			/* si se completa la l¡nea de scan, sale */
			if(bytes_decodif==totbytes) break;
		}
	} while(bytes_decodif<totbytes);

	/* dibuja l¡nea en pantalla */
	_fmemcpy(dirvideo,lin_scan256,anchura);
	dirvideo+=TAMLINPCX256;
}

fclose(fichpcx);

return(0);
}

/****************************************************************************
	PCX_16: decodifica una imagen PCX de 16 colores y la dibuja al
	  final; si no hay memoria para almacenar la imagen se llama a
	  una rutina alternativa que la visualiza seg£n la decodifica.
	  Entrada:      'nombre_pcx' nombre del fichero PCX
			'x', 'y' coordenadas de pantalla d¢nde se dibujar 
			el gr fico; la coordenada X queda redondeada a un
			valor m£ltiplo de 8
			'anchura', 'altura' dimensiones (en pixels) de la
			zona del gr fico que se dibujar , si alguno de los
			valores es 0, se coger n las dimensiones dadas por
			la cabecera del fichero PCX; el ancho se redondea
			a un valor m£ltiplo de 8
			'paleta' 0 si se debe coger la paleta de la imagen,
			1 si se debe respetar la paleta actual
	  Salida:       0 si no hubo ning£n error o un valor distinto de 0
			si se produjo alg£n error
			Errores:
			  E_PCX_APER - error de apertura de fichero PCX
			  E_PCX_LECT - error de lectura de fichero PCX
			  E_PCX_FORM - formato de imagen incorrecto
			  E_PCX_MODO - modo de pantalla incorrecto
****************************************************************************/
int pcx_16(char *nombre_pcx, short x, short y, int anchura, int altura,
  int paleta)
{
STC_IMAGEN imagpcx;
struct videoconfig vid;
register int i;
int ret, bytes_plano;
BYTE _far *dirvideo;

ret=pcx_decodifica(nombre_pcx,&imagpcx);

/* si no hubo memoria para la imagen, usa rutina alternativa */
if(ret==E_PCX_MEM) {
	pcx_16p(nombre_pcx,x,y,anchura,altura,paleta);
	return(0);
}
else if(ret) return(ret);

_getvideoconfig(&vid);

/* mira si es una VGA en modo de 16 colores y resoluci¢n */
/* horizontal de 640 pixels */
if((vid.adapter!=_VGA) || (vid.numcolors!=16) || (vid.numxpixels!=640))
  return(E_PCX_MODO);

/* mira si la imagen es de 16 colores */
if(imagpcx.num_colores!=16) {
	libera_mem_imagen(&imagpcx);
	return(E_PCX_FORM);
}

if(paleta==0) carga_paleta(&imagpcx.paleta[0][0],16);

/* si alguna dimensi¢n es 0 o mayor que la imagen, coge la de la imagen */
if(!altura || (altura>imagpcx.alto)) altura=imagpcx.alto;
if(!anchura || (anchura>imagpcx.ancho)) anchura=imagpcx.ancho;

/* si el gr fico a dibujar no cabe en la pantalla */
/* ajusta las dimensiones para que quepa */
if((y+altura)>vid.numypixels) altura=vid.numypixels-y;
if((x+anchura)>vid.numxpixels) anchura=vid.numxpixels-x;

/* calcula la anchura en bytes */
anchura/=8;

/* puntero al primer pixel a dibujar en buffer de v¡deo */
dirvideo=(BYTE _far *)0xa0000000+((x/8)+(y*TAMLINPCX16));

/* calcula n§ de bytes por plano */
bytes_plano=imagpcx.ancho/8;

/* dibuja imagen en pantalla */
for(i=0; i<altura; i++) {
	dibuja_lin16(imagpcx.lineas[i],dirvideo,anchura,bytes_plano);
	dirvideo+=TAMLINPCX16;
}

libera_mem_imagen(&imagpcx);

return(0);
}

/****************************************************************************
	PCX_16P: decodifica una imagen PCX de 16 colores; va dibujando la
	  imagen seg£n la decodifica.
	  Entrada:      'nombre_pcx' nombre del fichero PCX
			'x', 'y' coordenadas de pantalla d¢nde se dibujar 
			el gr fico; la coordenada X queda redondeada a un
			valor m£ltiplo de 8
			'anchura', 'altura' dimensiones (en pixels) de la
			zona del gr fico que se dibujar , si alguno de los
			valores es 0, se coger n las dimensiones dadas por
			la cabecera del fichero PCX; el ancho se redondea
			a un valor m£ltiplo de 8
			'paleta' 0 si se debe coger la paleta de la imagen,
			1 si se debe respetar la paleta actual
	  Salida:       0 si no hubo ning£n error o un valor distinto de 0
			si se produjo alg£n error
			Errores:
			  E_PCX_APER - error de apertura de fichero PCX
			  E_PCX_LECT - error de lectura de fichero PCX
			  E_PCX_FORM - formato de imagen incorrecto
			  E_PCX_MODO - modo de pantalla incorrecto
****************************************************************************/
int pcx_16p(char *nombre_pcx, short x, short y, int anchura, int altura,
  int paleta)
{
FILE *fichpcx;
STC_CABPCX pcx;
struct videoconfig vid;
BYTE _far *plin, _far *dirvideo;
register int i, j;
int img_ancho, img_alto, bytes_decodif, cuenta, totbytes, bpcx, bytes_plano;

/* obtiene informaci¢n de modo de v¡deo */
_getvideoconfig(&vid);

/* mira si es una VGA en modo de 16 colores y resoluci¢n */
/* horizontal de 640 pixels */
if((vid.adapter!=_VGA) || (vid.numcolors!=16) || (vid.numxpixels!=640))
  return(E_PCX_MODO);

if((fichpcx=fopen(nombre_pcx,"rb"))==NULL) return(E_PCX_APER);

/* lee la cabecera */
if(fread(&pcx,sizeof(STC_CABPCX),1,fichpcx)!=1) {
	fclose(fichpcx);
	return(E_PCX_LECT);
}

/* comprueba que sea formato PCX */
if(pcx.propietario!=0x0a) {
	fclose(fichpcx);
	return(E_PCX_FORM);
}

/* comprueba que sea una imagen con 4 planos de bits */
if(pcx.num_planos!=4) {
	fclose(fichpcx);
	return(E_PCX_FORM);
}

/* coge y convierte la informaci¢n de paleta */
for(i=0; i<vid.numcolors; i++) {
	pcx.paleta[i][0]>>=2;
	pcx.paleta[i][1]>>=2;
	pcx.paleta[i][2]>>=2;
}

if(paleta==0) carga_paleta(&pcx.paleta[0][0],16);

/* si alguna dimensi¢n es 0 o mayor que la imagen, coge la de la imagen */
img_alto=pcx.ventana[3]-pcx.ventana[1]+1;
img_ancho=pcx.ventana[2]-pcx.ventana[0]+1;
if(!altura || (altura>img_alto)) altura=img_alto;
if(!anchura || (anchura>img_ancho)) anchura=img_ancho;

/* si el gr fico a dibujar no cabe en la pantalla */
/* ajusta las dimensiones para que quepa */
if((y+altura)>vid.numypixels) altura=vid.numypixels-y;
if((x+anchura)>vid.numxpixels) anchura=vid.numxpixels-x;

/* calcula la anchura en bytes */
anchura/=8;

/* n£mero de bytes por l¡nea */
totbytes=pcx.bytes_scan*pcx.num_planos;

/* direcci¢n en buffer de v¡deo de primer pixel a dibujar */
dirvideo=(BYTE _far *)0xa0000000+((x/8)+(y*TAMLINPCX16));

bytes_plano=pcx.bytes_scan;

/* bucle para dibujar la parte de la imagen PCX correspondiente */
for(i=0; i<altura; i++) {
	/* inicializa puntero a buffer de l¡nea */
	plin=lin_scan16;

	/* inicializa contador de bytes decodificados */
	bytes_decodif=0;

	/* bucle para decodificar 4 planos de bits de una l¡nea */
	do {
		/* lee 1 byte del fichero PCX */
		if((bpcx=fgetc(fichpcx))==EOF) {
			fclose(fichpcx);
			return(E_PCX_LECT);
		}

		/* si los 2 bits altos est n a 1, recoge los 6 bits bajos */
		/* y lee el siguiente byte */
		if((bpcx & 0xc0)==0xc0) {
			cuenta=bpcx & 0x3f;
			if((bpcx=fgetc(fichpcx))==EOF) {
				fclose(fichpcx);
				return(E_PCX_LECT);
			}
		}
		else cuenta=1;

		/* expande byte */
		for(j=0; j<cuenta; j++) {
			*plin++=(BYTE)bpcx;
			/* incrementa n£mero de bytes decodificados */
			bytes_decodif++;
			/* si se completa la l¡nea de scan, sale */
			if(bytes_decodif==totbytes) break;
		}
	} while(bytes_decodif<totbytes);

	/* dibuja la l¡nea */
	dibuja_lin16(lin_scan16,dirvideo,anchura,bytes_plano);
	dirvideo+=TAMLINPCX16;
}

fclose(fichpcx);

return(0);
}
