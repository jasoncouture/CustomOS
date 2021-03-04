#include <stddef.h>
#include "../../common/include/kernelparameters.h"
#include "graphics/framebuffer.hpp"
#include "console/font.hpp"
#include "init.h"

#define RED 0x000000FF
#define GREEN 0x0000FF00
#define BLUE 0x00FF0000
#define BLACK 0x00000000
#define WHITE 0x00FFFFFF

extern "C" void _start(KernelParameters *kernelParameters)
{
    kInitializeFromParameters(kernelParameters);
    KernelFrameBuffer *frameBuffer = KernelFrameBuffer::GetInstance();
    KernelConsoleFont *font = KernelConsoleFont::GetInstance();
    font->DrawCharacterAt('H', 0, 0, GREEN, BLACK);
    frameBuffer->Clear(BLUE);
    font->DrawCharacterAt('e', 8, 0, GREEN, BLACK);
    font->DrawCharacterAt('l', 16, 0, GREEN, BLACK);
    font->DrawCharacterAt('l', 24, 0, GREEN, BLACK);
    font->DrawCharacterAt('o', 32, 0, GREEN, BLACK);
    frameBuffer->Clear(GREEN);
    while(1);
}