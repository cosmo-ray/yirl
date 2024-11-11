.segment "HEADER"
.byte "YIRL 0"

.segment "DATA"
hello:
.byte "hello world"

.segment "RAM"
gamestate:	.res 1  ; .rs 1 means reserve one byte of space
gamestate2:	.res 1  ; .rs 1 means reserve one byte of space


.segment "CODE"
	LDA #$66
	LDX #$40
	STA gamestate
	STA gamestate2
	LDA hello
	STA $FF00
	LDX #10
	LDY #30
	STA $FC00
