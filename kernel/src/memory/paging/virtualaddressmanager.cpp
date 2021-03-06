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
    PageTableIndexer indexer = PageTableIndexer((uint64_t)virtualAddress); // Create the indexes we need from the virtual address pointer.
    PageTableEntry *previousTable = this->RootTable;
    PageTableEntry pageDirectoryEntry = previousTable[indexer.TopLevelDirectoryPointerIndex]; //this->RootTable[indexer.TopLevelDirectoryPointerIndex];
    if (!pageDirectoryEntry.GetFlag(PageTableEntryFlag::Present))
    {
        PageTableEntry *pageDirectoryPointer = (PageTableEntry *)pageAllocator->AllocatePage();
        memset(pageDirectoryPointer, 0, pageSize);
        pageDirectoryEntry.SetAddress((uint64_t)pageDirectoryPointer);
        pageDirectoryEntry.SetFlag(PageTableEntryFlag::Writable, true);
        pageDirectoryEntry.SetFlag(PageTableEntryFlag::Present, true);
        previousTable[indexer.TopLevelDirectoryPointerIndex] = pageDirectoryEntry;
    }

    previousTable = (PageTableEntry *)pageDirectoryEntry.GetAddressPointer();

    pageDirectoryEntry = previousTable[indexer.PageDirectoryIndex]; //this->RootTable[indexer.TopLevelDirectoryPointerIndex];
    if (!pageDirectoryEntry.GetFlag(PageTableEntryFlag::Present))
    {
        PageTableEntry *pageDirectoryPointer = (PageTableEntry *)pageAllocator->AllocatePage();
        memset(pageDirectoryPointer, 0, pageSize);
        pageDirectoryEntry.SetAddress((uint64_t)pageDirectoryPointer);
        pageDirectoryEntry.SetFlag(PageTableEntryFlag::Writable, true);
        pageDirectoryEntry.SetFlag(PageTableEntryFlag::Present, true);
        previousTable[indexer.TopLevelDirectoryPointerIndex] = pageDirectoryEntry;
    }

    previousTable = (PageTableEntry *)pageDirectoryEntry.GetAddressPointer();

    pageDirectoryEntry = previousTable[indexer.PageTableIndex]; //this->RootTable[indexer.TopLevelDirectoryPointerIndex];
    if (!pageDirectoryEntry.GetFlag(PageTableEntryFlag::Present))
    {
        PageTableEntry *pageDirectoryPointer = (PageTableEntry *)pageAllocator->AllocatePage();
        memset(pageDirectoryPointer, 0, pageSize);
        pageDirectoryEntry.SetAddress((uint64_t)pageDirectoryPointer);
        pageDirectoryEntry.SetFlag(PageTableEntryFlag::Writable, true);
        pageDirectoryEntry.SetFlag(PageTableEntryFlag::Present, true);
        previousTable[indexer.TopLevelDirectoryPointerIndex] = pageDirectoryEntry;
    }

    previousTable = (PageTableEntry *)pageDirectoryEntry.GetAddressPointer();

    pageDirectoryEntry = previousTable[indexer.PageIndex]; //this->RootTable[indexer.TopLevelDirectoryPointerIndex];
    pageDirectoryEntry.SetAddress((uint64_t)physicalAddress);
    pageDirectoryEntry.SetFlag(PageTableEntryFlag::Writable, true);
    pageDirectoryEntry.SetFlag(PageTableEntryFlag::Present, true);
    previousTable[indexer.PageIndex] = pageDirectoryEntry;
}

void VirtualAddressManager::Activate()
{
    // TODO: Activate by giving cr3 the address of RootTable
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