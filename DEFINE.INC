; В модуле задаются адреса переменных, описаны константы.

;-------------- Number of Blocks ---------------------------
.equ                 N_BLOCKS = 5           ; Число временных
.equ                 Alarm_Time = 60        ; блокировок
;-----------------------------------------------------------

.equ             Armed_Delay_S =  16        ; Value for delay
                                            ;  (7 sec)
.equ             Armed_Delay_L =  33        ; Value for delay
                                            ; (14 sec)


.def             S_Timer_H   =  r1
.def             S_Timer_L   =  r2
.def             Sens_Alarms =  r3
.def             Temp        =  r4
.def             ACC         =  r18
.def             Counter     =  r19
.def             BCC_L       =  r20
.def             BCC_H       =  r21
.def             Bit_Cr      =  r22
.def             N_Read      =  r23
.def             Cr          =  r23
.def             N_Card      =  r24
.def             Flags       =  r25
.def             Blocks_Cr   =  r26
.def             State_Reg1  =  r30       ; This 2 registers reading from
.def             State_Reg2  =  r31       ; EEPROM and containing
                                          ; most important Safe information

.equ             stored_rxdt =  0        ; temporary 1-bit storage
.equ             crc_ok      =  1        ; card read succ. flag

;--------------------  State_Reg1 bits  -----------------

.equ             Alarm      =  0         ; 1 - сработала сигнализация
.equ             Ar_En      =  1         ; 0 - запрет постановки на охрану
.equ             F_Anti     =  2         ;
.equ             Dot_S      =  3         ; 1 - прерывистый сигнал на сирену
                                         ; 0 - непрерывный сигнал на сирену
.equ             Voice      =  4         ; 1 - постановка на охрану со звуком сирены
                                         ; 0 - постановка на охрану без звука сирены
.equ             Serv       =  5         ; 1 - Режим сервиса (Service mode)
.equ             Armed      =  6         ; 1 - Режим охраны  (Armed mode)
.equ             Passiv     =  7         ; 1 - Режим Passive mode

; ┌───────┬───────┬───────┬───────┬───────┬───────┬───────┬───────┐
; │   7   │   6   │   5   │   4   │   3   │   2   │   1   │   0   │
; ├───────┼───────┼───────┼───────┼───────┼───────┼───────┼───────┤
; │Passiv │ Armed │ Serv  │ Voice │ Dot_S │ F_anti│ Ar_En │ Alarm │
; └───────┴───────┴───────┴───────┴───────┴───────┴───────┴───────┘
;Def: 0       0       1       0       0       0       1       0
.equ             STR1          =  $82

;--------------------  State_Reg2 bits  -----------------

.equ             Dor_C      =  0     ; 1 - дверь была закрыта
.equ             Arm_T      =  1     ; 1 - выдержка на снятие с охраны 15 сек.
                                     ; 0 - выдержка на снятие с охраны 7 сек.
.equ             Hi_Jack    =  2     ; 1 - включения сигнализации по последней
                                     ; блокировке
.equ             Al_Sen     =  3     ; 1 - сигнализация по датчику
.equ             Al_Do      =  4     ; 1 - сигнализация по двери
.equ             Al_Ig      =  5     ; 1 - сигнализация по зажиганию



; ┌───────┬───────┬───────┬───────┬───────┬───────┬───────┬───────┐
; │   7   │   6   │   5   │   4   │   3   │   2   │   1   │   0   │
; ├───────┼───────┼───────┼───────┼───────┼───────┼───────┼───────┤
; │       │       │ Al_Ig │ Al_Do │Al_Sen │Hi_Jack│ Arm_T │ Dor_C │
; └───────┴───────┴───────┴───────┴───────┴───────┴───────┴───────┘
;Def: 0       0       0       0       0       1       1       0
.equ             STR2          =  $04

;--------------------  State_Reg3   -----------------
; Хранит счетчик блокировок 0 - полная блокировка

;Def 5
.equ             BLCR          =  $05
; --------------- Equal input signals to port pins --------------
.equ             Sensor      =  PD3      ; Вход от ударного датчика
.equ             Ignition    =  PD4      ; Вход от зажигания
.equ             Door        =  PD5      ; Вход от дверей
.equ             Valet       =  PD6      ; Вход кнопки "Valet"
.equ             RXCK        =  PB2      ; Sync. input
.equ             RXDT        =  PB1      ; Data  input
.equ             TXCT        =  PB0      ; Выход управления передатчиком
.equ             CS_RFM      =  PB3      ; Выход управления питанием RFM


;-------------------- Variables  ---------------------------
.equ                 Begin_Var    = $60

.equ             data_buffer  = Begin_Var    ; Receive buffer
.equ             rec_data     = data_buffer  ; Data ( 8 bytes )
.equ             rec_bcc      = rec_data+8   ; BCC  ( 2 bytes )
.equ             rec_stop     = rec_bcc+2    ; Stop Bits ( 1 byte )
.equ             rec_end      = rec_stop+1   ; End  Bits ( 1 byte )

.equ             Temp_buffer  = $80          ; Temporary buffer 16 bytes


