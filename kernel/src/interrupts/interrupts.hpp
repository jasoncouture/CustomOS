#pragma once
#include <stddef.h>
#include <stdint.h>

struct interrupt_frame;

__attribute__((interrupt)) void Interrupt_PageFaultHandler(struct interrupt_frame* frame);

void DisableInterrupts();

void EnableInterrupts();