#include <stddef.h>
#include "../../common/include/kernelparameters.h"
#include "graphics/framebuffer.h"
#include "console/font.h"
#include "init.h"

#define RED 0x00FF0000
#define GREEN 0x0000FF00
#define BLUE 0x000000FF
#define BLACK 0x00000000
#define WHITE 0x00FFFFFF

void _start(KernelParameters *kernelParameters)
{
    kInitializeFromParameters(kernelParameters);

    kPutCharacterWithColor('H', 0, 0, GREEN, BLACK);
    kPutCharacterWithColor('e', 8, 0, GREEN, BLACK);
    kPutCharacterWithColor('l', 16, 0, GREEN, BLACK);
    kPutCharacterWithColor('l', 24, 0, GREEN, BLACK);
    kPutCharacterWithColor('o', 32, 0, GREEN, BLACK);

    while(1);
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