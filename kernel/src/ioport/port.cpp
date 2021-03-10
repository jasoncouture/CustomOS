#include "port.hpp"

InputOutputPort WaitPort = InputOutputPort(0x80);

InputOutputPort::InputOutputPort(uint16_t portNumber)
{
    this->portNumber = portNumber;
}

void InputOutputPort::Write(uint8_t data) 
{
    
    asm volatile ("outb %0, %1" : : "a"(data), "Nd"(this->portNumber));
}

uint8_t InputOutputPort::Read() 
{
    uint8_t returnVal;
    asm volatile ("inb %1, %0"
    : "=a"(returnVal)
    : "Nd"(this->portNumber));
    return returnVal;
}