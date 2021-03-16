#include <memory/paging/pagetableindexer.hpp>

PageTableIndexer::PageTableIndexer(uint64_t virtualAddress) {
    uint64_t virtualPage = virtualAddress >> 12;
    this->PageIndex = virtualPage & 0x1ff;
    this->PageTableIndex = (virtualPage >> 9 ) & 0x1ff;
    this->PageDirectoryIndex = (virtualPage >> 18) & 0x1ff;
    this->TopLevelDirectoryPointerIndex = (virtualPage >> 27) & 0x1ff;
}