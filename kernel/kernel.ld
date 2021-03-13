OUTPUT_FORMAT(elf64-x86-64)
ENTRY(__entry)
SECTIONS
{
    . = 0x10000;
    _kernelStart = .;
    .text : ALIGN(4K)
    {
        *(.text)
    }
    .data : ALIGN(4K)
    {
        *(.data)
    }
    .rodata : ALIGN(4K)
    {
        *(.rodata)
    }
    .bss : ALIGN(4K)
    {
        *(COMMON)
        *(.bss)
    }
    _kernelEnd = .;
}