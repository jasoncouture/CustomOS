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

#define RED 0x000000FF
#define GREEN 0x0000FF00
#define BLUE 0x00FF0000
#define BLACK 0x00000000
#define WHITE 0x00FFFFFF

extern "C" void _start(KernelParameters *kernelParameters)
{
    kInit(kernelParameters);
    auto pageAllocator = PageAllocator::GetInstance();
    auto memory = Memory::GetInstance();
    auto font = KernelConsoleFont::GetInstance();
    auto freeMemoryInfo = pageAllocator->GetFreeMemoryInformation();
    font->DrawStringAt("Booting kernel (Early init)", 0, font->GetCharacterPixelHeight() * 0);
    font->DrawStringAt("Frame buffer initialized and console font loaded", 0, font->GetCharacterPixelHeight() * 1);
    font->DrawStringAt("Total system memory:", 0, font->GetCharacterPixelHeight() * 2);
    font->DrawStringAt(kToString(memory->Size()), 26 * 8, font->GetCharacterPixelHeight() * 2);
    font->DrawStringAt(kToString(freeMemoryInfo.BytesUsed), 0, font->GetCharacterPixelHeight() * 3);

    asm("hlt");
}