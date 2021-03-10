#include "cstr.hpp"

char stringConversionBuffer[128];

const char *kToString(uint64_t value)
{
    uint8_t size;
    uint64_t sizeTest = value;
    stringConversionBuffer[127] = 0;
    uint8_t stringStart = 127;

    if (value == 0)
    {
        // Check for 0, and explicitly set it, otherwise we'll hand over an empty string.
        stringStart--;
        stringConversionBuffer[stringStart] = '0';
    }

    while (value > 0)
    {
        uint8_t current = value % 10;
        value /= 10;
        stringStart--;
        stringConversionBuffer[stringStart] = '0' + current;
    }

    return stringConversionBuffer + stringStart;
}

const char *kToHexString(uint64_t value)
{
    uint8_t size;
    uint64_t sizeTest = value;
    stringConversionBuffer[127] = 0;
    uint8_t stringStart = 127;

    if (value == 0)
    {
        // Check for 0, and explicitly set it, otherwise we'll hand over an empty string.
        stringStart--;
        stringConversionBuffer[stringStart] = '0';
    }

    while (value > 0)
    {
        stringStart--;
        uint8_t current = value % 16;
        value /= 16;
        unsigned char startCharacter = '0';
        if(current > 9) {
            current -= 10;
            startCharacter = 'A';
        }
        stringConversionBuffer[stringStart] = startCharacter + current;
    }

    return stringConversionBuffer + stringStart;
}

uint64_t kStringLength(const char * str) {
    unsigned char* buffer = (unsigned char*)((void*)str);
    uint64_t size = 0;
    while(*buffer != '\0') {
        buffer++;
        size++;
    }
    return size;
}