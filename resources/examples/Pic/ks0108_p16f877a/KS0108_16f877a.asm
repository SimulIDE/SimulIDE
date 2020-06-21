;Program compiled by Great Cow BASIC (0.96.<<>> 2016-12-14)
;Need help? See the GCBASIC forums at http://sourceforge.net/projects/gcbasic/forums,
;check the documentation or email w_cholmondeley at users dot sourceforge dot net.

;********************************************************************************

;Set up the assembler options (Chip type, clock source, other bits and pieces)
 LIST p=16F877A, r=DEC
#include <P16F877A.inc>
 __CONFIG _LVP_OFF & _WDTE_OFF & _FOSC_HS

;********************************************************************************

;Set aside memory locations for variables
CHAR	EQU	32
CHARCODE	EQU	33
CHARCOL	EQU	34
CHARCOLS	EQU	36
CHARCOL_H	EQU	35
CHARLOCX	EQU	37
CHARLOCX_H	EQU	38
CHARLOCY	EQU	39
CHARLOCY_H	EQU	40
CHARROW	EQU	41
CHARROWS	EQU	43
CHARROW_H	EQU	42
COL	EQU	44
CURRCHARCOL	EQU	45
CURRCHARROW	EQU	46
CURRCHARVAL	EQU	47
CURRCOL	EQU	48
CURRPAGE	EQU	49
DELAYTEMP	EQU	112
DELAYTEMP2	EQU	113
DRAWLINE	EQU	50
DRAWLINE_H	EQU	51
GLCDBACKGROUND	EQU	52
GLCDBACKGROUND_H	EQU	53
GLCDBITNO	EQU	54
GLCDCHANGE	EQU	55
GLCDCOLOUR	EQU	56
GLCDCOLOUR_H	EQU	57
GLCDDATATEMP	EQU	58
GLCDFNTDEFAULT	EQU	59
GLCDFNTDEFAULTSIZE	EQU	60
GLCDFONTWIDTH	EQU	61
GLCDFOREGROUND	EQU	62
GLCDFOREGROUND_H	EQU	63
GLCDPRINTLOC	EQU	64
GLCDPRINTLOC_H	EQU	65
GLCDREADBYTE_KS0108	EQU	66
GLCDTEMP	EQU	67
GLCDX	EQU	68
GLCDY	EQU	69
GLCD_COUNT	EQU	70
GLCD_YORDINATE	EQU	71
GLCD_YORDINATE_H	EQU	72
LCDBYTE	EQU	73
LINECOLOUR	EQU	74
LINECOLOUR_H	EQU	75
LINEDIFFX	EQU	76
LINEDIFFX_H	EQU	77
LINEDIFFX_X2	EQU	78
LINEDIFFX_X2_H	EQU	79
LINEDIFFY	EQU	80
LINEDIFFY_H	EQU	81
LINEDIFFY_X2	EQU	82
LINEDIFFY_X2_H	EQU	83
LINEERR	EQU	84
LINEERR_H	EQU	85
LINESTEPX	EQU	86
LINESTEPX_H	EQU	87
LINESTEPY	EQU	88
LINESTEPY_H	EQU	89
LINEX1	EQU	90
LINEX1_H	EQU	91
LINEX2	EQU	92
LINEX2_H	EQU	93
LINEY1	EQU	94
LINEY1_H	EQU	95
LINEY2	EQU	96
LINEY2_H	EQU	97
PRINTLEN	EQU	98
PRINTLOCX	EQU	99
PRINTLOCX_H	EQU	100
PRINTLOCY	EQU	101
PRINTLOCY_H	EQU	102
ROW	EQU	103
STR	EQU	447
STRINGPOINTER	EQU	104
SYSBITVAR0	EQU	105
SYSBYTETEMPA	EQU	117
SYSBYTETEMPB	EQU	121
SYSBYTETEMPX	EQU	112
SYSCALCTEMPA	EQU	117
SYSCALCTEMPX	EQU	112
SYSCALCTEMPX_H	EQU	113
SYSCHARCOUNT	EQU	106
SYSDIVLOOP	EQU	116
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
SYSLCDPRINTDATAHANDLER	EQU	107
SYSLCDPRINTDATAHANDLER_H	EQU	108
SYSPRINTTEMP	EQU	109
SYSREPEATTEMP1	EQU	110
SYSSIGNBYTE	EQU	125
SYSSTRDATA	EQU	111
SYSSTRINGA	EQU	119
SYSSTRINGA_H	EQU	120
SYSSTRINGB	EQU	114
SYSSTRINGB_H	EQU	115
SYSSTRINGLENGTH	EQU	118
SYSSTRINGPARAM1	EQU	453
SYSTEMP1	EQU	126
SYSTEMP1_H	EQU	127
SYSTEMP2	EQU	160
SYSVALTEMP	EQU	161
SYSVALTEMP_H	EQU	162
SYSWAITTEMPMS	EQU	114
SYSWAITTEMPMS_H	EQU	115
SYSWAITTEMPS	EQU	116
SYSWAITTEMPUS	EQU	117
SYSWAITTEMPUS_H	EQU	118
SYSWORDTEMPA	EQU	117
SYSWORDTEMPA_H	EQU	118
SYSWORDTEMPB	EQU	121
SYSWORDTEMPB_H	EQU	122
SYSWORDTEMPX	EQU	112
SYSWORDTEMPX_H	EQU	113
XVAR	EQU	163

;********************************************************************************

;Alias variables
SYSSTR_0	EQU	447

;********************************************************************************

;Vectors
	ORG	0
	pagesel	BASPROGRAMSTART
	goto	BASPROGRAMSTART
	ORG	4
	retfie

;********************************************************************************

;Start of program memory page 0
	ORG	5
BASPROGRAMSTART
;Call initialisation routines
	pagesel	INITSYS
	call	INITSYS
	pagesel	$
	call	INITGLCD_KS0108

;Start of the main program
START
	pagesel	GLCDCLS_KS0108
	call	GLCDCLS_KS0108
	pagesel	$

	clrf	PRINTLOCX
	clrf	PRINTLOCX_H
	movlw	10
	movwf	PRINTLOCY
	clrf	PRINTLOCY_H
	movlw	low SYSSTRINGPARAM1
	movwf	SysStringB
	movlw	high SYSSTRINGPARAM1
	movwf	SysStringB_H
	movlw	low StringTable1
	movwf	SysStringA
	movlw	high StringTable1
	movwf	SysStringA_H
	pagesel	SysReadString
	call	SysReadString
	pagesel	$
	movlw	low SYSSTRINGPARAM1
	movwf	SysLCDPRINTDATAHandler
	movlw	high SYSSTRINGPARAM1
	movwf	SysLCDPRINTDATAHandler_H
	call	GLCDPRINT3

	movlw	1
	movwf	SysWaitTempS
	pagesel	Delay_S
	call	Delay_S
	pagesel	$
	clrf	PRINTLOCX
	clrf	PRINTLOCX_H
	movlw	10
	movwf	PRINTLOCY
	clrf	PRINTLOCY_H
	movlw	low SYSSTRINGPARAM1
	movwf	SysStringB
	movlw	high SYSSTRINGPARAM1
	movwf	SysStringB_H
	movlw	low StringTable2
	movwf	SysStringA
	movlw	high StringTable2
	movwf	SysStringA_H
	pagesel	SysReadString
	call	SysReadString
	pagesel	$
	movlw	low SYSSTRINGPARAM1
	movwf	SysLCDPRINTDATAHandler
	movlw	high SYSSTRINGPARAM1
	movwf	SysLCDPRINTDATAHandler_H
	call	GLCDPRINT3

	movlw	18
	movwf	LINEX1
	clrf	LINEX1_H
	movlw	30
	movwf	LINEY1
	clrf	LINEY1_H
	movlw	28
	movwf	LINEX2
	clrf	LINEX2_H
	movlw	40
	movwf	LINEY2
	clrf	LINEY2_H
	movf	GLCDFOREGROUND,W
	movwf	LINECOLOUR
	movf	GLCDFOREGROUND_H,W
	movwf	LINECOLOUR_H
	call	BOX

	movlw	14
	movwf	CHAR
SysForLoop1
	incf	CHAR,F
	movlw	17
	movwf	PRINTLOCX
	clrf	PRINTLOCX_H
	movlw	20
	movwf	PRINTLOCY
	clrf	PRINTLOCY_H
	movf	CHAR,W
	banksel	SYSVALTEMP
	movwf	SYSVALTEMP
	clrf	SYSVALTEMP_H
	banksel	STATUS
	call	FN_STR
	movlw	low STR
	movwf	SysLCDPRINTDATAHandler
	movlw	high STR
	movwf	SysLCDPRINTDATAHandler_H
	call	GLCDPRINT3

	movlw	20
	movwf	CHARLOCX
	clrf	CHARLOCX_H
	movlw	30
	movwf	CHARLOCY
	clrf	CHARLOCY_H
	movf	CHAR,W
	movwf	CHARCODE
	movf	GLCDFOREGROUND,W
	movwf	LINECOLOUR
	movf	GLCDFOREGROUND_H,W
	movwf	LINECOLOUR_H
	call	GLCDDRAWCHAR

	movlw	1
	movwf	SysWaitTempS
	pagesel	Delay_S
	call	Delay_S
	pagesel	$
	movlw	129
	subwf	CHAR,W
	btfss	STATUS, C
	goto	SysForLoop1
SysForLoopEnd1
	clrf	LINEX1
	clrf	LINEX1_H
	movlw	50
	movwf	LINEY1
	clrf	LINEY1_H
	movlw	127
	movwf	LINEX2
	clrf	LINEX2_H
	movlw	50
	movwf	LINEY2
	clrf	LINEY2_H
	movf	GLCDFOREGROUND,W
	movwf	LINECOLOUR
	movf	GLCDFOREGROUND_H,W
	movwf	LINECOLOUR_H
	call	LINE

	movlw	255
	banksel	XVAR
	movwf	XVAR
SysForLoop2
	incf	XVAR,F
	movf	XVAR,W
	banksel	GLCDX
	movwf	GLCDX
	movlw	63
	movwf	GLCDY
	movlw	1
	movwf	GLCDCOLOUR
	clrf	GLCDCOLOUR_H
	call	PSET_KS0108

	movlw	80
	banksel	XVAR
	subwf	XVAR,W
	btfss	STATUS, C
	goto	SysForLoop2
SysForLoopEnd2
	movlw	10
	movwf	SysWaitTempS
	banksel	STATUS
	pagesel	Delay_S
	call	Delay_S
	pagesel	$
	goto	START
	goto	BASPROGRAMEND
BASPROGRAMEND
	sleep
	goto	BASPROGRAMEND

;********************************************************************************

BOX
	movf	LINEX1,W
	movwf	SysWORDTempB
	movf	LINEX1_H,W
	movwf	SysWORDTempB_H
	movf	LINEX2,W
	movwf	SysWORDTempA
	movf	LINEX2_H,W
	movwf	SysWORDTempA_H
	call	SysCompLessThan16
	btfss	SysByteTempX,0
	goto	ENDIF15
	movf	LINEX1,W
	movwf	GLCDTEMP
	movf	LINEX2,W
	movwf	LINEX1
	movf	LINEX2_H,W
	movwf	LINEX1_H
	movf	GLCDTEMP,W
	movwf	LINEX2
	clrf	LINEX2_H
ENDIF15
	movf	LINEY1,W
	movwf	SysWORDTempB
	movf	LINEY1_H,W
	movwf	SysWORDTempB_H
	movf	LINEY2,W
	movwf	SysWORDTempA
	movf	LINEY2_H,W
	movwf	SysWORDTempA_H
	call	SysCompLessThan16
	btfss	SysByteTempX,0
	goto	ENDIF16
	movf	LINEY1,W
	movwf	GLCDTEMP
	movf	LINEY2,W
	movwf	LINEY1
	movf	LINEY2_H,W
	movwf	LINEY1_H
	movf	GLCDTEMP,W
	movwf	LINEY2
	clrf	LINEY2_H
ENDIF16
	movlw	1
	subwf	LINEX1,W
	movwf	DRAWLINE
	movlw	0
	btfss	STATUS,C
	addlw	1
	subwf	LINEX1_H,W
	movwf	DRAWLINE_H
	movf	LINEX1,W
	movwf	SysWORDTempB
	movf	LINEX1_H,W
	movwf	SysWORDTempB_H
	movf	LINEX2,W
	movwf	SysWORDTempA
	movf	LINEX2_H,W
	movwf	SysWORDTempA_H
	call	SysCompLessThan16
	btfsc	SysByteTempX,0
	goto	SysForLoopEnd8
SysForLoop8
	incf	DRAWLINE,F
	btfsc	STATUS,Z
	incf	DRAWLINE_H,F
	movf	DRAWLINE,W
	movwf	GLCDX
	movf	LINEY1,W
	movwf	GLCDY
	movf	LINECOLOUR,W
	movwf	GLCDCOLOUR
	movf	LINECOLOUR_H,W
	movwf	GLCDCOLOUR_H
	call	PSET_KS0108

	movf	DRAWLINE,W
	movwf	GLCDX
	movf	LINEY2,W
	movwf	GLCDY
	movf	LINECOLOUR,W
	movwf	GLCDCOLOUR
	movf	LINECOLOUR_H,W
	movwf	GLCDCOLOUR_H
	call	PSET_KS0108

	movf	DRAWLINE,W
	movwf	SysWORDTempA
	movf	DRAWLINE_H,W
	movwf	SysWORDTempA_H
	movf	LINEX2,W
	movwf	SysWORDTempB
	movf	LINEX2_H,W
	movwf	SysWORDTempB_H
	call	SysCompLessThan16
	btfsc	SysByteTempX,0
	goto	SysForLoop8
SysForLoopEnd8
	movlw	1
	subwf	LINEY1,W
	movwf	DRAWLINE
	movlw	0
	btfss	STATUS,C
	addlw	1
	subwf	LINEY1_H,W
	movwf	DRAWLINE_H
	movf	LINEY1,W
	movwf	SysWORDTempB
	movf	LINEY1_H,W
	movwf	SysWORDTempB_H
	movf	LINEY2,W
	movwf	SysWORDTempA
	movf	LINEY2_H,W
	movwf	SysWORDTempA_H
	call	SysCompLessThan16
	btfsc	SysByteTempX,0
	goto	SysForLoopEnd9
SysForLoop9
	incf	DRAWLINE,F
	btfsc	STATUS,Z
	incf	DRAWLINE_H,F
	movf	LINEX1,W
	movwf	GLCDX
	movf	DRAWLINE,W
	movwf	GLCDY
	movf	LINECOLOUR,W
	movwf	GLCDCOLOUR
	movf	LINECOLOUR_H,W
	movwf	GLCDCOLOUR_H
	call	PSET_KS0108

	movf	LINEX2,W
	movwf	GLCDX
	movf	DRAWLINE,W
	movwf	GLCDY
	movf	LINECOLOUR,W
	movwf	GLCDCOLOUR
	movf	LINECOLOUR_H,W
	movwf	GLCDCOLOUR_H
	call	PSET_KS0108

	movf	DRAWLINE,W
	movwf	SysWORDTempA
	movf	DRAWLINE_H,W
	movwf	SysWORDTempA_H
	movf	LINEY2,W
	movwf	SysWORDTempB
	movf	LINEY2_H,W
	movwf	SysWORDTempB_H
	call	SysCompLessThan16
	btfsc	SysByteTempX,0
	goto	SysForLoop9
SysForLoopEnd9
	return

;********************************************************************************

GLCDCHARCOL3
	movlw	113
	subwf	SysStringA, W
	btfsc	STATUS, C
	retlw	0
	movf	SysStringA, W
	addlw	low TableGLCDCHARCOL3
	movwf	SysStringA
	movlw	high TableGLCDCHARCOL3
	btfsc	STATUS, C
	addlw	1
	movwf	PCLATH
	movf	SysStringA, W
	movwf	PCL
TableGLCDCHARCOL3
	retlw	112
	retlw	0
	retlw	16
	retlw	12
	retlw	10
	retlw	136
	retlw	34
	retlw	56
	retlw	32
	retlw	8
	retlw	32
	retlw	16
	retlw	16
	retlw	128
	retlw	128
	retlw	64
	retlw	4
	retlw	0
	retlw	0
	retlw	0
	retlw	40
	retlw	72
	retlw	70
	retlw	108
	retlw	0
	retlw	0
	retlw	0
	retlw	40
	retlw	16
	retlw	0
	retlw	16
	retlw	0
	retlw	64
	retlw	124
	retlw	0
	retlw	132
	retlw	130
	retlw	48
	retlw	78
	retlw	120
	retlw	6
	retlw	108
	retlw	12
	retlw	0
	retlw	0
	retlw	16
	retlw	40
	retlw	0
	retlw	4
	retlw	100
	retlw	248
	retlw	254
	retlw	124
	retlw	254
	retlw	254
	retlw	254
	retlw	124
	retlw	254
	retlw	0
	retlw	64
	retlw	254
	retlw	254
	retlw	254
	retlw	254
	retlw	124
	retlw	254
	retlw	124
	retlw	254
	retlw	76
	retlw	2
	retlw	126
	retlw	62
	retlw	126
	retlw	198
	retlw	14
	retlw	194
	retlw	0
	retlw	4
	retlw	0
	retlw	8
	retlw	128
	retlw	0
	retlw	64
	retlw	254
	retlw	112
	retlw	112
	retlw	112
	retlw	16
	retlw	16
	retlw	254
	retlw	0
	retlw	64
	retlw	254
	retlw	0
	retlw	248
	retlw	248
	retlw	112
	retlw	248
	retlw	16
	retlw	248
	retlw	144
	retlw	16
	retlw	120
	retlw	56
	retlw	120
	retlw	136
	retlw	24
	retlw	136
	retlw	0
	retlw	0
	retlw	0
	retlw	32
	retlw	120

;********************************************************************************

GLCDCHARCOL4
	movlw	113
	subwf	SysStringA, W
	btfsc	STATUS, C
	retlw	0
	movf	SysStringA, W
	addlw	low TableGLCDCHARCOL4
	movwf	SysStringA
	movlw	high TableGLCDCHARCOL4
	btfsc	STATUS, C
	addlw	1
	movwf	PCLATH
	movf	SysStringA, W
	movwf	PCL
TableGLCDCHARCOL4
	retlw	112
	retlw	254
	retlw	56
	retlw	10
	retlw	6
	retlw	204
	retlw	102
	retlw	124
	retlw	112
	retlw	4
	retlw	64
	retlw	16
	retlw	56
	retlw	136
	retlw	162
	retlw	112
	retlw	28
	retlw	0
	retlw	0
	retlw	14
	retlw	254
	retlw	84
	retlw	38
	retlw	146
	retlw	10
	retlw	56
	retlw	130
	retlw	16
	retlw	16
	retlw	160
	retlw	16
	retlw	192
	retlw	32
	retlw	162
	retlw	132
	retlw	194
	retlw	130
	retlw	40
	retlw	138
	retlw	148
	retlw	2
	retlw	146
	retlw	146
	retlw	108
	retlw	172
	retlw	40
	retlw	40
	retlw	130
	retlw	2
	retlw	146
	retlw	36
	retlw	146
	retlw	130
	retlw	130
	retlw	146
	retlw	18
	retlw	130
	retlw	16
	retlw	130
	retlw	128
	retlw	16
	retlw	128
	retlw	4
	retlw	8
	retlw	130
	retlw	18
	retlw	130
	retlw	18
	retlw	146
	retlw	2
	retlw	128
	retlw	64
	retlw	128
	retlw	40
	retlw	16
	retlw	162
	retlw	254
	retlw	8
	retlw	130
	retlw	4
	retlw	128
	retlw	2
	retlw	168
	retlw	144
	retlw	136
	retlw	136
	retlw	168
	retlw	252
	retlw	168
	retlw	16
	retlw	144
	retlw	128
	retlw	32
	retlw	130
	retlw	8
	retlw	16
	retlw	136
	retlw	40
	retlw	40
	retlw	16
	retlw	168
	retlw	124
	retlw	128
	retlw	64
	retlw	128
	retlw	80
	retlw	160
	retlw	200
	retlw	16
	retlw	0
	retlw	130
	retlw	16
	retlw	68

;********************************************************************************

GLCDCHARCOL5
	movlw	113
	subwf	SysStringA, W
	btfsc	STATUS, C
	retlw	0
	movf	SysStringA, W
	addlw	low TableGLCDCHARCOL5
	movwf	SysStringA
	movlw	high TableGLCDCHARCOL5
	btfsc	STATUS, C
	addlw	1
	movwf	PCLATH
	movf	SysStringA, W
	movwf	PCL
TableGLCDCHARCOL5
	retlw	112
	retlw	124
	retlw	124
	retlw	0
	retlw	0
	retlw	238
	retlw	238
	retlw	124
	retlw	168
	retlw	254
	retlw	254
	retlw	84
	retlw	84
	retlw	148
	retlw	148
	retlw	124
	retlw	124
	retlw	0
	retlw	158
	retlw	0
	retlw	40
	retlw	254
	retlw	16
	retlw	170
	retlw	6
	retlw	68
	retlw	68
	retlw	124
	retlw	124
	retlw	96
	retlw	16
	retlw	192
	retlw	16
	retlw	146
	retlw	254
	retlw	162
	retlw	138
	retlw	36
	retlw	138
	retlw	146
	retlw	226
	retlw	146
	retlw	146
	retlw	108
	retlw	108
	retlw	68
	retlw	40
	retlw	68
	retlw	162
	retlw	242
	retlw	34
	retlw	146
	retlw	130
	retlw	130
	retlw	146
	retlw	18
	retlw	146
	retlw	16
	retlw	254
	retlw	130
	retlw	40
	retlw	128
	retlw	24
	retlw	16
	retlw	130
	retlw	18
	retlw	162
	retlw	50
	retlw	146
	retlw	254
	retlw	128
	retlw	128
	retlw	112
	retlw	16
	retlw	224
	retlw	146
	retlw	130
	retlw	16
	retlw	130
	retlw	2
	retlw	128
	retlw	4
	retlw	168
	retlw	136
	retlw	136
	retlw	136
	retlw	168
	retlw	18
	retlw	168
	retlw	8
	retlw	250
	retlw	136
	retlw	80
	retlw	254
	retlw	240
	retlw	8
	retlw	136
	retlw	40
	retlw	40
	retlw	8
	retlw	168
	retlw	144
	retlw	128
	retlw	128
	retlw	96
	retlw	32
	retlw	160
	retlw	168
	retlw	108
	retlw	254
	retlw	108
	retlw	16
	retlw	66

;********************************************************************************

GLCDCHARCOL6
	movlw	113
	subwf	SysStringA, W
	btfsc	STATUS, C
	retlw	0
	movf	SysStringA, W
	addlw	low TableGLCDCHARCOL6
	movwf	SysStringA
	movlw	high TableGLCDCHARCOL6
	btfsc	STATUS, C
	addlw	1
	movwf	PCLATH
	movf	SysStringA, W
	movwf	PCL
TableGLCDCHARCOL6
	retlw	112
	retlw	56
	retlw	254
	retlw	12
	retlw	10
	retlw	204
	retlw	102
	retlw	124
	retlw	32
	retlw	4
	retlw	64
	retlw	56
	retlw	16
	retlw	162
	retlw	136
	retlw	112
	retlw	28
	retlw	0
	retlw	0
	retlw	14
	retlw	254
	retlw	84
	retlw	200
	retlw	68
	retlw	0
	retlw	130
	retlw	56
	retlw	16
	retlw	16
	retlw	0
	retlw	16
	retlw	0
	retlw	8
	retlw	138
	retlw	128
	retlw	146
	retlw	150
	retlw	254
	retlw	138
	retlw	146
	retlw	18
	retlw	146
	retlw	82
	retlw	0
	retlw	0
	retlw	130
	retlw	40
	retlw	40
	retlw	18
	retlw	130
	retlw	36
	retlw	146
	retlw	130
	retlw	68
	retlw	146
	retlw	18
	retlw	146
	retlw	16
	retlw	130
	retlw	126
	retlw	68
	retlw	128
	retlw	4
	retlw	32
	retlw	130
	retlw	18
	retlw	66
	retlw	82
	retlw	146
	retlw	2
	retlw	128
	retlw	64
	retlw	128
	retlw	40
	retlw	16
	retlw	138
	retlw	130
	retlw	32
	retlw	254
	retlw	4
	retlw	128
	retlw	8
	retlw	168
	retlw	136
	retlw	136
	retlw	144
	retlw	168
	retlw	2
	retlw	168
	retlw	8
	retlw	128
	retlw	122
	retlw	136
	retlw	128
	retlw	8
	retlw	8
	retlw	136
	retlw	40
	retlw	48
	retlw	8
	retlw	168
	retlw	128
	retlw	64
	retlw	64
	retlw	128
	retlw	80
	retlw	160
	retlw	152
	retlw	130
	retlw	0
	retlw	16
	retlw	32
	retlw	68

;********************************************************************************

GLCDCHARCOL7
	movlw	113
	subwf	SysStringA, W
	btfsc	STATUS, C
	retlw	0
	movf	SysStringA, W
	addlw	low TableGLCDCHARCOL7
	movwf	SysStringA
	movlw	high TableGLCDCHARCOL7
	btfsc	STATUS, C
	addlw	1
	movwf	PCLATH
	movf	SysStringA, W
	movwf	PCL
TableGLCDCHARCOL7
	retlw	112
	retlw	16
	retlw	0
	retlw	10
	retlw	6
	retlw	136
	retlw	34
	retlw	56
	retlw	62
	retlw	8
	retlw	32
	retlw	16
	retlw	16
	retlw	128
	retlw	128
	retlw	64
	retlw	4
	retlw	0
	retlw	0
	retlw	0
	retlw	40
	retlw	36
	retlw	196
	retlw	160
	retlw	0
	retlw	0
	retlw	0
	retlw	40
	retlw	16
	retlw	0
	retlw	16
	retlw	0
	retlw	4
	retlw	124
	retlw	0
	retlw	140
	retlw	98
	retlw	32
	retlw	114
	retlw	96
	retlw	14
	retlw	108
	retlw	60
	retlw	0
	retlw	0
	retlw	0
	retlw	40
	retlw	16
	retlw	12
	retlw	124
	retlw	248
	retlw	108
	retlw	68
	retlw	56
	retlw	130
	retlw	2
	retlw	244
	retlw	254
	retlw	0
	retlw	2
	retlw	130
	retlw	128
	retlw	254
	retlw	254
	retlw	124
	retlw	12
	retlw	188
	retlw	140
	retlw	100
	retlw	2
	retlw	126
	retlw	62
	retlw	126
	retlw	198
	retlw	14
	retlw	134
	retlw	0
	retlw	64
	retlw	0
	retlw	8
	retlw	128
	retlw	0
	retlw	240
	retlw	112
	retlw	64
	retlw	254
	retlw	48
	retlw	4
	retlw	120
	retlw	240
	retlw	0
	retlw	0
	retlw	0
	retlw	0
	retlw	240
	retlw	240
	retlw	112
	retlw	16
	retlw	248
	retlw	16
	retlw	64
	retlw	64
	retlw	248
	retlw	56
	retlw	120
	retlw	136
	retlw	120
	retlw	136
	retlw	0
	retlw	0
	retlw	0
	retlw	16
	retlw	120

;********************************************************************************

GLCDDRAWCHAR
	movf	LINECOLOUR,W
	movwf	SysWORDTempA
	movf	LINECOLOUR_H,W
	movwf	SysWORDTempA_H
	movf	GLCDFOREGROUND,W
	movwf	SysWORDTempB
	movf	GLCDFOREGROUND_H,W
	movwf	SysWORDTempB_H
	pagesel	SysCompEqual16
	call	SysCompEqual16
	pagesel	$
	comf	SysByteTempX,F
	btfss	SysByteTempX,0
	goto	ENDIF6
	movlw	1
	movwf	GLCDBACKGROUND
	clrf	GLCDBACKGROUND_H
	clrf	GLCDFOREGROUND
	clrf	GLCDFOREGROUND_H
ENDIF6
	movlw	15
	subwf	CHARCODE,F
	movf	CHARCODE,W
	movwf	SysBYTETempA
	movlw	178
	movwf	SysBYTETempB
	pagesel	SysCompLessThan
	call	SysCompLessThan
	pagesel	$
	comf	SysByteTempX,F
	movf	SysByteTempX,W
	movwf	SysTemp1
	movf	CHARCODE,W
	movwf	SysBYTETempB
	movlw	202
	movwf	SysBYTETempA
	pagesel	SysCompLessThan
	call	SysCompLessThan
	pagesel	$
	comf	SysByteTempX,F
	movf	SysTemp1,W
	andwf	SysByteTempX,W
	banksel	SYSTEMP2
	movwf	SysTemp2
	btfss	SysTemp2,0
	goto	ENDIF7
	movlw	1
	banksel	CHARLOCY
	subwf	CHARLOCY,F
	movlw	0
	btfss	STATUS,C
	addlw	1
	subwf	CHARLOCY_H,F
ENDIF7
	movlw	1
	banksel	CHARCOL
	movwf	CHARCOL
	clrf	CHARCOL_H
	clrf	CURRCHARCOL
SysForLoop4
	incf	CURRCHARCOL,F
SysSelect1Case1
	decf	CURRCHARCOL,W
	btfss	STATUS, Z
	goto	SysSelect1Case2
	movf	CHARCODE,W
	movwf	SYSSTRINGA
	call	GLCDCHARCOL3
	movwf	CURRCHARVAL
	goto	SysSelectEnd1
SysSelect1Case2
	movlw	2
	subwf	CURRCHARCOL,W
	btfss	STATUS, Z
	goto	SysSelect1Case3
	movf	CHARCODE,W
	movwf	SYSSTRINGA
	call	GLCDCHARCOL4
	movwf	CURRCHARVAL
	goto	SysSelectEnd1
SysSelect1Case3
	movlw	3
	subwf	CURRCHARCOL,W
	btfss	STATUS, Z
	goto	SysSelect1Case4
	movf	CHARCODE,W
	movwf	SYSSTRINGA
	call	GLCDCHARCOL5
	movwf	CURRCHARVAL
	goto	SysSelectEnd1
SysSelect1Case4
	movlw	4
	subwf	CURRCHARCOL,W
	btfss	STATUS, Z
	goto	SysSelect1Case5
	movf	CHARCODE,W
	movwf	SYSSTRINGA
	call	GLCDCHARCOL6
	movwf	CURRCHARVAL
	goto	SysSelectEnd1
SysSelect1Case5
	movlw	5
	subwf	CURRCHARCOL,W
	btfss	STATUS, Z
	goto	SysSelect1Case6
	movf	CHARCODE,W
	movwf	SYSSTRINGA
	call	GLCDCHARCOL7
	movwf	CURRCHARVAL
SysSelect1Case6
SysSelectEnd1
	clrf	CHARROW
	clrf	CHARROW_H
	clrf	CURRCHARROW
SysForLoop5
	incf	CURRCHARROW,F
	clrf	CHARCOLS
	clrf	COL
	movlw	1
	subwf	GLCDFNTDEFAULTSIZE,W
	btfss	STATUS, C
	goto	SysForLoopEnd6
SysForLoop6
	incf	COL,F
	incf	CHARCOLS,F
	clrf	CHARROWS
	clrf	ROW
	movlw	1
	subwf	GLCDFNTDEFAULTSIZE,W
	btfss	STATUS, C
	goto	SysForLoopEnd7
SysForLoop7
	incf	ROW,F
	incf	CHARROWS,F
	btfss	CURRCHARVAL,0
	goto	ELSE10_1
	movf	CHARCOL,W
	addwf	CHARLOCX,W
	movwf	SysTemp1
	movf	CHARCOLS,W
	addwf	SysTemp1,W
	movwf	GLCDX
	movf	CHARROW,W
	addwf	CHARLOCY,W
	movwf	SysTemp1
	movf	CHARROWS,W
	addwf	SysTemp1,W
	movwf	GLCDY
	movf	LINECOLOUR,W
	movwf	GLCDCOLOUR
	movf	LINECOLOUR_H,W
	movwf	GLCDCOLOUR_H
	call	PSET_KS0108

	goto	ENDIF10
ELSE10_1
	movf	CHARCOL,W
	addwf	CHARLOCX,W
	movwf	SysTemp1
	movf	CHARCOLS,W
	addwf	SysTemp1,W
	movwf	GLCDX
	movf	CHARROW,W
	addwf	CHARLOCY,W
	movwf	SysTemp1
	movf	CHARROWS,W
	addwf	SysTemp1,W
	movwf	GLCDY
	movf	GLCDBACKGROUND,W
	movwf	GLCDCOLOUR
	movf	GLCDBACKGROUND_H,W
	movwf	GLCDCOLOUR_H
	call	PSET_KS0108

ENDIF10
	movf	GLCDFNTDEFAULTSIZE,W
	subwf	ROW,W
	btfss	STATUS, C
	goto	SysForLoop7
SysForLoopEnd7
	movf	GLCDFNTDEFAULTSIZE,W
	subwf	COL,W
	btfss	STATUS, C
	goto	SysForLoop6
SysForLoopEnd6
	rrf	CURRCHARVAL,F
	movf	GLCDFNTDEFAULTSIZE,W
	addwf	CHARROW,F
	movlw	0
	btfsc	STATUS,C
	addlw	1
	addwf	CHARROW_H,F
	movlw	8
	subwf	CURRCHARROW,W
	btfss	STATUS, C
	goto	SysForLoop5
SysForLoopEnd5
	movf	GLCDFNTDEFAULTSIZE,W
	addwf	CHARCOL,F
	movlw	0
	btfsc	STATUS,C
	addlw	1
	addwf	CHARCOL_H,F
	movlw	5
	subwf	CURRCHARCOL,W
	btfss	STATUS, C
	goto	SysForLoop4
SysForLoopEnd4
	clrf	GLCDBACKGROUND
	clrf	GLCDBACKGROUND_H
	movlw	1
	movwf	GLCDFOREGROUND
	clrf	GLCDFOREGROUND_H
	return

;********************************************************************************

;Overloaded signature: WORD:WORD:STRING:
GLCDPRINT3
	movf	SysLCDPRINTDATAHandler,W
	movwf	FSR
	bcf	STATUS, IRP
	btfsc	SysLCDPRINTDATAHandler_H,0
	bsf	STATUS, IRP
	movf	INDF,W
	movwf	PRINTLEN
	movf	PRINTLEN,F
	btfsc	STATUS, Z
	return
	movf	PRINTLOCX,W
	movwf	GLCDPRINTLOC
	movf	PRINTLOCX_H,W
	movwf	GLCDPRINTLOC_H
	clrf	SYSPRINTTEMP
	movlw	1
	subwf	PRINTLEN,W
	btfss	STATUS, C
	goto	SysForLoopEnd3
SysForLoop3
	incf	SYSPRINTTEMP,F
	movf	GLCDPRINTLOC,W
	movwf	CHARLOCX
	movf	GLCDPRINTLOC_H,W
	movwf	CHARLOCX_H
	movf	PRINTLOCY,W
	movwf	CHARLOCY
	movf	PRINTLOCY_H,W
	movwf	CHARLOCY_H
	movf	SYSPRINTTEMP,W
	addwf	SysLCDPRINTDATAHandler,W
	movwf	FSR
	bcf	STATUS, IRP
	btfsc	SysLCDPRINTDATAHandler_H,0
	bsf	STATUS, IRP
	movf	INDF,W
	movwf	CHARCODE
	movf	GLCDFOREGROUND,W
	movwf	LINECOLOUR
	movf	GLCDFOREGROUND_H,W
	movwf	LINECOLOUR_H
	call	GLCDDRAWCHAR

	movf	GLCDFONTWIDTH,W
	movwf	SysBYTETempA
	movf	GLCDFNTDEFAULTSIZE,W
	movwf	SysBYTETempB
	pagesel	SysMultSub
	call	SysMultSub
	pagesel	$
	movf	SysBYTETempX,W
	addwf	GLCDPRINTLOC,F
	movlw	0
	btfsc	STATUS,C
	addlw	1
	addwf	GLCDPRINTLOC_H,F
	movf	PRINTLEN,W
	subwf	SYSPRINTTEMP,W
	btfss	STATUS, C
	goto	SysForLoop3
SysForLoopEnd3
	return

;********************************************************************************

GLCDWRITEBYTE_KS0108
	bcf	SYSBITVAR0,0
	btfsc	PORTE,0
	bsf	SYSBITVAR0,0
	bcf	SYSBITVAR0,1
	btfsc	PORTC,1
	bsf	SYSBITVAR0,1
	btfsc	PORTC,0
	bcf	PORTC,1
	bcf	PORTE,0
SysWaitLoop1
	pagesel	FN_GLCDREADBYTE_KS0108
	call	FN_GLCDREADBYTE_KS0108
	pagesel	$
	btfsc	GLCDREADBYTE_KS0108,7
	goto	SysWaitLoop1
	bcf	PORTE,0
	btfsc	SYSBITVAR0,0
	bsf	PORTE,0
	bcf	PORTC,1
	btfsc	SYSBITVAR0,1
	bsf	PORTC,1
	bcf	PORTE,1
	banksel	TRISD
	bcf	TRISD,0
	bcf	TRISD,1
	bcf	TRISD,2
	bcf	TRISD,3
	bcf	TRISD,4
	bcf	TRISD,5
	bcf	TRISD,6
	bcf	TRISD,7
	banksel	PORTD
	bcf	PORTD,7
	btfsc	LCDBYTE,7
	bsf	PORTD,7
	bcf	PORTD,6
	btfsc	LCDBYTE,6
	bsf	PORTD,6
	bcf	PORTD,5
	btfsc	LCDBYTE,5
	bsf	PORTD,5
	bcf	PORTD,4
	btfsc	LCDBYTE,4
	bsf	PORTD,4
	bcf	PORTD,3
	btfsc	LCDBYTE,3
	bsf	PORTD,3
	bcf	PORTD,2
	btfsc	LCDBYTE,2
	bsf	PORTD,2
	bcf	PORTD,1
	btfsc	LCDBYTE,1
	bsf	PORTD,1
	bcf	PORTD,0
	btfsc	LCDBYTE,0
	bsf	PORTD,0
	goto	$+1
	goto	$+1
	bsf	PORTE,2
	goto	$+1
	goto	$+1
	bcf	PORTE,2
	goto	$+1
	goto	$+1
	return

;********************************************************************************

INITGLCD_KS0108
	banksel	TRISE
	bcf	TRISE,0
	bcf	TRISE,1
	bcf	TRISE,2
	bcf	TRISC,0
	bcf	TRISC,1
	bcf	TRISC,2
	banksel	PORTC
	bcf	PORTC,2
	movlw	1
	movwf	SysWaitTempMS
	clrf	SysWaitTempMS_H
	pagesel	Delay_MS
	call	Delay_MS
	pagesel	$
	bsf	PORTC,2
	movlw	1
	movwf	SysWaitTempMS
	clrf	SysWaitTempMS_H
	pagesel	Delay_MS
	call	Delay_MS
	pagesel	$
	bsf	PORTC,0
	bsf	PORTC,1
	bcf	PORTE,0
	movlw	63
	movwf	LCDBYTE
	call	GLCDWRITEBYTE_KS0108

	movlw	192
	movwf	LCDBYTE
	call	GLCDWRITEBYTE_KS0108

	bcf	PORTC,0
	bcf	PORTC,1
	clrf	GLCDBACKGROUND
	clrf	GLCDBACKGROUND_H
	movlw	1
	movwf	GLCDFOREGROUND
	clrf	GLCDFOREGROUND_H
	movlw	6
	movwf	GLCDFONTWIDTH
	clrf	GLCDFNTDEFAULT
	movlw	1
	movwf	GLCDFNTDEFAULTSIZE
	pagesel	GLCDCLS_KS0108
	call	GLCDCLS_KS0108
	pagesel	$

	return

;********************************************************************************

LINE
	clrf	LINEDIFFX
	clrf	LINEDIFFX_H
	clrf	LINEDIFFY
	clrf	LINEDIFFY_H
	clrf	LINESTEPX
	clrf	LINESTEPX_H
	clrf	LINESTEPY
	clrf	LINESTEPY_H
	clrf	LINEDIFFX_X2
	clrf	LINEDIFFX_X2_H
	clrf	LINEDIFFY_X2
	clrf	LINEDIFFY_X2_H
	clrf	LINEERR
	clrf	LINEERR_H
	movf	LINEX1,W
	subwf	LINEX2,W
	movwf	LINEDIFFX
	movf	LINEX1_H,W
	btfss	STATUS,C
	addlw	1
	subwf	LINEX2_H,W
	movwf	LINEDIFFX_H
	movf	LINEY1,W
	subwf	LINEY2,W
	movwf	LINEDIFFY
	movf	LINEY1_H,W
	btfss	STATUS,C
	addlw	1
	subwf	LINEY2_H,W
	movwf	LINEDIFFY_H
	movf	LINEDIFFX,W
	movwf	SysINTEGERTempB
	movf	LINEDIFFX_H,W
	movwf	SysINTEGERTempB_H
	clrf	SysINTEGERTempA
	clrf	SysINTEGERTempA_H
	pagesel	SysCompLessThanINT
	call	SysCompLessThanINT
	pagesel	$
	btfss	SysByteTempX,0
	goto	ELSE21_1
	movlw	1
	movwf	LINESTEPX
	clrf	LINESTEPX_H
	goto	ENDIF21
ELSE21_1
	movlw	255
	movwf	LINESTEPX
	movwf	LINESTEPX_H
ENDIF21
	movf	LINEDIFFY,W
	movwf	SysINTEGERTempB
	movf	LINEDIFFY_H,W
	movwf	SysINTEGERTempB_H
	clrf	SysINTEGERTempA
	clrf	SysINTEGERTempA_H
	pagesel	SysCompLessThanINT
	call	SysCompLessThanINT
	pagesel	$
	btfss	SysByteTempX,0
	goto	ELSE22_1
	movlw	1
	movwf	LINESTEPY
	clrf	LINESTEPY_H
	goto	ENDIF22
ELSE22_1
	movlw	255
	movwf	LINESTEPY
	movwf	LINESTEPY_H
ENDIF22
	movf	LINESTEPX,W
	movwf	SysINTEGERTempA
	movf	LINESTEPX_H,W
	movwf	SysINTEGERTempA_H
	movf	LINEDIFFX,W
	movwf	SysINTEGERTempB
	movf	LINEDIFFX_H,W
	movwf	SysINTEGERTempB_H
	pagesel	SysMultSubINT
	call	SysMultSubINT
	pagesel	$
	movf	SysINTEGERTempX,W
	movwf	LINEDIFFX
	movf	SysINTEGERTempX_H,W
	movwf	LINEDIFFX_H
	movf	LINESTEPY,W
	movwf	SysINTEGERTempA
	movf	LINESTEPY_H,W
	movwf	SysINTEGERTempA_H
	movf	LINEDIFFY,W
	movwf	SysINTEGERTempB
	movf	LINEDIFFY_H,W
	movwf	SysINTEGERTempB_H
	pagesel	SysMultSubINT
	call	SysMultSubINT
	pagesel	$
	movf	SysINTEGERTempX,W
	movwf	LINEDIFFY
	movf	SysINTEGERTempX_H,W
	movwf	LINEDIFFY_H
	movf	LINEDIFFX,W
	movwf	SysINTEGERTempA
	movf	LINEDIFFX_H,W
	movwf	SysINTEGERTempA_H
	movlw	2
	movwf	SysINTEGERTempB
	clrf	SysINTEGERTempB_H
	pagesel	SysMultSubINT
	call	SysMultSubINT
	pagesel	$
	movf	SysINTEGERTempX,W
	movwf	LINEDIFFX_X2
	movf	SysINTEGERTempX_H,W
	movwf	LINEDIFFX_X2_H
	movf	LINEDIFFY,W
	movwf	SysINTEGERTempA
	movf	LINEDIFFY_H,W
	movwf	SysINTEGERTempA_H
	movlw	2
	movwf	SysINTEGERTempB
	clrf	SysINTEGERTempB_H
	pagesel	SysMultSubINT
	call	SysMultSubINT
	pagesel	$
	movf	SysINTEGERTempX,W
	movwf	LINEDIFFY_X2
	movf	SysINTEGERTempX_H,W
	movwf	LINEDIFFY_X2_H
	movf	LINEDIFFX,W
	movwf	SysINTEGERTempA
	movf	LINEDIFFX_H,W
	movwf	SysINTEGERTempA_H
	movf	LINEDIFFY,W
	movwf	SysINTEGERTempB
	movf	LINEDIFFY_H,W
	movwf	SysINTEGERTempB_H
	pagesel	SysCompLessThanINT
	call	SysCompLessThanINT
	pagesel	$
	comf	SysByteTempX,F
	btfss	SysByteTempX,0
	goto	ELSE23_1
	movf	LINEDIFFX,W
	subwf	LINEDIFFY_X2,W
	movwf	LINEERR
	movf	LINEDIFFX_H,W
	btfss	STATUS,C
	addlw	1
	subwf	LINEDIFFY_X2_H,W
	movwf	LINEERR_H
SysDoLoop_S1
	movf	linex1,W
	movwf	SysWORDTempA
	movf	linex1_H,W
	movwf	SysWORDTempA_H
	movf	linex2,W
	movwf	SysWORDTempB
	movf	linex2_H,W
	movwf	SysWORDTempB_H
	pagesel	SysCompEqual16
	call	SysCompEqual16
	pagesel	$
	comf	SysByteTempX,F
	btfss	SysByteTempX,0
	goto	SysDoLoop_E1
	movf	LINEX1,W
	movwf	GLCDX
	movf	LINEY1,W
	movwf	GLCDY
	movf	LINECOLOUR,W
	movwf	GLCDCOLOUR
	movf	LINECOLOUR_H,W
	movwf	GLCDCOLOUR_H
	call	PSET_KS0108

	movf	LINESTEPX,W
	addwf	LINEX1,F
	movf	LINESTEPX_H,W
	btfsc	STATUS,C
	addlw	1
	addwf	LINEX1_H,F
	movf	LINEERR,W
	movwf	SysINTEGERTempA
	movf	LINEERR_H,W
	movwf	SysINTEGERTempA_H
	clrf	SysINTEGERTempB
	clrf	SysINTEGERTempB_H
	pagesel	SysCompLessThanINT
	call	SysCompLessThanINT
	pagesel	$
	btfss	SysByteTempX,0
	goto	ELSE24_1
	movf	LINEDIFFY_X2,W
	addwf	LINEERR,F
	movf	LINEDIFFY_X2_H,W
	btfsc	STATUS,C
	addlw	1
	addwf	LINEERR_H,F
	goto	ENDIF24
ELSE24_1
	movf	LINEDIFFX_X2,W
	subwf	LINEDIFFY_X2,W
	movwf	SysTemp1
	movf	LINEDIFFX_X2_H,W
	btfss	STATUS,C
	addlw	1
	subwf	LINEDIFFY_X2_H,W
	movwf	SysTemp1_H
	movf	SysTemp1,W
	addwf	LINEERR,F
	movf	SysTemp1_H,W
	btfsc	STATUS,C
	addlw	1
	addwf	LINEERR_H,F
	movf	LINESTEPY,W
	addwf	LINEY1,F
	movf	LINESTEPY_H,W
	btfsc	STATUS,C
	addlw	1
	addwf	LINEY1_H,F
ENDIF24
	goto	SysDoLoop_S1
SysDoLoop_E1
	movf	LINEX1,W
	movwf	GLCDX
	movf	LINEY1,W
	movwf	GLCDY
	movf	LINECOLOUR,W
	movwf	GLCDCOLOUR
	movf	LINECOLOUR_H,W
	movwf	GLCDCOLOUR_H
	call	PSET_KS0108

	goto	ENDIF23
ELSE23_1
	movf	LINEDIFFY,W
	subwf	LINEDIFFX_X2,W
	movwf	LINEERR
	movf	LINEDIFFY_H,W
	btfss	STATUS,C
	addlw	1
	subwf	LINEDIFFX_X2_H,W
	movwf	LINEERR_H
SysDoLoop_S2
	movf	liney1,W
	movwf	SysWORDTempA
	movf	liney1_H,W
	movwf	SysWORDTempA_H
	movf	liney2,W
	movwf	SysWORDTempB
	movf	liney2_H,W
	movwf	SysWORDTempB_H
	pagesel	SysCompEqual16
	call	SysCompEqual16
	pagesel	$
	comf	SysByteTempX,F
	btfss	SysByteTempX,0
	goto	SysDoLoop_E2
	movf	LINEX1,W
	movwf	GLCDX
	movf	LINEY1,W
	movwf	GLCDY
	movf	LINECOLOUR,W
	movwf	GLCDCOLOUR
	movf	LINECOLOUR_H,W
	movwf	GLCDCOLOUR_H
	call	PSET_KS0108

	movf	LINESTEPY,W
	addwf	LINEY1,F
	movf	LINESTEPY_H,W
	btfsc	STATUS,C
	addlw	1
	addwf	LINEY1_H,F
	movf	LINEERR,W
	movwf	SysINTEGERTempA
	movf	LINEERR_H,W
	movwf	SysINTEGERTempA_H
	clrf	SysINTEGERTempB
	clrf	SysINTEGERTempB_H
	pagesel	SysCompLessThanINT
	call	SysCompLessThanINT
	pagesel	$
	btfss	SysByteTempX,0
	goto	ELSE25_1
	movf	LINEDIFFX_X2,W
	addwf	LINEERR,F
	movf	LINEDIFFX_X2_H,W
	btfsc	STATUS,C
	addlw	1
	addwf	LINEERR_H,F
	goto	ENDIF25
ELSE25_1
	movf	LINEDIFFY_X2,W
	subwf	LINEDIFFX_X2,W
	movwf	SysTemp1
	movf	LINEDIFFY_X2_H,W
	btfss	STATUS,C
	addlw	1
	subwf	LINEDIFFX_X2_H,W
	movwf	SysTemp1_H
	movf	SysTemp1,W
	addwf	LINEERR,F
	movf	SysTemp1_H,W
	btfsc	STATUS,C
	addlw	1
	addwf	LINEERR_H,F
	movf	LINESTEPX,W
	addwf	LINEX1,F
	movf	LINESTEPX_H,W
	btfsc	STATUS,C
	addlw	1
	addwf	LINEX1_H,F
ENDIF25
	goto	SysDoLoop_S2
SysDoLoop_E2
	movf	LINEX1,W
	movwf	GLCDX
	movf	LINEY1,W
	movwf	GLCDY
	movf	LINECOLOUR,W
	movwf	GLCDCOLOUR
	movf	LINECOLOUR_H,W
	movwf	GLCDCOLOUR_H
	call	PSET_KS0108

ENDIF23
	return

;********************************************************************************

PSET_KS0108
	btfsc	GLCDX,6
	goto	ENDIF30
	bsf	PORTC,1
	bcf	PORTC,0
ENDIF30
	btfss	GLCDX,6
	goto	ENDIF31
	bsf	PORTC,0
	movlw	64
	subwf	GLCDX,F
	bcf	PORTC,1
ENDIF31
	movf	GLCDY,W
	movwf	SysBYTETempA
	movlw	8
	movwf	SysBYTETempB
	pagesel	SysDivSub
	call	SysDivSub
	pagesel	$
	movf	SysBYTETempA,W
	movwf	CURRPAGE
	bcf	PORTE,0
	movlw	184
	iorwf	CURRPAGE,W
	movwf	LCDBYTE
	call	GLCDWRITEBYTE_KS0108

	bcf	PORTE,0
	movlw	64
	iorwf	GLCDX,W
	movwf	LCDBYTE
	call	GLCDWRITEBYTE_KS0108

	bsf	PORTE,0
	pagesel	FN_GLCDREADBYTE_KS0108
	call	FN_GLCDREADBYTE_KS0108
	pagesel	$
	movf	GLCDREADBYTE_KS0108,W
	movwf	GLCDDATATEMP
	bsf	PORTE,0
	pagesel	FN_GLCDREADBYTE_KS0108
	call	FN_GLCDREADBYTE_KS0108
	pagesel	$
	movf	GLCDREADBYTE_KS0108,W
	movwf	GLCDDATATEMP
	movlw	7
	andwf	GLCDY,W
	movwf	GLCDBITNO
	btfsc	GLCDCOLOUR,0
	goto	ELSE32_1
	movlw	254
	movwf	GLCDCHANGE
	bsf	STATUS,C
	goto	ENDIF32
ELSE32_1
	movlw	1
	movwf	GLCDCHANGE
	bcf	STATUS,C
ENDIF32
	movf	GLCDBITNO,W
	movwf	SysRepeatTemp1
	btfsc	STATUS,Z
	goto	SysRepeatLoopEnd1
SysRepeatLoop1
	rlf	GLCDCHANGE,F
	decfsz	SysRepeatTemp1,F
	goto	SysRepeatLoop1
SysRepeatLoopEnd1
	btfsc	GLCDCOLOUR,0
	goto	ELSE33_1
	movf	GLCDDATATEMP,W
	andwf	GLCDCHANGE,W
	movwf	GLCDDATATEMP
	goto	ENDIF33
ELSE33_1
	movf	GLCDDATATEMP,W
	iorwf	GLCDCHANGE,W
	movwf	GLCDDATATEMP
ENDIF33
	bcf	PORTE,0
	movlw	64
	iorwf	GLCDX,W
	movwf	LCDBYTE
	call	GLCDWRITEBYTE_KS0108

	bsf	PORTE,0
	movf	GLCDDATATEMP,W
	movwf	LCDBYTE
	call	GLCDWRITEBYTE_KS0108

	bcf	PORTC,0
	bcf	PORTC,1
	return

;********************************************************************************

FN_STR
	clrf	SYSCHARCOUNT
	banksel	SYSVALTEMP
	movf	SYSVALTEMP,W
	movwf	SysWORDTempA
	movf	SYSVALTEMP_H,W
	movwf	SysWORDTempA_H
	movlw	16
	movwf	SysWORDTempB
	movlw	39
	movwf	SysWORDTempB_H
	banksel	STATUS
	call	SysCompLessThan16
	comf	SysByteTempX,F
	btfss	SysByteTempX,0
	goto	ENDIF61
	banksel	SYSVALTEMP
	movf	SYSVALTEMP,W
	movwf	SysWORDTempA
	movf	SYSVALTEMP_H,W
	movwf	SysWORDTempA_H
	movlw	16
	movwf	SysWORDTempB
	movlw	39
	movwf	SysWORDTempB_H
	banksel	STATUS
	call	SysDivSub16
	movf	SysWORDTempA,W
	movwf	SYSSTRDATA
	movf	SYSCALCTEMPX,W
	banksel	SYSVALTEMP
	movwf	SYSVALTEMP
	movf	SYSCALCTEMPX_H,W
	movwf	SYSVALTEMP_H
	banksel	SYSCHARCOUNT
	incf	SYSCHARCOUNT,F
	movlw	low(STR)
	addwf	SYSCHARCOUNT,W
	movwf	FSR
	bankisel	STR
	movlw	48
	addwf	SYSSTRDATA,W
	movwf	INDF
	goto	SYSVALTHOUSANDS
ENDIF61
	banksel	SYSVALTEMP
	movf	SYSVALTEMP,W
	movwf	SysWORDTempA
	movf	SYSVALTEMP_H,W
	movwf	SysWORDTempA_H
	movlw	232
	movwf	SysWORDTempB
	movlw	3
	movwf	SysWORDTempB_H
	banksel	STATUS
	call	SysCompLessThan16
	comf	SysByteTempX,F
	btfss	SysByteTempX,0
	goto	ENDIF62
SYSVALTHOUSANDS
	banksel	SYSVALTEMP
	movf	SYSVALTEMP,W
	movwf	SysWORDTempA
	movf	SYSVALTEMP_H,W
	movwf	SysWORDTempA_H
	movlw	232
	movwf	SysWORDTempB
	movlw	3
	movwf	SysWORDTempB_H
	banksel	STATUS
	call	SysDivSub16
	movf	SysWORDTempA,W
	movwf	SYSSTRDATA
	movf	SYSCALCTEMPX,W
	banksel	SYSVALTEMP
	movwf	SYSVALTEMP
	movf	SYSCALCTEMPX_H,W
	movwf	SYSVALTEMP_H
	banksel	SYSCHARCOUNT
	incf	SYSCHARCOUNT,F
	movlw	low(STR)
	addwf	SYSCHARCOUNT,W
	movwf	FSR
	bankisel	STR
	movlw	48
	addwf	SYSSTRDATA,W
	movwf	INDF
	goto	SYSVALHUNDREDS
ENDIF62
	banksel	SYSVALTEMP
	movf	SYSVALTEMP,W
	movwf	SysWORDTempA
	movf	SYSVALTEMP_H,W
	movwf	SysWORDTempA_H
	movlw	100
	movwf	SysWORDTempB
	clrf	SysWORDTempB_H
	banksel	STATUS
	call	SysCompLessThan16
	comf	SysByteTempX,F
	btfss	SysByteTempX,0
	goto	ENDIF63
SYSVALHUNDREDS
	banksel	SYSVALTEMP
	movf	SYSVALTEMP,W
	movwf	SysWORDTempA
	movf	SYSVALTEMP_H,W
	movwf	SysWORDTempA_H
	movlw	100
	movwf	SysWORDTempB
	clrf	SysWORDTempB_H
	banksel	STATUS
	call	SysDivSub16
	movf	SysWORDTempA,W
	movwf	SYSSTRDATA
	movf	SYSCALCTEMPX,W
	banksel	SYSVALTEMP
	movwf	SYSVALTEMP
	movf	SYSCALCTEMPX_H,W
	movwf	SYSVALTEMP_H
	banksel	SYSCHARCOUNT
	incf	SYSCHARCOUNT,F
	movlw	low(STR)
	addwf	SYSCHARCOUNT,W
	movwf	FSR
	bankisel	STR
	movlw	48
	addwf	SYSSTRDATA,W
	movwf	INDF
	goto	SYSVALTENS
ENDIF63
	banksel	SYSVALTEMP
	movf	SYSVALTEMP,W
	movwf	SysWORDTempA
	movf	SYSVALTEMP_H,W
	movwf	SysWORDTempA_H
	movlw	10
	movwf	SysWORDTempB
	clrf	SysWORDTempB_H
	banksel	STATUS
	call	SysCompLessThan16
	comf	SysByteTempX,F
	btfss	SysByteTempX,0
	goto	ENDIF64
SYSVALTENS
	banksel	SYSVALTEMP
	movf	SYSVALTEMP,W
	movwf	SysWORDTempA
	movf	SYSVALTEMP_H,W
	movwf	SysWORDTempA_H
	movlw	10
	movwf	SysWORDTempB
	clrf	SysWORDTempB_H
	banksel	STATUS
	call	SysDivSub16
	movf	SysWORDTempA,W
	movwf	SYSSTRDATA
	movf	SYSCALCTEMPX,W
	banksel	SYSVALTEMP
	movwf	SYSVALTEMP
	movf	SYSCALCTEMPX_H,W
	movwf	SYSVALTEMP_H
	banksel	SYSCHARCOUNT
	incf	SYSCHARCOUNT,F
	movlw	low(STR)
	addwf	SYSCHARCOUNT,W
	movwf	FSR
	bankisel	STR
	movlw	48
	addwf	SYSSTRDATA,W
	movwf	INDF
ENDIF64
	incf	SYSCHARCOUNT,F
	movlw	low(STR)
	addwf	SYSCHARCOUNT,W
	movwf	FSR
	bankisel	STR
	movlw	48
	banksel	SYSVALTEMP
	addwf	SYSVALTEMP,W
	movwf	INDF
	movf	SYSCALCTEMPX,W
	movwf	SYSVALTEMP
	movf	SYSCALCTEMPX_H,W
	movwf	SYSVALTEMP_H
	banksel	SYSCHARCOUNT
	movf	SYSCHARCOUNT,W
	banksel	SYSSTR_0
	movwf	SYSSTR_0
	banksel	STATUS
	return

;********************************************************************************

SYSCOMPLESSTHAN16
	clrf	SYSBYTETEMPX
	movf	SYSWORDTEMPA_H,W
	subwf	SYSWORDTEMPB_H,W
	btfss	STATUS,C
	return
	movf	SYSWORDTEMPB_H,W
	subwf	SYSWORDTEMPA_H,W
	btfss	STATUS,C
	goto	SCLT16TRUE
	movf	SYSWORDTEMPB,W
	subwf	SYSWORDTEMPA,W
	btfsc	STATUS,C
	return
SCLT16TRUE
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
	pagesel	SysCompEqual16
	call	SysCompEqual16
	pagesel	$
	btfss	SysByteTempX,0
	goto	ENDIF67
	clrf	SYSWORDTEMPA
	clrf	SYSWORDTEMPA_H
	return
ENDIF67
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
	goto	ENDIF68
	bcf	SYSDIVMULTA,0
	movf	SYSDIVMULTB,W
	addwf	SYSDIVMULTX,F
	movf	SYSDIVMULTB_H,W
	btfsc	STATUS,C
	addlw	1
	addwf	SYSDIVMULTX_H,F
ENDIF68
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

;Start of program memory page 1
	ORG	2048
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

Delay_S
DS_START
	movlw	232
	movwf	SysWaitTempMS
	movlw	3
	movwf	SysWaitTempMS_H
	call	Delay_MS

	decfsz	SysWaitTempS, F
	goto	DS_START
	return

;********************************************************************************

GLCDCLS_KS0108
	clrf	GLCD_YORDINATE
	clrf	GLCD_YORDINATE_H
	bsf	PORTC,0
	bcf	PORTC,1
	clrf	GLCD_COUNT
SysForLoop10
	incf	GLCD_COUNT,F
	movlw	255
	movwf	CURRPAGE
SysForLoop11
	incf	CURRPAGE,F
	bcf	PORTE,0
	movlw	184
	iorwf	CURRPAGE,W
	movwf	LCDBYTE
	pagesel	GLCDWRITEBYTE_KS0108
	call	GLCDWRITEBYTE_KS0108
	pagesel	$

	movlw	255
	movwf	CURRCOL
SysForLoop12
	incf	CURRCOL,F
	bcf	PORTE,0
	movlw	64
	iorwf	CURRCOL,W
	movwf	LCDBYTE
	pagesel	GLCDWRITEBYTE_KS0108
	call	GLCDWRITEBYTE_KS0108
	pagesel	$

	bsf	PORTE,0
	clrf	LCDBYTE
	pagesel	GLCDWRITEBYTE_KS0108
	call	GLCDWRITEBYTE_KS0108
	pagesel	$

	movlw	63
	subwf	CURRCOL,W
	btfss	STATUS, C
	goto	SysForLoop12
SysForLoopEnd12
	movlw	7
	subwf	CURRPAGE,W
	btfss	STATUS, C
	goto	SysForLoop11
SysForLoopEnd11
	bcf	PORTC,0
	bsf	PORTC,1
	movlw	2
	subwf	GLCD_COUNT,W
	btfss	STATUS, C
	goto	SysForLoop10
SysForLoopEnd10
	bcf	PORTC,0
	bcf	PORTC,1
	return

;********************************************************************************

FN_GLCDREADBYTE_KS0108
	banksel	TRISD
	bsf	TRISD,7
	bsf	TRISD,6
	bsf	TRISD,5
	bsf	TRISD,4
	bsf	TRISD,3
	bsf	TRISD,2
	bsf	TRISD,1
	bsf	TRISD,0
	banksel	PORTE
	bsf	PORTE,1
	bsf	PORTE,2
	movlw	2
	movwf	DELAYTEMP
DelayUS1
	decfsz	DELAYTEMP,F
	goto	DelayUS1
	nop
	bcf	GLCDREADBYTE_KS0108,7
	btfsc	PORTD,7
	bsf	GLCDREADBYTE_KS0108,7
	bcf	GLCDREADBYTE_KS0108,6
	btfsc	PORTD,6
	bsf	GLCDREADBYTE_KS0108,6
	bcf	GLCDREADBYTE_KS0108,5
	btfsc	PORTD,5
	bsf	GLCDREADBYTE_KS0108,5
	bcf	GLCDREADBYTE_KS0108,4
	btfsc	PORTD,4
	bsf	GLCDREADBYTE_KS0108,4
	bcf	GLCDREADBYTE_KS0108,3
	btfsc	PORTD,3
	bsf	GLCDREADBYTE_KS0108,3
	bcf	GLCDREADBYTE_KS0108,2
	btfsc	PORTD,2
	bsf	GLCDREADBYTE_KS0108,2
	bcf	GLCDREADBYTE_KS0108,1
	btfsc	PORTD,1
	bsf	GLCDREADBYTE_KS0108,1
	bcf	GLCDREADBYTE_KS0108,0
	btfsc	PORTD,0
	bsf	GLCDREADBYTE_KS0108,0
	bcf	PORTE,2
	movlw	2
	movwf	DELAYTEMP
DelayUS2
	decfsz	DELAYTEMP,F
	goto	DelayUS2
	nop
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
	clrf	PORTD
	clrf	PORTE
	return

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

SYSCOMPLESSTHAN
	clrf	SYSBYTETEMPX
	bsf	STATUS, C
	movf	SYSBYTETEMPB, W
	subwf	SYSBYTETEMPA, W
	btfss	STATUS, C
	comf	SYSBYTETEMPX,F
	return

;********************************************************************************

SYSCOMPLESSTHANINT
	clrf	SYSBYTETEMPX
	btfss	SYSINTEGERTEMPA_H,7
	goto	ELSE58_1
	btfsc	SYSINTEGERTEMPB_H,7
	goto	ENDIF59
	comf	SYSBYTETEMPX,F
	return
ENDIF59
	comf	SYSINTEGERTEMPA,W
	movwf	SYSDIVMULTA
	comf	SYSINTEGERTEMPA_H,W
	movwf	SYSDIVMULTA_H
	incf	SYSDIVMULTA,F
	btfsc	STATUS,Z
	incf	SYSDIVMULTA_H,F
	comf	SYSINTEGERTEMPB,W
	movwf	SYSINTEGERTEMPA
	comf	SYSINTEGERTEMPB_H,W
	movwf	SYSINTEGERTEMPA_H
	incf	SYSINTEGERTEMPA,F
	btfsc	STATUS,Z
	incf	SYSINTEGERTEMPA_H,F
	movf	SYSDIVMULTA,W
	movwf	SYSINTEGERTEMPB
	movf	SYSDIVMULTA_H,W
	movwf	SYSINTEGERTEMPB_H
	goto	ENDIF58
ELSE58_1
	btfsc	SYSINTEGERTEMPB_H,7
	return
ENDIF58
	movf	SYSINTEGERTEMPA_H,W
	subwf	SYSINTEGERTEMPB_H,W
	btfss	STATUS,C
	return
	movf	SYSINTEGERTEMPB_H,W
	subwf	SYSINTEGERTEMPA_H,W
	btfss	STATUS,C
	goto	SCLTINTTRUE
	movf	SYSINTEGERTEMPB,W
	subwf	SYSINTEGERTEMPA,W
	btfsc	STATUS,C
	return
SCLTINTTRUE
	comf	SYSBYTETEMPX,F
	return

;********************************************************************************

SYSDIVSUB
	movf	SYSBYTETEMPB, F
	btfsc	STATUS, Z
	return
	clrf	SYSBYTETEMPX
	movlw	8
	movwf	SYSDIVLOOP
SYSDIV8START
	bcf	STATUS, C
	rlf	SYSBYTETEMPA, F
	rlf	SYSBYTETEMPX, F
	movf	SYSBYTETEMPB, W
	subwf	SYSBYTETEMPX, F
	bsf	SYSBYTETEMPA, 0
	btfsc	STATUS, C
	goto	DIV8NOTNEG
	bcf	SYSBYTETEMPA, 0
	movf	SYSBYTETEMPB, W
	addwf	SYSBYTETEMPX, F
DIV8NOTNEG
	decfsz	SYSDIVLOOP, F
	goto	SYSDIV8START
	return

;********************************************************************************

SYSMULTSUB
	clrf	SYSBYTETEMPX
MUL8LOOP
	movf	SYSBYTETEMPA, W
	btfsc	SYSBYTETEMPB, 0
	addwf	SYSBYTETEMPX, F
	bcf	STATUS, C
	rrf	SYSBYTETEMPB, F
	bcf	STATUS, C
	rlf	SYSBYTETEMPA, F
	movf	SYSBYTETEMPB, F
	btfss	STATUS, Z
	goto	MUL8LOOP
	return

;********************************************************************************

SYSMULTSUB16
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
MUL16LOOP
	btfss	SYSDIVMULTB,0
	goto	ENDIF65
	movf	SYSDIVMULTA,W
	addwf	SYSDIVMULTX,F
	movf	SYSDIVMULTA_H,W
	btfsc	STATUS,C
	addlw	1
	addwf	SYSDIVMULTX_H,F
ENDIF65
	bcf	STATUS,C
	rrf	SYSDIVMULTB_H,F
	rrf	SYSDIVMULTB,F
	bcf	STATUS,C
	rlf	SYSDIVMULTA,F
	rlf	SYSDIVMULTA_H,F
	movf	SYSDIVMULTB,W
	movwf	SysWORDTempB
	movf	SYSDIVMULTB_H,W
	movwf	SysWORDTempB_H
	clrf	SysWORDTempA
	clrf	SysWORDTempA_H
	pagesel	SysCompLessThan16
	call	SysCompLessThan16
	pagesel	$
	btfsc	SysByteTempX,0
	goto	MUL16LOOP
	movf	SYSDIVMULTX,W
	movwf	SYSWORDTEMPX
	movf	SYSDIVMULTX_H,W
	movwf	SYSWORDTEMPX_H
	return

;********************************************************************************

SYSMULTSUBINT
	movf	SYSINTEGERTEMPA_H,W
	xorwf	SYSINTEGERTEMPB_H,W
	movwf	SYSSIGNBYTE
	btfss	SYSINTEGERTEMPA_H,7
	goto	ENDIF55
	comf	SYSINTEGERTEMPA,F
	comf	SYSINTEGERTEMPA_H,F
	incf	SYSINTEGERTEMPA,F
	btfsc	STATUS,Z
	incf	SYSINTEGERTEMPA_H,F
ENDIF55
	btfss	SYSINTEGERTEMPB_H,7
	goto	ENDIF56
	comf	SYSINTEGERTEMPB,F
	comf	SYSINTEGERTEMPB_H,F
	incf	SYSINTEGERTEMPB,F
	btfsc	STATUS,Z
	incf	SYSINTEGERTEMPB_H,F
ENDIF56
	call	SYSMULTSUB16

	btfss	SYSSIGNBYTE,7
	goto	ENDIF57
	comf	SYSINTEGERTEMPX,F
	comf	SYSINTEGERTEMPX_H,F
	incf	SYSINTEGERTEMPX,F
	btfsc	STATUS,Z
	incf	SYSINTEGERTEMPX_H,F
ENDIF57
	return

;********************************************************************************

SYSREADSTRING
	movf	SYSSTRINGB, W
	movwf	FSR
	bcf	STATUS, IRP
	btfsc	SYSSTRINGB_H, 0
	bsf	STATUS, IRP
	call	SYSSTRINGTABLES
	movwf	SYSCALCTEMPA
	movwf	INDF
	addwf	SYSSTRINGB, F
	goto	SYSSTRINGREADCHECK
SYSREADSTRINGPART
	movf	SYSSTRINGB, W
	movwf	FSR
	bcf	STATUS, IRP
	btfsc	SYSSTRINGB_H, 0
	bsf	STATUS, IRP
	call	SYSSTRINGTABLES
	movwf	SYSCALCTEMPA
	addwf	SYSSTRINGLENGTH,F
	addwf	SYSSTRINGB,F
SYSSTRINGREADCHECK
	movf	SYSCALCTEMPA,F
	btfsc	STATUS,Z
	return
SYSSTRINGREAD
	call	SYSSTRINGTABLES
	incf	FSR, F
	movwf	INDF
	decfsz	SYSCALCTEMPA, F
	goto	SYSSTRINGREAD
	return

;********************************************************************************

SysStringTables
	movf	SysStringA_H,W
	movwf	PCLATH
	movf	SysStringA,W
	incf	SysStringA,F
	btfsc	STATUS,Z
	incf	SysStringA_H,F
	movwf	PCL

StringTable1
	retlw	5
	retlw	72	;H
	retlw	101	;e
	retlw	108	;l
	retlw	108	;l
	retlw	111	;o


StringTable2
	retlw	8
	retlw	65	;A
	retlw	83	;S
	retlw	67	;C
	retlw	73	;I
	retlw	73	;I
	retlw	32	; 
	retlw	35	;#
	retlw	58	;:


;********************************************************************************

;Start of program memory page 2
	ORG	4096
;Start of program memory page 3
	ORG	6144

 END
