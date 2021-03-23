#include <timer/timer.hpp>

using namespace Kernel;

Timer *Timer::Instance = NULL;

Timer::Timer()
{
    this->elapsed = 0;
    this->timerPort = new InputOutputPort(0x40);
    this->SetFrequency(120);
}

Timer *Timer::GetInstance()
{
    if (Instance == NULL)
        Instance = new Timer();
    return Instance;
}

void Timer::SetDivisor(uint16_t divisor)
{
    if (divisor < 100)
        divisor = 100;
    this->divisor = divisor;
    this->timerPort->Write((uint8_t)(divisor & 0x00ff));
    this->timerPort->Write((uint8_t)((divisor >> 8) & 0x00ff));
}

uint64_t Timer::GetFrequency() const
{
    return BaseFrequency / divisor;
}

void Timer::SetFrequency(uint64_t frequency)
{
    this->SetDivisor(BaseFrequency / frequency);
}

void Timer::Tick()
{
    this->elapsedTicks += 1;
}

uint64_t Timer::ElapsedTimeMilliseconds() const
{
    return (uint64_t)((elapsedTicks * 1000) / this->GetFrequency());
}