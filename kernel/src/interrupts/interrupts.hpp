#pragma once
#include <stddef.h>
#include <stdint.h>

struct interrupt_frame;

__attribute__((interrupt)) void Interrupt_PageFaultHandler(struct interrupt_frame* frame);
__attribute__((interrupt)) void Interrupt_DoubleFaultHandler(struct interrupt_frame *frame);
__attribute__((interrupt)) void Interrupt_GeneralProtectionFault(struct interrupt_frame *frame);
__attribute__((interrupt)) void Interrupt_KeyboardInput(struct interrupt_frame *frame);
__attribute__((interrupt)) void Interrupt_Timer(struct interrupt_frame *frame);


void DisableInterrupts();

void EnableInterrupts();