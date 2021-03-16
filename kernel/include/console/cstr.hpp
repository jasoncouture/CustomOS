#pragma once
#include <stdint.h>
#include <stddef.h>

const char* kToString(uint64_t value);
const char* kToHexString(uint64_t value);

uint64_t kStringLength(const char * str);