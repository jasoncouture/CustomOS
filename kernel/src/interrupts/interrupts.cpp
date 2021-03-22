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
#include <syscall/syscall.hpp>

#define BLUE 0x00FF0000
#define WHITE 0x00FFFFFF

Process *ProcessDispatchStart(InterruptStack *frame)
{
    auto currentProcess = Process::Current();
    currentProcess->SetProcessState(frame);
    currentProcess->SaveFloatingPointState();
    if (currentProcess->State == ProcessState::Running)
        currentProcess->State = ProcessState::Ready;
    return currentProcess;
}

Process *ProcessDispatchEnd(InterruptStack *frame)
{
    auto currentProcess = Process::Current();
    auto nextProcess = Process::Next();
    auto eventLoop = Kernel::Events::EventLoop::GetInstance();
    nextProcess->State = ProcessState::Running;
    if (nextProcess->GetProcessId() != currentProcess->GetProcessId())
    {
        eventLoop->Publish(new Event(EventType::ContextSwitch));
        nextProcess->RestoreFloatingPointState();
        nextProcess->RestoreProcessState(frame);
        nextProcess->Activated();
        nextProcess->State = ProcessState::Running;
    }
    return nextProcess;
}

void ProcessDispatch(InterruptStack *frame)
{
    ProcessDispatchStart(frame);
    ProcessDispatchEnd(frame);
}

extern "C" void Interrupt_PageFaultHandler(struct InterruptStack *frame, size_t isr)
{
    char buffer[1024];
    kPanic("PAGE FAULT", isr, frame);
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
    ProcessDispatch(frame);
    static Kernel::Events::EventLoop *eventLoop = NULL;
    if (eventLoop == NULL)
    {
        eventLoop = Kernel::Events::EventLoop::GetInstance();
    }
    uint8_t scanCode = KeyboardPort->Read();
    EndPicInterruptPrimary();
    eventLoop->Publish(new Event(EventType::KeyboardScanCode, scanCode));
}

extern "C" void Interrupt_Syscall(struct InterruptStack *frame, size_t isr)
{
    ProcessDispatchStart(frame);
    HANDLE_SYSCALL(SYSCALL_EXIT, frame);

    // Before completing dispatch, we need to re-save frame, as the syscall might have changed it.
    Process::Current()->SetProcessState(frame);
    ProcessDispatchEnd(frame);
}

extern "C" void Interrupt_AssertionFailed(struct InterruptStack *frame, size_t isr)
{
    kPanic("ASSERTION FAILED", isr, frame);
}

extern "C" void Interrupt_Timer(struct InterruptStack *frame, size_t isr)
{
    static Kernel::Events::EventLoop *eventLoop = NULL;
    static Kernel::Timer *timer = NULL;

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
    ProcessDispatch(frame);
}

void DisableInterrupts()
{
    asm volatile("cli");
}

void EnableInterrupts()
{
    asm volatile("sti");
}