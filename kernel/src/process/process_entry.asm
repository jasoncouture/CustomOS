[bits 64]

GLOBAL ProcessStartTrampoline

ProcessStartTrampoline:
    mov rax, rdi
    call rax
    mov rdi, rax    ; Pass return code to the first parameter of the syscall
    mov rax, 60     ; syscall 60, ProcessExit
    int 80h         ; This will trigger process teardown.