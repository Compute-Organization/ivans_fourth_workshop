; Ejemplo de sumador
a: 0b110
e: 0b110
i: 0b110
numero1: 0b11 ;3
numero2: 0b110;6

inicio:
	; Se carga el numero1 a A
	mov ACC, numero1
	mov DPTR, ACC
	mov ACC, [DPTR]
	mov A, ACC
    ; Se carga el numero2 a ACC
	mov ACC, numero2
    mov DPTR, ACC
	mov ACC, [DPTR]
    ; Se suman
    add ACC, A
    mov A, ACC
    hlt ; Se detiene la ejecucion