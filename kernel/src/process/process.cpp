#include <process/process.hpp>
#include <memory/heap.hpp>
#include <stdint.h>

#define FPU_STATE_SIZE_BYTES 108

Process *Process::current = NULL;
Process *Process::next = NULL;
LinkedList<Process *> *Process::processes = NULL;

int64_t Process::NextId()
{
    int64_t nextId = 0;
    bool didModifyId = true;
    auto processList = *(Process::GetProcessList());
    while (didModifyId)
    {
        didModifyId = false;
        for (auto process : processList)
        {
            auto processId = process->processId;
            if (processId == nextId)
            {
                didModifyId = true;
                nextId = nextId + 1;
            }
        }
    }

    return nextId;
}

Process::Process(const char *name) : Process(new VirtualAddressManager(), name)
{
}

Process::Process(VirtualAddressManager *virtualAddressManager, const char *name)
{

    this->processId = Process::NextId();
    this->interruptStack = InterruptStack();
    this->virtualAddressManager = virtualAddressManager;
    this->FloatingPointState = malloc(FPU_STATE_SIZE_BYTES);
    auto fpuState = (uint8_t *)this->FloatingPointState;
    fpuState[0] = 234;
    fpuState[1] = 2;
    this->name = "";
    if (name != NULL)
    {
        this->name = name;
    }
}

void Process::Initialize(void *entrypoint, uint64_t stackSize, uint64_t flags)
{
    this->StackBase = this->Stack = calloc(stackSize, 1);
    InterruptStack initialFrame = this->interruptStack;
    initialFrame.ss = 0x10;
    initialFrame.cs = 0x08;
    initialFrame.rip = (uint64_t)(void *)ProcessStartTrampoline;
    initialFrame.rax = (uint64_t)entrypoint;
    initialFrame.rbp = (uint64_t)((uint8_t *)this->StackBase + stackSize);
    initialFrame.rsp = initialFrame.rbp;
    initialFrame.cr3 = (uint64_t)this->virtualAddressManager->GetPageTableAddress();
    initialFrame.rflags = flags;
    this->interruptStack = initialFrame;
}

InterruptStack Process::GetInterruptStack()
{
    return this->interruptStack;
}

void Process::SetInterruptStack(InterruptStack interruptStack)
{
    this->interruptStack = interruptStack;
}

void Process::SetProcessState(InterruptStack *current)
{
    this->interruptStack = *current;
}

void Process::RestoreProcessState(InterruptStack *current)
{
    *current = this->interruptStack;
}

void Process::SaveFloatingPointState()
{
    asm volatile("fnsave %0"
                 : "=m"(*((uint8_t *)this->FloatingPointState)));
}

void Process::RestoreFloatingPointState()
{
    asm volatile("frstor %0"
                 :
                 : "m"(*((uint8_t *)this->FloatingPointState)));
}

void Process::Activated()
{
    Process::current = Process::next;
}

void Process::Activate()
{
    Process::next = this;
}

// Stubs

void Process::Finalize()
{
}

void Process::Reap()
{
}