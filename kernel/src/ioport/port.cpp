#include "port.hpp"


InputOutputPort::InputOutputPort(uint16_t portNumber)
{
    this->portNumber = portNumber;
}

void InputOutputPort::Wait() 
{
    asm volatile ("outb %%al, $0x80" : : "a"(0));
}

void InputOutputPort::Write(uint8_t data, bool wait) 
{
    asm volatile ("outb %0, %1" : : "a"(data), "Nd"(this->portNumber));
    if(wait)
        this->Wait();
}

uint8_t InputOutputPort::Read() 
{
    uint8_t returnVal;
    asm volatile ("inb %1, %0"
    : "=a"(returnVal)
    : "Nd"(this->portNumber));
    return returnVal;
}