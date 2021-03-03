#pragma once
#include "framebuffer.h"
#include "font.h"

typedef struct
{
	FrameBuffer *FrameBuffer;
	Font *Font;
} __attribute__((__packed__)) KernelParameters;