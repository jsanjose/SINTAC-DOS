/****************************************************************************
				     WING.C
	Conjunto de funciones para gestionar impresi¢n por pantalla en
	ventanas, en los modos gr ficos de 16 y 256 colores de VGA.

			     (c)1993 JSJ Soft Ltd.

	Las siguientes funciones son p£blicas:
		- w_scroll: hace scroll de una ventana
		- w_crea: crea una ventana
		- w_cls: borra/inicializa una ventana
		- w_impc: imprime un car cter en una ventana
		- w_imps: imprime una cadena en una ventana
		- w_impn: imprime un n£mero en una ventana
		- w_input: permite la entrada de datos por teclado
		- w_lee_tecla: espera hasta que se pulse una tecla y la
		    devuelve

	Las siguientes estructuras est n definidas en WING.H
		STC_WINDOW: para definir una ventana

****************************************************************************/

#include <graph.h>
#include <bios.h>
#include "impchr.h"
#include "wing.h"

/*** Variables globales ***/
/* lista de teclas admitidos por la funci¢n w_lee_tecla() */
const char Cod_Teclas[]="ABCDEFGHIJKLMN¥OPQRSTUVWXYZ"
			"abcdefghijklmn¤opqrstuvwxyz ‚¡¢£"
			"0123456789"
			".,;:!­?¨\"\'\r\b "
			"\xff\xfe\xfd\xfc\xfb\xfa\xf9\xf8\xf7\xf6\xf5"
			"\xf4\xf3\xf2\xf1\xf0\xef";
/* diferentes tipos de bordes para ventanas */
const char Borde[][6] = {
	"Ä³Ú¿ÀÙ",
	"ÍºÉ»È¼",
	"Í³Õ¸Ô¾",
	"ÄºÖ·Ó½",
	"°°°°°°",
	"±±±±±±",
	"²²²²²²",
	"ÛÛÛÛÛÛ",
};

/*** Prototipos de funciones internas ***/
static void w_imp_borde(STC_WINDOW *w);
static char w_mayuscula(char c);
static int w_esta_en(const char *s, char c);

/****************************************************************************
	W_IMP_BORDE: imprime el borde de una ventana.
	  Entrada:      'w' puntero a ventana.
****************************************************************************/
void w_imp_borde(STC_WINDOW *w)
{
short i, j;

if(w->borde) {
	/* esquina superior izquierda */
	_moveto(w->wx*8,w->wy*w->chralt);
	imp_chr(Borde[w->borde-1][2],w->colorf,w->color,CHR_NORM);

	/* esquina inferior izquierda */
	_moveto(w->wx*8,(w->wy+w->ly-1)*w->chralt);
	imp_chr(Borde[w->borde-1][4],w->colorf,w->color,CHR_NORM);

	/* esquina superior derecha */
	_moveto((w->wx+w->lx-1)*8,w->wy*w->chralt);
	imp_chr(Borde[w->borde-1][3],w->colorf,w->color,CHR_NORM);

	/* esquina inferior derecha */
	_moveto((w->wx+w->lx-1)*8,(w->wy+w->ly-1)*w->chralt);
	imp_chr(Borde[w->borde-1][5],w->colorf,w->color,CHR_NORM);

	/* lineas superior e inferior */
	for(i=w->wx+1; i<(w->wx+w->lx-1); i++) {
		_moveto(i*8,w->wy*w->chralt);
		imp_chr(Borde[w->borde-1][0],w->colorf,w->color,CHR_NORM);
		_moveto(i*8,(w->wy+w->ly-1)*w->chralt);
		imp_chr(Borde[w->borde-1][0],w->colorf,w->color,CHR_NORM);
	}

	/* lineas laterales */
	for(i=w->wy+1; i<(w->wy+w->ly-1); i++) {
		_moveto(w->wx*8,i*w->chralt);
		imp_chr(Borde[w->borde-1][1],w->colorf,w->color,CHR_NORM);
		_moveto((w->wx+w->lx-1)*8,i*w->chralt);
		imp_chr(Borde[w->borde-1][1],w->colorf,w->color,CHR_NORM);
	}
}

}

/****************************************************************************
	W_MAYUSCULA: convierte una letra en may£scula.
	  Entrada:      'c' car cter a convertir
	  Salida:       may£scula del car cter
****************************************************************************/
char w_mayuscula(char c)
{
if((c>='a') && (c<='z')) return(c-(char)'a'+(char)'A');
switch(c) {
case (char)'¤' :
	c=(char)'¥';
	break;
case (char)' ' :
	c='A';
	break;
case (char)'‚' :
	c='E';
	break;
case (char)'¡' :
	c='I';
	break;
case (char)'¢' :
	c='O';
	break;
case (char)'£' :
case (char)'' :
	c='U';
	break;
}

return(c);
}

/****************************************************************************
	W_ESTA_EN: comprueba si un car cter est  en una cadena.
	  Entrada:      's' cadena con la que se comprobar  el car cter
			'c' car cter a comprobar
	  Salida:       1 si el car cter est  en la cadena
			0 si no
****************************************************************************/
int w_esta_en(const char *s, char c)
{

while(*s) {
	if(*s++==c) return(1);
}

return(0);
}

/****************************************************************************
	W_SCROLL: realiza el scroll de una ventana dada, una l¡nea hacia
	  arriba.
	  Entrada:      'w' puntero a ventana
****************************************************************************/
void w_scroll(STC_WINDOW *w)
{
BYTE x, y, finx, finy;
short color;

x=w->wxi;
y=w->wyi;
color=w->colorf;

/* calcula coordenadas esquina inferior derecha de la ventana */
finx=(BYTE)(w->wxi+w->lxi-1);
finy=(BYTE)(w->wyi+w->lyi-1);

_asm {
	mov ah,0x06                     ; funci¢n scroll de ventana
	mov al,1                        ; 1 l¡nea
	mov bh,BYTE PTR color           ; color de relleno
	mov ch,y
	mov cl,x
	mov dh,finy
	mov dl,finx
	int 10h
}

}

/****************************************************************************
	W_CREA: crea una ventana.
	  Entrada:      'f', 'c' fila y columna
			'ancho', 'alto' dimensiones de la ventana
			'colorf' color del fondo
			'color' color del primer plano
			'borde' tipo de borde para la ventana
			'w' puntero a la ventana
****************************************************************************/
void w_crea(BYTE f, BYTE c, BYTE ancho, BYTE alto, short colorf, short color,
  BYTE borde, STC_WINDOW *w)
{
struct videoconfig vc;

/* inicializa origen */
w->wx=c;
w->wy=f;

/* inicializa dimensiones */
w->lx=ancho;
w->ly=alto;

/* inicializa color */
w->colorf=colorf;
w->color=color;
w->colortf=colorf;
w->colort=color;

/* inicializa tipo de borde */
w->borde=borde;

/* inicializa coordenadas y dimensiones del interior de la ventana */
if(borde) {
	w->wxi=(BYTE)(w->wx+1);
	w->wyi=(BYTE)(w->wy+1);
	w->lxi=(BYTE)(w->lx-2);
	w->lyi=(BYTE)(w->ly-2);
}
else {
	w->wxi=w->wx;
	w->wyi=w->wy;
	w->lxi=w->lx;
	w->lyi=w->ly;
}

/* inicializa posici¢n cursor dentro de ventana */
w->cwx=0;
w->cwy=0;
w->cwxs=0;
w->cwys=0;

/* borra indicador de scroll */
w->scroll=0;

/* coge informaci¢n sobre el sistema de v¡deo */
_getvideoconfig(&vc);

/* VGA en modo 640*480 a 16 colores, altura caracteres 16 */
if(vc.mode==_VRES16COLOR) w->chralt=16;
/* VGA en modo 320*200 a 256 colores, altura caracteres 8 */
if(vc.mode==_MRES256COLOR) w->chralt=8;

}

/****************************************************************************
	W_CLS: borra/inicializa una ventana.
	  Entrada:      'w' puntero a ventana a inicializar
****************************************************************************/
void w_cls(STC_WINDOW *w)
{

/* borra ventana */
_setcolor(w->colortf);
_rectangle(_GFILLINTERIOR,w->wxi*8,w->wyi*w->chralt,((w->wxi+w->lxi)*8)-1,
  ((w->wyi+w->lyi)*w->chralt)-1);

/* posiciona el cursor al inicio de la ventana */
w->cwx=0;
w->cwy=0;

/* resetea el color temporal */
w->colortf=w->colorf;
w->colort=w->color;

/* borra indicador de scroll */
w->scroll=0;

/* imprime el borde de la ventana */
w_imp_borde(w);

}

/****************************************************************************
	W_IMPC: imprime un car cter en una ventana. S¢lo tiene en cuenta como
	  car cter especial el c¢digo 13 (avance de l¡nea).
	  Entrada:      'c' car cter a imprimir
			'w' puntero a ventana
****************************************************************************/
void w_impc(char c, STC_WINDOW *w)
{
short esqx, esqy;
/* buffer para guardar car cter cuando hay espera de scroll */
/* el tama¤o es m s que el necesario para guardar un car cter en */
/* el modo de 640x480x16 */
char img_car[90];

/* si no es avance de l¡nea imprime sin m s */
if(c!='\n') {
	_moveto((w->wxi+w->cwx)*8,(w->wyi+w->cwy)*w->chralt);
	imp_chr(c,w->colortf,w->colort,CHR_NORM);
	/* siguiente columna */
	w->cwx++;
}
/* si es avance de l¡nea, fuerza paso a siguiente l¡nea colocando */
/* el cursor al borde de la ventana */
else w->cwx=w->lxi;

/* si llega a la £ltima columna */
if(w->cwx>(BYTE)(w->lxi-1)) {
	/* pasa a siguiente l¡nea */
	w->cwy++;
	w->cwx=0;
	if(w->cwy>(BYTE)(w->lyi-1)) {
		/* coloca en £ltima l¡nea e incrementa indicador de scroll */
		w->cwy=(BYTE)(w->lyi-1);
		w->scroll++;
		if(w->scroll>w->lyi) w->scroll=1;
		/* espera pulsaci¢n de tecla antes de scroll */
		if(w->scroll==1) {
			/* imprime car cter de scroll y espera tecla */
			esqx=(w->wxi+w->lxi-1)*8;
			if(w->borde!=NO_BORDE) esqy=(w->wyi+w->lyi)*w->chralt;
			else esqy=(w->wyi+w->lyi-1)*w->chralt;

			/* guarda car cter sobre el que imprimir  */
			_getimage(esqx,esqy,esqx+7,esqy+w->chralt-1,
			  (char _huge *)img_car);

			/* imprime car cter de espera scroll */
			_moveto(esqx,esqy);
			imp_chr(CAR_SCROLL,w->colorf,w->color,CHR_NORM);

			/* espera tecla */
			_bios_keybrd(_KEYBRD_READ);

			/* borra car cter de espera */
			_putimage(esqx,esqy,(char _huge *)img_car,_GPSET);
		}
		/* si el indicador de scroll est  activo, hace scroll */
		if(w->scroll) w_scroll(w);
	}
}

}

/****************************************************************************
	W_IMPS: imprime una cadena en una ventana. Las palabras de final de
	  l¡nea que no quepan dentro de la ventana se pasan a la l¡nea
	  siguiente.
	  Entrada:      's' cadena a imprimir
			'w' puntero a ventana
****************************************************************************/
void w_imps(char *s, STC_WINDOW *w)
{
char b[MAX_PAL];                /* buffer para guardar palabras */
BYTE cuenta=0, i;

while(1) {
	/* si se encontr¢ fin de frase, espacio o avance de l¡nea */
	if((*s=='\0') || (*s==' ') || (*s=='\n')) {
		/* si cuenta==0, no hay palabra almacenada */
		if(!cuenta && (*s!='\0')) {
			w_impc(*s,w);
			s++;
		}
		else {
			/* si la palabra almacenada no cabe en lo que */
			/* queda de l¡nea pero cabe en la siguiente, */
			/* la imprime en la siguiente l¡nea, si no la */
			/* imprime sin m s */
			if((cuenta>(BYTE)(w->lxi-w->cwx)) &&
			  (cuenta<=w->lxi)) w_impc('\n',w);
			for(i=0; i<cuenta; i++) w_impc(b[i],w);
			cuenta=0;
			/* si fin de frase, sale */
			if(*s=='\0') break;
		}
	}
	else {
		/* si letra no es espacio ni avance de l¡nea la guarda */
		b[cuenta]=*s++;
		cuenta++;
		/* si se llena buffer */
		if(cuenta==MAX_PAL) {
			/* imprime lo que hay almacenado */
			for(i=0; i<MAX_PAL; i++) w_impc(b[i],w);
			cuenta=0;
			/* imprime resto de palabra */
			while((*s!='\0') && (*s!=' ') && (*s!='\n')) {
				w_impc(*s,w);
				s++;
			}
		}
	}
}

}

/****************************************************************************
	W_IMPN: imprime un n£mero en una ventana.
	  Entrada:      'n' n£mero (16 bits) a imprimir
			'w' puntero a ventana
****************************************************************************/
void w_impn(unsigned n, STC_WINDOW *w)
{
int i=0;
char d[5];

do {
	/* coge el resto de n/10 y lo pasa a ASCII */
	d[i]=(char)((n%10)+'0');
	n/=10;
	i++;
} while(n);

/* imprime los d¡gitos en orden inverso a como fueron hayados */
while(i--) w_impc(d[i],w);

}

/****************************************************************************
	W_INPUT: permite introducir por teclado una l¡nea de caracteres.
	  S¢lo admite los caracteres que contenga la cadena Cod_Teclas
	  (global).
	  Entrada:      'plin' puntero a buffer d¢nde se almacenar  la
			cadena introducida
			'maxlong' m ximo n£mero de caracteres permitidos;
			deber  ser menor o igual a WINPUT_MAXLIN
			'cursor' car cter a usar como cursor
			'conver' indica si la cadena tecleada debe ser
			convertida a may£sculas (WINPUT_CONV para convertir,
			WINPUT_NOCONV para dejar como se tecle¢)
			'w' puntero a ventana
	  Salida:       c¢digo de tecla de funci¢n pulsada o 0 si no se
			puls¢ ninguna
****************************************************************************/
unsigned w_input(char *plin, unsigned maxlong, char cursor, int conver,
  STC_WINDOW *w)
{
unsigned c, long_lin;
char *pcur, *pfinl, *i;
BYTE oldx, oldy;
short curx, cury;
static char buf_lin[WINPUT_MAXLIN+1]="";

/* guarda posici¢n del cursor y habilita scroll de ventana */
oldx=w->cwx;
oldy=w->cwy;
w->scroll=1;

/* puntero de la posici¢n del cursor */
pcur=plin;

/* puntero al final de la l¡nea */
pfinl=plin;
long_lin=1;

/* marca final de l¡nea */
*pfinl='\0';

/* imprime cursor */
curx=(w->wxi+w->cwx)*8;
cury=(w->wyi+w->cwy)*w->chralt;
_moveto(curx,cury);
imp_chr(cursor,0xff,0,CHR_AND);
_moveto(curx,cury);
imp_chr(cursor,0,w->colort,CHR_XOR);

do {
	c=w_lee_tecla();
	switch(c) {
		case BACKSPACE :        /* borrar car cter a izquierda */
			/* comprueba que cursor no est‚ a la izquierda */
			if(pcur!=plin) {
				pcur--;
				for(i=pcur; i<pfinl; i++) *i=*(i+1);
				pfinl--;
				long_lin--;
			}
			break;
		case COD_SUP :          /* borrar car cter a derecha */
			/* comprueba que cursor no est‚ al final */
			if(pcur!=pfinl) {
				for(i=pcur; i<pfinl; i++) *i=*(i+1);
				pfinl--;
				long_lin--;
			}
			break;
		case COD_IZQ :          /* mover cursor izquierda */
			/* comprueba que no est‚ a la izquierda */
			if(pcur!=plin) pcur--;
			break;
		case COD_DER :          /* mover cursor derecha */
			/* comprueba que no est‚ a la derecha */
			if(pcur!=pfinl) pcur++;
			break;
		case COD_ARR :          /* mover cursor arriba */
			/* si se pasa del inicio, copia l¡nea anterior */
			/* y sit£a cursor al final */
			if(pcur==plin) {
				/* borra l¡nea anterior */
				if(w->scroll!=1)
				  w->cwy=(BYTE)(oldy-(w->scroll-1));
				else w->cwy=oldy;
				w->cwx=oldx;
				for(i=plin; *i; i++) w_impc(' ',w);

				/* por si cursor est  al final */
				w_impc(' ',w);

				pcur=plin;
				for(i=buf_lin, long_lin=1; *i; i++, pcur++,
				  long_lin++) *pcur=*i;
				pfinl=pcur;
				*pfinl='\0';
			}
			else {
				/* resta el ancho de la ventana */
				pcur-=w->lxi;
				if(pcur<plin) pcur=plin;
			}
			break;
		case COD_ABJ :          /* mover cursor abajo */
			/* suma el ancho de la ventana */
			pcur+=w->lxi;
			/* si se pasa del final, lo coloca al final */
			if(pcur>pfinl) pcur=pfinl;
			break;
		case COD_ORG :          /* mover cursor al inicio */
			pcur=plin;
			break;
		case COD_FIN :          /* mover cursor al final */
			pcur=pfinl;
			break;
		/* si pulsa una tecla de funci¢n o RETURN */
		/* acaba la entrada y devuelve su c¢digo (tecla funci¢n) */
		case RETURN :
		case COD_F1 :
		case COD_F2 :
		case COD_F3 :
		case COD_F4 :
		case COD_F5 :
		case COD_F6 :
		case COD_F7 :
		case COD_F8 :
		case COD_F9 :
		case COD_F10 :
			break;
		default :
			/* inserta car cter si no se excede longitud m xima */
			if(long_lin==maxlong) break;
			if(pcur!=pfinl) for(i=pfinl; i>pcur; i--) *i=*(i-1);
			*pcur++=(char)c;
			*(++pfinl)='\0';
			long_lin++;
			break;
	}
	/* si al teclear algo hubo scroll en ventana calcula l¡nea */
	/* de inicio del cursor de acuerdo al n£mero de l¡neas */
	/* 'scrolleadas' (w->scroll), sino coge la antigua posici¢n */
	/* del cursor 'oldy' */
	if(w->scroll!=1) w->cwy=(BYTE)(oldy-(w->scroll-1));
	else w->cwy=oldy;
	w->cwx=oldx;

	i=plin;
	do {
		/* guarda posici¢n del cursor */
		curx=w->cwx;
		cury=w->cwy;

		/* imprime car cter */
		if(*i) w_impc(*i,w);

		/* si es posici¢n de cursor y no est  al final de */
		/* la l¡nea, lo imprime */
		if((i==pcur) && (pcur!=pfinl)) {
			/* convierte posici¢n de cursor a pixels */
			curx=(curx+w->wxi)*8;
			cury=(cury+w->wyi)*w->chralt;
			/* enmascara zona */
			_moveto(curx,cury);
			imp_chr(*i,0xff,0,CHR_AND);
			_moveto(curx,cury);
			imp_chr(cursor,0xff,0,CHR_AND);
			/* imprime car cter */
			_moveto(curx,cury);
			imp_chr(*i,0,w->colort,CHR_OR);
			/* imprime cursor sobre car cter */
			_moveto(curx,cury);
			imp_chr(cursor,0,w->colort,CHR_XOR);
		}

		/* siguiente car cter */
		i++;
	} while(i<pfinl);

	/* si cursor al final lo imprime ahora */
	curx=(w->wxi+w->cwx)*8;
	cury=(w->wyi+w->cwy)*w->chralt;
	if(pcur==pfinl) {
		w_impc(' ',w);
		_moveto(curx,cury);
		imp_chr(cursor,0xff,0,CHR_AND);
		_moveto(curx,cury);
		imp_chr(cursor,0,w->colort,CHR_XOR);
	}

	/* imprime un espacio al final (por si se borr¢ un car cter) */
	w_impc(' ',w);

} while((c!=RETURN) && (c<COD_F10));

/* reimprime la l¡nea de input para borrar el cursor */
/* posiciona el cursor para imprimir */
if(w->scroll!=1) w->cwy=(BYTE)(oldy-(w->scroll-1));
else w->cwy=oldy;
w->cwx=oldx;

i=plin;
do {
	if(*i) w_impc(*i,w);
	i++;
} while(i<=pfinl);
/* a¤ade espacio al final para compensar la desaparici¢n del cursor */
w_impc(' ',w);

/* copia l¡nea en buffer */
for(pcur=plin, i=buf_lin; *pcur; pcur++, i++) *i=*pcur;
*i='\0';

if(conver==WINPUT_CONV) {
	/* convierte l¡nea a may£sculas */
	i=plin;
	while(*i) {
		*i=w_mayuscula(*i);
		i++;
	}
}

/* si no se produjo scroll en ventana mientras entrada de datos */
/* y la posici¢n del cursor es menor que la £ltima l¡nea de la ventana */
/* desactiva indicador de scroll */
if((w->scroll==1) && (w->cwy<(BYTE)(w->lyi-1))) w->scroll=0;

/* si termin¢ la l¡nea pulsando RETURN devuelve 0, si no devuelve */
/* el c¢digo de la tecla conque se termin¢ */
if(c==RETURN) return(0);
else return(c);

}

/****************************************************************************
	W_LEE_TECLA: devuelve el c¢digo ASCII de la tecla pulsada.
	  Espera hasta que se pulse una tecla, o hasta que la tecla pulsada
	  sea una de las contenidas en la cadena Cod_Teclas (global).
	  Salida:       C¢digo ASCII de la tecla pulsada.
			Si se puls¢ una tecla de cursor devuelve COD_IZQ,
			COD_DER, COD_ARR, COD_ABJ, COD_ORG o COD_FIN.
			Si se puls¢ una tecla de funci¢n devuelve COD_Fx.
****************************************************************************/
unsigned w_lee_tecla(void)
{
unsigned tecla;

do {
	/* byte bajo=ASCII, byte alto=CODIGO*/
	tecla=_bios_keybrd(_KEYBRD_READ);

	/* comprueba teclas especiales y pone ASCII=c¢digo tecla */
	if(tecla==IZQ) tecla=COD_IZQ;
	else if(tecla==DER) tecla=COD_DER;
	else if(tecla==ARR) tecla=COD_ARR;
	else if(tecla==ABJ) tecla=COD_ABJ;
	else if(tecla==ORG) tecla=COD_ORG;
	else if(tecla==FIN) tecla=COD_FIN;
	else if(tecla==SUP) tecla=COD_SUP;
	else if(tecla==F1) tecla=COD_F1;
	else if(tecla==F2) tecla=COD_F2;
	else if(tecla==F3) tecla=COD_F3;
	else if(tecla==F4) tecla=COD_F4;
	else if(tecla==F5) tecla=COD_F5;
	else if(tecla==F6) tecla=COD_F6;
	else if(tecla==F7) tecla=COD_F7;
	else if(tecla==F8) tecla=COD_F8;
	else if(tecla==F9) tecla=COD_F9;
	else if(tecla==F10) tecla=COD_F10;

	/* desprecia byte alto */
	tecla &= 0x00ff;
} while(!w_esta_en(Cod_Teclas,(char)tecla));

return(tecla);
}

