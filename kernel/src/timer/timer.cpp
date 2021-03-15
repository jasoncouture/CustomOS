#include "timer.hpp"

using namespace Kernel;

Timer *Timer::Instance = NULL;

Timer::Timer()
{
    this->elapsed = 0;
    this->timerPort = new InputOutputPort(0x40);
    this->SetFrequency(200);
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

uint64_t Timer::GetFrequency()
{
    return BaseFrequency / divisor;
}

void Timer::SetFrequency(uint64_t frequency)
{
    this->SetDivisor(BaseFrequency / frequency);
}

void Timer::Tick()
{
    this->elapsed += 1.0 / (double)GetFrequency();
}
uint64_t Timer::ElapsedTimeMilliseconds() {
    return (uint64_t)(this->elapsed * 1000.0);
}
double Timer::ElapsedTime()
{
    return this->elapsed;
}