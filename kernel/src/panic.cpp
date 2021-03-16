#include <panic.hpp>
#include <graphics/framebuffer.hpp>
#include <console/font.hpp>
#include <console/cstr.hpp>
#define BLUE 0x00FF0000
#define WHITE 0x00FFFFFF

void WritePanicString(KernelFrameBuffer* frameBuffer, KernelConsoleFont* consoleFont, const char* message, uint64_t charRow)
{
    uint64_t rows = frameBuffer->GetHeight() / consoleFont->GetCharacterPixelHeight();
    uint64_t columns = frameBuffer->GetWidth() / consoleFont->GetCharacterPixelWidth();

    auto stringLength = kStringLength(message);

    auto halfStringLength = stringLength / 2;
    auto targetColumn = (columns / 2) - halfStringLength;
    auto targetRow = charRow;

    consoleFont->DrawStringAt(
        message, 
        targetColumn * consoleFont->GetCharacterPixelWidth(), 
        targetRow * consoleFont->GetCharacterPixelHeight(), 
        WHITE, 
        BLUE);
}


void kPanic(const char* message)
{
    auto frameBuffer = KernelFrameBuffer::GetInstance();
    auto consoleFont = KernelConsoleFont::GetInstance();
    frameBuffer->Clear(BLUE);

    uint64_t rows = frameBuffer->GetHeight() / consoleFont->GetCharacterPixelHeight();
    uint64_t columns = frameBuffer->GetWidth() / consoleFont->GetCharacterPixelWidth();
    auto targetRow = rows / 2;

    WritePanicString(frameBuffer, consoleFont, "Whoops, something has gone horribly wrong...", targetRow - 1);
    WritePanicString(frameBuffer, consoleFont, "Because Fuck you. That's why.", targetRow);
    WritePanicString(frameBuffer, consoleFont, message, targetRow + 1);
    while(true){
        asm("hlt");
    }
}