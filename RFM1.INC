;               Module RFM provide to Read transponder or card
;

;-------------------------------------------------------------------------
Charge:
                CBI     PORTB,TXCT      ; начало импульса накачки
                                        ; ────┐        ┌───
                rcall   delay_30ms      ;     └──90ms──┘
                rcall   delay_30ms
                rcall   delay_30ms
                SBI     PORTB,TXCT      ;  setb    TXCT
                ret
;-------------------------------------------------------------------------

;===================================================================
;  Читает карточку.
;  Устанавливает бит crc_ok, если карточка прочитана успешно
;  иначе он неопределён
;-------------------------------------------------------------------------
Read_Card:
                POWER_RFM_ON
                rcall    Delay_19ms

                rcall    Charge                  ; выдать импульс накачки

                rcall    Read

                POWER_RFM_OFF

                ret
;-------------------------------------------------------------------------

;-------------------------------------------------------------------------
Read:
                ldi      r16,d1_hi
                ldi      r17,d1_lo   ; Delay 1 ms
                rcall    Delay_NNms

                rcall    Wait_for_start_byte
                brcs     r_time_out
                rcall    Store_and_BCC
                rjmp     r_exit
r_time_out:
                cbr      Flags,(1<<crc_ok)       ;clr     crc_ok
r_exit:
                ret
;-------------------------------------------------------------------------

;-------------------------------------------------------------------------
Wait_for_start_byte:
                ldi      r16,d19_hi             ; set Timer1 for 19ms
                ldi      r17,d19_lo
                rcall    start_timer_r16r17
wait_first:
                sbis     PINB,RXDT
                rjmp     processed
                in       ACC,TIFR
                sbrc     ACC,TOV1
                rjmp     w_time_out             ; if TOV Set -> Time out!.
                rjmp     wait_first             ; Cont. wait
processed:
                ldi      Counter,$0
rx_front:
                rcall    RXCK_front

                in       ACC,TIFR
                sbrc     ACC,TOV1
                rjmp     w_time_out

                sbic     PINB,RXDT
                rjmp     wait_first

                inc      Counter
                cpi      Counter,$6
                brne     rx_front               ; кол-во "1" старт-битов

                rcall    RXCK_front

                in       ACC,TIFR
                sbrc     ACC,TOV1
                rjmp     w_time_out

                sbic     PINB,RXDT
                rjmp     w_ok
w_time_out:
                sec
                rjmp     w_exit
w_ok:
                clc
w_exit:
                ret
;-------------------------------------------------------------------------

;-------------------------------------------------------------------------
Store_and_BCC:
                ldi     BCC_L,0                ; Обнуление BCC
                ldi     BCC_H,0                ;
                ldi     Bit_Cr,0               ; r18 - счетчик битов
                ldi     YL,data_buffer-1;     ; загрузка Y - указателя
                ldi     YH,0                  ; ---------------------
rx3:
                rcall   RXCK_front

                in      ACC,TIFR
                sbrc    ACC,TOV1
                rjmp    rx_end

                rcall   store_rxdt
                rcall   gen_bcc

                inc     Bit_Cr
                cpi     Bit_Cr,80
                brne    rx4
                cpi     BCC_L,0
                brne    rx_end                  ; Конец чтения
                cpi     BCC_H,0                 ; если BCC<>0
                brne    rx_end
rx4:
                cpi     Bit_Cr,104
                brne    rx3                ;-------Конец цикла чтения
                lds     ACC,rec_stop
                cpi     ACC,$7E
                brne    rx_end             ; проверка STOP=7Eh
                lds     ACC,rec_end
                cpi     ACC,0
                brne    rx_end             ; проверка END=0000h
                lds     ACC,rec_end+1
                cpi     ACC,0
                brne    rx_end

                sbr     Flags,(1<<crc_ok)    ; код принят!
                rjmp    exit_rx
rx_end:
                cbr     Flags,(1<<crc_ok)     ; код не принят!
exit_rx:
                ret
;-------------------------------------------------------------------------

;-------------------------------------------------------------------------
RXCK_front:
rx_wait:
                sbis    PINB,RXCK
                rjmp    rx_processed

                in      ACC,TIFR
                sbrc    ACC,TOV1
                rjmp    rx_exit
                rjmp    rx_wait
rx_processed:
                sbic    PINB,RXCK
                rjmp    rx_exit

                in      ACC,TIFR
                sbrs    ACC,TOV1
                rjmp    rx_processed
rx_exit:
                ret
;----------------------------------------------------------------
store_rxdt:
        ; Процедура сохранения принятого бита в текущей позиции
        ; приема.  Bit_Cr - счетчик битов, Y - указатель на буфер
                mov     ACC,Bit_Cr
                andi    ACC,$7            ; anl     a,#7
                brne    store_rxdt_0
                inc     YL
store_rxdt_0:

                ld      ACC,Y            ; ACC <--- SRAM(Y)

                clc                      ; C=0
                clt                      ; T=0;

                sbic    PINB,RXDT
                rjmp    rxon
                sec                      ; if RXDT=0 -> C=1,
                set                      ; T=1
rxon:
                ror     ACC

                st      Y,ACC             ;  SRAM(Y) <-- ACC
                ret
;----------------------------------------------------------------

gen_bcc:
        ; Подпрограмма генерации Block Check Characters
                clc
                brtc    g_next
                sec
g_next:
                ror     BCC_H             ; Сдвиг вправо ст.б.
                ror     BCC_L             ; Сдвиг вправо мл.б. BCC
                brcc    gen_bcc_1
                ldi     ACC,$80
                eor     BCC_H,ACC         ; Инверсия BCC.15
gen_bcc_1:
                sbrs    BCC_H,7
                rjmp    gen_bcc_2

                ldi     ACC,$04
                eor     BCC_H,ACC
                ldi     ACC,$08
                eor     BCC_L,ACC
gen_bcc_2:
                ret
;----------------------------------------------------------------
Clear_Bufer:
                ldi     Counter,13
                ldi     YL,data_buffer
                ldi     YH,0
                ldi     ACC,0
cl_loop:
                st      Y+,ACC
                dec     Counter
                brne    cl_loop
                ret
