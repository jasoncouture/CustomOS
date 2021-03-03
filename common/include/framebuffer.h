#pragma once
typedef struct
{
	void *BaseAddress;
	unsigned long long Size;
	unsigned int Width;
	unsigned int Height;
	unsigned int PixelsPerScanLine;
}__attribute__((__packed__)) FrameBuffer;