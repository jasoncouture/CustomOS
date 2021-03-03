#include <stddef.h>
#include "../../common/include/kernelparameters.h"
#include "graphics/framebuffer.h"







void _start(KernelParameters *kernelParameters)
{
    kInitializeFrameBuffer(kernelParameters->FrameBuffer);
    // Color format is RESERVED- B G R, each is 8 bits
    // This is converted during the function call DirectWritePixel for GOP pixel mode 1 and 2 (See graphics/framebuffer.c)
    // Othermodes will result in framebuffer operations becoming a no-op. Because I'm far too lazy to implement anything
    // that isn't 24/32 bit color with 8BPP
    unsigned int colors[3] = {0xFF00FF00, 0xFF0000FF, 0xFFFF0000};
    unsigned int colorIndex = 0;
    while (1)
    {
        unsigned int selectedColor = colors[colorIndex];
        colorIndex = (colorIndex + 1) % 3;
        for (unsigned int y = 0; y < kernelParameters->FrameBuffer->Height; y++)
        {
            for (unsigned int x = 0; x < kernelParameters->FrameBuffer->Width; x++)
            {
                kSetPixel(x, y, selectedColor);
            }
        }
    }
}