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
    // KernelVirtualAddressManager = VirtualAddressManager();
    // uint64_t pageSize = memory->PageSize();
    // uint64_t page = 0;
    // for(; page < memory->Size(); page += pageSize) 
    // {
    //     // Identity map all of memory.
    //     KernelVirtualAddressManager.Map((void*)page, (void*)page);    
    // }
    // // Frame buffer might lie outside of memory space, so make sure it's mapped into virtual memory as well.
    // for(page = (uint64_t)kernelParameters->FrameBuffer->BaseAddress; page < kernelParameters->FrameBuffer->Size + pageSize; page += pageSize);
    // {
    //     KernelVirtualAddressManager.Map((void*)page, (void*)page);
    // }


    // And activate our virtual memory map. 
    //KernelVirtualAddressManager.Activate();
    // At this point, if the frame buffer initializes, we now have virtual memory addressing. For the kernel, this is identity mapped. But for processes it won't be.

    KernelFrameBuffer::InitializeInstance(kernelParameters->FrameBuffer, pageAllocator);
    auto font = KernelConsoleFont::InitializeInstance(kernelParameters->Font);
    auto freeMemoryInfo = pageAllocator->GetFreeMemoryInformation();
    font->DrawStringAt("Booting kernel (Early init)", 0, font->GetCharacterPixelHeight() * 0);
    font->DrawStringAt("Frame buffer initialized and console font loaded", 0, font->GetCharacterPixelHeight() * 1);
    font->DrawStringAt("Total system memory:", 0, font->GetCharacterPixelHeight() * 2);
    font->DrawStringAt(kToString(memory->Size()), 26*8, font->GetCharacterPixelHeight() * 2);
    font->DrawStringAt(kToString(freeMemoryInfo.BytesUsed), 0, font->GetCharacterPixelHeight() * 3);
    asm( "hlt" );
}