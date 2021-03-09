#include "pageallocator.hpp"
#include <kernelparameters.h>

#define MDT_EFI_CONVENTIONAL_MEMORY_TYPE 7
extern uint64_t _kernelStart;
extern uint64_t _kernelEnd;

PageAllocator PageAllocator::Instance = 0;

PageAllocator *PageAllocator::Initialize(Memory *memory)
{
    PageAllocator::Instance = PageAllocator(memory);
    return &PageAllocator::Instance;
}

PageAllocator *PageAllocator::GetInstance()
{
    return &PageAllocator::Instance;
}

PageAllocator::PageAllocator(Memory *memory)
{
    this->memory = memory;
    this->reservedMemory = this->usedMemory = 0;
    uint64_t memorySize = memory->Size();
    this->freeMemory = memorySize;
    BootMemoryMap *bootMemoryMap = memory->GetBootMemoryMap();
    uint64_t entries = bootMemoryMap->MemoryMapSize / bootMemoryMap->MemoryMapDescriptorSize;
    // Figure out how many bytes we need for the bitmap
    // We need 1 bit per page, and size is in bytes.
    uint64_t bitmapSize = memorySize;
    bitmapSize /= memory->PageSize();
    bitmapSize /= 8;
    if (memorySize % memory->PageSize() != 0)
        bitmapSize++;
    uint64_t bitmapPageSize = bitmapSize / memory->PageSize();
    if (bitmapSize % memory->PageSize())
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

    uint8_t *bitmapBuffer = (uint8_t *)selectedEntryDescriptor->PhysicalAddress;
    memset(bitmapBuffer, 0, bitmapSize);

    this->bitmap = Bitmap(bitmapBuffer, bitmapSize);
    // This can waste up to 4095 bytes of ram
    // but honestly, who gives a shit? We don't have virtual memory addressing quite yet.
    this->LockPages(bitmapBuffer, bitmapPageSize);
    //uint64_t kernelPageCount = (((uint64_t)&_kernelEnd - (uint64_t)&_kernelStart) / this->memory->PageSize()) + 1;
    //this->LockPages(&_kernelStart, kernelPageCount);

    for (uint64_t i = 0; i < entries; i++)
    {
        BootMemoryDescriptor *descriptor = (BootMemoryDescriptor *)((uint64_t)bootMemoryMap->MemoryMap + (i * bootMemoryMap->MemoryMapDescriptorSize));
        // Guard against weird descriptors.
        if (descriptor->PageCount == 0 || (uint64_t)descriptor->PhysicalAddress == 0xffffffffffffffff)
            continue;
        if (descriptor->Type == MDT_EFI_CONVENTIONAL_MEMORY_TYPE)
            continue;
        this->ReservePages(descriptor->PhysicalAddress, descriptor->PageCount);
    }
    // Lock bitmap pages, and reserve non-conventional pages.
}

// From the page allocators view, there's no difference between reserved and allocated pages
// The only difference is how we track used memory (used vs reserved)
// In fact, that's really our only job: Tracking the state of memory pages, and finding ones that aren't allocated and giving them out.
// And tracking some stats as we do so.

void *PageAllocator::AllocatePage()
{
    auto bitmapSize = this->bitmap.Size();
    for (auto index = this->earliestKnownFreePage; index < bitmapSize; index++)
    {
        if (this->bitmap[index])
            continue; // This page is allocated already;
        this->earliestKnownFreePage = index;
        void *newPage = (void *)(index * this->memory->PageSize());
        this->LockPage(newPage); // Mark the page as allocated
        return newPage;
    }

    // Allocation failed (This is where the virtual memory manager will take over.)
    return NULL;
}

void PageAllocator::LockPage(void *address)
{
    uint64_t index = (uint64_t)address / this->memory->PageSize();
    if (this->bitmap[index])
        return;
    if (!this->bitmap.Set(index))
        return;
    freeMemory -= this->memory->PageSize();
    usedMemory += this->memory->PageSize();
}

void PageAllocator::FreePage(void *address)
{
    uint64_t index = (uint64_t)address / this->memory->PageSize();
    if (!this->bitmap[index])
        return;
    if (!this->bitmap.Unset(index))
        return;
    freeMemory += this->memory->PageSize();
    usedMemory -= this->memory->PageSize();
    if (index < this->earliestKnownFreePage)
        this->earliestKnownFreePage = index;
}

void PageAllocator::ReservePage(void *address)
{
    uint64_t index = (uint64_t)address / this->memory->PageSize();
    if (this->bitmap[index])
        return;
    if (!this->bitmap.Set(index))
        return;
    freeMemory -= this->memory->PageSize();
    reservedMemory += this->memory->PageSize();
}

void PageAllocator::UnreservePage(void *address)
{
    uint64_t index = (uint64_t)address / this->memory->PageSize();
    if (!this->bitmap[index])
        return;
    if (!this->bitmap.Unset(index))
        return;
    freeMemory += this->memory->PageSize();
    reservedMemory -= this->memory->PageSize();
    if (index < this->earliestKnownFreePage)
        this->earliestKnownFreePage = index;
}

void PageAllocator::FreePages(void *address, uint64_t count)
{
    for (uint64_t x = 0; x < count; x++)
        FreePage((void *)((uint64_t)address + (x * this->memory->PageSize())));
}

void PageAllocator::LockPages(void *address, uint64_t count)
{
    for (uint64_t x = 0; x < count; x++)
        LockPage((void *)((uint64_t)address + (x * this->memory->PageSize())));
}

void PageAllocator::ReservePages(void *address, uint64_t count)
{
    for (uint64_t x = 0; x < count; x++)
        ReservePage((void *)((uint64_t)address + (x * this->memory->PageSize())));
}

void PageAllocator::UnreservePages(void *address, uint64_t count)
{
    for (uint64_t x = 0; x < count; x++)
        UnreservePage((void *)((uint64_t)address + (x * this->memory->PageSize())));
}

FreeMemoryInformation PageAllocator::GetFreeMemoryInformation()
{
    return {this->freeMemory, this->reservedMemory, this->usedMemory};
}

Bitmap * PageAllocator::GetBitmap() {
    return &this->bitmap;
}