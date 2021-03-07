#include <stddef.h>
#include <kernelparameters.h>
#include "graphics/framebuffer.hpp"
#include "console/font.hpp"
#include "memory/memory.hpp"
#include "memory/pageallocator.hpp"
#include "console/cstr.hpp"
#include "memory/paging/virtualaddressmanager.hpp"

#define RED 0x000000FF
#define GREEN 0x0000FF00
#define BLUE 0x00FF0000
#define BLACK 0x00000000
#define WHITE 0x00FFFFFF

extern "C" void _start(KernelParameters *kernelParameters)
{
    auto memory = Memory::Initialize(kernelParameters->BootMemoryMap);
    auto pageAllocator = PageAllocator::Initialize(memory);
    pageAllocator->ReservePages(kernelParameters->FrameBuffer->BaseAddress, (kernelParameters->FrameBuffer->Size / pageAllocator->PageSize()));
    auto freeMemoryBeforeVirtualMemoryAddressing = pageAllocator->GetFreeMemoryInformation();
    KernelVirtualAddressManager = VirtualAddressManager();
    auto virtualAddressManager = &KernelVirtualAddressManager;
    uint64_t pageSize = memory->PageSize();
    uint64_t memorySize = memory->Size();
    uint64_t page = 0;
    for (page = 0; page < memorySize; page += pageSize)
    {
        // Identity map all of memory.
        virtualAddressManager->Map((void *)page, (void *)page);
    }
    // Frame buffer might lie outside of memory space, so make sure it's mapped into virtual memory as well.
    for (page = (uint64_t)kernelParameters->FrameBuffer->BaseAddress; page < kernelParameters->FrameBuffer->Size + pageSize; page += pageSize)
    {
        KernelVirtualAddressManager.Map((void *)page, (void *)page);
    }

    auto freeMemoryAfterVirtualMemoryAddressing = pageAllocator->GetFreeMemoryInformation();

    auto bytesUsedForKernelMemoryMap = freeMemoryBeforeVirtualMemoryAddressing.BytesFree - freeMemoryAfterVirtualMemoryAddressing.BytesFree;

    // And activate our virtual memory map.
    KernelVirtualAddressManager.Activate();
    // At this point, if we can write to the frame buffer, virtual memory addressing is working. The kernel needs memory identity mapped (VirtualAddress == PhysicalAddress)
    // but processes will not be identity mapped.
    KernelFrameBuffer::InitializeInstance(kernelParameters->FrameBuffer);
    auto font = KernelConsoleFont::InitializeInstance(kernelParameters->Font);
    auto freeMemoryInfo = pageAllocator->GetFreeMemoryInformation();
    font->DrawStringAt("Booting kernel (Early init)", 0, font->GetCharacterPixelHeight() * 0);
    font->DrawStringAt("Frame buffer initialized and console font loaded", 0, font->GetCharacterPixelHeight() * 1);
    font->DrawStringAt("Total system memory:", 0, font->GetCharacterPixelHeight() * 2);
    font->DrawStringAt(kToString(memory->Size()), 26 * 8, font->GetCharacterPixelHeight() * 2);
    font->DrawStringAt(kToString(freeMemoryInfo.BytesUsed), 0, font->GetCharacterPixelHeight() * 3);
    font->DrawStringAt(kToString(bytesUsedForKernelMemoryMap), 0, font->GetCharacterPixelHeight() * 4);

    asm("hlt");
}