#pragma once
#include <memory/paging/virtualaddressmanager.hpp>
#include <interrupts/interruptframe.hpp>
#include <stdint.h>
#include <collections/linkedlist.hpp>

// bit 2 is always set, Also enable interrupts in newly created processes
// and allow CPUID
#define DEFAULT_FLAGS 0x02 | 0x0200 | 0x00200000

#ifndef MAX_PROCESSES
#define MAX_PROCESSES 0xFFFF
#endif

#ifndef DEFAULT_STACK_SIZE
#define DEFAULT_STACK_SIZE 0x10000
#endif

enum ProcessState
{
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
    static Process *Current() { return Process::current; }
    static Process *Next() { return Process::next; }

    static void Add(Process *process)
    {
        GetProcessList()->Add(process);
    }

    static LinkedList<Process *> *GetProcessList()
    {
        if (Process::processes == NULL)
        {
            Process::processes = new LinkedList<Process *>();
        }
        return Process::processes;
    }
    void Activate();
    void Activated();
    VirtualAddressManager *GetVirtualAddressManager() { return this->virtualAddressManager; }
    void Initialize(void *entrypoint, uint64_t stackSize = DEFAULT_STACK_SIZE, uint64_t flags = DEFAULT_FLAGS);
    void SetProcessState(InterruptStack *interruptStack);
    void RestoreProcessState(InterruptStack *interruptStack);
    void SaveFloatingPointState();
    void RestoreFloatingPointState();
    void Finalize();
    void Reap();
    InterruptStack GetInterruptStack();
    ProcessState State;
    void *FloatingPointState;
    void *Stack;

    uint64_t ExitCode;

private:
    InterruptStack interruptStack;
    static Process *current;
    static Process *next;
    static LinkedList<Process *> *processes;
    void *StackBase;
    VirtualAddressManager *virtualAddressManager;
    int64_t processId;
};