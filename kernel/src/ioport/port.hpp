#pragma once
#include <stddef.h>
#include <stdint.h>

class InputOutputPort {
    public:
        InputOutputPort(uint16_t portNumber);
        void Write(uint8_t data);
        uint8_t Read();
    private:
        uint16_t portNumber;
};


extern InputOutputPort WaitPort;