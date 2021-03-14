#pragma once
#include "idt.hpp"
#include "interruptframe.hpp"

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
            void SetInterruptHandler(void (*handler)(interrupt_frame*), uint64_t vector, uint8_t typeAndAttribute = IDT_TYPEATTRIBUTE_INTERRUPTGATE, uint16_t globalDescriptorTableSelector = 0x08);
            void Activate();
    };
}