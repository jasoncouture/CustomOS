#include <stdint.h>
#include <stddef.h>
#include "interrupts.hpp"
#include "../console/font.hpp"
#include "../console/cstr.hpp"
#include "../graphics/framebuffer.hpp"
#include "apic.hpp"
#include "../panic.hpp"

#define BLUE 0x00FF0000
#define WHITE 0x00FFFFFF

__attribute__((interrupt)) void Interrupt_PageFaultHandler(struct interrupt_frame *frame)
{
    kPanic("An unrecoverable page fault has occured.");
}

__attribute__((interrupt)) void Interrupt_DoubleFaultHandler(struct interrupt_frame *frame) 
{
    kPanic("A double fault has occurred!");
}

__attribute__((interrupt)) void Interrupt_GeneralProtectionFault(struct interrupt_frame *frame)
{
    kPanic("A general protection fault has occurred!");
}

InputOutputPort KeyboardPort = InputOutputPort(0x60);

__attribute__((interrupt)) void Interrupt_KeyboardInput(struct interrupt_frame *frame)
{
    KernelConsoleFont::GetInstance()->DrawStringAt("Key Pressed", 800, 500);
    uint8_t scanCode = KeyboardPort.Read();
    KernelConsoleFont::GetInstance()->DrawStringAt(kToString(scanCode), 800, 516);
    EndPicInterruptPrimary();
}

void DisableInterrupts()
{
    asm("cli");
}

void EnableInterrupts()
{
    asm("sti");
}