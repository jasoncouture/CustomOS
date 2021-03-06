#pragma once
#include <stdint.h>
#include <stddef.h>

// Convert a virtual address to a page table location
class PageTableIndexer {
    public:
        PageTableIndexer(uint64_t virtualAddress);
        uint16_t TopLevelDirectoryPointerIndex;
        uint16_t PageDirectoryIndex;
        uint16_t PageTableIndex;
        uint16_t PageIndex;
};