#include "bitmap.hpp"

Bitmap::Bitmap(void *buffer, size_t size)
{
    this->size = size;
    this->buffer = (uint8_t *)buffer;
}

uint64_t Bitmap::Size()
{
    return this->size * 8;
}

bool Bitmap::operator[](uint64_t index)
{
    uint64_t byteIndex = index / 8;
    uint8_t bitIndex = index % 8;
    if (byteIndex >= this->size)
        return false;

    uint8_t mask = (uint8_t)0b10000000 >> bitIndex;
    if (buffer[byteIndex] & mask)
        return true;
    return false;
}

bool Bitmap::Set(uint64_t index, bool state)
{
    if (state)
    {
        return this->Set(index);
    }
    else
    {
        return this->Unset(index);
    }
}

bool Bitmap::Set(uint64_t index)
{
    uint64_t byteIndex = index / 8;
    uint8_t bitIndex = index % 8;
    if (byteIndex >= this->size)
        return false;
    uint8_t mask = (uint8_t)0b10000000 >> bitIndex;
    buffer[byteIndex] |= mask;
    return true;
}

bool Bitmap::Unset(uint64_t index)
{
    uint64_t byteIndex = index / 8;
    uint8_t bitIndex = index % 8;
    if (byteIndex >= this->size)
        return false;

    uint8_t mask = (uint8_t)0b10000000 >> bitIndex;

    buffer[byteIndex] &= ~mask;
    return true;
}

uint8_t * Bitmap::GetBuffer(){
    return this->buffer;
}