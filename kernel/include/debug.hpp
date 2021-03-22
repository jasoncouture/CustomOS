
#define ASSERT(cond) \
    if ((int)(cond))        \
        asm("int $0x82");