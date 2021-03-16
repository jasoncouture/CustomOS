#include <interrupts/interruptdescriptortable.hpp>
#include <memory/pageallocator.hpp>
#include <memory/heap.hpp>

using namespace Kernel::Interrupts;

InterruptDescriptors* InterruptDescriptors::Instance = NULL;

InterruptDescriptors::InterruptDescriptors() 
{
    auto pageAllocator = PageAllocator::GetInstance();
    this->table = (InterruptDescriptorTableEntry*)pageAllocator->AllocatePage(true);
    this->interruptDesciptorTableLocation.Limit = 0x0FFF;
    this->interruptDesciptorTableLocation.InterruptDesciptors = this->table;
}

void InterruptDescriptors::SetInterruptHandler(void (*handler)(interrupt_frame*), uint64_t vector, uint8_t typeAndAttribute, uint16_t globalDescriptorTableSelector)
{
    InterruptDescriptorTableEntry *entry = table + vector;
    memset(entry, 0, sizeof(InterruptDescriptorTableEntry));
    entry->SetOffset((uint64_t)(*handler));
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

InterruptDescriptors* InterruptDescriptors::GetInstance() {
    if(Instance == NULL){
        Instance = new InterruptDescriptors();
    }

    return Instance;
}