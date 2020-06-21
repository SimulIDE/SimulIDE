;Program compiled by Great Cow BASIC (0.94 2015-10-27)
;Need help? See the GCBASIC forums at http://sourceforge.net/projects/gcbasic/forums,
;check the documentation or email w_cholmondeley at users dot sourceforge dot net.

;********************************************************************************

;Set up the assembler options (Chip type, clock source, other bits and pieces)
 LIST p=18F2550, r=DEC
#include <P18F2550.inc>
 CONFIG LVP = OFF, MCLRE = OFF, WDT = OFF, FOSC = HS

;********************************************************************************

;Set aside memory locations for variables
DELAYTEMP	EQU	0
DELAYTEMP2	EQU	1
SYSCALCTEMPA	EQU	5
SYSSTRINGLENGTH	EQU	6
SysStringA	EQU	7
SysStringA_H	EQU	8
SysWaitTempMS	EQU	2
SysWaitTempMS_H	EQU	3
SysWaitTempS	EQU	4
SysWaitTempUS	EQU	5
SysWaitTempUS_H	EQU	6
SYSSTRINGPARAM1	EQU	2006
CC_X	EQU	9
CHARLINE	EQU	10
C_X	EQU	11
C_Y	EQU	12
DISPLAY	EQU	13
D_X	EQU	14
D_Y	EQU	15
GLCDCHAR	EQU	16
GLCDCOMDAT	EQU	17
GLCDDAT	EQU	18
GLCDINDEX	EQU	19
GLCDP_X	EQU	20
GLCDP_Y	EQU	21
PRINTLEN	EQU	22
SERDATA	EQU	23
SYSPRINTTEMP	EQU	24
StringPointer	EQU	25
SysGLCDSTRDATHandler	EQU	26
SysGLCDSTRDATHandler_H	EQU	27
SysPRINTDATAHandler	EQU	28
SysPRINTDATAHandler_H	EQU	29
SysRepeatTemp1	EQU	30
SysRepeatTemp2	EQU	31
SysRepeatTemp3	EQU	32
SysRepeatTemp3_H	EQU	33
SysRepeatTemp4	EQU	34
SysTemp1	EQU	35

;********************************************************************************

;Alias variables
AFSR0	EQU	4073
AFSR0_H	EQU	4074

;********************************************************************************

;Vectors
	ORG	0
	goto	BASPROGRAMSTART
	ORG	8
	retfie

;********************************************************************************

;Start of program memory page 0
	ORG	12
BASPROGRAMSTART
;Call initialisation routines
	rcall	INITSYS
	rcall	INITUSART
;Automatic pin direction setting
	bcf	TRISB,7,ACCESS
	bcf	TRISB,6,ACCESS
	bcf	TRISB,5,ACCESS
	bcf	TRISB,4,ACCESS
	bcf	TRISB,3,ACCESS
	bcf	TRISB,2,ACCESS
	bcf	TRISB,0,ACCESS

;Start of the main program
	bcf	LATB,3,ACCESS
	bcf	LATB,2,ACCESS
	rcall	INITGLCD
	movlw	3
	movwf	SysRepeatTemp1,BANKED
SysRepeatLoop1
	bcf	LATB,3,ACCESS
	bsf	LATB,2,ACCESS
	movlw	1
	movwf	DISPLAY,BANKED
	lfsr	1,SYSSTRINGPARAM1
	movlw	low StringTable5
	movwf	TBLPTRL,ACCESS
	movlw	high StringTable5
	movwf	TBLPTRH,ACCESS
	rcall	SysReadString
	movlw	low SYSSTRINGPARAM1
	movwf	SysPRINTDATAHandler,BANKED
	movlw	high SYSSTRINGPARAM1
	movwf	SysPRINTDATAHandler_H,BANKED
	rcall	HSERPRINT125
	movlw	10
	movwf	SERDATA,BANKED
	rcall	HSERSEND
	rcall	GLCDCLEAR
	movlw	244
	movwf	SysWaitTempMS,ACCESS
	movlw	1
	movwf	SysWaitTempMS_H,ACCESS
	rcall	Delay_MS
	bsf	LATB,3,ACCESS
	bcf	LATB,2,ACCESS
	movlw	2
	movwf	DISPLAY,BANKED
	lfsr	1,SYSSTRINGPARAM1
	movlw	low StringTable6
	movwf	TBLPTRL,ACCESS
	movlw	high StringTable6
	movwf	TBLPTRH,ACCESS
	rcall	SysReadString
	movlw	low SYSSTRINGPARAM1
	movwf	SysPRINTDATAHandler,BANKED
	movlw	high SYSSTRINGPARAM1
	movwf	SysPRINTDATAHandler_H,BANKED
	rcall	HSERPRINT125
	movlw	10
	movwf	SERDATA,BANKED
	rcall	HSERSEND
	rcall	GLCDCLEAR
	movlw	244
	movwf	SysWaitTempMS,ACCESS
	movlw	1
	movwf	SysWaitTempMS_H,ACCESS
	rcall	Delay_MS
	bcf	LATB,3,ACCESS
	bsf	LATB,2,ACCESS
	movlw	1
	movwf	DISPLAY,BANKED
	lfsr	1,SYSSTRINGPARAM1
	movlw	low StringTable5
	movwf	TBLPTRL,ACCESS
	movlw	high StringTable5
	movwf	TBLPTRH,ACCESS
	rcall	SysReadString
	movlw	low SYSSTRINGPARAM1
	movwf	SysPRINTDATAHandler,BANKED
	movlw	high SYSSTRINGPARAM1
	movwf	SysPRINTDATAHandler_H,BANKED
	rcall	HSERPRINT125
	movlw	10
	movwf	SERDATA,BANKED
	rcall	HSERSEND
	rcall	GLCDCLEAR
	rcall	GLCDDRAWFRAME
	movlw	1
	movwf	GLCDP_X,BANKED
	movlw	1
	movwf	GLCDP_Y,BANKED
	rcall	GLCDGOTOXY
	lfsr	1,SYSSTRINGPARAM1
	movlw	low StringTable1
	movwf	TBLPTRL,ACCESS
	movlw	high StringTable1
	movwf	TBLPTRH,ACCESS
	rcall	SysReadString
	movlw	low SYSSTRINGPARAM1
	movwf	SysGLCDSTRDATHandler,BANKED
	movlw	high SYSSTRINGPARAM1
	movwf	SysGLCDSTRDATHandler_H,BANKED
	rcall	GLCDPRINT9
	movlw	244
	movwf	SysWaitTempMS,ACCESS
	movlw	1
	movwf	SysWaitTempMS_H,ACCESS
	rcall	Delay_MS
	bsf	LATB,3,ACCESS
	bcf	LATB,2,ACCESS
	movlw	2
	movwf	DISPLAY,BANKED
	lfsr	1,SYSSTRINGPARAM1
	movlw	low StringTable6
	movwf	TBLPTRL,ACCESS
	movlw	high StringTable6
	movwf	TBLPTRH,ACCESS
	rcall	SysReadString
	movlw	low SYSSTRINGPARAM1
	movwf	SysPRINTDATAHandler,BANKED
	movlw	high SYSSTRINGPARAM1
	movwf	SysPRINTDATAHandler_H,BANKED
	rcall	HSERPRINT125
	movlw	10
	movwf	SERDATA,BANKED
	rcall	HSERSEND
	rcall	GLCDCLEAR
	rcall	GLCDDRAWFRAME
	rcall	GLCDDRAWFRAME
	movlw	1
	movwf	GLCDP_X,BANKED
	movlw	1
	movwf	GLCDP_Y,BANKED
	rcall	GLCDGOTOXY
	lfsr	1,SYSSTRINGPARAM1
	movlw	low StringTable2
	movwf	TBLPTRL,ACCESS
	movlw	high StringTable2
	movwf	TBLPTRH,ACCESS
	rcall	SysReadString
	movlw	low SYSSTRINGPARAM1
	movwf	SysGLCDSTRDATHandler,BANKED
	movlw	high SYSSTRINGPARAM1
	movwf	SysGLCDSTRDATHandler_H,BANKED
	rcall	GLCDPRINT9
	movlw	244
	movwf	SysWaitTempMS,ACCESS
	movlw	1
	movwf	SysWaitTempMS_H,ACCESS
	rcall	Delay_MS
	decfsz	SysRepeatTemp1,F,BANKED
	bra	SysRepeatLoop1
SysRepeatLoopEnd1
	movlw	1
	movwf	SysWaitTempS,ACCESS
	rcall	Delay_S
	clrf	CC_X,BANKED
	clrf	C_X,BANKED
	clrf	C_Y,BANKED
	clrf	D_X,BANKED
	clrf	D_Y,BANKED
SysDoLoop_S1
	bsf	LATB,0,ACCESS
	rcall	ANIMATE
	movlw	200
	movwf	SysWaitTempMS,ACCESS
	clrf	SysWaitTempMS_H,ACCESS
	rcall	Delay_MS
	bcf	LATB,0,ACCESS
	rcall	ANIMATE
	movlw	200
	movwf	SysWaitTempMS,ACCESS
	clrf	SysWaitTempMS_H,ACCESS
	rcall	Delay_MS
	bra	SysDoLoop_S1
SysDoLoop_E1
BASPROGRAMEND
	sleep
	bra	BASPROGRAMEND

;********************************************************************************

ANIMATE
	movf	C_X,W,BANKED
	sublw	78
	btfsc	STATUS, C,ACCESS
	bra	ELSE1_1
	decf	DISPLAY,W,BANKED
	btfss	STATUS, Z,ACCESS
	bra	ENDIF9
	bsf	LATB,3,ACCESS
	bcf	LATB,2,ACCESS
	movlw	2
	movwf	DISPLAY,BANKED
	lfsr	1,SYSSTRINGPARAM1
	movlw	low StringTable6
	movwf	TBLPTRL,ACCESS
	movlw	high StringTable6
	movwf	TBLPTRH,ACCESS
	rcall	SysReadString
	movlw	low SYSSTRINGPARAM1
	movwf	SysPRINTDATAHandler,BANKED
	movlw	high SYSSTRINGPARAM1
	movwf	SysPRINTDATAHandler_H,BANKED
	rcall	HSERPRINT125
	movlw	10
	movwf	SERDATA,BANKED
	rcall	HSERSEND
ENDIF9
	bra	ENDIF1
ELSE1_1
	movlw	2
	subwf	DISPLAY,W,BANKED
	btfss	STATUS, Z,ACCESS
	bra	ENDIF10
	bcf	LATB,3,ACCESS
	bsf	LATB,2,ACCESS
	movlw	1
	movwf	DISPLAY,BANKED
	lfsr	1,SYSSTRINGPARAM1
	movlw	low StringTable5
	movwf	TBLPTRL,ACCESS
	movlw	high StringTable5
	movwf	TBLPTRH,ACCESS
	rcall	SysReadString
	movlw	low SYSSTRINGPARAM1
	movwf	SysPRINTDATAHandler,BANKED
	movlw	high SYSSTRINGPARAM1
	movwf	SysPRINTDATAHandler_H,BANKED
	rcall	HSERPRINT125
	movlw	10
	movwf	SERDATA,BANKED
	rcall	HSERSEND
ENDIF10
ENDIF1
	movff	CC_X,GLCDP_X
	movff	C_Y,GLCDP_Y
	rcall	GLCDGOTOXY
	lfsr	1,SYSSTRINGPARAM1
	movlw	low StringTable3
	movwf	TBLPTRL,ACCESS
	movlw	high StringTable3
	movwf	TBLPTRH,ACCESS
	rcall	SysReadString
	movlw	low SYSSTRINGPARAM1
	movwf	SysGLCDSTRDATHandler,BANKED
	movlw	high SYSSTRINGPARAM1
	movwf	SysGLCDSTRDATHandler_H,BANKED
	rcall	GLCDPRINT9
	movf	D_X,F,BANKED
	btfss	STATUS, Z,ACCESS
	bra	ELSE2_1
	movlw	6
	addwf	C_X,F,BANKED
	bra	ENDIF2
ELSE2_1
	movlw	6
	subwf	C_X,F,BANKED
ENDIF2
	movf	D_Y,F,BANKED
	btfss	STATUS, Z,ACCESS
	bra	ELSE3_1
	incf	C_Y,F,BANKED
	bra	ENDIF3
ELSE3_1
	decf	C_Y,F,BANKED
ENDIF3
	movlw	5
	subwf	C_Y,W,BANKED
	btfss	STATUS, Z,ACCESS
	bra	ENDIF4
	movlw	1
	movwf	D_Y,BANKED
ENDIF4
	movf	C_Y,F,BANKED
	btfsc	STATUS, Z,ACCESS
	clrf	D_Y,BANKED
	movlw	162
	subwf	C_X,W,BANKED
	btfss	STATUS, Z,ACCESS
	bra	ENDIF6
	movlw	1
	movwf	D_X,BANKED
ENDIF6
	movf	C_X,F,BANKED
	btfsc	STATUS, Z,ACCESS
	clrf	D_X,BANKED
	movf	C_X,W,BANKED
	sublw	78
	btfsc	STATUS, C,ACCESS
	bra	ELSE8_1
	movlw	84
	subwf	C_X,W,BANKED
	movwf	CC_X,BANKED
	decf	DISPLAY,W,BANKED
	btfss	STATUS, Z,ACCESS
	bra	ENDIF11
	bsf	LATB,3,ACCESS
	bcf	LATB,2,ACCESS
	movlw	2
	movwf	DISPLAY,BANKED
	lfsr	1,SYSSTRINGPARAM1
	movlw	low StringTable6
	movwf	TBLPTRL,ACCESS
	movlw	high StringTable6
	movwf	TBLPTRH,ACCESS
	rcall	SysReadString
	movlw	low SYSSTRINGPARAM1
	movwf	SysPRINTDATAHandler,BANKED
	movlw	high SYSSTRINGPARAM1
	movwf	SysPRINTDATAHandler_H,BANKED
	rcall	HSERPRINT125
	movlw	10
	movwf	SERDATA,BANKED
	rcall	HSERSEND
ENDIF11
	bra	ENDIF8
ELSE8_1
	movff	C_X,CC_X
	movlw	2
	subwf	DISPLAY,W,BANKED
	btfss	STATUS, Z,ACCESS
	bra	ENDIF12
	bcf	LATB,3,ACCESS
	bsf	LATB,2,ACCESS
	movlw	1
	movwf	DISPLAY,BANKED
	lfsr	1,SYSSTRINGPARAM1
	movlw	low StringTable5
	movwf	TBLPTRL,ACCESS
	movlw	high StringTable5
	movwf	TBLPTRH,ACCESS
	rcall	SysReadString
	movlw	low SYSSTRINGPARAM1
	movwf	SysPRINTDATAHandler,BANKED
	movlw	high SYSSTRINGPARAM1
	movwf	SysPRINTDATAHandler_H,BANKED
	rcall	HSERPRINT125
	movlw	10
	movwf	SERDATA,BANKED
	rcall	HSERSEND
ENDIF12
ENDIF8
	movff	CC_X,GLCDP_X
	movff	C_Y,GLCDP_Y
	rcall	GLCDGOTOXY
	lfsr	1,SYSSTRINGPARAM1
	movlw	low StringTable4
	movwf	TBLPTRL,ACCESS
	movlw	high StringTable4
	movwf	TBLPTRH,ACCESS
	rcall	SysReadString
	movlw	low SYSSTRINGPARAM1
	movwf	SysGLCDSTRDATHandler,BANKED
	movlw	high SYSSTRINGPARAM1
	movwf	SysGLCDSTRDATHandler_H,BANKED
	rcall	GLCDPRINT9
	lfsr	1,SYSSTRINGPARAM1
	movlw	low StringTable4
	movwf	TBLPTRL,ACCESS
	movlw	high StringTable4
	movwf	TBLPTRH,ACCESS
	rcall	SysReadString
	movlw	low SYSSTRINGPARAM1
	movwf	SysPRINTDATAHandler,BANKED
	movlw	high SYSSTRINGPARAM1
	movwf	SysPRINTDATAHandler_H,BANKED
	bra	HSERPRINT125

;********************************************************************************

ASCII_TABLE_H
	movlw	241
	cpfslt	SysStringA,ACCESS
	retlw	0
	movf	SysStringA, W,ACCESS
	addlw	low TableASCII_TABLE_H
	movwf	TBLPTRL,ACCESS
	movlw	high TableASCII_TABLE_H
	btfsc	STATUS, C,ACCESS
	addlw	1
	movwf	TBLPTRH,ACCESS
	tblrd*
	movf	TABLAT, W,ACCESS
	return
TableASCII_TABLE_H
	db	240,127,9,9,9,6,62,65,81,33,94,127,9,25,41,70,70,73,73,73,49,1,1,127,1,1,63,64
	db	64,64,63,31,32,64,32,31,63,64,56,64,63,99,20,8,20,99,7,8,112,8,7,97,81,73,69,67
	db	0,127,65,65,0,2,4,8,16,32,0,65,65,127,0,4,2,1,2,4,64,64,64,64,64,0,1,2,4,0,32,84
	db	84,84,120,127,72,68,68,56,56,68,68,68,32,56,68,68,72,127,56,84,84,84,24,8,126,9
	db	1,2,12,82,82,82,62,127,8,4,4,120,0,68,125,64,0,32,64,68,61,0,127,16,40,68,0,0
	db	65,127,64,0,124,4,24,4,120,124,8,4,4,120,56,68,68,68,56,124,20,20,20,8,8,20,20,24
	db	124,124,8,4,4,8,72,84,84,84,32,4,63,68,64,32,60,64,64,32,124,28,32,64,32,28,60,64
	db	48,64,60,68,40,16,40,68,12,80,80,80,60,68,100,84,76,68,0,8,54,65,0,0,0,127,0,0
	db	0,65,54,8,0,16,8,8,16,8,0,6,9,9,6

;********************************************************************************

ASCII_TABLE_L
	movlw	241
	cpfslt	SysStringA,ACCESS
	retlw	0
	movf	SysStringA, W,ACCESS
	addlw	low TableASCII_TABLE_L
	movwf	TBLPTRL,ACCESS
	movlw	high TableASCII_TABLE_L
	btfsc	STATUS, C,ACCESS
	addlw	1
	movwf	TBLPTRH,ACCESS
	tblrd*
	movf	TABLAT, W,ACCESS
	return
TableASCII_TABLE_L
	db	240,0,0,0,0,0,0,0,95,0,0,0,7,0,7,0,20,127,20,127,20,36,42,127,42,18,35,19,8,100
	db	98,54,73,85,34,80,0,5,3,0,0,0,28,34,65,0,0,65,34,28,0,20,8,62,8,20,8,8,62,8,8,0
	db	80,48,0,0,8,8,8,8,8,0,96,96,0,0,32,16,8,4,2,62,81,73,69,62,0,66,127,64,0,66,97,81
	db	73,70,33,65,69,75,49,24,20,18,127,16,39,69,69,69,57,60,74,73,73,48,1,113,9,5,3,54
	db	73,73,73,54,6,73,73,41,30,0,54,54,0,0,0,86,54,0,0,8,20,34,65,0,20,20,20,20,20,0
	db	65,34,20,8,2,1,81,9,6,50,73,121,65,62,126,17,17,17,126,127,73,73,73,54,62,65,65,65
	db	34,127,65,65,34,28,127,73,73,73,65,127,9,9,9,1,62,65,73,73,122,127,8,8,8,127,0,65
	db	127,65,0,32,64,65,63,1,127,8,20,34,65,127,64,64,64,64,127,2,12,2,127,127,4,8,16,127
	db	62,65,65,65,62

;********************************************************************************

Delay_MS
	incf	SysWaitTempMS_H, F,ACCESS
DMS_START
	movlw	108
	movwf	DELAYTEMP2,ACCESS
DMS_OUTER
	movlw	11
	movwf	DELAYTEMP,ACCESS
DMS_INNER
	decfsz	DELAYTEMP, F,ACCESS
	bra	DMS_INNER
	decfsz	DELAYTEMP2, F,ACCESS
	bra	DMS_OUTER
	decfsz	SysWaitTempMS, F,ACCESS
	bra	DMS_START
	decfsz	SysWaitTempMS_H, F,ACCESS
	bra	DMS_START
	return

;********************************************************************************

Delay_S
DS_START
	movlw	232
	movwf	SysWaitTempMS,ACCESS
	movlw	3
	movwf	SysWaitTempMS_H,ACCESS
	rcall	Delay_MS
	decfsz	SysWaitTempS, F,ACCESS
	bra	DS_START
	return

;********************************************************************************

GLCDCHARACTER
	movlw	80
	subwf	GLCDCHAR,W,BANKED
	btfsc	STATUS, C,ACCESS
	bra	ELSE24_1
	movlw	32
	subwf	GLCDCHAR,W,BANKED
	movwf	SysTemp1,BANKED
	mullw	5
	movff	PRODL,GLCDCHAR
	movlw	5
	movwf	SysRepeatTemp4,BANKED
SysRepeatLoop4
	incf	GLCDCHAR,F,BANKED
	movff	GLCDCHAR,SysStringA
	rcall	ASCII_TABLE_L
	movwf	CHARLINE,BANKED
	movlw	1
	movwf	GLCDCOMDAT,BANKED
	movff	CHARLINE,GLCDDAT
	rcall	GLCDWRITE
	movff	GLCDDAT,CHARLINE
	decfsz	SysRepeatTemp4,F,BANKED
	bra	SysRepeatLoop4
SysRepeatLoopEnd4
	bra	ENDIF24
ELSE24_1
	movlw	80
	subwf	GLCDCHAR,W,BANKED
	movwf	SysTemp1,BANKED
	mullw	5
	movff	PRODL,GLCDCHAR
	movlw	5
	movwf	SysRepeatTemp4,BANKED
SysRepeatLoop5
	incf	GLCDCHAR,F,BANKED
	movff	GLCDCHAR,SysStringA
	rcall	ASCII_TABLE_H
	movwf	CHARLINE,BANKED
	movlw	1
	movwf	GLCDCOMDAT,BANKED
	movff	CHARLINE,GLCDDAT
	rcall	GLCDWRITE
	movff	GLCDDAT,CHARLINE
	decfsz	SysRepeatTemp4,F,BANKED
	bra	SysRepeatLoop5
SysRepeatLoopEnd5
ENDIF24
	movlw	1
	movwf	GLCDCOMDAT,BANKED
	clrf	GLCDDAT,BANKED
	bra	GLCDWRITE

;********************************************************************************

GLCDCLEAR
	movlw	247
	movwf	SysRepeatTemp3,BANKED
	movlw	2
	movwf	SysRepeatTemp3_H,BANKED
SysRepeatLoop3
	movlw	1
	movwf	GLCDCOMDAT,BANKED
	clrf	GLCDDAT,BANKED
	rcall	GLCDWRITE
	decfsz	SysRepeatTemp3,F,BANKED
	bra	SysRepeatLoop3
	decfsz	SysRepeatTemp3_H,F,BANKED
	bra	SysRepeatLoop3
SysRepeatLoopEnd3
	return

;********************************************************************************

GLCDDRAWFRAME
	setf	GLCDINDEX,BANKED
SysForLoop2
	incf	GLCDINDEX,F,BANKED
	movff	GLCDINDEX,GLCDP_X
	clrf	GLCDP_Y,BANKED
	rcall	GLCDGOTOXY
	movlw	1
	movwf	GLCDCOMDAT,BANKED
	movlw	1
	movwf	GLCDDAT,BANKED
	rcall	GLCDWRITE
	movlw	83
	subwf	GLCDINDEX,W,BANKED
	btfss	STATUS, C,ACCESS
	bra	SysForLoop2
SysForLoopEnd2
	setf	GLCDINDEX,BANKED
SysForLoop3
	incf	GLCDINDEX,F,BANKED
	movff	GLCDINDEX,GLCDP_X
	movlw	5
	movwf	GLCDP_Y,BANKED
	rcall	GLCDGOTOXY
	movlw	1
	movwf	GLCDCOMDAT,BANKED
	movlw	128
	movwf	GLCDDAT,BANKED
	rcall	GLCDWRITE
	movlw	83
	subwf	GLCDINDEX,W,BANKED
	btfss	STATUS, C,ACCESS
	bra	SysForLoop3
SysForLoopEnd3
	setf	GLCDINDEX,BANKED
SysForLoop4
	incf	GLCDINDEX,F,BANKED
	movlw	83
	movwf	GLCDP_X,BANKED
	movff	GLCDINDEX,GLCDP_Y
	rcall	GLCDGOTOXY
	movlw	1
	movwf	GLCDCOMDAT,BANKED
	setf	GLCDDAT,BANKED
	rcall	GLCDWRITE
	movlw	5
	subwf	GLCDINDEX,W,BANKED
	btfss	STATUS, C,ACCESS
	bra	SysForLoop4
SysForLoopEnd4
	setf	GLCDINDEX,BANKED
SysForLoop5
	incf	GLCDINDEX,F,BANKED
	clrf	GLCDP_X,BANKED
	movff	GLCDINDEX,GLCDP_Y
	rcall	GLCDGOTOXY
	movlw	1
	movwf	GLCDCOMDAT,BANKED
	setf	GLCDDAT,BANKED
	rcall	GLCDWRITE
	movlw	5
	subwf	GLCDINDEX,W,BANKED
	btfss	STATUS, C,ACCESS
	bra	SysForLoop5
SysForLoopEnd5
	return

;********************************************************************************

GLCDGOTOXY
	clrf	GLCDCOMDAT,BANKED
	movlw	128
	iorwf	GLCDP_X,W,BANKED
	movwf	GLCDDAT,BANKED
	rcall	GLCDWRITE
	clrf	GLCDCOMDAT,BANKED
	movlw	64
	iorwf	GLCDP_Y,W,BANKED
	movwf	GLCDDAT,BANKED
	bra	GLCDWRITE

;********************************************************************************

;Overloaded signature: STRING:
GLCDPRINT9
	clrf	GLCDINDEX,BANKED
	movff	SysGLCDSTRDATHandler,AFSR0
	movff	SysGLCDSTRDATHandler_H,AFSR0_H
	movlw	1
	subwf	INDF0,W,ACCESS
	btfss	STATUS, C,ACCESS
	bra	SysForLoopEnd1
SysForLoop1
	incf	GLCDINDEX,F,BANKED
	movf	GLCDINDEX,W,BANKED
	addwf	SysGLCDSTRDATHandler,W,BANKED
	movwf	AFSR0,ACCESS
	movlw	0
	addwfc	SysGLCDSTRDATHandler_H,W,BANKED
	movwf	AFSR0_H,ACCESS
	movff	INDF0,GLCDCHAR
	rcall	GLCDCHARACTER
	movf	GLCDINDEX,W,BANKED
	addwf	SysGLCDSTRDATHandler,W,BANKED
	movwf	AFSR0,ACCESS
	movlw	0
	addwfc	SysGLCDSTRDATHandler_H,W,BANKED
	movwf	AFSR0_H,ACCESS
	movff	GLCDCHAR,INDF0
	movff	SysGLCDSTRDATHandler,AFSR0
	movff	SysGLCDSTRDATHandler_H,AFSR0_H
	movf	INDF0,W,ACCESS
	subwf	GLCDINDEX,W,BANKED
	btfss	STATUS, C,ACCESS
	bra	SysForLoop1
SysForLoopEnd1
	return

;********************************************************************************

GLCDWRITE
	decf	GLCDCOMDAT,W,BANKED
	btfss	STATUS, Z,ACCESS
	bra	ELSE13_1
	bsf	LATB,6,ACCESS
	bra	ENDIF13
ELSE13_1
	bcf	LATB,6,ACCESS
ENDIF13
	movlw	8
	movwf	SysRepeatTemp2,BANKED
SysRepeatLoop2
	bcf	LATB,4,ACCESS
	btfss	GLCDDAT,7,BANKED
	bra	ELSE14_1
	bsf	LATB,5,ACCESS
	bra	ENDIF14
ELSE14_1
	bcf	LATB,5,ACCESS
ENDIF14
	rlcf	GLCDDAT,F,BANKED
	bsf	LATB,4,ACCESS
	nop
	nop
	nop
	nop
	decfsz	SysRepeatTemp2,F,BANKED
	bra	SysRepeatLoop2
SysRepeatLoopEnd2
	return

;********************************************************************************

;Overloaded signature: STRING:
HSERPRINT125
	movff	SysPRINTDATAHandler,AFSR0
	movff	SysPRINTDATAHandler_H,AFSR0_H
	movff	INDF0,PRINTLEN
	movf	PRINTLEN,F,BANKED
	btfsc	STATUS, Z,ACCESS
	bra	ENDIF21
	clrf	SYSPRINTTEMP,BANKED
	movlw	1
	subwf	PRINTLEN,W,BANKED
	btfss	STATUS, C,ACCESS
	bra	SysForLoopEnd6
SysForLoop6
	incf	SYSPRINTTEMP,F,BANKED
	movf	SYSPRINTTEMP,W,BANKED
	addwf	SysPRINTDATAHandler,W,BANKED
	movwf	AFSR0,ACCESS
	movlw	0
	addwfc	SysPRINTDATAHandler_H,W,BANKED
	movwf	AFSR0_H,ACCESS
	movff	INDF0,SERDATA
	rcall	HSERSEND
	movlw	12
	movwf	SysWaitTempMS,ACCESS
	clrf	SysWaitTempMS_H,ACCESS
	rcall	Delay_MS
	movf	PRINTLEN,W,BANKED
	subwf	SYSPRINTTEMP,W,BANKED
	btfss	STATUS, C,ACCESS
	bra	SysForLoop6
SysForLoopEnd6
ENDIF21
	return

;********************************************************************************

HSERSEND
SysWaitLoop1
	btfss	PIR1,TXIF,ACCESS
	bra	SysWaitLoop1
	movff	SERDATA,TXREG
	return

;********************************************************************************

INITGLCD
	bcf	LATB,7,ACCESS
	movlw	100
	movwf	SysWaitTempMS,ACCESS
	clrf	SysWaitTempMS_H,ACCESS
	rcall	Delay_MS
	bsf	LATB,7,ACCESS
	clrf	GLCDCOMDAT,BANKED
	movlw	33
	movwf	GLCDDAT,BANKED
	rcall	GLCDWRITE
	clrf	GLCDCOMDAT,BANKED
	movlw	191
	movwf	GLCDDAT,BANKED
	rcall	GLCDWRITE
	clrf	GLCDCOMDAT,BANKED
	movlw	4
	movwf	GLCDDAT,BANKED
	rcall	GLCDWRITE
	clrf	GLCDCOMDAT,BANKED
	movlw	20
	movwf	GLCDDAT,BANKED
	rcall	GLCDWRITE
	clrf	GLCDCOMDAT,BANKED
	movlw	12
	movwf	GLCDDAT,BANKED
	rcall	GLCDWRITE
	clrf	GLCDCOMDAT,BANKED
	movlw	32
	movwf	GLCDDAT,BANKED
	rcall	GLCDWRITE
	clrf	GLCDCOMDAT,BANKED
	movlw	12
	movwf	GLCDDAT,BANKED
	bra	GLCDWRITE

;********************************************************************************

INITSYS
	clrf	BSR,ACCESS
	clrf	TBLPTRU,ACCESS
	bcf	ADCON2,ADFM,ACCESS
	bcf	ADCON0,ADON,ACCESS
	bsf	ADCON1,PCFG3,ACCESS
	bsf	ADCON1,PCFG2,ACCESS
	bsf	ADCON1,PCFG1,ACCESS
	bsf	ADCON1,PCFG0,ACCESS
	movlw	7
	movwf	CMCON,ACCESS
	clrf	PORTA,ACCESS
	clrf	PORTB,ACCESS
	clrf	PORTC,ACCESS
	clrf	PORTE,ACCESS
	return

;********************************************************************************

INITUSART
	movlw	33
	movwf	SPBRG,ACCESS
	clrf	SPBRGH,ACCESS
	bsf	BAUDCON,BRG16,ACCESS
	bsf	TXSTA,BRGH,ACCESS
	bcf	TXSTA,SYNC,ACCESS
	bsf	RCSTA,SPEN,ACCESS
	bsf	RCSTA,CREN,ACCESS
	bsf	TXSTA,TXEN,ACCESS
	return

;********************************************************************************

SYSREADSTRING
	tblrd*+
	movff	TABLAT,SYSCALCTEMPA
	movff	TABLAT,INDF1
	tblrd*+
	bra	SYSSTRINGREADCHECK
SYSREADSTRINGPART
	tblrd*+
	movf	TABLAT, W,ACCESS
	movwf	SYSCALCTEMPA,ACCESS
	addwf	SYSSTRINGLENGTH,F,ACCESS
	tblrd*+
SYSSTRINGREADCHECK
	movf	SYSCALCTEMPA,F,ACCESS
	btfsc	STATUS,Z,ACCESS
	return
SYSSTRINGREAD
	tblrd*+
	movff	TABLAT,PREINC1
	decfsz	SYSCALCTEMPA, F,ACCESS
	bra	SYSSTRINGREAD
	return

;********************************************************************************

SysStringTables

StringTable1
	dw	2827,"  Display-1"


StringTable2
	dw	2827,"  Display-2"


StringTable3
	dw	257," "


StringTable4
	dw	257,"*"


StringTable5
	dw	4883,"Printing Display: 1"


StringTable6
	dw	4883,"Printing Display: 2"


;********************************************************************************


 END
