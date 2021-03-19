#pragma once
#include <console/font.hpp>
#include <math/point.hpp>
#include <graphics/framebuffer.hpp>

class KernelConsole 
{
    public:
    Point GetConsoleSize();
    void SetCursorPosition(uint64_t x, uint64_t y);
    Point GetCursorPosition();

    void PutCharacter(char c);
    void Clear();

    static KernelConsole* GetInstance();

    private:
    KernelConsole(KernelConsoleFont* font, KernelFrameBuffer *frameBuffer);
    void FixPosition();
    Point position;
    Point screenSize;
    uint32_t foregroundColor;
    uint32_t backgroundColor;
    KernelConsoleFont *font;
    KernelFrameBuffer *frameBuffer;
};