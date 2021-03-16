#pragma once
#include <stdint.h>

// The packed and aligned attributes are important here.
// the GDT must be page aligned, and start at the beginning of a page.
// And since these represent hardware data structures, we must not allow
// the compiler to pad these structs (A good example is the compiler will insert 48 bits into GlobalDescriptorLocation to align it to the system pointer size)
// This will break, since the processor is expecting it exactly as the code is written here.

struct GlobalDesciptorTableEntry {
    uint16_t LowerLimit;
    uint16_t LowerBase;
    uint8_t MidBase;
    uint8_t Access;
    uint8_t HighLimitAndFlags;
    uint8_t HighBase;
} __attribute__((packed));


struct GlobalDesciptorTable {
    GlobalDesciptorTableEntry Null; // first entry must be all zeros.
    GlobalDesciptorTableEntry KernelCodeSegment;
    GlobalDesciptorTableEntry KernelDataSegment;
    GlobalDesciptorTableEntry UserNullSegment;
    GlobalDesciptorTableEntry UserCodeSegment;
    GlobalDesciptorTableEntry UserDataSegment;
} 
__attribute__((packed));

struct GlobalDescriptorLocation {
    uint16_t Size;
    struct GlobalDesciptorTable* GlobalDescriptorTable;
} __attribute__((packed));

extern GlobalDesciptorTable DefaultGlobalDesciptorTable;

extern "C" void LoadGlobalDescriptorTable(GlobalDescriptorLocation* globalDescriptorLocation);