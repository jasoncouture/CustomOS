#pragma once
#include <event/event.hpp>
#include <collections/queue.hpp>
#include <collections/map.hpp>

namespace Kernel::Events
{
    using namespace Kernel::Collections;
    class EventLoop
    {
    public:
        EventLoop();
        void Publish(Event *event);
        void Run(void (*onEvent)(Event *));
        static EventLoop *GetInstance();
        uint64_t Pending();
        void SetHandler(EventType eventType, void (*eventHandler)(Event *));
        void ClearHandler(EventType eventType);

    private:
        static EventLoop *GlobalEventLoop;
        Map<EventType, void (*)(Event *)> *eventHooks;
        Queue<Event *> *eventQueue;
    };
}