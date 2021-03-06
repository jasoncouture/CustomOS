[bits 64]
EXTERN DisableInterrupts
EXTERN EnableInterrupts
EXTERN InterruptStatus

DisableInterrupts:
    cli
    nop
    ret

EnableInterrupts:
    sti
    nop
    ret

InterruptStatus:
    pushfq
    pop rax
    and rax, 0x0200
    jnz .InterruptsEnabled
    xor rax, rax
    ret
    .InterruptsEnabled:
    mov rax, 1
    ret