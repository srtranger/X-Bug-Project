;-----------------------------------------------------------
; Инициализация
Init:
                     ldi    ACC,$07        ;00000111
                     out    DDRD,ACC       ; Set-up for pins PORTD

                     ldi    ACC,$F8        ;11111000
                     out    PORTD,ACC      ; Definition Pull's PORTD

                     ldi    ACC,$F9        ; 11111001
                     out    DDRB,ACC       ;  Set-up for pins PORTB

                     SBI    PORTB,PB1      ;Definition Pull's PORTB
                     SBI    PORTB,PB2      ;Definition Pull's PORTB

                     LED_OFF
                     SPEAKER_OFF
                     SIREN_OFF
                     BLOCK_OFF
                     LIGHT_OFF
                     POWER_RFM_OFF

                     ldi    r16,EE_FLAGS
                     rcall  Read_EEPROM          ; Читаем регистры
                     mov    State_Reg1,r17       ; состояния из EEPROM

                     ldi    r16,EE_FLAGS+1
                     rcall  Read_EEPROM
                     mov    State_Reg2,r17

                     ldi    r16,Blocks_CRE
                     rcall  Read_EEPROM
                     mov    Blocks_Cr,r17
                     ret
;----------------------------------------------------------
