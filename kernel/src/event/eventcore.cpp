#include "eventcore.hpp"
#include "keyboard/keyboard.hpp"

void DispatchKernelEvent(Event *event)
{
    switch ((EventType)event->EventId())
    {
    case EventType::KeyboardScanCode:
        KeyboardScanCodeEvent(event);
        break;
    }
}