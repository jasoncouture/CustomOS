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

    kPutCharacter('H', 0, 0);
    kPutCharacter('e', 8, 0);
    kPutCharacter('l', 16, 0);
    kPutCharacter('l', 24, 0);
    kPutCharacter('o', 32, 0);
    
    while(1);
}