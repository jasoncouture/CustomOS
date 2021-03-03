#include <stddef.h>
#include "../../common/include/kernelparameters.h"
#include "graphics/framebuffer.h"
#include "console/font.h"
#include "init.h"

#define RED 0x00FF0000
#define GREEN 0x0000FF00
#define BLUE 0x000000FF
#define BLACK 0x00000000
#define WHITE 0x00FFFFFF

void _start(KernelParameters *kernelParameters)
{
    kInitializeFromParameters(kernelParameters);

    kPutCharacterWithColor('H', 0, 0, GREEN, BLACK);
    kPutCharacterWithColor('e', 8, 0, GREEN, BLACK);
    kPutCharacterWithColor('l', 16, 0, GREEN, BLACK);
    kPutCharacterWithColor('l', 24, 0, GREEN, BLACK);
    kPutCharacterWithColor('o', 32, 0, GREEN, BLACK);

    while(1);
}