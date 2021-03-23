#pragma once
#include <stdint.h>
#include <stddef.h>
#include <ioport/port.hpp>

namespace Kernel
{
    class Timer
    {
    private:
        Timer();
        uint16_t divisor;
        uint64_t elapsedTicks;
        double elapsed;
        InputOutputPort *timerPort;
        static Timer *Instance;
        const uint64_t BaseFrequency = 1193182ull;

    public:
        static Timer *GetInstance();
        void SetDivisor(uint16_t divisor);
        uint64_t GetFrequency() const;
        void SetFrequency(uint64_t frequency);
        void Tick();

        uint64_t ElapsedTimeMilliseconds() const;
    };
}