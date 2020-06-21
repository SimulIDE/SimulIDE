;Program compiled by Great Cow BASIC (0.94 2015-08-05)
;Need help? See the GCBASIC forums at http://sourceforge.net/projects/gcbasic/forums,
;check the documentation or email w_cholmondeley at users dot sourceforge dot net.

;********************************************************************************

;Chip Model: MEGA328P
;Assembler header file
.INCLUDE "m328pdef.inc"

;SREG bit names (for AVR Assembler compatibility, GCBASIC uses different names)
#define I 7
#define T 6
#define H 5
#define S 4
#define V 3
#define N 2
#define Z 1
#define C 0

;********************************************************************************

;Set aside memory locations for variables
.EQU	ADREADPORT=256
.EQU	COUNT=257
.EQU	READAD=258

;********************************************************************************

;Register variables
.DEF	DELAYTEMP=r25
.DEF	DELAYTEMP2=r26
.DEF	SysBitTest=r5
.DEF	SysCalcTempA=r22
.DEF	SysCalcTempB=r28
.DEF	SysValueCopy=r21
.DEF	SysWaitTemp10US=r27
.DEF	SysWaitTempMS=r29
.DEF	SysWaitTempMS_H=r30

;********************************************************************************

;Vectors
;Interrupt vectors
.ORG	0
	rjmp	BASPROGRAMSTART ;Reset
.ORG	2
	reti	;INT0
.ORG	4
	reti	;INT1
.ORG	6
	reti	;PCINT0
.ORG	8
	reti	;PCINT1
.ORG	10
	reti	;PCINT2
.ORG	12
	reti	;WDT
.ORG	14
	reti	;TIMER2_COMPA
.ORG	16
	reti	;TIMER2_COMPB
.ORG	18
	reti	;TIMER2_OVF
.ORG	20
	reti	;TIMER1_CAPT
.ORG	22
	reti	;TIMER1_COMPA
.ORG	24
	reti	;TIMER1_COMPB
.ORG	26
	reti	;TIMER1_OVF
.ORG	28
	reti	;TIMER0_COMPA
.ORG	30
	reti	;TIMER0_COMPB
.ORG	32
	reti	;TIMER0_OVF
.ORG	34
	reti	;SPI_STC
.ORG	36
	reti	;USART_RX
.ORG	38
	reti	;USART_UDRE
.ORG	40
	reti	;USART_TX
.ORG	42
	reti	;ADC
.ORG	44
	reti	;EE_READY
.ORG	46
	reti	;ANALOG_COMP
.ORG	48
	reti	;TWI
.ORG	50
	reti	;SPM_READY

;********************************************************************************

;Start of program memory page 0
.ORG	52
BASPROGRAMSTART:
;Initialise stack
	ldi	SysValueCopy,high(RAMEND)
	out	SPH, SysValueCopy
	ldi	SysValueCopy,low(RAMEND)
	out	SPL, SysValueCopy
;Call initialisation routines
	rcall	INITSYS
;Automatic pin direction setting
	sbi	DDRB,0

;Start of the main program
MAIN:
;Source:F1L30S0I30
	ldi	SysValueCopy,0
	sts	ADREADPORT,SysValueCopy
	rcall	FN_READAD
	lds	SysValueCopy,READAD
	sts	COUNT,SysValueCopy
;Source:F1L32S0I32
	lds	SysCalcTempA,COUNT
	cpi	SysCalcTempA,75
	brsh	ENDIF1
;Source:F1L33S0I33
	ldi	SysValueCopy,75
	sts	COUNT,SysValueCopy
;Source:F1L34S0I34
ENDIF1:
;Source:F1L36S0I36
	ldi	SysCalcTempA,225
	lds	SysCalcTempB,COUNT
	cp	SysCalcTempA,SysCalcTempB
	brsh	ENDIF2
;Source:F1L37S0I37
	ldi	SysValueCopy,225
	sts	COUNT,SysValueCopy
;Source:F1L38S0I38
ENDIF2:
;Source:F1L39S0I39
;Source:F7L158S0I1
	sbi	PORTB,0
;Source:F7L159S0I2
	lds	SysWaitTemp10US,COUNT
	rcall	Delay_10US
;Source:F7L160S0I3
	cbi	PORTB,0
;Source:F1L40S0I40
	ldi	SysWaitTempMS,20
	ldi	SysWaitTempMS_H,0
	rcall	Delay_MS
;Source:F1L42S0I42
	rjmp	MAIN
;Source:F2L34S0I34
;Source:F2L35S0I35
;Source:F2L38S0I38
;Source:F2L44S0I44
;Source:F2L47S0I47
;Source:F2L49S0I49
;Source:F2L50S0I50
;Source:F2L51S0I51
;Source:F2L52S0I52
;Source:F2L55S0I55
;Source:F2L56S0I56
;Source:F2L57S0I57
;Source:F2L58S0I58
;Source:F2L62S0I62
;Source:F2L63S0I63
;Source:F2L64S0I64
;Source:F2L65S0I65
;Source:F2L66S0I66
;Source:F2L67S0I67
;Source:F2L68S0I68
;Source:F2L69S0I69
;Source:F2L70S0I70
;Source:F2L71S0I71
;Source:F2L72S0I72
;Source:F2L73S0I73
;Source:F2L74S0I74
;Source:F2L75S0I75
;Source:F2L76S0I76
;Source:F2L77S0I77
;Source:F2L78S0I78
;Source:F2L79S0I79
;Source:F2L80S0I80
;Source:F2L81S0I81
;Source:F2L82S0I82
;Source:F2L83S0I83
;Source:F2L84S0I84
;Source:F2L85S0I85
;Source:F2L86S0I86
;Source:F2L87S0I87
;Source:F2L88S0I88
;Source:F2L89S0I89
;Source:F2L92S0I92
;Source:F2L93S0I93
;Source:F2L94S0I94
;Source:F2L95S0I95
;Source:F2L96S0I96
;Source:F2L97S0I97
;Source:F2L98S0I98
;Source:F2L99S0I99
;Source:F2L100S0I100
;Source:F2L101S0I101
;Source:F2L102S0I102
;Source:F2L103S0I103
;Source:F2L104S0I104
;Source:F2L105S0I105
;Source:F3L31S0I31
;Source:F3L32S0I32
;Source:F4L55S0I55
;Source:F4L56S0I56
;Source:F4L57S0I57
;Source:F4L60S0I60
;Source:F4L61S0I61
;Source:F4L64S0I64
;Source:F4L114S0I114
;Source:F5L122S0I83
;Source:F6L25S0I25
;Source:F6L26S0I26
;Source:F6L54S0I23
;Source:F7L35S0I35
;Source:F7L36S0I36
;Source:F7L37S0I37
;Source:F7L38S0I38
;Source:F7L39S0I39
;Source:F7L40S0I40
;Source:F7L41S0I41
;Source:F7L43S0I43
;Source:F7L46S0I46
;Source:F7L47S0I47
;Source:F7L48S0I48
;Source:F9L111S0I111
;Source:F9L114S0I114
;Source:F9L115S0I115
;Source:F9L117S0I117
;Source:F9L119S0I119
;Source:F9L120S0I120
;Source:F9L122S0I122
;Source:F9L124S0I124
;Source:F9L125S0I125
;Source:F9L126S0I126
;Source:F9L127S0I127
;Source:F9L129S0I129
;Source:F9L130S0I130
;Source:F9L131S0I131
;Source:F9L133S0I133
;Source:F9L137S0I137
;Source:F9L138S0I138
;Source:F9L139S0I139
;Source:F9L140S0I140
;Source:F9L143S0I143
;Source:F9L144S0I144
;Source:F9L146S0I146
;Source:F9L147S0I147
;Source:F9L148S0I148
;Source:F9L150S0I150
;Source:F9L151S0I151
;Source:F9L153S0I153
;Source:F9L154S0I154
;Source:F9L157S0I157
;Source:F9L158S0I158
;Source:F9L161S0I161
;Source:F9L165S0I165
;Source:F9L166S0I166
;Source:F9L285S0I9
;Source:F9L286S0I10
;Source:F10L34S0I34
;Source:F10L35S0I35
;Source:F10L36S0I36
;Source:F10L37S0I37
;Source:F10L38S0I38
;Source:F10L39S0I39
;Source:F10L40S0I40
;Source:F10L41S0I41
;Source:F11L123S0I123
;Source:F11L124S0I124
;Source:F11L125S0I125
;Source:F11L131S0I131
;Source:F11L132S0I132
;Source:F11L133S0I133
;Source:F11L134S0I134
;Source:F11L135S0I135
;Source:F11L136S0I136
;Source:F11L138S0I138
;Source:F11L139S0I139
;Source:F11L140S0I140
;Source:F11L141S0I141
;Source:F11L142S0I142
;Source:F11L143S0I143
;Source:F11L145S0I145
;Source:F11L146S0I146
;Source:F11L147S0I147
;Source:F11L148S0I148
;Source:F11L149S0I149
;Source:F11L150S0I150
;Source:F11L152S0I152
;Source:F11L153S0I153
;Source:F11L154S0I154
;Source:F11L155S0I155
;Source:F11L156S0I156
;Source:F11L157S0I157
;Source:F11L159S0I159
;Source:F11L160S0I160
;Source:F11L161S0I161
;Source:F11L162S0I162
;Source:F11L163S0I163
;Source:F11L164S0I164
;Source:F11L166S0I166
;Source:F11L167S0I167
;Source:F11L168S0I168
;Source:F11L169S0I169
;Source:F11L170S0I170
;Source:F11L171S0I171
;Source:F11L200S0I200
;Source:F11L201S0I201
;Source:F11L203S0I203
;Source:F11L204S0I204
;Source:F11L206S0I206
;Source:F11L207S0I207
;Source:F11L209S0I209
;Source:F11L210S0I210
;Source:F11L212S0I212
;Source:F11L213S0I213
;Source:F11L215S0I215
;Source:F11L216S0I216
;Source:F11L231S0I231
;Source:F11L232S0I232
;Source:F11L233S0I233
;Source:F11L234S0I234
;Source:F11L235S0I235
;Source:F11L236S0I236
;Source:F11L237S0I237
;Source:F11L238S0I238
;Source:F11L239S0I239
;Source:F11L240S0I240
;Source:F11L241S0I241
;Source:F11L242S0I242
;Source:F11L243S0I243
;Source:F11L244S0I244
;Source:F11L245S0I245
;Source:F11L246S0I246
;Source:F11L249S0I249
;Source:F11L250S0I250
;Source:F11L251S0I251
;Source:F11L252S0I252
;Source:F11L253S0I253
;Source:F11L254S0I254
;Source:F11L255S0I255
;Source:F11L256S0I256
;Source:F11L259S0I259
;Source:F11L260S0I260
;Source:F11L261S0I261
;Source:F11L262S0I262
;Source:F11L263S0I263
;Source:F11L264S0I264
;Source:F11L265S0I265
;Source:F11L266S0I266
;Source:F11L269S0I269
;Source:F11L270S0I270
;Source:F11L271S0I271
;Source:F11L272S0I272
;Source:F11L273S0I273
;Source:F11L274S0I274
;Source:F11L275S0I275
;Source:F11L276S0I276
;Source:F11L279S0I279
;Source:F11L280S0I280
;Source:F11L281S0I281
;Source:F11L282S0I282
;Source:F11L283S0I283
;Source:F11L284S0I284
;Source:F11L285S0I285
;Source:F11L286S0I286
;Source:F11L289S0I289
;Source:F11L290S0I290
;Source:F11L291S0I291
;Source:F11L292S0I292
;Source:F11L293S0I293
;Source:F11L294S0I294
;Source:F11L295S0I295
;Source:F11L296S0I296
;Source:F11L299S0I299
;Source:F11L300S0I300
;Source:F11L301S0I301
;Source:F11L302S0I302
;Source:F11L303S0I303
;Source:F11L304S0I304
;Source:F11L305S0I305
;Source:F11L306S0I306
;Source:F11L309S0I309
;Source:F11L310S0I310
;Source:F11L311S0I311
;Source:F11L312S0I312
;Source:F11L313S0I313
;Source:F11L314S0I314
;Source:F11L315S0I315
;Source:F11L316S0I316
;Source:F11L319S0I319
;Source:F11L320S0I320
;Source:F11L321S0I321
;Source:F11L322S0I322
;Source:F11L323S0I323
;Source:F11L324S0I324
;Source:F11L325S0I325
;Source:F11L326S0I326
;Source:F11L329S0I329
;Source:F11L330S0I330
;Source:F11L331S0I331
;Source:F11L332S0I332
;Source:F11L333S0I333
;Source:F11L334S0I334
;Source:F11L335S0I335
;Source:F11L336S0I336
;Source:F11L339S0I339
;Source:F11L340S0I340
;Source:F11L341S0I341
;Source:F11L342S0I342
;Source:F11L343S0I343
;Source:F11L344S0I344
;Source:F11L345S0I345
;Source:F11L346S0I346
;Source:F12L26S0I26
;Source:F12L27S0I27
;Source:F12L28S0I28
;Source:F12L29S0I29
;Source:F12L32S0I32
;Source:F12L33S0I33
;Source:F12L34S0I34
;Source:F12L35S0I35
;Source:F12L36S0I36
;Source:F12L39S0I39
;Source:F13L41S0I41
;Source:F13L42S0I42
;Source:F13L43S0I43
;Source:F13L44S0I44
;Source:F13L45S0I45
;Source:F13L49S0I49
;Source:F13L50S0I50
;Source:F13L51S0I51
;Source:F13L52S0I52
;Source:F13L253S0I10
;Source:F14L57S0I57
;Source:F14L64S0I64
;Source:F14L66S0I66
;Source:F14L67S0I67
;Source:F14L68S0I68
;Source:F14L69S0I69
;Source:F14L70S0I70
;Source:F14L71S0I71
;Source:F14L72S0I72
;Source:F15L22S0I22
;Source:F15L25S0I25
;Source:F17L54S0I54
;Source:F17L57S0I57
;Source:F18L130S0I130
;Source:F18L131S0I131
;Source:F18L132S0I132
;Source:F18L133S0I133
;Source:F18L137S0I137
;Source:F18L138S0I138
;Source:F18L139S0I139
;Source:F18L140S0I140
;Source:F18L142S0I142
;Source:F18L143S0I143
;Source:F18L144S0I144
;Source:F18L151S0I151
;Source:F19L78S0I78
;Source:F19L81S0I81
;Source:F19L82S0I82
;Source:F19L84S0I84
;Source:F19L85S0I85
;Source:F19L86S0I86
;Source:F19L87S0I87
;Source:F19L88S0I88
;Source:F19L90S0I90
;Source:F19L91S0I91
;Source:F19L92S0I92
;Source:F19L93S0I93
BASPROGRAMEND:
	sleep
	rjmp	BASPROGRAMEND

;********************************************************************************

Delay_10US:
D10US_START:
	ldi	DELAYTEMP,52
DelayUS1:
	dec	DELAYTEMP
	brne	DelayUS1
	nop
	dec	SysWaitTemp10US
	brne	D10US_START
	ret

;********************************************************************************

Delay_MS:
	inc	SysWaitTempMS_H
DMS_START:
	ldi	DELAYTEMP2,254
DMS_OUTER:
	ldi	DELAYTEMP,20
DMS_INNER:
	dec	DELAYTEMP
	brne	DMS_INNER
	dec	DELAYTEMP2
	brne	DMS_OUTER
	dec	SysWaitTempMS
	brne	DMS_START
	dec	SysWaitTempMS_H
	brne	DMS_START
	ret

;********************************************************************************

INITSYS:
;Source:F12L431S0I388
	ldi	SysValueCopy,0
	out	PORTB,SysValueCopy
;Source:F12L434S0I391
	ldi	SysValueCopy,0
	out	PORTC,SysValueCopy
;Source:F12L437S0I394
	ldi	SysValueCopy,0
	out	PORTD,SysValueCopy
	ret

;********************************************************************************

FN_READAD:
;Source:F2L569S0I3
;Source:F2L486S0I379
	lds	SysValueCopy,ADREADPORT
	sts	ADMUX,SysValueCopy
;Source:F2L504S0I397
;Source:F2L505S0I398
	sbr	SysValueCopy,1<<ADLAR
	sts	ADMUX,SysValueCopy
;Source:F2L506S0I399
;Source:F2L507S0I400
;Source:F2L514S0I407
;Source:F2L515S0I408
	sbr	SysValueCopy,1<<REFS0
	sts	ADMUX,SysValueCopy
;Source:F2L516S0I409
;Source:F2L517S0I410
;Source:F2L540S0I433
	lds	SysValueCopy,ADCSRA
	sbr	SysValueCopy,1<<ADPS2
	sts	ADCSRA,SysValueCopy
;Source:F2L541S0I434
	cbr	SysValueCopy,1<<ADPS1
	sts	ADCSRA,SysValueCopy
;Source:F2L550S0I443
	ldi	SysWaitTemp10US,2
	rcall	Delay_10US
;Source:F2L553S0I446
	lds	SysValueCopy,ADCSRA
	sbr	SysValueCopy,1<<ADEN
	sts	ADCSRA,SysValueCopy
;Source:F2L555S0I448
;Source:F2L558S0I451
	sbr	SysValueCopy,1<<ADSC
	sts	ADCSRA,SysValueCopy
;Source:F2L559S0I452
SysWaitLoop1:
	lds	SysBitTest,ADCSRA
	sbrc	SysBitTest,ADSC
	rjmp	SysWaitLoop1
;Source:F2L560S0I453
	lds	SysValueCopy,ADCSRA
	cbr	SysValueCopy,1<<ADEN
	sts	ADCSRA,SysValueCopy
;Source:F2L582S0I16
	lds	SysValueCopy,ADCH
	sts	READAD,SysValueCopy
	ret

;********************************************************************************


