extern _ThreadFinish

global SchSwitchTaskKernel
SchSwitchTaskKernel:
    cli
    mov r8, rdi
    mov rsp, [r8+24]

    mov rbp, rdi
    add rbp, 40

    ; Move all registers to their place
    mov r15, [rbp]
    mov r14, [rbp+8]
    mov r13, [rbp+16]
    mov r12, [rbp+24]
    mov r11, [rbp+32]
    mov r10, [rbp+40]
    mov r9, [rbp+48]
    mov rsi, [rbp+56]
    mov rdi, [rbp+64]
    mov rdx, [rbp+72]
    mov rcx, [rbp+80]
    mov rbx, [rbp+88]
    mov rax, [rbp+96]
    push qword [rbp-40]
    mov rbp, [rbp+104]
    sti
    ret
