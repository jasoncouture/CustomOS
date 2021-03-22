#include <stddef.h>
#include <stdint.h>
#include <memory/memory.hpp>
#include <memory/heap.hpp>
#include <memory/paging/virtualaddressmanager.hpp>
#include <memory/pageallocator.hpp>
#include <debug.hpp>

#define EARLY_HEAP_SIZE 0x10000
#define MINIMUM_ALLOCATION_UNIT 24
#define HEAP_VIRTUAL_ADDRESS_BASE 0x10000000000

HeapSegment *heapHead = NULL;
void *heapEnd = NULL;
__attribute__((aligned(0x1000)))
uint8_t heap[EARLY_HEAP_SIZE];

bool canExpandHeap = false;

void InitializeHeap(VirtualAddressManager *virtualAddressManager, PageAllocator *pageAllocator)
{
    // The first thing we need to do, is find some obscene address area that no one is using,
    // and map our existing heap there.
    // From there, we need to update our existing pointer to point to that.
    // After that, we set a flag to indicate it's now OK to expand the heap.
    auto heapPages = EARLY_HEAP_SIZE / pageAllocator->PageSize();
    if (EARLY_HEAP_SIZE % pageAllocator->PageSize())
        heapPages += 1;
    uint8_t *heapBaseStart = (uint8_t *)heapHead;
    // Remap the early heap into the new virtual address space.
    for (uint64_t x = 0; x < heapPages; x++)
    {
        auto pageOffset = x * pageAllocator->PageSize();
        virtualAddressManager->Map((void *)(HEAP_VIRTUAL_ADDRESS_BASE + pageOffset), (heapBaseStart + pageOffset));
    }
    // Redirect our heap to it's new home.
    
    heapHead = (HeapSegment *)HEAP_VIRTUAL_ADDRESS_BASE;
    heapEnd = (void *)(HEAP_VIRTUAL_ADDRESS_BASE + EARLY_HEAP_SIZE);

    auto current = heapHead;

    // Fix all the pointers in the current heap.
    while(current != NULL) {
        if(current->Next != NULL) {
            current->Next = (HeapSegment*)((uint8_t*)current + sizeof(HeapSegment)+current->Length);
            current->Next->Previous = current;
        }
        // We don't have to worry about Previous, because Previous is fixed by Next.
        current = current->Next;
    }

    canExpandHeap = true;
}

void ExpandHeap(size_t size)
{
    size = size + sizeof(HeapSegment);
    auto pageAllocator = PageAllocator::GetInstance();
    auto virtualAddressManager = VirtualAddressManager::GetKernelVirtualAddressManager();
    auto lastSegment = heapHead;
    // Get the last segment.
    while (lastSegment->Next != NULL)
        lastSegment = lastSegment->Next;
    if(lastSegment->GetFlag(HeapSegmentFlag::IsFree))
        size -= lastSegment->Length + sizeof(HeapSegment);
    if(size < (pageAllocator->PageSize() * 4)) size = pageAllocator->PageSize() * 4;

    if (size % pageAllocator->PageSize())
    {
        size -= size % pageAllocator->PageSize();
        size += pageAllocator->PageSize();
    }

    

    auto sizeInPages = size / pageAllocator->PageSize();
    uint8_t *currentHeapEnd = (uint8_t *)heapEnd;
    for (uint64_t x = 0; x < sizeInPages; x++)
    {
        // Grow the heap, one page at a time, mapping it onto the end of our current heap.
        virtualAddressManager->Map(currentHeapEnd + (x * pageAllocator->PageSize()), pageAllocator->AllocatePage(false));
    }
    virtualAddressManager->Activate();
    
    // If the last segment isn't free, create a new segment at the end.
    if (!lastSegment->GetFlag(HeapSegmentFlag::IsFree))
    {
        // Zero out our newly allocated memory.
        memset(heapEnd, 0, sizeof(HeapSegment));
        lastSegment->Next = (HeapSegment *)heapEnd;
        lastSegment->Next->Previous = lastSegment;
        lastSegment->Next->Length = size - sizeof(HeapSegment);
        lastSegment->Next->SetFlag(HeapSegmentFlag::IsFree);
        lastSegment->Next->SetFlag(HeapSegmentFlag::Valid);
        lastSegment = lastSegment->Next;
    }
    else
    {
        // otherwise we can just update the size of the last segment :)
        lastSegment->Length = lastSegment->Length + size;
    }
    heapEnd = currentHeapEnd + (sizeInPages*pageAllocator->PageSize());
}

size_t GetEarlyHeapSize()
{
    return EARLY_HEAP_SIZE;
}
void *GetHeapBase()
{
    return (void *)heap;
}

void HeapSegment::SetFlag(HeapSegmentFlag flag, bool value)
{
    if (value)
        this->SetFlag(flag);
    else
        this->ClearFlag(flag);
}

void HeapSegment::ClearFlag(HeapSegmentFlag flag)
{
    uint8_t bitSelector = (uint8_t)1 << flag;
    this->Flags &= ~bitSelector;
}

void HeapSegment::SetFlag(HeapSegmentFlag flag)
{
    uint8_t bitSelector = (uint8_t)1 << flag;
    this->Flags |= bitSelector;
}

bool HeapSegment::GetFlag(HeapSegmentFlag flag)
{
    uint8_t bitSelector = (uint8_t)1 << flag;
    return (bool)(this->Flags & bitSelector);
}

void HeapSegment::CombineWithNext()
{
    if (this->Next == NULL)
        return;
    this->Next->CombineWithPrevious();
}

// After calling this, the current segment may no longer be valid. Don't use it.
void HeapSegment::CombineWithPrevious()
{
    if (this->Previous == NULL)
        return;
    // Both segments must be free.
    if (!this->GetFlag(HeapSegmentFlag::IsFree))
        return;
    if (!this->Previous->GetFlag(HeapSegmentFlag::IsFree))
        return;
    // Walk as far as we can, before we combine ourselves.
    if (this->Next != NULL)
    {
        this->Next->Previous = this->Previous;
    }
    this->Previous->Length = this->Previous->Length + this->Length + sizeof(HeapSegment);
    this->Previous->Next = this->Next;

    // We're no longer valid at this point.
}

void InitEarlyHeap()
{
    if (heapHead == NULL)
    {
        memset(heap, 0, EARLY_HEAP_SIZE);
        heapHead = (HeapSegment *)heap;
        heapEnd = (void *)((uint8_t *)heap + EARLY_HEAP_SIZE);
        // We zeroed all memory, so we only have to set length and flags since we want everything to be NULL.
        heapHead->Length = EARLY_HEAP_SIZE - sizeof(HeapSegment);
        heapHead->SetFlag(HeapSegmentFlag::IsFree);
        heapHead->SetFlag(HeapSegmentFlag::Valid); // Valid is used as a guard against heap corruption.
    }
}

void *HeapSegment::Address()
{
    return (void *)(this+1);
}

HeapSegment *HeapSegment::Split(size_t size)
{
    // There wouldn't be a worthwhile amount of space left after a split
    // just use this segment as is.
    if ((size + sizeof(HeapSegment) + MINIMUM_ALLOCATION_UNIT) > this->Length)
        return this;
    // Create a new entry, size bytes ahead of us and insert it into the linked list.
    auto next = (HeapSegment *)((uint8_t *)this->Address() + size);
    memset(next, 0, sizeof(HeapSegment));
    next->Length = this->Length - sizeof(HeapSegment) - size;
    next->SetFlag(HeapSegmentFlag::Valid);
    next->SetFlag(HeapSegmentFlag::IsFree);
    next->Previous = this;
    this->Length = size;
    if (this->Next)
    {
        // If we had a next, update it's previous to be this inserted node
        this->Next->Previous = next;
        // and update the new nodes next to be our current next.
        next->Next = this->Next;
    }
    // and finally update our next to be the newly inserted next.
    this->Next = next;
    // and return ourselves as the newly split node.
    return this;
}

void *malloc(size_t size)
{
    if (size == 0)
        return NULL;
    // There's no way to handle sizes larger than the max pre-allocated size.
    if (!canExpandHeap && size > EARLY_HEAP_SIZE)
        return NULL;
    // We haven't been called before, set the initial pointer.
    if (heapHead == NULL)
    {
        InitEarlyHeap();
    }

    HeapSegment *current = heapHead;
    while (current != NULL)
    {
        if (current->GetFlag(HeapSegmentFlag::IsFree))
        {
            if (current->Length > size)
            {
                auto target = current->Split(size);
                ASSERT(target->Length >= size);
                target->ClearFlag(HeapSegmentFlag::IsFree);
                return target->Address();
            }
            else if (current->Length == size)
            {
                current->ClearFlag(HeapSegmentFlag::IsFree);
                ASSERT(current->Length >= size);
                return current->Address();
            }
        }

        current = current->Next;
    }

    if (!canExpandHeap)
        return NULL;
    // We need more heap space!
    ExpandHeap(size);
    return malloc(size);
}

void *kmalloc(size_t size) { return malloc(size); }

void *realloc(void *pointer, size_t size)
{
    if (size == 0)
        return NULL;
    if (pointer == NULL)
        return malloc(size);
    auto heapSegment = (HeapSegment *)((uint8_t *)pointer - sizeof(HeapSegment));
    size_t sizeToCopy = heapSegment->Length;
    if (sizeToCopy > size)
        sizeToCopy = size;
    void *returnValue = malloc(size);
    if (sizeToCopy)
    {
        memcopy(pointer, returnValue, sizeToCopy);
    }
    free(pointer);
    return returnValue;
}

void free(void *address)
{
    auto segment = (HeapSegment *)((uint8_t *)address - sizeof(HeapSegment));
    if (!segment->GetFlag(HeapSegmentFlag::Valid))
        return;
    if (segment->GetFlag(HeapSegmentFlag::IsFree))
        return;

    segment->SetFlag(HeapSegmentFlag::IsFree);
    segment->CombineWithNext();
    segment->CombineWithPrevious();
}

void *calloc(size_t count, size_t size)
{
    auto pointer = malloc(count * size);
    if(pointer == NULL)
    {
        return NULL;
    }
    memset(pointer, 0, count * size);
    return pointer;
}

void *operator new(size_t size) { return calloc(1, size); }
void *operator new[](size_t size) { return calloc(1, size); }
void operator delete(void *p) { free(p); }
void operator delete[](void *p) { free(p); }
void operator delete(void *p, uint64_t size) { free(p); }
