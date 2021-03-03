#include "framebuffer.h"
#include <stddef.h>
#define PIXEL_FORMAT_RGB_RESERVED_8BIT_PER_COLOR 0
#define PIXEL_FORMAT_BGR_RESERVED_8BIT_PER_COLOR 1
#define PIXEL_BIT_MASK 2
#define PIXEL_BLT_ONLY 3
#define PIXEL_FORMAT_MAX 4

KernelFrameBuffer gKernelFrameBuffer;

void kInitializeFrameBuffer(FrameBuffer *frameBuffer)
{
    gKernelFrameBuffer.FrameBuffer = frameBuffer;
    if (gKernelFrameBuffer.FrameBuffer == NULL)
    {
        gKernelFrameBuffer.BytesPerPixel = 0;
        return;
    }
    switch (frameBuffer->PixelFormat)
    {
    case PIXEL_FORMAT_RGB_RESERVED_8BIT_PER_COLOR:

        gKernelFrameBuffer.RedPosition = 0;
        gKernelFrameBuffer.RedSize = 8;

        gKernelFrameBuffer.GreenPosition = 8;
        gKernelFrameBuffer.GreenSize = 8;

        gKernelFrameBuffer.BluePosition = 16;
        gKernelFrameBuffer.BlueSize = 8;

        gKernelFrameBuffer.BytesPerPixel = 4;
        break;
    case PIXEL_FORMAT_BGR_RESERVED_8BIT_PER_COLOR:

        gKernelFrameBuffer.BluePosition = 0;
        gKernelFrameBuffer.BlueSize = 8;

        gKernelFrameBuffer.GreenPosition = 8;
        gKernelFrameBuffer.GreenSize = 8;

        gKernelFrameBuffer.RedPosition = 16;
        gKernelFrameBuffer.RedSize = 8;

        gKernelFrameBuffer.BytesPerPixel = 4;
        break;
    case PIXEL_BIT_MASK:
        // I shudder to think that i'll need to implement this someday.
        gKernelFrameBuffer.BytesPerPixel = 0;
        break;
    default:
        // And I guess you're not getting graphics?
        gKernelFrameBuffer.BytesPerPixel = 0;
        break;
    }
}

KernelFrameBuffer *kGetFrameBuffer()
{
    return &gKernelFrameBuffer;
}


void DirectWritePixel(uint8_t* buffer, uint8_t* colorDataBuffer, KernelFrameBuffer* kernelFrameBuffer) 
{
    if((kernelFrameBuffer->RedPosition / 8) < kernelFrameBuffer->BytesPerPixel) 
    {
        buffer[kernelFrameBuffer->RedPosition / 8] = colorDataBuffer[0];
    }
    if((kernelFrameBuffer->GreenPosition / 8) < kernelFrameBuffer->BytesPerPixel) 
    {
        buffer[kernelFrameBuffer->GreenPosition / 8] = colorDataBuffer[1];
    }
    if((kernelFrameBuffer->BluePosition / 8) < kernelFrameBuffer->BytesPerPixel) 
    {
        buffer[kernelFrameBuffer->BluePosition / 8] = colorDataBuffer[2];
    }
}

void kSetPixel(unsigned int x, unsigned int y, unsigned int color)
{
    KernelFrameBuffer *kernelFrameBuffer = &gKernelFrameBuffer;
    FrameBuffer* frameBuffer = kernelFrameBuffer->FrameBuffer;
    if (frameBuffer == NULL || frameBuffer->BaseAddress == NULL || kernelFrameBuffer->BytesPerPixel == 0)
        return;
    unsigned long long xOffset = x * kernelFrameBuffer->BytesPerPixel;
    // y * width + x = offset
    // In this case, * bytes per pixel
    // Cast to a unsigned int (32 bit) pointer
    // Dereferenced, and assigned the value provided
    void *colorData = &color;
    void *buffer = (uint8_t *)(xOffset + (y * kernelFrameBuffer->BytesPerPixel * frameBuffer->PixelsPerScanLine) + frameBuffer->BaseAddress);

    DirectWritePixel(buffer, colorData, kernelFrameBuffer);
}