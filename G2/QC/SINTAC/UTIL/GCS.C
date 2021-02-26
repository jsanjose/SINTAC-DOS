/****************************************************************************
		       GENERADOR DE CARACTERES SINTAC
			    (c)1993 JSJ Soft Ltd.
****************************************************************************/

#include <stdio.h>
#include <process.h>
#include <stdlib.h>
#include <graph.h>
#include <dos.h>
#include <bios.h>
#include <io.h>
#include "boton.h"
#include "puntero.h"
#include "ventana.h"
#include "chrimp.h"
#include "..\sintac.h"
#include "gcs.h"

/*** Variables globales ***/
/* tablas donde se almacenar n las definiciones de los caracteres 8x16 y 8x8 */
BYTE tabla_ascii8x16[256][16];
BYTE tabla_ascii8x8[256][8];

BYTE *tabla_ascii;              /* puntero a tabla definiciones caracteres */
int altura_cuad=16;             /* altura de caracteres actual */
BYTE caracter[16];              /* definici¢n car cter actual */
int ascii=0;                    /* c¢digo ASCII de car cter actual */
int car_modificado=0;           /* indicador de modificaci¢n de caracteres */

/* £ltimo cuadro sobre el que se puls¢ el puntero en la rejilla de dibujo */
short ant_cuadrox=9999, ant_cuadroy=9999;

STC_CODBOTON pulsado;           /* para recoger bot¢n pulsado */

struct videoconfig vc;          /* configuraci¢n del sistema de v¡deo */

/*** Programa principal ***/
void main(void)
{
STC_PUNTERO puntero;
int i;

/* ajusta modo de v¡deo */
if(!_setvideomode(_ERESCOLOR)) {
	printf("\nEste programa requiere tarjeta EGA o VGA.\n");
	exit(1);
}

/* inicializa las rutinas de control del puntero */
inic_puntero();

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
escribe_texto_boton(GCS_COPYRIGHT,&barra[0].bot[1],barra[0].color_pplano,
  barra[0].color_tecla,barra[0].color_fondo);

/* cuadricula zona de dibujo */
dibuja_cuadricula(barra[0].bot[3].xb+3,barra[0].bot[3].yb+3);

/* copia definiciones de tabla ROM de caracteres 8x16 y 8x8 en RAM */
copia_tabla_rom(TAB_ROM8x16,&tabla_ascii8x16[0][0],16);
copia_tabla_rom(TAB_ROM8x8,&tabla_ascii8x8[0][0],8);

/* inicializa puntero a definiciones de caracteres */
tabla_ascii=&tabla_ascii8x16[0][0];

/* tabla ASCII */
dibuja_tabla_ascii(barra[0].bot[4].xb+3,barra[0].bot[4].yb+3);

/* dibuja marcador en el car cter actual de la tabla */
dibuja_marcador_ascii(ascii);

/* escribe c¢digo ASCII del car cter actual */
imp_ascii_car();

/* borra la definici¢n del car cter y lo dibuja en pantalla */
limpia_caracter(caracter,0);
_setcolor(0);
_rectangle(_GFILLINTERIOR,barra[0].bot[5].xb+3,barra[0].bot[5].yb+3,
  barra[0].bot[5].xb+barra[0].bot[5].ancho-4,
  barra[0].bot[5].yb+barra[0].bot[5].alto-4);
dibuja_caracter();

/* muestra puntero del rat¢n */
vis_puntero(MUESTRA);

do {
	do {
		if(hay_raton()) {
			coge_pos_puntero(&puntero);
			if(!(puntero.botones & PULSADO_IZQ)) {
				ant_cuadrox=9999;
				ant_cuadroy=9999;
			}
		}
		pulsado=esta_pulsado(&barra[0],NUM_BARR);
	} while(pulsado.boton==NO_BOTON);

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
					tablacar_modificada();
					cargar_fuente();
					break;
				case 1 :        /* Grabar fuente */
					grabar_fuente();
					break;
				case 2 :        /* Fuente 8x16 */
					/* si no est  en 8x16 */
					if(altura_cuad!=16) pasa_a8x16();
					break;
				case 3 :        /* Fuente 8x8 */
					/* si no est  en 8x8 */
					if(altura_cuad!=8) pasa_a8x8();
					break;
				case 4 :        /* Prueba */
					prueba_car();
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
				case 3 :        /* Invertir */
					invertir_def();
					break;
				case 4 :        /* Arriba */
					scroll_def_arr();
					break;
				case 5 :        /* Abajo */
					scroll_def_abj();
					break;
				case 6 :        /* Izquierda */
					scroll_def_izq();
					break;
				case 7 :        /* Derecha */
					scroll_def_der();
					break;
				case 8 :        /* Girar 180 grados */
					gira180();
					break;
				case 9 :        /* Espejo */
					espejo_def();
					break;
			}
			break;
	}
} while((pulsado.barra!=0) || (pulsado.boton!=2));

tablacar_modificada();

/* oculta puntero del rat¢n */
vis_puntero(ESCONDE);

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
	  NOTA: las dimensiones de cada cuadro son CUAD_ANCHO y CUAD_ALTO
****************************************************************************/
void dibuja_cuadricula(short x, short y)
{
short x0, y0;
int i, j;

/* color de fondo de la zona de dibujo */
_setcolor(COLOR_F_CUAD);
_rectangle(_GFILLINTERIOR,x,y,x+(CUAD_ANCHO*8)-1,y+(CUAD_ALTO*altura_cuad)-1);

y0=y;

/* color de borde de cuadr¡cula */
_setcolor(COLOR_B_CUAD);
for(i=0; i<altura_cuad; i++, y0+=CUAD_ALTO) {
	x0=x;
	for(j=0; j<8; j++, x0+=CUAD_ANCHO)
	  _rectangle(_GBORDER,x0,y0,x0+(CUAD_ANCHO-1),y0+(CUAD_ALTO-1));
}

}

/****************************************************************************
	CAMBIA_CUADRICULA: invierte el estado de un cuadro de la cuadr¡cula.
	  Entrada:      'x', 'y' coordenadas del cuadro a modificar ([0..7] y
			[0..(altura_cuad-1)])
			'orgx', 'orgy' origen de la cuadr¡cula
****************************************************************************/
void cambia_cuadricula(short x, short y, short orgx, short orgy)
{
short cuadx, cuady;

/* oculta el puntero durante el cambio de cuadro */
vis_puntero(ESCONDE);

/* comprueba que no se salga de los l¡mites */
x=(x>7) ? 7 : x;
y=(y>(altura_cuad-1)) ? altura_cuad-1 : y;

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
	IMP_CHRDEF: imprime la definici¢n de un car cter en la posici¢n
	  actual de pantalla; la posici¢n actual es la £ltima definida por
	  la funci¢n _moveto(), o por alguna otra funci¢n gr fica.
	  Entrada:      'pchr' puntero a definici¢n de car cter a imprimir
			'colorf', 'color' colores de fondo y primer plano
			'alt' altura de la definici¢n del car cter
			'modo' modo de dibujo (C_NORM, C_XOR, C_OR, C_AND)
	  Salida:       posici¢n de dibujo actualizada

	  NOTA: esta rutina s¢lo sirve para los modos de 16 colores de alta
	    resoluci¢n de EGA y VGA
****************************************************************************/
void imp_chrdef(BYTE _far *pchr, short colorf, short color, short alt,
  unsigned char modo)
{
unsigned char _far *video;
struct videoconfig vc;
struct xycoord pos;
short x, y, maxx, maxy;
short varshift;

/* coge configuraci¢n del sistema de v¡deo */
_getvideoconfig(&vc);

/* coge la posici¢n de dibujo actual */
pos=_getcurrentposition();
x=pos.xcoord;
y=pos.ycoord;

/* calcula las m ximas coordenadas */
maxx=vc.numxpixels-1;
maxy=vc.numypixels-1;

/* calcula direcci¢n en buffer de v¡deo */
/* modo de 16 colores de alta resoluci¢n de EGA y VGA */
if((vc.mode==_ERESCOLOR) || (vc.mode==_VRES16COLOR))
  video=(unsigned char _far *)0xa0000000+((x/8)+(y*80));
else return;

_asm {
	push di
	push si
	push ds

	mov cx,x                ; CL = byte bajo de coordenada X
	and cl,7
	xor cl,7                ; CL = n§ de bits a desplazar izq.
	inc cx
	and cl,7                ; CL = n§ bits a despl. para enmascarar
	mov ch,0ffh
	shl ch,cl               ; CH = m scara lado derecho de car cter
	mov varshift,cx

	les di,pchr             ; ES:DI = dir. definici¢n de car cter
	lds si,video            ; DS:SI = dir. en buffer de v¡deo
	mov cx,alt              ; CX = altura en pixels del car cter

	; establecer registros del controlador de gr ficos

	mov dx,3ceh             ; puerto del registro de direcciones
	mov ax,0a05h            ; modo escritura 2, modo lectura 1
	out dx,ax
	mov ah,modo             ; modo de dibujo: 18h = XOR, 10h = OR,
				; 08h = AND, 00h = NORM
	mov al,3
	out dx,ax
	mov ax,0007h            ; color "don't care" todos los planos
	out dx,ax

	mov bl,BYTE PTR color   ; BL = valor de pixel imagen
	mov bh,BYTE PTR colorf  ; BH = valor de pixel fondo

	cmp BYTE PTR varshift,0
	jne chr_no_alin         ; salta si car cter no alineado

	; rutina para caracteres alineados

	mov al,8                ; AL = n§ registro de m scara de bit

chr_alin:
	mov ah,es:[di]          ; AH = patr¢n de fila de pixels
	out dx,ax               ; lo carga en registro m scara de bit
	and [si],bl             ; actualiza pixels de imagen
	not ah
	out dx,ax
	and [si],bh             ; actualiza pixels de fondo

	inc di                  ; siguiente byte del car cter
	add si,80               ; siguiente l¡nea de pantalla

	loop chr_alin
	jmp chr_fin

	; rutina para caracteres no alineados

chr_no_alin:
	push cx                 ; guarda contador n§ de l¡neas de car.
	mov cx,varshift         ; CH = m scara lado derecho
				; CL = n§ de bits a desplazar
	; dibuja lado izquierdo del car cter
	mov al,es:[di]          ; AL = patr¢n de fila de pixels
	xor ah,ah
	shl ax,cl               ; AH = patr¢n para lado izquierdo
				; AL = patr¢n para lado derecho
	push ax                 ; guarda patrones
	mov al,8                ; n§ de registro de m scara de bit
	out dx,ax               ; carga patr¢n de lado izquierdo
	and [si],bl             ; actualiza pixels de imagen
	not ch                  ; CH = m scara para lado izquierdo
	xor ah,ch
	out dx,ax
	and [si],bh             ; actualiza pixels de fondo
	; dibuja lado derecho del car cter
	pop ax                  ; recupera patrones
	mov ah,al               ; AH = patr¢n lado derecho
	mov al,8                ; AL = n§ registro de m scara de bit
	out dx,ax               ; carga patr¢n
	inc si                  ; posici¢n en buffer de v¡deo
	and [si],bl             ; actualiza pixels de imagen
	not ch                  ; CH = m scara para lado derecho
	xor ah,ch
	out dx,ax
	and [si],bh             ; actualiza pixels de fondo

	inc di                  ; siguiente byte del car cter
	dec si
	add si,80               ; siguiente l¡nea de pantalla
	pop cx
	loop chr_no_alin

	; restaura registros de controlador a sus valores por defecto

chr_fin:
	mov ax,0ff08h           ; m scara de bit
	out dx,ax
	mov ax,0005h            ; registro de modo
	out dx,ax
	mov ax,0003h            ; rotar dato/selecc. funci¢n
	out dx,ax
	mov ax,0f07h            ; color "don't care"
	out dx,ax

	pop ds
	pop si
	pop di
}

/* actualiza posici¢n de dibujo */
/* actualiza coordenada X */
x+=8;
if(x>maxx) {
	x=0;
	y+=altura_cuad;
}
/* actualiza coordenada Y, si se sale de pantalla pasa a (0,0) */
if(y>(maxy-(altura_cuad-1))) {
	x=0;
	y=0;
}

_moveto(x,y);

}

/****************************************************************************
	DIBUJA_TABLA_ASCII: dibuja la tabla de caracteres ASCII en forma
	  de matriz de 16x16.
	  Entrada:      'x', 'y' posici¢n de inicio de la tabla
****************************************************************************/
void dibuja_tabla_ascii(short x, short y)
{
BYTE espacio[16];
int i, j, car=0;

/* esconde puntero mientras dibuja tabla */
vis_puntero(ESCONDE);

/* crea una definici¢n de espacio */
for(i=0; i<16; i++) espacio[i]=0;

for(i=0; i<16; i++) {
	_moveto(x,y);

	/* imprime espacio al inicio de cada l¡nea */
	imp_chrdef(&espacio[0],0,COLOR_L_ASCII,altura_cuad,C_NORM);

	for(j=0; j<16; j++, car++) {
		imp_chrdef(tabla_ascii+(car*altura_cuad),0,COLOR_L_ASCII,
		  altura_cuad,C_NORM);
		imp_chrdef(&espacio[0],0,COLOR_L_ASCII,altura_cuad,C_NORM);
	}

	y+=altura_cuad;
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

mascara >>= x;
car[y] ^= mascara;

}

/****************************************************************************
	LIMPIA_CARACTER: rellena la definici¢n de un car cter con un BYTE
	  dado.
	  Entrada:      'car' puntero a definici¢n del car cter
			'byte' byte de relleno
****************************************************************************/
void limpia_caracter(BYTE *car, BYTE byte)
{
int i;

for(i=0; i<altura_cuad; car[i++]=byte);

}

/****************************************************************************
	PULSA_EN_ASCII: devuelve el c¢digo ASCII que corresponde a un
	  car cter situado en la tabla ASCII 16x16
	  Entrada:      'x', 'y' posici¢n relativa al origen de la tabla
	  Salida:       c¢digo ASCII del car cter cuya posici¢n coincide
			con las coordenadas 'x', 'y'
****************************************************************************/
int pulsa_en_ascii(short x, short y)
{
short x0, y0;

/* transforma en valor 0..15 */
x0=x/16;
y0=y/altura_cuad;

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
orgx=barra[0].bot[4].xb+3;
orgy=barra[0].bot[4].yb+3;

/* coordenadas relativas de un car cter dentro de la tabla 16x16 en */
/* funci¢n de su c¢digo ASCII */
x=(((ascii%16)*2)+1)*8;
y=(ascii/16)*altura_cuad;

/* desactiva puntero mientras dibuja */
vis_puntero(ESCONDE);

/* dibuja un rect ngulo alrededor del car cter seleccionado */
modo=_setwritemode(_GXOR);
_setcolor(COLOR_R_ASCII);
_rectangle(_GBORDER,orgx+x-1,orgy+y-1,orgx+x+8,orgy+y+altura_cuad);

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
			'alt' altura de caracteres a copiar
****************************************************************************/
void copia_tabla_rom(BYTE tabla, BYTE _far *tabla_car, int alt)
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

for(i=0; i<256*alt; i++) tabla_car[i]=*dir_tabla_rom++;

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

for(i=0; i<altura_cuad; i++) destino[i]=origen[i];

}

/****************************************************************************
	DIBUJA_EN_CUADRICULA: dibuja la definici¢n del car cter actual en la
	  cuadr¡cula de dibujo.
****************************************************************************/
void dibuja_en_cuadricula(void)
{
int i, j;
BYTE c, mascara;
short x, y;

/* esconde puntero mientras dibuja */
vis_puntero(ESCONDE);

y=barra[0].bot[3].yb+3;
for(i=0; i<altura_cuad; i++, y+=CUAD_ALTO) {
	c=caracter[i];
	mascara=0x80;
	x=barra[0].bot[3].xb+3;
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
	  Entrada:      'x', 'y' origen de la tabla
			'ascii' c¢digo ASCII del car cter
****************************************************************************/
void actualiza_tabla(short x, short y, int ascii)
{
short xcar, ycar;

/* oculta puntero mientras actualiza tabla ASCII */
vis_puntero(ESCONDE);

/* calcula posici¢n del car cter de acuerdo a su c¢digo ASCII */
/* teniendo en cuenta que hay un espacio entre cada columna de la tabla */
ycar=(ascii/16)*altura_cuad;
xcar=(((ascii%16)*2)+1)*8;

/* imprime car cter en su posici¢n dentro de la tabla */
_moveto(x+xcar,y+ycar);
imp_chrdef(tabla_ascii+(ascii*altura_cuad),0,COLOR_L_ASCII,altura_cuad,
  C_NORM);

vis_puntero(MUESTRA);

}

/****************************************************************************
	DIBUJA_CARACTER: dibuja la definici¢n de un car cter a tama¤o real.
****************************************************************************/
void dibuja_caracter(void)
{

/* oculta el puntero mientras dibuja car cter */
vis_puntero(ESCONDE);

_moveto(barra[0].bot[5].xb+7,barra[0].bot[5].yb+7+((16-altura_cuad)/2));
imp_chrdef(caracter,0,COLOR_L_ASCII,altura_cuad,C_NORM);

vis_puntero(MUESTRA);

}

/****************************************************************************
	GRABA_DEF: graba en un fichero las definiciones de los caracteres.
	  Entrada:      'nombre' nombre con que se grabar  la fuente
	  Salida:       0 si hubo error o un valor distinto de 0 en
			otro caso
****************************************************************************/
int graba_def(char *nombre)
{
FILE *ffuente;
size_t num_bytes;

/* abre el fichero para escritura en modo binario */
ffuente=fopen(nombre,"wb");
/* sale si hubo error */
if(ffuente==NULL) return(0);

/* escribe cadena de reconocimiento */
if(fwrite(RECON_FUENTE,sizeof(char),LONG_RECON_F+1,ffuente)<LONG_RECON_F+1) {
	fclose(ffuente);
	return(0);
}

/* escribe definiciones de caracteres 8x16 */
num_bytes=(size_t)(256*16);
if(fwrite(tabla_ascii8x16,sizeof(BYTE),num_bytes,ffuente)<num_bytes) {
	fclose(ffuente);
	return(0);
}

/* escribe definiciones de caracteres 8x8 */
num_bytes=(size_t)(256*8);
if(fwrite(tabla_ascii8x8,sizeof(BYTE),num_bytes,ffuente)<num_bytes) {
	fclose(ffuente);
	return(0);
}

fclose(ffuente);

car_modificado=0;

return(1);
}

/****************************************************************************
	CARGA_DEF: carga de un fichero las definiciones de los
	  caracteres.
	  Entrada:      'nombre' nombre del fichero
	  Salida:       0 si hubo error o un valor distinto de 0 en
			otro caso
****************************************************************************/
int carga_def(char *nombre)
{
FILE *ffuente;
char cad_recon[LONG_RECON_F+1];
char *recon_fuente=RECON_FUENTE;
size_t num_bytes;

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
/* definiciones 8x16 */
num_bytes=(size_t)(256*16);
if(fread(tabla_ascii8x16,sizeof(BYTE),num_bytes,ffuente)<num_bytes) {
	fclose(ffuente);
	return(0);
}

/* definiciones 8x8 */
num_bytes=(size_t)(256*8);
if(fread(tabla_ascii8x8,sizeof(BYTE),num_bytes,ffuente)<num_bytes) {
	fclose(ffuente);
	return(0);
}

fclose(ffuente);

car_modificado=0;

return(1);
}

/****************************************************************************
	CUADRICULA: rutina de manejo de la cuadr¡cula de dibujo.
****************************************************************************/
void cuadricula(void)
{
short cuadrox, cuadroy;

cuadrox=pulsado.x/CUAD_ANCHO;
cuadroy=pulsado.y/CUAD_ALTO;

if((cuadrox!=ant_cuadrox) || (cuadroy!=ant_cuadroy)) {
	/* modifica cuadr¡cula */
	cambia_cuadricula(pulsado.x/CUAD_ANCHO,pulsado.y/CUAD_ALTO,
	  barra[0].bot[3].xb+3,barra[0].bot[3].yb+3);

	/* modifica bit del car cter */
	cambia_caracter(caracter,pulsado.x/CUAD_ANCHO,pulsado.y/CUAD_ALTO);

	/* dibuja car cter */
	dibuja_caracter();

	ant_cuadrox=cuadrox;
	ant_cuadroy=cuadroy;
}

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

/* escribe c¢digo ASCII del car cter actual */
imp_ascii_car();

/* dibuja marcador */
dibuja_marcador_ascii(ascii);

/* copia definici¢n de caracter en buffer auxiliar */
copia_def_car(tabla_ascii+(ascii*altura_cuad),caracter);

/* dibuja en cuadr¡cula y a tama¤o real */
dibuja_en_cuadricula();
dibuja_caracter();

}

/****************************************************************************
	DEF_CARACTER: rutina de manejo de pulsaci¢n en cuadro con
	  la definici¢n del car cter a tama¤o real.
****************************************************************************/
void def_caracter(void)
{
STC_PUNTERO puntero;
int asc;

/* marca recuadro de bot¢n para indicar que ha sido pulsado */
cambia_boton(barra,5,INV,IGUAL);
dibuja_cuadro_boton(barra[0].bot[5].xb,barra[0].bot[5].yb,
  barra[0].bot[5].ancho,barra[0].bot[5].alto,barra[0].color_fondo,
  barra[0].color_s1,barra[0].color_s2,barra[0].bot[5].modo,OFF);

/* espera a que se suelte el bot¢n del rat¢n */
if(hay_raton()) do {
	coge_pos_puntero(&puntero);
} while(puntero.botones & PULSADO_IZQ);

/* espera hasta que pulse en otro bot¢n */
do {
	pulsado=esta_pulsado(&barra[0],NUM_BARR);
} while(pulsado.boton==NO_BOTON);

/* restaura recuadro de bot¢n */
cambia_boton(barra,5,INV,IGUAL);
dibuja_cuadro_boton(barra[0].bot[5].xb,barra[0].bot[5].yb,
  barra[0].bot[5].ancho,barra[0].bot[5].alto,barra[0].color_fondo,
  barra[0].color_s1,barra[0].color_s2,barra[0].bot[5].modo,OFF);

/* si pulsa en tabla ASCII pasa la definici¢n actual a la */
/* posici¢n de la tabla d¢nde puls¢ */
if((pulsado.barra==0) && (pulsado.boton)==4) {
	asc=pulsa_en_ascii(pulsado.x,pulsado.y);

	/* copia definici¢n de car cter en tabla ASCII */
	copia_def_car(caracter,tabla_ascii+(asc*altura_cuad));

	/* reimprime en pantalla */
	actualiza_tabla(barra[0].bot[4].xb+3,barra[0].bot[4].yb+3,asc);

	car_modificado=1;
}

}

/****************************************************************************
	ALMACENA_DEFINICION: almacena la definici¢n actual del car cter
	  en la posici¢n actual de la tabla ASCII.
****************************************************************************/
void almacena_definicion(void)
{

/* copia la definici¢n actual en la posici¢n actual de la tabla ASCII */
copia_def_car(caracter,tabla_ascii+(ascii*altura_cuad));

/* y actualiza tabla en pantalla */
actualiza_tabla(barra[0].bot[4].xb+3,barra[0].bot[4].yb+3,ascii);

car_modificado=1;

}

/****************************************************************************
	BORRA_REJILLA: borra la rejilla de dibujo y la definici¢n actual de
	  car cter.
****************************************************************************/
void borra_rejilla(void)
{

/* limpia definici¢n del car cter usando 0 */
limpia_caracter(caracter,0);

/* dibuja definici¢n en cuadr¡cula */
dibuja_en_cuadricula();
dibuja_caracter();

}

/****************************************************************************
	LLENA_REJILLA: llena la rejilla de dibujo y la definici¢n actual de
	  car cter.
****************************************************************************/
void llena_rejilla(void)
{

/* limpia definici¢n del car cter usando 0xFF */
limpia_caracter(caracter,0xff);

/* dibuja definici¢n en cuadr¡cula */
dibuja_en_cuadricula();
dibuja_caracter();

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
for(i=1; i<altura_cuad; caracter[i-1]=caracter[i], i++);

/* pone la primera l¡nea en la £ltima */
caracter[altura_cuad-1]=c;

/* dibuja definici¢n en cuadr¡cula */
dibuja_en_cuadricula();
dibuja_caracter();

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
c=caracter[altura_cuad-1];

/* desplaza la definici¢n 1 l¡nea hacia abajo */
for(i=altura_cuad-1; i>0; caracter[i]=caracter[i-1], i--);

/* pone la £ltima l¡nea en la primera */
caracter[0]=c;

/* dibuja definici¢n en cuadr¡cula */
dibuja_en_cuadricula();
dibuja_caracter();

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

for(i=0; i<altura_cuad; i++) {
	/* coge el £ltimo bit y lo guarda como primer bit */
	c=(BYTE)((caracter[i] & 0x80) >> 7);
	/* desplaza hacia izquierda y mete £ltimo bit en primero */
	caracter[i]=(BYTE)((caracter[i] << 1) | c);
}

/* dibuja definici¢n en cuadr¡cula */
dibuja_en_cuadricula();
dibuja_caracter();

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

for(i=0; i<altura_cuad; i++) {
	/* coge el primer bit y lo guarda como £ltimo bit */
	c=(BYTE)((caracter[i] & 0x01) << 7);
	/* desplaza hacia derecha y mete primer bit en £ltimo */
	caracter[i]=(BYTE)((caracter[i] >> 1) | c);
}

/* dibuja definici¢n en cuadr¡cula */
dibuja_en_cuadricula();
dibuja_caracter();

}

/****************************************************************************
	INVERTIR_DEF: invierte la definici¢n del car cter de la cuadr¡-
	  cula; pone a 1 los bits 0 y a 0 los bits 1.
****************************************************************************/
void invertir_def(void)
{
int i;

for(i=0; i<altura_cuad; i++) caracter[i]=(BYTE)~caracter[i];

/* dibuja definici¢n en cuadr¡cula */
dibuja_en_cuadricula();
dibuja_caracter();

}

/****************************************************************************
	GIRA180: gira el car cter 180 grados.
****************************************************************************/
void gira180(void)
{
BYTE c;
int i;

for(i=0; i<(altura_cuad/2); i++) {
	/* intercambia caracteres */
	c=caracter[(altura_cuad-1)-i];
	caracter[(altura_cuad-1)-i]=caracter[i];
	caracter[i]=c;
}

/* dibuja definici¢n en cuadr¡cula */
dibuja_en_cuadricula();
dibuja_caracter();

}

/****************************************************************************
	ESPEJO_DEF: efecto espejo.
****************************************************************************/
void espejo_def(void)
{
BYTE c, m1, m2;
int i, j;

for(i=0; i<altura_cuad; i++) {
	c=0;
	m1=0x01;
	m2=0x80;
	/* refleja byte */
	for(j=0; j<8; j++) {
		if(caracter[i] & m1) c |= m2;
		m1 <<= 1;
		m2 >>= 1;
	}
	/* almacena byte reflejado */
	caracter[i]=c;
}

/* dibuja definici¢n en cuadr¡cula */
dibuja_en_cuadricula();
dibuja_caracter();

}

/****************************************************************************
	CARGAR_FUENTE: rutina para cargar un fichero con definiciones de
	  caracteres.
****************************************************************************/
void cargar_fuente(void)
{
char nfich[_MAX_PATH+LNG_NFICH];
int acc;

acc=cuadro_fich(V_CENT,100,COLOR_FONDO,COLOR_PPLAN,COLOR_S1,COLOR_S2,
  COLOR_TECLA,"&Cargar",nfich);

if(acc!=CF_ACCION) return;

/* sale si no se eligi¢ ning£n fichero */
if(!*nfich) return;

if(!carga_def(nfich)) {
	cuadro_aviso("\nFichero no v lido\no error en lectura.",V_CENT,V_CENT,
	  AVISO_CF,AVISO_CPP,AVISO_CS1,AVISO_CS2,AVISO_CT);
}

/* redibuja la tabla ASCII */
/* borra el marcador */
dibuja_marcador_ascii(ascii);
dibuja_tabla_ascii(barra[0].bot[4].xb+3,barra[0].bot[4].yb+3);
/* dibuja el marcador */
dibuja_marcador_ascii(ascii);

}

/****************************************************************************
	GRABAR_FUENTE: rutina para grabar un fichero con definiciones de
	  caracteres.
****************************************************************************/
void grabar_fuente(void)
{
char nfich[_MAX_PATH+LNG_NFICH];
int acc, resp;

acc=cuadro_fich(V_CENT,100,COLOR_FONDO,COLOR_PPLAN,COLOR_S1,COLOR_S2,
  COLOR_TECLA,"&Grabar",nfich);

if(acc!=CF_ACCION) return;

/* sale si no se eligi¢ ning£n fichero */
if(!*nfich) return;

/* comprueba si existe y pide confirmaci¢n */
if(!access(nfich,0)) {
	resp=cuadro_siono("\nEse fichero ya existe.\n¨Reemplazarlo?",V_CENT,
	  V_CENT,AVISO_CF,AVISO_CPP,AVISO_CS1,AVISO_CS2,AVISO_CT);
	if(resp==SIONO_NO) return;
}

if(!graba_def(nfich)) {
	cuadro_aviso("\nNo se pudo abrir fichero\no error en escritura.",
	  V_CENT,V_CENT,AVISO_CF,AVISO_CPP,AVISO_CS1,AVISO_CS2,AVISO_CT);
}

}

/****************************************************************************
	PASA_A8x16: pasa al modo de 8x16.
****************************************************************************/
void pasa_a8x16(void)
{

vis_puntero(ESCONDE);

altura_cuad=16;
tabla_ascii=&tabla_ascii8x16[0][0];

cambia_boton(&barra[1],2,INV,NO_ACT);
dibuja_boton(&barra[1].bot[2],barra[1].color_fondo,barra[1].color_pplano,
  barra[1].color_s1,barra[1].color_s2,barra[1].color_tecla);
cambia_boton(&barra[1],3,INV,ACT);
dibuja_boton(&barra[1].bot[3],barra[1].color_fondo,barra[1].color_pplano,
  barra[1].color_s1,barra[1].color_s2,barra[1].color_tecla);

/* borra cuadr¡cula y tabla ASCII */
_setcolor(barra[0].color_fondo);
_rectangle(_GFILLINTERIOR,barra[0].bot[3].xb,barra[0].bot[3].yb,
  barra[0].bot[3].xb+barra[0].bot[3].ancho-1,
  barra[0].bot[3].yb+barra[0].bot[3].alto-1);
_rectangle(_GFILLINTERIOR,barra[0].bot[4].xb,barra[0].bot[4].yb,
  barra[0].bot[4].xb+barra[0].bot[4].ancho-1,
  barra[0].bot[4].yb+barra[0].bot[4].alto-1);

/* cambia altura de cuadr¡cula de dibujo y tabla ascii */
barra[0].bot[3].alto=6+(16*CUAD_ALTO);
barra[0].bot[4].alto=ALT_C8x16;

/* cuadricula zona de dibujo */
dibuja_boton(&barra[0].bot[3],barra[0].color_fondo,barra[0].color_pplano,
  barra[0].color_s1,barra[0].color_s2,barra[0].color_tecla);
dibuja_cuadricula(barra[0].bot[3].xb+3,barra[0].bot[3].yb+3);

/* tabla ASCII */
dibuja_boton(&barra[0].bot[4],barra[0].color_fondo,barra[0].color_pplano,
  barra[0].color_s1,barra[0].color_s2,barra[0].color_tecla);
dibuja_tabla_ascii(barra[0].bot[4].xb+3,barra[0].bot[4].yb+3);

/* dibuja marcador en el car cter actual de la tabla */
dibuja_marcador_ascii(ascii);

/* copia definici¢n de caracter en buffer auxiliar */
copia_def_car(tabla_ascii+(ascii*altura_cuad),caracter);

/* dibuja car cter a tama¤o real */
_setcolor(0);
_rectangle(_GFILLINTERIOR,barra[0].bot[5].xb+3,barra[0].bot[5].yb+3,
  barra[0].bot[5].xb+barra[0].bot[5].ancho-4,
  barra[0].bot[5].yb+barra[0].bot[5].alto-4);
dibuja_caracter();

/* dibuja car cter en cuadr¡cula */
dibuja_en_cuadricula();

vis_puntero(MUESTRA);

}

/****************************************************************************
	PASA_A8x8: pasa al modo de 8x8.
****************************************************************************/
void pasa_a8x8(void)
{

vis_puntero(ESCONDE);

altura_cuad=8;
tabla_ascii=&tabla_ascii8x8[0][0];

cambia_boton(&barra[1],2,INV,ACT);
dibuja_boton(&barra[1].bot[2],barra[1].color_fondo,barra[1].color_pplano,
  barra[1].color_s1,barra[1].color_s2,barra[1].color_tecla);
cambia_boton(&barra[1],3,INV,NO_ACT);
dibuja_boton(&barra[1].bot[3],barra[1].color_fondo,barra[1].color_pplano,
  barra[1].color_s1,barra[1].color_s2,barra[1].color_tecla);

/* borra cuadr¡cula y tabla ASCII */
_setcolor(barra[0].color_fondo);
_rectangle(_GFILLINTERIOR,barra[0].bot[3].xb,barra[0].bot[3].yb,
  barra[0].bot[3].xb+barra[0].bot[3].ancho-1,
  barra[0].bot[3].yb+barra[0].bot[3].alto-1);
_rectangle(_GFILLINTERIOR,barra[0].bot[4].xb,barra[0].bot[4].yb,
  barra[0].bot[4].xb+barra[0].bot[4].ancho-1,
  barra[0].bot[4].yb+barra[0].bot[4].alto-1);

/* cambia altura de cuadr¡cula de dibujo y tabla ascii */
barra[0].bot[3].alto=6+(8*CUAD_ALTO);
barra[0].bot[4].alto=ALT_C8x8;

/* cuadricula zona de dibujo */
dibuja_boton(&barra[0].bot[3],barra[0].color_fondo,barra[0].color_pplano,
  barra[0].color_s1,barra[0].color_s2,barra[0].color_tecla);
dibuja_cuadricula(barra[0].bot[3].xb+3,barra[0].bot[3].yb+3);

/* tabla ASCII */
dibuja_boton(&barra[0].bot[4],barra[0].color_fondo,barra[0].color_pplano,
  barra[0].color_s1,barra[0].color_s2,barra[0].color_tecla);
dibuja_tabla_ascii(barra[0].bot[4].xb+3,barra[0].bot[4].yb+3);

/* dibuja marcador en el car cter actual de la tabla */
dibuja_marcador_ascii(ascii);

/* copia definici¢n de caracter en buffer auxiliar */
copia_def_car(tabla_ascii+(ascii*altura_cuad),caracter);

/* dibuja car cter a tama¤o real */
_setcolor(0);
_rectangle(_GFILLINTERIOR,barra[0].bot[5].xb+3,barra[0].bot[5].yb+3,
  barra[0].bot[5].xb+barra[0].bot[5].ancho-4,
  barra[0].bot[5].yb+barra[0].bot[5].alto-4);
dibuja_caracter();

/* dibuja car cter en cuadr¡cula */
dibuja_en_cuadricula();

vis_puntero(MUESTRA);

}

/****************************************************************************
	PRUEBA_CAR: abre una ventana para comprobar juego de caracteres.
****************************************************************************/
void prueba_car(void)
{
STC_VENTANA v;
BYTE espacio[16];
unsigned tecla;
short xcar;
int i, ncar=0;

/* crea una definici¢n de espacio */
for(i=0; i<16; i++) espacio[i]=0;

vis_puntero(ESCONDE);

v.x=100;
v.y=100;
v.ancho=(NPRB_CAR*8)+22;
v.alto=38+altura_cuad;
v.color_fondo=COLOR_FONDO;
v.color_pplano=COLOR_PPLAN;
v.color_s1=COLOR_S1;
v.color_s2=COLOR_S2;

abre_ventana(&v);

_moveto(v.x+((v.ancho-160)/2),v.y+4);
imp_str("Prueba de caracteres",v.color_fondo,v.color_pplano,CHR_NORM);
_moveto(v.x+((v.ancho-240)/2),v.y+altura_cuad+18);
imp_str("Teclea los caracteres y cuando",v.color_fondo,v.color_pplano,
  CHR_NORM);
_moveto(v.x+((v.ancho-176)/2),v.y+altura_cuad+26);
imp_str("termines pulsa RETURN.",v.color_fondo,v.color_pplano,
  CHR_NORM);

/* zona de prueba */
_setcolor(COLOR_F_PRB);
_rectangle(_GFILLINTERIOR,v.x+9,v.y+13,v.x+(NPRB_CAR*8)+11,v.y+altura_cuad+14);

xcar=v.x+11;

do {
	/* imprime cursor */
	_setcolor(COLOR_P_PRB);
	_moveto(xcar,v.y+altura_cuad+13);
	_lineto(xcar+7,v.y+altura_cuad+13);

	tecla=_bios_keybrd(_KEYBRD_READ);
	/* coge c¢digo ASCII de tecla pulsada */
	tecla &= 0x00ff;

	/* si pulsa borrado */
	if(tecla==0x08) {
		/* retrocede una posici¢n si no est  al principio */
		/* e imprime un espacio */
		if(ncar) {
			xcar-=8;
			ncar--;
			/* borra car cter y cursor */
			_moveto(xcar,v.y+14);
			imp_chrdef(&espacio[0],COLOR_F_PRB,COLOR_P_PRB,
			  altura_cuad,C_NORM);
			_moveto(xcar+8,v.y+14);
			imp_chrdef(&espacio[0],COLOR_F_PRB,COLOR_P_PRB,
			  altura_cuad,C_NORM);
		}
	}
	else if((ncar<(NPRB_CAR-1)) && (tecla!=0x0d)) {
		/* imprime car cter */
		_moveto(xcar,v.y+14);
		imp_chrdef(tabla_ascii+(tecla*altura_cuad),COLOR_F_PRB,
		  COLOR_P_PRB,altura_cuad,C_NORM);
		ncar++;
		/* siguiente posici¢n */
		xcar+=8;
	}
} while(tecla!=0x0d);

cierra_ventana(&v);

vis_puntero(MUESTRA);

}

/****************************************************************************
	IMP_ASCII_CAR: imprime el c¢digo ASCII del car cter actual.
****************************************************************************/
void imp_ascii_car(void)
{
char cascii[4];

sprintf(cascii,"%03d",ascii);
escribe_texto_boton(cascii,&barra[0].bot[6],barra[0].color_pplano,
  barra[0].color_tecla,barra[0].color_fondo);

}

/****************************************************************************
	TABLACAR_MODIFICADA: comprueba si la tabla de caracteres fue
	  modificada y si es as¡ pregunta si quiere grabarla.
****************************************************************************/
void tablacar_modificada(void)
{
int i;

if(car_modificado) {
	i=cuadro_siono("\nJuego de caracteres ha sido modificado\n"
	  "¨Grabarlo?",V_CENT,V_CENT,AVISO_CF,AVISO_CPP,AVISO_CS1,AVISO_CS2,
	  AVISO_CT);
	if(i==SIONO_SI) grabar_fuente();
}

car_modificado=0;

}
