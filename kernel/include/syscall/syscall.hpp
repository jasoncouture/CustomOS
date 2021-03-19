#pragma once
#include <stdint.h>
#include <interrupts/interruptframe.hpp>
#define SYSCALL_EXIT 1
#define SYSCALL1(syscall) SYSCALL2(syscall, __UNUSED_RAX)
#define SYSCALL2(syscall, p1) SYSCALL3(syscall, p1, __UNUSED_RDI)
#define SYSCALL3(syscall, p1, p2) SYSCALL4(syscall, p1, p2, __UNUSED_RSI)
#define SYSCALL4(syscall, p1, p2, p3) SYSCALL5(syscall, p1, p2, p3, __UNUSED_RDX)
#define SYSCALL5(syscall, p1, p2, p3, p4) SYSCALL6(syscall, p1, p2, p3, p4, frame)
#define SYSCALL6(syscall, p1, p2, p3, p4, p5) void sys$##syscall(uint64_t p1, uint64_t p2, uint64_t p3, uint64_t p4, InterruptStack *p5)
#define GET_MACRO(_1, _2, _3, _4, _5, _6, NAME, ...) NAME
#define SYSCALL(...) GET_MACRO(__VA_ARGS__, SYSCALL6, SYSCALL5, SYSCALL4, SYSCALL3, SYSCALL2, SYSCALL1) \
(__VA_ARGS__)

SYSCALL(1);

#define CALL_SYSCALL(syscall, frame) sys$##syscall(frame->rax, frame->rdi, frame->rsi, frame->rdx, frame)
#define HANDLE_SYSCALL(syscall, frame) \
    if (frame->rax == syscall)         \
    {                                  \
        CALL_SYSCALL(syscall, frame);  \
    }