#pragma once

template <class T>
class Iterable
{
public:
    virtual T begin() = 0;
    virtual T end() = 0;
};