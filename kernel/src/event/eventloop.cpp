#include "eventloop.hpp"

using namespace Kernel::Events;
using namespace Kernel::Collections;

EventLoop *EventLoop::GetInstance()
{
    if (GlobalEventLoop == NULL)
        GlobalEventLoop = new EventLoop();
    return GlobalEventLoop;
}
EventLoop::EventLoop()
{
    this->eventQueue = new Queue<Event *>(4);
}
EventLoop *EventLoop::GlobalEventLoop = NULL;
void EventLoop::Run(void (*onEvent)(Event *))
{
    while (true)
    {
        Event *next;
        if (!this->eventQueue->TryDequeue(&next))
        {
            asm("hlt"); // Wait for an interrupt to wake us up.
            continue;
        }
        asm("cli");
        onEvent(next);
        asm("sti");
        delete next;
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