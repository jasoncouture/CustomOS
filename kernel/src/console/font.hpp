#pragma once
#include "../graphics/framebuffer.hpp"
#include "../../../common/include/font.h"
class KernelConsoleFont {
    private:
        unsigned int foregroundColor = 0x0000FF00;
        unsigned int backgroundColor = 0x00000000;
        Font* font = 0;
        KernelFrameBuffer* surface;

        KernelConsoleFont();
        static KernelConsoleFont instance;
    
    public:
        static void InitializeInstance(Font* font, KernelFrameBuffer* kernelFrameBuffer);
        static KernelConsoleFont* GetInstance();

        KernelConsoleFont(Font* font, KernelFrameBuffer* kernelFrameBuffer);
        
        void SetForegroundColor(unsigned int foregroundColor);
        void SetBackgroundColor(unsigned int backgroundColor);

        unsigned int GetCharacterPixelWidth();
        unsigned int GetCharacterPixelHeight();

        void DrawCharacterAt(unsigned char character, unsigned int x, unsigned int y);
        void DrawCharacterAt(unsigned char character, unsigned int x, unsigned int y, unsigned int color);
        void DrawCharacterAt(unsigned char character, unsigned int x, unsigned int y, unsigned int foregroundColor, unsigned int backgroundColor);
};