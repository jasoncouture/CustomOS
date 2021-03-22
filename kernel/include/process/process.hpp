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
    Process(const char* name = NULL);
    Process(VirtualAddressManager *, const char* name = NULL);
    int64_t GetProcessId() { return this->processId; }
    static void Yield()
    {
        asm volatile("int $0x81");
    }
    static Process *Current() { return Process::current; }
    static Process *Next() { return Process::next; }
    static int64_t NextId();
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
    void SetInterruptStack(InterruptStack interruptStack);
    ProcessState State;
    void *FloatingPointState;
    void *Stack;
    const char* GetName() const { return this->name == NULL ? "" : this->name; }
    uint64_t ExitCode;

private:
    InterruptStack interruptStack;
    static Process *current;
    static Process *next;
    static LinkedList<Process *> *processes;
    const char* name;
    void *StackBase;
    VirtualAddressManager *virtualAddressManager;
    int64_t processId;
};
