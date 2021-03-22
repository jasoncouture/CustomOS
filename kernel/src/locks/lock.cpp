#include <locks/lock.hpp>
#include <process/process.hpp>

#define LOCK_STATE_UNLOCKED 0
#define LOCK_STATE_LOCKED 1

Lock::Lock()
{
    this->LockCookie = LOCK_STATE_UNLOCKED;
}

uint64_t AtomicCompareExchange(volatile uint64_t *current, uint64_t currentValue, uint64_t newValue)
{
    uint64_t operationResult;
    // Execute a locked cmpxchgq
    // computer volatileCurrentPointer to currentValue, if they match, write newValue to volatileCurrentPointer, and finally return the value of volatileCurrentPointer
    // success is indicated by currentValue == operationResult
    asm volatile("lock; cmpxchgq %2,%1"
                 : "=a"(operationResult), "+m"(*current)
                 : "r"(newValue), "0"(currentValue)
                 : "memory");

    return operationResult;
}

bool Lock::TryAcquire()
{
    auto previousState = AtomicCompareExchange(&this->LockCookie, LOCK_STATE_UNLOCKED, LOCK_STATE_LOCKED);
    return previousState == LOCK_STATE_UNLOCKED;
}

void Lock::SpinWait(bool halt)
{
    while (!this->TryAcquire())
    {
        if (halt)
        {
            Process::Yield();
        }
    }
}

void Lock::Unlock()
{
    // This will fail if it's not locked, but if it's not locked... we didn't want it locked?
    // So a failure isn't actually a failure?
    AtomicCompareExchange(&this->LockCookie, LOCK_STATE_LOCKED, LOCK_STATE_UNLOCKED);
}