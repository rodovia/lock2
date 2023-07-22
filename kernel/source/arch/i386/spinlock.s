; Spinlock implementation

global SlLockAcquire
SlLockAcquire:
    cli
    lock bts qword [rdi], 0
    jc .spin
    sti
    ret
.spin:
    pause
    test qword [rdi], 1
    jnz .spin
    jmp SlLockAcquire

global SlLockRelease
SlLockRelease:
    mov qword [rdi], 0
    ret
