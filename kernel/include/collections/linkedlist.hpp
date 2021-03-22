#pragma once
#include <stdint.h>
#include <collections/iterable.hpp>

template <class T>
struct LinkedListIterator;

template <class T>
class LinkedList;

template <class T>
class LinkedListItem
{
public:
    LinkedListItem(T value)
    {
        Value = value;
        next = NULL;
        previous = NULL;
    }
    T Value;

    void InsertBefore(LinkedListItem<T> *value)
    {
        value->previous = this->previous;
        value->next = this;
        if (value->previous)
        {
            value->previous->next = value;
        }
        this->previous = value;
    }

    void InsertAfter(LinkedListItem<T> *value)
    {
        value->next = this->next;
        value->previous = this;
        if (value->next)
        {
            value->next->previous = value;
        }
        this->next = value;
    }

    void InsertAfter(T value)
    {
        auto newValue = new LinkedListItem<T>(value);
        InsertAfter(newValue);
    }

    void InsertBefore(T value)
    {
        auto newValue = new LinkedListItem<T>(value);
        InsertBefore(newValue);
    }

    LinkedListItem<T> *Next()
    {
        return this->next;
    }

    LinkedListItem<T> *Previous()
    {
        return this->previous;
    }

private:
    LinkedListItem<T> *next;
    LinkedListItem<T> *previous;
    friend class LinkedList<T>;
    friend struct LinkedListIterator<T>;
};

template <class T>
struct LinkedListIterator
{
    using value_type = LinkedListItem<T>;
    using pointer = T*;
    using reference = T&;

public:
    LinkedListIterator(value_type *next)
    {
        this->next = next;
    }

    reference operator*() const { return next->Value; }
    pointer operator->() { return &(next->Value); }

    LinkedListIterator<T> &operator++()
    {
        next = next->next;
        return *this;
    }

    LinkedListIterator<T> operator++(int)
    {
        auto tmp = *this;
        ++(*this);
        return tmp;
    }

    friend bool operator==(const LinkedListIterator<T> &a, const LinkedListIterator<T> &b) { return a.next == b.next; }
    friend bool operator!=(const LinkedListIterator<T> &a, const LinkedListIterator<T> &b) { return a.next != b.next; }

private:
    value_type* next;
};

template <class T>
class LinkedList : public Iterable<LinkedListIterator<T>>
{
public:
    void Add(T item)
    {
        if (head == NULL)
        {
            tail = head = new LinkedListItem<T>(item);
            return;
        }
        tail->InsertAfter(item);
        tail = tail->Next();
    }

    void RemoveAt(uint64_t index)
    {
        auto current = head;
        for (; current != NULL && index > 0; index--)
        {
            current = current->next;
        }
        if (current == NULL)
            return;

        if (current == head)
        {
            head = head->next;
        }

        if (current == tail)
        {
            tail = tail->previous;
        }

        if (current->next != NULL)
        {
            current->next->previous = current->previous;
        }

        if (current->previous != NULL)
        {
            current->previous->next = current->next;
        }

        delete current;
    }

    bool IsEmpty()
    {
        return head == NULL;
    }

    LinkedListIterator<T> begin() { return LinkedListIterator<T>(head); }
    LinkedListIterator<T> end() { return LinkedListIterator<T>(NULL); }

private:
    LinkedListItem<T> *head;
    LinkedListItem<T> *tail;
};