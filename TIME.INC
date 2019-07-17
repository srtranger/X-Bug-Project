;-----  Њ®¤г«м Ї®¤Їа®Ја ¬¬ § ¤Ґа¦ҐЄ Ё в ©¬Ґа®ў  ------
;
; Set_WDT         - § ЇгбЄ Ґв WatchDog Timer ­  ­Ґ®Ўе®¤Ё¬®Ґ
;                   ўаҐ¬п. Ќ  ўе®¤Ґ ў r16 Є®­бв ­в  § Јаг§ЄЁ
;                   ( б¬ в Ў«Ёжг ў ¬®¤г«Ґ define.inc )
;
; Delay_NNms      - § ¤Ґа¦Є  ў ¬Ё««ЁбҐЄг­¤ е (1..255)
;
; Delay_50ms,     - ®аЈ ­Ё§ жЁп § ¤Ґа¦ҐЄ Ї® в ©¬Ґаг.
; Delay_19ms,       Ќ  ўе®¤Ґ ў r16,r17  б®®вўҐвбвўҐ­­®
; Delay_100ms       бв аиЁ© Ё ¬« ¤иЁ© Ў ©в Є®­бв ­вл § ¤Ґа¦ЄЁ.
;                   ( б¬ в Ў«Ёжг ў ¬®¤г«Ґ define.inc )
;
; Delay_ACCx100ms - § ¤Ґа¦Є  (1*100..255*100 ¬Ё««ЁбҐЄг­¤).
;                    Њ­®¦ЁвҐ«м - ў ACC.
;
; Delay_500ms,    - § ¤Ґа¦ЄЁ ­  0.5 Ё 1 бҐЄ. €бЇ®«м§гҐвбп
; Delay_1s          ЇаҐ¤л¤гй п Ї®¤Їа®Ја ¬¬ .
;
; Delay_30s_check - § ¤Ґа¦Є  ­  30 бҐЄ. ‚®§ўа в Ё§ Ї®¤Їа®Ја ¬¬л
;                   ¤® ®Є®­з ­Ёп ўл¤Ґ¦ЄЁ Ї® д Євг § ЄалвЁп ¤ўҐаЁ
;                   Ё«Ё ўлЄ«озҐ­Ёп § ¦ЁЈ ­Ёп.
;-----------------------------------------------------

;-----------------------------------------------------
; WatchDog Timer presets:
.equ            disable   = 0b00010000
.equ            del_15ms  = 0b00011000
.equ            del_30ms  = 0b00011001
.equ            del_60ms  = 0b00011010
.equ            del_120ms = 0b00011011
.equ            del_240ms = 0b00011100
.equ            del_490ms = 0b00011101
.equ            del_970ms = 0b00011110
.equ            del_2s    = 0b00011111
;-----------------------------------------------------
; Timer1 Presets for delays:
; 50ms:
.equ            d50_hi   = $3C
.equ            d50_lo   = $AF

; 1 ms
.equ            d1_hi    = $FC
.equ            d1_lo    = $17


; 30 ms
.equ            d30_hi   = $8A
.equ            d30_lo   = $CF

; 19 ms
.equ            d19_hi   = $B5
.equ            d19_lo   = $C7
; 16 ms
.equ            d16_hi   = $C1
.equ            d16_lo   = $7F
;-----------------------------------------------------

;-----------------------------------------------------
Set_WDT:     ;  ‚ ACC - §­ зҐ­ЁҐ ¤«п аҐЈЁвба  WDTCR
                     wdr
                     out WDTCR,ACC
                     ret
;-----------------------------------------------------

;------------------------------------------------------------
Start_Timer:
                     ldi ACC,$02
                     out TCCR1B,ACC
                     ret
;------------------------------------------------------------

;------------------------------------------------------------
Reset_Timer:
                     ldi  ACC,0
                     out  TCCR1A,ACC  ; €­ЁжЁ «Ё§ жЁп Ё ®бв ­®ў в ©¬Ґа 
                     out  TCCR1B,ACC

                     in   ACC,TIFR               ; read TIFR
                     sbr  ACC,(1<<TOV1)          ; clear TOV1
                     out  TIFR,ACC
                     ret
;---------------------------------------------------------------

;---------------------------------------------------------------
start_timer_r16r17:
                     rcall Reset_Timer
                     out TCNT1H,r16
                     out TCNT1L,r17
                     rcall Start_Timer
                     ret
;---------------------------------------------------------------

;----------------------------------------------------------
Delay_NNms:
                     rcall    start_timer_r16r17
delNN_loop:
                     in       r16,TIFR
                     sbrs     r16,TOV1
                     rjmp     DelNN_loop

                     ret
;----------------------------------------------------------

;----------------------------------------------------------
Delay_50ms:
                     ldi     r16,d50_hi
                     ldi     r17,d50_lo
                     rcall   Delay_NNms
                     ret
;----------------------------------------------------------

;----------------------------------------------------------
Delay_19ms:
                     ldi     r16,d19_hi
                     ldi     r17,d19_lo
                     rcall   Delay_NNms
                     ret
;----------------------------------------------------------

;----------------------------------------------------------
Delay_30ms:
                     ldi     r16,d30_hi
                     ldi     r17,d30_lo
                     rcall   Delay_NNms
                     ret
;----------------------------------------------------------

;----------------------------------------------------------
Delay_100ms:
                     rcall   Delay_50ms
                     rcall   Delay_50ms
                     ret
;----------------------------------------------------------

;----------------------------------------------------------
Delay_1s:
                     ldi     ACC,10
                     rcall   Delay_ACCx100ms
                     ret
;----------------------------------------------------------
;----------------------------------------------------------
Delay_500ms:
                     ldi     ACC,5
                     rcall   Delay_ACCx100ms
                     ret
;----------------------------------------------------------

;----------------------------------------------------------
Del_ACCx100ms_Check:
                     clt
                     ldi     Cr,10
delacc1_loop:
                     push    ACC
                     rcall   Delay_100ms
                     rcall   Check_Ignition
                     brcc    d_clcret
                     dec     Cr
                     brne    d_next_step1
                     ldi     Cr,5
                     rcall   Verify_Card_NXX
                     brcs    d_sett

                     ldi     Cr,10
d_next_step1:
                     pop     ACC
                     dec     ACC
                     brne    delacc1_loop
                     rjmp    da_toret

d_sett:              set

d_clcret:
                     pop     ACC
da_toret:
                     ret
;----------------------------------------------------------

;----------------------------------------------------------
Delay_ACCx100ms:
                     push    ACC
                     rcall   Delay_100ms
                     pop     ACC
                     dec     ACC
                     brne    Delay_ACCx100ms
                     ret
;----------------------------------------------------------

Delay_30s_check:
                     ldi     Cr,30
del20c_loop:
                     rcall   Delay_1s
                     rcall   Check_Ignition
                     brcc    toclcret
                     rcall   Check_Door
                     brcc    door_now_close
                     dec     Cr
                     brne    del20c_loop
door_now_close:
                     sec
                     rjmp    d3_toret
toclcret:            clc
d3_toret:
                     ret
;----------------------------------------------------------

