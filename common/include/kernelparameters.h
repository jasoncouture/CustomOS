#pragma once
#include "framebuffer.h"
#include "font.h"

struct KernelParameters
{
	struct FrameBuffer *FrameBuffer;
	struct Font *Font;
};