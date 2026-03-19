;Addition Operation Using Bitwise Operators.

varX: 0x02
varY: 0x03
varC: 0x00

while_start:            @ Loop until Y value comes back to zero from X range of counting.
    
    ldY_condition:      @ Bring the value of Y through the auxiliary.
        MOV ACC, CTE    @ Load address of variable Y in the accumulator.
        varY            @ Address of variable Y.
        MOV DPTR, ACC   @ Move the address to the data pointer.
        MOV ACC, [DPTR] @ Load the value from Y address to the accumulator.
        MOV A, ACC      @ A <- ACC.

    check_end:          @ End the loop through the condition specified.
        JZ CTE          @ If y == 0, exit the loop.
        0x05            @ Jump to the ending program scope.

    ldX_and:            @ Bring the value of X into the accumulator.
        MOV ACC, CTE    @ Load address of variable X in the accumulator.
        varX            @ Address of variable X.
        MOV DPTR, ACC   @ Move the address to the data pointer.
        MOV ACC, [DPTR] @ Load the value from Y address to the accumulator.

    andXY:              @ Compute AND logic gate.
        AND ACC, A      @ ACC <- X % Y 
        MOV A, ACC      @ Load the result into the auxiliary.

    store_carry:        @ Save result on the Carry variable (varC).
        MOV ACC, CTE    @ Load address of variable C in the accumulator.
        varC            @ Address of variable C.
        MOV DPTR, ACC   @ Move the address to the data pointer.
        MOV ACC, A      @ Load the result into the accumulator.
        MOV [DPTR], ACC @ Store carry in varC.

    ldY_xor:            @ Bring the value of Y through the auxiliary.
        MOV ACC, CTE    @ Load address of variable Y in the accumulator.
        varY            @ Address of variable Y.
        MOV DPTR, ACC   @ Move the address to the data pointer.
        MOV ACC, [DPTR] @ Load the value from Y address to the accumulator.
        MOV A, ACC      @ Move the accumulator to the auxiliary.

    ldX_xor:            @ Reload X into ACC to compute the partial sum X ^ Y.
        MOV ACC, CTE    @ Load the memory address of variable X into ACC.
        varX            @ Operand: address assigned to variable X.
        MOV DPTR, ACC   @ Copy the address from ACC into DPTR.
        MOV ACC, [DPTR] @ Read the value stored at X's address into ACC.

    xorXY:              @ Compute X ^ Y, which is the partial sum without carry.
        XOR ACC, A      @ ACC <- X ^ Y.
        MOV A, ACC      @ Copy the XOR result into A for temporary storage.

    storeX:             @ Store the updated X value back into varX.
        MOV ACC, CTE    @ Load the memory address of variable X into ACC.
        varX            @ Operand: address assigned to variable X.
        MOV DPTR, ACC   @ Copy the address from ACC into DPTR.
        MOV ACC, A      @ Move the XOR result from A into ACC.
        MOV [DPTR], ACC @ Store the updated X value into varX.

    ld1_shift:          @ Load the shift amount 1 into A.
        MOV ACC, CTE    @ Load the immediate constant into ACC.
        0x01            @ Operand: constant value 1.
        MOV A, ACC      @ Copy the shift amount into A.

    ldC_shift:          @ Bring the value of C (carry) through the auxiliary.
        MOV ACC, CTE    @ Load address of variable Y in the accumulator.
        varC            @ Address of variable C.
        MOV DPTR, ACC   @ Move the address to the data pointer.
        MOV ACC, [DPTR] @ Load the value from C address to the accumulator.

    lshC:               @ Compute the new Y value as carry shifted left by one bit.
        SLL ACC, A      @ ACC <- varC << 1, using A = 1 as shift amount.
        MOV A, ACC      @ Copy the shifted result into A for temporary storage.

    storeY:             @ Store the new Y value back into varY.
        MOV ACC, CTE    @ Load the memory address of variable Y into ACC.
        varY            @ Operand: address assigned to variable Y.
        MOV DPTR, ACC   @ Copy the address from ACC into DPTR.
        MOV ACC, A      @ Move the shifted carry result into ACC.
        MOV [DPTR], ACC @ Store the updated Y value into varY.

    continueL:          @ Unconditional jump back to the beginning of the loop.
        JMP CTE         @ Jump to the loop start address.
        0x00            @ Jump target operand for JMP CTE (current encoded while_start address).

end_program:            @ Program termination block.
    RET                 @ Stop execution and return control to the simulator.

