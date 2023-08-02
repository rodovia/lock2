bits 64

extern KeStartThunk
extern __cxa_finalize

global _prebegin
_prebegin:
    
    call KeStartThunk
    xor rsi, rsi
    call __cxa_finalize
    cli
    hlt
    ret