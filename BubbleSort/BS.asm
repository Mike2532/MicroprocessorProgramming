;
; BubbleSort.asm
;
; Created: 15.03.2025 1:02:03
; Author : Mike
;


.set ARR_SIZE = 12 
.dseg
arr: .BYTE ARR_SIZE
.cseg

reset: 
    rjmp main 

main: 
    ;в регистровую пару Z загружается адрес src 
    ldi ZH, High(src*2)
    ldi ZL, Low(src*2) 
    ;в регистровую пару Y загружается адрес arr 
    ldi YH, High(arr) 
    ldi YL, Low(arr) 
    ;инициализация итератора
    ldi R16, ARR_SIZE 

copy:
    lpm R18, Z+
    st Y+, R18 
    out OCR0A, R18
    dec R16 
    brne copy 
    ldi YH, High(arr) 
    ldi YL, Low(arr) 

while: 
    ldi R17, 0x00; флаг окончания цикла 
    ldi R16, 2; итератор для for 

for: 
    ld R18, Y+; v11
    ld R19, Y+; v12 
    ld R20, Y+; v13 
    ld R21, Y+; v14 
    ld R22, Y+; v21 
    ld R23, Y+; v22 
    ld R24, Y+; v23 
    ld R25, Y; v24 
    sbiw YL, 3

ComMod1: 
    cp R21, R25 
    breq ComMod2;   случай равенства байт 
    brlt  iteration; случай первого long меньше второго (переход iteration)  
    rjmp replace;   случай первого long больше второго (переход replace)

ComMod2:
    cp R20, R24 
    breq ComMod3;   случай равенства байт 
    brlt  iteration; случай первого long меньше второго (переход iteration)  
    rjmp replace;   случай первого long больше второго (переход replace)

ComMod3:
    cp R19, R23 
    breq ComMod4;   случай равенства байт 
    brlt  iteration; случай первого long меньше второго (переход iteration)  
    rjmp replace;   случай первого long больше второго (переход replace)

ComMod4:
    cp R18, R22 
    breq iteration;   случай равенства байт 
    brlt  iteration; случай первого long меньше второго (переход iteration)  
    rjmp replace;   случай первого long больше второго (переход replace)

replace: 
    ldi R17, 0x11
    sbiw YL, 4
    st Y+, R22 
    st Y+, R23 
    st Y+, R24 
    st Y+, R25 
    st Y+, R18 
    st Y+, R19 
    st Y+, R20 
    st Y,  R21 
    sbiw YL, 3 

iteration: 
    dec R16 
    brne for 
    ldi YH, High(arr) 
    ldi YL, Low(arr) 
    cpi R17, 0x11
    breq while 

outputlog: 
	ldi YH, High(arr) 
    ldi YL, Low(arr) 
	ldi R16, 0x07
	
log:
	ld R18, Y+
	out OCR0B, R18
	dec R16
	brne log

loop: 
    nop 
    rjmp loop

src: .dw 0x2C9B, 0x3F7A, 0xD08A, 0xB4E1, 0xF72E, 0x5CD3