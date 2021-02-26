
		NumChannels = 32

		MaxSamples = 100

	STRUC	CHANNEL

		Active		DB	1
		Volume		DB	0 
		Panning		DB	0 
		SampleSeg	DW	0		; No sample
		Mierdecilla	DW	0
		SampleOffset	DW	0
		SampleFOffset	DW	0		; Frac.
		SampleSize	DW	0
		SampleInc	DW	0
		SampleFInc	DW	0
		SampleLoop	DB	0
		SampleLoopStart	DW	0
		SampleLoopEnd	DW	0
		Period		DW	0
		Note		DB	0
		OldVolume	DB	0
		EffectCount	DB	0
		C2SPD		DW	0
		Effect		DB	0
		EffectInfo	DB	0
		Instrument	DB	0FFh

		; (Necesito que el canal tenga 32 bytes de tama¤o)
	
	ENDS	CHANNEL

	STRUC	SAMPLEHEADER
	
		Type		DB	0FFh
		Filename	DB	13 DUP (0)
		MemSeg		DW	0
		Size		DD	0
		LoopBeg		DD	0
		LoopEnd		DD	0
		Volume		DB	0, 0
		Pack		DB	0
		Flags		DB	0
		C2SPD		DD	0
		Quesesto	DB	12 DUP (0)
		SampleName	DB	28 DUP (0)
		Firma		DB	'S'
		Firma2		DB	'C'
		Firma3		DB	'R'
		Firma4		DB	'S'

		; Flags: bit 0 - ¨Loop activo?
		; Pack:  debe ser 0 - unpacked
		; Type:  debe ser 1 - sample
	
	ENDS	SAMPLEHEADER

	STRUC	entry
	
		Note		DB	?
		Instrument	DB	?
		Volume		DB	?
		Effect		DB	?
		EffectInfo	DB	?
	
	ENDS	entry

	KeyOff = 0FEh


		; Efectos

		Ef_SetSpeed	= 1
		Ef_OrderJump	= 2
		Ef_Break	= 3
		Ef_VolSlide	= 4
		Ef_SlideDown	= 5
		Ef_SlideUp	= 6
		Ef_TonePort	= 7
		Ef_Vibrato	= 8		; No implementado
		Ef_Tremor	= 9
		Ef_Arpeggio	= 10
		Ef_Vib_VolSl	= 11		; No implementado
		Ef_Tone_VolSl	= 12		; No implementado
		Ef_Offset	= 15
		Ef_Retrig	= 17
		Ef_Tremolo	= 18
		Ef_Multiple	= 19
		Ef_Tempo	= 20		; No implementado
		Ef_FVibrato	= 21		; No implementado
		Ef_SetGlobalVol = 22
		Ef_Unused	= 24		; No implementado

		; Efectos Sx (de Ef_Multiple)

		MEf_SetFilter	= 0		; No implementado
		MEf_Glissando	= 1		; No implementado
		MEf_FineTune	= 2		; No implementado
		MEf_VibWave	= 3		; No implementado
		MEf_TreWave	= 4		; No implementado
		MEf_Panning	= 8
		MEf_Stereo	= 10		; No implementado
		MEf_Loop	= 11		; No implementado
		MEf_NoteCut	= 12		; No implementado
		MEf_NoteDelay	= 13		; No implementado
		MEf_PatterDelay	= 14		; No implementado
		MEf_FunkRepeat	= 15		; No implementado
		
MACRO	NumInstrument2InstrumentOffset

	; BX -> N§ de instrumento
	; SALIDA: BX -> Offset del instrumento
	
		mov dx, bx
		shl bx, 6
		shl dx, 4
		add bx, dx
		add bx, offset _Samples

ENDM	NumInstrument2InstrumentOffset


MACRO	NumChannel2ChannelOffset

	; CX -> N§ canal
	; SALIDA: SI -> Canal
	
		shl cx, 5
		mov si, offset _Channels
		add si, cx
		
ENDM	NumChannel2ChannelOffset

