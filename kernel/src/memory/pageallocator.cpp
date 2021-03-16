#include <memory/pageallocator.hpp>
#include <kernelparameters.h>
#include <memory/heap.hpp>
#include <console/font.hpp>
#include <console/cstr.hpp>
#include <graphics/framebuffer.hpp>

#define MDT_EFI_CONVENTIONAL_MEMORY_TYPE 7
extern uint64_t _kernelStart;
extern uint64_t _kernelEnd;

PageAllocator *PageAllocator::Instance = NULL;

PageAllocator *PageAllocator::Initialize(Memory *memory)
{
    if (PageAllocator::Instance == NULL)
    {
        PageAllocator::Instance = new PageAllocator(memory);
    }
    return PageAllocator::Instance;
}

PageAllocator *PageAllocator::GetInstance()
{
    if (PageAllocator::Instance == NULL)
        return PageAllocator::Initialize(Memory::GetInstance());
    return PageAllocator::Instance;
}

PageAllocator::PageAllocator(Memory *memory)
{
    this->memory = memory;
    this->reservedMemory = this->usedMemory = 0;
    auto memorySize = memory->Size();
    auto pageSize = memory->PageSize();
    auto memorySizePages = (memorySize / pageSize) + ((memorySize % pageSize) != 0 ? 1 : 0);
    this->freeMemory = memorySize;
    BootMemoryMap *bootMemoryMap = memory->GetBootMemoryMap();
    uint64_t entries = bootMemoryMap->MemoryMapSize / bootMemoryMap->MemoryMapDescriptorSize;
    // Figure out how many bytes we need for the bitmap
    // We need 1 bit per page, and size is in bytes.
    uint64_t bitmapSize = memorySize;
    bitmapSize /= pageSize;
    bitmapSize /= 8;
    if (memorySize % pageSize != 0)
        bitmapSize++;
    uint64_t bitmapPageSize = bitmapSize / pageSize;
    if (bitmapSize % pageSize)
        bitmapPageSize++;

    uint64_t selectedEntryPageCount = 0;
    BootMemoryDescriptor *selectedEntryDescriptor = NULL;
    for (uint64_t i = 0; i < entries; i++)
    {
        BootMemoryDescriptor *descriptor = (BootMemoryDescriptor *)((uint64_t)bootMemoryMap->MemoryMap + (i * bootMemoryMap->MemoryMapDescriptorSize));
        if (descriptor->Type != MDT_EFI_CONVENTIONAL_MEMORY_TYPE)
            continue;
        // Find smallest page range that is larger than the page size of the bitmap.
        if (
            descriptor->PageCount > bitmapPageSize &&
            (selectedEntryPageCount < bitmapPageSize ||       // If the selected page count is too small for our bitmap (IE we haven't found a spot yet)
             descriptor->PageCount < selectedEntryPageCount)) // or the current descriptor is smaller than the currently selected one
        {
            // Select this descriptor as the location for our page bitmap.
            selectedEntryDescriptor = descriptor;
            selectedEntryPageCount = descriptor->PageCount;
        }
    }
    auto consoleFont = KernelConsoleFont::GetInstance();
    uint8_t *bitmapBuffer = (uint8_t *)selectedEntryDescriptor->PhysicalAddress;
    memset(bitmapBuffer, 0x00, bitmapSize);
    this->bitmap = new Bitmap(bitmapBuffer, bitmapSize);
    // Reserve the entire address space.
    this->ReservePages((void *)0, memorySizePages);
    // The UEFI memory map may be sparse
    // Anything not listed, we should consider reserved.
    // So we're going to un-reserve anything that matches EFI Conventional memory type.
    for (uint64_t i = 0; i < entries; i++)
    {
        BootMemoryDescriptor *descriptor = (BootMemoryDescriptor *)((uint64_t)bootMemoryMap->MemoryMap + (i * bootMemoryMap->MemoryMapDescriptorSize));
        // Guard against weird descriptors.
        if (descriptor->PageCount == 0 || (uint64_t)descriptor->PhysicalAddress == 0xffffffffffffffff)
            continue;
        if (descriptor->Type == MDT_EFI_CONVENTIONAL_MEMORY_TYPE)
        {
            this->UnreservePages(descriptor->PhysicalAddress, descriptor->PageCount);
            continue;
        }
    }
    // Reserve the bitmap, since we allocated it within conventional memory
    this->ReservePages(bitmapBuffer, bitmapPageSize);
    
    // Lock the kernel, in case it isn't already.
    uint64_t kernelPageCount = (((uint64_t)&_kernelEnd - (uint64_t)&_kernelStart) / pageSize) + 1;
    this->LockPages(&_kernelStart, kernelPageCount);
    // Reserve the lower 640k of RAM, even if they aren't reserved by the memory map.
    this->ReservePages((void *)0, 0x100); 
}

// From the page allocators view, there's no difference between reserved and allocated pages
// The only difference is how we track used memory (used vs reserved)
// In fact, that's really our only job: Tracking the state of memory pages, and finding ones that aren't allocated and giving them out.
// And tracking some stats as we do so.

void *PageAllocator::AllocatePage(bool zero)
{
    uint64_t bitmapSize = this->bitmap->Size();
    uint64_t pageSize = this->memory->PageSize();
    for (uint64_t index = earliestKnownFreePage; index < bitmapSize; index++)
    {
        if ((*this->bitmap)[index])
        {
            continue; // This page is allocated already;
        }
        this->earliestKnownFreePage = index - 1;
        void *newPage = (void *)(index * pageSize);
        this->LockPage(newPage); // Mark the page as allocated
        if(zero)
        {
            memset(newPage, 0, this->PageSize());
        }
        return newPage;
    }

    // Allocation failed (This is where the paging manager will take over.)
    return NULL;
}

void PageAllocator::LockPage(void *address)
{
    uint64_t index = (uint64_t)address / this->memory->PageSize();
    if ((*this->bitmap)[index])
        return;
    if (!this->bitmap->Set(index))
        return;
    freeMemory -= this->memory->PageSize();
    usedMemory += this->memory->PageSize();
}

void PageAllocator::FreePage(void *address)
{
    uint64_t index = (uint64_t)address / this->memory->PageSize();
    if (!(*this->bitmap)[index])
        return;
    if (!this->bitmap->Unset(index))
        return;
    freeMemory += this->memory->PageSize();
    usedMemory -= this->memory->PageSize();
    if (index < this->earliestKnownFreePage)
        this->earliestKnownFreePage = index;
}

void PageAllocator::ReservePage(void *address)
{
    uint64_t index = (uint64_t)address / this->memory->PageSize();
    if ((*this->bitmap)[index])
        return;
    if (!this->bitmap->Set(index))
        return;
    freeMemory -= this->memory->PageSize();
    reservedMemory += this->memory->PageSize();
}

void PageAllocator::UnreservePage(void *address)
{
    uint64_t index = (uint64_t)address / this->memory->PageSize();
    if (!(*this->bitmap)[index])
        return;
    if (!this->bitmap->Unset(index))
        return;
    freeMemory += this->memory->PageSize();
    reservedMemory -= this->memory->PageSize();
    if (index < this->earliestKnownFreePage)
        this->earliestKnownFreePage = index;
}

void PageAllocator::FreePages(void *address, uint64_t count)
{
    auto baseAddress = (uint64_t)address;
    for (uint64_t x = 0; x < count; x++)
        FreePage((void *)(baseAddress + (x * this->memory->PageSize())));
}

void PageAllocator::LockPages(void *address, uint64_t count)
{
    auto baseAddress = (uint64_t)address;
    for (uint64_t x = 0; x < count; x++)
        LockPage((void *)(baseAddress + (x * this->memory->PageSize())));
}

void PageAllocator::ReservePages(void *address, uint64_t count)
{
    auto baseAddress = (uint64_t)address;
    for (uint64_t x = 0; x < count; x++)
        ReservePage((void *)(baseAddress + (x * this->memory->PageSize())));
}

void PageAllocator::UnreservePages(void *address, uint64_t count)
{
    auto baseAddress = (uint64_t)address;
    for (uint64_t x = 0; x < count; x++)
        UnreservePage((void *)(baseAddress + (x * this->memory->PageSize())));
}

FreeMemoryInformation PageAllocator::GetFreeMemoryInformation()
{
    return {this->freeMemory, this->reservedMemory, this->usedMemory};
}

Bitmap *PageAllocator::GetBitmap()
{
    return this->bitmap;
}