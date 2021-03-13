#include "memory.hpp"

#define PAGE_SIZE 0x1000

Memory *Memory::Instance = NULL;

uint64_t ComputeSize(BootMemoryMap *bootMemoryMap) 
{
    uint64_t memorySizeBytes = 0;
    uint64_t entries = bootMemoryMap->MemoryMapSize / bootMemoryMap->MemoryMapDescriptorSize;
    for (uint64_t i = 0; i < entries; i++)
    {
        BootMemoryDescriptor *descriptor = (BootMemoryDescriptor *)((uint64_t)bootMemoryMap->MemoryMap + (i * bootMemoryMap->MemoryMapDescriptorSize));
        if(descriptor->PageCount == 0 || (uint64_t)descriptor->PhysicalAddress == 0xffffffffffffffff) continue;
        memorySizeBytes += descriptor->PageCount;
    }

    memorySizeBytes *= PAGE_SIZE;

    return memorySizeBytes;
}

Memory::Memory(BootMemoryMap *bootMemoryMap)
{
    this->bootMemoryMap = bootMemoryMap;
    this->memorySizeBytes = ComputeSize(bootMemoryMap);
}

Memory *Memory::GetInstance()
{
    return Instance;
}

Memory *Memory::Initialize(BootMemoryMap *bootMemoryMap)
{
    if(Instance == NULL) 
    {
        Instance = new Memory(bootMemoryMap);
    }
    return Instance;
}
uint64_t Memory::PageSize()
{
    return PAGE_SIZE;
}

uint64_t Memory::Size()
{
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