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
#include <collections/linkedlist.hpp>

#define BLUE 0x00FF0000
#define WHITE 0x00FFFFFF

Process *ProcessDispatchStart(InterruptStack *frame)
{
    auto currentProcess = Process::Current();
    if (currentProcess == NULL)
        return NULL;
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
    if (nextProcess->State != ProcessState::Created)
        nextProcess->RestoreFloatingPointState();
    nextProcess->RestoreProcessState(frame);
    nextProcess->Activated();
    nextProcess->State = ProcessState::Running;
    Process::GetIdle()->Activate();
    return nextProcess;
}

void ProcessDispatch(InterruptStack *frame)
{
    ProcessDispatchStart(frame);
    ProcessDispatchEnd(frame);
}

static inline void flush_tlb(unsigned long addr)
{
    asm volatile("invlpg (%0)" ::"r"(addr)
                 : "memory");
}

extern "C" void Interrupt_PageFaultHandler(struct InterruptStack *frame, size_t isr)
{
    ProcessDispatchStart(frame);
    if (frame->error_code & 0x04) // Was this triggered by the kernel?
    {
        // Yes.
        if (!(frame->error_code & 0x01)) // Is the page not-present?
        {
            // The page is not present.
            if (frame->cr2 & ~0xfffull) //mask out the lower 12 bits, and check the value. If this results in zero, it's a null pointer access.
            {
                // If it's not the zero page, was it supposed to be present?
                auto virtualAddressManager = Process::Current()->GetVirtualAddressManager();
                auto virtualAddress = (void *)frame->cr2;
                auto pageTableEntry = virtualAddressManager->GetPageTableEntry(virtualAddress, false);
                if (pageTableEntry != NULL && pageTableEntry->GetFlag(PageTableEntryFlag::Present))
                {
                    // Invalidate the TLB for this address.
                    flush_tlb(frame->cr2);
                    ProcessDispatchEnd(frame);
                    return;
                }
            }
        }
    }
    kPanic("PAGE FAULT", isr, frame);
}

extern "C" void Interrupt_DoubleFaultHandler(struct InterruptStack *frame, size_t isr)
{
    kPanic("A double fault has occurred!");
}

extern "C" void Interrupt_GeneralProtectionFault(struct InterruptStack *frame, size_t isr)
{
    kPanic("General Protection Fault", isr, frame);
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
    eventLoop->Publish(Event(EventType::KeyboardScanCode, scanCode));
}

extern "C" void Interrupt_Syscall(struct InterruptStack *frame, size_t isr)
{
    ProcessDispatchStart(frame);
    HANDLE_SYSCALL(SYSCALL_EXIT, frame);

    // Before completing dispatch, we need to re-save frame, as the syscall might have changed it.
    Process::Current()->SetProcessState(frame);
    ProcessDispatchEnd(frame);
}

extern "C" void Interrupt_Yield(struct InterruptStack *frame, size_t isr)
{
    ProcessDispatch(frame);
}

extern "C" void Interrupt_AssertionFailed(struct InterruptStack *frame, size_t isr)
{
    kPanic("ASSERTION FAILED", isr, frame);
}

extern "C" void Interrupt_Timer(struct InterruptStack *frame, size_t isr)
{
    ProcessDispatchStart(frame);
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
    eventLoop->Publish(Event(EventType::TimerTick));
    EndPicInterruptPrimary();
    ProcessDispatchEnd(frame);
}