/****************************************************************************
		       GENERADOR DE CARACTERES SINTAC
			   (c)1992-93 JSJ Soft Ltd.
****************************************************************************/

#include <stdio.h>
#include <process.h>
#include <stdlib.h>
#include <graph.h>
#include <dos.h>
#include <io.h>
#include "boton.h"
#include "puntero.h"
#include "..\sintac.h"
#include "gcs.h"

/*** Variables globales ***/
/* tabla d¢nde se almacenar n las definiciones de los caracteres 8x14 */
BYTE tabla_ascii[256][ALTURA_CUAD];

BYTE caracter[ALTURA_CUAD];             /* definici¢n de car cter actual */
int ascii=0;                            /* c¢digo ASCII de car cter actual */

STC_CODBOTON pulsado;                   /* para recoger bot¢n pulsado */

/* configuraci¢n del sistema de v¡deo */
struct videoconfig vc;

/*** Programa principal ***/
void main(void)
{
int i;

/* ajusta modo de v¡deo */
if(!_setvideomode(_ERESCOLOR)) {
	printf("\nEste programa requiere tarjeta EGA o VGA.\n");
	exit(1);
}
/* intenta cargar fuente Helv‚tica usada por las funciones de BOTON.C */
if(_registerfonts("HELVB.FON")<0) {
	_setvideomode(_DEFAULTMODE);
	printf("\nFichero HELVB.FON no encontrado o no v lido.\n");
	exit(1);
}
/* intenta inicializar el puntero */
if(!inic_puntero()) {
	_setvideomode(_DEFAULTMODE);
	printf("\nRat¢n no disponible.\n");
	exit(1);
}

/* recoge informaci¢n sobre sistema de v¡deo */
_getvideoconfig(&vc);

/* instala 'handler' de errores cr¡ticos */
_harderr(int24_hnd);

/* esconde puntero mientras dibuja paneles */
vis_puntero(ESCONDE);

/* inicializa y dibuja barras de botones */
for(i=0; i<NUM_BARR; i++) {
	crea_barra_boton(&barra[i]);
	dibuja_barra_boton(&barra[i]);
}

/* encabezamiento */
escribe_texto_boton("GENERADOR DE CARACTERES SINTAC versi¢n 1.0",
  &barra[0].bot[1],barra[0].color_pplano,barra[0].color_s1);

/* cuadricula zona de dibujo */
dibuja_cuadricula(barra[0].bot[3].xb+3,barra[0].bot[3].yb+3,ALTURA_CUAD);

/* copia definiciones de tabla ROM de caracteres 8x14 en RAM */
copia_tabla_rom(TAB_ROM8x14,&tabla_ascii[0][0]);

/* direcciona tabla de caracteres en RAM */
_asm {
	push bp
	mov ax,1121h
	mov bl,2                ; 25 filas de caracteres por pantalla (8x14)
	mov cx,ALTURA_CUAD      ; bytes por car cter
	mov dx,SEG tabla_ascii
	mov es,dx
	mov bp,OFFSET tabla_ascii
	int 10h
	pop bp
}

/* tabla ASCII */
dibuja_tabla_ascii(FILA_TABLA,COLUMNA_TABLA);

/* dibuja marcador en el primer car cter de la tabla */
dibuja_marcador_ascii(0);

/* borra la definici¢n del car cter y lo dibuja en pantalla */
limpia_caracter(caracter,ALTURA_CUAD,0);
_setcolor(0);
_rectangle(_GFILLINTERIOR,barra[0].bot[5].xb+3,barra[0].bot[5].yb+3,
  barra[0].bot[5].xb+barra[0].bot[5].ancho-3,
  barra[0].bot[5].yb+barra[0].bot[5].alto-3);
dibuja_caracter(barra[0].bot[5].xb+7,barra[0].bot[5].yb+7,COLOR_L_ASCII,
  caracter);

/* muestra puntero del rat¢n */
vis_puntero(MUESTRA);

do {
	do {
		pulsado=esta_pulsado(&barra[0],NUM_BARR);
	} while(pulsado.boton==99);

	switch(pulsado.barra) {
		case 0 :
			switch(pulsado.boton) {
				case 3 :        /* cuadr¡cula de dibujo */
					cuadricula();
					break;
				case 4 :        /* tabla ASCII */
					tab_ascii();
					break;
				case 5 :        /* definici¢n de car cter */
					def_caracter();
					break;
			}
			break;
		case 1 :
			switch(pulsado.boton) {
				case 0 :        /* Cargar fuente */
					cargar_fuente();
					break;
				case 1 :        /* Grabar fuente */
					grabar_fuente();
					break;
			}
			break;
		case 2 :
			switch(pulsado.boton) {
				case 0 :        /* Almacena definici¢n */
					almacena_definicion();
					break;
				case 1 :        /* Borra rejilla */
					borra_rejilla();
					break;
				case 2 :        /* Llena rejilla */
					llena_rejilla();
					break;
				case 3 :        /* Arriba */
					scroll_def_arr();
					break;
				case 4 :        /* Abajo */
					scroll_def_abj();
					break;
				case 5 :        /* Izquierda */
					scroll_def_izq();
					break;
				case 6 :        /* Derecha */
					scroll_def_der();
					break;
			}
			break;
	}
} while((pulsado.barra!=0) || (pulsado.boton!=2));

_unregisterfonts();
_setvideomode(_DEFAULTMODE);

}

/****************************************************************************
	INT24_HND: rutina de manejo de errores cr¡ticos de hardware.
****************************************************************************/
void _far int24_hnd(unsigned deverror, unsigned errcode,
  unsigned _far *devhdr)
{

_hardresume(_HARDERR_FAIL);

}

/****************************************************************************
	DIBUJA_CUADRICULA: cuadricula la zona de dibujo.
	  Entrada:      'x', 'y' origen de la zona de dibujo
			'alt' altura de la cuadr¡cula
	  NOTA: las dimensiones de cada cuadro son CUAD_ANCHO y CUAD_ALTO
****************************************************************************/
void dibuja_cuadricula(short x, short y, int alt)
{
short x0, y0;
int i, j;

/* color de fondo de la zona de dibujo */
_setcolor(COLOR_F_CUAD);
_rectangle(_GFILLINTERIOR,x,y,x+(CUAD_ANCHO*8)-1,y+(CUAD_ALTO*alt)-1);

/* cuadricula */
y0=y;
/* color de borde de cuadr¡cula */
_setcolor(COLOR_B_CUAD);
for(i=0; i<alt; i++, y0+=CUAD_ALTO) {
	x0=x;
	for(j=0; j<8; j++, x0+=CUAD_ANCHO)
	  _rectangle(_GBORDER,x0,y0,x0+(CUAD_ANCHO-1),y0+(CUAD_ALTO-1));
}

}

/****************************************************************************
	CAMBIA_CUADRICULA: invierte el estado de un cuadro de la cuadr¡cula.
	  Entrada:      'x', 'y' coordenadas del cuadro a modificar ([0..7] y
			[0..(alt-1)])
			'orgx', 'orgy' origen de la cuadr¡cula
			'alt' altura de la cuadr¡cula
****************************************************************************/
void cambia_cuadricula(short x, short y, short orgx, short orgy, int alt)
{
short cuadx, cuady;

/* oculta el puntero durante el cambio de cuadro */
vis_puntero(ESCONDE);

/* comprueba que no se salga de los l¡mites */
x=(x>7) ? 7 : x;
y=(y>alt-1) ? alt-1 : y;

/* calcula origen del cuadro */
cuadx=orgx+x*CUAD_ANCHO;
cuady=orgy+y*CUAD_ALTO;

/* si cuadro est  vac¡o lo llena, si no lo vac¡a */
if(_getpixel(cuadx+1,cuady+1)==COLOR_F_CUAD) {
	_setcolor(COLOR_R_CUAD);
	_rectangle(_GFILLINTERIOR,cuadx+1,cuady+1,cuadx+(CUAD_ANCHO-2),
	  cuady+(CUAD_ALTO-2));
}
else {
	_setcolor(COLOR_F_CUAD);
	_rectangle(_GFILLINTERIOR,cuadx+1,cuady+1,cuadx+(CUAD_ANCHO-2),
	  cuady+(CUAD_ALTO-2));
}

vis_puntero(MUESTRA);

}

/****************************************************************************
	DIBUJA_TABLA_ASCII: dibuja la tabla de caracteres ASCII en forma
	  de matriz de 16x16.
	  Entrada:      'fila', 'columna' posici¢n de inicio de la tabla
****************************************************************************/
void dibuja_tabla_ascii(short fila, short columna)
{
char car[2];
short fil;
int i, j;

/* esconde puntero durante el dibujado de tabla */
vis_puntero(ESCONDE);

car[0]=0;
car[1]=' ';

_settextcolor(COLOR_L_ASCII);

fil=fila;
for(i=0; i<16; i++) {
	_settextposition(fil,columna);
	/* imprime espacio al inicio de cada l¡nea */
	_outmem(&car[1],1);
	for(j=0; j<16; j++, car[0]++) _outmem(car,2);
	fil++;
}

vis_puntero(MUESTRA);

}

/****************************************************************************
	CAMBIA_CARACTER: invierte el estado de un bit de la definici¢n
	  de un car cter.
	  Entrada:      'car' puntero a definici¢n del car cter
			'x', 'y' coordenadas del bit a cambiar
****************************************************************************/
void cambia_caracter(BYTE *car, short x, short y)
{
BYTE mascara=0x80;

mascara>>=x;
car[y]^=mascara;

}

/****************************************************************************
	LIMPIA_CARACTER: rellena la definici¢n de un car cter con un BYTE
	  dado.
	  Entrada:      'car' puntero a definici¢n del car cter
			'alt' altura del car cter
			'byte' byte de relleno
****************************************************************************/
void limpia_caracter(BYTE *car, int alt, BYTE byte)
{
int i;

for(i=0; i<alt; car[i++]=byte);

}

/****************************************************************************
	PULSA_EN_ASCII: devuelve el c¢digo ASCII que corresponde a un
	  car cter situado en la tabla ASCII 16x16 dibujada por la funci¢n
	  dibuja_tabla_ascii().
	  Entrada:      'x', 'y' posici¢n relativa al origen de la tabla
	  Salida:       c¢digo ASCII del car cter cuya posici¢n coincide
			con las coordenadas 'x', 'y'
****************************************************************************/
int pulsa_en_ascii(short x, short y)
{
short x0, y0;

/* transforma en valor 0..15 */
x0=x/16;
y0=y/ALTURA_CUAD;

/* comprueba que no est‚ fuera de los l¡mites */
x0=(x0>15) ? 15 : x0;
y0=(y0>15) ? 15 : y0;

return(y0*16+x0);
}

/****************************************************************************
	DIBUJA_MARCADOR_ASCII: dibuja un marcador alrededor de un car cter
	  de la tabla ASCII.
	  Entrada:      'ascii' c¢digo ASCII del car cter
****************************************************************************/
void dibuja_marcador_ascii(int ascii)
{
short modo;
short orgx, orgy, x, y;

/* origen de la tabla */
orgx=(COLUMNA_TABLA-1)*8;
orgy=(FILA_TABLA-1)*ALTURA_CUAD;

/* coordenadas relativas de un car cter dentro de la tabla 16x16 en */
/* funci¢n de su c¢digo ASCII */
x=(((ascii%16)*2)+1)*8;
y=(ascii/16)*ALTURA_CUAD;

/* desactiva puntero mientras dibuja */
vis_puntero(ESCONDE);

/* dibuja un rect ngulo alrededor del car cter seleccionado */
modo=_setwritemode(_GXOR);
_setcolor(COLOR_R_ASCII);
_rectangle(_GBORDER,orgx+x-1,orgy+y-1,orgx+x+8,orgy+y+ALTURA_CUAD);

/* restaura modo de dibujo */
_setwritemode(modo);

vis_puntero(MUESTRA);

}

/****************************************************************************
	COPIA_TABLA_ROM: copia las definiciones de una tabla de caracteres
	  de la ROM en memoria RAM.
	  Entrada:      'tabla' n£mero de la tabla de la ROM
			'tabla_car' puntero a tabla d¢nde se copiar n las
			definiciones de los caracteres
****************************************************************************/
void copia_tabla_rom(BYTE tabla, BYTE _far *tabla_car)
{
BYTE _far *dir_tabla_rom;
int i;

_asm {
	push bp
	mov ah,11h              ; interfaz del generador de caracteres
	mov al,30h              ; obtener informaci¢n del generador
	mov bh,tabla
	int 10h                 ; ES:BP=direcci¢n de tabla en ROM
	mov ax,bp
	pop bp
	mov WORD PTR (dir_tabla_rom+2),es
	mov WORD PTR dir_tabla_rom,ax
}

for(i=0; i<256*ALTURA_CUAD; i++) tabla_car[i]=*dir_tabla_rom++;

}

/****************************************************************************
	COPIA_DEF_CAR: copia la definici¢n de un car cter de un buffer
	  a otro.
	  Entrada:      'origen' puntero a la definici¢n de car cter a
			copiar
			'destino' puntero a d¢nde copiarla
****************************************************************************/
void copia_def_car(BYTE _far *origen, BYTE _far *destino)
{
int i;

for(i=0; i<ALTURA_CUAD; i++) destino[i]=origen[i];

}

/****************************************************************************
	DIBUJA_EN_CUADRICULA: dibuja la definici¢n correspondiente a un
	  car cter en la cuadr¡cula de dibujo.
	  Entrada:      'orgx', 'orgy' origen de la cuadr¡cula
			'def_car' puntero a definici¢n de car cter
****************************************************************************/
void dibuja_en_cuadricula(short orgx, short orgy, BYTE *def_car)
{
int i, j;
BYTE c, mascara;
short x, y;

/* esconde puntero mientras dibuja */
vis_puntero(ESCONDE);

y=orgy;
for(i=0; i<ALTURA_CUAD; i++, y+=CUAD_ALTO) {
	c=def_car[i];
	mascara=0x80;
	x=orgx;
	/* dibuja bit por bit */
	for(j=0; j<8; j++, mascara>>=1, x+=CUAD_ANCHO) {
		/* si el bit est  activo dibuja de color de relleno */
		/* si no de color de fondo */
		if(mascara & c) _setcolor(COLOR_R_CUAD);
		else _setcolor(COLOR_F_CUAD);
		_rectangle(_GFILLINTERIOR,x+1,y+1,x+(CUAD_ANCHO-2),
		  y+(CUAD_ALTO-2));
	}
}

vis_puntero(MUESTRA);

}

/****************************************************************************
	ACTUALIZA_TABLA: dibuja una definici¢n de car cter en su posici¢n
	  en la tabla ASCII de pantalla.
	  Entrada:      'fila', 'columna' origen de la tabla
			'ascii' c¢digo ASCII del car cter
****************************************************************************/
void actualiza_tabla(short fila, short columna, int ascii)
{
short filcar, colcar;
char car;

/* oculta puntero mientras actualiza tabla ASCII */
vis_puntero(ESCONDE);

/* calcula posici¢n del car cter de acuerdo a su c¢digo ASCII */
/* teniendo en cuenta que hay un espacio entre cada columna de la tabla */
filcar=ascii/16;
colcar=((ascii%16)*2)+1;

/* imprime car cter en su posici¢n dentro de la tabla */
car=(char)ascii;
_settextposition(fila+filcar,columna+colcar);
_outmem(&car,1);

vis_puntero(MUESTRA);

}

/****************************************************************************
	DIBUJA_CARACTER: dibuja la definici¢n de un car cter a tama¤o real.
	  Entrada:      'x', 'y' posici¢n d¢nde se dibujar  la definici¢n
			del car cter
			'color' color con el que se dibujar 
			'caracter' puntero a la definici¢n del car cter
****************************************************************************/
void dibuja_caracter(short x, short y, short color, BYTE *caracter)
{
short x0;
BYTE *c, mascara;
int i, j;

/* oculta el puntero mientras dibuja car cter */
vis_puntero(ESCONDE);

c=caracter;

/* dibuja car cter */
for(i=0; i<ALTURA_CUAD; i++, c++, y++) {
	x0=x;
	mascara=0x80;
	for(j=0; j<8; j++, mascara>>=1, x0++) {
		/* selecciona color de acuerdo a si bit est  a 0 o 1 */
		if(*c & mascara) _setcolor(color);
		else _setcolor(0);
		/* dibuja bit */
		_setpixel(x0,y);
	}
}

vis_puntero(MUESTRA);

}

/****************************************************************************
	GRABA_DEF: graba en un fichero las definiciones de los caracteres.
	  Entrada:      'nombre' nombre con que se grabar  la fuente
			'fuente' puntero a tabla de definiciones de
			caracteres
	  Salida:       0 si hubo error o un valor distinto de 0 en
			otro caso
****************************************************************************/
int graba_def(char *nombre, BYTE *fuente)
{
FILE *ffuente;

/* abre el fichero para escritura en modo binario */
ffuente=fopen(nombre,"wb");
/* sale si hubo error */
if(ffuente==NULL) return(0);

/* escribe cadena de reconocimiento */
if(fwrite(RECON_FUENTE,sizeof(char),LONG_RECON_F+1,ffuente)<LONG_RECON_F+1) {
	fclose(ffuente);
	return(0);
}

/* escribe definiciones de caracteres */
if(fwrite(fuente,sizeof(BYTE),256*ALTURA_CUAD,ffuente)<256*ALTURA_CUAD) {
	fclose(ffuente);
	return(0);
}

fclose(ffuente);

return(1);
}

/****************************************************************************
	CARGA_DEF: carga de un fichero las definiciones de los
	  caracteres.
	  Entrada:      'nombre' nombre del fichero
			'fuente' puntero a tabla de definiciones de
			caracteres
	  Salida:       0 si hubo error o un valor distinto de 0 en
			otro caso
****************************************************************************/
int carga_def(char *nombre, BYTE *fuente)
{
FILE *ffuente;
char cad_recon[LONG_RECON_F+1];
char *recon_fuente=RECON_FUENTE;

/* abre el fichero para lectura */
ffuente=fopen(nombre,"rb");
/* sale si hubo error */
if(ffuente==NULL) return(0);

/* lee cadena de reconocimiento */
if(fread(cad_recon,sizeof(char),LONG_RECON_F+1,ffuente)<LONG_RECON_F+1) {
	fclose(ffuente);
	return(0);
}

/* comprueba la versi¢n del fichero */
if(cad_recon[LONG_RECON_F-1]!=recon_fuente[LONG_RECON_F-1]) {
	fclose(ffuente);
	return(0);
}

/* si la versi¢n ha sido v lida lee las definiciones de los caracteres */
if(fread(fuente,sizeof(BYTE),256*ALTURA_CUAD,ffuente)<256*ALTURA_CUAD) {
	fclose(ffuente);
	return(0);
}

fclose(ffuente);

return(1);
}

/****************************************************************************
	CUADRICULA: rutina de manejo de la cuadr¡cula de dibujo.
****************************************************************************/
void cuadricula(void)
{
STC_PUNTERO puntero;

/* modifica cuadr¡cula */
cambia_cuadricula(pulsado.x/CUAD_ANCHO,pulsado.y/CUAD_ALTO,
  barra[0].bot[3].xb+3,barra[0].bot[3].yb+3,ALTURA_CUAD);

/* modifica bit del car cter */
cambia_caracter(caracter,pulsado.x/CUAD_ANCHO,pulsado.y/CUAD_ALTO);

/* dibuja car cter */
dibuja_caracter(barra[0].bot[5].xb+7,barra[0].bot[5].yb+7,COLOR_L_ASCII,
  caracter);

/* espera a que se suelte el bot¢n */
do {
	coge_pos_puntero(&puntero);
} while(puntero.botones & PULSADO_IZQ);

}

/****************************************************************************
	TAB_ASCII: rutina de manejo de la tabla ASCII.
****************************************************************************/
void tab_ascii(void)
{

/* borra marcador actual */
dibuja_marcador_ascii(ascii);

/* coge c¢digo ASCII */
ascii=pulsa_en_ascii(pulsado.x,pulsado.y);

/* dibuja marcador */
dibuja_marcador_ascii(ascii);

/* copia definici¢n de caracter en buffer auxiliar */
copia_def_car(tabla_ascii[ascii],caracter);

/* dibuja en cuadr¡cula */
dibuja_en_cuadricula(barra[0].bot[3].xb+3,barra[0].bot[3].yb+3,caracter);
dibuja_caracter(barra[0].bot[5].xb+7,barra[0].bot[5].yb+7,COLOR_L_ASCII,
  caracter);

}

/****************************************************************************
	DEF_CARACTER: rutina de manejo de pulsaci¢n en cuadro con
	  la definici¢n del car cter a tama¤o real.
****************************************************************************/
void def_caracter(void)
{
STC_PUNTERO puntero;
int asc;

/* oculta puntero  mientras dibuja */
vis_puntero(ESCONDE);

/* marca recuadro de bot¢n para indicar que ha sido pulsado */
cambia_boton(barra,5,INV,IGUAL);
dibuja_cuadro_boton(barra[0].bot[5].xb,barra[0].bot[5].yb,
  barra[0].bot[5].ancho,barra[0].bot[5].alto,barra[0].color_fondo,
  barra[0].color_s1,barra[0].color_s2,barra[0].bot[5].modo,OFF);

vis_puntero(MUESTRA);

/* espera a que se suelte el bot¢n */
do {
	coge_pos_puntero(&puntero);
} while(puntero.botones & PULSADO_IZQ);

/* espera hasta que pulse en otro bot¢n */
do {
	pulsado=esta_pulsado(&barra[0],NUM_BARR);
} while(pulsado.boton==99);

/* oculta puntero  mientras dibuja */
vis_puntero(ESCONDE);

/* restaura recuadro de bot¢n */
cambia_boton(barra,5,INV,IGUAL);
dibuja_cuadro_boton(barra[0].bot[5].xb,barra[0].bot[5].yb,
  barra[0].bot[5].ancho,barra[0].bot[5].alto,barra[0].color_fondo,
  barra[0].color_s1,barra[0].color_s2,barra[0].bot[5].modo,OFF);

vis_puntero(MUESTRA);

/* si pulsa en tabla ASCII pasa la definici¢n actual a la */
/* posici¢n de la tabla d¢nde puls¢ */
if((pulsado.barra==0) && (pulsado.boton)==4) {
	asc=pulsa_en_ascii(pulsado.x,pulsado.y);

	/* copia definici¢n de car cter en tabla ASCII */
	copia_def_car(caracter,tabla_ascii[asc]);

	/* reimprime en pantalla */
	actualiza_tabla(FILA_TABLA,COLUMNA_TABLA,asc);
}

}

/****************************************************************************
	ALMACENA_DEFINICION: almacena la definici¢n actual del car cter
	  en la posici¢n actual de la tabla ASCII.
****************************************************************************/
void almacena_definicion(void)
{

/* copia la definici¢n actual en la posici¢n actual de la tabla ASCII */
copia_def_car(caracter,tabla_ascii[ascii]);

/* y actualiza tabla en pantalla */
actualiza_tabla(FILA_TABLA,COLUMNA_TABLA,ascii);

}

/****************************************************************************
	BORRA_REJILLA: borra la rejilla de dibujo y la definici¢n actual de
	  car cter.
****************************************************************************/
void borra_rejilla(void)
{

/* limpia definici¢n del car cter usando 0 */
limpia_caracter(caracter,ALTURA_CUAD,0);

/* dibuja definici¢n en cuadr¡cula */
dibuja_en_cuadricula(barra[0].bot[3].xb+3,barra[0].bot[3].yb+3,caracter);
dibuja_caracter(barra[0].bot[5].xb+7,barra[0].bot[5].yb+7,COLOR_L_ASCII,
  caracter);

}

/****************************************************************************
	LLENA_REJILLA: llena la rejilla de dibujo y la definici¢n actual de
	  car cter.
****************************************************************************/
void llena_rejilla(void)
{

/* limpia definici¢n del car cter usando 0xFF */
limpia_caracter(caracter,ALTURA_CUAD,0xff);

/* dibuja definici¢n en cuadr¡cula */
dibuja_en_cuadricula(barra[0].bot[3].xb+3,barra[0].bot[3].yb+3,caracter);
dibuja_caracter(barra[0].bot[5].xb+7,barra[0].bot[5].yb+7,COLOR_L_ASCII,
  caracter);

}

/****************************************************************************
	SCROLL_DEF_ARR: desplaza la definici¢n del car cter una l¡nea
	  hacia arriba, la l¡nea que sale por arriba aparece por abajo.
****************************************************************************/
void scroll_def_arr(void)
{
BYTE c;
int i;

/* guarda la primera l¡nea */
c=caracter[0];

/* desplaza la definici¢n 1 l¡nea hacia arriba */
for(i=1; i<ALTURA_CUAD; caracter[i-1]=caracter[i], i++);

/* pone la primera l¡nea en la £ltima */
caracter[ALTURA_CUAD-1]=c;

/* dibuja definici¢n en cuadr¡cula */
dibuja_en_cuadricula(barra[0].bot[3].xb+3,barra[0].bot[3].yb+3,caracter);
dibuja_caracter(barra[0].bot[5].xb+7,barra[0].bot[5].yb+7,COLOR_L_ASCII,
  caracter);

}

/****************************************************************************
	SCROLL_DEF_ABJ: desplaza la definici¢n del car cter una l¡nea
	  hacia abajo, la l¡nea que sale por abajo aparece por arriba.
****************************************************************************/
void scroll_def_abj(void)
{
BYTE c;
int i;

/* guarda la primera l¡nea */
c=caracter[ALTURA_CUAD-1];

/* desplaza la definici¢n 1 l¡nea hacia abajo */
for(i=ALTURA_CUAD-1; i>0; caracter[i]=caracter[i-1], i--);

/* pone la £ltima l¡nea en la primera */
caracter[0]=c;

/* dibuja definici¢n en cuadr¡cula */
dibuja_en_cuadricula(barra[0].bot[3].xb+3,barra[0].bot[3].yb+3,caracter);
dibuja_caracter(barra[0].bot[5].xb+7,barra[0].bot[5].yb+7,COLOR_L_ASCII,
  caracter);

}

/****************************************************************************
	SCROLL_DEF_IZQ: desplaza la definici¢n del car cter una columna
	  hacia la izquierda, la columna que sale por la izquierda aparece
	  por la derecha.
****************************************************************************/
void scroll_def_izq(void)
{
BYTE c;
int i;

for(i=0; i<ALTURA_CUAD; i++) {
	/* coge el £ltimo bit y lo guarda como primer bit */
	c=(BYTE)((caracter[i] & 0x80) >> 7);
	/* desplaza hacia izquierda y mete £ltimo bit en primero */
	caracter[i]=(BYTE)((caracter[i] << 1) | c);
}

/* dibuja definici¢n en cuadr¡cula */
dibuja_en_cuadricula(barra[0].bot[3].xb+3,barra[0].bot[3].yb+3,caracter);
dibuja_caracter(barra[0].bot[5].xb+7,barra[0].bot[5].yb+7,COLOR_L_ASCII,
  caracter);

}

/****************************************************************************
	SCROLL_DEF_DER: desplaza la definici¢n del car cter una columna
	  hacia la derecha, la columna que sale por la derecha aparece
	  por la izquierda.
****************************************************************************/
void scroll_def_der(void)
{
BYTE c;
int i;

for(i=0; i<ALTURA_CUAD; i++) {
	/* coge el primer bit y lo guarda como £ltimo bit */
	c=(BYTE)((caracter[i] & 0x01) << 7);
	/* desplaza hacia derecha y mete primer bit en £ltimo */
	caracter[i]=(BYTE)((caracter[i] >> 1) | c);
}

/* dibuja definici¢n en cuadr¡cula */
dibuja_en_cuadricula(barra[0].bot[3].xb+3,barra[0].bot[3].yb+3,caracter);
dibuja_caracter(barra[0].bot[5].xb+7,barra[0].bot[5].yb+7,COLOR_L_ASCII,
  caracter);

}

/****************************************************************************
	PIDE_NOMBRE_FICH: pide el nombre de un fichero; para ello usa el
	  bot¢n 6 de la barra 0 como zona de entrada.
	  Entrada:      'nombre' puntero a buffer d¢nde se almacenar 
			el nombre
			'longitud' m ximo n£mero de caracteres permitidos
			sin contar el '\0' final
	  Salida:       n£mero de caracteres tecleados y nombre de
			fichero almacenado en buffer
****************************************************************************/
int pide_nombre_fich(char *nombre, int longitud)
{
int lng;
struct xycoord coord_ant, coord_act;
short ant_color;

/* oculta puntero mientras pide caracteres */
vis_puntero(ESCONDE);

/* coordenadas para mensaje de introducci¢n (guarda las actuales) */
coord_ant=_moveto(barra[0].bot[6].xb+6,barra[0].bot[6].yb+3);
/* color para mensaje de introducci¢n (guarda el actual) */
ant_color=_setcolor(barra[0].color_pplano);

/* selecciona fuente e imprime mensaje */
if(_setfont("t'helv'h12")>=0) _outgtext("Fichero: ");

/* coge coordenadas actuales */
coord_act=_getcurrentposition();

lng=lee_input(coord_act.xcoord,coord_act.ycoord,nombre,longitud,
  barra[0].color_s2,barra[0].color_fondo);

vis_puntero(MUESTRA);

/* recupera coordenadas antiguas y color */
_moveto(coord_ant.xcoord,coord_ant.ycoord);
_setcolor(ant_color);

return(lng-1);
}

/****************************************************************************
	ESPERA_TECLA: espera hasta que se pulse una tecla.
	  Salida:       c¢digo de scan (byte alto) y c¢digo ASCII
			(byte bajo) de la tecla pulsada
****************************************************************************/
short espera_tecla(void)
{
short tecla;

_asm {
	mov ax,0
	int 16h
	mov tecla,ax
}

return(tecla);
}

/****************************************************************************
	CARGAR_FUENTE: rutina para cargar un fichero con definiciones de
	  caracteres.
****************************************************************************/
void cargar_fuente(void)
{
char nfich[41];

/* pide nombre de fichero */
pide_nombre_fich(nfich,40);

if(!carga_def(nfich,&tabla_ascii[0][0])) {
	escribe_texto_boton("Fichero no v lido o error en lectura "
	  "(pulsa una tecla).",&barra[0].bot[7],barra[0].color_pplano,
	  barra[0].color_s1);
	espera_tecla();
}

/* borra zona de entrada de datos y de mensajes */
dibuja_boton(&barra[0].bot[6],barra[0].color_fondo,barra[0].color_pplano,
  barra[0].color_s1,barra[0].color_s2);
dibuja_boton(&barra[0].bot[7],barra[0].color_fondo,barra[0].color_pplano,
  barra[0].color_s1,barra[0].color_s2);

/* redibuja la tabla ASCII */
/* borra el marcador */
dibuja_marcador_ascii(ascii);
dibuja_tabla_ascii(FILA_TABLA,COLUMNA_TABLA);
/* dibuja el marcador */
dibuja_marcador_ascii(ascii);

}

/****************************************************************************
	GRABAR_FUENTE: rutina para grabar un fichero con definiciones de
	  caracteres
****************************************************************************/
void grabar_fuente(void)
{
char nfich[41];
short tecla='S';

/* pide nombre de fichero */
pide_nombre_fich(nfich,40);

/* comprueba si existe y pide confirmaci¢n */
if(!access(nfich,0)) {
	escribe_texto_boton("Ese fichero ya existe. ¨Reemplazarlo?",
	  &barra[0].bot[7],barra[0].color_pplano,barra[0].color_s1);
	tecla=espera_tecla() & 0x00ff;
	dibuja_boton(&barra[0].bot[7],barra[0].color_fondo,
	  barra[0].color_pplano,barra[0].color_s1,barra[0].color_s2);
}

if((tecla=='s') || (tecla=='S')) {
	if(!graba_def(nfich,&tabla_ascii[0][0])) {
		escribe_texto_boton("No se pudo abrir fichero o error "
		  "en escritura (pulsa una tecla).",&barra[0].bot[7],
		  barra[0].color_pplano,barra[0].color_s1);
		espera_tecla();
	}
}

/* borra zona de entrada de datos y de mensajes */
dibuja_boton(&barra[0].bot[6],barra[0].color_fondo,barra[0].color_pplano,
  barra[0].color_s1,barra[0].color_s2);
dibuja_boton(&barra[0].bot[7],barra[0].color_fondo,barra[0].color_pplano,
  barra[0].color_s1,barra[0].color_s2);

}

