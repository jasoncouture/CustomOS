#include "font.hpp"
#include "../graphics/framebuffer.hpp"
#include "../../../common/include/font.h"

KernelConsoleFont KernelConsoleFont::instance;

KernelConsoleFont::KernelConsoleFont() {}

void KernelConsoleFont::InitializeInstance(Font *font, KernelFrameBuffer *kernelFrameBuffer)
{
    KernelConsoleFont::instance = KernelConsoleFont(font, kernelFrameBuffer);
}

KernelConsoleFont *KernelConsoleFont::GetInstance() { &KernelConsoleFont::instance; }

KernelConsoleFont::KernelConsoleFont(Font *font, KernelFrameBuffer *kernelFrameBuffer)
{
    font = font;
    surface = kernelFrameBuffer;
    foregroundColor = 0x0000FF00;
    backgroundColor = 0x00000000;
}

void KernelConsoleFont::SetForegroundColor(unsigned int foregroundColor) { this->foregroundColor = foregroundColor; }
void KernelConsoleFont::SetBackgroundColor(unsigned int backgroundColor) { this->backgroundColor = backgroundColor; }

unsigned int KernelConsoleFont::GetCharacterPixelWidth() { return 8; }
unsigned int KernelConsoleFont::GetCharacterPixelHeight() { return 16; }

void KernelConsoleFont::DrawCharacterAt(unsigned char character, unsigned int x, unsigned int y)
{
    this->DrawCharacterAt(character, x, y, this->foregroundColor);
}

void KernelConsoleFont::DrawCharacterAt(unsigned char character, unsigned int x, unsigned int y, unsigned int color)
{
    this->DrawCharacterAt(character, x, y, color, this->backgroundColor);
}

void KernelConsoleFont::DrawCharacterAt(unsigned char character, unsigned int xOffset, unsigned int yOffset, unsigned int foregroundColor, unsigned int backgroundColor)
{
    char *fontPointer = ((char *)font->GlyphBuffer) + ((unsigned long long)character * (unsigned long long)font->Header->CharacterSize);
    this->surface->Clear(0x000000FF);
    unsigned int characterHeight = GetCharacterPixelHeight();
    unsigned int characterWidth = GetCharacterPixelWidth();
    for (unsigned long y = 0; y < characterHeight; y++)
    {
        for (unsigned int x = 0; x < characterWidth; x++)
        {
            unsigned int selectedColor = (*fontPointer & (0b10000000 >> (x))) > 0 ? foregroundColor : backgroundColor;
            this->surface->SetPixel(x, y, selectedColor);
        }
        fontPointer++;
    }
    // for (unsigned long y = yOffset; yOffset < (yOffset + 16); y++)
    // {
    //     for (unsigned long x = xOffset; x < (xOffset + 8); x++)
    //     {
    //         unsigned int selectedColor = (*fontPointer & (0b10000000 >> (x - xOffset))) > 0 ? foregroundColor : backgroundColor;
    //         this->surface->SetPixel(x, y, selectedColor);
    //     }
    //     fontPointer++;
    // }
}