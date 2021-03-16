#pragma once
#include <event/event.hpp>

enum KeyboardKey {
    Escape = 0x01,
    Number1 = 0x02,
    Number2 = 0x03,
    Number3 = 0x04,
    Number4 = 0x05,
    Number5 = 0x06,
    Number6 = 0x07,
    Number7 = 0x08,
    Number8 = 0x09,
    Number9 = 0x0A,
    Number0 = 0x0B,
    Dash = 0x0C,
    Equals = 0x0D,
    Backspace = 0x0E,
    Tab = 0x0F,
    Q = 0x10,
    W = 0x11,
    E = 0x12,
    R = 0x13,
    T = 0x14,
    Y = 0x15,
    U = 0x16,
    I = 0x17,
    O = 0x18,
    P = 0x19,
    LeftBracket = 0x1A,
    RightBracket = 0x1B,
    Enter = 0x1C,
    LeftControl = 0x1D,
    A = 0x1E,
    S = 0x1F,
    D = 0x20,
    F = 0x21,
    G = 0x22,
    H = 0x23,
    J = 0x24,
    K = 0x25,
    L = 0x26,
    Colon = 0x27,
    Quote = 0x28,
    Backtick = 0x29,
    LeftShift = 0x2A,
    BackSlash = 0x2B,
    Z = 0x2C,
    X = 0x2D,
    C = 0x2E,
    V = 0x2F,
    B = 0x30,
    N = 0x31,
    M = 0x32,
    Comma = 0x33,
    Period = 0x34,
    ForwardSlash = 0x35,
    RightShift = 0x36,
    KeypadStar = 0x37,
    LeftAlt = 0x38,
    Space = 0x39,
    CapsLock = 0x3A,
    F1 = 0x3B,
    F2 = 0x3C,
    F3 = 0x3D,
    F4 = 0x3E,
    F5 = 0x3F,
    F6 = 0x40,
    F7 = 0x41,
    F8 = 0x42,
    F9 = 0x43,
    F10 = 0x44,
    NumberLock = 0x45,
    ScrollLock = 0x46,
    KeyPad7 = 0x47,
    KeyPad8 = 0x48,
    KeyPad9 = 0x49,
    KeyPadMinus = 0x4A,
    KeyPad4 = 0x4B,
    KeyPad5 = 0x4C,
    KeyPad6 = 0x4D,
    KeyPadPlus = 0x4E,
    KeyPad1 = 0x4F,
    KeyPad2 = 0x50,
    KeyPad3 = 0x51,
    KeyPad0 = 0x52,
    KeyPadDecimal = 0x53,
    F11 = 0x57,
    F12 = 0x58,

    Extended = 0xE0,
    PausePrefix = 0xE1
};

enum KeyboardKeyFlags 
{
    Pressed = 0,
    Released = 1
};

struct KeyboardEventData
{
    KeyboardKeyFlags Flags;
    KeyboardKey Key;
    char Character;
};

void KeyboardScanCodeEvent(Event* event);