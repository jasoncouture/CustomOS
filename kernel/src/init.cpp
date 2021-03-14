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
#include "interrupts/apic.hpp"
#include "memory/heap.hpp"


void kInitGlobalDesciptorTable()
{
    auto globalDescriptorLocation = (GlobalDescriptorLocation*)kmalloc(sizeof(GlobalDescriptorLocation));
    globalDescriptorLocation->Size = (uint16_t)(sizeof(GlobalDesciptorTable)) - 1;
    globalDescriptorLocation->GlobalDescriptorTable = &DefaultGlobalDesciptorTable;
    LoadGlobalDescriptorTable(globalDescriptorLocation);
}

void kInitMemory(BootMemoryMap *bootMemoryMap)
{
    Memory::Initialize(bootMemoryMap);
}

void *PageToAddress(uint64_t page, uint64_t pageSize)
{
    return (void *)(page * pageSize);
}

void kInitPageManager(FrameBuffer *frameBuffer) 
{
    auto memory = Memory::GetInstance();
    auto pageAllocator = PageAllocator::Initialize(memory);
    auto memoryMap = memory->GetBootMemoryMap();

    // Move the framebuffer from "used" (if it was there) to "reserved" memory
    pageAllocator->FreePages(frameBuffer->BaseAddress, (frameBuffer->Size / pageAllocator->PageSize()));
    pageAllocator->ReservePages(frameBuffer->BaseAddress, (frameBuffer->Size / pageAllocator->PageSize()));
    pageAllocator->ReservePages((void*)0, 4096); // Reserve the first 1MB of ram, it seems EFI doesn't report this in the memory map, but when we write to
}

void kInitVirtualMemory(FrameBuffer *frameBuffer)
{
    auto memory = Memory::GetInstance();
    // Setup the virtual memory manager.
    
    uint64_t pageSize = memory->PageSize();
    uint64_t memorySize = memory->Size();
    uint64_t memoryPageSize = memorySize / pageSize;
    uint64_t frameBufferStart = ((uint64_t)frameBuffer->BaseAddress / pageSize);
    uint64_t frameBufferPageCount = frameBuffer->Size / pageSize;
    uint64_t page = 0;

    if (frameBuffer->Size % pageSize)
        frameBufferPageCount++;

    if (memorySize % pageSize)
        memoryPageSize++;
    auto virtualAddressManager = VirtualAddressManager::GetKernelVirtualAddressManager();   
    // Frame buffer might lie outside of memory space, so make sure it's mapped into virtual memory.
    for (page = 0; page < frameBufferPageCount; page++)
        virtualAddressManager->Map(PageToAddress(page + frameBufferStart, pageSize));
    // Identity map all memory.
    for (page = 0; page < memoryPageSize; page++)
        virtualAddressManager->Map(PageToAddress(page, pageSize), page != 0);

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

    InterruptDescriptorTableEntry *doubleFaultEntry = interruptDesciptorTableLocation.InterruptDesciptors + 0x8;
    doubleFaultEntry->SetOffset((uint64_t)Interrupt_DoubleFaultHandler);
    doubleFaultEntry->TypeAndAttribute = IDT_TYPEATTRIBUTE_INTERRUPTGATE;
    doubleFaultEntry->Selector = 0x08; // Kernel code segment selector (See GDT);

    InterruptDescriptorTableEntry *generalProtectionFaultEntry = interruptDesciptorTableLocation.InterruptDesciptors + 0xD;
    generalProtectionFaultEntry->SetOffset((uint64_t)Interrupt_GeneralProtectionFault);
    generalProtectionFaultEntry->TypeAndAttribute = IDT_TYPEATTRIBUTE_INTERRUPTGATE;
    generalProtectionFaultEntry->Selector = 0x08; // Kernel code segment selector (See GDT);

    InterruptDescriptorTableEntry *keyboardEntry = interruptDesciptorTableLocation.InterruptDesciptors + 0x21;
    keyboardEntry->SetOffset((uint64_t)Interrupt_KeyboardInput);
    keyboardEntry->TypeAndAttribute = IDT_TYPEATTRIBUTE_INTERRUPTGATE;
    keyboardEntry->Selector = 0x08;


    // Load IDT
    asm("lidt %0"
        :
        : "m"(interruptDesciptorTableLocation));
}

void kInitApic() 
{
    InitPorts();
    InitApic();
    // Unmask PIC Interrupts.
    PIC1DataPort->Write(0b11111101);
    PIC2DataPort->Write(0b11111111);
}

void kInitHeap() 
{
    InitializeHeap(VirtualAddressManager::GetKernelVirtualAddressManager(), PageAllocator::GetInstance());
}

void kInit(KernelParameters *kernelParameters)
{
    DisableInterrupts();
    kInitFrameBuffer(kernelParameters->FrameBuffer);
    kInitConsoleFont(kernelParameters->Font);
    kInitGlobalDesciptorTable();
    auto kernelFrameBuffer = KernelFrameBuffer::GetInstance();
    auto consoleFont = KernelConsoleFont::GetInstance();
    
    consoleFont->DrawStringAt("Framebuffer online, initializing memory", 0, consoleFont->GetCharacterPixelHeight() * 19);



    kInitMemory(kernelParameters->BootMemoryMap);
    consoleFont->DrawStringAt("Memory map loaded, building page map", 0, consoleFont->GetCharacterPixelHeight() * 20);
    kInitPageManager(kernelParameters->FrameBuffer);
    kInitVirtualMemory(kernelParameters->FrameBuffer);
    kInitHeap();
    // Restore the messages we just cleared.
    consoleFont->DrawStringAt("Framebuffer online, initializing memory", 0, consoleFont->GetCharacterPixelHeight() * 19);
    consoleFont->DrawStringAt("Memory map loaded, building page map", 0, consoleFont->GetCharacterPixelHeight() * 20);
    consoleFont->DrawStringAt("Memory initialized, initializing Interrupts", 0, consoleFont->GetCharacterPixelHeight() * 21);
    kInitInterrupts();
    kInitApic();
    EnableInterrupts();
    consoleFont->DrawStringAt("Interrupts Initialized.", 0, consoleFont->GetCharacterPixelHeight() * 22);
}

extern "C" void __entry(KernelParameters *kernelParameters)
{
    kInit(kernelParameters);
    kMain(kernelParameters);
}
