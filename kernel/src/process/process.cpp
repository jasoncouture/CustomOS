#include <process/process.hpp>
#include <memory/heap.hpp>

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
        for (auto linkedListEntry : processList)
        {
            auto processId = linkedListEntry.Value->processId;
            if (processId == nextId)
            {
                didModifyId = true;
                nextId = nextId + 1;
            }
        }
    }

    return nextId;
}

Process::Process() : Process(new VirtualAddressManager())
{
}

Process::Process(VirtualAddressManager *virtualAddressManager)
{

    this->processId = Process::NextId();
    this->interruptStack = InterruptStack();
    this->virtualAddressManager = virtualAddressManager;
    this->FloatingPointState = calloc(108, 1);
}

void Process::Initialize(void *entrypoint, uint64_t stackSize, uint64_t flags)
{
    this->StackBase = this->Stack = calloc(stackSize, 1);
    InterruptStack initialFrame = this->interruptStack;
    initialFrame.ss = 0x10;
    initialFrame.cs = 0x08;
    initialFrame.rip = (uint64_t)(void *)ProcessStartTrampoline;
    initialFrame.rdi = (uint64_t)entrypoint;
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