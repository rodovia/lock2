global SegmsReload
SegmsReload:
    push rdi
    lea rax, [rel ._ReloadCs]
    push rax
    retfq
._ReloadCs:
    mov ds, si
    mov ss, si
    mov es, si
    mov gs, si
    mov fs, si
    mov ax, 0x38
    ltr ax
    ret
