#pragma once

struct InterruptFrame;
__attribute__((interrupt)) void Interrupt_PageFaultHandler(struct InterruptFrame* frame);