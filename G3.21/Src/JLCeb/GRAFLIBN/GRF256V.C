/*

	Librer¡a gr fica SINTAC

	(c) 1995 Javier San Jos‚ y Jos‚ Luis Cebri n
								*/

/* G_MM_256C_V (modos VESA SuperVGA de 256 colores) */

#include <alloc.h>

#include "grf.h"

#define abs(i) ((i) < 0 ? -(i) : (i))
#define max(a,b)    (((a) > (b)) ? (a) : (b))
#define min(a,b)    (((a) < (b)) ? (a) : (b))

extern int g_bytesfila ;

extern unsigned char *_anchchr ;
extern unsigned char *_defchr ;
extern unsigned char *_anchchr8x8 ;
extern unsigned char *_defchr8x8 ;

void swapint(int *i1, int *i2) ;
void g_rectangulo256v(int x0, int y0, int x1, int y1, unsigned char color,
  unsigned char modo) ;

/* calcula direcci¢n de pixel en buffer de v¡deo en el modo 320x200x256 */
/* a la entrada: AX=coordenada Y, BX=coordenada X */
/* a la salida: ES:BX=direcci¢n en buffer de v¡deo */
#define DIR_PIXEL256v	asm {			 \
				mov dx, g_bytesfila ;\
				mul dx ;\
				add bx, ax ;\
				adc dx, 0 ;\
				push bx ;\
				push dx ;\
				call far ptr g_setwindow ;\
				pop ax ;\
				mov ax,0a000h	;\
				mov es,ax  	;\
				pop bx ;\
			}

#define TEST_BANK_JUMP   	asm {                            \
				jnc no_bank_jump                ;\
				push ds                         ;\
				push es                         ;\
				pusha                           ;\
				mov ax, g_ds                    ;\
				mov ds, ax                      ;\
				call far ptr g_incwindow        ;\
				popa                            ;\
				pop es                          ;\
				pop ds                           \
				}                                \
				no_bank_jump :

/****************************************************************************
	G_BORRA_PANTALLA: borra la pantalla.
****************************************************************************/
void g_borra_pantalla256v(void)
{
int veces = g_bytespantalla/65536L+1 ;

g_setwindow(0) ;

while (veces--)
asm {
	push di

	mov di,0a000h
	mov es,di
	xor di,di	// ES:DI=direcci¢n inicio buffer de v¡deo
	db 0x66
	xor ax,ax	// EAX=0 (borrar buffer)
	mov cx,16384	// bytes de buffer de v¡deo/2
	db 0x66
	rep stosw	// rep stosd

	call far ptr g_incwindow
	pop di
}

}

/****************************************************************************
	G_COGE_PIXEL: devuelve el color de un pixel.
	  Entrada:	'x', 'y' coordenadas del pixel
	  Salida:	color del pixel
****************************************************************************/
unsigned char g_coge_pixel256v(int x, int y)
{
unsigned char color;

asm {
	mov ax,y
	mov bx,x
}
	DIR_PIXEL256v;	// ES:BX=direcci¢n en buffer de v¡deo
asm {
	mov al,es:[bx]
	mov color,al
}
return color ;
}

/****************************************************************************
	G_PUNTO256v: dibuja un punto en el modo de 256v colores.
	  Entrada:	'x', 'y' coordenadas
			'color' color del punto
			'modo' modo de dibujo (G_NORM, G_XOR, G_OR, G_AND)
****************************************************************************/
void g_punto256v(int x, int y, unsigned char color, unsigned char modo)
{

asm {
	mov ax,y
	mov bx,x
}
	DIR_PIXEL256v;	// ES:BX=direcci¢n en buffer de v¡deo
asm {
	mov al,color

	cmp modo,G_NORM
	jne punto256v_and

	mov es:[bx],al
}
	return;
punto256v_and:
asm {
	cmp modo,G_AND
	jne punto256v_or

	and es:[bx],al
}
	return;
punto256v_or:
asm {
	cmp modo,G_OR
	jne punto256v_xor

	or es:[bx],al
}
	return;
punto256v_xor:
asm {
	xor es:[bx],al
}

}

/****************************************************************************
	G_LINEA256V: dibuja un l¡nea en el modo VESA 256c.
	  Entrada:	'x0', 'y0' punto de origen
			'x1', 'y1' punto final
			'color' color de la l¡nea
			'modo' modo de dibujo (G_NORM, G_XOR, G_OR, G_AND)
****************************************************************************/
void g_linea256v(int x0, int y0, int x1, int y1, unsigned char color,
  unsigned char modo)
{
register int x, y;
int d, dx, dy, aincr, bincr, xincr, yincr;

if (x0 == x1 || y0 == y1)
{
   g_rectangulo256v (x0, y0, x1, y1, color, modo) ;
   return ;
}

/* ir por el eje X o Y */
if(abs(x1-x0)<abs(y1-y0)) {	/* por el eje Y */
	if(y0>y1) {
		swapint(&x0,&x1);
		swapint(&y0,&y1);
	}
	xincr=(x1>x0) ? 1 : -1;

	dy=y1-y0;
	dx=abs(x1-x0);
	d=2*dx-dy;
	aincr=2*(dx-dy);
	bincr=2*dx;
	x=x0;
	y=y0;

	g_punto256v(x,y,color,modo);
	for(y=y0+1; y<=y1; y++) {
		if(d>=0) {
			x+=xincr;
			d+=aincr;
		}
		else d+=bincr;
		g_punto256v(x,y,color,modo);
	}
}
else {				/* por el eje X */
	if(x0>x1) {
		swapint(&x0,&x1);
		swapint(&y0,&y1);
	}

	yincr=(y1>y0) ? 1 : -1;

	dx=x1-x0;
	dy=abs(y1-y0);
	d=2*dy-dx;
	aincr=2*(dy-dx);
	bincr=2*dy;
	x=x0;
	y=y0;

	g_punto256v(x,y,color,modo);
	for(x=x0+1; x<=x1; x++) {
		if(d>=0) {
			y+=yincr;
			d+=aincr;
		}
		else d+=bincr;
		g_punto256v(x,y,color,modo);
	}
}

}


/****************************************************************************
	G_RECTANGULO256v: dibuja un rect ngulo s¢lido en el modo de 256v
	  colores.
	  Entrada:	'x0', 'y0' esquina superior izquierda
			'x1', 'y1' esquina inferior derecha
			'color' color del rect ngulo
			'modo' modo de dibujo (G_NORM, G_XOR, G_OR, G_AND)
****************************************************************************/
void g_rectangulo256v(int x0, int y0, int x1, int y1, unsigned char color,
  unsigned char modo)
{
int bytesfila = g_bytesfila, g_ds = _DS ;

if (y0 > y1) swapint (&y1, &y0) ;
if (x0 > x1) swapint (&x0, &x1) ;

asm {
	push si
	push di
	push ds

	mov ax,y0
	mov bx,x0
}
	DIR_PIXEL256v;	// ES:BX=direcci¢n en buffer de v¡deo
asm {
	mov di,bx       // ES:DI=direcci¢n en buffer de v¡deo

	mov cx,x1
	sub cx,x0
	inc cx          // CX=cantidad de pixels a dibujar

	mov bx,y1
	sub bx,y0
	inc bx		// BX=n£mero de l¡neas

	mov al,color    // AL=valor de pixel
	mov ah, al
	push ax
	db 0x66
	shl ax, 16
	pop ax

	cmp modo, G_NORM
	je rect256vnorm
	jmp rect256vnonorm

}
rect256vnorm:
asm {
	push di
	push cx

	mov dx, di
	not dx
	inc dx
	jz noproblem
	cmp cx, dx
	jbe noproblem
	push cx
	mov cx, dx
	rep stosb
	pusha
	push ds
	mov ax, g_ds
	mov ds, ax
	call far ptr g_incwindow
	pop ds
	popa
	pop cx
	sub cx, dx
	sub di, di
	rep stosb

	pop cx
	pop di
	add di,bytesfila	// siguiente l¡nea
	jmp noproblem2
}
	noproblem:

asm {
	mov dx, cx
	shr cx, 2
	jz thedx1
	db 0x66
	rep stosw
}
thedx1:
asm {
	mov cx, dx
	and cx, 3
	jz nodx
}
thedx:
asm {
	rep stosb
}
nodx:
asm {
	pop cx
	pop di
	add di,bytesfila	// siguiente l¡nea
}
	TEST_BANK_JUMP ;
	noproblem2:
asm {
	dec bx
	jnz rect256vnorm

	pop ds
	pop di
	pop si
}
return ;
/* ---------------------------------------------------------------- */

rect256vnonorm:
asm {
	push di
	push cx

	mov dx, di
	not dx
	inc dx
	jz problem_start
	cmp cx, dx
	ja problem_start
	jmp nonoproblem
}
problem_start:
asm {
	push cx
	mov cx, dx
	cmp modo, G_XOR
	je pas1_xor
	cmp modo, G_OR
	je pas1_or
}
pas1_and:
asm {
	and [es:di], al
	inc di
	dec cx ; jnz pas1_and
	jmp pas1_end
}
pas1_xor:
asm {
	xor [es:di], al
	inc di
	dec cx ; jnz pas1_xor
	jmp pas1_end
}
pas1_or:
asm {
	or [es:di], al
	inc di
	dec cx ; jnz pas1_or
	jmp pas1_end
}
pas1_end:
asm {
	pusha
	push ds
	mov ax, g_ds
	mov ds, ax
	call far ptr g_incwindow
	pop ds
	popa
	pop cx
	sub cx, dx
	sub di, di
	cmp modo, G_XOR
	je pas2_xor
	cmp modo, G_OR
	je pas2_or
}
pas2_and:
asm {
	and [es:di], al
	inc di
	dec cx ; jnz pas2_and
	jmp pas2_end
}
pas2_xor:
asm {
	xor [es:di], al
	inc di
	dec cx ; jnz pas2_xor
	jmp pas2_end
}
pas2_or:
asm {
	or [es:di], al
	inc di
	dec cx ; jnz pas2_or
	jmp pas2_end
}
pas2_end:
asm {

	pop cx
	pop di
	add di,bytesfila	// siguiente l¡nea
	jmp nonoproblem2
}
/*****************************/
nonoproblem:
asm {
	cmp modo, G_XOR
	je modoxor
	cmp modo, G_OR
	je modoor
}
modoand:
asm {
	and [es:di], al
	inc di
	dec cx ; jnz modoand
	jmp siglinea
}
modoxor:
asm {
	mov dx, cx
	shr cx, 1
	jz modoxor_buc2
}
modoxor_buc:
asm {
	xor [es:di], ax
	add di, 2
	dec cx
	jnz modoxor_buc
}
modoxor_buc2:
asm {
	test dx, 1
	jz siglinea
	xor [es:di], al
	jmp siglinea
}
modoor:
asm {
	or [es:di], al
	inc di
	dec cx ; jnz modoor
	jmp siglinea
}
siglinea:
asm {
	pop cx
	pop di
	add di,bytesfila	// siguiente l¡nea
	jnc nono_bank_jump
	push ds
	push es
	pusha
	mov ax, g_ds
	mov ds, ax
	call far ptr g_incwindow
	popa
	pop es
	pop ds
}
nono_bank_jump :
nonoproblem2:
asm {
	dec bx
	jz rect256end
	jmp rect256vnonorm
}
rect256end:
asm {
	pop ds
	pop di
	pop si
}

}

/****************************************************************************
	G_MOVER_ZONA: desplaza una zona de pantalla sobre la misma
		      pantalla, utilizando blq_coge y blq_pon.
	  Entrada:	'x1' 'y1' 'x2' 'y2' coordenadas de la zona a mover
			'xd' 'yd' coordenadas de destino
****************************************************************************/
char emergency_buffer[1024+4] ;

void g_mover_zona256v (int x1, int y1, int x2, int y2, int xd, int yd)
{
long res_size = coreleft() ;
int res_lineas, bfila = x2-x1+1, lineas = y2-y1+1 ;
char *blq ;

if (res_size > 65535L) res_size = 65535L ;
res_size -= 4 ;
res_lineas = res_size/bfila ;
if (res_lineas > lineas) res_lineas = lineas ;

blq = (char *)malloc (res_lineas*bfila+4) ;
if (!blq)
{
   blq = emergency_buffer ;
   res_lineas = 1 ;
}

if (y2 < y1 || (y2 == y1 && x2 < x1))
{
   /* Copiar atr s: fila superior primero */
   while (lineas > 0)
   {
      blq_coge (x1,y1,x2,y1+min(res_lineas,lineas), blq) ;
      blq_pon (xd,yd, blq) ;
      y1 += res_lineas ;
      yd += res_lineas ;
      lineas -= res_lineas ;
   }
}
else
{
   /* Copiar delante: fila superior £ltima */
   yd += lineas-1 ;
   while (lineas > 0)
   {
      blq_coge (x1,y2+1-min(res_lineas,lineas),x2,y2, blq) ;
      blq_pon (xd,yd+1-min(res_lineas,lineas), blq) ;
      y2 -= res_lineas ;
      yd -= res_lineas ;
      lineas -= res_lineas ;
   }
}

if (blq != emergency_buffer) free(blq) ;

}

/****************************************************************************
	G_SCROLL_ARR: realiza scroll hacia arriba de una ventana.
	  Entrada:      'fila', 'columna' inicio de la ventana
			'ancho', 'alto' dimensiones de la ventana
			'color' color de relleno
****************************************************************************/
void g_scroll_arr256v(int fila, int columna, int ancho, int alto,
  unsigned char color)
{
   int yd = fila*chr_altura(), xd = columna*8 ;
   int y1 = yd+chr_altura(), y2 = yd+alto*chr_altura()-1 ;
   int x1 = xd, x2 = x1+ancho*8-1 ;

   g_mover_zona256v (x1, y1, x2, y2, xd, yd) ;
   g_rectangulo (x1,y2-chr_altura()+1,x2,y2,color,G_NORM,1) ;
}

/****************************************************************************
	G_SCROLL_ABJ: realiza scroll hacia abajo de una ventana.
	  Entrada:      'fila', 'columna' inicio de la ventana
			'ancho', 'alto' dimensiones de la ventana
			'color' color de relleno
****************************************************************************/
void g_scroll_abj256v(int fila, int columna, int ancho, int alto,
  unsigned char color)
{
   int y1 = fila*chr_altura(), x1 = columna*8 ;
   int yd = y1+chr_altura(), y2 = y1+(alto-1)*chr_altura()-1 ;
   int x2 = x1+ancho*8-1, xd = x1 ;

   g_mover_zona256v (x1, y1, x2, y2, xd, yd) ;
   g_rectangulo (x1,y1,x2,y1+chr_altura()-1,color,G_NORM,1) ;
}

/****************************************************************************
	BLQ_COGE256v: recoge un bloque de pantalla en el modo VESA y lo guarda.
	  Entrada:      'x0', 'y0' esquina superior izquierda del bloque
			'x1', 'y1' esquina inferior derecha del bloque
			'blq' puntero a buffer para guardar bloque
****************************************************************************/
void blq_coge256v(int x0, int y0, int x1, int y1, unsigned char far *blq)
{
int bytes_fila = g_bytesfila, g_ds = _DS ;

asm {
	push di
	push si
	push ds

	mov ax,y0
	mov bx,x0
}
	DIR_PIXEL256v ;
asm {
	mov ax, es
	mov ds,ax
	mov si,bx       // DS:SI=direcci¢n en buffer de v¡deo
	les di,blq      // ES:DI=direcci¢n de buffer para guardar bloque

	mov ax,x1       // construye cabecera del bloque
	sub ax,x0       // AX=x1-x0
	inc ax
	mov cx,ax       // CX=anchura de bloque en bytes
	mov dx, cx
	and dx, 3
	stosw
	mov ax,y1
	sub ax,y0       // AX=y1-y0
	inc ax
	mov bx,ax       // BX=altura de bloque
	stosw
}
sgte_fila:
asm {
	add si, cx
	jnc noproblem
	sub si, cx
	push dx
	push si
	push cx
	mov cx, si
	not cx
	inc cx
	mov dx, cx
	rep movsb
	pusha
	push ds
	mov ax, g_ds
	mov ds, ax
	call far ptr g_incwindow
	pop ds
	popa
	pop cx
	push cx
	sub cx, dx
	jz problemilla
	rep movsb
}
problemilla:
asm {
	pop cx
	pop si
	pop dx
	add si, bytes_fila
	jmp problemok
}
noproblem:
asm {
	sub si, cx
	push cx
	shr cx, 2
	db 0x66
	rep movsw
	mov cx, dx
	jcxz nomasuno
	rep movsb
}
nomasuno:
asm {
	pop cx
	sub si, cx

	add si,bytes_fila      // pasa a siguiente fila
}
	TEST_BANK_JUMP ;
problemok:
asm {
	dec bx
	jnz sgte_fila

	pop ds
	pop si
	pop di
}

}

/****************************************************************************
	BLQ_PON256v: pone un bloque en pantalla en el modo 256v colores.
	  Entrada:      'x', 'y' posici¢n de pantalla donde se pondr  el
			bloque
			'blq' puntero a buffer que contiene el bloque
****************************************************************************/
void blq_pon256v(int x, int y, unsigned char far *blq)
{
int bytes_fila = g_bytesfila, g_ds = _DS ;

asm {
	push di
	push si
	push ds

	mov ax,y
	mov bx,x
}
	DIR_PIXEL256v ;
asm {
	mov di,bx       // ES:DI=direcci¢n en buffer de v¡deo
	lds si,blq      // DS:SI=direcci¢n de buffer donde est  bloque

	lodsw           // coge anchura del bloque
	mov cx,ax
	mov dx, ax
	and dx, 3
	lodsw           // coge altura del bloque
	mov bx,ax
}
sgte_fila:
asm {
	add di, cx
	jnc noproblem
	sub di, cx
	push dx
	push di
	push cx
	mov cx, di
	not cx
	inc cx
	mov dx, cx
	rep movsb
	pusha
	push ds
	mov ax, g_ds
	mov ds, ax
	call far ptr g_incwindow
	pop ds
	popa
	pop cx
	push cx
	sub cx, dx
	jz problemilla
	rep movsb
}
problemilla:
asm {
	pop cx
	pop di
	pop dx
	add di, bytes_fila
	jmp problemok
}
noproblem:
asm {
	sub di, cx
	push cx
	shr cx, 2
	db 0x66
	rep movsw
	mov cx, dx
	jcxz nomasuno
	rep movsb
}
nomasuno:
asm {
	pop cx
	sub di, cx

	add di,bytes_fila      // pasa a siguiente fila
}
	TEST_BANK_JUMP ;
problemok:
asm {
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
void blq_clip256v(int x, int y, int ancho, int alto, unsigned char far *blq)
{
int bytes_fila = g_bytesfila, g_ds = _DS ;
int jumpx = *(int *)blq - ancho ;
if (jumpx < 0) jumpx = 0, ancho = *(int *)blq ;

asm {
	push di
	push si
	push ds

	mov ax,y
	mov bx,x
}
	DIR_PIXEL256v ;
asm {
	mov di,bx       // ES:DI=direcci¢n en buffer de v¡deo
	lds si,blq      // DS:SI=direcci¢n de buffer donde est  bloque

	lodsw           // coge anchura del bloque
	mov cx,ancho
	mov dx, ax
	and dx, 3
	lodsw           // coge altura del bloque
	mov bx,alto
}
sgte_fila:
asm {
	add di, cx
	jnc noproblem
	sub di, cx
	push dx
	push di
	push cx
	mov cx, di
	not cx
	inc cx
	mov dx, cx
	rep movsb
	pusha
	push ds
	mov ax, g_ds
	mov ds, ax
	call far ptr g_incwindow
	pop ds
	popa
	pop cx
	push cx
	sub cx, dx
	jz problemilla
	rep movsb
}
problemilla:
asm {
	pop cx
	pop di
	pop dx
	add si, jumpx
	add di, bytes_fila
	jmp problemok
}
noproblem:
asm {
	sub di, cx
	push cx
	shr cx, 2
	db 0x66
	rep movsw
	mov cx, dx
	jcxz nomasuno
	rep movsb
}
nomasuno:
asm {
	pop cx
	sub di, cx

	add si, jumpx
	add di,bytes_fila      // pasa a siguiente fila
}
	TEST_BANK_JUMP ;
problemok:
asm {
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
int imp_chr256v(unsigned char chr, unsigned char colorf, unsigned char color,
  unsigned char modo)
{
unsigned char far *pchr;
unsigned char masc, masci, mascd;
int x, y, maxx, maxy, alt, anch, varshift;
int bytesfila = g_bytesfila-8, g_ds = _DS ;

/* coordenadas de impresi¢n */
x=ult_x;
y=ult_y;

/* calcula par metros seg£n modo de v¡deo */
maxx=g_max_x;
maxy=g_max_y;
alt=g_chr_alto;
if (alt == 16)
	anch=_anchchr[chr];
else
	anch=_anchchr8x8[chr];
anch=_anchchr8x8[chr];

/* si car cter no cabe, lo pasa a siguiente l¡nea */
if((x+anch-1)>maxx) {
	x=0;
	y+=alt;
	if((y+alt-1)>maxy) y=0;
}

/* m scara de car cter */
masc=0xff>>anch;

if (alt == 16)
	pchr=_defchr+(chr*g_chr_alto);
else
	pchr=_defchr8x8+(chr*g_chr_alto);

asm {
	push di
	push si
	push ds

	mov bx,x
	mov ax,y
}
	DIR_PIXEL256v;
asm {
	mov di,bx		// ES:DI=dir. en buffer de v¡deo
	lds si,pchr             // DS:SI=dir. definici¢n car cter
	mov cx,alt              // CX=altura en pixels del car cter
	mov bl,color   		// BL=valor pixel de imagen
	mov bh,colorf  		// BH=valor pixel de fondo
}
ic256v_1:
asm {
	push cx
	mov cx,8                // CX=anchura car cter en pixels
	lodsb                   // AL=byte de car cter
	mov ah,al               // AH=patr¢n de bit para sgte. fila
	mov dl,masc		// m scara car cter

	cmp modo, G_NORM
	je ic256v_put
	cmp modo, G_XOR
	je ic256v_xor
	cmp modo, G_AND
	je ic256v_and
	cmp modo, G_NOBACK
	jne orjump
	jmp ic256v_noback
}
orjump:
asm     jmp ic256v_or
/* -------------------------------------------------------------- */
ic256v_and:
/* ------------- G_AND ------------------------------------------ */
asm {
	shl ah,1                // CARRY=bit alto
	jc ic_and               // salta si es pixel de imagen
	and es:[di],bh
	inc di
	jz ic_err_and
	dec cx ; jnz ic256v_and
	jmp sgte_linea
}
ic_and:
asm {
	and es:[di], bl
	inc di
	jz ic_err_and
	dec cx ; jnz ic256v_and
	jmp sgte_linea
}
ic_err_and:
asm {
	pusha
	push ds
	mov ax, g_ds
	mov ds, ax
	call far ptr g_incwindow
	pop ds
	popa
	dec cx ; jnz ic256v_and
	jmp sgte_linea
}
/* -------------------------------------------------------------- */
ic256v_xor:
/* ------------- G_XOR ------------------------------------------ */
asm {
	shl ah,1                // CARRY=bit alto
	jc ic_xor               // salta si es pixel de imagen
	xor es:[di],bh
	inc di
	jz ic_err_xor
	dec cx ; jnz ic256v_xor
	jmp sgte_linea
}
ic_xor:
asm {
	xor es:[di], bl
	inc di
	jz ic_err_xor
	dec cx ; jnz ic256v_xor
	jmp sgte_linea
}
ic_err_xor:
asm {
	pusha
	push ds
	mov ax, g_ds
	mov ds, ax
	call far ptr g_incwindow
	pop ds
	popa
	dec cx ; jnz ic256v_xor
	jmp sgte_linea
}
/* -------------------------------------------------------------- */
ic256v_put:
/* ------------- G_NORM ----------------------------------------- */
asm {
	shl ah,1                // CARRY=bit alto
	jc ic_put               // salta si es pixel de imagen
	mov es:[di],bh
	inc di
	jz ic_err_put
	dec cx ; jnz ic256v_put
	jmp sgte_linea
}
ic_put:
asm {
	mov es:[di], bl
	inc di
	jz ic_err_put
	dec cx ; jnz ic256v_put
	jmp sgte_linea
}
ic_err_put:
asm {
	pusha
	push ds
	mov ax, g_ds
	mov ds, ax
	call far ptr g_incwindow
	pop ds
	popa
	dec cx ; jnz ic256v_put
	jmp sgte_linea
}
ic256v_noback:
/* ------------- G_NOBACK --------------------------------------- */
asm {
	shl ah,1                // CARRY=bit alto
	jc ic_noback            // salta si es pixel de imagen
	inc di
	jz ic_err_put
	dec cx ; jnz ic256v_noback
	jmp sgte_linea
}
ic_noback:
asm {
	mov es:[di], bl
	inc di
	jz ic_err_put
	dec cx ; jnz ic256v_noback
	jmp sgte_linea
}
ic_err_noback:
asm {
	pusha
	push ds
	mov ax, g_ds
	mov ds, ax
	call far ptr g_incwindow
	pop ds
	popa
	dec cx ; jnz ic256v_noback
	jmp sgte_linea
}
ic256v_or:
/* ------------- G_OR ------------------------------------------- */
asm {
	shl ah,1                // CARRY=bit alto
	jc ic_or                // salta si es pixel de imagen
	or  es:[di],bh
	inc di
	jz ic_err_or
	dec cx ; jnz ic256v_or
	jmp sgte_linea
}
ic_or:
asm {
	or  es:[di], bl
	inc di
	jz ic_err_or
	dec cx ; jnz ic256v_or
	jmp sgte_linea
}
ic_err_or:
asm {
	pusha
	push ds
	mov ax, g_ds
	mov ds, ax
	call far ptr g_incwindow
	pop ds
	popa
	dec cx ; jnz ic256v_or
	jmp sgte_linea
}
/* -------------------------------------------------------------- */
sgte_linea:
asm {
	add di,bytesfila              // siguiente l¡nea en pantalla
}
	TEST_BANK_JUMP ;
asm {
	pop cx
	dec cx
	jz ic256_fin
	jmp ic256v_1
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

