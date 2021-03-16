#pragma once
#include <stdint.h>
#define IDT_ATTRIBUTE_PRESENT 0b10000000
#define IDT_ATTRIBUTE_PRIVILIGED 0b00000000

#define IDT_TYPE_GATE_INTERRUPT 0b1110
#define IDT_TYPE_GATE_CALL 0b1100
#define IDT_TYPE_GATE_TRAP 0b1111



struct InterruptDescriptorTableEntry {
    uint16_t Offset0;
    uint16_t Selector;
    uint8_t IST;
    uint8_t TypeAndAttribute;
    uint16_t Offset1;
    uint32_t Offset2;
    uint32_t Unused;

    void SetOffset(uint64_t offset);
    uint64_t GetOffset();
} __attribute__((packed));


struct InterruptDesciptorTableLocation {
    uint16_t Limit;
    InterruptDescriptorTableEntry* InterruptDesciptors;
} __attribute__((packed));
