#include <stddef.h>
#include <kernelparameters.h>
#include "graphics/framebuffer.hpp"
#include "console/font.hpp"
#include "memory/memory.hpp"
#include "memory/pageallocator.hpp"
#include "console/cstr.hpp"
#include "memory/paging/virtualaddressmanager.hpp"
#include "memory/gdt/gdt.hpp"
#include "interrupts/idt.hpp"
#include "interrupts/interrupts.hpp"

void kInitGlobalDesciptorTable() 
{
    GlobalDescriptorLocation globalDescriptorLocation;
    globalDescriptorLocation.Size = sizeof(GlobalDesciptorTable) - 1;
    globalDescriptorLocation.GlobalDescriptorTable = &DefaultGlobalDesciptorTable;
    LoadGlobalDescriptorTable(&globalDescriptorLocation);
}

void kInitMemory(BootMemoryMap *bootMemoryMap) {
    auto memory = Memory::Initialize(bootMemoryMap);
    PageAllocator::Initialize(memory);
}

void kInitVirtualMemory(FrameBuffer *frameBuffer, BootMemoryMap *memoryMap) {
    auto pageAllocator = PageAllocator::GetInstance();
    auto memory = Memory::GetInstance();
    
    // Move the framebuffer from "used" (if it was there) to "reserved" memory
    pageAllocator->FreePages(frameBuffer->BaseAddress, (frameBuffer->Size / pageAllocator->PageSize()));
    pageAllocator->ReservePages(frameBuffer->BaseAddress, (frameBuffer->Size / pageAllocator->PageSize()));
    // Setup the virtual memory manager.
    KernelVirtualAddressManager = VirtualAddressManager();
    auto virtualAddressManager = &KernelVirtualAddressManager;
    uint64_t pageSize = memory->PageSize();
    uint64_t memorySize = memory->Size();
    uint64_t page = 0;
    // Identity map all memory.
    uint64_t entries = memoryMap->MemoryMapSize / memoryMap->MemoryMapDescriptorSize;
    for (uint64_t i = 0; i < entries; i++)
    {
        BootMemoryDescriptor *descriptor = (BootMemoryDescriptor *)((uint64_t)memoryMap->MemoryMap + (i * memoryMap->MemoryMapDescriptorSize));
        if(descriptor->PageCount == 0 || (uint64_t)descriptor->PhysicalAddress == 0xffffffffffffffff) continue;
        KernelVirtualAddressManager.Map(descriptor->VirtualAddress, descriptor->PhysicalAddress);
    }
    // for (page = 0; page < memorySize + pageSize; page += pageSize)
    //     virtualAddressManager->Map((void *)page, (void *)page);
    // Frame buffer might lie outside of memory space, so make sure it's mapped into virtual memory as well.
    for (page = (uint64_t)frameBuffer->BaseAddress; page < frameBuffer->Size + pageSize; page += pageSize)
        KernelVirtualAddressManager.Map((void *)page, (void *)page);

    // And activate our virtual memory map.
    KernelVirtualAddressManager.Activate();
}

void kInitFrameBuffer(FrameBuffer *frameBuffer)
{
    auto kernelFrameBuffer = KernelFrameBuffer::InitializeInstance(frameBuffer);
    kernelFrameBuffer->Clear(0);
}

void kInitConsoleFont(Font *font) 
{
    KernelConsoleFont::InitializeInstance(font);
}

InterruptDesciptorTableLocation interruptDesciptorTableLocation;

void kInitInterrupts() 
{
    interruptDesciptorTableLocation.Limit = 0x0FFF; // Maximum number of entries;
    interruptDesciptorTableLocation.InterruptDesciptors = (InterruptDescriptorTableEntry*)PageAllocator::GetInstance()->AllocatePage();

    InterruptDescriptorTableEntry *pageFaultEntry = interruptDesciptorTableLocation.InterruptDesciptors + 0xE;
    pageFaultEntry->SetOffset((uint64_t)Interrupt_PageFaultHandler);
    pageFaultEntry->TypeAndAttribute = IDT_TYPEATTRIBUTE_INTERRUPTGATE;
    pageFaultEntry->Selector = 0x08; // Kernel code segment selector (See GDT);
    // Load IDT
    asm ( "lidt %0" : : "m" (interruptDesciptorTableLocation) );
}

void kInit(KernelParameters *kernelParameters)
{
    
    kInitMemory(kernelParameters->BootMemoryMap);
    kInitVirtualMemory(kernelParameters->FrameBuffer, kernelParameters->BootMemoryMap);
    kInitGlobalDesciptorTable();
    kInitFrameBuffer(kernelParameters->FrameBuffer);   
    kInitConsoleFont(kernelParameters->Font);
    //kInitInterrupts();
}