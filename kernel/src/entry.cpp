#include <stddef.h>
#include <kernelparameters.h>
#include "init.hpp"
#include "graphics/framebuffer.hpp"
#include "console/font.hpp"
#include "memory/memory.hpp"
#include "memory/pageallocator.hpp"
#include "console/cstr.hpp"
#include "memory/paging/virtualaddressmanager.hpp"
#include "memory/gdt/gdt.hpp"
#include "memory/heap.hpp"
#include "event/eventloop.hpp"
#include "event/eventcore.hpp"
#include "timer/timer.hpp"

#define RED 0x000000FF
#define GREEN 0x0000FF00
#define BLUE 0x00FF0000
#define BLACK 0x00000000
#define WHITE 0x00FFFFFF

void WriteDebugData(const char *description, uint64_t value, uint64_t lineNumber, bool hex = false)
{
    auto font = KernelConsoleFont::GetInstance();
    font->DrawStringAt("                                                                              ", 0, font->GetCharacterPixelHeight() * lineNumber);
    font->DrawStringAt(description, 0, font->GetCharacterPixelHeight() * lineNumber);
    // This is done after processing Description, because kToHexString and kToString share a single global buffer
    // and calling it will nuke the value located in the pointer above if it's used as an input value.
    const char *numericString = hex ? kToHexString(value) : kToString(value);
    font->DrawStringAt(numericString, font->GetCharacterPixelWidth() * 30, font->GetCharacterPixelHeight() * lineNumber);
}

void OnEvent(Event *event)
{
    WriteDebugData("Event:", event->EventId(), 2);
    WriteDebugData("Event Data:", event->EventData(), 3);
    auto eventLoop = Kernel::Events::EventLoop::GetInstance();
    WriteDebugData("Timestamp:", (uint64_t)(Kernel::Timer::GetInstance()->ElapsedTime() * 1000), 4);
    WriteDebugData("Pending events:", eventLoop->Pending(), 5);
    
    DispatchKernelEvent(event);
}

void kMain(KernelParameters *kernelParameters)
{
    auto pageAllocator = PageAllocator::GetInstance();
    auto memory = Memory::GetInstance();
    auto font = KernelConsoleFont::GetInstance();
    auto frameBuffer = KernelFrameBuffer::GetInstance();
    font->DrawStringAt("Booting kernel (Early init)", 0, font->GetCharacterPixelHeight() * 0);

    font->DrawStringAt("Frame buffer initialized and console font loaded", 0, font->GetCharacterPixelHeight() * 1);

    auto bitmap = pageAllocator->GetBitmap();
    WriteDebugData("Bitmap located at:", (uint64_t)bitmap->GetBuffer(), 6, true);
    WriteDebugData("Bitmap size:", (uint64_t)bitmap->Size(), 7);
    auto eventLoop = Kernel::Events::EventLoop::GetInstance();
    eventLoop->Publish(new Event(EventType::TimerTick, 0));
    eventLoop->Publish(new Event(EventType::TimerTick, 1));
    eventLoop->Publish(new Event(EventType::TimerTick, 2));
    eventLoop->Publish(new Event(EventType::TimerTick, 3));
    eventLoop->Run(OnEvent);
}