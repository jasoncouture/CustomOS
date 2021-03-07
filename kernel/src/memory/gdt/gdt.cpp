#include "gdt.hpp"

__attribute__((aligned(0x1000)))
GlobalDesciptorTable DefaultGlobalDesciptorTable = {
    { 0, 0, 0, 0x00, 0x00, 0}, // Null segment
    { 0, 0, 0, 0x9a, 0xa0, 0}, // Kernel code segment
    { 0, 0, 0, 0x92, 0xa0, 0}, // Kernel data segment
    { 0, 0, 0, 0x00, 0x00, 0}, // User null segment
    { 0, 0, 0, 0x9a, 0xa0, 0}, // User code segment
    { 0, 0, 0, 0x92, 0xa0, 0}  // User data segment
};