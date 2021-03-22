#pragma once
#include <interrupts/interruptframe.hpp>
extern "C"
{
    void Interrupt_PageFaultHandler(struct InterruptStack *frame, size_t isr);
    void Interrupt_DoubleFaultHandler(struct InterruptStack *frame, size_t isr);
    void Interrupt_GeneralProtectionFault(struct InterruptStack *frame, size_t isr);
    void Interrupt_KeyboardInput(struct InterruptStack *frame, size_t isr);
    void Interrupt_Timer(struct InterruptStack *frame, size_t isr);
    void Interrupt_Syscall(struct InterruptStack *frame, size_t isr);
    void Interrupt_AssertionFailed(struct InterruptStack *frame, size_t isr);
}

void DisableInterrupts();

void EnableInterrupts();