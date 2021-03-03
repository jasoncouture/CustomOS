#pragma once
#include "../graphics/framebuffer.h"
#include "../../../common/include/font.h"

typedef struct {
    Font* Font;
    unsigned int ForegroundColor;
    unsigned int BackgroundColor;
} KernelConsoleFontInfo;

KernelConsoleFontInfo *kGetConsoleFontInfo();
void kSetConsoleFont(Font* font);
void kSetTextColor(unsigned int foregroundColor, unsigned int backgroundColor);
void kPutCharacter(unsigned char character, unsigned int col, unsigned int row);
void kPutCharacterWithColor(unsigned char character, unsigned int col, unsigned int row, unsigned int foregroundColor, unsigned int backgroundColor);