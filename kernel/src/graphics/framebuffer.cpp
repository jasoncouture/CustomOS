#include "framebuffer.hpp"
#include <stddef.h>
#include "../memory/heap.hpp"
#define PIXEL_FORMAT_RGB_RESERVED_8BIT_PER_COLOR 0
#define PIXEL_FORMAT_BGR_RESERVED_8BIT_PER_COLOR 1
#define PIXEL_BIT_MASK 2
#define PIXEL_BLT_ONLY 3
#define PIXEL_FORMAT_MAX 4

void InitializeKernelFrameBuffer(FrameBuffer *frameBuffer, KernelFrameBufferInfo *kernelFrameBuffer)
{
    kernelFrameBuffer->FrameBuffer = frameBuffer;
    if (kernelFrameBuffer->FrameBuffer == NULL)
    {
        kernelFrameBuffer->BytesPerPixel = 0;
        return;
    }
    switch (frameBuffer->PixelFormat)
    {
    case PIXEL_FORMAT_RGB_RESERVED_8BIT_PER_COLOR:

        kernelFrameBuffer->RedPosition = 0;
        kernelFrameBuffer->RedSize = 8;

        kernelFrameBuffer->GreenPosition = 8;
        kernelFrameBuffer->GreenSize = 8;

        kernelFrameBuffer->BluePosition = 16;
        kernelFrameBuffer->BlueSize = 8;

        kernelFrameBuffer->BytesPerPixel = 4;
        break;
    case PIXEL_FORMAT_BGR_RESERVED_8BIT_PER_COLOR:

        kernelFrameBuffer->BluePosition = 0;
        kernelFrameBuffer->BlueSize = 8;

        kernelFrameBuffer->GreenPosition = 8;
        kernelFrameBuffer->GreenSize = 8;

        kernelFrameBuffer->RedPosition = 16;
        kernelFrameBuffer->RedSize = 8;

        kernelFrameBuffer->BytesPerPixel = 4;
        break;
    case PIXEL_BIT_MASK:
        // I shudder to think that i'll need to implement this someday.
        kernelFrameBuffer->BytesPerPixel = 0;
        break;
    default:
        // And I guess you're not getting graphics?
        kernelFrameBuffer->BytesPerPixel = 0;
        break;
    }
}

KernelFrameBuffer *KernelFrameBuffer::GlobalSurface;

KernelFrameBuffer *KernelFrameBuffer::InitializeInstance(FrameBuffer *frameBuffer)
{
    GlobalSurface = new KernelFrameBuffer(frameBuffer);
    return GetInstance();
}

KernelFrameBuffer *KernelFrameBuffer::GetInstance()
{
    return KernelFrameBuffer::GlobalSurface;
}

void KernelFrameBuffer::DirectWritePixel(uint8_t *buffer, uint8_t *colorDataBuffer, KernelFrameBufferInfo *kernelFrameBuffer)
{
    buffer[kernelFrameBuffer->RedPosition / 8] = colorDataBuffer[0];
    buffer[kernelFrameBuffer->GreenPosition / 8] = colorDataBuffer[1];
    buffer[kernelFrameBuffer->BluePosition / 8] = colorDataBuffer[2];
}

KernelFrameBuffer::KernelFrameBuffer(FrameBuffer *frameBuffer)
{
    this->kFrameBufferInfo = (KernelFrameBufferInfo*)malloc(sizeof(KernelFrameBufferInfo));
    InitializeKernelFrameBuffer(frameBuffer, this->kFrameBufferInfo);
}

void KernelFrameBuffer::SetPixel(const unsigned int x, const unsigned int y, const unsigned int color)
{

    FrameBuffer *frameBuffer = this->kFrameBufferInfo->FrameBuffer;
    if (frameBuffer == NULL || frameBuffer->BaseAddress == NULL || this->kFrameBufferInfo->BytesPerPixel == 0)
        return;
    // y * width + x = offset
    // In this case, * bytes per pixel
    // Cast to a unsigned int (32 bit) pointer
    // Dereferenced, and assigned the value provided
    unsigned int localColor = color;
    uint8_t *colorData = (uint8_t *)&localColor;
    uint64_t frameBufferOffset = (x * this->kFrameBufferInfo->BytesPerPixel) + (y * this->kFrameBufferInfo->BytesPerPixel * frameBuffer->PixelsPerScanLine);
    // If we'd go out of the framebuffer bounds, don't.
    if (frameBufferOffset + this->kFrameBufferInfo->BytesPerPixel > frameBuffer->Size)
        return;
    uint8_t *buffer = (uint8_t *)frameBuffer->BaseAddress;

    DirectWritePixel(buffer + frameBufferOffset, colorData, this->kFrameBufferInfo);
}

void KernelFrameBuffer::Clear(const unsigned int color)
{
    for (unsigned int y = 0; y < this->kFrameBufferInfo->FrameBuffer->Height; y++)
    {
        for (unsigned int x = 0; x < this->kFrameBufferInfo->FrameBuffer->Width; x++)
        {
            this->SetPixel(x, y, color);
        }
    }
}

unsigned int KernelFrameBuffer::GetWidth()
{
    return this->kFrameBufferInfo->FrameBuffer->Width;
}

unsigned int KernelFrameBuffer::GetHeight()
{
    return this->kFrameBufferInfo->FrameBuffer->Height;
}
