#include "init.h"
#include "../../common/include/kernelparameters.h"
#include "graphics/framebuffer.hpp"
#include "console/font.hpp"

void kInitializeFromParameters(KernelParameters *kernelParameters)
{
    KernelFrameBuffer* kernelFrameBuffer = KernelFrameBuffer::InitializeInstance(kernelParameters->FrameBuffer);
    KernelConsoleFont::InitializeInstance(kernelParameters->Font, kernelFrameBuffer);
}