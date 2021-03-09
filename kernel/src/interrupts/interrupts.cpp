#include <stdint.h>
#include <stddef.h>
#include "interrupts.hpp"
#include "../console/font.hpp"
#include "../graphics/framebuffer.hpp"

__attribute__((interrupt)) void Interrupt_PageFaultHandler(struct interrupt_frame *frame)
{
    auto consoleFont = KernelConsoleFont::GetInstance();
    //frameBuffer->Clear(0x000000ff);
    consoleFont->DrawStringAt("Panic: Page fault", 0, 0, 0, 0x000000ff);

    asm( "hlt" );
}

void DisableInterrupts()
{
    asm("cli");
}

void EnableInterrupts()
{
    asm("sti");
}