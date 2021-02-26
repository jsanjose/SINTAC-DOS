/*

	Librer¡a gr fica SINTAC

	(c) 1995 Javier San Jos‚ y Jos‚ Luis Cebri n
								*/

/* G_MM_256C_X (modos X de 256 colores) */

#include "grf.h"

#define abs(i) ((i) < 0 ? -(i) : (i))

extern int g_bytesfila ;

extern unsigned char *_anchchr ;
extern unsigned char *_defchr ;
extern unsigned char *_anchchr8x8 ;
extern unsigned char *_defchr8x8 ;

/* calcula direcci¢n de pixel en buffer de v¡deo en el modo 360x480x256 */
/* a la entrada: AX=coordenada Y, BX=coordenada X */
/* a la salida: ES:BX=direcci¢n en buffer de v¡deo */
#define DIR_PIXEL256X	asm {			 \
				mov dx,g_bytesfila	;\
				mul dx          ;\
				shr bx,1        ;\
				shr bx,1        ;\
				add bx,ax       ;\
				mov ax,0a000h   ;\
				mov es,ax       ;\
			}

/****************************************************************************
	SWAPINT: intercambia dos variables enteras.
	  Entrada:	'i1', 'i2' punteros a variables a intercambiar
****************************************************************************/
void swapint(int *i1, int *i2)
{
int i;

i=*i2;
*i2=*i1;
*i1=i;

}

/****************************************************************************
	G_BORRA_PANTALLA: borra la pantalla.
****************************************************************************/
void g_borra_pantalla256x(void)
{
int filas_pixels;

asm {
	push di

	mov dx,03c4h	// puerto del secuenciador
	mov ax,0f02h	// permite escribir en todos los planos
	out dx,ax

	mov di,0a000h
	mov es,di
	xor di,di	// ES:DI=direcci¢n inicio buffer de v¡deo

	xor ax,ax	// AX=0 (borrar buffer)

	mov cx,21600
	rep stosw	// limpia la pantalla

	pop di
}
}

/****************************************************************************
	G_COGE_PIXEL: devuelve el color de un pixel.
	  Entrada:	'x', 'y' coordenadas del pixel
	  Salida:	color del pixel
****************************************************************************/
unsigned char g_coge_pixel256x(int x, int y)
{
unsigned char color;

asm {
	mov ax,x
	and ax,3
	mov ah,al       // AH=n§ de plano de bits
	mov al,4
	mov dx,03ceh	// puerto del controlador gr fico
	out dx,ax	// selecciona plano

	mov ax,y
	mov bx,x
}
	DIR_PIXEL256X;	// ES:BX=direcci¢n en buffer de v¡deo
asm {
	mov al,es:[bx]	// guarda un byte desde cada plano de bits
	mov color,al
}
return color ;
}

/****************************************************************************
	G_PUNTO256X: dibuja un punto en el modo 360x480x256.
	  Entrada:	'x', 'y' coordenadas del punto
			'color' color del punto
			'modo' modo de dibujo (G_NORM, G_XOR, G_OR, G_AND)
****************************************************************************/
void g_punto256x(int x, int y, unsigned char color, unsigned char modo)
{

asm {
	mov cx,x
	and cx,3
	mov ax,1
	shl ax,cl	// m scara planos de bit
	mov ah,al
	mov al,2
	mov dx,03c4h	// puerto del secuenciador
	out dx,ax	// selecciona plano

	mov ax,y
	mov bx,x
}
	DIR_PIXEL256X;
asm {
	mov dx,03ceh	// puerto del controlador gr fico
	mov ah,modo
	mov al,3	// AL=registro rotar dato/seleccionar funci¢n
	out dx,ax

	mov al,es:[bx]	// guarda un byte desde cada plano de bits
	mov al,color
	mov es:[bx],al  // actualiza planos de bits

	mov ax,0003h	// inicializa registro seleccionar funci¢n
	out dx,ax
}

}

/****************************************************************************
	G_LINEA256X: dibuja un l¡nea en el modo 360x480x256.
	  Entrada:	'x0', 'y0' punto de origen
			'x1', 'y1' punto final
			'color' color de la l¡nea
			'modo' modo de dibujo (G_NORM, G_XOR, G_OR, G_AND)
****************************************************************************/
void g_linea256x(int x0, int y0, int x1, int y1, unsigned char color,
  unsigned char modo)
{
register int x, y;
int d, dx, dy, aincr, bincr, xincr, yincr;

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

	g_punto256x(x,y,color,modo);
	for(y=y0+1; y<=y1; y++) {
		if(d>=0) {
			x+=xincr;
			d+=aincr;
		}
		else d+=bincr;
		g_punto256x(x,y,color,modo);
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

	g_punto256x(x,y,color,modo);
	for(x=x0+1; x<=x1; x++) {
		if(d>=0) {
			y+=yincr;
			d+=aincr;
		}
		else d+=bincr;
		g_punto256x(x,y,color,modo);
	}
}

}

/****************************************************************************
	G_RECTANGULO256X: dibuja un rect ngulo s¢lido en el modo 360x480x256.
	  Entrada:	'x0', 'y0' esquina superior izquierda
			'x1', 'y1' esquina inferior derecha
			'color' color del rect ngulo
			'modo' modo de dibujo (G_NORM, G_XOR, G_OR, G_AND)
****************************************************************************/
char LeftClipPlaneMask[4] = { 0xf,0xe,0xc,0x8 } ;
char RightClipPlaneMask[4] = { 0xf,0x1,0x3,0x7 } ;

void g_rectangulo256x(int x0, int y0, int x1, int y1, unsigned char color,
  unsigned char modo)
{
int bytesfila = g_bytesfila;

asm {
	push si
	push di
	push bp

	mov dx,03ceh	// puerto del controlador gr fico
	mov ah,modo
	mov al,3	// AL=registro rotar dato/seleccionar funci¢n
	out dx,ax

	mov ax,y0
	mov bx,x0
}
	DIR_PIXEL256X;
asm {
	mov di, bx

	mov dx, 03C4h
	mov al, 2
	out dx, al
	inc dx

	mov si, x0
	and si, 3
	mov bh, byte ptr [LeftClipPlaneMask+si]
	mov si, x1
	and si, 3
	mov bl, byte ptr [RightClipPlaneMask+si]

	mov cx, x1
	mov si, x0
	cmp cx, si
	jle END
	dec cx
	and si, not 011h
	sub cx, si
	sar cx, 1
	sar cx, 1
	jnz MASKSET
	and bh, bl
}
	MASKSET:
asm {
	mov si, y1
	sub si, y0
	jle END
	mov ah, color
	mov bp, bytesfila
	sub bp, cx
	dec bp
}
	FILLROWSLOOP:
asm {
	push cx
	mov al, bh
	out dx, al
	mov al, [es:di]
	mov [es:di], ah
	inc di
	dec cx
	js FILLLOOPBOTTOM
	jz DORIGHTEDGE
	mov al, 0Fh
	out dx, al
	mov al, ah
}
	REPLOOP:
asm {
	mov al, [es:di]
	mov [es:di], ah
	inc di
	dec cx ; jnz REPLOOP
}
	DORIGHTEDGE:
asm {
	mov al, bl
	out dx, al
	mov al, ah
	mov al, [es:di]
	mov [es:di], ah
	inc di
}
	FILLLOOPBOTTOM:
asm {
	add di, bp
	pop cx
	dec si
	jnz FILLROWSLOOP
}
	END:
asm {
	pop bp
	pop di
	pop si

	mov dx,03ceh	// puerto del registro de direcciones
	mov ax,0003h
	out dx,ax	// inicializa registro seleccionar funci¢n
}

}

/****************************************************************************
	G_RECTANGULO256XNORM: dibuja un rect ngulo s¢lido en modos X
	  Entrada:	'x0', 'y0' esquina superior izquierda
			'x1', 'y1' esquina inferior derecha
			'color' color del rect ngulo

	  VERSION OPTIMIZADA QUE SOLO FUNCIONA EN EL MODO G_NORM
****************************************************************************/
void g_rectangulo256xnorm(int x0, int y0, int x1, int y1, unsigned char color)
{
int bytesfila = g_bytesfila;

asm {
	push si
	push di
	push bp

	mov ax,y0
	mov bx,x0
}
	DIR_PIXEL256X;
asm {
	mov di, bx

	mov dx, 03C4h
	mov al, 2
	out dx, al
	inc dx

	mov si, x0
	and si, 3
	mov bh, byte ptr [LeftClipPlaneMask+si]
	mov si, x1
	and si, 3
	mov bl, byte ptr [RightClipPlaneMask+si]

	mov cx, x1
	mov si, x0
	cmp cx, si
	jle END
	dec cx
	and si, not 011h
	sub cx, si
	sar cx, 1
	sar cx, 1
	jnz MASKSET
	and bh, bl
}
	MASKSET:
asm {
	mov si, y1
	sub si, y0
	jle END
	mov ah, color
	mov bp, bytesfila
	sub bp, cx
	dec bp
}
	FILLROWSLOOP:
asm {
	push cx
	mov al, bh
	out dx, al
	mov al, ah
	stosb
	dec cx
	js FILLLOOPBOTTOM
	jz DORIGHTEDGE
	mov al, 0Fh
	out dx, al
	mov al, ah
	rep stosb
}
	DORIGHTEDGE:
asm {
	mov al, bl
	out dx, al
	mov al, ah
	stosb
}
	FILLLOOPBOTTOM:
asm {
	add di, bp
	pop cx
	dec si
	jnz FILLROWSLOOP
}
	END:
asm {
	pop bp
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
void g_scroll_arr256x(int fila, int columna, int ancho, int alto,
  unsigned char color)
{
unsigned char far *pvideo;
unsigned char far *pvid2;
int bytesfila = g_bytesfila, chr_alto = g_chr_alto ;

/* direcci¢n en buffer de v¡deo de zona a desplazar */
pvideo=(unsigned char far *)0xa0000000L+((fila*g_bytesfila*g_chr_alto)+(columna*2));
/* puntero auxiliar a l¡nea siguiente */
pvid2=pvideo+(g_bytesfila*g_chr_alto);
ancho*=2;
alto=(alto-1)*g_chr_alto;

asm {
	push di
	push si
	push ds

	mov dx,03c4h		// DX=puerto del secuenciador
	mov ax,0f02h		// seleccionar todos los planos
	out dx,ax

	mov dx,03ceh            // DX=puerto del controlador gr fico
	mov ax,0008h            // reg. m scara de bit=00h
	out dx,ax

	les di,pvideo           // direcci¢n de inicio
	lds si,pvid2            // puntero a l¡nea siguiente

	mov cx,alto             // n£mero de l¡neas a desplazar
}
scr_arr256x:
asm {
	push cx
	push di
	push si
	mov cx,ancho            // transfiere l¡nea siguiente a actual
	rep movsb
	pop si
	pop di
	pop cx

	add di,bytesfila               // siguiente l¡nea
	add si,bytesfila

	dec cx ; jnz scr_arr256x        // repite hasta desplazar todo bloque

	mov ax,0ff08h           // reg. m scara de bit=ffh
	out dx,ax

	mov al,color		// AL=color de relleno
	mov cx,chr_alto      	// CX=n£mero de l¡neas a rellenar
}
scr_arr_rell256x:
asm {
	push cx
	push di
	mov cx,ancho
	rep stosb               // rellena l¡nea con 'color'
	pop di
	pop cx

	add di,bytesfila					// siguiente l¡nea
	dec cx ; jnz scr_arr_rell256x

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
void g_scroll_abj256x(int fila, int columna, int ancho, int alto,
  unsigned char color)
{
unsigned char far *pvideo;
unsigned char far *pvid2;
int bytesfila = g_bytesfila, chr_alto = g_chr_alto ;

/* direcci¢n en buffer de v¡deo de final de zona a desplazar */
pvideo=(unsigned char far *)0xa0000000L+(((((fila+alto)*g_chr_alto)-1)*g_bytesfila)+
  (columna*2));
/* puntero auxiliar a l¡nea anterior */
pvid2=pvideo-(g_bytesfila*g_chr_alto);
ancho*=2;
alto=(alto-1)*g_chr_alto;

asm {
	push di
	push si
	push ds

	mov dx,03c4h		// DX=puerto del secuenciador
	mov ax,0f02h		// activa todos los planos
	out dx,ax

	mov dx,03ceh            // DX=puerto del controlador gr fico
	mov ax,0008h            // reg. m scara de bit=00h
	out dx,ax

	les di,pvideo           // direcci¢n de inicio
	lds si,pvid2            // puntero a l¡nea anterior

	mov cx,alto		// n£mero de l¡neas a desplazar
}
scr_abj256x:
asm {
	push cx
	push di
	push si
	mov cx,ancho            // transfiere l¡nea anterior a actual
	rep movsb
	pop si
	pop di
	pop cx

	sub di,bytesfila					// l¡nea anterior
	sub si,bytesfila

	dec cx ; jnz scr_abj256x        // repite hasta desplazar todo bloque

	mov ax, bytesfila
	mov bx, chr_alto
	dec bx
	mul bl
	sub di,ax		// ES:DI=dir. origen primera l¡nea
				// 1350=90*(16-1)

	mov ax,0ff08h           // reg. m scara de bit=ffh
	out dx,ax

	mov al,color		// AL=color de relleno
	mov cx,chr_alto      	// CX=n£mero de l¡neas a rellenar
}
scr_abj_rell256x:
asm {
	push cx
	push di
	mov cx,ancho
	rep stosb               // rellena l¡nea con 'color'
	pop di
	pop cx

	add di,bytesfila				  // siguiente l¡nea
	dec cx ; jnz scr_abj_rell256x

	pop ds
	pop si
	pop di
}
}

/****************************************************************************
	BLQ_COGE256X: recoge un bloque de pantalla en el modo 360x480x256
	  y lo guarda.
	  Entrada:      'x0', 'y0' esquina superior izquierda del bloque
			'x1', 'y1' esquina inferior derecha del bloque
			'blq' puntero a buffer para guardar bloque
****************************************************************************/
void blq_coge256x(int x0, int y0, int x1, int y1, unsigned char far *blq)
{
int bytesfila = g_bytesfila ;

asm {
	push di
	push si
	push ds

	mov ax,y0
	mov dx,g_bytesfila
	mul dx		// AX=90*y0
	mov bx,x0
	shr bx,1
	shr bx,1	// BX=x0/4
	add bx,ax	// BX=(90*y0)+(x0/4), desplazamiento en buffer v¡deo
	mov ax,0a000h
	mov ds,ax
	mov si,bx	// DS:SI=direcci¢n en buffer de v¡deo
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

	mov dx,03ceh	// puerto del controlador de gr ficos
}
sgte_fila:
asm {
	push si
	push cx
	mov ax,x0
	and ax,3
	mov ah,al       // AH=n§ de plano de bits
}
sgte_byte1:
asm {
	mov al,4
	out dx,ax	// selecciona plano

	mov al,ds:[si]
	stosb

	cmp ah,3        // comprueba si hay que cambiar de byte
	je sgte_byte2
	inc ah		// siguiente plano de bits
	dec cx ; jnz sgte_byte1
	jmp sgte_byte3
}
sgte_byte2:
asm {
	xor ah,ah
	inc si
	dec cx ; jnz sgte_byte1
}
sgte_byte3:
asm {
	pop cx
	pop si

	add si,bytesfila		// pasa a siguiente fila
	dec bx
	jnz sgte_fila

	pop ds
	pop si
	pop di
}

}

/****************************************************************************
	BLQ_PON256X: pone un bloque en pantalla en el modo 360x480x256.
	  Entrada:      'x', 'y' posici¢n de pantalla donde se pondr  el
			bloque
			'blq' puntero a buffer que contiene el bloque
****************************************************************************/
void blq_pon256x(int x, int y, unsigned char far *blq)
{
unsigned char masc_pln;
int bytesfila = g_bytesfila ;

asm {
	push di
	push si
	push ds

	mov dx,03ceh	// puerto del registro de direcciones
	mov ax,0003h
	out dx,ax	// inicializa registro seleccionar funci¢n

	mov ax,y
	mov dx,bytesfila
	mul dx		// AX=90*y
	mov bx,x
	shr bx,1
	shr bx,1	// BX=x/4
	add bx,ax	// BX=(90*y)+(x/4), desplazamiento en buffer v¡deo
	mov ax,0a000h
	mov es,ax
	mov di,bx	// ES:DI=direcci¢n en buffer de v¡deo
	lds si,blq      // DS:SI=direcci¢n de buffer donde est  el bloque

	mov cx,x
	and cx,3
	mov ax,1
	shl ax,cl
	mov masc_pln,al	// m scara planos de bit

	lodsw
	mov cx,ax       // CX=anchura del bloque
	lodsw
	mov bx,ax       // BX=altura del bloque

	mov dx,03c4h	// puerto del secuenciador
}
sgte_fila:
asm {
	push si
	push di
	push bx
	push cx
	mov ah,masc_pln
	mov bx,0
	add cx, si
}
sgte_byte1:
asm {
	mov al,2
	out dx,ax	// selecciona plano
	push di
	push si
}
sgte_byte2:
asm {
	movsb
	add si,3
	cmp si,cx
	jb sgte_byte2

	pop si
	pop di
	inc si
	cmp si, cx
	jae sgte_byte3
	inc bx
	cmp bx,4	// comprueba si hay que pasar a siguiente l¡nea
	je sgte_byte3

	cmp ah,8        // comprueba si est  en £ltimo plano
	je primer_plano
	shl ah,1	// siguiente plano de bits
	jmp sgte_byte1
}
primer_plano:
asm {
	inc di
	mov ah,1
	jmp sgte_byte1
}
sgte_byte3:
asm {
	pop cx
	pop bx
	pop di
	pop si

	add di,bytesfila		// pasa a siguiente fila
	add si,cx
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
void blq_clip256x(int x, int y, int ancho, int alto, unsigned char far *blq)
{
unsigned char masc_pln;
int jumpx = *(int *)blq - ancho ;
int bytesfila = g_bytesfila ;
if (jumpx < 0) jumpx = 0, ancho = *(int *)blq ;

asm {
	push di
	push si
	push ds

	mov dx,03ceh	// puerto del registro de direcciones
	mov ax,0003h
	out dx,ax	// inicializa registro seleccionar funci¢n

	mov ax,y
	mov dx,bytesfila
	mul dx		// AX=90*y
	mov bx,x
	shr bx,1
	shr bx,1	// BX=x/4
	add bx,ax	// BX=(90*y)+(x/4), desplazamiento en buffer v¡deo
	mov ax,0a000h
	mov es,ax
	mov di,bx	// ES:DI=direcci¢n en buffer de v¡deo
	lds si,blq      // DS:SI=direcci¢n de buffer donde est  el bloque

	mov cx,x
	and cx,3
	mov ax,1
	shl ax,cl
	mov masc_pln,al	// m scara planos de bit

	lodsw
	mov cx,ancho    // CX=anchura del bloque
	lodsw
	mov bx,alto     // BX=altura del bloque

	mov dx,03c4h	// puerto del secuenciador
}
sgte_fila:
asm {
	push si
	push di
	push bx
	push cx
	mov ah,masc_pln
	mov bx,0
	add cx, si
}
sgte_byte1:
asm {
	mov al,2
	out dx,ax	// selecciona plano
	push di
	push si
}
sgte_byte2:
asm {
	movsb
	add si,3
	cmp si,cx
	jb sgte_byte2

	pop si
	pop di
	inc si
	cmp si, cx
	jae sgte_byte3
	inc bx
	cmp bx,4	// comprueba si hay que pasar a siguiente l¡nea
	je sgte_byte3

	cmp ah,8        // comprueba si est  en £ltimo plano
	je primer_plano
	shl ah,1	// siguiente plano de bits
	jmp sgte_byte1
}
primer_plano:
asm {
	inc di
	mov ah,1
	jmp sgte_byte1
}
sgte_byte3:
asm {
	pop cx
	pop bx
	pop di
	pop si

	add di,bytesfila		// pasa a siguiente fila
	add si,cx
	add si,jumpx
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

int imp_chr256x(unsigned char chr, unsigned char colorf, unsigned char color,
  unsigned char modo)
{
unsigned char far *pchr;
unsigned char masc, masci, mascd;
int x, y, maxx, maxy, alt, anch, varshift;
int bytesfila = g_bytesfila-2 ;

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

/* si car cter no cabe, lo pasa a siguiente l¡nea */
if((x+anch-1)>maxx) {
	x=0;
	y+=alt;
	if((y+alt-1)>maxy) y=0;
}

/* m scara de car cter */
masc=0xff>>anch;

/* direcci¢n de car cter */
if (alt == 16)
	pchr=_defchr+(chr*g_chr_alto);
else
	pchr=_defchr8x8+(chr*g_chr_alto);

asm {
	push di
	push si
	push ds

	mov dx,03ceh	// puerto del registro de direcciones
	mov ah,modo
	mov al,3	// AL=registro rotar dato/seleccionar funci¢n
	out dx,ax	// inicializa registro seleccionar funci¢n

	mov ax,y
	mov dx,g_bytesfila
	mul dx		// AX=90*y
	mov bx,x
	shr bx,1
	shr bx,1 	// BX=x/4
	add bx,ax	// BX=(90*y)+(x/4)
	mov ax,0a000h
	mov es,ax
	mov di,bx	// ES:DI=dir. en buffer de v¡deo
	lds si,pchr     // DS:SI=dir. definici¢n car cter

	mov cx,x
	and cx,3
	mov ax,1
	shl ax,cl
	mov ah,al	// AH=m scara planos de bit

	mov cx,alt      // CX=altura en pixels del car cter
	mov bl,color   	// BL=valor pixel de imagen
	mov bh,colorf  	// BH=valor pixel de fondo
}
ic256x_1:
asm {
	push cx
	push ax
	mov cl,8        // CL=anchura car cter en pixels
	lodsb           // AL=byte de car cter
	mov dh,al       // DH=patr¢n de bit para sgte. fila
	mov dl,masc	// m scara car cter
}
ic256x_2:
asm {
	push ax
	push dx
	mov dx,03c4h	// puerto del secuenciador
	mov al,2
	out dx,ax	// selecciona plano
	pop dx
	pop ax

	mov al,bl       // AL=valor de pixel imagen
	shl dh,1        // CARRY=bit alto
	jc ic256x_3     // salta si es pixel de imagen
	mov al,bh       // AL=valor de pixel fondo
}
ic256x_3:
asm {
	shl dl,1	// CARRY=bit alto
	jc no_imp256x

	mov ch,es:[di]	// actualiza 'latches'
	mov es:[di],al
}
no_imp256x:
asm {
	cmp ah,8	// comprueba si hay que cambiar de byte
	je ic256x_4
	shl ah,1	// siguiente plano de bits
	dec cl
	jnz ic256x_2
	jmp sgte_lin256x
}
ic256x_4:
asm {
	mov ah,1
	inc di
	dec cl
	jnz ic256x_2
}
sgte_lin256x:
asm {
	add di,bytesfila     	// siguiente l¡nea en pantalla
	pop ax
	pop cx
	dec cx ; jnz ic256x_1

	mov dx,03ceh	// puerto del registro de direcciones
	mov ax,0003h	// inicializa registro seleccionar funci¢n
	out dx,ax

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
