; Spinlock implementation

SlLockAcquire:
    lock bts qword [rdi], 0
    jc .spin
    ret
.spin:
    pause
    test qword [rdi], 1
    jnz .spin
    jmp SlLockAcquire

SlLockRelease:
    mov qword [rdi], 0
    ret
