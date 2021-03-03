#include <stddef.h>
#include "../../common/include/kernelparameters.h"
#include "graphics/framebuffer.h"


void DirectWritePixel(uint8_t* buffer, uint8_t* colorDataBuffer, KernelFrameBuffer* kernelFrameBuffer) 
{
    if((kernelFrameBuffer->RedPosition / 8) < kernelFrameBuffer->BytesPerPixel) 
    {
        buffer[kernelFrameBuffer->RedPosition / 8] = colorDataBuffer[0]; // Red
    }
    if((kernelFrameBuffer->GreenPosition / 8) < kernelFrameBuffer->BytesPerPixel) 
    {
        buffer[kernelFrameBuffer->GreenPosition / 8] = colorDataBuffer[1]; // Red
    }
    if((kernelFrameBuffer->BluePosition / 8) < kernelFrameBuffer->BytesPerPixel) 
    {
        buffer[kernelFrameBuffer->BluePosition / 8] = colorDataBuffer[2]; // Red
    }
}

void DrawPixel(unsigned int x, unsigned int y, unsigned int color)
{
    KernelFrameBuffer *kernelFrameBuffer = kGetFrameBuffer();
    FrameBuffer* frameBuffer = kernelFrameBuffer->FrameBuffer;
    if (frameBuffer == NULL || frameBuffer->BaseAddress == NULL || kernelFrameBuffer->BytesPerPixel == 0)
        return;
    unsigned long long xOffset = x * kernelFrameBuffer->BytesPerPixel;
    // y * width + x = offset
    // In this case, * bytes per pixel
    // Cast to a unsigned int (32 bit) pointer
    // Dereferenced, and assigned the value provided
    uint8_t *colorData = &color;
    uint8_t *buffer = (uint8_t *)(xOffset + (y * kernelFrameBuffer->BytesPerPixel * frameBuffer->PixelsPerScanLine) + frameBuffer->BaseAddress);

    DirectWritePixel(buffer, colorData, kernelFrameBuffer);
}




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
                DrawPixel(x, y, selectedColor);
            }
        }
    }
}