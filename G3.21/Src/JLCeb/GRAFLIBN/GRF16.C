/*

	Librer¡a gr fica SINTAC

	(c) 1995 Javier San Jos‚ y Jos‚ Luis Cebri n
								*/

/* G_MM_16C (modos BIOS de 16 colores) */

#include "grf.h"

extern int g_bytesfila ;

extern unsigned char *_anchchr ;
extern unsigned char *_defchr ;
extern unsigned char *_anchchr8x8 ;
extern unsigned char *_defchr8x8 ;

/*** Macros ***/
/* calcula direcci¢n de pixel en buffer de v¡deo en los modos de 16 colores */
/* a la entrada: AX=coordenada Y, BX=coordenada X */
/* a la salida: AH=m scara de bit, CL=n§ de bits a desplazar a izquierda */
/*              ES:BX=direcci¢n en buffer de v¡deo */
#define DIR_PIXEL16	asm { 			 \
				mov cl,bl	;\
				push dx		;\
				mov dx,g_bytesfila	;\
				mul dx          ;\
				pop dx		;\
				shr bx,1	;\
				shr bx,1	;\
				shr bx,1	;\
				add bx,ax	;\
				mov ax,0a000h	;\
				mov es,ax	;\
				and cl,7	;\
				xor cl,7        ;\
				mov ah,1	;\
			}

/****************************************************************************
	G_BORRA_PANTALLA: borra la pantalla.
****************************************************************************/
void g_borra_pantalla16(void)
{
int filas_pixels;
	/* calcula filas de pixels */
	filas_pixels=(g_max_y+1)*g_bytesfila/2;

	asm {
		push di

		mov di,0a000h
		mov es,di
		xor di,di	// ES:DI=direcci¢n inicio buffer de v¡deo

		mov dx,03ceh	// puerto del controlador de gr ficos
		mov ax,0	// AL=n£mero registro set/reset, AH=0 (borrar)
		out dx,ax
		mov ax,0f01h	// AH=m scara activaci¢n set/reset
				// AL=n£mero registro activaci¢n set/reset
		out dx,ax

		mov cx,filas_pixels
		rep stosw	// rellena buffer de v¡deo

		mov ax,0001h	// inicializar registro activaci¢n set/reset
		out dx,ax

		pop di
	}
}


/****************************************************************************
	G_COGE_PIXEL: devuelve el color de un pixel.
	  Entrada:	'x', 'y' coordenadas del pixel
	  Salida:	color del pixel
****************************************************************************/
unsigned char g_coge_pixel16(int x, int y)
{
unsigned char color ;

asm {
	push si

	mov ax,y
	mov bx,x
}
	DIR_PIXEL16;	// AH=m scara de bit
			// ES:BX=direcci¢n en buffer v¡deo
			// CL=cantidad de bits a desplazar a izquierda
asm {
	mov ch,ah
	shl ch,cl	// CH=m scara de bit en posici¢n adecuada

	mov si,bx	// ES:SI=direcci¢n en buffer de v¡deo
	xor bl,bl	// BL se utiliza para acumular valor pixel

	mov dx,03ceh	// puerto del controlador de gr ficos
	mov ax,0304h	// AH=n£mero de plano de bits inicial
			// AL=n£mero registro selecci¢n de mapa lectura
}
cogepixel16:
asm {
	out dx,ax     	// selecciona plano de bits
	mov bh,es:[si]	// BH=bytes desde plano actual
	and bh,ch	// enmascara bit
	neg bh		// bit 7 de BH=1 si bit enmascarado es 1
			// bit 7 de BH=0 si bit enmascarado es 0
	rol bx,1	// bit 0 de BL=siguiente bit de valor pixel
	dec ah		// AH=siguiente plano de bits
	jge cogepixel16

	mov color,bl

	pop si
}

return(color);
}

/****************************************************************************
	G_PUNTO16: dibuja un punto en los modos de 16 colores.
	  Entrada:	'x', 'y' coordenadas
			'color' color del punto
			'modo' modo de dibujo (G_NORM, G_XOR, G_OR, G_AND)
****************************************************************************/
void g_punto16(int x, int y, unsigned char color, unsigned char modo)
{

asm {
	mov ax,y
	mov bx,x
}
	DIR_PIXEL16;	// AH=m scara de bit, ES:BX=direcci¢n en buffer v¡deo
			// CL=cantidad de bits a desplazar a la izquierda
asm {
	shl ah,cl	// AH=m scara de bit en posici¢n correcta

	mov dx,03ceh	// puerto del controlador gr fico
	mov al,8	// AL=n£mero registro de m scara de bit
	out dx,ax

	mov ax,0205h	// AL=n£mero de registro de modo
			// AH=modo escritura 2, lectura 0
	out dx,ax

	mov ah,modo
	mov al,3	// AL=registro rotar dato/seleccionar funci¢n
	out dx,ax

	mov al,es:[bx]	// guarda un byte desde cada plano de bits
	mov al,color
	mov es:[bx],al	// actualiza planos de bits

	mov ax,0ff08h	// inicializa m scara de bit
	out dx,ax
	mov ax,0005h	// inicializa registro de modo
	out dx,ax
	mov ax,0003h	// inicializa registro seleccionar funci¢n
	out dx,ax
}

}

/****************************************************************************
	G_LINEA16: dibuja una l¡nea en los modos de 16 colores.
	  Entrada:	'x0', 'y0' punto de origen
			'x1', 'y1' punto final
			'color' color de la l¡nea
			'modo' modo de dibujo (G_NORM, G_XOR, G_OR, G_AND)
****************************************************************************/
void g_linea16(int x0, int y0, int x1, int y1, unsigned char color,
  unsigned char modo)
{
int incvert, incr1, incr2, rutina;

asm {
	push si
	push di
	push ds

	mov dx,03ceh	// DX=puerto del controlador de gr ficos
	mov ah,color
	xor al,al	// AL=n£mero de registro set/reset
	out dx,ax

	mov ax,0f01h	// AH=m scara de plano de bit
			// AL=n£mero registro activaci¢n set/reset
	out dx,ax

	mov ah,modo	// modo de dibujo: 18h=XOR, 10h=OR, 08h=AND, 00h=NORM
	mov al,3	// AL=n£mero registro rotar dato/selecc. funci¢n
	out dx,ax

	mov si,g_bytesfila	// incremento para buffer de v¡deo
	mov cx,x1
	sub cx,x0	// CX=x1-x0
	jnz lin256_00
	jmp linea_vert16 	// salta si l¡nea vertical
}
lin256_00:
asm {
	jns lin16_01	// salta si x1>x0

	neg cx		// CX=x0-x1
	mov bx,x1
	xchg bx,x0
	mov x1,bx       // intercambia x0 y x1
	mov bx,y1
	xchg bx,y0
	mov y1,bx	// intercambia y0 e y1
}
lin16_01:
asm {
	mov bx,y1
	sub bx,y0	// BX=y1-y0
	jnz lin256_02
	jmp linea_horz16	// salta si l¡nea horizontal
}
lin256_02:
asm {
	jns lin16_03	// salta si pendiente positiva

	neg bx		// BX=y0-y1
	neg si		// incremento negativo para buffer de v¡deo
}
lin16_03:
asm {
	mov incvert,si	// guarda incremento vertical
	mov rutina,0
	cmp bx,cx
	jle lin16_04	// salta si dy<=dx (pendiente<=1)
	mov rutina,1
	xchg bx,cx	// intercambia dy y dx
}
lin16_04:
asm {
	shl bx,1
	mov incr1,bx    // incr1=2*dy
	sub bx,cx
	mov si,bx	// SI=d=2*dy-dx
	sub bx,cx
	mov incr2,bx	// incr2=2*(dy-dx)

	push cx
	mov ax,y0
	mov bx,x0
}
	DIR_PIXEL16;	// AH=m scara de bit, ES:BX=direcci¢n en buffer v¡deo
			// CL=cantidad de bits a desplazar a la izquierda
asm {
	mov di,bx	// ES:DI=direcci¢n en buffer de v¡deo
	shl ah,cl	// AH=m scara de bit desplazada
	mov bl,ah
	mov al,8	// AL=n£mero registro de m scara de bit
	pop cx
	inc cx		// CX=cantidad de pixels a dibujar

	cmp rutina,1
	je pendiente1
	jmp lin16_10
}
pendiente1:
asm {
	jmp lin16_15
}
linea_vert16:
asm {
	mov ax,y0
	mov bx,y1
	mov cx,bx
	sub cx,ax	// CX=dy
	jge lin16_05	// salta si dy>=0

	neg cx		// fuerza dy>=0
	mov ax,bx
}
lin16_05:
asm {
	inc cx		// CX=cantidad de pixels a dibujar
	mov bx,x0
	push cx
}
	DIR_PIXEL16;	// AH=m scara de bit, ES:BX=direcci¢n en buffer v¡deo
			// CL=cantidad de bits a desplazar a la izquierda
asm {
	shl ah,cl	// AH=m scara desplazada
	mov al,8	// AL=n£mero registro m scara de bit
	out dx,ax
	pop cx
}
lin16_06:
asm {
	or es:[bx],al	// activa pixel
	add bx,si	// siguiente pixel
	dec cx ; jnz lin16_06
	jmp fin16
}
linea_horz16:
asm {
	push ds
	mov ax,y0
	mov bx,x0
}
	DIR_PIXEL16;	// AH=m scara de bit, ES:BX=direcci¢n en buffer v¡deo
			// CL=cantidad de bits a desplazar a la izquierda
asm {
	mov di,bx	// ES:DI=direcci¢n en buffer de v¡deo
	mov dh,ah
	not dh
	shl dh,cl
	not dh          // DH=m scara para primer byte
	mov cx,x1
	and cl,7
	xor cl,7
	mov dl,0ffh
	shl dl,cl	// DL=m scara para £ltimo byte

	mov ax,x1
	mov bx,x0
	mov cl,3
	shr ax,cl
	shr bx,cl
	mov cx,ax
	sub cx,bx	// CX=n£mero de bytes por l¡nea-1

	mov bx,dx
	mov dx,03ceh	// DX=puerto del controlador de gr ficos
	mov al,8	// AL=n£mero registro de m scara de bit

	push es
	pop ds
	mov si,di	// DS:SI=direcci¢n en buffer de v¡deo

	or bh,bh
	js lin16_08	// salta si alineado por byte

	or cx,cx
	jnz lin16_07	// salta si hay m s de un byte en la l¡nea

	and bl,bh 	// BL=m scara de bit para la l¡nea
	jmp lin16_09
}
lin16_07:
asm {
	mov ah,bh	// AH=m scara para primer byte
	out dx,ax
	movsb		// dibuja primer byte
	dec cx
}
lin16_08:
asm {
	mov ah,0ffh	// AH=m scara de bit
	out dx,ax
	rep movsb	// dibuja resto de la l¡nea
}
lin16_09:
asm {
	mov ah,bl	// AH=m scara para £ltimo byte
	out dx,ax
	movsb		// dibuja £ltimo byte
	pop ds
	jmp fin16
}
lin16_10:
asm {
	mov ah,bl	// AH=m scara de bit para siguiente pixel
}
lin16_11:
asm {
	or ah,bl 	// posici¢n de pixel actual en m scara
	ror bl,1
	jc lin16_13     // salta si m scara est  girada a la posici¢n de
			// pixel m s a la izquierda
	or si,si
	jns lin16_12	// salta si d>=0

	add si,incr1	// d=d+incr1
	dec cx ; jnz lin16_11

	out dx,ax
	or es:[di],al
	jmp fin16
}
lin16_12:
asm {
	add si,incr2	// d=d+incr2
	out dx,ax
	or es:[di],al
	add di,incvert	// incrementa y
	dec cx ; jnz lin16_10
	jmp fin16
}
lin16_13:
asm {
	out dx,ax
	or es:[di],al
	inc di		// incrementa x

	or si,si
	jns lin16_14	// salta si d>=0

	add si,incr1
	dec cx ; jnz lin16_10
	jmp fin16
}
lin16_14:
asm {
	add si,incr2
	add di,incvert
	dec cx ; jnz lin16_10
	jmp fin16
}
lin16_15:
asm {
	mov bx,incvert
}
lin16_16:
asm {
	out dx,ax
	or es:[di],al
	add di,bx	// incrementa y
	or si,si
	jns lin16_17	// salta si d>=0

	add si,incr1	// d=d+incr1
	dec cx ; jnz lin16_16
	jmp fin16
}
lin16_17:
asm {
	add si,incr2	// d=d+incr2
	ror ah,1
	adc di,0	// incrementa DI si la m scara est  girada a la
			// posici¢n de pixel m s a la izquieda
	dec cx ; jnz lin16_16
}
fin16:
asm {
	xor ax,ax
	out dx,ax	// inicializa registro set/reset
	inc ax
	out dx,ax	// inicializa registro activaci¢n set/reset
	mov al,3
	out dx,ax	// inicializa registro rotar dato/seleccionar funci¢n
	mov ax,0ff08h
	out dx,ax	// inicializa registro m scara de bit

	pop ds
	pop di
	pop si
}

}

/****************************************************************************
	G_RECTANGULO16: dibuja un rect ngulo s¢lido en los modos de 16
	  colores.
	  Entrada:	'x0', 'y0' esquina superior izquierda
			'x1', 'y1' esquina inferior derecha
			'color' color del rect ngulo
			'modo' modo de dibujo (G_NORM, G_XOR, G_OR, G_AND)
****************************************************************************/
void g_rectangulo16(int x0, int y0, int x1, int y1, unsigned char color,
  unsigned char modo)
{
int lineas, bytesfila = g_bytesfila ;

asm {
	push si
	push di
	push ds

	mov dx,03ceh	// DX=puerto del controlador de gr ficos
	mov ah,color
	xor al,al	// AL=n£mero de registro set/reset
	out dx,ax

	mov ax,0f01h	// AH=m scara de plano de bit
			// AL=n£mero registro activaci¢n set/reset
	out dx,ax

	mov ah,modo	// modo de dibujo: 18h=XOR, 10h=OR, 08h=AND, 00h=NORM
	mov al,3	// AL=n£mero registro rotar dato/selecc. funci¢n
	out dx,ax

	mov ax,y0
	mov bx,x0
}
	DIR_PIXEL16;	// AH=m scara de bit, ES:BX=direcci¢n en buffer v¡deo
			// CL=cantidad de bits a desplazar a la izquierda
asm {
	mov di,bx	// ES:DI=direcci¢n en buffer de v¡deo
	mov dh,ah
	not dh
	shl dh,cl
	not dh          // DH=m scara para primer byte
	mov cx,x1
	and cl,7
	xor cl,7
	mov dl,0ffh
	shl dl,cl	// DL=m scara para £ltimo byte

	mov ax,x1
	mov bx,x0
	mov cl,3
	shr ax,cl
	shr bx,cl
	mov cx,ax
	sub cx,bx	// CX=n£mero de bytes por l¡nea-1

	mov ax,y1
	sub ax,y0
	inc ax		// BX=n£mero de l¡neas
	mov lineas,ax

	mov bx,dx
	mov dx,03ceh	// DX=puerto del controlador de gr ficos
	mov al,8	// AL=n£mero registro de m scara de bit

	push es
	pop ds
	mov si,di	// DS:SI=direcci¢n en buffer de v¡deo
}
rect16_00:
asm {
	push si
	push di
	push cx

	or bh,bh
	js rect16_02	// salta si alineado por byte

	or cx,cx
	jnz rect16_01	// salta si hay m s de un byte en la l¡nea

	and bl,bh 	// BL=m scara de bit para la l¡nea
	jmp rect16_03
}
rect16_01:
asm {
	mov ah,bh	// AH=m scara para primer byte
	out dx,ax
	movsb		// dibuja primer byte
	dec cx
}
rect16_02:
asm {
	mov ah,0ffh	// AH=m scara de bit
	out dx,ax
	rep movsb	// dibuja resto de la l¡nea
}
rect16_03:
asm {
	mov ah,bl	// AH=m scara para £ltimo byte
	out dx,ax
	movsb		// dibuja £ltimo byte

	pop cx
	pop di
	pop si

	add si,bytesfila	// siguiente l¡nea
	add di,bytesfila

	dec lineas
	jnz rect16_00

	xor ax,ax
	out dx,ax	// inicializa registro set/reset
	inc ax
	out dx,ax	// inicializa registro activaci¢n set/reset
	mov al,3
	out dx,ax	// inicializa registro rotar dato/seleccionar funci¢n
	mov ax,0ff08h
	out dx,ax	// inicializa registro m scara de bit

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
void g_scroll_arr16(int fila, int columna, int ancho, int alto,
  unsigned char color)
{
unsigned char far *pvideo;
unsigned char far *pvid2;
int bytesfila = g_bytesfila, chr_alto = g_chr_alto ;

/* direcci¢n en buffer de v¡deo de zona a desplazar */
pvideo=(unsigned char far *)0xa0000000L+((fila*g_bytesfila*chr_alto)+columna);
/* puntero auxiliar a l¡nea siguiente */
pvid2=pvideo+(g_bytesfila*chr_alto);
alto=(alto-1)*chr_alto;

asm {
	push di
	push si
	push ds

	mov dx,03ceh            // DX=puerto del controlador gr fico
	mov ax,0008h            // reg. m scara de bit=00h
	out dx,ax

	les di,pvideo           // direcci¢n de inicio
	lds si,pvid2            // puntero a l¡nea siguiente

	mov cx,alto             // n£mero de l¡neas a desplazar
}
scr_arr16:
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

	dec cx ; jnz scr_arr16          // repite hasta desplazar todo bloque

	mov ah,color   		// AH=color de relleno
	mov al,0                // AL=registro set/reset
	out dx,ax
	mov ax,0f01h            // reg. activaci¢n set/reset=0fh
	out dx,ax
	mov ax,0ff08h           // reg. m scara de bit=ffh
	out dx,ax

	mov cx,chr_alto		// CX=n£mero de l¡neas a rellenar
}
scr_arr_rell16:
asm {
	push cx
	push di
	mov cx,ancho
	rep stosb               // rellena l¡nea con 'color'
	pop di
	pop cx

	add di,bytesfila               // siguiente l¡nea
	dec cx ; jnz scr_arr_rell16

	mov ax,0001h            // restaura reg. activaci¢n set/reset
	out dx,ax

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
void g_scroll_abj16(int fila, int columna, int ancho, int alto,
  unsigned char color)
{
unsigned char far *pvideo;
unsigned char far *pvid2;
int bytesfila = g_bytesfila, chr_alto = g_chr_alto ;

/* direcci¢n en buffer de v¡deo de final de zona a desplazar */
pvideo=(unsigned char far *)0xa0000000L+(((((fila+alto)*chr_alto)-1)*g_bytesfila)+
  columna);
/* puntero auxiliar a l¡nea anterior */
pvid2=pvideo-(g_bytesfila*chr_alto);
alto=(alto-1)*chr_alto;

asm {
	push di
	push si
	push ds

	mov dx,03ceh            // DX=puerto del controlador gr fico
	mov ax,0008h            // reg. m scara de bit=00h
	out dx,ax

	les di,pvideo           // direcci¢n de inicio
	lds si,pvid2            // puntero a l¡nea anterior

	mov cx,alto             // n£mero de l¡neas a desplazar
}
scr_abj16:
asm {
	push cx
	push di
	push si
	mov cx,ancho            // transfiere l¡nea anterior a actual
	rep movsb
	pop si
	pop di
	pop cx

	sub di,bytesfila               // l¡nea anterior
	sub si,bytesfila

	dec cx ; jnz scr_abj16          // repite hasta desplazar todo bloque

	mov ax, bytesfila
	mov bx, chr_alto
	dec bx
	mul bl
	sub di,ax  		// ES:DI=dir. origen primera l¡nea
				// 1200=80*(16-1)

	mov ah,color   		// AH=color de relleno
	mov al,0                // AL=registro set/reset
	out dx,ax
	mov ax,0f01h            // reg. activaci¢n set/reset=0fh
	out dx,ax
	mov ax,0ff08h           // reg. m scara de bit=ffh
	out dx,ax

	mov cx,chr_alto		// CX=n£mero de l¡neas a rellenar
}
scr_abj_rell16:
asm {
	push cx
	push di
	mov cx,ancho
	rep stosb               // rellena l¡nea con 'color'
	pop di
	pop cx

	add di,bytesfila               // siguiente l¡nea
	dec cx ; jnz scr_abj_rell16

	mov ax,0001h            // restaura reg. activaci¢n set/reset
	out dx,ax

	pop ds
	pop si
	pop di
}
}

/****************************************************************************
	BLQ_COGE16: recoge un bloque de pantalla en el modo 16 colores y
	  lo guarda.
	  Entrada:      'x0', 'y0' esquina superior izquierda del bloque
			'x1', 'y1' esquina inferior derecha del bloque
			'blq' puntero a buffer para guardar bloque
****************************************************************************/
void blq_coge16(int x0, int y0, int x1, int y1, unsigned char far *blq)
{
int filas, g_bytes_fila = g_bytesfila, bytes_fila ;

asm {
	push di
	push si
	push ds

	mov ax,x1
	sub ax,x0       // AX=x1-x0
	mov cx,0ff07h   // CH=m scara no desplazada, CL=m scara AND para AL
	and cl,al       // CL=pixels en £ltimo byte de fila
	xor cl,7        // CL=bits a desplazar
	shl ch,cl       // CH=m scara para £ltimo byte de fila
	mov cl,ch
	push cx
	mov cl,3
	shr ax,cl
	inc ax          // AX=bytes por fila de cada plano de bits
	push ax
	mov ax,y1
	sub ax,y0       // AX=y1-y0
	inc ax          // AX=n£mero de filas
	push ax

	mov ax,y0
	mov bx,x0
	mov cl,bl
	mov dx,g_bytes_fila
	mul dx          // AX=y0*80
	shr bx,1
	shr bx,1
	shr bx,1        // BX=x0/8
	add bx,ax       // BX=(y0*80)+(x0/8), desplazamiento en buffer v¡deo
	mov ax,0a000h
	mov ds,ax       // DS:BX=direcci¢n del byte
	and cl,7        // CL=n§ de bits a desplazar a la izquierda

	mov si,bx       // DS:SI=direcci¢n de bloque en buffer de v¡deo
	les di,blq      // ES:DI=direcci¢n de buffer para guardar bloque

	pop ax          // construye cabecera del bloque
	mov filas,ax    // n§ de filas
	stosw
	pop ax          // bytes por fila
	mov bytes_fila,ax
	stosw
	pop ax
	mov ch,al       // m scara para £ltimo byte de fila
	stosb

	mov dx,03ceh    // DX=puerto del controlador de gr ficos
	mov ax,0005h    // AH=modo de lectura 0, escritura 0
	out dx,ax
	mov ax,0304h    // AH=primer plano de bits a leer
}
sgte_plano:
asm {
	out dx,ax
	push ax
	push filas
	push si
}
sgte_fila:
asm {
	mov bx,bytes_fila
	push si
}
sgte_byte:
asm {
	lodsw
	dec si
	rol ax,cl       // AL=siguientes 4 pixels en la fila
	stosb           // copia en buffer
	dec bx
	jnz sgte_byte

	and es:[di-1],ch        // enmascara £ltimo byte de fila
	pop si
	add si,g_bytes_fila		 // DS:SI=siguiente fila
	dec filas
	jnz sgte_fila

	pop si          // DS:SI=inicio del bloque
	pop filas
	pop ax
	dec ah          // pasa a siguiente plano
	jns sgte_plano

	pop ds
	pop si
	pop di
}

}

/****************************************************************************
	BLQ_PON16: pone un bloque en pantalla en el modo 16 colores.
	  Entrada:      'x', 'y' posici¢n de pantalla donde se pondr  el
			bloque
			'blq' puntero a buffer que contiene el bloque
****************************************************************************/
void blq_pon16(int x, int y, unsigned char far *blq)
{
int filas, bytes_fila, masc_inic, masc_fini, masc_find, cont_fila;
int g_bytes_fila = g_bytesfila ;

asm {
	push di
	push si
	push ds

	mov ax,y
	mov bx,x
	mov cl,bl
	mov dx,g_bytes_fila
	mul dx          // AX=y*80
	shr bx,1
	shr bx,1
	shr bx,1        // BX=x/8
	add bx,ax       // BX=(y*80)+(x/8), desplazamiento en buffer v¡deo
	mov ax,0a000h
	mov es,ax       // ES:BX=direcci¢n del byte
	and cl,7
	xor cl,7
	inc cl
	and cl,7        // CL= n§ de bits a desplazar a la izquierda
	mov di,bx       // ES:DI=direcci¢n del bloque en buffer de v¡deo
	lds si,blq      // DS:SI=direcci¢n de buffer

	lodsw           // AX=n£mero de filas
	mov filas,ax
	lodsw           // AX=bytes por fila
	mov bytes_fila,ax
	lodsb           // AL=m scara para £ltimo byte de fila
	mov ch,al

	mov dx,03ceh    // DX=puerto del controlador de gr ficos
	mov ah,0        // AH=modo de dibujo: 18h=XOR, 10h=OR,
			// 08h=AND, 00h=NORM
	mov al,3
	out dx,ax
	mov ax,0805h    // AH=modo lectura 1, escritura 0
	out dx,ax
	mov ax,0007h    // AH=color don't care para todos los mapas
	out dx,ax
	mov ax,0ff08h   // AH=valor para registro de m scara de bits
	out dx,ax
	mov dl,0c4h     // DX=puerto del secuenciador
	mov ax,0802h    // AH=1000b, valor para registro de m scara de mapa

	cmp cx,0ff00h   // si m scara<>0ffh o bit a desplazar<>0
	jne blq_no_alin

	mov cx,bytes_fila
}
sgte_plano:
asm {
	out dx,ax
	push ax
	push di
	mov bx,filas
}
sgte_fila:
asm {
	push di
	push cx
}
sgte_byte:
asm {
	lodsb           // AL=bytes de pixels
	and es:[di],al  // actualiza plano de bits
	inc di
	dec cx ; jnz sgte_byte

	pop cx
	pop di
	add di,g_bytes_fila		 // DI=siguiente fila
	dec bx
	jnz sgte_fila

	pop di
	pop ax
	shr ah,1        // siguiente plano de bits
	jnz sgte_plano
	jmp blq_fin
}
blq_no_alin:
asm {
	push ax
	mov bx,00ffh    // BH=m scara para primer byte en fila, BL=0ffh
	mov al,ch       // AL=m scara para £ltimo byte en fila
	cbw             // AH=0ffh, m scara para pen£ltimo byte en fila

	cmp bytes_fila,1
	jne mbytes      // salta si hay m s de 1 byte por fila

	mov bl,ch
	mov ah,ch       // AH=m scara para pen£ltimo byte
	xor al,al       // AL=0, m scara para £ltimo byte
}
mbytes:
asm {
	shl ax,cl       // desplaza m scaras y las guarda con n§ de registro
	shl bx,cl       // de m scara de bit
	mov bl,al
	mov al,8
	mov masc_fini,ax
	mov ah,bl
	mov masc_find,ax
	mov ah,bh
	mov masc_inic,ax

	mov bx,bytes_fila
	pop ax
}
sgte_plano2:
asm {
	out dx,ax       // activa plano de bit
	push ax
	push di
	mov dl,0ceh     // DX=puerto del controlado de gr ficos
	mov ax,filas
	mov cont_fila,ax
}
sgte_fila2:
asm {
	push di
	push si
	push bx
	mov ax,masc_inic
	out dx,ax       // registro de m scara para primer byte de fila
	lodsw           // AH=segundo byte de pixels, AL=primer byte
	dec si          // DS:SI=segundo byte de pixels
	test cl,cl
	jnz no_izq      // salta si no alineado a la izquierda

	dec bx          // BX=bytes por fila-1
	jnz m2bytes     // salta si menos de 2 bytes por fila
	jmp m1byte      // salta si s¢lo hay 1 byte por fila
}
no_izq:
asm {
	rol ax,cl       // AH=parte izquierda de primer byte
			// AL=parte derecha
	and es:[di],ah  // pone parte izquierda de primer byte
	inc di
	dec bx          // BX=bytes por fila-2
}
m2bytes:
asm {
	push ax
	mov ax,0ff08h
	out dx,ax       // asigna m scara de bit para bytes sucesivos
	pop ax
	dec bx
	jng m1byte      // salta si 1 o 2 bytes en fila de pixels
}
sgte_byte2:
asm {
	and es:[di],al  // pone parte izquierda de byte actual y derecha
			// de siguiente
	inc di
	lodsw
	dec si
	rol ax,cl       // AH=parte izquierda, AL=parte derecha
	dec bx
	jnz sgte_byte2
}
m1byte:
asm {
	mov bx,ax       // BH=parte derecha £ltimo byte, izquierda pen£ltimo
			// BL=parte izquierda £ltimo, derecha pen£ltimo
	mov ax,masc_fini
	out dx,ax
	and es:[di],bl  // pone pen£ltimo byte
	mov ax,masc_find
	out dx,ax
	and es:[di+1],bh        // pone £ltimo byte

	pop bx
	pop si
	add si,bx       // DS:SI=siguiente fila
	pop di
	add di,g_bytes_fila		 // ES:DE=siguiente fila
	dec cont_fila
	jnz sgte_fila2

	pop di
	pop ax
	mov dl,0c4h
	shr ah,1        // siguiente plano de bits
	jnz sgte_plano2
}
blq_fin:
asm {
	mov ax,0f02h    // restaura valores por defecto
	out dx,ax
	mov dl,0ceh
	mov ax,0003h
	out dx,ax
	mov ax,0005h
	out dx,ax
	mov ax,0f07h
	out dx,ax
	mov ax,0ff08h
	out dx,ax

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
static char tmasc[8] = { 0xFF, 0xFE, 0xFC, 0xF8, 0xF0, 0xE0, 0xC0, 0x80 } ;

void blq_clip16(int x, int y, int ancho, int alto, unsigned char far *blq)
{
int masc_inic, masc_fini, masc_find, cont_fila;
int g_bytes_fila = g_bytesfila ;

int bytes_fila = ancho/8+((ancho&7)?1:0) ;
char nmasc = tmasc[bytes_fila*8-ancho] ;
int jumpx = *(int *)(blq+2)-bytes_fila ;
int jumpp = (*(int *)blq - alto) * *(int *)(blq+2) ;

if (jumpx < 0) jumpx = 0 ;
if (jumpp < 0) jumpp = 0 ;

asm {
	push di
	push si
	push ds

	mov ax,y
	mov bx,x
	mov cl,bl
	mov dx,g_bytes_fila
	mul dx          // AX=y*80
	shr bx,1
	shr bx,1
	shr bx,1        // BX=x/8
	add bx,ax       // BX=(y*80)+(x/8), desplazamiento en buffer v¡deo
	mov ax,0a000h
	mov es,ax       // ES:BX=direcci¢n del byte
	and cl,7
	xor cl,7
	inc cl
	and cl,7        // CL= n§ de bits a desplazar a la izquierda
	mov di,bx       // ES:DI=direcci¢n del bloque en buffer de v¡deo
	lds si,blq      // DS:SI=direcci¢n de buffer

	lodsw           // AX=n£mero de filas
	lodsw           // AX=bytes por fila
	lodsb           // AL=m scara para £ltimo byte de fila
	mov al, byte ptr nmasc
	mov ch, al

	mov dx,03ceh    // DX=puerto del controlador de gr ficos
	mov ah,0        // AH=modo de dibujo: 18h=XOR, 10h=OR,
			// 08h=AND, 00h=NORM
	mov al,3
	out dx,ax
	mov ax,0805h    // AH=modo lectura 1, escritura 0
	out dx,ax
	mov ax,0007h    // AH=color don't care para todos los mapas
	out dx,ax
	mov ax,0ff08h   // AH=valor para registro de m scara de bits
	out dx,ax
	mov dl,0c4h     // DX=puerto del secuenciador
	mov ax,0802h    // AH=1000b, valor para registro de m scara de mapa

	cmp cx,0ff00h   // si m scara<>0ffh o bit a desplazar<>0
	jne blq_no_alin

	mov cx,bytes_fila
}
sgte_plano:
asm {
	out dx,ax
	push ax
	push di
	mov bx,alto
}
sgte_fila:
asm {
	push di
	push cx
}
sgte_byte:
asm {
	lodsb           // AL=bytes de pixels
	and es:[di],al  // actualiza plano de bits
	inc di
	dec cx ; jnz sgte_byte

	pop cx
	pop di
	add si,jumpx
	add di,g_bytes_fila		 // DI=siguiente fila
	dec bx
	jnz sgte_fila

	pop di
	pop ax
	add si,jumpp
	shr ah,1        // siguiente plano de bits
	jnz sgte_plano
	jmp blq_fin
}
blq_no_alin:
asm {
	push ax
	mov bx,00ffh    // BH=m scara para primer byte en fila, BL=0ffh
	mov al,ch       // AL=m scara para £ltimo byte en fila
	cbw             // AH=0ffh, m scara para pen£ltimo byte en fila

	cmp bytes_fila,1
	jne mbytes      // salta si hay m s de 1 byte por fila

	mov bl,ch
	mov ah,ch       // AH=m scara para pen£ltimo byte
	xor al,al       // AL=0, m scara para £ltimo byte
}
mbytes:
asm {
	shl ax,cl       // desplaza m scaras y las guarda con n§ de registro
	shl bx,cl       // de m scara de bit
	mov bl,al
	mov al,8
	mov masc_fini,ax
	mov ah,bl
	mov masc_find,ax
	mov ah,bh
	mov masc_inic,ax

	mov bx,bytes_fila
	pop ax
}
sgte_plano2:
asm {
	out dx,ax       // activa plano de bit
	push ax
	push di
	mov dl,0ceh     // DX=puerto del controlado de gr ficos
	mov ax,alto
	mov cont_fila,ax
}
sgte_fila2:
asm {
	push di
	push si
	push bx
	mov ax,masc_inic
	out dx,ax       // registro de m scara para primer byte de fila
	lodsw           // AH=segundo byte de pixels, AL=primer byte
	dec si          // DS:SI=segundo byte de pixels
	test cl,cl
	jnz no_izq      // salta si no alineado a la izquierda

	dec bx          // BX=bytes por fila-1
	jnz m2bytes     // salta si menos de 2 bytes por fila
	jmp m1byte      // salta si s¢lo hay 1 byte por fila
}
no_izq:
asm {
	rol ax,cl       // AH=parte izquierda de primer byte
			// AL=parte derecha
	and es:[di],ah  // pone parte izquierda de primer byte
	inc di
	dec bx          // BX=bytes por fila-2
}
m2bytes:
asm {
	push ax
	mov ax,0ff08h
	out dx,ax       // asigna m scara de bit para bytes sucesivos
	pop ax
	dec bx
	jng m1byte      // salta si 1 o 2 bytes en fila de pixels
}
sgte_byte2:
asm {
	and es:[di],al  // pone parte izquierda de byte actual y derecha
			// de siguiente
	inc di
	lodsw
	dec si
	rol ax,cl       // AH=parte izquierda, AL=parte derecha
	dec bx
	jnz sgte_byte2
}
m1byte:
asm {
	mov bx,ax       // BH=parte derecha £ltimo byte, izquierda pen£ltimo
			// BL=parte izquierda £ltimo, derecha pen£ltimo
	mov ax,masc_fini
	out dx,ax
	and es:[di],bl  // pone pen£ltimo byte
	mov ax,masc_find
	out dx,ax
	and es:[di+1],bh        // pone £ltimo byte

	pop bx
	pop si
	add si,bx       // DS:SI=siguiente fila
	pop di
	add di,g_bytes_fila		 // ES:DE=siguiente fila
	add si,jumpx
	dec cont_fila
	jnz sgte_fila2

	pop di
	pop ax
	mov dl,0c4h
	add si,jumpp
	shr ah,1        // siguiente plano de bits
	jnz sgte_plano2
}
blq_fin:
asm {
	mov ax,0f02h    // restaura valores por defecto
	out dx,ax
	mov dl,0ceh
	mov ax,0003h
	out dx,ax
	mov ax,0005h
	out dx,ax
	mov ax,0f07h
	out dx,ax
	mov ax,0ff08h
	out dx,ax

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
int imp_chr16(unsigned char chr, unsigned char colorf, unsigned char color,
  unsigned char modo)
{
unsigned char far *pchr;
unsigned char masc, masci, mascd;
int x, y, maxx, maxy, alt, anch, varshift;
int bytesfila = g_bytesfila ;

/* coordenadas de impresi¢n */
x=ult_x;
y=ult_y;
maxx=g_max_x;
maxy=g_max_y;
alt=g_chr_alto;
if (alt == 16)
	anch = _anchchr[chr] ;
else
	anch = _anchchr8x8[chr] ;

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

/* invierte m scara */
masc^=0xff;

asm {
	push di
	push si
	push ds

	mov bx,x
	mov ax,y
	mov cl,bl
	mov dx,bytesfila
	mul dx
	shr bx,1
	shr bx,1
	shr bx,1
	add bx,ax
	mov ax,0a000h
	mov ds,ax
	mov si,bx		// DS:SI=dir. en buffer de v¡deo

	and cl,7
	xor cl,7       		// CL=n§ de bits a desplazar izq.
	inc cx
	and cl,7		// CL=n§ bits a despl. para enmascarar
	mov ch,0ffh
	shl ch,cl		// CH=m scara lado derecho de car cter
	mov varshift,cx

	les di,pchr             // ES:DI=dir. definici¢n de car cter
	mov cx,alt              // CX=altura en pixels del car cter

	// establecer registros del controlador de gr ficos

	mov dx,3ceh             // puerto del registro de direcciones
	mov ax,0a05h            // modo escritura 2, modo lectura 1
	out dx,ax
	mov ah,modo             // modo de dibujo: 18h=XOR, 10h=OR,
				// 08h=AND, 00h=NORM
	mov al,3
	out dx,ax
	mov ax,0007h            // color "don't care" todos los planos
	out dx,ax

	mov bl,color   		// BL=valor de pixel imagen
	mov bh,colorf  		// BH=valor de pixel fondo

	cmp byte ptr varshift,0
	jne chr_no_alin         // salta si car cter no alineado

	// rutina para caracteres alineados

	mov al,8                // AL=n§ registro de m scara de bit
}
chr_alin:
asm {
	mov ah,es:[di]          // AH=patr¢n de fila de pixels
	and ah,masc		// enmascara
	out dx,ax               // lo carga en registro m scara de bit
	and [si],bl             // actualiza pixels de imagen
	not ah
	and ah,masc		// enmascara
	out dx,ax
	and [si],bh             // actualiza pixels de fondo

	inc di                  // siguiente byte del car cter
	add si,bytesfila					// siguiente l¡nea de pantalla

	dec cx ; jnz chr_alin
	jmp chr_fin

	// rutina para caracteres no alineados
}
chr_no_alin:
asm {
	push cx                 // guarda contador n§ de l¡neas de car.
	mov cx,varshift         // CH=m scara lado derecho
				// CL=n§ de bits a desplazar
	mov al,masc		// AL=m scara de car cter
	xor ah,ah
	shl ax,cl		// AH = izquierdo, AL = derecho
	mov masci,ah
	mov mascd,al

	// dibuja lado izquierdo del car cter
	mov al,es:[di]          // AL=patr¢n de fila de pixels
	xor ah,ah
	shl ax,cl               // AH=patr¢n para lado izquierdo
				// AL=patr¢n para lado derecho
	push ax                 // guarda patrones
	mov al,8                // n§ de registro de m scara de bit
	and ah,masci		// enmascara
	out dx,ax               // carga patr¢n de lado izquierdo
	and [si],bl             // actualiza pixels de imagen
	not ch                  // CH=m scara para lado izquierdo
	xor ah,ch
	and ah,masci		// enmascara
	out dx,ax
	and [si],bh             // actualiza pixels de fondo
	// dibuja lado derecho del car cter
	pop ax                  // recupera patrones
	mov ah,al               // AH=patr¢n lado derecho
	mov al,8                // AL=n§ registro de m scara de bit
	and ah,mascd		// enmascara
	out dx,ax               // carga patr¢n
	inc si                  // posici¢n en buffer de v¡deo
	and [si],bl             // actualiza pixels de imagen
	not ch                  // CH=m scara para lado derecho
	xor ah,ch
	and ah,mascd		// enmascara
	out dx,ax
	and [si],bh             // actualiza pixels de fondo

	inc di                  // siguiente byte del car cter
	dec si
	add si,bytesfila					// siguiente l¡nea de pantalla

	pop cx
	dec cx ; jnz chr_no_alin

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
