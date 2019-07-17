; В модуле описаны следующие подпрограммы:
;   Store_To_Buffer       - сохраненяет 8 байт буфера приемника
;                           ( номер карточки ) в буфер обучения
;                           ( адрес в рег. Z ).
;   Test_Card_Number      - сверяет прочитанный номер карточки с
;                           номерами в EEPROM. Возвращает С=1 если
;                           есть совпадение.
;   Verify_Card_N10       - 10 раз пытается распознать карточку,
;                           возвращает C=1 если карточка распознана
;                           и и ее номер имеется в EEPROM.
;
;   Learn_Mode_Enter      - проверяет зажигание, двери и карточку. В
;                           случае выполнения условий входа в режим
;                           обучения выполняет подпрограмму обучения.
;   Check_Ignition        - возвращает C=1 если зажигание включено
;
;   Check_Door            - возвращает C=1 если дверь открыта
;
;   Check_Sensor          - возвращает C=1 если ударный датчик сработал
;
;   Check_Valet           - опрашивает кнопку "Valet". Если кнопка нажата,
;                           выдается короткое звуковое подтверждение и
;                           если кнопка отпускается в течении 1 сек, то
;                           считается что было нажатие ( C=1 )
;   Count_Valet_Press     - считает до 5 количество нажатий кнопки "Valet".
;                           Результат - в регистре ACC:
;                               0 если ничего не нажато;
;                               1..5 - количество нажатий.
;                           Если очередной раз кнопка не нажимается в
;                           течении примерно 1 сек, счет оканчивается.
;   Change1_2Menu         - Подпрограмма "меню" - возвращает следующие
;                           значения флагов S_REG:
;                            С=1, Z=0 - Два нажатия "Valet";
;                            C=0, Z=0 - Одно нажатие "Valet";
;                            Z=1 - если кнопка "Valet" нажата более 2 раз;
;                            T=1 - если закрыты двери (прерывание режимов);
;--------------------------------------------------------------------------

;----------------------------------------------------------
Store_To_Buffer:
                     ldi    Cr,8
                     ldi    YL,data_buffer
                     ldi    YH,0
bcc_loop:
                     ld     ACC,Y+
                     st     Z+,ACC
                     dec    Cr
                     brne   bcc_loop
                     ret
;----------------------------------------------------------

;----------------------------------------------------------
Test_Card_Number:
                     sbrs   Flags,crc_ok         ; Если контрольная сумма
                     rjmp   bad_ret              ; неверная, считаем номер
                     ldi    N_Card,2             ; не распознанным
                     ldi    r16,0
check_again:
                     ldi    YL,data_buffer
                     ldi    YH,0
                     ldi    Cr,8

test_loop:
                     rcall  Read_EEPROM          ; Побайтно сверяем буфер
                     ld     ACC , Y+             ; приема с  номерами в
                     inc    r16                  ; EEPROM
                     cp     r17 , ACC
                     brne   next_card
                     dec    Cr
                     brne   test_loop
                     sec                         ; Номер имеется в базе,
                     rjmp   tc_toret             ; т.е. карточка "своя"
next_card:
                     ldi    r16,8
                     dec    N_Card
                     brne   check_again

bad_ret:             clc                         ; Номер не распознан
                     rjmp   tc_toret             ; или "чужая" карточка
tc_toret:
                     ret
;----------------------------------------------------------

;----------------------------------------------------------
Verify_Card_NXX:
                     push   ACC
ver_loop:
                     push   Cr
                     rcall  Read_Card            ; Читаем карточку
                     rcall  Test_Card_Number     ; Проверяем
                     pop    Cr
                     brcs   Ver_ok
                     dec    Cr
                     brne   ver_loop
                     clc                         ; Карточка "чужая"
                     rjmp   ver_exit             ; или не распознана

Ver_Ok:
                     sec                         ; Карточка "своя"
ver_exit:
                     pop    ACC
                     ret
;----------------------------------------------------------

;----------------------------------------------------------
Verify_Card_N10:
                     ldi Cr, 10
                     rcall  Verify_Card_NXX
                     ret
;----------------------------------------------------------

;----------------------------------------------------------
Check_Ignition:
                     sbic   PIND,Ignition
                     rjmp   ci_not
                     rcall  delay_30ms
                     sbic   PIND,Ignition
                     rjmp   Check_Ignition
                     sec
                     rjmp   ci_exit
ci_not:
                     rcall  delay_30ms
                     sbis   PIND,Ignition
                     rjmp   Check_Ignition
                     clc
ci_exit:
                     ret
;----------------------------------------------------------

;----------------------------------------------------------
Check_Door:
                     sbic   PIND,Door
                     rjmp   do_not
                     rcall  delay_19ms
                     sbic   PIND,Door
                     rjmp   Check_Door
                     sec
                     rjmp   do_exit
do_not:
                     rcall  delay_19ms
                     sbis   PIND,Door
                     rjmp   Check_Door
                     clc
do_exit:
                     ret
;----------------------------------------------------------

;----------------------------------------------------------
Check_Sensor:
                     sbic   PIND,Sensor
                     rjmp   br_not
                     sec
                     rjmp   br_exit
br_not:
                     clc
br_exit:
                     ret
;----------------------------------------------------------


;----------------------------------------------------------
Check_Valet:
                     sbic   PIND,VALET
                     rjmp   cv_clc

                     rcall  Delay_19ms

                     sbic   PIND,VALET
                     rjmp   cv_clc

                     SPEAKER_ON
                     rcall  delay_50ms
                     SPEAKER_OFF
                     ldi    Cr,40
reswdt:
                     dec    Cr
                     rcall  Delay_50ms
                     breq   cv_clc
                     sbis   PIND,VALET
                     rjmp   reswdt
                     rcall  Delay_19ms
                     sbis   PIND,VALET
                     rjmp   reswdt

                     sec
                     rjmp   cv_exit
cv_clc:
                     clc
cv_exit:
                     ret
;----------------------------------------------------------

;----------------------------------------------------------
Check_Valet_Wo:
                     sbic   PIND,VALET
                     rjmp   valw_not

                     rcall  Delay_19ms

                     sbic   PIND,VALET
                     rjmp   Check_Valet_Wo

                     sec
                     rjmp   cvw_exit
valw_not:
                     sbis   PIND,VALET
                     rjmp   Check_Valet_Wo
                     clc
cvw_exit:
                     ret
;----------------------------------------------------------
;----------------------------------------------------------
Count_Valet_Press:
                     ldi    Cr,10
                     ldi    Bit_Cr,0
vc_loop:
                     dec    Cr
                     breq   exit_cvp
                     rcall  delay_100ms
                     rcall  Check_Valet
                     brcc   v_exit
                     inc    Bit_Cr
                     ldi    Cr,10
                     cpi    Bit_Cr,10
                     brne   vc_loop
v_exit:
                     cpi    Bit_Cr,0
                     brne   vc_loop
exit_cvp:
                     mov    ACC,Bit_Cr
                     ret
;----------------------------------------------------------

;----------------------------------------------------------
Change1_2Menu:
                     clt
                     rcall  Check_Door
                     brcc   door_stop
                     rcall  Count_Valet_Press
                     cpi    ACC,0                ; 0 - Nothing pressed
                     breq   Change1_2Menu
                     cpi    ACC,1
                     breq   toclc
                     cpi    ACC,2
                     breq   tosetc
                     rcall  delay_500ms
                     rcall  Long_Speaker
                     rcall  delay_1s
                     sez
                     rjmp   exit_menu

toclc:               clc
                     clz
                     rjmp   exit_menu
tosetc:              sec
                     clz
                     rjmp   exit_menu
door_stop:
                     set
exit_menu:
                     ret
;----------------------------------------------------------
