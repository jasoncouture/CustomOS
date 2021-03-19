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
#include <process/process.hpp>
#include <console/console.hpp>

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
    static Kernel::Events::EventLoop *eventLoop = NULL;
    if (eventLoop == NULL)
    {
        eventLoop = Kernel::Events::EventLoop::GetInstance();
    }
    uint8_t scanCode = KeyboardPort->Read();
    EndPicInterruptPrimary();
    eventLoop->Publish(new Event(EventType::KeyboardScanCode, scanCode));
}

extern "C" void Interrupt_Timer(struct InterruptStack *frame, size_t isr)
{
    static Kernel::Events::EventLoop *eventLoop = NULL;
    static Kernel::Timer *timer = NULL;

    auto currentProcess = Process::Current();
    currentProcess->SetProcessState(frame);
    currentProcess->SaveFloatingPointState();
    currentProcess->State = ProcessState::Ready;

    if (timer == NULL)
    {
        timer = Kernel::Timer::GetInstance();
    }
    timer->Tick();
    if (eventLoop == NULL)
    {
        eventLoop = Kernel::Events::EventLoop::GetInstance();
    }
    eventLoop->Publish(new Event(EventType::TimerTick, timer->ElapsedTimeMilliseconds()));
    EndPicInterruptPrimary();

    auto nextProcess = Process::Next();
    nextProcess->State = ProcessState::Running;
    if (nextProcess->GetProcessId() != currentProcess->GetProcessId())
    {
        auto console = KernelConsole::GetInstance();
        auto position = console->GetCursorPosition();
        auto size = console->GetConsoleSize();
        console->SetCursorPosition(0, size.Y - 4);
        printf("Switching context from %d to %d", currentProcess->GetProcessId(), nextProcess->GetProcessId());
        console->SetCursorPosition(position.X, position.Y);        
        eventLoop->Publish(new Event(EventType::ContextSwitch));
        nextProcess->RestoreFloatingPointState();
        nextProcess->RestoreProcessState(frame);
    }
}

void DisableInterrupts()
{
    asm volatile("cli");
}

void EnableInterrupts()
{
    asm volatile("sti");
}