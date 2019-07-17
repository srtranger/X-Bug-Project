;-----------------------------------------------------------
.MACRO               SPEAKER_ON
                     sbi    PORTD ,   PD1
.ENDMACRO

.MACRO               SPEAKER_OFF
                     cbi    PORTD ,   PD1
.ENDMACRO
;-----------------------------------------------------------

;-----------------------------------------------------------
.MACRO               LIGHT_ON
                     sbi    PORTD ,   PD0
.ENDMACRO

.MACRO               LIGHT_OFF
                     cbi    PORTD ,   PD0
.ENDMACRO
;-----------------------------------------------------------

;-----------------------------------------------------------
.MACRO               SIREN_ON
                     sbi    PORTD ,   PD2
.ENDMACRO

.MACRO               SIREN_OFF
                     cbi    PORTD ,   PD2
.ENDMACRO
;-----------------------------------------------------------

;-----------------------------------------------------------
.MACRO               BLOCK_ON
                     sbi    PORTB ,   PB4
.ENDMACRO

.MACRO               BLOCK_OFF
                     cbi    PORTB ,   PB4
.ENDMACRO
;-----------------------------------------------------------

;-----------------------------------------------------------
.MACRO               LED_ON
                     cbi    PORTB ,   PB5        ;(PIN17)
.ENDMACRO

.MACRO               LED_OFF
                     sbi    PORTB ,   PB5
.ENDMACRO
;-----------------------------------------------------------

;-----------------------------------------------------------
.MACRO               POWER_RFM_ON
                     cbi    PORTB ,   PB3        ;( PIN15 )
.ENDMACRO

.MACRO               POWER_RFM_OFF
                     sbi    PORTB ,   PB3
.ENDMACRO
;-----------------------------------------------------------
