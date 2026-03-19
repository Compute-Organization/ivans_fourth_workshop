; Booth's Algorithm - Signed 8-bit Multiplication

variableA: 0b00000000
Q: 0b00000011               ; Multiplicando (Q = 3)
Q_1: 0b0
M: 0b00000101               ; Multiplicador (M = 5)
count: 0x8

inicio:
    mov ACC, count
    mov DPTR, ACC
    mov ACC, [DPTR]
    mov A, ACC
    jz end_program

check_case:
    ; Cargar Q[0] en ACC
    mov ACC, Q
    mov DPTR, ACC
    mov ACC, [DPTR]
    mov A, ACC               ; A <- Q

    ; Aislar Q[0] usando AND con 0x01
    mov ACC, 0x01
    and ACC, A               ; ACC <- Q & 0x01 = Q[0]
    mov A, ACC               ; A <- Q[0]

    ; Cargar Q_1
    mov ACC, Q_1
    mov DPTR, ACC
    mov ACC, [DPTR]          ; ACC <- Q_1

    sub ACC, A               ; ACC <- Q_1 - Q[0]
    jz do_shift              ; Si Q[0] == Q_1, solo shift

check_add:
    ; Verificar si Q[0]=0 y Q_1=1 -> sumar M
    mov ACC, Q
    mov DPTR, ACC
    mov ACC, [DPTR]
    mov A, ACC
    mov ACC, 0x01
    and ACC, A               ; ACC <- Q[0]
    jz do_add                ; Si Q[0]=0 -> sumar M

do_sub:
    ; variableA <- variableA - M
    mov ACC, M
    mov DPTR, ACC
    mov ACC, [DPTR]
    mov A, ACC               ; A <- M
    mov ACC, variableA
    mov DPTR, ACC
    mov ACC, [DPTR]          ; ACC <- variableA
    sub ACC, A               ; ACC <- variableA - M
    mov A, ACC
    mov ACC, variableA
    mov DPTR, ACC
    mov ACC, A
    mov [DPTR], ACC
    jmp do_shift

do_add:
    ; variableA <- variableA + M
    mov ACC, M
    mov DPTR, ACC
    mov ACC, [DPTR]
    mov A, ACC               ; A <- M
    mov ACC, variableA
    mov DPTR, ACC
    mov ACC, [DPTR]          ; ACC <- variableA
    add ACC, A               ; ACC <- variableA + M
    mov A, ACC
    mov ACC, variableA
    mov DPTR, ACC
    mov ACC, A
    mov [DPTR], ACC

do_shift:
    ; Guardar Q[0] en Q_1
    mov ACC, Q
    mov DPTR, ACC
    mov ACC, [DPTR]
    mov A, ACC
    mov ACC, 0x01
    and ACC, A               ; ACC <- Q[0]
    mov A, ACC
    mov ACC, Q_1
    mov DPTR, ACC
    mov ACC, A
    mov [DPTR], ACC          ; Q_1 <- Q[0]

    ; Tomar LSB de variableA para insertar en MSB de Q
    mov ACC, variableA
    mov DPTR, ACC
    mov ACC, [DPTR]
    mov A, ACC
    mov ACC, 0x01
    and ACC, A               ; ACC <- LSB de variableA
    sll ACC, 0x07            ; Moverlo al bit 7
    mov A, ACC               ; A <- bit listo para insertar

    ; Shift lógico derecho de Q
    mov ACC, Q
    mov DPTR, ACC
    mov ACC, [DPTR]
    srl ACC, 0x01            ; Q <- Q >> 1
    or ACC, A                ; Insertar MSB desde variableA
    mov A, ACC
    mov ACC, Q
    mov DPTR, ACC
    mov ACC, A
    mov [DPTR], ACC

    ; Shift aritmético derecho de variableA
    mov ACC, variableA
    mov DPTR, ACC
    mov ACC, [DPTR]
    sra ACC, 0x01
    mov A, ACC
    mov ACC, variableA
    mov DPTR, ACC
    mov ACC, A
    mov [DPTR], ACC

decrement_count:
    mov ACC, count
    mov DPTR, ACC
    mov ACC, [DPTR]
    sub ACC, 0x01
    mov A, ACC
    mov ACC, count
    mov DPTR, ACC
    mov ACC, A
    mov [DPTR], ACC

    jmp inicio

end_program:
    hlt