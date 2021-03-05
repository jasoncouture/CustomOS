#include "memory.hpp"

Memory Memory::Instance = 0;

Memory::Memory(BootMemoryMap *bootMemoryMap)
{
    this->bootMemoryMap = bootMemoryMap;
}

Memory *Memory::GetInstance()
{
    return &Instance;
}

Memory *Memory::Initialize(BootMemoryMap *bootMemoryMap)
{
    Instance = Memory(bootMemoryMap);
    return &Instance;
}
uint64_t Memory::PageSize()
{
    return 4096;
}

uint64_t Memory::Size()
{
    // only calculate this once.
    static uint64_t memorySizeBytes = 0;
    if (memorySizeBytes > 0)
        return memorySizeBytes;
    uint64_t entries = this->bootMemoryMap->MemoryMapSize / this->bootMemoryMap->MemoryMapDescriptorSize;
    for (uint64_t i = 0; i < entries; i++)
    {
        BootMemoryDescriptor *descriptor = (BootMemoryDescriptor *)((uint64_t)this->bootMemoryMap->MemoryMap + (i * this->bootMemoryMap->MemoryMapDescriptorSize));
        memorySizeBytes += descriptor->PageCount * this->PageSize();
    }

    return memorySizeBytes;
}

BootMemoryMap *Memory::GetBootMemoryMap() 
{
    return this->bootMemoryMap;
}

void memset(void* memoryLocation, uint8_t value, size_t size) 
{
    auto buffer = (uint8_t*)memoryLocation;
    for(size_t i = 0; i < size; i++)
        buffer[i] = value;
}