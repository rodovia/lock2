global TscRead
TscRead:
    rdtsc
    shl rdx, 32
    mov edx, eax
    mov rax, rdx
    ret
