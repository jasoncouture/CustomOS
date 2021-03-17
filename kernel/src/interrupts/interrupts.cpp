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
#include <console/printf.hpp>

#define BLUE 0x00FF0000
#define WHITE 0x00FFFFFF

extern "C" void Interrupt_PageFaultHandler(struct InterruptStack *frame, size_t isr)
{
    char buffer[1024];
    sprintf(buffer, "PAGE FAULT @ IP: 0x%016X", frame->rip);
    kPanic(buffer);
}

extern "C" void Interrupt_DoubleFaultHandler(struct InterruptStack *frame, size_t isr) 
{
    kPanic("A double fault has occurred!");
}

extern "C" void Interrupt_GeneralProtectionFault(struct InterruptStack *frame, size_t isr)
{
    kPanic("A general protection fault has occurred!");
}

extern "C" void Interrupt_KeyboardInput(struct InterruptStack *frame, size_t isr)
{
    uint8_t scanCode = KeyboardPort->Read();
    EndPicInterruptPrimary();
    Kernel::Events::EventLoop::GetInstance()->Publish(new Event(EventType::KeyboardScanCode, scanCode));
}

extern "C" void Interrupt_Timer(struct InterruptStack *frame, size_t isr) 
{
    Kernel::Timer::GetInstance()->Tick();
    Kernel::Events::EventLoop::GetInstance()->Publish(new Event(EventType::TimerTick, Kernel::Timer::GetInstance()->ElapsedTimeMilliseconds()));
    EndPicInterruptPrimary();
}

void DisableInterrupts()
{
    asm volatile("cli");
}

void EnableInterrupts()
{
    asm volatile("sti");
}