; Модуль подпрограмм звуковых и световых "сообщений" системы.
;
; Message_Learn_Mode      - вход в режим обучения;
; Message_Read_Ok         - карточка распознана;
; Message_Mode_Ok         - конец режима (обучение или
;                           программирование функций);
; Message_Leave           - прерывание режима;
; Message_Bad_Card        - карточка не распознана
;                           (не прочиталась);
; Message_Ser_To_Pass     - переход из "Service Mode" в
;                           "Passive Mode";
; Message_Pass_To_Ser     - переход из "Passive Mode" в
;                           "Service Mode";
; Message_Arm_To_Passiv   - cнятие с охраны;
; Message_To_Armed        - постановка на охрану;
; Sensor_Short_Alarm      - предупреждающая сигнализация;
; Short_Speaker           - короткий сигнал бипером;
; Long_Speaker            - долгий сигнал бипером;
; N_Short_Beeps           - несколько (1..255) коротких
;                           сигналов бипером;
; Message_FC_Mode         - вход в режим программирования
;                           функций.
;
;----------------------------------------------------------
Message_Learn_Mode:
                     ldi      Cr,3
beep1_loop:
                     rcall  Short_Speaker
                     dec    Cr
                     brne   beep1_loop
                     rcall  Delay_500ms
                     ret
;----------------------------------------------------------

;----------------------------------------------------------
Message_Read_Ok:
                     rcall  Short_Speaker
                     rcall  Delay_500ms
                     ret
;----------------------------------------------------------

;----------------------------------------------------------
Message_Bad_Card:
                     ldi    Cr,2
beep2_loop:
                     rcall  Short_Speaker
                     dec    Cr
                     brne   beep2_loop
                     rcall  Long_Speaker
                     rcall  Delay_500ms
                     ret
;----------------------------------------------------------

;----------------------------------------------------------
Message_Mode_Ok:
                     ldi     Cr,2
beep3_loop:
                     rcall  Long_Speaker
                     dec    Cr
                     brne   beep3_loop
                     rcall  Delay_500ms
                     ret

;----------------------------------------------------------

;----------------------------------------------------------
Message_Leave:
                     ldi    Cr,5
beep4_loop:
                     rcall  Short_Speaker
                     dec    Cr
                     brne   beep4_loop
                     rcall  Delay_500ms
                     ret
;----------------------------------------------------------

;----------------------------------------------------------
Message_Arm_To_Passiv:
                     ldi    Cr,2
beep5_loop:
                     rcall  Short_Speaker
                     dec    Cr
                     brne   beep5_loop

                     ldi    Cr,2
                     sbrc   State_Reg2,Al_Sen
                     ldi    ACC,3
                     sbrc   State_Reg2,Al_Do
                     ldi    ACC,4
nch1:                sbrc   State_Reg2,Al_Ig
                     ldi    ACC,5
matp_pred:
                     push   Cr
matp_loop:
                     Light_ON
                     rcall  Delay_500ms
                     Light_OFF
                     rcall  Delay_500ms
                     dec    Cr
                     brne   matp_loop

                     pop    Cr

                     sbrs   State_Reg1,Voice
                     rjmp   toret_pas
beepV_loop:
                     SIREN_ON
                     rcall delay_50ms
                     SIREN_OFF
                     rcall delay_1s
                     dec    Cr
                     brne   beepV_loop
toret_pas:
                     rcall  delay_500ms
                     ret
;----------------------------------------------------------

;----------------------------------------------------------
Message_To_Armed:
                     cbr    State_Reg2,(1<<Dor_C)
                     rcall  Check_Door
                     brcs   skip_sbr
                     sbr    State_Reg2,(1<<Dor_C)
skip_sbr:

                     SPEAKER_ON
                     rcall  Delay_1s
                     rcall  Delay_1s
                     SPEAKER_OFF
                     LED_OFF

                     ldi    Cr,24          ; Выдерка на 30 сек.
beep6_loop:
                     rcall  Short_Speaker  ; С интервалом в 1 сек идут beep'ы
                     rcall  delay_1s

                     sbrc   State_Reg2,Dor_C
                     rjmp   skip_dorcheck  ; Дверь не была открыта после
                                           ; начала постановки на охрану
                     rcall  Check_Door
                     brcs   next_loop_d    ; Дверь была открыта, но еще не закрывалась
                     sbr    State_Reg2,(1<<Dor_C) ; ! Дверь закрылась!
; Факт первого закрытия двери
door1_closed:

                     ldi    Cr,44      ; Даем выдержку времени на 5 сек. Если
check_door_close:                      ; дверь не откроют снова, ставим на охрану
                     rcall  Check_Door ; иначе ждем еще 10 сек на закрытие
                     brcs   open_again ; ! Дверь открыта снова
                     rcall  Delay_100ms
                     dec    Cr
                     brne   check_door_close
                     rjmp   normal_arm ; Дверь закрыта 5 сек. Ставим на охрану
open_again:
                     ldi    Cr,8       ; Ждем опять закрытия двери 10 сек.
                     cbr    State_Reg2,(1<<Dor_C)
                     rjmp   beep6_loop
skip_dorcheck:
                     rcall  Check_Door
                     brcc   next_loop_d
                     cbr    State_Reg2,(1<<Dor_C)
next_loop_d:
                     dec    Cr
                     brne   beep6_loop
arm_timer_end:
                     cbr    State_Reg2,(1<<Dor_C)
normal_arm:
                     ldi    Cr,3
beep7_loop:
                     Light_ON
                     rcall  Delay_500ms
                     Light_OFF
                     rcall  Delay_500ms
                     dec    Cr
                     brne   beep7_loop

                     sbrs   State_Reg1,Voice
                     rjmp   toret_arm
                     ldi    Cr,1
beep8_loop:
                     Siren_ON
                     rcall  Delay_50ms
                     Siren_OFF
                     rcall  Delay_500ms
                     dec    Cr
                     brne   beep8_loop
toret_arm:
                     rcall  Delay_500ms
                     ret
;----------------------------------------------------------

;----------------------------------------------------------
Message_Ser_To_Pass:
                     ldi    Cr,2
stp_loop:
                     rcall  Long_Speaker
                     dec    Cr
                     brne   stp_loop
                     ret
;----------------------------------------------------------

;----------------------------------------------------------
Message_Pass_To_Ser:
                     ldi    Cr,4
pts_loop:
                     rcall  Long_Speaker
                     dec    Cr
                     brne   pts_loop
                     ret
;----------------------------------------------------------

;----------------------------------------------------------
Sensor_Short_Alarm:
                     ldi    Cr,2
beepssa_loop:
                     Siren_ON
                     Light_ON
                     rcall  Delay_1s
                     Siren_OFF
                     Light_OFF
                     rcall  Delay_1s
                     dec    Cr
                     brne   beepssa_loop
                     ret
;----------------------------------------------------------

;----------------------------------------------------------
Short_Speaker:
                     SPEAKER_ON
                     rcall  Delay_50ms
                     SPEAKER_OFF
                     rcall  Delay_100ms
                     ret
;----------------------------------------------------------

;----------------------------------------------------------
Long_Speaker:
                     SPEAKER_ON
                     rcall  Delay_500ms
                     SPEAKER_OFF
                     rcall  Delay_500ms
                     ret
;----------------------------------------------------------

;----------------------------------------------------------
N_Short_Beeps:
                     push   ACC
                     rcall  delay_100ms
                     rcall  Short_Speaker
                     pop    ACC
                     dec    ACC
                     brne   N_Short_Beeps
                     rcall  delay_500ms
                     ret
;----------------------------------------------------------

;----------------------------------------------------------
Message_FC_Mode:
                     rcall  Short_Speaker
                     rcall  Delay_500ms

                     ldi      Cr,3
mfc1_loop:
                     rcall  Short_Speaker
                     dec    Cr
                     brne   mfc1_loop
                     rcall  Delay_500ms
                     ret
;----------------------------------------------------------

