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

GlobalDescriptorLocation globalDescriptorLocation;

void kInitGlobalDesciptorTable()
{
    globalDescriptorLocation.Size = (uint16_t)(sizeof(GlobalDesciptorTable)) - 1;
    globalDescriptorLocation.GlobalDescriptorTable = &DefaultGlobalDesciptorTable;
    LoadGlobalDescriptorTable(&globalDescriptorLocation);
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

}

void kInitVirtualMemory(FrameBuffer *frameBuffer)
{
    auto memory = Memory::GetInstance();
    auto pageAllocator = PageAllocator::Initialize(memory);
    auto memoryMap = memory->GetBootMemoryMap();

    // Move the framebuffer from "used" (if it was there) to "reserved" memory
    pageAllocator->FreePages(frameBuffer->BaseAddress, (frameBuffer->Size / pageAllocator->PageSize()));
    pageAllocator->ReservePages(frameBuffer->BaseAddress, (frameBuffer->Size / pageAllocator->PageSize()));
    // Setup the virtual memory manager.
    
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
    auto virtualAddressManager = VirtualAddressManager::GetKernelVirtualAddressManager();   
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
    InitApic();
    // Unmask PIC Interrupts.
    PIC1DataPort.Write(0b11111101);
    PIC2DataPort.Write(0b11111111);
}

void kInit(KernelParameters *kernelParameters)
{
    DisableInterrupts();
    kInitFrameBuffer(kernelParameters->FrameBuffer);
    kInitConsoleFont(kernelParameters->Font);
    auto kernelFrameBuffer = KernelFrameBuffer::GetInstance();
    auto consoleFont = KernelConsoleFont::GetInstance();
    
    consoleFont->DrawStringAt("Framebuffer online, initializing memory", 0, consoleFont->GetCharacterPixelHeight() * 19);



    kInitMemory(kernelParameters->BootMemoryMap);
    consoleFont->DrawStringAt("Memory map loaded, building page map", 0, consoleFont->GetCharacterPixelHeight() * 20);
    kInitPageManager(kernelParameters->FrameBuffer);
    kInitVirtualMemory(kernelParameters->FrameBuffer);
    kernelFrameBuffer->Clear(0);
    // Restore the messages we just cleared.
    consoleFont->DrawStringAt("Framebuffer online, initializing memory", 0, consoleFont->GetCharacterPixelHeight() * 19);
    consoleFont->DrawStringAt("Memory map loaded, building page map", 0, consoleFont->GetCharacterPixelHeight() * 20);
    consoleFont->DrawStringAt("Memory initialized, initializing GDT", 0, consoleFont->GetCharacterPixelHeight() * 21);
    
    kInitGlobalDesciptorTable();
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
