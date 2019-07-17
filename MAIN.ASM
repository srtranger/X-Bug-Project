.include "2313def.inc"
.include "macros.inc"
.include "define.inc"

.CSEG
.ORG                 $0000

                     rjmp    main

.ORG                 $000B
;------------------------------------------------------------------------

;------------------------------------------------------------------------
main:
                     ldi    ACC,low (RAMEND)     ; Stack pointer
                     out    spl,ACC

                     rcall  Init                 ; Инициализация cистемы
                                                 ; Определение текущего режима

                     sbrc   State_Reg1,Alarm
                     rjmp   Alarm_mode

                     sbrc   State_Reg1,Serv
                     rjmp   Service_Mode

                     sbrc   State_Reg1,Armed
                     rjmp   Armed_Mode
       ; В остальных случаях становимся в Passive Mode
;-----------------------------------------------------------
;**************  Passive MODE  *******************************
Passive_Mode:
                     LED_OFF                    ; Светодиод выключен
                     cpi    Blocks_Cr,0
                     brne   No_Blocks
                     BLOCK_ON
                     rjmp   full_block
No_Blocks:
                     rcall  Check_Ignition      ; возврат C=1 если зажигание включено
                     brcs   Ign_On              ; переход по C=1 на проверку карточки

                     rcall  Count_Valet_Press   ; Возвращает число нажатий кнопки "Valet"

                     cpi    ACC,0               ; Число нажатий должно быть
                     breq   No_Blocks           ; в пределах от 1 до 5

                     cpi    ACC,6               ; иначе идет возврат в начало
                     brcc   No_Blocks           ; цикла

                     push   ACC
                     rcall  Verify_Card_N10     ; Проверка "своей" карточки:
                     pop    ACC
                     brcc   No_Blocks           ; - карточка не распознана;

                                                ; - карточка "своя".
                                                ; Поведение по числу нажатий:
                     cpi    ACC,1               ; 1 - "громкая" постановка на охрану
                     brne   next_pass1
                     rjmp   To_Armed_Mode_With_Siren
next_pass1:
                     cpi    ACC,2               ; 2 - "тихая" постановка на охрану
                     brne   next_pass2
                     rjmp   To_Armed_Mode_Without_Siren
next_pass2:
                     cpi    ACC,3
                     brne   next_pass3
                     rjmp   To_Service_Mode       ; в сервисный режим
next_pass3:
                     mov    Temp,ACC
                     rcall  Check_Door            ; обучение и программирование возможно лишь при открытой двери
                     brcc   No_Blocks

                     mov    ACC,Temp
                     cpi    ACC,4
                     brne   next_pass4
                     rjmp   Funct_Ch_Mode        ; 4+Door - в режим программирования функций
next_pass4:
                     cpi    ACC,5
                     brne   No_Blocks
                     rjmp   Learn_Mode

; при включенном зажигании
Ign_On:
                     rcall  Verify_Card_N10     ; Проверка карточки:
                     brcs   Normal_Drive        ;  - все в порядке
Blocks_place:                                   ;  - нет карточки или чужая
                     ldi    r16,Blocks_addr-1   ; Организация блокировок
                     add    r16,Blocks_Cr       ; Читаем значение задержки для
                     rcall  Read_EEPROM         ; очередной блокировки из EEPROM
                     mov    ACC,r17
                     rcall  Del_ACCx100ms_Check ; Даем полученную выдержку времени
                     brts   Normal_Drive
                     BLOCK_ON                   ; Включаем блокировки

                     dec    Blocks_Cr           ; Уменьшаем счетчик временных блокировок
                     brne   ig_off_loop         ; если не ноль - разблокировка допустима

                     rcall  Save_Context
                     sbrc   State_Reg2,Hi_Jack  ; Если флаг Hi_Jack=1
                     rjmp   Alarm_Mode          ; то включаем сигнализацию
full_block:
                     rcall  Check_Ignition      ; ждем выключения зажигания
                     brcs   full_block          ; и по ее нажатию при наличие
                     rcall  Verify_Card_N10     ; "своей" карточки
                     brcc   full_loop           ; переходим далее
                     rjmp   Go_Pass
full_loop:
                     rcall  delay_1s
                     rcall  Check_Ignition
                     brcc   full_loop           ; и по ее нажатию при наличие
                     rcall  Verify_Card_N10     ; "своей" карточки
                     brcc   full_loop           ; переходим в начало Passive_Mode c
                     rcall  Message_Arm_To_Passiv
                     rjmp   Go_Pass

ig_off_loop:
                     rcall  Save_Context
                     rcall  Check_Ignition     ; Снятие врем. блокир-ки
                     brcs   ig_off_loop        ; по выкл. зажигания
                     BLOCK_OFF                 ;
                     rjmp   Passive_Mode


;---------------------- Зажигание включено, карточка распознана
Normal_Drive:
                     BLOCK_OFF                 ; снятие блокировок
                     ldi    Blocks_Cr,N_Blocks
                     cbr    State_Reg1,(1<<F_Anti)
                     rcall  Save_Context
                     rcall  Message_Read_Ok
;-------------------
; Смысл цикла Hi_Jack: Если двери закрыты, карточку проверять не надо, двигатель
; может работать даже при отсутствии карточки в зоне антенны.
; Как только было обнаружено открывание двери - это запоминается в флаге
; и пока открыты двери каждые 30 секунд пока включено зажигание опрашивается
; карточка. Если дверь теперь закрыть, карточка еще раз проверится и до следующего
; открытия либо до выключения зажигания проверятся не будет.

Hi_Jack_Loop:
                     rcall  Check_Ignition     ; При выключении зажигания
                     brcs   next_hi_j1
                     rjmp   Go_Pass            ; возвращаемся в начало режима
next_hi_j1:
                     rcall  Check_Door         ; при открытой двери -
                     brcc   door_clos
                     rcall  Delay_30s_check    ; задержка 30 сек,

                     brcs   next_hi_j2
                     rjmp   Go_Pass            ; нет зажигания - возврат в начало режима
next_hi_j2:
                     rcall  Verify_Card_N10    ; проверяем карточку
                     brcc   Blocks_place       ; нет карточки в зоне (чужая)

                     sbr    State_Reg1,(1<<F_Anti) ; уст. флаг открытия двери
                     rjmp   hi_jack_loop
door_clos:                                     ; Если дверь закрыта, то:
                     sbrs   State_Reg1,F_Anti  ; если дверь была открыта
                     rjmp   Hi_Jack_Loop       ; и закрылась,

                     rcall  Verify_Card_N10    ; проверяем карточку и
                     brcc   Blocks_place       ; блокируем если карточки нет

                     cbr    State_Reg1,(1<<F_Anti) ; сброс флага открытия двери
                     rjmp   Hi_Jack_Loop
;--------------------------------------------------------------------------------

;-----------------------------------------------------------------------------------
;                    Переключения режимов работы системы

To_Service_Mode:
                     cbr    State_Reg1,(1<<Passiv)
                     sbr    State_Reg1,(1<<Serv)
                     rcall  Save_Context
                     rcall  Message_Pass_To_Ser
                     rjmp   Service_Mode

;----------------------------------------------------
To_Armed_Mode_With_Siren:
                     sbr    State_Reg1,(1<<Voice)
                     rjmp   To_Armed_Mode

;----------------------------------------------------
To_Armed_Mode_Without_Siren:
                     cbr    State_Reg1,(1<<Voice)
                     rjmp   To_Armed_Mode

;----------------------------------------------------
To_Armed_Mode:
                     sbrs   State_Reg1,Ar_En          ; Проверка запрета
                     rjmp   No_Blocks                 ; постановки на охрану

                     sbr    State_Reg1,(1<<Armed)
                     cbr    State_Reg1,(1<<Passiv)

                     cbr    State_Reg2,(1<<Al_Do) ; Сброс "памяти" событий
                     cbr    State_Reg2,(1<<Al_Sen) ; для режима охраны
                     cbr    State_Reg2,(1<<Al_Ig)

                     rcall  Message_To_Armed

                     rcall  Check_Ignition
                     brcc   next_toam
                     sbr    State_Reg2,(1<<Al_Ig)
next_toam:
                     rcall  Save_Context        ; сохранение изменений в EEPROM
                     rjmp   Armed_Mode
;---------------------------------------------------
To_Passive_Mode:
                    BLOCK_OFF
                    sbr    State_Reg1,(1<<Passiv) ; Переход в Passiv_Mode
                    cbr    State_Reg1,(1<<Armed)
                    cbr    State_Reg1,(1<<Alarm)
                    rcall  Save_Context
                    rcall  Message_Arm_To_Passiv  ; Сообщение о переходе

Go_Pass:
                    cbr    State_Reg2,(1<<Al_Do) ; Сброс "памяти" событий
                    cbr    State_Reg2,(1<<Al_Sen) ; для режима охраны
                    cbr    State_Reg2,(1<<Al_Ig)
                    ldi    Blocks_Cr,N_BLOCKS
                    rcall  Save_Context
                    rjmp   Passive_Mode
;----------------------------------------------------------
.include "time.inc"
.include "init.inc"
.include "io.inc"
.include "message.inc"
.include "modes.inc"
.include "command.inc"
.include "rfm1.inc"

.ESEG

CARD_ADDR:
                     .db    $2C,$0E,$FF,$01,$0,$0,$0,$0 ; Коды карточек
                     .db    $84,$11,$FF,$01,$0,$0,$0,$0

EE_FLAGS:            .db    STR1,STR2,BLCR

Blocks_addr:         .db    $08,$22,$86,$36,$B8

Blocks_CRE:          .db    5



