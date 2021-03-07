#include "idt.hpp"

void InterruptDescriptorTableEntry::SetOffset(uint64_t offset)
{
    Offset0 = (uint16_t)(offset & 0x000000000000ffff);
    Offset1 = (uint16_t)((offset & 0x00000000ffff0000) >> 16);
    Offset2 = (uint32_t)((offset & 0xffffffff00000000) >> 32);
}

uint64_t InterruptDescriptorTableEntry::GetOffset()
{
    uint64_t offset = 0;
    
    offset |= (uint64_t)Offset0;
    offset |= (uint64_t)Offset1 << 16;
    offset |= (uint64_t)Offset2 << 32;

    return offset;
}