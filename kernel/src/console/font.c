#include "font.h"
#include "../graphics/framebuffer.h"
#include "../../../common/include/font.h"

KernelConsoleFontInfo gKernelConsoleFontInfo;

KernelConsoleFontInfo *kGetConsoleFontInfo()
{
    return &gKernelConsoleFontInfo;
}

void kSetConsoleFont(Font *font)
{
    KernelConsoleFontInfo* consoleFontInfo = kGetConsoleFontInfo();
    consoleFontInfo->Font = font;
    consoleFontInfo->ForegroundColor = 0x0000FF00;
    consoleFontInfo->BackgroundColor = 0x00000000;
}

void kSetTextColor(unsigned int foregroundColor, unsigned int backgroundColor)
{
    KernelConsoleFontInfo* consoleFontInfo = kGetConsoleFontInfo();
    consoleFontInfo->ForegroundColor = foregroundColor;
    consoleFontInfo->BackgroundColor = backgroundColor;
}

void kPutCharacterWithColor(unsigned char character, unsigned int col, unsigned int row, unsigned int foregroundColor, unsigned int backgroundColor)
{
    KernelConsoleFontInfo* consoleFontInfo = kGetConsoleFontInfo();
    char *fontPointer = ((char*)consoleFontInfo->Font->GlyphBuffer) + ((unsigned long long)character * (unsigned long long)consoleFontInfo->Font->Header->CharacterSize);

    for (unsigned long y = row; y < (row + 16); y++)
    {
        for (unsigned long x = col; x < (col + 8); x++)
        {
            unsigned int selectedColor = (*fontPointer & (0b10000000 >> (x - col))) > 0 ? foregroundColor : backgroundColor;
            kSetPixel(x, y, selectedColor);
        }
        fontPointer++;
    }
}

void kPutCharacter(unsigned char character, unsigned int col, unsigned int row)
{
    KernelConsoleFontInfo* consoleFontInfo = kGetConsoleFontInfo();
    kPutCharacterWithColor(character, col, row, consoleFontInfo->ForegroundColor, consoleFontInfo->BackgroundColor);
}