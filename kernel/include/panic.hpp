#pragma once
#include <interrupts/interruptframe.hpp>
#include <stdint.h>

void kPanic(const char* message);
void kPanic(const char *message, size_t isr, InterruptStack *interruptFrame);