#pragma once
#include <stdint.h>
#include <stddef.h>
#include "framebuffer.h"
#include "font.h"

struct BootMemoryDescriptor
{
	uint32_t Type;
	uint32_t Reserved;
	void *PhysicalAddress;
	void *VirtualAddress;
	uint64_t PageCount;
	uint64_t Attributes;
};

struct BootMemoryMap
{
	struct BootMemoryDescriptor *MemoryMap;
	uint64_t MemoryMapSize;
	uint64_t MemoryMapDescriptorSize;
};

struct KernelParameters
{
	struct FrameBuffer *FrameBuffer;
	struct Font *Font;
	struct BootMemoryMap *BootMemoryMap;
	void *FirmwareRuntimeServices;
	void *AcpiRootSystemDescriptorPointer;
};