/**************************************
	N£meros de versiones
	y mensajes de copyright
**************************************/

#if !defined (VERSION_H)
#define VERSION_H

#define SHARE		0	/* 0=versi¢n completa, 1=versi¢n "shareware" */

#define COPYRIGHT	"S.I.N.T.A.C. (c)1996 JSJ Soft Ltd."
#define COPYRIGHT2	"  Apoyo t‚cnico y rutinas de sonido\n" \
			"         (c)Jos‚ Luis Cebri n"

#if SHARE==0
#define VERSION 	"G3.21"
#else
#define VERSION  	"G3.SH"
#endif

#endif  /* VERSION_H */
