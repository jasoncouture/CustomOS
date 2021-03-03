#include <stddef.h>
#include "../../common/include/kernelparameters.h"

typedef unsigned int COLOR;

void DrawPixel(FrameBuffer *frameBuffer, unsigned int x, unsigned int y, COLOR color)
{
    if (frameBuffer == NULL || frameBuffer->BaseAddress == NULL)
        return;
    const unsigned long long bytesPerPixel = 4;
    unsigned int *bufferPointer = frameBuffer->BaseAddress;
    *(COLOR *)((x * bytesPerPixel) + (y * bytesPerPixel * frameBuffer->PixelsPerScanLine) + frameBuffer->BaseAddress) = color;
}

int _start(KernelParameters *kernelParameters)
{
    return kernelParameters->FrameBuffer->PixelsPerScanLine * kernelParameters->FrameBuffer->Height * 4;
    COLOR colors[] = {0xFF0000FF, 0x00FF00FF, 0x0000FFFF};
    unsigned int colorIndex = 0;
    FrameBuffer frameBuffer = *(kernelParameters->FrameBuffer);
    while (1)
    {
        colorIndex = (colorIndex + 1) % 3;
        COLOR selectedColor = colors[colorIndex];
        for (int y = 50; y < frameBuffer.Height - 60; y++)
        {
            for (int x = 50; x < frameBuffer.Width - 60; x++)
            {
                DrawPixel(&frameBuffer, x, y, selectedColor);
                for (int z = 0; z < 10000; z++)
                    ;
            }
        }
    }
    // UNREACHABLE CODE DETECTED?!?! WHAT WILL WE EVER DO!?!?!
    return 124;
}