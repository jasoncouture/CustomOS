#include <memory/memory.hpp>

#define PAGE_SIZE 0x1000

Memory *Memory::Instance = NULL;

uint64_t ComputeSize(BootMemoryMap *bootMemoryMap)
{
    uint64_t memorySizeBytes = 0;
    uint64_t entries = bootMemoryMap->MemoryMapSize / bootMemoryMap->MemoryMapDescriptorSize;
    for (uint64_t i = 0; i < entries; i++)
    {
        BootMemoryDescriptor *descriptor = (BootMemoryDescriptor *)((uint64_t)bootMemoryMap->MemoryMap + (i * bootMemoryMap->MemoryMapDescriptorSize));
        if (descriptor->PageCount == 0 || (uint64_t)descriptor->PhysicalAddress == 0xffffffffffffffff)
            continue;
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
    static bool initialized = false;
    if (!initialized)
    {
        initialized = true;
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

void memset(void *memoryLocation, uint8_t value, size_t size)
{
    auto buffer = (uint8_t *)memoryLocation;
    for (size_t i = 0; i < size; i++)
        buffer[i] = value;
}

void memcopy(uint64_t *source, uint64_t *destination, size_t size)
{
    auto remainder = size % sizeof(uint64_t);

    for (size_t x = 0; x < (size - remainder); x += sizeof(uint64_t))
    {
        auto offset = x / sizeof(uint64_t);
        destination[offset] = source[offset];
    }
    if (remainder)
        memcopy(((uint8_t *)source + (size - remainder)), ((uint8_t *)destination + (size - remainder)), remainder);
}

void memcopy(uint32_t *source, uint32_t *destination, size_t size)
{
    auto remainder = size % sizeof(uint32_t);

    for (size_t x = 0; x < (size - remainder); x += sizeof(uint32_t))
    {
        auto offset = x / sizeof(uint32_t);
        destination[offset] = source[offset];
    }

    if (remainder)
        memcopy(((uint8_t *)source + (size - remainder)), ((uint8_t *)destination + (size - remainder)), remainder);
}

inline void memcopy(uint8_t *source, uint8_t *destination, size_t size)
{
    for (size_t x = 0; x < size; x++)
        destination[x] = source[x];
}

void memcopy(void *source, void *destination, size_t size)
{
    if (size >= sizeof(uint64_t))
    {
        memcopy((uint64_t *)source, (uint64_t *)destination, size);
        return;
    }

    if (size >= sizeof(uint32_t))
    {
        memcopy((uint32_t *)source, (uint32_t *)destination, size);
        return;
    }

    memcopy((uint8_t *)source, (uint8_t *)destination, size);
}