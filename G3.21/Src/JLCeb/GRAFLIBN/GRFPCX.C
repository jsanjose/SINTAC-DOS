/****************************************************************************
				   GRFPCX.C

	Conjunto de funciones para decodificar y visualizar ficheros
	gr ficos en formato PCX.

	Las siguientes funciones son p£blicas:
		- pcx_decodifica: decodifica una imagen PCX
		- pcx_libera_mem: libera memoria ocupada por imagen
		- pcx_dibuja: dibuja una imagen PCX
		- pcx_visualiza: decodifica una imagen PCX y la va dibujando

	Las siguiente estructura est  definida en GRF.H:
		STC_IMAGEN: imagen decodificada
****************************************************************************/

/* cabecera de fichero PCX */
typedef struct {
	BYTE propietario;       /* deber¡a ser siempre 0x0A */
	BYTE version;           /* version del fichero PCX */
	BYTE metodo_codif;      /* 1=codificaci¢n PCX 'run-lenght' */
	BYTE bits_pixel;        /* n£mero de bits por pixel */
	int ventana[4];         /* tama¤o dibujo (izq., arr., der., abajo) */
	int resh, resv;         /* resoluci¢n horizontal y vertical */
	BYTE paleta[16][3];     /* informaci¢n de paleta para la imagen */
	BYTE reserv1;
	BYTE num_planos;        /* n£mero de planos de color */
	int bytes_scan;         /* n£mero de bytes por plano de l¡nea scan */
	BYTE reserv2[56];
	long size ;		/* tama¤o de la zona a descodificar */
} STC_CABPCX;


static int abrir (FILE **file, char *fname, STC_IMAGEN *pcx, STC_CABPCX *header)
{
   int errval = 0, bits_pixel ;
   char test ;
   long fsize ;

   file = fopen (fname, "rb") ;
   if (!*file) return E_PCX_APER ;
   if (fread (header, sizeof(STC_CABPCX), 1, *file) != 1)
      errval = E_PCX_LECT ;
   else
   {
      fseek (*file, 0, SEEK_END) ;
      fsize = ftell(*file) ;

      if (header->propietario != 0x0A || header->metodo_codif != 1)
	 errval = E_PCX_FORM ;
      bits_pixel = header->bits_pixel * header->num_planos ;
      if (g_memmodel == G_MM_16C)
	 errval = bits_pixel == 4 ? 0 : E_PCX_MODO ;
      else
	 errval = bits_pixel == 8 ? 0 : E_PCX_MODO ;
      if (!errval)
      {
	 pcx->ancho = header.ventana[2]-header.ventana[0] ;
	 pcx->alto  = header.ventana[3]-header.ventana[1] ;
         pcx->bits_pixel = bits_pixel ;

	 if (bits_pixel == 4)
	    memcpy (pcx->paleta, header->paleta, 16*3) ;
	 else
	 {
	    fseek (*file, fsize-769, SEEK_SET) ;
	    test = 0 ;
	    fread (&test, 1, 1, *file) ;
	    if (test == 0xC)
	    {
	       fread (pcx->paleta, 768, 1, *file) ;
	       fsize -= 769 ;
	    }
	 }
	 fseek (*file, 128, SEEK_SET) ;
	 header->size = fsize-128 ;
      }
   }
   if (errval) fclose (*file) ;
   return errval ;
}

static int decodifica (FILE *file, char *blq, int ancho, int alto, char *buffer, int buffer_size)
{
   char *bptr = buffer ;
   char *bmax = buffer + buffer_size ;
   char *ptr = blq+4 ;

   if (g_memmodel == G_MM_16C)
      ;
   else
   {
   }
}

char emergency_buffer[1024] ;

int pcx_decodifica (char *fname, STC_IMAGEN *pcx)
{
   FILE *file ;
   STC_CABPCX header ;
   int errval, i, j ;
   long blq_size, freem ;
   int blq_lines ;
   char *ptr ;

   errval = abrir (&file, fname, pcx, &header) ;
   if (errval) return errval ;

   blq_size = pcx->ancho*pcx->alto ;
   blq_lines = pcx->alto ;
   while (blq_size > 65530L)
   {
      blq_lines = 65530L/pcx->ancho ;
      blq_size = pcx->ancho*blq_lines ;
   }
   pcx->num_bloques = pcx->alto/blq_lines ;
   if (blq_lines*num_bloques < pcx->alto) num_bloques++ ;

   if (pcx->num_bloques < 6)
   {
      int j = pcx->alto ;
      for (i = 0 ; i < pcx->num_bloques ; i++, j -= blq_lines)
	 if (i < pcx->num_bloques)
	    ptr = pcx->bloques[i] = (char *)malloc(blq_tam(0,0,pcx->ancho,blq_lines)) ;
	 else
	    ptr = pcx->bloques[i] = (char *)malloc(blq_tam(0,0,pcx->ancho,j)) ;
      if (ptr)
      {
	 freem = coreleft() ;
	 if (freem > 1024)
	    ptr = malloc (freem > 65530L ? 65530 : freem) ;
	 else
	 {
	    freem = 1024 ;
	    ptr = emergency_buffer ;
	 }

	 j = pcx->alto ;
	 for (i = 0 ; i < pcx->num_bloques ; i++, j -= blq_lines)
	    decodifica (file, pcx->bloques[i], pcx->ancho,
			j > blq_lines ? blq_lines : j, ptr, freem) ;

	 if (ptr != emergency_buffer)
	    free(ptr) ;
      }
      else
      {
	 errval = E_PCX_MEM ;
	 for (i = 0 ; i < pcx->num_bloques ; i++)
	    free (pcx->bloques[i]) ;
      }
   }
   else
      errval = E_PCX_MEM ;

   fclose (file) ;
   return errval ;
}