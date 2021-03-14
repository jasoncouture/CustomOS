#pragma once
#include <stdint.h>

enum EventType {
    TimerTick = 0,
    KernelEvent = 1,
    KeyboardScanCode = 2,
    Keyboard = 3,
    KeyboardCharacterInput = 4,
    KeyboardKeyAvailable = 5,
    KeyboardBufferFull = 6
};

class Event 
{
    public:
        uint64_t EventId() { return this->eventType; }
        virtual uint64_t EventData() { return this->eventData; }
        Event(EventType eventType, uint64_t eventData = 0);

    private:
        EventType eventType;
        uint64_t eventData;
};