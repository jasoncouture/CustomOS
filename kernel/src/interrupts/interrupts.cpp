#include <stdint.h>
#include <stddef.h>
#include <interrupts/interrupts.hpp>
#include <console/font.hpp>
#include <console/cstr.hpp>
#include <graphics/framebuffer.hpp>
#include <interrupts/apic.hpp>
#include <panic.hpp>
#include <event/eventloop.hpp>
#include <timer/timer.hpp>

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

__attribute__((interrupt)) void Interrupt_KeyboardInput(struct interrupt_frame *frame)
{
    uint8_t scanCode = KeyboardPort->Read();
    EndPicInterruptPrimary();
    Kernel::Events::EventLoop::GetInstance()->Publish(new Event(EventType::KeyboardScanCode, scanCode));
}

__attribute__((interrupt)) void Interrupt_Timer(struct interrupt_frame *frame) 
{
    Kernel::Timer::GetInstance()->Tick();
    Kernel::Events::EventLoop::GetInstance()->Publish(new Event(EventType::TimerTick, Kernel::Timer::GetInstance()->ElapsedTimeMilliseconds()));
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