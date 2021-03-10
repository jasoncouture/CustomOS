#include <stddef.h>
#include <stdint.h>
#include "memory.hpp"
// 32 MB early fixed size heap.
#define EARLY_HEAP_SIZE 0x2000000

uint8_t *heapHead = NULL;
uint8_t heap[EARLY_HEAP_SIZE];
void* kMallocForever(size_t size) 
{
    // There's no way to handle sizes larger than the max pre-allocated size.
    if(size > EARLY_HEAP_SIZE) 
        return NULL;
    // We haven't been called before, set the initial pointer.
    if(heapHead == NULL) 
    {
        heapHead = heap;
        memset(heapHead, 0, EARLY_HEAP_SIZE);
    }
    // We don't have enough space for Size
    if((uint64_t)heapHead + size > (((uint64_t)heap) + EARLY_HEAP_SIZE)) 
        return NULL;
    // Grab the current pointer
    uint8_t *allocatedSpace = heapHead;
    // Advance the pointer by size
    heapHead = heapHead + size;

    // and return the preivously captured pointer.
    return allocatedSpace;
}

typedef void* (*MemoryAllocator)(size_t size);

MemoryAllocator DefaultMemoryAllocator = kMallocForever;

void * operator new(size_t size) 
{ 
    void * p = DefaultMemoryAllocator(size);
    memset(p, 0, size);
    return p; 
}

void * operator new[](size_t size)
{
    void * p = DefaultMemoryAllocator(size); 
    memset(p, 0, size);
    return p;
}

void operator delete(void *p) 
{

}

void operator delete[](void *p)
{
    // By default, we're using kMallocForever, so these do nothing
    // and you shouldn't be using new/delete with these.... Just new..
}

void operator delete(void *p, uint64_t size) 
{
}
