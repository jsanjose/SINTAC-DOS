/****************************************************************************
				   CHRIMP.C

	Conjunto de funciones para imprimir textos en los modos gr ficos
	en cualquier posici¢n de la pantalla.

			    (c)1993 JSJ Soft Ltd.

	Las siguientes funciones son p£blicas:
		- imp_chr: imprime un car cter en una posici¢n de pantalla
		    dada
		- imp_str: imprime una cadena de caracteres
		- impn_str: imprime un n£mero dado de caracteres de una
		    cadena
****************************************************************************/

#include <graph.h>
#include "chrimp.h"
#include "chrdef.h"

/****************************************************************************
	IMP_CHR: imprime un car cter en la posici¢n actual de pantalla; la
	  posici¢n actual es la £ltima definida por la funci¢n _moveto(), o
	  por alguna otra funci¢n gr fica.
	  Entrada:      'chr' car cter a imprimir
			'colorf', 'color' colores de fondo y primer plano
			'modo' modo de dibujo (CHR_NORM, CHR_XOR, CHR_OR,
			CHR_AND)
	  Salida:       posici¢n de dibujo actualizada

	  NOTA: esta rutina s¢lo sirve para los modos de 16 colores de alta
	    resoluci¢n de EGA y VGA y para el modo de 256 colores de VGA
****************************************************************************/
void imp_chr(int chr, short colorf, short color, unsigned char modo)
{
unsigned char _far *video;
unsigned char _far *pchr;
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
/* modo de 256 colores de VGA */
else if(vc.mode==_MRES256COLOR)
  video=(unsigned char _far *)0xa0000000+(x+(y*320));
else return;

/* direcci¢n de la definici¢n del car cter */
pchr=(unsigned char _far *)(&_defchr8x8[chr & 0x00ff][0]);

if(vc.mode==_MRES256COLOR) {
	_asm {
		push di
		push si
		push ds

		les di,video            ; ES:DI = dir. en buffer de v¡deo
		lds si,pchr             ; DS:SI = dir. definici¢n car cter
		mov cx,8                ; CX = altura en pixels del car cter

	ic256_1:
		push cx
		mov cx,8                ; CX = anchura car cter en pixels
		lodsb                   ; AL = byte de car cter
		mov ah,al               ; AH = patr¢n de bit para sgte. fila
		mov bl,BYTE PTR color   ; BL = valor pixel de imagen
		mov bh,BYTE PTR colorf  ; BH = valor pixel de fondo

	ic256_2:
		mov al,bl               ; AL = valor de pixel imagen
		shl ah,1                ; CARRY = bit alto
		jc ic_xor               ; salta si es pixel de imagen
		mov al,bh               ; AL = valor de pixel fondo

	ic_xor:
		cmp modo,CHR_XOR
		jne ic_or
		xor es:[di],al
		inc di
		loop ic256_2
		jmp sgte_linea
	ic_or:
		cmp modo,CHR_OR
		jne ic_and
		or es:[di],al
		inc di
		loop ic256_2
		jmp sgte_linea
	ic_and:
		cmp modo,CHR_AND
		jne ic_norm
		and es:[di],al
		inc di
		loop ic256_2
		jmp sgte_linea
	ic_norm:
		stosb                   ; actualiza pixel en buffer (NORM)
		loop ic256_2
	sgte_linea:
		add di,312              ; siguiente l¡nea en pantalla
		pop cx
		loop ic256_1

		pop ds
		pop si
		pop di
	}
}
else {
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
		mov cx,8                ; CX = altura en pixels del car cter

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
}

/* actualiza posici¢n de dibujo */
/* actualiza coordenada X */
x+=8;
if(x>maxx) {
	x=0;
	y+=8;
}
/* actualiza coordenada Y, si se sale de pantalla pasa a (0,0) */
if(y>(maxy-7)) {
	x=0;
	y=0;
}

_moveto(x,y);

}

/****************************************************************************
	IMP_STR: imprime una cadena de caracteres en la posici¢n actual de
	  pantalla; la posici¢n actual es la £ltima definida con la funci¢n
	  _moveto(), o alguna otra funci¢n gr fica.
	  Entrada:      'cadena' puntero a cadena a imprimir
			'colorf', 'color' colores de fondo y primer plano
			'modo' modo de impresi¢n
****************************************************************************/
void imp_str(char *cadena, short colorf, short color, unsigned char modo)
{

while(*cadena) {
	imp_chr(*cadena,colorf,color,modo);
	/* siguiente car cter */
	cadena++;
}

}

/****************************************************************************
	IMPN_STR: imprime una cadena de caracteres en la posici¢n actual de
	  pantalla; la posici¢n actual es la £ltima definida con la funci¢n
	  _moveto(), o alguna otra funci¢n gr fica.
	  Entrada:      'cadena' puntero a cadena a imprimir
			'numcar' n£mero de caracteres a imprimir
			'colorf', 'color' colores de fondo y primer plano
			'modo' modo de impresi¢n
****************************************************************************/
void impn_str(char *cadena, int numcar, short colorf, short color,
  unsigned char modo)
{
int i;

for(i=0; i<numcar; i++) {
	imp_chr(*cadena,colorf,color,modo);
	/* siguiente car cter */
	cadena++;
}

}

