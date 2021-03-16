#include <event/keyboard/keyboard.hpp>
#include <event/eventloop.hpp>
#include <collections/ringbuffer.hpp>
#include <stddef.h>
#include <stdint.h>

using namespace Kernel::Collections;

RingBuffer<char> *keyboardCharacterBuffer = NULL;

char scanCodeLookupTable[128] =
    {
        0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
        '\t', /* <-- Tab */
        'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
        0, /* <-- control key */
        'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
        '*',
        0,   /* Alt */
        ' ', /* Space bar */
        0,   /* Caps lock */
        0,   /* 59 - F1 key ... > */
        0, 0, 0, 0, 0, 0, 0, 0,
        0, /* < ... F10 */
        0, /* 69 - Num lock*/
        0, /* Scroll Lock */
        0, /* Home key */
        0, /* Up Arrow */
        0, /* Page Up */
        '-',
        0, /* Left Arrow */
        0,
        0, /* Right Arrow */
        '+',
        0, /* 79 - End key*/
        0, /* Down Arrow */
        0, /* Page Down */
        0, /* Insert Key */
        0, /* Delete Key */
        0, 0, 0,
        0, /* F11 Key */
        0, /* F12 Key */
        0, /* All other keys are undefined */
};

char shiftScanCodeLookupTable[128] =
    {
        0, 27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
        '\t', /* <-- Tab */
        'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
        0, /* <-- control key */
        'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '\"', '~', 0, '|',
        'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0,
        '*',
        0,   /* Alt */
        ' ', /* Space bar */
        0,   /* Caps lock */
        0,   /* 59 - F1 key ... > */
        0, 0, 0, 0, 0, 0, 0, 0,
        0, /* < ... F10 */
        0, /* 69 - Num lock*/
        0, /* Scroll Lock */
        0, /* Home key */
        0, /* Up Arrow */
        0, /* Page Up */
        '-',
        0, /* Left Arrow */
        0,
        0, /* Right Arrow */
        '+',
        0, /* 79 - End key*/
        0, /* Down Arrow */
        0, /* Page Down */
        0, /* Insert Key */
        0, /* Delete Key */
        0, 0, 0,
        0, /* F11 Key */
        0, /* F12 Key */
        0, /* All other keys are undefined */
};

RingBuffer<char> *GetCharacterBuffer()
{
    if (keyboardCharacterBuffer == NULL)
    {
        keyboardCharacterBuffer = new RingBuffer<char>(256);
    }

    return keyboardCharacterBuffer;
}

uint8_t keyboardState = 0;
char GetCharacterFromKeycode(uint8_t keyCode, bool shifted)
{
    char *table = shifted ? shiftScanCodeLookupTable : scanCodeLookupTable;
    if (keyCode >= 128)
        return 0;
    return table[keyCode];
}

void KeyboardEvent(KeyboardKey key, KeyboardKeyFlags flags)
{
    static uint64_t skip = 0;

    switch (key)
    {
    case KeyboardKey::LeftShift:
    case KeyboardKey::RightShift:
        if (flags == KeyboardKeyFlags::Pressed)
        {
            // We should track this better some how.
            keyboardState |= (uint8_t)0x01;
        }
        else
        {
            keyboardState &= ~((uint8_t)0x01);
        }
        break;
    case KeyboardKey::Extended:
        skip = 1;
        break;
    case KeyboardKey::PausePrefix:
        skip = 2;
        break;
    }
    static bool bufferFull = false;
    if (flags == KeyboardKeyFlags::Pressed && skip == 0)
    {
        bool shifted = keyboardState & 0x01;
        auto character = GetCharacterFromKeycode(key, shifted);
        if (character != '\0')
        {
            auto eventLoop = Kernel::Events::EventLoop::GetInstance();
            eventLoop->Publish(new Event(EventType::KeyboardCharacterInput, character));
            if (GetCharacterBuffer()->TryWrite(character))
            {
                bufferFull = false;
                eventLoop->Publish(new Event(EventType::KeyboardKeyAvailable));
            } 
            else if (!bufferFull) 
            {
                bufferFull = true;
                eventLoop->Publish(new Event(EventType::KeyboardBufferFull));
            }
        }
    }
}

void KeyboardScanCodeEvent(Event *event)
{
    auto flags = KeyboardKeyFlags::Pressed;
    auto scanCodeData = event->EventData();
    auto key = (KeyboardKey)scanCodeData;
    if (!(scanCodeData == KeyboardKey::Extended || scanCodeData == KeyboardKey::PausePrefix) && scanCodeData > 0x80)
    {
        // Above 0x80 is a key release event, with the exception of 0xE0 and 0xE1
        // So at this stage we translate pressed/released + the pressed scan code.
        flags = KeyboardKeyFlags::Released;
        key = (KeyboardKey)(scanCodeData - 0x80);
    }

    KeyboardEvent(key, flags);
}
