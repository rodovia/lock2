extern IdtGenericHandler

%macro _Enter 0
    push rax
    push rbx
    push rcx
    push rdx
    push rdi
    push rsi
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15
%endmacro

%macro _Leave 0
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop rsi
    pop rdi
    pop rdx
    pop rcx
    pop rbx
    pop rax
%endmacro

%macro IdtGenericHandlerPrologue 1
IdtGenericHandler%+%1:
    push rbp
    _Enter
    mov rdi, rbp
    mov rsi, %1
    xor rdx, rdx
    mov rcx, rsp
    cld
    call IdtGenericHandler
    _Leave
    pop rbp
    iretq
%endmacro

%macro IdtExceptionHandlerPrologue 1
IdtGenericHandler%+%1:
    push rbp
    mov rbp, rsp
    add rbp, 8
    _Enter

    mov rdi, rbp
    mov rsi, %1
    mov rdx, [rbp] ; Error code
    mov rcx, rsp   ; Register state
    add rdi, 8
    cld
    call IdtGenericHandler

    _Leave
    pop rbp
    add rsp, 8 ; Discard error code
    iretq
%endmacro

IdtGenericHandlerPrologue 0
IdtGenericHandlerPrologue 1
IdtGenericHandlerPrologue 2
IdtGenericHandlerPrologue 3
IdtGenericHandlerPrologue 4
IdtGenericHandlerPrologue 5
IdtGenericHandlerPrologue 6
IdtGenericHandlerPrologue 7
IdtExceptionHandlerPrologue 8
IdtGenericHandlerPrologue 9
IdtExceptionHandlerPrologue 10
IdtExceptionHandlerPrologue 11
IdtExceptionHandlerPrologue 12
IdtExceptionHandlerPrologue 13
IdtExceptionHandlerPrologue 14
IdtGenericHandlerPrologue 15
IdtGenericHandlerPrologue 16
IdtExceptionHandlerPrologue 17
IdtGenericHandlerPrologue 18
IdtGenericHandlerPrologue 19
IdtGenericHandlerPrologue 20
IdtExceptionHandlerPrologue 21
IdtGenericHandlerPrologue 22
IdtGenericHandlerPrologue 23
IdtGenericHandlerPrologue 24
IdtGenericHandlerPrologue 25
IdtGenericHandlerPrologue 26
IdtGenericHandlerPrologue 27
IdtGenericHandlerPrologue 28
IdtExceptionHandlerPrologue 29
IdtExceptionHandlerPrologue 30
IdtGenericHandlerPrologue 31
IdtGenericHandlerPrologue 32
IdtGenericHandlerPrologue 33
IdtGenericHandlerPrologue 34
IdtGenericHandlerPrologue 35
IdtGenericHandlerPrologue 36

global idtTable
idtTable:
%assign i 0
%rep 36
    dq IdtGenericHandler%+i
%assign i i + 1
%endrep
