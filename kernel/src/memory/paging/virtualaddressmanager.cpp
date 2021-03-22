#include <stdint.h>
#include <stddef.h>
#include <memory/paging/virtualaddressmanager.hpp>
#include <memory/paging/pagetableindexer.hpp>
#include <memory/memory.hpp>
#include <memory/pageallocator.hpp>
#include <console/font.hpp>
#include <console/cstr.hpp>
#include <graphics/framebuffer.hpp>
#include <panic.hpp>

// Class members

VirtualAddressManager *VirtualAddressManager::KernelVirtualAddressManager = NULL;
bool VirtualAddressManager::IsInitialized = false;

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

void *VirtualAddressManager::GetPageTableAddress()
{
    return (void *)this->RootTable;
}

PageTableEntry *VirtualAddressManager::GetPageTableEntry(void *virtualAddress, bool create)
{
    auto pageAllocator = PageAllocator::GetInstance();
    auto pageSize = pageAllocator->PageSize();

    uint64_t virtualPage = ((uint64_t)virtualAddress >> 12) & 0x000FFFFFFFFFFFFF;
    uint16_t PageIndex = virtualPage & 0x1ff;
    uint16_t PageTableIndex = (virtualPage >> 9) & 0x1ff;
    uint16_t PageDirectoryIndex = (virtualPage >> 18) & 0x1ff;
    uint16_t TopLevelDirectoryPointerIndex = (virtualPage >> 27) & 0x1ff;

    uint16_t indexTable[] = {TopLevelDirectoryPointerIndex, PageDirectoryIndex, PageTableIndex, PageIndex};
    PageTableEntry *previousTable = this->RootTable;
    for (uint16_t index = 0; index < 4; index++)
    {
        PageTableEntry pageDirectoryEntry = previousTable[indexTable[index]]; //this->RootTable[indexer.TopLevelDirectoryPointerIndex];
        if (index < 3)
        {
            if (!pageDirectoryEntry.GetFlag(PageTableEntryFlag::Present))
            {
                if (!create)
                    return NULL;
                PageTableEntry *pageDirectoryPointer = (PageTableEntry *)pageAllocator->AllocatePage();
                memset(pageDirectoryPointer, 0, pageSize);
                pageDirectoryEntry.SetAddress((uint64_t)pageDirectoryPointer);
                pageDirectoryEntry.SetFlag(PageTableEntryFlag::Writable, true);
                pageDirectoryEntry.SetFlag(PageTableEntryFlag::Present, true);
                previousTable[indexTable[index]] = pageDirectoryEntry;
            }
        }
        else
        {
            if (!create && !pageDirectoryEntry.GetFlag(PageTableEntryFlag::Present))
                return NULL;
            return &previousTable[indexTable[index]];
        }

        previousTable = (PageTableEntry *)pageDirectoryEntry.GetAddressPointer();
    }
    return NULL;
}

void VirtualAddressManager::Map(void *virtualAddress, void *physicalAddress, bool writable)
{
    auto pageTableEntry = this->GetPageTableEntry(virtualAddress, true);
    memset(pageTableEntry, 0, sizeof(PageTableEntry));
    auto phyiscalAddressInt = (uint64_t)physicalAddress;
    if (phyiscalAddressInt % 4096)
    {
        kPanic("Physical address isn't page aligned!");
    }
    if ((uint64_t)virtualAddress % 4096)
    {
        kPanic("Virtual address isn't page aligned!");
    }
    pageTableEntry->SetAddress((uint64_t)physicalAddress);
    pageTableEntry->SetFlag(PageTableEntryFlag::Writable, writable);
    pageTableEntry->SetFlag(PageTableEntryFlag::Present, true);
}

void *VirtualAddressManager::GetPhysicalAddress(void *virtualAddress)
{
    auto pageTableEntry = this->GetPageTableEntry(virtualAddress, false);
    if (pageTableEntry == NULL || !pageTableEntry->GetFlag(PageTableEntryFlag::Present))
        return NULL;
    return pageTableEntry->GetAddressPointer();
}

void VirtualAddressManager::Map(void *physicalAddress, bool writable)
{
    this->Map(physicalAddress, physicalAddress, writable);
}

void VirtualAddressManager::Activate()
{
    // Move the address of our root table into CR3
    // This will cause the MMU on the CPU to use it.
    asm("mov %0, %%cr3"
        :
        : "r"(this->RootTable));
}

VirtualAddressManager *VirtualAddressManager::GetKernelVirtualAddressManager()
{
    if (KernelVirtualAddressManager == NULL)
    {
        KernelVirtualAddressManager = new VirtualAddressManager();
    }
    return KernelVirtualAddressManager;
}