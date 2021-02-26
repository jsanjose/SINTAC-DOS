/* Gesti¢n de memoria */

int far AllocMem (unsigned size) ;
void far FreeMem (int handle) ;
char far * far AccessMem (int handle) ;

extern far unsigned int Hz ;
extern far unsigned char Stereo ;

struct Channel
   {
   unsigned char 	activo ;
   unsigned char 	volume ;
   unsigned char 	panning ;
   char far *		sample ;
   unsigned 		offset ;
   unsigned 		frac_offset ;
   unsigned 		size ;
   unsigned 		inc ;
   unsigned 		frac_inc ;
   unsigned char 	loop ;
   unsigned 		loopstart ;
   unsigned 		loopend ;
   unsigned 		period ;
   unsigned char	note ;
   unsigned char	mierda[7] ;
   } ;

struct Sample
   {
   unsigned char	type ;
   unsigned char	filename[13] ;
   unsigned		memseg ;
   long			size ;
   long			loopstart ;
   long			loopend ;
   unsigned char	volume ;
   unsigned char	volume2 ;
   unsigned char	pack ;
   unsigned char	flags ;
   unsigned long	c2spd ;
   unsigned char	mierda[12] ;
   unsigned char	name[28] ;
   unsigned char	firma[4] ;
   } ;

int far SBStart(void) ;
void far SBEnd(void) ;
void far StartSong(void) ;
void far StopSong(void) ;
int far LoadS3M (char far *) ;

int far CreateInstrument (int handle, int size, int c2spd, int loopbeg, int loopend) ;
void far PlayInstrument (int inst, int canal, int volumen, int nota, int octava, int panning) ;

extern void far ProcessSBString (char far *str) ;

extern far int SB2x0 ;
extern far unsigned char far SBDMAChannel ;
extern far unsigned char far SBIrq ;
extern far char far *DMABufferPtr ;
extern far unsigned DMASize ;
extern far struct Channel Channels[32] ;
extern far struct Sample Samples[100] ;
extern far char SongName[] ;
extern far int OrderNum, InsNum, PatNum ;

/* Constantes £tiles */

#define ESI_ERRABRIR -1
#define ESI_ERRFORMATO -2
#define ESI_ERRVARIANTE -3
#define ESI_ERRFICHERO -4
#define ESI_ERRMEM -5

#define NOTE_C  0
#define NOTE_Db 1
#define NOTE_D  2
#define NOTE_Eb 3
#define NOTE_E  4
#define NOTE_F  5
#define NOTE_Gb 6
#define NOTE_G  7
#define NOTE_Ab 8
#define NOTE_A  9
#define NOTE_Bb 10
#define NOTE_B  11

#define PANNING_RIGHT 0
#define PANNING_MID   7
#define PANNING_LEFT  15
