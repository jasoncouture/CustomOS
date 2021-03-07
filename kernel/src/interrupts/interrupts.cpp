#include "interrupts.hpp"
#include "../console/font.hpp"
#include "../graphics/framebuffer.hpp"

__attribute__((interrupt)) void Interrupt_PageFaultHandler(struct InterruptFrame* frame) 
{
    auto consoleFont = KernelConsoleFont::GetInstance();
    auto frameBuffer = KernelFrameBuffer::GetInstance();

    frameBuffer->Clear(0x000000FF);
    consoleFont->SetForegroundColor(0xffffffff);
    consoleFont->SetBackgroundColor(0x000000ff);

    consoleFont->DrawStringAt("Panic: Page fault", 0, 0);

    asm( "hlt" );
}