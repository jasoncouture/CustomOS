#include "init.h"
#include "../../common/include/kernelparameters.h"
#include "graphics/framebuffer.h"
#include "console/font.h"

void kInitializeFromParameters(KernelParameters *kernelParameters)
{
    kInitializeFrameBuffer(kernelParameters->FrameBuffer);
    kSetConsoleFont(kernelParameters->Font);
}