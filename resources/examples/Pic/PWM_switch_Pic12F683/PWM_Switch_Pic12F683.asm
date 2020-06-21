;   CLOCK: Internal RC 4MHz (instruction execution time: 1usec)
;    To control H-Bridge direction and speed.
;   Read ADC GP0 write 10-bit value to PWM GP2
;    SW1 connected GP3, SW2 connected GP4
;    SW1 toggles GP1 ON-OFF
;    SW2 toggles GP5 ON-OFF
;    GP1 and GP5 connected to H-Bridge inputs.
;    If GP1 is LOW AND GP5 is LOW PWM turned off.
;    Lewis Loflin    
;   www.bristolwatch.com
;==================================================================================
           list     P=12F683, ST=OFF   ; Turnoff Symbol Table in List file.

        errorlevel  -302        ; Ignore error message when storing to Bank 1.
;==================================================================================

        #include   <p12f683.inc>

   __config _INTOSCIO & _WDT_OFF & _PWRTE_ON & _MCLRE_OFF & _CP_OFF & _CPD_OFF & _BOD_NSLEEP & _IESO_OFF & _FCMEN_OFF

;*****[ Data Storage Reg's ]*****
            CBLOCK  0x20        ; Assign each reg. from Bank 0 RAM area.
            count1              
            count2             
            count3
            count4
            temp
            RESULTHI
            RESULTLO
            ENDC                ; Conclude Bank 0 RAM assignments.

;**********************************************************************
    ORG     0x000             ; processor reset vector
    goto    setup              ; go to beginning of program
    
    ORG     0x004             ; interrupt vector location
    
;**********************************************************************
            
            
            
setup
        BANKSEL OSCCON          ; Switch to Bank 1.
        MOVLW   b'01100001'     ; 4MHz Clk, IntOsc, SysClk via IntOsc
        MOVWF   OSCCON
;
        BANKSEL CMCON0          ; Switch to Bank 0.
        MOVLW   b'00000111'     ; Turn off Comparator.
        MOVWF   CMCON0  
 
        BANKSEL ANSEL           ; Switch to Bank 1.
        CLRF    ANSEL           ; Set I/O pins to Digital.
;
        ; Define inputs & outputs.
        CLRF   TRISIO ; all output
        BSF       TRISIO, GP3 ; GP3 input
        BSF       TRISIO, GP4 
;
        BANKSEL GPIO            ; Switch to Bank 0.
        CLRF    GPIO            

; This code block configures the ADC for polling, 
; Vdd reference, Frc clock and GP0 input.

    BANKSEL TRISIO ; bank 1
    BSF     TRISIO, 0 ; Set GP0 for ADC input
    BANKSEL ANSEL ; 0x9F P32 
    MOVLW   b'01110001' ; ADC Frc clock,
    IORWF   ANSEL ; and GP0 as analog 
    BANKSEL ADCON0 ; 0x1F
    MOVLW   b'00000001' ; Left justify,
    MOVWF   ADCON0 ; Vdd Vref, AN0, On
    CALL    delay_1ms ; Acquisiton delay




; setup PWM
; CCP1CON in bank 0 bits 4-5 are LSB of 10-bit PWM


    movlw    b'00000111'
    movwf    T2CON ; turn on TMR2 prescale 16 - frequency
    movlw    d'127' ; duty cycle = TMR2 = CCPR1L:CCP1CON<5:4>

    movwf    CCPR1L
    movlw    b'00001110'
    movwf    CCP1CON     ; turn on PWM 
        
        goto loop
;
;==================================================================================

loop
    CALL  ADCtoPWM
    BTFSS GPIO, GP3 ; SW1
    CALL  toggleGP1
    BTFSS GPIO, GP3
    GOTO $-1 ; wait for release

    BTFSS GPIO, GP4
    CALL  toggleGP5
    BTFSS GPIO, GP4
    GOTO $-1 ; wait for release

;    If GP1 is LOW AND GP5 is LOW PWM turned off.
    BTFSC GPIO, 1
    goto $+5
    BTFSC GPIO, 5
    goto $+3
    CALL PWMOFF
    goto loop
    Call PWMON

goto loop
        
; delay routines *************************************

; Calculating a 1mSec delay. 4mHz is divided by 4 internally to
; 1,000,000. Take reciprocal divide 1mSec by 1uSec = 1000.
; GOTO uses 2 cycles DECFSZ 1 cycle = 3 cycles or 3uSec.
; 3 * 167 * 2 = 1000uSec. or 1mSec.


delay_1ms
    ; 4 mHz crystal
    movlw D'2'
    movwf count1
aa    movlw D'167'
    decfsz count2, F
    goto $-1 ; two cycles
    decfsz count1, F
    goto aa 
    return


delay_100ms
    movlw d'100'
    movwf count3
bb    call delay_1ms
    decfsz count3, F
    goto bb
    return
;============================================================    

PWMOFF
    BANKSEL TRISIO
    BSF TRISIO, GP2
    BANKSEL GPIO
    CALL delay_1ms
    RETURN

PWMON
    BANKSEL TRISIO
    BCF TRISIO, GP2
    BANKSEL GPIO
    CALL delay_1ms
    RETURN
    
;####################################

readADC0
    BSF     ADCON0,GO  ;Start conversion
    BTFSC   ADCON0,GO ;Is conversion done?
    GOTO    $-1  ;No, test again
    MOVF    ADRESH,W ;Read upper 8 bits
    MOVWF   RESULTHI ;Store in GPR space
; ADRESL bits 6, 7 LSB of ADC
    BANKSEL ADRESL
    MOVFW   ADRESL
    BANKSEL ADRESH ; bank 0
; right shift bits 7, 6 to bits 4, 5
    MOVWF   RESULTLO
    RRF     RESULTLO, f
    RRF     RESULTLO, f
    return    

;######################################

ADCtoPWM
    CALL  readADC0
    MOVFW RESULTHI
    MOVWF CCPR1L
; RESULTLO bits 5, 4 LSB of ADC
; CCP1CON bit 5,4 LSB PWM

    ;MOVLW    b'00001110'
    ;IORWF  RESULTLO, w
    ;MOVWF    CCP1CON     ; turn on PWM 
    return
    
;######################################

; XOR toggle
toggleGP1 ; LED on GP1
    movlw 2
    xorwf GPIO, f
    CALL delay_1ms
    return    


; non-XOR toggle
toggleGP5 ; LED on GP5

    BTFSS GPIO, 5
    goto $+3
    BCF GPIO, 5
    goto $+2    
    BSF GPIO, 5
    CALL delay_1ms
    return    
    
    
end
