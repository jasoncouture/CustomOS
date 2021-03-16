#pragma once
#include <stdint.h>
#include <collections/ringbuffer.hpp>
#include <locks/lock.hpp>

namespace Kernel::Collections
{

    template <class T>
    class Queue
    {
    private:
        RingBuffer<T> *ringBuffer;
        Lock *lock;
        void Grow();

    public:
        Queue();
        Queue(uint64_t capacity);
        void Enqueue(T item);
        bool TryDequeue(T *item);
        bool IsEmpty();
        uint64_t Count() {
            return this->ringBuffer->Count();
        }
    };

    template <class T>
    Queue<T>::Queue() : Queue<T>::Queue(512)
    {
    }

    template <class T>
    Queue<T>::Queue(uint64_t capacity)
    {
        this->lock = new Lock();
        this->ringBuffer = new RingBuffer<T>(capacity);
    }

    template <class T>
    void Queue<T>::Enqueue(T item)
    {
        this->lock->SpinWait();
        while (!this->ringBuffer->TryWrite(item))
        {
            this->Grow();
        }
        this->lock->Unlock();
    }

    template <class T>
    void Queue<T>::Grow()
    {
        auto targetSize = this->ringBuffer->Size();
        auto growthSize = targetSize;
        if (growthSize > 1024)
        {
            growthSize = 1024;
        }
        targetSize += growthSize;

        auto replacementRingBuffer = new RingBuffer<T>(targetSize);
        T current;

        while (this->ringBuffer->TryRead(&current))
        {
            replacementRingBuffer->TryWrite(current);
        }
        auto originalRingBuffer = this->ringBuffer;
        this->ringBuffer = replacementRingBuffer;

        delete originalRingBuffer;
    }

    template <class T>
    bool Queue<T>::IsEmpty()
    {
        return this->ringBuffer->Count() == 0;
    }

    template <class T>
    bool Queue<T>::TryDequeue(T *item)
    {
        this->lock->SpinWait();
        bool returnValue = this->ringBuffer->TryRead(item);
        this->lock->Unlock();
        return returnValue;
    }
}