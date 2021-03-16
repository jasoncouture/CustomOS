#include <memory/paging/pagetableentry.hpp>

#define ADDRESS_MASK 0x000ffffffff000
#define FLAGS_MASK 0xfff00000000fff

void PageTableEntry::SetFlag(PageTableEntryFlag flag, bool value) {
    uint64_t bitSelector = (uint64_t)1 << flag;
    if(value) // Set the bit
        Data |= bitSelector;
    else // Clear the bit
        Data &= ~bitSelector;
}

bool PageTableEntry::GetFlag(PageTableEntryFlag flag) {
    uint64_t bitSelector = (uint64_t)1 << flag;
    return (bool)(Data & bitSelector);
}

uint64_t PageTableEntry::GetAddress() {
    return (Data & ADDRESS_MASK);
}

void* PageTableEntry::GetAddressPointer() {
    return (void*)(GetAddress());
}

void PageTableEntry::SetAddress(uint64_t address)
{
    address &= ADDRESS_MASK;  //mask it
    Data &= FLAGS_MASK; // Clear the existing address, if set.
    Data |= address; // OR the address into the entry data.
}