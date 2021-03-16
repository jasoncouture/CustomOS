#pragma once
#include <kernelparameters.h>

class Memory
{
    private:
        BootMemoryMap* bootMemoryMap;
        static Memory* Instance;
        uint64_t memorySizeBytes;
        Memory(BootMemoryMap* BootMemoryMap);
    
    public:
        static Memory * GetInstance();
        static Memory * Initialize(BootMemoryMap* bootMemoryMap);
        uint64_t Size();
        uint64_t PageSize();
        BootMemoryMap *GetBootMemoryMap();
};


void memset(void* memoryLocation, uint8_t value, size_t size);
void memcopy(void* source, void* destination, size_t size);