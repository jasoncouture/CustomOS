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
#include <console/console.hpp>

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

char chars[4] = {
    '/',
    '-',
    '\\',
    '|'};

uint64_t index = 0;
void OnEvent(Event event)
{
    static KernelConsole *console = NULL;
    static Kernel::Timer *timer = NULL;
    static Memory *memory = NULL;
    static PageAllocator *pageAllocator = NULL;
    static KernelFrameBuffer *frameBuffer = NULL;
    if (console == NULL)
        console = KernelConsole::GetInstance();
    if (timer == NULL)
        timer = Kernel::Timer::GetInstance();
    if (pageAllocator == NULL)
        pageAllocator = PageAllocator::GetInstance();
    if (memory == NULL)
        memory = Memory::GetInstance();
    if (frameBuffer == NULL)
        frameBuffer = KernelFrameBuffer::GetInstance();
    auto freeMemoryInfo = pageAllocator->GetFreeMemoryInformation();
    console->SetCursorPosition(0, 0);
    printf("%d MiB of memory total\r\n", memory->Size() / (1024 * 1024));
    printf("Free memory: %d MiB\r\n", freeMemoryInfo.BytesFree / (1024 * 1024));
    printf("Reserved memory: %d MiB\r\n", freeMemoryInfo.BytesReserved / (1024 * 1024));
    printf("Used memory: %d MiB\r\n", freeMemoryInfo.BytesUsed / (1024 * 1024));
    DispatchKernelEvent(event);

    index++;
    auto selectedChar = chars[index % 4];
    console->SetCursorPosition(0, 10);
    printf("%c\r\n%d\r\n", selectedChar, timer->ElapsedTimeMilliseconds());
    printf("EventID: %d, Data: %d    \r\n", event.EventId(), event.EventData());
    printf("Events seen so far: %d\r\n", index);
    printf("Events pending: %d    \r\n", Kernel::Events::EventLoop::GetInstance()->Pending());
}

int KernelEventLoop()
{
    //printf("Kernel event thread started\r\n");
    auto eventLoop = Kernel::Events::EventLoop::GetInstance();
    eventLoop->Run(OnEvent);
    return 0;
}

void kMain(KernelParameters *kernelParameters)
{

    auto eventLoopProcess = new Process(VirtualAddressManager::GetKernelVirtualAddressManager(), "EventLoop");

    eventLoopProcess->Initialize((void *)KernelEventLoop);
    //Process::Add(eventLoopProcess);
    //printf("Event loop thread scheduled, Process ID: %d\r\n", eventLoopProcess->GetProcessId());

    auto eventLoop = Kernel::Events::EventLoop::GetInstance();
    eventLoop->SetHandler(EventType::KeyboardBufferFull, [](Event event) {
        //printf("WARN: Keyboard buffer is full\r\n");
    });

    //printf("Event handlers attached.\r\n");
    //printf("Processes:\r\n");
    uint64_t counter = 0;
    auto console = KernelConsole::GetInstance();
    while (true)
    {
        // Very, VERY simple scheduler.
        auto processList = *Process::GetProcessList();
        bool didSchedule = false;
        DisableInterrupts();
        for (auto process : processList)
        {
            if (process->State != ProcessState::Ready && process->State != ProcessState::Created)
                continue;

            didSchedule = true;
            auto currentPosition = console->GetCursorPosition();
            console->SetCursorPosition(0, 0);
            double elapsedSeconds = (double)Kernel::Timer::GetInstance()->ElapsedTimeMilliseconds() / 1000.0;
            printf("Uptime: %f seconds\r\n", elapsedSeconds);
            console->SetCursorPosition(currentPosition.X, currentPosition.Y);
            process->Activate();
            EnableInterrupts();
            Process::Yield();
            DisableInterrupts();
        }
        EnableInterrupts();
        if (!didSchedule)
        {
            asm("hlt");
        }
    }
}