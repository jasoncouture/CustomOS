#include <event/eventloop.hpp>
#include <process/process.hpp>
#include <interrupts/interrupts.hpp>

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
    this->eventQueue = new Queue<Event>(8192);
    this->eventHooks = new Map<EventType, void (*)(Event)>();
}
EventLoop *EventLoop::GlobalEventLoop = NULL;
void EventLoop::Run(void (*onEvent)(Event))
{
    while (true)
    {
        Event next;
        DisableInterrupts();
        if (!this->eventQueue->TryDequeue(&next) || next.EventId() == EventType::Null)
        {
            EnableInterrupts();
            Process::Yield();
            //asm("hlt"); // Wait for an interrupt to wake us up.
            continue;
        }
        EnableInterrupts();

        if (onEvent != NULL)
            onEvent(next);
        void (*handler)(Event);
        if (this->eventHooks->TryGet((EventType)next.EventId(), &handler))
        {
            if (handler != NULL)
            {
                handler(next);
            }
        }
    }
}

void EventLoop::Publish(Event event)
{
    this->eventQueue->Enqueue(event);
}

uint64_t EventLoop::Pending()
{
    return this->eventQueue->Count();
}

void EventLoop::SetHandler(EventType eventType, void (*handler)(Event))
{
    this->eventHooks->Remove(eventType);
    this->eventHooks->Add(eventType, handler);
}

void EventLoop::ClearHandler(EventType eventType)
{
    this->eventHooks->Remove(eventType);
}