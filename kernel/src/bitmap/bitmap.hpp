#pragma once
#include <stdint.h>
#include <stddef.h>

class Bitmap
{
    private:
        uint8_t* buffer;
        size_t size;

    public:
        Bitmap() : Bitmap((void*)NULL, (size_t)0) { }
        Bitmap(void* buffer, size_t size);
        size_t Size();
        bool operator[](uint64_t index);
        bool Set(uint64_t index);
        bool Set(uint64_t index, bool value);
        bool Unset(uint64_t index);
};