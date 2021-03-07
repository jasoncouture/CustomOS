[bits 64]
# Thanks to https://github.com/Absurdponcho/PonchoOS - I hate this code, I stole almost all of it, other than changing it to suit my liking.
# Makefile, EFI libs, and some of the EFI boot loader also stolen from him too.
# As a side note, I hate GDT and IDT, I'm glad it's one of the last remaining remnants of legacy x86, long live UEFI.

LoadGlobalDescriptorTable:
    lgdt [rdi]
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    
    pop rdi
    mov rax, 0x08
    push rax
    push rdi
    retfq

GLOBAL LoadGlobalDescriptorTable