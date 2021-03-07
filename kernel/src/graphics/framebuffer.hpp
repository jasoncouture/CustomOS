#pragma once
#include <stddef.h>
#include <stdint.h>
#include <framebuffer.h>
#include "../memory/pageallocator.hpp"

struct KernelFrameBufferInfo
{
    struct FrameBuffer *FrameBuffer;
    uint8_t BytesPerPixel;
    uint8_t RedSize;
    uint8_t RedPosition;
    uint8_t GreenSize;
    uint8_t GreenPosition;
    uint8_t BlueSize;
    uint8_t BluePosition;
};



class KernelFrameBuffer 
{
    private:
        KernelFrameBuffer();
        KernelFrameBufferInfo kFrameBufferInfo;
        static KernelFrameBuffer GlobalSurface;

        void DirectWritePixel(uint8_t *buffer, uint8_t *colorDataBuffer, KernelFrameBufferInfo *kernelFrameBuffer);

    public:
        KernelFrameBuffer(FrameBuffer* frameBuffer);
        unsigned int GetWidth();
        unsigned int GetHeight();
        void SetPixel(const unsigned int x, const unsigned int y, const unsigned int color);
        void Clear(const unsigned int color);
        static KernelFrameBuffer* GetInstance();
        static KernelFrameBuffer* InitializeInstance(FrameBuffer* frameBuffer);
};


void kInitializeFrameBuffer(FrameBuffer* frameBuffer);

