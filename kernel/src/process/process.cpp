#include <process/process.hpp>
#include <interrupts/interruptframe.hpp>
#include <memory/heap.hpp>

Process::Process(int64_t processId) : Process(processId, new VirtualAddressManager())
{
}

Process::Process(int64_t processId, VirtualAddressManager *virtualAddressManager)
{
    this->processId = processId;
    this->virtualAddressManager = virtualAddressManager;
    this->FloatingPointState = calloc(108, 1);
}

void Process::Initialize(void *entrypoint, uint64_t stackSize)
{
    this->StackBase = this->Stack = calloc(stackSize, 1);
    auto initialFrame = (InterruptStack *)this->Stack;
    initialFrame->rdi = (uint64_t)entrypoint;
    // initialFrame->rsp = (uint64_t)((InterruptStack *)this->Stack + 1);
    // initialFrame->rbp = (uint64_t)this->Stack;
    initialFrame->rip = (uint64_t)(void *)ProcessStartTrampoline;
    initialFrame->ss = 0x10;
    initialFrame->cs = 0x08;
    // This is the stack we'll restore in an interrupt to launch this process.
    this->Stack = initialFrame + 1;

}
