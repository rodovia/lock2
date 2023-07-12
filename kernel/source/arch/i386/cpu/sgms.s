global SegmsReload
SegmsReload:
    push rdi
    lea rax, [rel .ReloadCs]
    push rax
    retfq
.ReloadCs:
    mov ds, si
    mov ss, si
    mov es, si
    mov gs, si
    mov fs, si
    ret
