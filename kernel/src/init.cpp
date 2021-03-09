#include <stddef.h>
#include <kernelparameters.h>
#include "init.hpp"
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
    globalDescriptorLocation.Size = (uint16_t)(sizeof(GlobalDesciptorTable)) - 1;
    globalDescriptorLocation.GlobalDescriptorTable = &DefaultGlobalDesciptorTable;
    LoadGlobalDescriptorTable(&globalDescriptorLocation);
}

void kInitMemory(BootMemoryMap *bootMemoryMap)
{
    auto memory = Memory::Initialize(bootMemoryMap);
    PageAllocator::Initialize(memory);
}

void *PageToAddress(uint64_t page, uint64_t pageSize)
{
    return (void *)(page * pageSize);
}

void kInitVirtualMemory(FrameBuffer *frameBuffer)
{
    auto pageAllocator = PageAllocator::GetInstance();
    auto memory = Memory::GetInstance();
    auto memoryMap = memory->GetBootMemoryMap();

    // Move the framebuffer from "used" (if it was there) to "reserved" memory
    pageAllocator->FreePages(frameBuffer->BaseAddress, (frameBuffer->Size / pageAllocator->PageSize()));
    pageAllocator->ReservePages(frameBuffer->BaseAddress, (frameBuffer->Size / pageAllocator->PageSize()));
    // Setup the virtual memory manager.
    auto virtualAddressManager = VirtualAddressManager::GetKernelVirtualAddressManager();
    uint64_t pageSize = memory->PageSize();
    uint64_t memorySize = memory->Size();
    uint64_t memoryPageSize = memorySize / pageSize;
    uint64_t frameBufferStart = ((uint64_t)frameBuffer->BaseAddress / pageSize) * pageSize;
    uint64_t frameBufferPageCount = frameBuffer->Size / pageSize;
    uint64_t page = 0;

    if (frameBuffer->Size % pageSize)
        frameBufferPageCount++;

    if (memorySize % pageSize)
        memoryPageSize++;

    // Identity map all memory.
    for (page = 0; page < memoryPageSize; page++)
        virtualAddressManager->Map(PageToAddress(page, pageSize), PageToAddress(page, pageSize));

    // Frame buffer might lie outside of memory space, so make sure it's mapped into virtual memory as well.
    for (page = 0; page < frameBufferPageCount; page++)
        virtualAddressManager->Map(PageToAddress(page + frameBufferStart, pageSize), PageToAddress(page + frameBufferStart, pageSize));

    uint64_t entries = memoryMap->MemoryMapSize / memoryMap->MemoryMapDescriptorSize;
    for (uint64_t i = 0; i < entries; i++)
    {
        BootMemoryDescriptor *descriptor = (BootMemoryDescriptor *)((uint64_t)memoryMap->MemoryMap + (i * memoryMap->MemoryMapDescriptorSize));
        if(descriptor->PageCount == 0 || (uint64_t)descriptor->PhysicalAddress == 0xffffffffffffffff) continue;
        virtualAddressManager->Map(memoryMap->MemoryMap->VirtualAddress, memoryMap->MemoryMap->PhysicalAddress);
    }

    // And activate our virtual memory map.
    virtualAddressManager->Activate();
}

void kInitFrameBuffer(FrameBuffer *frameBuffer)
{
    auto kernelFrameBuffer = KernelFrameBuffer::InitializeInstance(frameBuffer);
    //kernelFrameBuffer->Clear(0x00ff0000);
}

void kInitConsoleFont(Font *font)
{
    KernelConsoleFont::InitializeInstance(font);
}

InterruptDesciptorTableLocation interruptDesciptorTableLocation;

void kInitInterrupts()
{
    interruptDesciptorTableLocation.Limit = 0x0FFF; // Maximum number of entries;
    interruptDesciptorTableLocation.InterruptDesciptors = (InterruptDescriptorTableEntry *)PageAllocator::GetInstance()->AllocatePage();

    InterruptDescriptorTableEntry *pageFaultEntry = interruptDesciptorTableLocation.InterruptDesciptors + 0xE;
    pageFaultEntry->SetOffset((uint64_t)Interrupt_PageFaultHandler);
    pageFaultEntry->TypeAndAttribute = IDT_TYPEATTRIBUTE_INTERRUPTGATE;
    pageFaultEntry->Selector = 0x08; // Kernel code segment selector (See GDT);
    // Load IDT
    asm("lidt %0"
        :
        : "m"(interruptDesciptorTableLocation));
}

void kInit(KernelParameters *kernelParameters)
{
    kInitGlobalDesciptorTable();
    kInitFrameBuffer(kernelParameters->FrameBuffer);
    kInitMemory(kernelParameters->BootMemoryMap);
    kInitVirtualMemory(kernelParameters->FrameBuffer);
    
    kInitConsoleFont(kernelParameters->Font);
    auto kernelFrameBuffer = KernelFrameBuffer::GetInstance();
    auto consoleFont = KernelConsoleFont::GetInstance();
    consoleFont->DrawStringAt("Frame buffer initialized, initializing GDT", 0, consoleFont->GetCharacterPixelHeight() * 20);
    kInitGlobalDesciptorTable();
    consoleFont->DrawStringAt("GDT Initialized.", 0, consoleFont->GetCharacterPixelHeight() * 21);
    kInitInterrupts();
    consoleFont->DrawStringAt("Interrupts Initialized.", 0, consoleFont->GetCharacterPixelHeight() * 22);
}

extern "C" void __entry(struct KernelParameters *kernelParameters)
{
    kInit(kernelParameters);
    kMain(kernelParameters);
}
