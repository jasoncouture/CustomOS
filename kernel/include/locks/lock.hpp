#pragma once
#include <stdint.h>
#include <stddef.h>

class Lock {
    public:
    Lock();
    bool TryAcquire();
    void SpinWait(bool halt = false);
    void Unlock();

    private:
    volatile uint64_t LockCookie;
};