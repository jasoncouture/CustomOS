#pragma once
#include <stdint.h>

#define IDT_TYPEATTRIBUTE_INTERRUPTGATE 0b10001110
#define IDT_TYPEATTRIBUTE_CALLGATE      0b10001100
#define IDT_TYPEATTRIBUTE_TRAPGATE      0b10001111



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
