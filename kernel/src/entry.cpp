#include <stddef.h>
#include "../../common/include/kernelparameters.h"
#include "graphics/framebuffer.hpp"
#include "console/font.hpp"

#define RED 0x000000FF
#define GREEN 0x0000FF00
#define BLUE 0x00FF0000
#define BLACK 0x00000000
#define WHITE 0x00FFFFFF

extern "C" void _start(KernelParameters *kernelParameters)
{
    KernelFrameBuffer::InitializeInstance(kernelParameters->FrameBuffer);
    KernelConsoleFont *font = KernelConsoleFont::InitializeInstance(kernelParameters->Font);
    font->DrawStringAt("Booting kernel (Early init)", 0, 0);
    font->DrawStringAt("Frame buffer initialized and console font loaded", 0, font->GetCharacterPixelHeight());
    while(1);
}