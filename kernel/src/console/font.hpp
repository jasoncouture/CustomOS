#pragma once
#include "../graphics/framebuffer.hpp"
#include "../../../common/include/font.h"
#include "zap-light-16.hpp"

class KernelConsoleFont {
    private:
        unsigned int foregroundColor = 0x0000FF00;
        unsigned int backgroundColor = 0x00000000;
        KernelFrameBuffer* surface;
        Font* font;

        KernelConsoleFont();
        static Font FontInstance;
        static KernelConsoleFont Instance;
    
    public:
        static KernelConsoleFont* InitializeInstance();
        static KernelConsoleFont* InitializeInstance(Font* font);
        static KernelConsoleFont* GetInstance();

        KernelConsoleFont(Font* font, KernelFrameBuffer* kernelFrameBuffer);
        
        void SetForegroundColor(unsigned int foregroundColor);
        void SetBackgroundColor(unsigned int backgroundColor);

        unsigned int GetCharacterPixelWidth();
        unsigned int GetCharacterPixelHeight();

        void DrawCharacterAt(const unsigned char character, const unsigned int x, const unsigned int y);
        void DrawCharacterAt(const unsigned char character, const unsigned int x, const unsigned int y, const unsigned int color);
        void DrawCharacterAt(const unsigned char character, const unsigned int x, const unsigned int y, const unsigned int foregroundColor, const unsigned int backgroundColor);

        void DrawStringAt(const char* character, const unsigned int x, const unsigned int y);
        void DrawStringAt(const char* character, const unsigned int x, const unsigned int y, const unsigned int color);
        void DrawStringAt(const char* character, const unsigned int x, const unsigned int y, const unsigned int foregroundColor, const unsigned int backgroundColor);
        

};