#pragma once
#include <stddef.h>
#include <stdint.h>
#include <memory/pageallocator.hpp>
#include <memory/paging/virtualaddressmanager.hpp>

enum HeapSegmentFlag
{
    IsFree = 0,
    Valid = 1
};

struct HeapSegment {
    size_t Length;
    HeapSegment* Next;
    HeapSegment* Previous;
    uint8_t Flags;
    void CombineWithNext();
    void CombineWithPrevious();
    bool GetFlag(HeapSegmentFlag flag);
    void SetFlag(HeapSegmentFlag flag);
    void SetFlag(HeapSegmentFlag flag, bool value);
    void ClearFlag(HeapSegmentFlag flag);
    HeapSegment* Split(size_t size);
    void* Address();
}__attribute__((packed));

void* kmalloc(size_t size);
void* calloc(size_t count, size_t size);
void* malloc(size_t size);
void* realloc(void* pointer, size_t size);
void free(void* address);
void InitializeHeap(VirtualAddressManager *virtualAddressManager, PageAllocator *pageAllocator);