#include <graphics/framebuffer.hpp>
#include <stddef.h>
#include <memory/heap.hpp>
#include <interrupts/interrupts.hpp>

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

uint64_t DeviceColorFromKernelColor(uint64_t kernelColor, KernelFrameBufferInfo *kernelFrameBufferInfo)
{
    uint64_t deviceColor = 0;
    uint8_t *buffer = (uint8_t *)(void *)&deviceColor;
    uint8_t *colorDataBuffer = (uint8_t *)(void *)&kernelColor;
    buffer[kernelFrameBufferInfo->RedPosition / 8] = colorDataBuffer[0];
    buffer[kernelFrameBufferInfo->GreenPosition / 8] = colorDataBuffer[1];
    buffer[kernelFrameBufferInfo->BluePosition / 8] = colorDataBuffer[2];
    return deviceColor;
}

void KernelFrameBuffer::DirectWritePixel(uint64_t *buffer, uint64_t deviceColor)
{
    *(uint32_t *)buffer = (uint32_t)deviceColor;
}

KernelFrameBuffer::KernelFrameBuffer(FrameBuffer *frameBuffer)
{

    this->kFrameBufferInfo = (KernelFrameBufferInfo *)malloc(sizeof(KernelFrameBufferInfo));
    InitializeKernelFrameBuffer(frameBuffer, this->kFrameBufferInfo);
    this->buffer = (uint32_t *)malloc(frameBuffer->Size);
    this->shadowBuffer = (uint32_t *)malloc(frameBuffer->Size);
    this->bufferDirty = false;
    this->FrameBufferLock = new Lock();
    memcopy(frameBuffer->BaseAddress, this->buffer, frameBuffer->Size);
    memcopy(this->buffer, this->shadowBuffer, frameBuffer->Size);
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
    uint64_t frameBufferOffset = (x * this->kFrameBufferInfo->BytesPerPixel) + (y * this->kFrameBufferInfo->BytesPerPixel * frameBuffer->PixelsPerScanLine);
    // If we'd go out of the framebuffer bounds, don't.
    if (frameBufferOffset + this->kFrameBufferInfo->BytesPerPixel >= frameBuffer->Size)
        return;
    auto interruptStatus = InterruptStatus();
    if (interruptStatus)
        DisableInterrupts();
    FrameBufferLock->SpinWait(false);
    uint64_t *buffer = (uint64_t *)((uint8_t *)this->buffer + frameBufferOffset);
    uint64_t deviceColor = DeviceColorFromKernelColor(color, this->kFrameBufferInfo);
    this->DirectWritePixel(buffer, deviceColor);
    if (interruptStatus)
        EnableInterrupts();
    FrameBufferLock->Unlock();
}

void KernelFrameBuffer::Update()
{
    this->bufferDirty = true;
}

bool KernelFrameBuffer::NeedsBufferSwap(bool fast)
{
    return this->bufferDirty;
}

void KernelFrameBuffer::SwapBuffers()
{
    auto temp = this->buffer;
    auto size = this->kFrameBufferInfo->FrameBuffer->PixelsPerScanLine * this->kFrameBufferInfo->BytesPerPixel * this->kFrameBufferInfo->FrameBuffer->Height;
    DisableInterrupts();
    // Update the shadow buffer inside a lock, with interrupts disabled.
    this->FrameBufferLock->SpinWait();
    memcopy8(this->buffer, this->shadowBuffer, size);
    this->FrameBufferLock->Unlock();
    EnableInterrupts();
    // Re-enable interrupts before writing to the framebuffer.
    memcopy32(this->shadowBuffer, this->kFrameBufferInfo->FrameBuffer->BaseAddress, size - sizeof(uint32_t));
    this->bufferDirty = false;
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
