/***************************************
	N£meros de versiones para
	cada parte del sistema SINTAC
	y mensajes de copyright
***************************************/

#if !defined (VERSION_H)
#define VERSION_H

#define SHARE       0           /* 0=versi¢n completa, 1=versi¢n "shareware" */

#define COPYRIGHT   "S.I.N.T.A.C. (c)1994 JSJ Soft Ltd."
#if SHARE==0
#define VERSION_CS  "G2.02"     /* versi¢n compilador */
#define VERSION_IS  "G2.02"     /* versi¢n int‚rprete-debugger */
#define VERSION_LS  "G2.02"     /* versi¢n listador */
#define VERSION_LKS "G2.02"     /* versi¢n linkador */
#define VERSION_EDS "G2.02"     /* versi¢n entorno programaci¢n */
#else
#define VERSION_CS  "G2.sh"     /* versi¢n "shareware" compilador */
#define VERSION_IS  "G2.sh"   	/* versi¢n "shareware" int‚rprete-debugger */
#define VERSION_LS  "G2.sh"   	/* versi¢n "shareware" listador */
#define VERSION_LKS "G2.sh"   	/* versi¢n "shareware" linkador */
#define VERSION_EDS "G2.sh"   	/* versi¢n "shareware" entorno programaci¢n */
#endif

#endif  /* VERSION_H */
