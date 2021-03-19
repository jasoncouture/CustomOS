#pragma once
#include <memory/paging/virtualaddressmanager.hpp>
#include <stdint.h>

#ifndef DEFAULT_STACK_SIZE
#define DEFAULT_STACK_SIZE 0x10000
#endif

enum ProcessState
{
    Created,
    Ready,
    Suspended,
    Running,
    Exited,
    Zombie
};

extern "C"
{
    extern void ProcessStartTrampoline(void *entryPoint, void *stackLocation);
}

class Process
{
public:
    Process(int64_t);
    Process(int64_t, VirtualAddressManager *);
    int64_t GetProcessId() { return this->processId; }
    VirtualAddressManager *GetVirtualAddressManager() { return this->virtualAddressManager; }
    void Initialize(void *entrypoint, uint64_t stackSize = DEFAULT_STACK_SIZE);
    ProcessState State;
    void *FloatingPointState;
    void *Stack;

private:
    void *StackBase;
    VirtualAddressManager *virtualAddressManager;
    int64_t processId;
};