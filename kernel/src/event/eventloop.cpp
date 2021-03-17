#include <event/eventloop.hpp>

using namespace Kernel::Events;
using namespace Kernel::Collections;

EventLoop *EventLoop::GetInstance()
{
    if (EventLoop::GlobalEventLoop == NULL)
        EventLoop::GlobalEventLoop = new EventLoop();
    return EventLoop::GlobalEventLoop;
}
EventLoop::EventLoop()
{
    this->eventQueue = new Queue<Event *>(4);
    this->eventHooks = new Map<EventType, void (*)(Event *)>();
}
EventLoop *EventLoop::GlobalEventLoop = NULL;
void EventLoop::Run(void (*onEvent)(Event *))
{
    while (true)
    {
        asm("cli");
        Event *next;
        if (!this->eventQueue->TryDequeue(&next))
        {
            asm("sti");
            asm("hlt"); // Wait for an interrupt to wake us up.
            continue;
        }
        
        if(onEvent != NULL)
            onEvent(next);
        void (*handler)(Event *);
        if (this->eventHooks->TryGet((EventType)next->EventId(), &handler))
        {
            if (handler != NULL)
            {
                handler(next);
            }
        }
        delete next;
        asm("sti");
    }
}

void EventLoop::Publish(Event *event)
{
    this->eventQueue->Enqueue(event);
}

uint64_t EventLoop::Pending()
{
    return this->eventQueue->Count();
}

void EventLoop::SetHandler(EventType eventType, void (*handler)(Event *))
{
    this->eventHooks->Remove(eventType);
    this->eventHooks->Add(eventType, handler);
}

void EventLoop::ClearHandler(EventType eventType) 
{
    this->eventHooks->Remove(eventType);
}