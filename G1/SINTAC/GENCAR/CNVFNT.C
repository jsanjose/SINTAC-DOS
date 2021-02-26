/****************************************************************************
		       CONVERSOR DE JUEGOS DE CARACTERES
			    (c)1993 JSJ Soft Ltd.
****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <string.h>
#include "..\sintac.h"

/*** Constantes ***/
#define TAB_ROM8x8    3         /* tabla de 8x8 de la ROM */

/*** Prototipos ***/
int graba_def(char *nombre);
int carga_def(char *nombre);
void copia_tabla_rom(BYTE tabla, BYTE _far *tabla_car, int alt);

/*** Variables globales ***/
/* tablas donde se almacenar n las definiciones de los caracteres */
BYTE tabla_ascii8x14[256][14];
BYTE tabla_ascii8x16[256][16];
BYTE tabla_ascii8x8[256][8];

/*** Programa principal ***/
void main(int argc, char *argv[])
{
int i, j;
char f_conv[_MAX_PATH], f_drive[_MAX_DRIVE], f_dir[_MAX_DIR],
  f_nomb[_MAX_FNAME], f_ext[_MAX_EXT];

printf("\nCONVERSOR DE JUEGOS DE CARACTERES S.I.N.T.A.C."
       "\n(c)1993 JSJ Soft Ltd.\n");
if(argc<2) {
	printf("\nUso: CNVFNT fuente.fnt\n"
	       "\nConvierte el fichero 'fuente.fnt' al nuevo formato y lo"
	       "\nalmacena con el mismo nombre pero con la extensi¢n .FN2.\n");
	exit(0);
}

if(!carga_def(argv[1])) {
	printf("\nError: fichero %s no v lido o formato incorrecto.\n",
	  argv[1]);
	exit(1);
}

/* mete fuente de 8x8 */
copia_tabla_rom(TAB_ROM8x8,&tabla_ascii8x8[0][0],8);

/* pasa fuente de 8x14 a 8x16 */
for(i=0; i<256; i++) {
	for(j=0; j<16; j++) {
		if(j<14) tabla_ascii8x16[i][j]=tabla_ascii8x14[i][j];
		else tabla_ascii8x16[i][j]=0;
	}
}

/* crea nombre para fichero convertido */
_splitpath(argv[1],f_drive,f_dir,f_nomb,f_ext);
strcpy(f_conv,f_nomb);
strcat(f_conv,".FN2");

/* graba fichero convertido */
if(!graba_def(f_conv)) {
	printf("\nError: no se puede grabar fichero convertido %s.\n",f_conv);
	exit(1);
}

printf("\nConversi¢n finalizada. Fichero %s tiene el juego de caracteres"
       "\nconvertido.\n",f_conv);

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
if(cad_recon[LONG_RECON_F-1]!='1') {
	fclose(ffuente);
	return(0);
}

/* si la versi¢n ha sido v lida lee las definiciones de los caracteres */
/* definiciones 8x14 */
num_bytes=(size_t)(256*14);
if(fread(tabla_ascii8x14,sizeof(BYTE),num_bytes,ffuente)<num_bytes) {
	fclose(ffuente);
	return(0);
}

fclose(ffuente);

return(1);
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
