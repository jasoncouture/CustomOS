#pragma once
#include <stddef.h>
#include <stdint.h>
#include "../../../common/include/framebuffer.h"

typedef struct
{
    FrameBuffer *FrameBuffer;
    uint8_t BytesPerPixel;
    uint8_t RedSize;
    uint8_t RedPosition;
    uint8_t GreenSize;
    uint8_t GreenPosition;
    uint8_t BlueSize;
    uint8_t BluePosition;
} KernelFrameBuffer;


void kInitializeFrameBuffer(FrameBuffer* frameBuffer);

KernelFrameBuffer *kGetFrameBuffer();
void kSetPixel(unsigned int x, unsigned int y, unsigned int color);