#pragma once
struct FrameBuffer
{
	void *BaseAddress;
	unsigned long long Size;
	unsigned int Width;
	unsigned int Height;
	unsigned int PixelsPerScanLine;
	unsigned int PixelFormat;
};