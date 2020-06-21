;Program compiled by Great Cow BASIC (0.95 2016-01-24)
;Need help? See the GCBASIC forums at http://sourceforge.net/projects/gcbasic/forums,
;check the documentation or email w_cholmondeley at users dot sourceforge dot net.

;********************************************************************************

;Set up the assembler options (Chip type, clock source, other bits and pieces)
 LIST p=16F876A, r=DEC
#include <P16F876A.inc>
 __CONFIG _HS_OSC & _WDT_OFF & _LVP_OFF

;********************************************************************************

;Set aside memory locations for variables
DELAYTEMP	EQU	112
DELAYTEMP2	EQU	113
SYSDIVMULTA	EQU	119
SYSDIVMULTA_H	EQU	120
SYSDIVMULTB	EQU	123
SYSDIVMULTB_H	EQU	124
SYSDIVMULTX	EQU	114
SYSDIVMULTX_H	EQU	115
SYSINTEGERTEMPA	EQU	117
SYSINTEGERTEMPA_H	EQU	118
SYSINTEGERTEMPB	EQU	121
SYSINTEGERTEMPB_H	EQU	122
SYSINTEGERTEMPX	EQU	112
SYSINTEGERTEMPX_H	EQU	113
SYSSIGNBYTE	EQU	125
SYSWORDTEMPA	EQU	117
SYSWORDTEMPA_H	EQU	118
SYSWORDTEMPB	EQU	121
SYSWORDTEMPB_H	EQU	122
SYSWORDTEMPX	EQU	112
SYSWORDTEMPX_H	EQU	113
SysByteTempX	EQU	112
SysDivLoop	EQU	116
SysStringA	EQU	119
SysWaitTemp10US	EQU	117
SysWaitTempMS	EQU	114
SysWaitTempMS_H	EQU	115
ADN_PORT	EQU	32
ADREADPORT	EQU	33
DISPCHAR	EQU	34
DISPPORT	EQU	35
DISPTEMP	EQU	36
READAD	EQU	37
READAD_H	EQU	38
SysTemp1	EQU	39
SysTemp1_H	EQU	40
VALUE	EQU	41

;********************************************************************************

;Vectors
	ORG	0
	goto	BASPROGRAMSTART
	ORG	4
	retfie

;********************************************************************************

;Start of program memory page 0
	ORG	5
BASPROGRAMSTART
;Call initialisation routines
	call	INITSYS
	call	INITSEVENSEG

;Start of the main program
	banksel	TRISB
	clrf	TRISB
	bsf	TRISA,0
	bcf	TRISC,7
MAIN
	banksel	ADREADPORT
	clrf	ADREADPORT
	movlw	255
	movwf	ADN_PORT
	call	FN_READAD
	movf	READAD,W
	movwf	SysINTEGERTempA
	movf	READAD_H,W
	movwf	SysINTEGERTempA_H
	movlw	26
	movwf	SysINTEGERTempB
	clrf	SysINTEGERTempB_H
	call	SysDivSubINT
	movf	SysINTEGERTempA,W
	movwf	VALUE
	movlw	1
	movwf	DISPPORT
	movf	VALUE,W
	movwf	DISPCHAR
	call	DISPLAYVALUE
	movlw	10
	movwf	SysWaitTempMS
	clrf	SysWaitTempMS_H
	call	Delay_MS
	goto	MAIN
BASPROGRAMEND
	sleep
	goto	BASPROGRAMEND

;********************************************************************************

DISPLAYVALUE
	incf	DISPCHAR,W
	movwf	SYSSTRINGA
	call	SEVENSEGDISPDIGIT
	movwf	DISPTEMP
	bcf	PORTB,7
	bcf	PORTB,6
	bcf	PORTB,5
	bcf	PORTB,4
	bcf	PORTB,3
	bcf	PORTB,2
	bcf	PORTB,1
	bcf	PORTB,0
	bcf	PORTC,7
	decf	DISPPORT,W
	btfsc	STATUS, Z
	bsf	PORTC,7
	btfsc	DISPTEMP,0
	bsf	PORTB,7
	btfsc	DISPTEMP,1
	bsf	PORTB,6
	btfsc	DISPTEMP,2
	bsf	PORTB,5
	btfsc	DISPTEMP,3
	bsf	PORTB,4
	btfsc	DISPTEMP,4
	bsf	PORTB,3
	btfsc	DISPTEMP,5
	bsf	PORTB,2
	btfsc	DISPTEMP,6
	bsf	PORTB,1
	btfsc	DISPTEMP,7
	bsf	PORTB,0
	return

;********************************************************************************

Delay_10US
D10US_START
	movlw	12
	movwf	DELAYTEMP
DelayUS0
	decfsz	DELAYTEMP,F
	goto	DelayUS0
	decfsz	SysWaitTemp10US, F
	goto	D10US_START
	return

;********************************************************************************

Delay_MS
	incf	SysWaitTempMS_H, F
DMS_START
	movlw	108
	movwf	DELAYTEMP2
DMS_OUTER
	movlw	11
	movwf	DELAYTEMP
DMS_INNER
	decfsz	DELAYTEMP, F
	goto	DMS_INNER
	decfsz	DELAYTEMP2, F
	goto	DMS_OUTER
	decfsz	SysWaitTempMS, F
	goto	DMS_START
	decfsz	SysWaitTempMS_H, F
	goto	DMS_START
	return

;********************************************************************************

INITSEVENSEG
	banksel	TRISB
	bcf	TRISB,7
	bcf	TRISB,6
	bcf	TRISB,5
	bcf	TRISB,4
	bcf	TRISB,3
	bcf	TRISB,2
	bcf	TRISB,1
	bcf	TRISB,0
	bcf	TRISC,7
	banksel	STATUS
	return

;********************************************************************************

INITSYS
	banksel	ADCON1
	bcf	ADCON1,ADFM
	banksel	ADCON0
	bcf	ADCON0,ADON
	banksel	ADCON1
	bcf	ADCON1,PCFG3
	bsf	ADCON1,PCFG2
	bsf	ADCON1,PCFG1
	bcf	ADCON1,PCFG0
	movlw	7
	movwf	CMCON
	banksel	PORTA
	clrf	PORTA
	clrf	PORTB
	clrf	PORTC
	return

;********************************************************************************

FN_READAD
	banksel	ADCON1
	bcf	ADCON1,PCFG3
	bcf	ADCON1,PCFG2
	bcf	ADCON1,PCFG1
	bcf	ADCON1,PCFG0
	banksel	ADCON0
	bcf	ADCON0,ADCS1
	bsf	ADCON0,ADCS0
	bcf	ADCON0,CHS0
	bcf	ADCON0,CHS1
	bcf	ADCON0,CHS2
	btfsc	ADREADPORT,0
	bsf	ADCON0,CHS0
	btfsc	ADREADPORT,1
	bsf	ADCON0,CHS1
	btfsc	ADREADPORT,2
	bsf	ADCON0,CHS2
	bsf	ADCON0,ADON
	movlw	2
	movwf	SysWaitTemp10US
	call	Delay_10US
	bsf	ADCON0,GO_DONE
SysWaitLoop1
	btfsc	ADCON0,GO_DONE
	goto	SysWaitLoop1
	bcf	ADCON0,ADON
	banksel	ADCON1
	bcf	ADCON1,PCFG3
	bsf	ADCON1,PCFG2
	bsf	ADCON1,PCFG1
	bcf	ADCON1,PCFG0
	banksel	ADRESH
	movf	ADRESH,W
	movwf	READAD
	clrf	READAD_H
	return

;********************************************************************************

SEVENSEGDISPDIGIT
	movlw	17
	subwf	SysStringA, W
	btfsc	STATUS, C
	retlw	0
	movf	SysStringA, W
	addlw	low TableSEVENSEGDISPDIGIT
	movwf	SysStringA
	movlw	high TableSEVENSEGDISPDIGIT
	btfsc	STATUS, C
	addlw	1
	movwf	PCLATH
	movf	SysStringA, W
	movwf	PCL
TableSEVENSEGDISPDIGIT
	retlw	16
	retlw	63
	retlw	6
	retlw	91
	retlw	79
	retlw	102
	retlw	109
	retlw	125
	retlw	7
	retlw	127
	retlw	111
	retlw	119
	retlw	124
	retlw	57
	retlw	94
	retlw	121
	retlw	113

;********************************************************************************

SYSCOMPEQUAL16
	clrf	SYSBYTETEMPX
	movf	SYSWORDTEMPA, W
	subwf	SYSWORDTEMPB, W
	btfss	STATUS, Z
	return
	movf	SYSWORDTEMPA_H, W
	subwf	SYSWORDTEMPB_H, W
	btfss	STATUS, Z
	return
	comf	SYSBYTETEMPX,F
	return

;********************************************************************************

SYSDIVSUB16
	movf	SYSWORDTEMPA,W
	movwf	SYSDIVMULTA
	movf	SYSWORDTEMPA_H,W
	movwf	SYSDIVMULTA_H
	movf	SYSWORDTEMPB,W
	movwf	SYSDIVMULTB
	movf	SYSWORDTEMPB_H,W
	movwf	SYSDIVMULTB_H
	clrf	SYSDIVMULTX
	clrf	SYSDIVMULTX_H
	movf	SYSDIVMULTB,W
	movwf	SysWORDTempA
	movf	SYSDIVMULTB_H,W
	movwf	SysWORDTempA_H
	clrf	SysWORDTempB
	clrf	SysWORDTempB_H
	call	SysCompEqual16
	btfss	SysByteTempX,0
	goto	ENDIF16
	clrf	SYSWORDTEMPA
	clrf	SYSWORDTEMPA_H
	return
ENDIF16
	movlw	16
	movwf	SYSDIVLOOP
SYSDIV16START
	bcf	STATUS,C
	rlf	SYSDIVMULTA,F
	rlf	SYSDIVMULTA_H,F
	rlf	SYSDIVMULTX,F
	rlf	SYSDIVMULTX_H,F
	movf	SYSDIVMULTB,W
	subwf	SYSDIVMULTX,F
	movf	SYSDIVMULTB_H,W
	btfss	STATUS,C
	addlw	1
	subwf	SYSDIVMULTX_H,F
	bsf	SYSDIVMULTA,0
	btfsc	STATUS,C
	goto	ENDIF17
	bcf	SYSDIVMULTA,0
	movf	SYSDIVMULTB,W
	addwf	SYSDIVMULTX,F
	movf	SYSDIVMULTB_H,W
	btfsc	STATUS,C
	addlw	1
	addwf	SYSDIVMULTX_H,F
ENDIF17
	decfsz	SYSDIVLOOP, F
	goto	SYSDIV16START
	movf	SYSDIVMULTA,W
	movwf	SYSWORDTEMPA
	movf	SYSDIVMULTA_H,W
	movwf	SYSWORDTEMPA_H
	movf	SYSDIVMULTX,W
	movwf	SYSWORDTEMPX
	movf	SYSDIVMULTX_H,W
	movwf	SYSWORDTEMPX_H
	return

;********************************************************************************

SYSDIVSUBINT
	movf	SYSINTEGERTEMPA_H,W
	xorwf	SYSINTEGERTEMPB_H,W
	movwf	SYSSIGNBYTE
	btfss	SYSINTEGERTEMPA_H,7
	goto	ENDIF13
	comf	SYSINTEGERTEMPA,F
	comf	SYSINTEGERTEMPA_H,F
	incf	SYSINTEGERTEMPA,F
	btfsc	STATUS,Z
	incf	SYSINTEGERTEMPA_H,F
ENDIF13
	btfss	SYSINTEGERTEMPB_H,7
	goto	ENDIF14
	comf	SYSINTEGERTEMPB,F
	comf	SYSINTEGERTEMPB_H,F
	incf	SYSINTEGERTEMPB,F
	btfsc	STATUS,Z
	incf	SYSINTEGERTEMPB_H,F
ENDIF14
	call	SYSDIVSUB16
	btfss	SYSSIGNBYTE,7
	goto	ENDIF15
	comf	SYSINTEGERTEMPA,F
	comf	SYSINTEGERTEMPA_H,F
	incf	SYSINTEGERTEMPA,F
	btfsc	STATUS,Z
	incf	SYSINTEGERTEMPA_H,F
	comf	SYSINTEGERTEMPX,F
	comf	SYSINTEGERTEMPX_H,F
	incf	SYSINTEGERTEMPX,F
	btfsc	STATUS,Z
	incf	SYSINTEGERTEMPX_H,F
ENDIF15
	return

;********************************************************************************

;Start of program memory page 1
	ORG	2048
;Start of program memory page 2
	ORG	4096
;Start of program memory page 3
	ORG	6144

 END
