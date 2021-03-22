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
    void SetColor(uint32_t foregroundColor, uint32_t backgroundColor)
    {
        this->foregroundColor = foregroundColor;
        this->backgroundColor = backgroundColor;
    }
    uint32_t GetForegroundColor() { return this->foregroundColor; }
    uint32_t GetBackgroundColor() { return this->backgroundColor; }
    void PutCharacter(char c);
    void Clear();

    static KernelConsole *GetInstance();

private:
    KernelConsole(KernelConsoleFont *font, KernelFrameBuffer *frameBuffer);
    void FixPosition();
    Point position;
    Point screenSize;
    uint32_t foregroundColor;
    uint32_t backgroundColor;
    KernelConsoleFont *font;
    KernelFrameBuffer *frameBuffer;
};