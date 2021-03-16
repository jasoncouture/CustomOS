#pragma once
#include <stddef.h>
#include <stdint.h>
typedef uint64_t uword_t;

struct interrupt_frame {
    uword_t ip;
    uword_t cs;
    uword_t flags;
    uword_t sp;
    uword_t ss;
};