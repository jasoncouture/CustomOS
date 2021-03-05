#pragma once
#include <kernelparameters.h>
#include <stdint.h>
#include "../bitmap/bitmap.hpp"
#include "memory.hpp"

struct FreeMemoryInformation 
{
    uint64_t BytesFree;
    uint64_t BytesReserved;
    uint64_t BytesUsed;
};

class PageAllocator
{
    private:
        Memory* memory;
        PageAllocator(Memory* memory);
        Bitmap bitmap;
        uint64_t freeMemory;
        uint64_t reservedMemory;
        uint64_t usedMemory;
        uint64_t earliestKnownFreePage = 0;

        static PageAllocator Instance;

        void ReservePage(void* address);
        void ReservePages(void* address, uint64_t count);
        void UnreservePage(void* address);
        void UnreservePages(void* address, uint64_t count);
    public:

        static PageAllocator * Initialize(Memory* memory);
        static PageAllocator * GetInstance();
        size_t PageSize() { this->memory->PageSize(); }
        void FreePage(void* address);
        void FreePages(void* address, uint64_t count);
        void LockPage(void* address);
        void LockPages(void* address, uint64_t count);

        void* AllocatePage();

        FreeMemoryInformation GetFreeMemoryInformation();
};

