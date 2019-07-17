;------------ Модуль подпрограмм для сервисных режимов -----
;
; Learn_Mode    -  режим обучения карточкам.
;
; Service_Mode  - сервисный режим.
;
; Funct_Ch_Mode - режим программирования функций.
; 1 -  Const(1)/Dot(2) siren signal
; 2 -  Armed mode disable(1)/enable(2)
; 3 -  Armed door open time = 7sec(1) / 15sec(2)
; 4 -  Full_Block without(1)/with(2) Alarm
;
; Armed_Mode    - режим охраны
;
; Alarm_Mode    - Cигнализация
;-----------------------------------------------------------

;------------------- Learn Mode ----------------------------
Learn_Mode:
             BLOCK_ON              ; Блокировки включены
             LED_ON
             rcall      delay_1s           ; Пауза 1 сек.
             rcall      Message_Learn_Mode ; Сообщение о входе в режим
             ldi        N_Card,2           ; Число обучаемых карточек
             ldi        ZL,Temp_Buffer     ; Адрес в RAM для буфера номеров
             ldi        ZH,0
learn_loop:
             rcall      Short_Speaker      ; Короткий beep
             cpi        N_Card,1
             brne       first_card
             rcall      delay_100ms        ; для 2-й карточки еще один beep
             rcall      Short_Speaker
first_card:
             rcall      delay_500ms
wait_valet:
             rcall      Check_Door
             brcc       leave_l_mode        ; Выход из режима по закрытию дверцы

             rcall      Check_Valet
             brcc       wait_valet        ; if C=1 - кнопка нажата

             ldi        N_Read,10
nread_loop:
             rcall      Read_Card
             sbrs       Flags,crc_ok
             rjmp       bad_card          ; Карточка не прочиталась
             rcall      delay_500ms
             rcall      Store_To_Buffer

             dec        N_Card
             brne       learn_loop

             ldi        Yl,Temp_Buffer    ; Сохраняем в EEPROM
             ldi        YH,0
             ldi        Cr,16
             ldi        r16,CARD_ADDR
wre_loop:
             ld         r17,Y+
             rcall      Write_EEPROM
             inc        r16
             dec        Cr
             brne       wre_loop

             ldi        Yl,Temp_Buffer    ; Сохраняем в EEPROM 2 копию
             ldi        YH,0
             ldi        Cr,16
             ldi        r16,CARD_ADDR
wre_loop1:
             ld         r17,Y+
             rcall      Write_EEPROM
             inc        r16
             dec        Cr
             brne       wre_loop1

             rcall      Message_Mode_Ok

             rjmp       learn_exit
bad_card:
             dec        N_Read
             brne       nread_loop
             rcall      Message_Bad_Card
             rjmp       wait_valet

leave_l_mode:
             rcall      Message_Leave
learn_exit:

             BLOCK_OFF               ; Блокировки сняты
             LED_OFF
             rjmp       Go_Pass
;-------------------------------------------------------------------

;-------------------------------------------------------------------
; ******************* Service Mode ****************************
Service_Mode:
                     LED_ON
                     BLOCK_OFF

                     rcall  Count_Valet_Press
                     cpi    ACC,2
                     brne   Service_Mode

                     rcall  Verify_Card_N10
                     brcc   Service_Mode

                     sbr    State_Reg1,(1<<Passiv)
                     cbr    State_Reg1,(1<<Serv)
                     rcall  Save_Context
                     rcall  Message_Ser_To_Pass
                     rjmp   Go_Pass
;----------------------------------------------------------

;---------------- Function programming mode ---------------
Funct_Ch_Mode:
                     BLOCK_ON
                     LED_ON
                     rcall  Message_FC_Mode
                     rcall  delay_1s

ch_again1:
                     ldi    ACC,1
                     rcall  N_Short_Beeps

                     rcall  Change1_2Menu
                     brts   mod_ch_setdefault
                     breq   ch_again1
                     brcs   ch1_2
                     cbr    State_Reg1,(1<<Dot_S)
                     rjmp   ch_again2
ch1_2:
                     sbr    State_Reg1,(1<<Dot_S)

ch_again2:
                     ldi    ACC,2
                     rcall  N_Short_Beeps
                     rcall  Change1_2Menu
                     brts   mod_ch_setdefault
                     breq   ch_again2
                     brcs   ch2_2
                     cbr    State_Reg1,(1<<Ar_En)
                     rjmp   ch_again3
ch2_2:
                     sbr    State_Reg1,(1<<Ar_En)

ch_again3:
                     ldi    ACC,3
                     rcall  N_Short_Beeps
                     rcall  Change1_2Menu
                     brts   mod_ch_setdefault
                     breq   ch_again3
                     brcs   ch3_2
                     cbr    State_Reg2,(1<<Arm_T)
                     rjmp   ch_again4
ch3_2:
                     sbr    State_Reg2,(1<<Arm_T)

ch_again4:
                     ldi    ACC,4
                     rcall  N_Short_Beeps
                     rcall  Change1_2Menu
                     brts   mod_ch_setdefault
                     breq   ch_again4
                     brcs   ch4_2
                     cbr    State_Reg2,(1<<Hi_Jack)
                     rjmp   mod_ch_ok
ch4_2:
                     sbr    State_Reg2,(1<<Hi_Jack)
                     rjmp   mod_ch_ok

mod_ch_setdefault:
                     ldi        State_Reg1,STR1
                     ldi        State_Reg2,STR2
                     rcall      Save_Context
                     rcall      Message_Leave
                     rjmp       mod_ch_exit
mod_ch_ok:
                     rcall      Save_Context
                     rcall      Message_Mode_Ok
mod_ch_exit:
                     BLOCK_OFF               ; Блокировки сняты
                     LED_OFF
                     rjmp       Go_Pass
;----------------------------------------------------------

;--------------------------------------------------------------------------------
;******************** Alarm_Mode ******************************
Alarm_Mode:                                      ; Сигнализация!
                     ldi    Cr,Alarm_Time        ; Установка времени аварии
ala_loop:
                     Siren_ON
                     Light_ON

                     rcall delay_1s
                     sbrs   State_Reg1,Dot_S
                     rjmp   const_signal
                     Siren_OFF
const_signal:
                     Light_OFF
                     rcall  delay_1s
                     dec    Cr
                     breq   end_al_cycle

;                     rcall  Check_Valet_Wo       ; Процесс выключения
;                     brcc   ala_loop             ; и переход в
                     ldi    Cr,6
                     rcall  Verify_Card_NXX      ; Passive Mode
                     brcc   ala_loop             ; если есть карточка и
Alarm_Off:
                     Siren_OFF                   ; нажата кнопка
                     rjmp   To_Passive_Mode
end_al_cycle:
                     SIREN_OFF
                     sbrc   State_Reg1,Armed    ; Если машина на охране -
                     rjmp   Armed_Mode          ; возврат в начало цикла охраны,
fb_cycle:
                     rcall  Check_Valet_Wo      ; иначе - ждем нажатия Valet
                     brcc   fb_cycle

                     rcall  Verify_Card_N10
                     brcc   Alarm_Mode

                     rjmp   To_Passive_Mode
;------------------------------------------------------------
; **************  ARMED MODE  *******************************
Armed_Mode:                                      ; Машина на охране
                     LED_OFF
                     BLOCK_ON                    ; Блокировки включены
timer_load:
                     ldi    ACC,3
                     mov    Sens_Alarms,ACC      ; Счетчик срабатывания датчика
                     ldi    ACC,3
                     mov    S_Timer_H,ACC        ; Установка таймера сброса
                     clr    S_Timer_L            ; счетчика сраб. датчика
AM_loop:
                     dec    S_Timer_L
                     brne   skip_hi
                     dec    S_Timer_H
                     breq   timer_load           ; Сброс счетика датчика
skip_hi:
                     sbrc   State_Reg2,Al_Ig     ; Обход зоны зажигания
                     rjmp   skip_igch

                     rcall  Check_Ignition       ; Срабатывание сигнализ-ии
                     brcs   ATA_Ign              ; по включению зажигания

skip_igch:
                     sbrc   State_Reg2,Al_Sen
                     rjmp   sens_no_activ        ; обход зоны датчика
                     rcall  Check_Sensor         ; проверка датчика
                     brcc   sens_no_activ
                     sbr    State_Reg2,(1<<Al_Sen)
                     dec    Sens_Alarms          ; Если датчик сработал 3 раза
                     breq   Arm_To_Alarm         ; в течении одного цикла S_Timer
                                                 ; - включаем сигнализацию, иначе
                     rcall  Sensor_Short_Alarm   ; выдаем предупреждающий сигнал сиреной
                     cbr    State_Reg2,(1<<Al_Sen)
sens_no_activ:
                     rcall  Check_Valet_Wo       ; Опрос кнопки Valet без ожидания отпускания
                     brcc   arm_next
Verif:                                           ; Если нажата кнопка Valet то
                     rcall  Verify_Card_N10      ; если читается "своя" карточка
                     brcc   Arm_To_Alarm         ; cнимаем с охраны систему
                                                 ; иначе - вкл. сигнализацию
                     rjmp   To_Passive_Mode

arm_next:
                     sbrc   State_Reg2,Al_Do
                     rjmp   AM_loop             ; обход зоны двери

                     sbrs   State_Reg2,Dor_C    ;  Проверка флага
                     rjmp   AM_loop             ;  игнорирования дверей

                     rcall  Check_Door          ; Опрос двери
                     brcc   AM_loop

                     sbrc   State_Reg2,Arm_T    ; Если открылась дверь
                     rjmp   tolong              ; Задаем выдержку времени
                     ldi    Cr,Armed_Delay_S    ; ( 7 сек. или )
                     rjmp   veral_loop
tolong:
                     ldi    Cr,Armed_Delay_L    ; ( 15 сек.)
veral_loop:
                     push   Cr
                     rcall  delay_100ms
                     ldi    Cr,3
                     rcall  Verify_Card_NXX
                     pop    Cr
                     brcs   A_TPM
                     dec    Cr
                     brne   veral_loop
                     sbr    State_Reg2,(1<<Al_Do)
Arm_To_Alarm:                                   ; Если закончился таймер
                     sbr    State_Reg1,(1<<Alarm)
                     rcall  Save_Context        ; запомнили факт включения
                     rjmp   Alarm_mode          ; включаем сигнализацию

ATA_Ign:
                     sbrc   State_Reg2, Al_Ig
                     rjmp   skip_igch
                     sbr    State_Reg2,(1<<Al_Ig)
                     rjmp   Alarm_mode
A_TPM:               rjmp   To_Passive_Mode
;------------
