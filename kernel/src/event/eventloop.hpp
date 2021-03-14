#include "event.hpp"
#include "../collections/queue.hpp"

namespace Kernel::Events 
{
    using namespace Kernel::Collections;
    class EventLoop 
    {
        private:
            Queue<Event*>* eventQueue;
            static EventLoop* GlobalEventLoop;
        public:
            EventLoop();
            void Publish(Event* event);
            void Run(void (*onEvent)(Event*));
            static EventLoop* GetInstance();
            uint64_t Pending();
    };
}