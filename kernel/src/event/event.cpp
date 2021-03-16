#include <event/event.hpp>

Event::Event(EventType eventType, uint64_t eventData) 
{
    this->eventType = eventType;
    this->eventData = eventData;
}