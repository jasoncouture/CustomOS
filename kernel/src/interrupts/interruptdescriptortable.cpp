#include <interrupts/interruptdescriptortable.hpp>
#include <memory/pageallocator.hpp>
#include <memory/heap.hpp>
#include "interrupthandler.hpp"
#define ISR_MAX 0x81

using namespace Kernel::Interrupts;

InterruptDescriptors *InterruptDescriptors::Instance = NULL;

InterruptDescriptors::InterruptDescriptors()
{
    auto pageAllocator = PageAllocator::GetInstance();
    this->table = (InterruptDescriptorTableEntry *)pageAllocator->AllocatePage(true);
    this->interruptDesciptorTableLocation.Limit = 0x0FFF;
    this->interruptDesciptorTableLocation.InterruptDesciptors = this->table;
}
void (*InterruptHandlers[ISR_MAX])(struct InterruptStack *, size_t);
#define ISR_ADDRESS(vector) (uint64_t)(void *) isr##vector
#define ISR_CASE(v)  \
    if (vector == v) \
        return ISR_ADDRESS(v);
uint64_t GetInterruptEntryPoint(uint64_t vector)
{
    ISR_CASE(0);
    ISR_CASE(1);
    ISR_CASE(2);
    ISR_CASE(3);
    ISR_CASE(4);
    ISR_CASE(5);
    ISR_CASE(6);
    ISR_CASE(7);
    ISR_CASE(8);
    ISR_CASE(9);
    ISR_CASE(10);
    ISR_CASE(11);
    ISR_CASE(12);
    ISR_CASE(13);
    ISR_CASE(14);
    //ISR_CASE(15);
    ISR_CASE(16);
    ISR_CASE(17);
    ISR_CASE(18);
    ISR_CASE(19);
    ISR_CASE(20);
    //ISR_CASE(21);
    //ISR_CASE(22);
    ISR_CASE(32);
    ISR_CASE(33);
    //ISR_CASE(34);
    ISR_CASE(35);
    ISR_CASE(36);
    ISR_CASE(37);
    //ISR_CASE(38);
    ISR_CASE(39);
    //ISR_CASE(40);
    ISR_CASE(128);
    return 0ull;
}

#undef ISR_CASE
#undef ISR_ADDRESS

void InterruptDescriptors::SetInterruptHandler(void (*handler)(InterruptStack *, size_t), uint64_t vector, uint8_t typeAndAttribute, uint16_t globalDescriptorTableSelector)
{
    auto vectorHandlerAddress = GetInterruptEntryPoint(vector);
    if (vectorHandlerAddress == 0)
        return;
    InterruptHandlers[vector] = handler;

    InterruptDescriptorTableEntry *entry = table + vector;
    memset(entry, 0, sizeof(InterruptDescriptorTableEntry));
    entry->SetOffset(vectorHandlerAddress);
    entry->TypeAndAttribute = typeAndAttribute;
    entry->Selector = globalDescriptorTableSelector; // Kernel code segment selector (See GDT);
}

void InterruptDescriptors::Activate()
{
    // Load IDT
    asm("lidt %0"
        :
        : "m"(this->interruptDesciptorTableLocation));
}

InterruptDescriptors *InterruptDescriptors::GetInstance()
{
    if (Instance == NULL)
    {
        Instance = new InterruptDescriptors();
    }

    return Instance;
}