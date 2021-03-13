#include "font.hpp"
#include "../graphics/framebuffer.hpp"

KernelConsoleFont* KernelConsoleFont::Instance = NULL;

KernelConsoleFont *KernelConsoleFont::InitializeInstance(Font* font) 
{
    KernelConsoleFont::Instance = new KernelConsoleFont(font, KernelFrameBuffer::GetInstance());
    return KernelConsoleFont::Instance;
}

KernelConsoleFont *KernelConsoleFont::GetInstance() { return KernelConsoleFont::Instance; }

KernelConsoleFont::KernelConsoleFont(Font *font, KernelFrameBuffer *kernelFrameBuffer)
{
    this->font = font;
    this->surface = kernelFrameBuffer;
    this->foregroundColor = 0x0000FF00;
    this->backgroundColor = 0x00000000;
}

void KernelConsoleFont::SetForegroundColor(unsigned int foregroundColor) { this->foregroundColor = foregroundColor; }
void KernelConsoleFont::SetBackgroundColor(unsigned int backgroundColor) { this->backgroundColor = backgroundColor; }

unsigned int KernelConsoleFont::GetCharacterPixelWidth() { return 8; }
unsigned int KernelConsoleFont::GetCharacterPixelHeight() { return 16; }

void KernelConsoleFont::DrawCharacterAt(const unsigned char character, const unsigned int x, const unsigned int y)
{
    this->DrawCharacterAt(character, x, y, this->foregroundColor);
}

void KernelConsoleFont::DrawCharacterAt(const unsigned char character, const unsigned int x, const unsigned int y, const unsigned int color)
{
    this->DrawCharacterAt(character, x, y, color, this->backgroundColor);
}

void KernelConsoleFont::DrawCharacterAt(const unsigned char character, const unsigned int xOffset, const unsigned int yOffset, const unsigned int characterForegroundColor, const unsigned int characterBackgroundColor)
{
    char *fontPointer = (char *)font->GlyphBuffer + (font->Header->CharacterSize * character);
    for (unsigned long y = 0; y < this->GetCharacterPixelHeight(); y++)
    {
        for (unsigned int x = 0; x < this->GetCharacterPixelWidth(); x++)
        {
            int bitState = (*fontPointer) & (0b10000000 >> x);
            if (bitState)
            {
                this->surface->SetPixel(
                    x + xOffset,
                    y + yOffset,
                    characterForegroundColor);
            }
            else
            {
                this->surface->SetPixel(
                    x + xOffset,
                    y + yOffset,
                    characterBackgroundColor);
            }
        }
        fontPointer++;
    }
}

void KernelConsoleFont::DrawStringAt(const char* string, const unsigned int x, const unsigned int y)
{
    this->DrawStringAt(string, x, y, this->foregroundColor);
}

void KernelConsoleFont::DrawStringAt(const char* string, const unsigned int x, const unsigned int y, const unsigned int color)
{
    this->DrawStringAt(string, x, y, color, this->backgroundColor);
}

void KernelConsoleFont::DrawStringAt(const char* string, const unsigned int x, const unsigned int y, const unsigned int foregroundColor, const unsigned int backgroundColor) 
{
    char* character = (char*)string;
    unsigned int width = this->surface->GetWidth();
    unsigned int height = this->surface->GetHeight();
    unsigned int currentX = x;
    unsigned int currentY = y;
    while(*character != 0) {
        if(*character == '\n') {
            // Don't actually draw anything, just reset to the next line.
            currentY = currentY + this->GetCharacterPixelHeight();
            character++;
            continue;
        }

        if(*character == '\r') {
            currentX = 0;
            character++;
            continue;
        }
        
        if(currentX + this->GetCharacterPixelWidth() > width) {
            // We've reached the end of the line, reset X, and move down the screen 1 character.
            currentX = 0;
            currentY = currentY + this->GetCharacterPixelHeight();
        }

        if(currentY + this->GetCharacterPixelHeight() > height) {
            // Scrolling code would go here, but for now we'll just wrap.
            currentY = 0;
        }

        this->DrawCharacterAt(*character, currentX, currentY, foregroundColor, backgroundColor);
        currentX = currentX + this->GetCharacterPixelWidth(); // And sliiiidddde to the right.
        character++;
    }
}