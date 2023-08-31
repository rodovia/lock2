global SegmsReload
SegmsReload:
    mov rax, rsp
    push rsi
    push rax
    pushf
    push rdi
    push _ReloadCs
    iretq

_ReloadCs:
    mov ds, si
    mov ss, si
    mov es, si
    mov gs, si
    mov fs, si
    ret
