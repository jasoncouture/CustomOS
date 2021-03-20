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
#include <event/keyboard/keyboard.hpp>
#include <process/process.hpp>

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
const double interval = 1.0 / 60.0;
void OnEvent(Event *event)
{
    static uint64_t eventCount = 0;
    static uint64_t eventCountAtLastScreenUpdate;
    static uint64_t maxPending = 0;
    static double lastScreenUpdate = 0.0;
    static Kernel::Events::EventLoop *eventLoop = NULL;
    if (eventLoop == NULL)
        eventLoop = Kernel::Events::EventLoop::GetInstance();
    auto pendingEvents = eventLoop->Pending();
    if (pendingEvents > maxPending)
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

int KernelEventLoop()
{
    printf("Kernel event thread started\r\n");
    auto eventLoop = Kernel::Events::EventLoop::GetInstance();
    eventLoop->Run(OnEvent);
    return 0;
}

void kMain(KernelParameters *kernelParameters)
{
    auto eventLoopProcess = new Process(1, VirtualAddressManager::GetKernelVirtualAddressManager());
    {
        eventLoopProcess->Initialize((void *)KernelEventLoop);
        eventLoopProcess->SaveFloatingPointState();
        eventLoopProcess->RestoreFloatingPointState();
        Process::Add(eventLoopProcess);
    }

    auto pageAllocator = PageAllocator::GetInstance();
    auto memory = Memory::GetInstance();
    auto bitmap = pageAllocator->GetBitmap();
    printf("Kernel booted. Starting event loop.\r\n");
    auto eventLoop = Kernel::Events::EventLoop::GetInstance();
    eventLoop->SetHandler(EventType::KeyboardBufferFull, [](Event *event) {
        printf("WARN: Keyboard buffer is full\r\n");
    });
    eventLoop->SetHandler(EventType::TimerTick, [](Event *event) {
        auto processList = *Process::GetProcessList();
        for (auto item : processList)
        {
            auto process = item.Value;
            if (process->GetProcessId() == 0)
            {
                process->Activate();
                break;
            }
        }
        printf("T");
    });

    eventLoop->SetHandler(EventType::ContextSwitch, [](Event *event) {
        printf(".");
    });
    eventLoopProcess->Activate();
    eventLoop->Publish(new Event(EventType::TimerTick, 0));
    eventLoop->Publish(new Event(EventType::TimerTick, 1));
    eventLoop->Publish(new Event(EventType::TimerTick, 2));
    eventLoop->Publish(new Event(EventType::TimerTick, 3));
    uint64_t counter = 0;
    while (true)
    {
        auto processList = *Process::GetProcessList();
        for (auto item : processList)
        {
            auto process = item.Value;
            if (process->GetProcessId() == 1)
            {
                process->Activate();
                break;
            }
        }
        asm("hlt");
    }
}