#pragma once
#include <stdint.h>
#include "../memory/heap.hpp"
namespace Kernel::Collections
{

    template <class T>
    class RingBuffer
    {
    private:
        uint64_t size;
        uint64_t readPosition;
        uint64_t writePosition;
        bool didWrite;

    public:
        RingBuffer(uint64_t capacity);
        ~RingBuffer();
        bool TryWrite(T item);
        bool TryRead(T *item);
        uint64_t Count();
        uint64_t Size();
        bool Full();

    private:
        T *buffer;
    };

    template <class T>
    RingBuffer<T>::RingBuffer(uint64_t capacity)
    {
        this->size = capacity;
        this->readPosition = 0;
        this->writePosition = 0;
        this->buffer = (T*)malloc(sizeof(T)*capacity);
        this->didWrite = false;
    }

    template <class T>
    uint64_t RingBuffer<T>::Size()
    {
        return this->size;
    }

    template <class T>
    bool RingBuffer<T>::Full()
    {
        return this->Count() == 0;
    }

    template <class T>
    bool RingBuffer<T>::TryWrite(T item)
    {
        if (this->writePosition == this->readPosition && this->didWrite)
            return false; // The buffer is full.

        this->didWrite = true;

        this->buffer[this->writePosition] = item;
        this->writePosition = (this->writePosition + 1) % this->size;
        return true;
    }

    template <class T>
    bool RingBuffer<T>::TryRead(T *item)
    {
        if (this->writePosition == this->readPosition && !this->didWrite)
            return false; // The buffer is empty.

        this->didWrite = false;
        *item = this->buffer[this->readPosition];
        this->readPosition = (this->readPosition + 1) % this->size;
        return true;
    }

    template <class T>
    uint64_t RingBuffer<T>::Count()
    {
        if (this->readPosition == this->writePosition)
        {
            // Either we've wrapped all the way around, and we're out of space, or
            // we're entirely empty. Depends what the last operation was.
            if (this->didWrite)
                return 0;
        }

        if (this->readPosition > this->writePosition)
        {
            // Writer has wrapped around, but the reader hasn't.
            return size - this->readPosition + this->writePosition;
        }
        else
        {
            return this->writePosition - this->readPosition;
        }
    }

    template <class T>
    RingBuffer<T>::~RingBuffer()
    {
        delete this->buffer;
    }
}