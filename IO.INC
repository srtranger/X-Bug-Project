;----------------------------------------------------------
; В модуле функции работы с EEPROM: чтение байта, запись байта и
;  сохранение регистров состояния.
;
;----------------------------------------------------------

;----------------------------------------------------------
;     Пишет байт в EEPROM. В r16 - адрес [0..64], r17 - данные
Write_EEprom:

we_loop:
                     sbic EECR,EEWE
                     rjmp we_loop                   ; Ожидаем EEWE==0
                     out  EEAR,r16                  ; Выставили адрес,
                     out  EEDR,r17                  ; данные
                     sbi  EECR,EEMWE                ; Разрешение записи EEPROM
                     sbi  EECR,EEWE                 ; Cтроб записи
ee_loop:
                     sbic EECR,EEWE                 ; EEWE=0, когда закончится
                     rjmp ee_loop                   ; запись
                     cbi  EECR,EEMWE                ; Запрет на запись EEPROM
                     ret
;----------------------------------------------------------

;----------------------------------------------------------
              ;     Читает байт из EEPROM
              ;     Вход: r16 - адрес [0..64].   Выход: r17 - данные
Read_EEprom:
                     out  EEAR,r16                  ; Выставили адрес,
                     sbi  EECR,EERE                 ; Разрешение чтения EEPROM
                     in   r17,EEDR
                     ret
;----------------------------------------------------------

;----------------------------------------------------------
; Сохраняет два байта состояния State_Reg1 и State_Reg2 в EEPROM

Save_Context:
                     ldi    r16,EE_FLAGS
                     rcall  Read_EEPROM ; r17 contain data
                     cp     r17,State_Reg1
                     breq   skip_save1

                     ldi    r16,EE_FLAGS
                     mov    r17,State_Reg1
                     rcall  Write_EEPROM
skip_save1:
                     ldi    r16,EE_FLAGS+1
                     rcall  Read_EEPROM ; r17 contain data
                     cp     r17,State_Reg2
                     breq   skip_save2

                     ldi    r16,EE_FLAGS+1
                     mov    r17,State_Reg2
                     rcall  Write_EEPROM
skip_save2:
                     ldi    r16,Blocks_CRE
                     rcall  Read_EEPROM ; r17 contain data
                     cp     r17,Blocks_Cr
                     breq   skip_save3

                     ldi    r16,Blocks_CRE
                     mov    r17,Blocks_Cr
                     rcall  Write_EEPROM
skip_save3:
                     ret
;----------------------------------------------------------

