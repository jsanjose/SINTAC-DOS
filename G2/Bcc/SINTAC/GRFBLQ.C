/****************************************************************************
				   GRFBLQ.C
	Conjunto de funciones para manejo de bloques gr ficos en los
	modos gr ficos de 16 y 256 colores de VGA.

			     (c)1993 JSJ Soft Ltd.

	Las siguientes funciones son p£blicas:
		- blq_tam: devuelve tama¤o de bloque
		- blq_coge: guarda bloque gr fico en memoria
		- blq_pon: dibuja un bloque
****************************************************************************/

#include <stddef.h>

/*** Prototipos de funciones internas ***/
static int modo_video(void);
static void blq_coge16(short x0, short y0, short x1, short y1,
  unsigned char far *blq);
static void blq_coge256(short x0, short y0, short x1, short y1,
  unsigned char far *blq);
static void blq_pon16(short x, short y, unsigned char far *blq);
static void blq_pon256(short x, short y, unsigned char far *blq);

/****************************************************************************
	MODO_VIDEO: devuelve el modo de v¡deo actual.
	  Salida:       modo de v¡deo actual
****************************************************************************/
int modo_video(void)
{
int mvid;

asm {
	mov ah,0fh
	int 10h         // AH=n§ de columnas, AL=modo de v¡deo
	mov ah,0
	mov mvid,ax
}

return(mvid);
}

/****************************************************************************
	BLQ_COGE16: recoge un bloque de pantalla en el modo 16 colores y
	  lo guarda.
	  Entrada:      'x0', 'y0' esquina superior izquierda del bloque
			'x1', 'y1' esquina inferior derecha del bloque
			'blq' puntero a buffer para guardar bloque
****************************************************************************/
void blq_coge16(short x0, short y0, short x1, short y1,
  unsigned char far *blq)
{
int filas, bytes_fila;

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
	inc ax          // AX=bytes por fila
	push ax
	mov ax,y1
	sub ax,y0       // AX=y1-y0
	inc ax          // AX=n£mero de filas
	push ax

	mov ax,y0
	mov bx,x0
	mov cl,bl
	mov dx,80
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
	add si,80       // DS:SI=siguiente fila
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
	BLQ_COGE256: recoge un bloque de pantalla en el modo 256 colores y
	  lo guarda.
	  Entrada:      'x0', 'y0' esquina superior izquierda del bloque
			'x1', 'y1' esquina inferior derecha del bloque
			'blq' puntero a buffer para guardar bloque
****************************************************************************/
void blq_coge256(short x0, short y0, short x1, short y1,
  unsigned char far *blq)
{

asm {
	push di
	push si
	push ds

	mov ax,y0
	mov bx,x0
	xchg ah,al      // AX=256*y0
	add bx,ax       // BX=(256*y0)+x0
	shr ax,1
	shr ax,1        // AX=64*y0
	add bx,ax       // BX=(320*y0)+x0, desplazamiento en buffer de v¡deo
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
	rep movsb
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
	BLQ_PON16: pone un bloque en pantalla en el modo 16 colores.
	  Entrada:      'x', 'y' posici¢n de pantalla donde se pondr  el
			bloque
			'blq' puntero a buffer que contiene el bloque
****************************************************************************/
void blq_pon16(short x, short y, unsigned char far *blq)
{
int filas, bytes_fila, masc_inic, masc_fini, masc_find, cont_fila;

asm {
	push di
	push si
	push ds

	mov ax,y
	mov bx,x
	mov cl,bl
	mov dx,80
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
	loop sgte_byte

	pop cx
	pop di
	add di,80       // DI=siguiente fila
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
	add di,80       // ES:DE=siguiente fila
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
	BLQ_PON256: pone un bloque en pantalla en el modo 256 colores.
	  Entrada:      'x', 'y' posici¢n de pantalla donde se pondr  el
			bloque
			'blq' puntero a buffer que contiene el bloque
****************************************************************************/
void blq_pon256(short x, short y, unsigned char far *blq)
{

asm {
	push di
	push si
	push ds

	mov ax,y
	mov bx,x
	xchg ah,al      // AX=256*y
	add bx,ax       // BX=(256*y)+x
	shr ax,1
	shr ax,1        // AX=64*y
	add bx,ax       // BX=(320*y)+x, desplazamiento en buffer de v¡deo
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
	rep movsb
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
	BLQ_TAM: calcula el tama¤o (en bytes) de un bloque gr fico.
	  Entrada:      'x0', 'y0' esquina superior izquierda del bloque
			'x1', 'y1' esquina inferior derecha del bloque
	  Salida:       tama¤o del bloque en bytes
****************************************************************************/
unsigned long blq_tam(short x0, short y0, short x1, short y1)
{
unsigned long tam;

if(modo_video()==0x13) tam=5+((x1-x0+1)*(y1-y0+1));
else tam=5+((((x1-x0+1)/2)+1)*(y1-y0+1));

return(tam);
}

/****************************************************************************
	BLQ_COGE: recoge un bloque de pantalla y lo guarda.
	  Entrada:      'x0', 'y0' esquina superior izquierda del bloque
			'x1', 'y1' esquina inferior derecha del bloque
			'blq' puntero a buffer para guardar bloque
****************************************************************************/
void blq_coge(short x0, short y0, short x1, short y1, unsigned char far *blq)
{

if(blq==NULL) return;

if(modo_video()==0x13) blq_coge256(x0,y0,x1,y1,blq);
else blq_coge16(x0,y0,x1,y1,blq);

}

/****************************************************************************
	BLQ_PON: pone un bloque en pantalla.
	  Entrada:      'x', 'y' posici¢n de pantalla donde se pondr  el
			bloque
			'blq' puntero a buffer que contiene el bloque
****************************************************************************/
void blq_pon(short x, short y, unsigned char far *blq)
{

if(blq==NULL) return;

if(modo_video()==0x13) blq_pon256(x,y,blq);
else blq_pon16(x,y,blq);

}
