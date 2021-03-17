#include <stddef.h>
#include <kernelparameters.h>
#include <init.hpp>
#include <graphics/framebuffer.hpp>
#include <console/font.hpp>
#include <memory/memory.hpp>
#include <memory/pageallocator.hpp>
#include <console/cstr.hpp>
#include <memory/paging/virtualaddressmanager.hpp>
#include <memory/gdt/gdt.hpp>
#include <memory/heap.hpp>
#include <event/eventloop.hpp>
#include <event/eventcore.hpp>
#include <timer/timer.hpp>
#include <console/printf.hpp>

#define RED 0x000000FF
#define GREEN 0x0000FF00
#define BLUE 0x00FF0000
#define BLACK 0x00000000
#define WHITE 0x00FFFFFF

void WriteDebugData(const char *description, uint64_t value, uint64_t lineNumber, bool hex = false)
{
    // auto font = KernelConsoleFont::GetInstance();
    // font->DrawStringAt("                                                                              ", 0, font->GetCharacterPixelHeight() * lineNumber);
    // font->DrawStringAt(description, 0, font->GetCharacterPixelHeight() * lineNumber);
    // // This is done after processing Description, because kToHexString and kToString share a single global buffer
    // // and calling it will nuke the value located in the pointer above if it's used as an input value.
    // const char *numericString = hex ? kToHexString(value) : kToString(value);
    // font->DrawStringAt(numericString, font->GetCharacterPixelWidth() * 30, font->GetCharacterPixelHeight() * lineNumber);
}
const double interval = 1.0/60.0;
void OnEvent(Event *event)
{
    static uint64_t eventCount = 0;
    static uint64_t eventCountAtLastScreenUpdate;
    static uint64_t maxPending = 0;
    static double lastScreenUpdate = 0.0;
    static Kernel::Events::EventLoop* eventLoop = NULL;
    if(eventLoop == NULL)
        eventLoop = Kernel::Events::EventLoop::GetInstance();
    auto pendingEvents = eventLoop->Pending();
    if(pendingEvents > maxPending)
        maxPending = pendingEvents;
    eventCount++;
    
    DispatchKernelEvent(event);
    if (event->EventId() == EventType::TimerTick)
    {
        auto currentTime = Kernel::Timer::GetInstance()->ElapsedTime();
        if (currentTime - lastScreenUpdate >= interval)
        {
            auto eventsSinceLastUpdate = eventCount - eventCountAtLastScreenUpdate;
            eventCountAtLastScreenUpdate = eventCount;
            WriteDebugData("Event:", event->EventId(), 2);
            WriteDebugData("Event Data:", event->EventData(), 3);
            WriteDebugData("Timestamp (MS):", (uint64_t)(Kernel::Timer::GetInstance()->ElapsedTime() * 1000), 4);
            WriteDebugData("Timestamp (Seconds):", (uint64_t)(Kernel::Timer::GetInstance()->ElapsedTime()), 5);
            WriteDebugData("Pending events:", pendingEvents, 6);
            WriteDebugData("Max Pending events:", maxPending, 7);
            WriteDebugData("Events:", eventCount, 8);
            WriteDebugData("Events per second:", (uint64_t)((double)eventsSinceLastUpdate / (currentTime - lastScreenUpdate)), 9);
            lastScreenUpdate = currentTime;
        }
    }
}

void kMain(KernelParameters *kernelParameters)
{
    auto pageAllocator = PageAllocator::GetInstance();
    auto memory = Memory::GetInstance();
    auto bitmap = pageAllocator->GetBitmap();
    printf("Kernel booted. Starting event loop.\r\n");
    auto eventLoop = Kernel::Events::EventLoop::GetInstance();
    eventLoop->Publish(new Event(EventType::TimerTick, 0));
    eventLoop->Publish(new Event(EventType::TimerTick, 1));
    eventLoop->Publish(new Event(EventType::TimerTick, 2));
    eventLoop->Publish(new Event(EventType::TimerTick, 3));
    eventLoop->Run(OnEvent);
}