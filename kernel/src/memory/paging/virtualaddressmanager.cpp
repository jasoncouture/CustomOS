#include "virtualaddressmanager.hpp"
#include "pagetableindexer.hpp"
#include "../memory.hpp"
#include "../pageallocator.hpp"

VirtualAddressManager KernelVirtualAddressManager = 0;

VirtualAddressManager::VirtualAddressManager()
{
    auto pageAllocator = PageAllocator::GetInstance();
    // The first REAL use of AllocatePage! We've finally allocated memory for something! :)
    void *rootEntryPage = pageAllocator->AllocatePage();
    memset(rootEntryPage, 0, pageAllocator->PageSize());
    this->RootTable = (PageTableEntry *)rootEntryPage;
    this->FreeOnDestroy = true;
}

VirtualAddressManager::VirtualAddressManager(PageTableEntry rootTable[512], bool freeOnDestory)
{
    this->RootTable = rootTable;
    this->FreeOnDestroy = freeOnDestory;
}

void VirtualAddressManager::Map(void *virtualAddress, void *physicalAddress)
{
    auto pageAllocator = PageAllocator::GetInstance();
    auto pageSize = pageAllocator->PageSize();

    uint64_t virtualPage = (uint64_t)virtualAddress >> 12;
    auto PageIndex = virtualPage & 0x1ff;
    auto PageTableIndex = (virtualPage >> 9 ) & 0x1ff;
    auto PageDirectoryIndex = (virtualPage >> 18) & 0x1ff;
    auto TopLevelDirectoryPointerIndex = (virtualPage >> 27) & 0x1ff;
    
    PageTableEntry *previousTable = this->RootTable;
    PageTableEntry pageDirectoryEntry = previousTable[TopLevelDirectoryPointerIndex]; //this->RootTable[indexer.TopLevelDirectoryPointerIndex];
    if (!pageDirectoryEntry.GetFlag(PageTableEntryFlag::Present))
    {
        PageTableEntry *pageDirectoryPointer = (PageTableEntry *)pageAllocator->AllocatePage();
        memset(pageDirectoryPointer, 0, pageSize);
        pageDirectoryEntry.SetAddress((uint64_t)pageDirectoryPointer);
        pageDirectoryEntry.SetFlag(PageTableEntryFlag::Writable, true);
        pageDirectoryEntry.SetFlag(PageTableEntryFlag::Present, true);
        previousTable[TopLevelDirectoryPointerIndex] = pageDirectoryEntry;
    }

    previousTable = (PageTableEntry *)pageDirectoryEntry.GetAddressPointer();

    pageDirectoryEntry = previousTable[PageDirectoryIndex];
    if (!pageDirectoryEntry.GetFlag(PageTableEntryFlag::Present))
    {
        PageTableEntry *pageDirectoryPointer = (PageTableEntry *)pageAllocator->AllocatePage();
        memset(pageDirectoryPointer, 0, pageSize);
        pageDirectoryEntry.SetAddress((uint64_t)pageDirectoryPointer);
        pageDirectoryEntry.SetFlag(PageTableEntryFlag::Writable, true);
        pageDirectoryEntry.SetFlag(PageTableEntryFlag::Present, true);
        previousTable[PageDirectoryIndex] = pageDirectoryEntry;
    }

    previousTable = (PageTableEntry *)pageDirectoryEntry.GetAddressPointer();

    pageDirectoryEntry = previousTable[PageTableIndex]; //this->RootTable[indexer.TopLevelDirectoryPointerIndex];
    if (!pageDirectoryEntry.GetFlag(PageTableEntryFlag::Present))
    {
        PageTableEntry *pageDirectoryPointer = (PageTableEntry *)pageAllocator->AllocatePage();
        memset(pageDirectoryPointer, 0, pageSize);
        pageDirectoryEntry.SetAddress((uint64_t)pageDirectoryPointer);
        pageDirectoryEntry.SetFlag(PageTableEntryFlag::Writable, true);
        pageDirectoryEntry.SetFlag(PageTableEntryFlag::Present, true);
        previousTable[PageTableIndex] = pageDirectoryEntry;
    }

    previousTable = (PageTableEntry *)pageDirectoryEntry.GetAddressPointer();

    pageDirectoryEntry = previousTable[PageIndex]; //this->RootTable[indexer.TopLevelDirectoryPointerIndex];
    pageDirectoryEntry.SetAddress((uint64_t)physicalAddress);
    pageDirectoryEntry.SetFlag(PageTableEntryFlag::Writable, true);
    pageDirectoryEntry.SetFlag(PageTableEntryFlag::Present, true);
    previousTable[PageIndex] = pageDirectoryEntry;
}

void VirtualAddressManager::Activate()
{
    // Move the address of our root table into CR3
    // This will cause the MMU on the CPU to use it.
    asm ("mov %0, %%cr3" : : "r" (this->RootTable) );
}

// VirtualAddressManager::~VirtualAddressManager()
// {
//     if (this->FreeOnDestroy)
//     {
//         auto pageAllocator = PageAllocator::GetInstance();
//         pageAllocator->FreePage(this->RootTable);
//     }
// }