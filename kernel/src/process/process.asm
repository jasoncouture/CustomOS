[bits 64]

GLOBAL InitializeFloatingPointUnit

InitializeFloatingPointUnit:
    call initialize_fpu
    call initialize_sse
    ;call initialize_avx
    ret

initialize_fpu:
    fninit
    ret

initialize_sse:
    push rax
    mov rax, cr0
    and ax, 0xFFFB		;clear coprocessor emulation CR0.EM
    or ax, 0x2			;set coprocessor monitoring  CR0.MP
    mov cr0, rax
    mov rax, cr4
    or ax, 3 << 9		;set CR4.OSFXSR and CR4.OSXMMEXCPT at the same time
    mov cr4, rax
    pop rax
    ret

initialize_avx:
    push rax
    push rcx
    push rdx
 
    xor rcx, rcx
    xgetbv ;Load XCR0 register
    or eax, 7 ;Set AVX, SSE, X87 bits
    xsetbv ;Save back to XCR0
 
    pop rdx
    pop rcx
    pop rax
    ret