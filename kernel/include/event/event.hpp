#pragma once
#include <stdint.h>

enum EventType
{
    Null,
    TimerTick,
    KernelEvent ,
    KeyboardScanCode,
    Keyboard,
    KeyboardCharacterInput,
    KeyboardKeyAvailable,
    KeyboardBufferFull,
    ContextSwitch
};

struct Event
{
public:
    uint64_t EventId() { return this->eventType; }
    virtual uint64_t EventData() { return this->eventData; }
    Event(EventType eventType, uint64_t eventData = 0);
    Event() : Event(EventType::Null, 0ul) {}

private:
    EventType eventType;
    uint64_t eventData;
};