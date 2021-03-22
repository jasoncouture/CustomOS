#pragma once
#include <interrupts/idt.hpp>
#include <interrupts/interruptframe.hpp>

namespace Kernel::Interrupts 
{
    class InterruptDescriptors {
        private:
            InterruptDescriptors();
            InterruptDesciptorTableLocation interruptDesciptorTableLocation;
            InterruptDescriptorTableEntry* table;
            static InterruptDescriptors* Instance;
        public:
            static InterruptDescriptors* GetInstance();
            void SetInterruptHandler(void (*handler)(InterruptStack*, size_t), uint64_t vector, uint8_t typeAndAttribute = IDT_ATTRIBUTE_PRESENT | IDT_ATTRIBUTE_PRIVILIGED | IDT_TYPE_GATE_INTERRUPT, uint16_t globalDescriptorTableSelector = 0x08);
            void Activate();
    };
}
