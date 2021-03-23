#include <console/console.hpp>
#include <console/printf.hpp>
#include <graphics/color.hpp>

extern "C" void _putchar(char character)
{
    static KernelConsole *console = NULL;
    if (console == NULL)
    {
        console = KernelConsole::GetInstance(false);
        if (console == NULL)
            return;
    }
    console->PutCharacter(character);
    KernelFrameBuffer::GetInstance()->Update();
}

KernelConsole *KernelConsole::GetInstance(bool createInstance)
{
    static KernelConsole *kernelConsole = NULL;
    if (kernelConsole == NULL && createInstance)
    {
        kernelConsole = new KernelConsole(KernelConsoleFont::GetInstance(), KernelFrameBuffer::GetInstance());
    }
    return kernelConsole;
}

void KernelConsole::FixPosition()
{
    if (this->position.X >= this->screenSize.X)
    {
        this->position.X = 0;
        this->position.Y = this->position.Y + 1;
    }
    if (this->position.Y >= this->screenSize.Y)
    {
        this->position.Y = 0;
    }
}

Point KernelConsole::GetConsoleSize()
{
    return this->screenSize;
}

Point KernelConsole::GetCursorPosition()
{
    return this->position;
}

void KernelConsole::SetCursorPosition(uint64_t x, uint64_t y)
{
    this->position = {x, y};
    this->FixPosition();
}

void KernelConsole::PutCharacter(char character)
{

    if (character == '\0')
    {
        return;
    }
    if (character == '\r')
    {
        this->position.X = 0;
        return;
    }
    if (character == '\n')
    {
        this->position.Y = this->position.Y + 1;
        this->FixPosition();
        return;
    }
    uint64_t x = this->position.X * this->font->GetCharacterPixelWidth();
    uint64_t y = this->position.Y * this->font->GetCharacterPixelHeight();
    this->font->DrawCharacterAt(character, x, y, this->foregroundColor, this->backgroundColor);

    this->position.X = this->position.X + 1;
    this->FixPosition();
}

void KernelConsole::Clear()
{
    this->frameBuffer->Clear(this->backgroundColor);
    this->position = {0, 0};
}

KernelConsole::KernelConsole(KernelConsoleFont *font, KernelFrameBuffer *frameBuffer)
{
    this->font = font;
    this->frameBuffer = frameBuffer;

    this->position = {0ull, 0ull};
    uint64_t characterScreenWidth = frameBuffer->GetWidth() / this->font->GetCharacterPixelWidth();
    uint64_t characterScreenHeight = frameBuffer->GetHeight() / this->font->GetCharacterPixelHeight();
    this->screenSize = {characterScreenWidth, characterScreenHeight};

    this->foregroundColor = ARGB(0, 0, 0xFF, 0);
    this->backgroundColor = ARGB(0, 0, 0, 0);
}
