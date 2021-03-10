#include <stdint.h>
#include <stddef.h>
#include "virtualaddressmanager.hpp"
#include "pagetableindexer.hpp"
#include "../memory.hpp"
#include "../pageallocator.hpp"
#include "../../console/font.hpp"
#include "../../console/cstr.hpp"
#include "../../graphics/framebuffer.hpp"

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

void kVirtualAddressDebug(const char *message, int column = 0)
{
    #ifdef _K_MEMORY_DEBUG
    static uint64_t yLocation = 0;
    if (yLocation == 0)
        yLocation = KernelFrameBuffer::GetInstance()->GetHeight() - (KernelConsoleFont::GetInstance()->GetCharacterPixelHeight() * 2);

    for (uint64_t x = column * KernelConsoleFont::GetInstance()->GetCharacterPixelWidth(); x < KernelFrameBuffer::GetInstance()->GetWidth(); x += KernelConsoleFont::GetInstance()->GetCharacterPixelWidth())
    {
        KernelConsoleFont::GetInstance()->DrawCharacterAt(' ', x, yLocation);
    }
    KernelConsoleFont::GetInstance()->DrawStringAt(message, 0, yLocation);
    #endif
}

void VirtualAddressManager::Map(void *virtualAddress, void *physicalAddress)
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
    PageTableEntry pageDirectoryEntry;
    for (uint16_t index = 0; index < 4; index++)
    {
        PageTableEntry pageDirectoryEntry = previousTable[indexTable[index]]; //this->RootTable[indexer.TopLevelDirectoryPointerIndex];
        if (index < 3)
        {
            if (!pageDirectoryEntry.GetFlag(PageTableEntryFlag::Present))
            {

                PageTableEntry *pageDirectoryPointer = (PageTableEntry *)pageAllocator->AllocatePage();
                kVirtualAddressDebug(kToHexString((uint64_t)pageDirectoryPointer));
                memset(pageDirectoryPointer, 0, pageSize);
                pageDirectoryEntry.SetAddress((uint64_t)pageDirectoryPointer);
                pageDirectoryEntry.SetFlag(PageTableEntryFlag::Writable, true);
                pageDirectoryEntry.SetFlag(PageTableEntryFlag::Present, true);
                previousTable[indexTable[index]] = pageDirectoryEntry;
            }
        }
        else
        {

            pageDirectoryEntry.SetAddress((uint64_t)physicalAddress);
            if (physicalAddress == NULL)
                pageDirectoryEntry.SetFlag(PageTableEntryFlag::Writable, false); // Cause page faults on null pointer access. :)
            else
                pageDirectoryEntry.SetFlag(PageTableEntryFlag::Writable, true);
            pageDirectoryEntry.SetFlag(PageTableEntryFlag::Present, true);
            previousTable[indexTable[index]] = pageDirectoryEntry;
            return;
        }

        previousTable = (PageTableEntry *)pageDirectoryEntry.GetAddressPointer();
    }

    // PageTableEntry pageDirectoryEntry = previousTable[TopLevelDirectoryPointerIndex]; //this->RootTable[indexer.TopLevelDirectoryPointerIndex];
    // if (!pageDirectoryEntry.GetFlag(PageTableEntryFlag::Present))
    // {
    //     PageTableEntry *pageDirectoryPointer = (PageTableEntry *)pageAllocator->AllocatePage();
    //     memset(pageDirectoryPointer, 0, pageSize);
    //     pageDirectoryEntry.SetAddress((uint64_t)pageDirectoryPointer);
    //     pageDirectoryEntry.SetFlag(PageTableEntryFlag::Writable, true);
    //     pageDirectoryEntry.SetFlag(PageTableEntryFlag::Present, true);
    //     previousTable[TopLevelDirectoryPointerIndex] = pageDirectoryEntry;
    // }

    // previousTable = (PageTableEntry *)pageDirectoryEntry.GetAddressPointer();

    // pageDirectoryEntry = previousTable[PageDirectoryIndex];
    // if (!pageDirectoryEntry.GetFlag(PageTableEntryFlag::Present))
    // {
    //     PageTableEntry *pageDirectoryPointer = (PageTableEntry *)pageAllocator->AllocatePage();
    //     memset(pageDirectoryPointer, 0, pageSize);
    //     pageDirectoryEntry.SetAddress((uint64_t)pageDirectoryPointer);
    //     pageDirectoryEntry.SetFlag(PageTableEntryFlag::Writable, true);
    //     pageDirectoryEntry.SetFlag(PageTableEntryFlag::Present, true);
    //     previousTable[PageDirectoryIndex] = pageDirectoryEntry;
    // }

    // previousTable = (PageTableEntry *)pageDirectoryEntry.GetAddressPointer();

    // pageDirectoryEntry = previousTable[PageTableIndex]; //this->RootTable[indexer.TopLevelDirectoryPointerIndex];
    // if (!pageDirectoryEntry.GetFlag(PageTableEntryFlag::Present))
    // {
    //     PageTableEntry *pageDirectoryPointer = (PageTableEntry *)pageAllocator->AllocatePage();
    //     memset(pageDirectoryPointer, 0, pageSize);
    //     pageDirectoryEntry.SetAddress((uint64_t)pageDirectoryPointer);
    //     pageDirectoryEntry.SetFlag(PageTableEntryFlag::Writable, true);
    //     pageDirectoryEntry.SetFlag(PageTableEntryFlag::Present, true);
    //     previousTable[PageTableIndex] = pageDirectoryEntry;
    // }

    // previousTable = (PageTableEntry *)pageDirectoryEntry.GetAddressPointer();

    // pageDirectoryEntry = previousTable[PageIndex]; //this->RootTable[indexer.TopLevelDirectoryPointerIndex];
    // pageDirectoryEntry.SetAddress((uint64_t)physicalAddress);
    // pageDirectoryEntry.SetFlag(PageTableEntryFlag::Writable, true);
    // pageDirectoryEntry.SetFlag(PageTableEntryFlag::Present, true);
    // previousTable[PageIndex] = pageDirectoryEntry;
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

// VirtualAddressManager::~VirtualAddressManager()
// {
//     if (this->FreeOnDestroy)
//     {
//         auto pageAllocator = PageAllocator::GetInstance();
//         pageAllocator->FreePage(this->RootTable);
//     }
// }