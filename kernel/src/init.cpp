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

GlobalDescriptorLocation globalDescriptorLocation;
void kInitGlobalDesciptorTable() 
{
    globalDescriptorLocation.Size = sizeof(GlobalDesciptorTable) - 1;
    globalDescriptorLocation.Offset = (uint64_t)&DefaultGlobalDesciptorTable;
    LoadGlobalDescriptorTable(&globalDescriptorLocation);
}

void kInitMemory(BootMemoryMap *bootMemoryMap) {
    auto memory = Memory::Initialize(bootMemoryMap);
    PageAllocator::Initialize(memory);
}

void kInitVirtualMemory(FrameBuffer *frameBuffer) {
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
    for (page = 0; page < memorySize + pageSize; page += pageSize)
        virtualAddressManager->Map((void *)page, (void *)page);
    // Frame buffer might lie outside of memory space, so make sure it's mapped into virtual memory as well.
    for (page = (uint64_t)frameBuffer->BaseAddress; page < frameBuffer->Size + pageSize; page += pageSize)
        KernelVirtualAddressManager.Map((void *)page, (void *)page);

    // And activate our virtual memory map.
    KernelVirtualAddressManager.Activate();
}

void kInitFrameBuffer(FrameBuffer *frameBuffer)
{
    KernelFrameBuffer::InitializeInstance(frameBuffer);
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
    static uint64_t initialized = 0;
    if(initialized) return; // This will be replaced with panic, once written.
    initialized = 1;
    kInitGlobalDesciptorTable();
    kInitMemory(kernelParameters->BootMemoryMap);
    kInitVirtualMemory(kernelParameters->FrameBuffer);
    kInitFrameBuffer(kernelParameters->FrameBuffer);   
    kInitConsoleFont(kernelParameters->Font);
    kInitInterrupts();
}