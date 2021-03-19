#pragma once
#include <stdint.h>

enum DriverType {
    Keyboard,
    Mouse
};

class IDriver {
    public:
        virtual char* GetName() const = 0;
        virtual DriverType GetDriverType() const = 0;
};