/****************************************************************************
				   IMPCHR.C

	Conjunto de funciones para imprimir textos en los modos gr ficos de
	16 colores (640x480) y de 256 colores (320x200) de VGA.

			    (c)1993 JSJ Soft Ltd.

	Las siguientes funciones son p£blicas:
		- imp_chr_pos: coloca posici¢n de impresi¢n
		- imp_chr: imprime un car cter en una posici¢n de pantalla
		  dada
		- def_chr_set: actualiza punteros a un nuevo 'set' de
		  caracteres
****************************************************************************/

#include "impchr.h"
#include "defchr.h"

/*** Variables globales internas ***/
/* punteros a tablas con las definiciones de caracteres de 8xCHR_ALT y 8x8 */
static unsigned char *_defchr=&_defchr_sys[0][0];
static unsigned char *_defchr8x8=&_defchr8x8_sys[0][0];

/* £ltimas coordenadas de impresi¢n */
static short ult_x=0;
static short ult_y=0;

/****************************************************************************
	IMP_CHR_POS: coloca la posici¢n de impresi¢n de textos.
	  Entrada:      'x', 'y' posici¢n de impresi¢n
	  Salida:       posici¢n de dibujo actualizada
****************************************************************************/
void imp_chr_pos(short x, short y)
{

ult_x=x;
ult_y=y;

}

/****************************************************************************
	IMP_CHR: imprime un car cter en una posici¢n de pantalla.
	  Entrada:      'chr' car cter a imprimir
			'colorf', 'color' colores de fondo y primer plano
			'modo' modo de dibujo (CHR_NORM, CHR_XOR, CHR_OR,
			CHR_AND)
	  Salida:       posici¢n de impresi¢n actualizada
****************************************************************************/
void imp_chr(int chr, short colorf, short color, unsigned char modo)
{
unsigned char far *video;
unsigned char far *pchr;
int modovideo;
short x, y, maxx, maxy, alt;
short varshift;

/* coordenadas de impresi¢n */
x=ult_x;
y=ult_y;

/* calcula las m ximas coordenadas seg£n modo de v¡deo */
asm {
	mov ah,0fh
        int 10h		// AH=n§ de columnas, AL=modo v¡deo
        mov ah,0
        mov modovideo,ax
}

/* modo de 16 colores */
if(modovideo==0x12) {
	maxx=639;
	maxy=479;
}
/* modo de 256 colores */
else if(modovideo==0x13) {
     	maxx=319;
        maxy=199;
}
else return;

/* calcula direcci¢n en buffer de v¡deo */
/* y la direcci¢n de la definici¢n del car cter a usar */
/* modo de 16 colores de alta resoluci¢n de VGA */
if(modovideo==0x12) {
	video=(unsigned char far *)0xa0000000L+((x/8)+(y*80));
	pchr=_defchr+((chr & 0x00ff)*16);
	alt=CHR_ALT;
}
/* modo de 256 colores de VGA */
else if(modovideo==0x13) {
	video=(unsigned char far *)0xa0000000L+(x+(y*320));
	pchr=_defchr8x8+((chr & 0x00ff)*8);
	alt=8;
}
else return;

if(modovideo==0x13) {
	asm {
		push di
		push si
		push ds

		les di,video            // ES:DI = dir. en buffer de v¡deo
		lds si,pchr             // DS:SI = dir. definici¢n car cter
		mov cx,alt              // CX = altura en pixels del car cter
        }
	ic256_1:
        asm {
		push cx
		mov cx,8                // CX = anchura car cter en pixels
		lodsb                   // AL = byte de car cter
		mov ah,al               // AH = patr¢n de bit para sgte. fila
		mov bl,BYTE PTR color   // BL = valor pixel de imagen
		mov bh,BYTE PTR colorf  // BH = valor pixel de fondo
        }
	ic256_2:
        asm {
		mov al,bl               // AL = valor de pixel imagen
		shl ah,1                // CARRY = bit alto
		jc ic_xor               // salta si es pixel de imagen
		mov al,bh               // AL = valor de pixel fondo
	}
	ic_xor:
        asm {
		cmp modo,CHR_XOR
		jne ic_or
		xor es:[di],al
		inc di
		loop ic256_2
		jmp sgte_linea
        }
	ic_or:
        asm {
		cmp modo,CHR_OR
		jne ic_and
		or es:[di],al
		inc di
		loop ic256_2
		jmp sgte_linea
        }
	ic_and:
        asm {
		cmp modo,CHR_AND
		jne ic_norm
		and es:[di],al
		inc di
		loop ic256_2
		jmp sgte_linea
        }
	ic_norm:
        asm {
		stosb                   // actualiza pixel en buffer (NORM)
		loop ic256_2
        }
	sgte_linea:
        asm {
		add di,312              // siguiente l¡nea en pantalla
		pop cx
		loop ic256_1

		pop ds
		pop si
		pop di
	}
}
else {
	asm {
		push di
		push si
		push ds

		mov cx,x                // CL = byte bajo de coordenada X
		and cl,7
		xor cl,7                // CL = n§ de bits a desplazar izq.
		inc cx
		and cl,7                // CL = n§ bits a despl. para enmascarar
		mov ch,0ffh
		shl ch,cl               // CH = m scara lado derecho de car cter
		mov varshift,cx

		les di,pchr             // ES:DI = dir. definici¢n de car cter
		lds si,video            // DS:SI = dir. en buffer de v¡deo
		mov cx,alt              // CX = altura en pixels del car cter

		// establecer registros del controlador de gr ficos

		mov dx,3ceh             // puerto del registro de direcciones
		mov ax,0a05h            // modo escritura 2, modo lectura 1
		out dx,ax
		mov ah,modo             // modo de dibujo: 18h = XOR, 10h = OR,
					// 08h = AND, 00h = NORM
		mov al,3
		out dx,ax
		mov ax,0007h            // color "don't care" todos los planos
		out dx,ax

		mov bl,BYTE PTR color   // BL = valor de pixel imagen
		mov bh,BYTE PTR colorf  // BH = valor de pixel fondo

		cmp BYTE PTR varshift,0
		jne chr_no_alin         // salta si car cter no alineado

		// rutina para caracteres alineados

		mov al,8                // AL = n§ registro de m scara de bit
	}
	chr_alin:
        asm {
		mov ah,es:[di]          // AH = patr¢n de fila de pixels
		out dx,ax               // lo carga en registro m scara de bit
		and [si],bl             // actualiza pixels de imagen
		not ah
		out dx,ax
		and [si],bh             // actualiza pixels de fondo

		inc di                  // siguiente byte del car cter
		add si,80               // siguiente l¡nea de pantalla

		loop chr_alin
		jmp chr_fin

		// rutina para caracteres no alineados
        }
	chr_no_alin:
        asm {
		push cx                 // guarda contador n§ de l¡neas de car.
		mov cx,varshift         // CH = m scara lado derecho
					// CL = n§ de bits a desplazar
		// dibuja lado izquierdo del car cter
		mov al,es:[di]          // AL = patr¢n de fila de pixels
		xor ah,ah
		shl ax,cl               // AH = patr¢n para lado izquierdo
					// AL = patr¢n para lado derecho
		push ax                 // guarda patrones
		mov al,8                // n§ de registro de m scara de bit
		out dx,ax               // carga patr¢n de lado izquierdo
		and [si],bl             // actualiza pixels de imagen
		not ch                  // CH = m scara para lado izquierdo
		xor ah,ch
		out dx,ax
		and [si],bh             // actualiza pixels de fondo
		// dibuja lado derecho del car cter
		pop ax                  // recupera patrones
		mov ah,al               // AH = patr¢n lado derecho
		mov al,8                // AL = n§ registro de m scara de bit
		out dx,ax               // carga patr¢n
		inc si                  // posici¢n en buffer de v¡deo
		and [si],bl             // actualiza pixels de imagen
		not ch                  // CH = m scara para lado derecho
		xor ah,ch
		out dx,ax
		and [si],bh             // actualiza pixels de fondo

		inc di                  // siguiente byte del car cter
		dec si
		add si,80               // siguiente l¡nea de pantalla

		pop cx
		loop chr_no_alin

		// restaura registros de controlador a sus valores por defecto
        }
	chr_fin:
        asm {
		mov ax,0ff08h           // m scara de bit
		out dx,ax
		mov ax,0005h            // registro de modo
		out dx,ax
		mov ax,0003h            // rotar dato/selecc. funci¢n
		out dx,ax
		mov ax,0f07h            // color "don't care"
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
	y+=alt;
}
/* actualiza coordenada Y, si se sale de pantalla pasa a (0,0) */
if(y>(maxy-(alt-1))) {
	x=0;
	y=0;
}

imp_chr_pos(x,y);

}

/****************************************************************************
	DEF_CHR_SET: actualiza los punteros a las definiciones de los
	  juegos de caracteres.
	  Entrada:      'ptr_set1' puntero a juego de caracteres de
			8xCHR_ALT; si es 0 (NULL), se apuntar  al juego de
			caracteres por defecto
			'ptr_set2' puntero a juego de caracteres de 8x8, si
			es 0 (NULL), se apuntar  al juego de caracteres po
			defecto
****************************************************************************/
void def_chr_set(unsigned char far *ptr_set1, unsigned char far *ptr_set2)
{

/* actualiza puntero a juego de caracteres de 8xCHR_ALT */
if(ptr_set1==(unsigned char far *)0) _defchr=&_defchr_sys[0][0];
else _defchr=ptr_set1;

/* actualiza puntero a juego de caracteres de 8x8 */
if(ptr_set2==(unsigned char far *)0) _defchr8x8=&_defchr8x8_sys[0][0];
else _defchr8x8=ptr_set2;

}
