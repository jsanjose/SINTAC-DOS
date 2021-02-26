/****************************************************************************
				   CHRIMP.C

	Conjunto de funciones para imprimir textos en los modos gr ficos
	de 16 colores en cualquier posici¢n de la pantalla.

			    (c)1993 JSJ Soft Ltd.

	Las siguientes funciones son p£blicas:
		- imp_chr: imprime un car cter en una posici¢n de pantalla
		    dada
		- imp_cad: imprime una cadena de caracteres
		- impn_cad: imprime un n£mero dado de caracteres de una
		    cadena
****************************************************************************/

#include <graphics.h>
#include "chrimp.h"
#include "chrdef.h"

/****************************************************************************
	IMP_CHR: imprime un car cter en la posici¢n actual de pantalla; la
	  posici¢n actual es la £ltima definida por la funci¢n moveto(), o
	  por alguna otra funci¢n gr fica.
	  Entrada:      'chr' car cter a imprimir
			'colorf', 'color' colores de fondo y primer plano
			'modo' modo de dibujo (CHR_NORM, CHR_XOR, CHR_OR,
			CHR_AND)
	  Salida:       posici¢n de dibujo actualizada
****************************************************************************/
void imp_chr(int chr, short colorf, short color, unsigned char modo)
{
unsigned char far *video;
unsigned char far *pchr;
int x, y, maxx, maxy, modog;
short varshift;

/* coge la posici¢n de dibujo actual y m ximas coordenadas */
x=getx();
y=gety();
maxx=getmaxx();
maxy=getmaxy();

/* calcula direcci¢n en buffer de v¡deo */
video=(unsigned char far *)0xa0000000L+((x/8)+(y*80));

/* direcci¢n de la definici¢n del car cter */
pchr=(unsigned char far *)(&_defchr8x8[chr & 0x00ff][0]);

modog=getgraphmode();

if((modog==EGAHI) || (modog==VGAHI)) {
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
		shl ch,cl               // CH = m scara lado derecho car cter
		mov varshift,cx

		les di,pchr             // ES:DI = dir. definici¢n de car cter
		lds si,video            // DS:SI = dir. en buffer de v¡deo
		mov cx,8                // CX = altura en pixels del car cter

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
	}
	chr_no_alin:
	asm {
		// rutina para caracteres no alineados

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
	}
	chr_fin:
	asm {
		// restaura registros de controlador a sus valores por defecto

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
else return;

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

moveto(x,y);

}

/****************************************************************************
	IMP_CAD: imprime una cadena de caracteres en la posici¢n actual de
	  pantalla; la posici¢n actual es la £ltima definida con la funci¢n
	  moveto(), o alguna otra funci¢n gr fica.
	  Entrada:      'cadena' puntero a cadena a imprimir
			'colorf', 'color' colores de fondo y primer plano
			'modo' modo de impresi¢n
****************************************************************************/
void imp_cad(char *cadena, short colorf, short color, unsigned char modo)
{

while(*cadena) {
	imp_chr(*cadena,colorf,color,modo);
	/* siguiente car cter */
	cadena++;
}

}

/****************************************************************************
	IMPN_CAD: imprime una cadena de caracteres en la posici¢n actual de
	  pantalla; la posici¢n actual es la £ltima definida con la funci¢n
	  moveto(), o alguna otra funci¢n gr fica.
	  Entrada:      'cadena' puntero a cadena a imprimir
			'numcar' n£mero de caracteres a imprimir
			'colorf', 'color' colores de fondo y primer plano
			'modo' modo de impresi¢n
****************************************************************************/
void impn_cad(char *cadena, int numcar, short colorf, short color,
  unsigned char modo)
{
int i;

for(i=0; i<numcar; i++) {
	imp_chr(*cadena,colorf,color,modo);
	/* siguiente car cter */
	cadena++;
}

}

