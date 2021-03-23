#include <panic.hpp>
#include <graphics/framebuffer.hpp>
#include <console/font.hpp>
#include <console/cstr.hpp>
#include <interrupts/interruptframe.hpp>
#include <console/printf.hpp>

#define BLUE 0x00FF0000
#define WHITE 0x00FFFFFF
void __HALT()
{
    while (true)
    {
        asm("cli");
        asm("hlt");
    }
}
void WritePanicString(KernelFrameBuffer *frameBuffer, KernelConsoleFont *consoleFont, const char *message, uint64_t charRow)
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

void kPanic(const char *message, size_t isr, InterruptStack *interruptFrame)
{
    uint64_t cr2_state;
    asm volatile(
        "mov %%cr2, %%rax\n\t"
        "mov %%rax, %0"
        : "=m"(cr2_state) // Outputs
        :                 // No inputs
        : "%rax"          // We clobber RAX.
    );

    char panicMessage[8192];
    snprintf(panicMessage, 8191, "Panic in ISR #%d", isr);
    auto frameBuffer = KernelFrameBuffer::GetInstance();
    auto consoleFont = KernelConsoleFont::GetInstance();
    frameBuffer->Clear(BLUE);
    WritePanicString(frameBuffer, consoleFont, panicMessage, 0);
    WritePanicString(frameBuffer, consoleFont, "Debug Info", 1);
    snprintf(panicMessage, 8191, "CR2: %016llx, RIP: %016llx, Error Code: %d", cr2_state, interruptFrame->rip, interruptFrame->error_code);
    WritePanicString(frameBuffer, consoleFont, panicMessage, 2);
    frameBuffer->SwapBuffers();
    __HALT();
}

void kPanic(const char *message)
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
    frameBuffer->SwapBuffers();
    __HALT();
}