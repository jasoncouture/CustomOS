#pragma once
#include <stdint.h>
#include <stddef.h>

enum PageTableEntryFlag
{
    Present = 0,
    Writable = 1,
    UserAccessible = 2,
    WriteThrough = 3,
    CacheDisabled = 4,
    Accessed = 5,
    LargePages = 7,
    // Bits 9, 10, and 11 will be added in the future, they're for kernel use.
    // Bits 12 -> 52 are the page address (physical address >> 12)
    // The final 12 bits are reserved, except for the last bit, which is used for the NX bit.
    NoExecute = 63
};

struct PageTableEntry {
    uint64_t Data;
    void SetFlag(PageTableEntryFlag flag, bool value);
    bool GetFlag(PageTableEntryFlag flag);
    void SetAddress(uint64_t address);
    uint64_t GetAddress();
};