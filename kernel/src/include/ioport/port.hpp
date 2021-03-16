#pragma once
#include <stddef.h>
#include <stdint.h>

class InputOutputPort {
    public:
        InputOutputPort(uint16_t portNumber);
        void Write(uint8_t data, bool wait = true);
        uint8_t Read();
        void Wait();
    private:
        uint16_t portNumber;
};