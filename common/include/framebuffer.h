#pragma once
typedef struct
{
	void *BaseAddress;
	unsigned long long Size;
	unsigned int Width;
	unsigned int Height;
	unsigned int PixelsPerScanLine;
	unsigned int PixelFormat;
}__attribute__((__packed__)) FrameBuffer;