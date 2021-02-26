/*

	Librer¡a gr fica SINTAC

	(c) 1995 Javier San Jos‚ y Jos‚ Luis Cebri n
								*/

/* G_MM_256C (modo BIOS 13h) */

#include "grf.h"

extern int g_bytesfila ;

extern unsigned char *_anchchr8x8 ;
extern unsigned char *_defchr8x8 ;

/* calcula direcci¢n de pixel en buffer de v¡deo en el modo 320x200x256 */
/* a la entrada: AX=coordenada Y, BX=coordenada X */
/* a la salida: ES:BX=direcci¢n en buffer de v¡deo */
#define DIR_PIXEL256	asm {			 \
				mov dx, g_bytesfila ;\
				mul dx          ;\
				add bx,ax       ;\
				mov ax,0a000h	;\
				mov es,ax  	;\
			}

/****************************************************************************
	G_BORRA_PANTALLA: borra la pantalla.
****************************************************************************/
void g_borra_pantalla256(void)
{
int filas_pixels;

asm {
	push di

	mov di,0a000h
	mov es,di
	xor di,di	// ES:DI=direcci¢n inicio buffer de v¡deo

	xor ax,ax	// AX=0 (borrar buffer)

	mov cx,32000	// bytes de buffer de v¡deo/2
	rep stosw

	pop di
}

}

/****************************************************************************
	G_COGE_PIXEL: devuelve el color de un pixel.
	  Entrada:	'x', 'y' coordenadas del pixel
	  Salida:	color del pixel
****************************************************************************/
unsigned char g_coge_pixel256(int x, int y)
{
unsigned char color;

asm {
	mov ax,y
	mov bx,x
}
	DIR_PIXEL256;	// ES:BX=direcci¢n en buffer de v¡deo
asm {
	mov al,es:[bx]
	mov color,al
}
return color ;
}

/****************************************************************************
	G_PUNTO256: dibuja un punto en el modo de 256 colores.
	  Entrada:	'x', 'y' coordenadas
			'color' color del punto
			'modo' modo de dibujo (G_NORM, G_XOR, G_OR, G_AND)
****************************************************************************/
void g_punto256(int x, int y, unsigned char color, unsigned char modo)
{

asm {
	mov ax,y
	mov bx,x
}
	DIR_PIXEL256;	// ES:BX=direcci¢n en buffer de v¡deo
asm {
	mov al,color

	cmp modo,G_NORM
	jne punto256_and

	mov es:[bx],al
}
	return;
punto256_and:
asm {
	cmp modo,G_AND
	jne punto256_or

	and es:[bx],al
}
	return;
punto256_or:
asm {
	cmp modo,G_OR
	jne punto256_xor

	or es:[bx],al
}
	return;
punto256_xor:
asm {
	xor es:[bx],al
}

}

/****************************************************************************
	G_LINEA256: dibuja una l¡nea en el modo de 256 colores.
	  Entrada:	'x0', 'y0' punto de origen
			'x1', 'y1' punto final
			'color' color de la l¡nea
			'modo' modo de dibujo (G_NORM, G_XOR, G_OR, G_AND)
****************************************************************************/
void g_linea256(int x0, int y0, int x1, int y1, unsigned char color,
  unsigned char modo)
{
int incr1, incr2, rutina;
int bytesfila = g_bytesfila ;

asm {
	push si
	push di
	push ds

	mov si,bytesfila	// incremento inicial de y
	mov cx,x1
	sub cx,x0      	// CX=x1-x0
	jnz lin256_00
	jmp linea_vert256	// salta si es una l¡nea vertical
}
lin256_00:
asm {
	jns lin256_01  	// salta si x1>x0

	neg cx          // CX=x0-x1
	mov bx,x1
	xchg bx,x0
	mov x1,bx	// intercambia x0 y x1
	mov bx,y1
	xchg bx,y0
	mov y1,bx	// intercambia y0 e y1
}
lin256_01:
asm {
	mov bx,y1
	sub bx,y0      	// BX=y1-y0
	jnz lin256_02
	jmp linea_horz256	// salta si es una l¡nea horizontal
}
lin256_02:
asm {
	jns lin256_03   // salta si la pendiente es positiva

	neg bx          // BX=y0-y1
	neg si          // niega el incremento de y
}
lin256_03:
asm {
	push si
	mov rutina,0
	cmp bx,cx
	jle lin256_04  	// salta si dy<=dx (pendiente<=1)
	mov rutina,1
	xchg bx,cx      // intercambia dy y dx
}
lin256_04:
asm {
	shl bx,1        // BX=2*dy
	mov incr1,bx    // incr1=2*dy
	sub bx,cx
	mov si,bx       // SI=d=2*dy-dx
	sub bx,cx
	mov incr2,bx    // incr2=2*(dy-dx)

	push cx
	mov ax,y0
	mov bx,x0
}
	DIR_PIXEL256;	// ES:BX=direcci¢n en buffer de v¡deo
asm {
	mov di,bx       // ES:DI=direcci¢n en buffer de v¡deo
	pop cx
	inc cx          // CX=cantidad de pixels a dibujar

	pop bx          // BX=incremento de y
	cmp rutina,1
	je pendiente1
	jmp lin256_07
}
pendiente1:
asm {
	jmp lin256_10
}
linea_vert256:
asm {
	mov ax,y0
	mov bx,y1
	mov cx,bx
	sub cx,ax    	// CX=dy
	jge lin256_05   // salta si dy>=0

	neg cx          // fuerza dy>=0
	mov ax,bx
}
lin256_05:
asm {
	inc cx          // CX=cantidad de pixels a dibujar
	mov bx,x0
	push cx
}
	DIR_PIXEL256;	// ES:BX=direcci¢n en buffer de v¡deo
asm {
	pop cx
	mov di,bx	// ES:DI=direcci¢n en buffer de v¡deo
	dec si          // SI=bytes/l¡nea-1
	mov al,color    // AL=valor de pixel
}
lin256_06:
asm {
	cmp modo,G_NORM
	jne lin256_06and

	stosb

	add di,si       // siguiente l¡nea
	dec cx ; jnz lin256_06
	jmp fin256
}
lin256_06and:
asm {
	cmp modo,G_AND
	jne lin256_06or
	and es:[di],al
	inc di
	add di,si       // siguiente l¡nea
	dec cx ; jnz lin256_06and
	jmp fin256
}
lin256_06or:
asm {
	cmp modo,G_OR
	jne lin256_06xor
	or es:[di],al
	inc di
	add di,si       // siguiente l¡nea
	dec cx ; jnz lin256_06or
	jmp fin256
}
lin256_06xor:
asm {
	xor es:[di],al
	inc di
	add di,si       // siguiente l¡nea
	dec cx ; jnz lin256_06xor
	jmp fin256
}
linea_horz256:
asm {
	push cx
	mov ax,y0
	mov bx,x0
}
	DIR_PIXEL256;	// ES:BX=direcci¢n en buffer de v¡deo
asm {
	mov di,bx       // ES:DI=direcci¢n en buffer de v¡deo
	pop cx
	inc cx          // CX=cantidad de pixels a dibujar
	mov al,color    // AL=valor de pixel

	cmp modo,G_NORM
	jne linea_horz256and

	rep stosb

	jmp fin256
}
linea_horz256and:
asm {
	cmp modo,G_AND
	jne linea_horz256or
}
linea_horz256andr:
asm {
	and es:[di],al
	inc di
	dec cx ; jnz linea_horz256andr
	jmp fin256
}
linea_horz256or:
asm {
	cmp modo,G_OR
	jne linea_horz256xor
}
linea_horz256orr:
asm {
	or es:[di],al
	inc di
	dec cx ; jnz linea_horz256orr
	jmp fin256
}
linea_horz256xor:
asm {
	xor es:[di],al
	inc di
	dec cx ; jnz linea_horz256xor
	jmp fin256
}
lin256_07:
asm {
	mov al,color 	// AL=valor de pixel

	cmp modo,G_NORM
	je lin256_08
	jmp lin256_07and
}
lin256_08:
asm {
	stosb

	or si,si        // comprueba el signo de d
	jns lin256_09   // salta si d>=0

	add si,incr1    // d=d+incr1
	dec cx ; jnz lin256_08
	jmp fin256
}
lin256_09:
asm {
	add si,incr2    // d=d+incr2
	add di,bx       // incrementa y
	dec cx ; jnz lin256_08
	jmp fin256
}
lin256_07and:
asm {
	cmp modo,G_AND
	je lin256_07and1
	jmp lin256_07or
}
lin256_07and1:
asm {
	and es:[di],al
	inc di

	or si,si        // comprueba el signo de d
	jns lin256_07and2	// salta si d>=0

	add si,incr1    // d=d+incr1
	dec cx ; jnz lin256_07and1
	jmp fin256
}
lin256_07and2:
asm {
	add si,incr2    // d=d+incr2
	add di,bx       // incrementa y
	dec cx ; jnz lin256_07and1
	jmp fin256
}
lin256_07or:
asm {
	cmp modo,G_OR
	je lin256_07or1
	jmp lin256_07xor
}
lin256_07or1:
asm {
	or es:[di],al
	inc di

	or si,si        // comprueba el signo de d
	jns lin256_07or2	// salta si d>=0

	add si,incr1    // d=d+incr1
	dec cx ; jnz lin256_07or1
	jmp fin256
}
lin256_07or2:
asm {
	add si,incr2    // d=d+incr2
	add di,bx       // incrementa y
	dec cx ; jnz lin256_07or1
	jmp fin256
}
lin256_07xor:
asm {
	xor es:[di],al
	inc di

	or si,si        // comprueba el signo de d
	jns lin256_07xor2	// salta si d>=0

	add si,incr1    // d=d+incr1
	dec cx ; jnz lin256_07xor
	jmp fin256
}
lin256_07xor2:
asm {
	add si,incr2    // d=d+incr2
	add di,bx       // incrementa y
	dec cx ; jnz lin256_07xor
	jmp fin256
}
lin256_10:
asm {
	mov al,color    // AL=valor de pixel

	cmp modo,G_NORM
	je lin256_11
	jmp lin256_10and
}
lin256_11:
asm {
	stosb

	add di,bx       // incrementa y
	or si,si        // comprueba el signo de d
	jns lin256_12   // salta si d>=0
	add si,incr1    // d=d+incr1
	dec di          // decrementa x (ya incrementado)
	dec cx ; jnz lin256_11
	jmp fin256
}
lin256_12:
asm {
	add si,incr2    // d=d+incr2
	dec cx ; jnz lin256_11
	jmp fin256
}
lin256_10and:
asm {
	cmp modo,G_AND
	je lin256_10and1
	jmp lin256_10or
}
lin256_10and1:
asm {
	and es:[di],al
	inc di

	add di,bx       // incrementa y
	or si,si        // comprueba el signo de d
	jns lin256_10and2	// salta si d>=0
	add si,incr1    // d=d+incr1
	dec di          // decrementa x (ya incrementado)
	dec cx ; jnz lin256_10and1
	jmp fin256
}
lin256_10and2:
asm {
	add si,incr2    // d=d+incr2
	dec cx ; jnz lin256_10and1
	jmp fin256
}
lin256_10or:
asm {
	cmp modo,G_OR
	je lin256_10or1
	jmp lin256_10xor
}
lin256_10or1:
asm {
	or es:[di],al
	inc di

	add di,bx       // incrementa y
	or si,si        // comprueba el signo de d
	jns lin256_10or2	// salta si d>=0
	add si,incr1    // d=d+incr1
	dec di          // decrementa x (ya incrementado)
	dec cx ; jnz lin256_10or1
	jmp fin256
}
lin256_10or2:
asm {
	add si,incr2    // d=d+incr2
	dec cx ; jnz lin256_10or1
	jmp fin256
}
lin256_10xor:
asm {
	xor es:[di],al
	inc di

	add di,bx       // incrementa y
	or si,si        // comprueba el signo de d
	jns lin256_10xor2	// salta si d>=0
	add si,incr1    // d=d+incr1
	dec di          // decrementa x (ya incrementado)
	dec cx ; jnz lin256_10xor
	jmp fin256
}
lin256_10xor2:
asm {
	add si,incr2    // d=d+incr2
	dec cx ; jnz lin256_10xor
}
fin256:
asm {
	pop ds
	pop di
	pop si
}

}

/****************************************************************************
	G_RECTANGULO256: dibuja un rect ngulo s¢lido en el modo de 256
	  colores.
	  Entrada:	'x0', 'y0' esquina superior izquierda
			'x1', 'y1' esquina inferior derecha
			'color' color del rect ngulo
			'modo' modo de dibujo (G_NORM, G_XOR, G_OR, G_AND)
****************************************************************************/
void g_rectangulo256(int x0, int y0, int x1, int y1, unsigned char color,
  unsigned char modo)
{
int bytesfila = g_bytesfila ;

asm {
	push si
	push di
	push ds

	mov ax,y0
	mov bx,x0
}
	DIR_PIXEL256;	// ES:BX=direcci¢n en buffer de v¡deo
asm {
	mov di,bx       // ES:DI=direcci¢n en buffer de v¡deo

	mov cx,x1
	sub cx,x0
	inc cx          // CX=cantidad de pixels a dibujar

	mov bx,y1
	sub bx,y0
	inc bx		// BX=n£mero de l¡neas

	mov al,color    // AL=valor de pixel

	cmp modo,G_NORM
	jne rect256and
}
rect256norm:
asm {
	push di
	push cx

	rep stosb

	pop cx
	pop di
	add di,bytesfila	// siguiente l¡nea

	dec bx
	jnz rect256norm

	jmp finrect256
}
rect256and:
asm {
	cmp modo,G_AND
	jne rect256or
}
rect256andr0:
asm {
	push di
	push cx
}
rect256andr1:
asm {
	and es:[di],al
	inc di
	dec cx ; jnz rect256andr1

	pop cx
	pop di
	add di,bytesfila	// siguiente l¡nea

	dec bx
	jnz rect256andr0

	jmp finrect256
}
rect256or:
asm {
	cmp modo,G_OR
	jne rect256xorr0
}
rect256orr0:
asm {
	push di
	push cx
}
rect256orr1:
asm {
	or es:[di],al
	inc di
	dec cx ; jnz rect256orr1

	pop cx
	pop di
	add di,bytesfila	// siguiente l¡nea

	dec bx
	jnz rect256orr0

	jmp finrect256
}
rect256xorr0:
asm {
	push di
	push cx
}
rect256xorr1:
asm {
	xor es:[di],al
	inc di
	dec cx ; jnz rect256xorr1

	pop cx
	pop di
	add di,bytesfila	// siguiente l¡nea

	dec bx
	jnz rect256xorr0
}
finrect256:
asm {
	pop ds
	pop di
	pop si
}

}

/****************************************************************************
	G_SCROLL_ARR: realiza scroll hacia arriba de una ventana.
	  Entrada:      'fila', 'columna' inicio de la ventana
			'ancho', 'alto' dimensiones de la ventana
			'color' color de relleno
****************************************************************************/
void g_scroll_arr256(int fila, int columna, int ancho, int alto,
  unsigned char color)
{
unsigned char far *pvideo;
unsigned char far *pvid2;
int bytesfila = g_bytesfila ;

/* direcci¢n en buffer de v¡deo de zona a desplazar */
pvideo=(unsigned char far *)0xa0000000L+((fila*bytesfila*8)+(columna*8));
/* puntero auxiliar a l¡nea siguiente */
pvid2=pvideo+(bytesfila*8);
ancho*=8;
alto=(alto-1)*8;

asm {
	push di
	push si
	push ds

	les di,pvideo           // direcci¢n de inicio
	lds si,pvid2            // puntero a l¡nea siguiente

	mov cx,alto             // n£mero de l¡neas a desplazar
}
scr_arr256:
asm {
	push cx
	push di
	push si
	mov cx,ancho            // transfiere l¡nea siguiente a actual
	shr cx, 1
	rep movsw
	jnc impar
	movsb
}
impar:
asm {
	pop si
	pop di
	pop cx

	add di,bytesfila              // siguiente l¡nea
	add si,bytesfila

	dec cx ; jnz scr_arr256         // repite hasta desplazar todo bloque

	mov al,color   		// AL=color de relleno
	mov ah, al
	mov cx,8          	// CX=n£mero de l¡neas a rellenar
}
scr_arr_rell256:
asm {
	push cx
	push di
	mov cx,ancho
	shr cx, 1
	rep stosw
	jnc impar2
	stosb
}
impar2:
asm {
	pop di
	pop cx

	add di,bytesfila              // siguiente l¡nea
	dec cx ; jnz scr_arr_rell256

	pop ds
	pop si
	pop di
}

}

/****************************************************************************
	G_SCROLL_ABJ: realiza scroll hacia abajo de una ventana.
	  Entrada:      'fila', 'columna' inicio de la ventana
			'ancho', 'alto' dimensiones de la ventana
			'color' color de relleno
****************************************************************************/
void g_scroll_abj256(int fila, int columna, int ancho, int alto,
  unsigned char color)
{
unsigned char far *pvideo;
unsigned char far *pvid2;
int bytesfila = g_bytesfila ;

/* direcci¢n en buffer de v¡deo de final de zona a desplazar */
pvideo=(unsigned char far *)0xa0000000L+(((((fila+alto)*8)-1)*bytesfila)+
  (columna*8));
/* puntero auxiliar a l¡nea anterior */
pvid2=pvideo-(bytesfila*8);
ancho*=8;
alto=(alto-1)*8;

asm {
	push di
	push si
	push ds

	les di,pvideo           // direcci¢n de inicio
	lds si,pvid2            // puntero a l¡nea anterior

	mov cx,alto		// n£mero de l¡neas a desplazar
}
scr_abj256:
asm {
	push cx
	push di
	push si
	mov cx,ancho            // transfiere l¡nea anterior a actual
	shr cx, 1
	rep movsw
	jnc impar
	movsb
}
impar:
asm {
	pop si
	pop di
	pop cx

	sub di,bytesfila              // l¡nea anterior
	sub si,bytesfila

	dec cx ; jnz scr_abj256         // repite hasta desplazar todo bloque

	mov ax, bytesfila
	mov bx, 7
	mul bx
	sub di,ax		// ES:DI=dir. origen primera l¡nea
				// 2240=320*(8-1)

	mov al,color   		// AL=color de relleno
	mov ah, al
	mov cx,8          	// CX=n£mero de l¡neas a rellenar
}
scr_abj_rell256:
asm {
	push cx
	push di
	mov cx,ancho
	shr cx, 1
	rep stosw
	jnc impar2
	stosb
}
impar2:
asm {
	pop di
	pop cx

	add di,bytesfila              // siguiente l¡nea
	dec cx ; jnz scr_abj_rell256

	pop ds
	pop si
	pop di
}

}

/****************************************************************************
	BLQ_COGE256: recoge un bloque de pantalla en el modo 256 colores y
	  lo guarda.
	  Entrada:      'x0', 'y0' esquina superior izquierda del bloque
			'x1', 'y1' esquina inferior derecha del bloque
			'blq' puntero a buffer para guardar bloque
****************************************************************************/
void blq_coge256(int x0, int y0, int x1, int y1, unsigned char far *blq)
{

asm {
	push di
	push si
	push ds

	mov ax,y0
	mov bx,x0
	xchg ah,al
	add bx,ax
	shr ax,1
	shr ax,1
	add bx,ax       // BX=desplazamiento en buffer de v¡deo
	mov ax,0a000h
	mov ds,ax
	mov si,bx       // DS:SI=direcci¢n en buffer de v¡deo
	les di,blq      // ES:DI=direcci¢n de buffer para guardar bloque

	mov ax,x1       // construye cabecera del bloque
	sub ax,x0       // AX=x1-x0
	inc ax
	mov cx,ax       // CX=anchura de bloque en bytes
	stosw
	mov ax,y1
	sub ax,y0       // AX=y1-y0
	inc ax
	mov bx,ax       // BX=altura de bloque
	stosw
}
sgte_fila:
asm {
	push si
	push cx
	shr cx, 1
	rep movsw
	jnc impar
	movsb
}
impar:
asm {
	pop cx
	pop si

	add si,320      // pasa a siguiente fila
	dec bx
	jnz sgte_fila

	pop ds
	pop si
	pop di
}

}

/****************************************************************************
	BLQ_PON256: pone un bloque en pantalla en el modo 256 colores.
	  Entrada:      'x', 'y' posici¢n de pantalla donde se pondr  el
			bloque
			'blq' puntero a buffer que contiene el bloque
****************************************************************************/
void blq_pon256(int x, int y, unsigned char far *blq)
{

asm {
	push di
	push si
	push ds

	mov ax,y
	mov bx,x
	xchg ah,al
	add bx,ax
	shr ax,1
	shr ax,1
	add bx,ax       // BX=desplazamiento en buffer de v¡deo
	mov ax,0a000h
	mov es,ax
	mov di,bx       // ES:DI=direcci¢n en buffer de v¡deo
	lds si,blq      // DS:SI=direcci¢n de buffer donde est  bloque

	lodsw           // coge anchura del bloque
	mov cx,ax
	lodsw           // coge altura del bloque
	mov bx,ax
}
sgte_fila:
asm {
	push di
	push cx
	shr cx, 1
	rep movsw
	jnc impar
	movsb
}
impar:
asm {
	pop cx
	pop di

	add di,320      // pasa a siguiente fila
	dec bx
	jnz sgte_fila

	pop ds
	pop si
	pop di
}

}

/****************************************************************************
	BLQ_CLIP256: pone un bloque en pantalla en el modo 256 colores,
		     con un nuevo ancho y alto (recort ndolo)
	  Entrada:      'x', 'y' posici¢n de pantalla donde se pondr 
			'ancho', 'alto' tama¤o a recortar
			'blq' puntero a buffer que contiene el bloque
****************************************************************************/
void blq_clip256(int x, int y, int ancho, int alto, unsigned char far *blq)
{
int jumpx = *(int *)blq - ancho ;
if (jumpx < 0) jumpx = 0, ancho = *(int *)blq ;

asm {
	push di
	push si
	push ds

	mov ax,y
	mov bx,x
	xchg ah,al
	add bx,ax
	shr ax,1
	shr ax,1
	add bx,ax       // BX=desplazamiento en buffer de v¡deo
	mov ax,0a000h
	mov es,ax
	mov di,bx       // ES:DI=direcci¢n en buffer de v¡deo
	lds si,blq      // DS:SI=direcci¢n de buffer donde est  bloque

	lodsw           // coge anchura del bloque
	mov cx,ancho
	lodsw           // coge altura del bloque
	mov bx,alto
}
sgte_fila:
asm {
	push di
	push cx
	shr cx, 1
	rep movsw
	jnc impar
	movsb
}
impar:
asm {
	pop cx
	pop di

	add si, jumpx
	add di,320      // pasa a siguiente fila
	dec bx
	jnz sgte_fila

	pop ds
	pop si
	pop di
}

}

/****************************************************************************
	IMP_CHR: imprime un car cter en una posici¢n de pantalla.
	  Entrada:      'chr' car cter a imprimir
			'colorf', 'color' colores de fondo y primer plano
			'modo' modo de dibujo (CHR_NORM, CHR_XOR, CHR_OR,
			CHR_AND)
	  Salida:       anchura del car cter impreso
			posici¢n de impresi¢n actualizada
****************************************************************************/
int imp_chr256(unsigned char chr, unsigned char colorf, unsigned char color,
  unsigned char modo)
{
unsigned char far *pchr;
unsigned char masc, masci, mascd;
int x, y, maxx, maxy, alt, anch, varshift;
int bytesfila = g_bytesfila-8 ;

/* coordenadas de impresi¢n */
x=ult_x;
y=ult_y;

/* calcula par metros seg£n modo de v¡deo */
maxx=g_max_x;
maxy=g_max_y;
alt=g_chr_alto;
anch=_anchchr8x8[chr];

/* si car cter no cabe, lo pasa a siguiente l¡nea */
if((x+anch-1)>maxx) {
	x=0;
	y+=alt;
	if((y+alt-1)>maxy) y=0;
}

/* m scara de car cter */
masc=0xff>>anch;

pchr=_defchr8x8+(chr*g_chr_alto);

asm {
	push di
	push si
	push ds

	mov bx,x
	mov ax,y
	xchg ah,al
	add bx,ax
	shr ax,1
	shr ax,1
	add bx,ax
	mov ax,0a000h
	mov es,ax
	mov di,bx		// ES:DI=dir. en buffer de v¡deo
	lds si,pchr             // DS:SI=dir. definici¢n car cter
	mov cx,alt              // CX=altura en pixels del car cter
	mov bl,color   		// BL=valor pixel de imagen
	mov bh,colorf  		// BH=valor pixel de fondo
}
ic256_1:
asm {
	push cx
	mov cx,8                // CX=anchura car cter en pixels
	lodsb                   // AL=byte de car cter
	mov ah,al               // AH=patr¢n de bit para sgte. fila
	mov dl,masc		// m scara car cter

	cmp modo, G_NORM
	je ic256_2_nor
	cmp modo, G_AND
	je ic256_2_and
	cmp modo, G_OR
	je ic256_2_or
	cmp modo, G_XOR
	je ic256_2_xor
	jmp ic256_2_noback
}
/* -------------------------------------------------------------- */
ic256_2_xor:
asm {
	shl ah,1                // CARRY=bit alto
	jc ic_xor               // salta si es pixel de imagen
	xor es:[di], bh
	inc di
	dec cx ; jnz ic256_2_xor
	jmp sgte_linea
}
ic_xor:
asm {
	xor es:[di], bl
	inc di
	dec cx ; jnz ic256_2_xor
	jmp sgte_linea
}
/* -------------------------------------------------------------- */
ic256_2_or:
asm {
	shl ah,1                // CARRY=bit alto
	jc ic_or               // salta si es pixel de imagen
	or es:[di], bh
	inc di
	dec cx ; jnz ic256_2_or
	jmp sgte_linea
}
ic_or:
asm {
	or es:[di], bl
	inc di
	dec cx ; jnz ic256_2_or
	jmp sgte_linea
}
/* -------------------------------------------------------------- */
ic256_2_and:
asm {
	shl ah,1                // CARRY=bit alto
	jc ic_and               // salta si es pixel de imagen
	and es:[di], bh
	inc di
	dec cx ; jnz ic256_2_and
	jmp sgte_linea
}
ic_and:
asm {
	and es:[di], bl
	inc di
	dec cx ; jnz ic256_2_and
	jmp sgte_linea
}
/* -------------------------------------------------------------- */
ic256_2_nor:
asm {
	shl ah,1                // CARRY=bit alto
	jc ic_nor               // salta si es pixel de imagen
	mov es:[di], bh
	inc di
	dec cx ; jnz ic256_2_nor
	jmp sgte_linea
}
ic_nor:
asm {
	mov es:[di], bl
	inc di
	dec cx ; jnz ic256_2_nor
	jmp sgte_linea
}
/* -------------------------------------------------------------- */
ic256_2_noback:
asm {
	shl ah,1                // CARRY=bit alto
	jc ic_noback            // salta si es pixel de imagen
	inc di
	dec cx ; jnz ic256_2_noback
	jmp sgte_linea
}
ic_noback:
asm {
	mov es:[di], bl
	inc di
	dec cx ; jnz ic256_2_noback
	jmp sgte_linea
}
/* -------------------------------------------------------------- */
sgte_linea:
asm {
	add di,bytesfila              // siguiente l¡nea en pantalla
	pop cx
	dec cx ;
	jz ic256_fin
	jmp ic256_1
}
ic256_fin:
asm {

	pop ds
	pop si
	pop di
}

/* actualiza coordenada X */
x+=anch;
if(x>maxx) {
	x=0;
	y+=alt;
}
/* actualiza coordenada Y, si se sale de pantalla pasa a (0,0) */
if((y+alt-1)>maxy) {
	x=0;
	y=0;
}

ult_x=x;
ult_y=y;

return(anch);
}

