[bits 64]

GLOBAL ProcessStartTrampoline

ProcessStartTrampoline:
    mov rax, rdi
    call rax
    xor rdi, rdi ; syscall 0, ProcessExit
    mov rsi, rax ; sys$ProcessExit(RAX);
    int 80h       ; This will trigger process teardown.