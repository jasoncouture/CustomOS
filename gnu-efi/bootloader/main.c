#include <elf.h>
#include "graphics.h"
#include "../../common/include/framebuffer.h"
#include "../../common/include/kernelparameters.h"
typedef void __attribute__((sysv_abi)) (*KernelStart)(struct KernelParameters *);
struct KernelParameters gKernelParameters;

EFI_FILE *LoadFile(EFI_FILE *directory, CHAR16 *path, EFI_HANDLE imageHandle, EFI_SYSTEM_TABLE *systemTable)
{
	EFI_FILE *loadedFile;

	EFI_LOADED_IMAGE_PROTOCOL *loadedImage;
	systemTable->BootServices->HandleProtocol(imageHandle, &gEfiLoadedImageProtocolGuid, (void **)&loadedImage);

	EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *fileSystem;
	systemTable->BootServices->HandleProtocol(loadedImage->DeviceHandle, &gEfiSimpleFileSystemProtocolGuid, (void **)&fileSystem);

	if (directory == NULL)
	{
		fileSystem->OpenVolume(fileSystem, &directory);
	}

	EFI_STATUS s = directory->Open(directory, &loadedFile, path, EFI_FILE_MODE_READ, EFI_FILE_READ_ONLY);

	if (s != EFI_SUCCESS)
	{
		return NULL;
	}

	return loadedFile;
}

int memcmp(const void *leftPointer, const void *rightPointer, size_t size)
{
	const unsigned char *left = leftPointer, *right = rightPointer;
	for (size_t index = 0; index < size; index++)
	{
		if (left[index] < right[index])
			return -1;
		if (left[index] > right[index])
			return 1;
	}
	return 0;
}

void DumpMemoryMap(EFI_MEMORY_DESCRIPTOR *memoryMap, size_t size, size_t descriptorSize, EFI_SYSTEM_TABLE *systemTable){
	uint64_t entries = size / descriptorSize;
    for (uint64_t i = 0; i < entries; i++)
    {
        EFI_MEMORY_DESCRIPTOR *descriptor = (EFI_MEMORY_DESCRIPTOR *)((uint64_t)memoryMap + (i * descriptorSize));
		if(descriptor->Type != EfiConventionalMemory) continue; // Let's see where we can write.
		Print(L"0x%02x: 0x%016x-0x%016x\r\n", descriptor->Type, descriptor->PhysicalStart, descriptor->PhysicalStart + (0x1000 * descriptor->NumberOfPages));
		systemTable->BootServices->Stall(5000);
    }
}

EFI_STATUS BootFailed()
{
	Print(L"Boot failed.\r\n\r\n");
	return EFI_LOAD_ERROR;
}

EFI_STATUS LoadKernelEntry(uint64_t *kernelStart, EFI_HANDLE imageHandle, EFI_SYSTEM_TABLE *systemTable)
{
	EFI_FILE *kernel = LoadFile(NULL, L"kernel.elf", imageHandle, systemTable);
	if (kernel == NULL)
	{
		Print(L"Failed to locate kernel\r\n");
		return EFI_NOT_FOUND;
	}
	else
	{
		Print(L"Kernel found, loading... \r\n");
	}

	Elf64_Ehdr header;
	{
		UINTN fileInfoSize;
		EFI_FILE_INFO fileInfo;
		// Ask boot services how big the data structure is.
		kernel->GetInfo(kernel, &gEfiFileInfoGuid, &fileInfoSize, NULL);
		// Ask boot services for a pool of ram to hold the data structure for info about this file
		systemTable->BootServices->AllocatePool(EfiLoaderData, fileInfoSize, (void **)&fileInfoSize);
		// And finally actually query for info about the file (File size is what we're mainly after.)
		kernel->GetInfo(kernel, &gEfiFileInfoGuid, &fileInfoSize, &fileInfo);

		Print(L"File Size: %d bytes\r\n", fileInfo.Size);

		UINTN size = sizeof(header);
		kernel->Read(kernel, &size, &header);
	}

	

	int bootFailed = 0;

	if (memcmp(&header.e_ident[EI_MAG0], ELFMAG, SELFMAG) != 0)
	{
		Print(L"Kernel ELF Magic is invalid.\r\n");
		bootFailed = 1;
	}

	Print(L"ELF Class: 0x%02x\r\n", header.e_ident[EI_CLASS]);
	if (header.e_ident[EI_CLASS] != ELFCLASS64)
	{
		Print(L"Kernel is not a 64 bit ELF binary.\r\n");
		bootFailed = 1;
	}

	Print(L"ELF Data: 0x%02x\r\n", header.e_ident[EI_DATA]);
	if (header.e_ident[EI_DATA] != ELFDATA2LSB)
	{
		Print(L"Kernel data segment endianess is incorrect.\r\n");
		bootFailed = 1;
	}

	Print(L"ELF Type: 0x%02x\r\n", header.e_type);
	if (header.e_type != ET_EXEC)
	{
		Print(L"Kernel ELF Binary type is not an executable.\r\n");
		bootFailed = 1;
	}

	Print(L"ELF Machine: 0x%02x\r\n", header.e_machine);
	if (header.e_machine != EM_X86_64)
	{
		Print(L"Kernel machine type is not valid for this system\r\n");
		bootFailed = 1;
	}

	Print(L"ELF Version: 0x%02x\r\n", header.e_version);
	if (header.e_version != EV_CURRENT)
	{
		Print(L"Kernel ELF version mismatch\r\n");
		bootFailed = 1;
	}

	if (bootFailed)
		return BootFailed();

	Print(L"Kernel ELF Header validated successfully\r\n");
	Print(L"Reading program headers\r\n");

	Elf64_Phdr *programHeaders;
	{
		kernel->SetPosition(kernel, header.e_phoff);
		UINTN size = header.e_phnum * header.e_phentsize;
		systemTable->BootServices->AllocatePool(EfiLoaderData, size, (void **)&programHeaders);
		kernel->Read(kernel, &size, programHeaders);
	}

	for (
		Elf64_Phdr *programHeader = programHeaders;
		(char *)programHeader < (char *)programHeaders + header.e_phnum * header.e_phentsize;
		programHeader++)
	{
		// Read loadable segments into RAM
		switch (programHeader->p_type)
		{
		case PT_LOAD:
		{
			int segmentPagesRequired = (programHeader->p_memsz + 0x1000 + 1) / 0x1000; // Route memory size up to the nearest page boundary.
			Elf64_Addr segment = programHeader->p_paddr;
			Print(L"Loading segment: %016x\r\n", programHeader->p_paddr);
			
			systemTable->BootServices->AllocatePages(AllocateAddress, EfiLoaderData, segmentPagesRequired, &segment); // Allocate the required pages

			kernel->SetPosition(kernel, programHeader->p_offset); // Seek to the appropriate location in the kernel

			UINTN size = programHeader->p_filesz;
			kernel->Read(kernel, &size, (void *)segment); // Read the bytes from the elf binary into the memory pages we just allocated.

			break;
		}
		}
	}

	Print(L"Kernel loaded, executing entrypoint at address: %012x\r\n", header.e_entry);

	*kernelStart = header.e_entry;
	return EFI_SUCCESS;
}

struct Font *LoadFont(EFI_FILE *directory, CHAR16 *path, EFI_HANDLE imageHandle, EFI_SYSTEM_TABLE *systemTable)
{
	EFI_FILE *font = LoadFile(directory, path, imageHandle, systemTable);
	if (font == NULL)
	{
		return NULL;
	}

	struct FontHeader* fontHeader;
	UINTN size = sizeof(struct FontHeader);
	systemTable->BootServices->AllocatePool(EfiLoaderData, size, (void**)&fontHeader);
	font->Read(font, &size, fontHeader);
	unsigned char expectedMagic[2] = { PSF1_MAGIC0, PSF1_MAGIC1 };
	if(memcmp(fontHeader->Magic, expectedMagic, 2)) {
		return NULL;
	}
	// Mode 1 has 512 glyphs, otherwise 256
	UINTN glyphBufferSize = fontHeader->CharacterSize * (fontHeader->Mode == 1 ? 512 : 256);

	void* glyphBuffer;
	systemTable->BootServices->AllocatePool(EfiLoaderData, glyphBufferSize, (void**)&glyphBuffer);
	Print(L"Allocated %d bytes for the glpyh buffer pool at %016x\r\n", glyphBufferSize, glyphBuffer);
	size = sizeof(struct FontHeader);
	font->SetPosition(font, size);
	font->Read(font, &glyphBufferSize, glyphBuffer);
	
	struct Font* returnValue;

	systemTable->BootServices->AllocatePool(EfiLoaderData, sizeof(struct Font), (void**)&returnValue);
	returnValue->Header = fontHeader;
	returnValue->GlyphBuffer = glyphBuffer;
	return returnValue;
}

EFI_STATUS efi_main(EFI_HANDLE imageHandle, EFI_SYSTEM_TABLE *systemTable)
{
	InitializeLib(imageHandle, systemTable);
	Print(L"UEFI Library initializtion successful.\r\n");

	uint64_t kernelStartAddress;
	EFI_STATUS kernelLoadStatus = LoadKernelEntry(&kernelStartAddress, imageHandle, systemTable);
	if (kernelLoadStatus != EFI_SUCCESS)
	{
		return kernelLoadStatus;
	}

	struct FrameBuffer *frameBuffer = InitializeGraphics();
	Print(L"Base: 0x%016X\r\nSize %d\r\nWidth: %d\r\nHeight: %d\r\nPixels per scan line: %d\r\nPixel format: %d\r\n\r\n", frameBuffer->BaseAddress, frameBuffer->Size, frameBuffer->Width, frameBuffer->Height, frameBuffer->PixelsPerScanLine, frameBuffer->PixelFormat);
	struct Font* consoleFont = LoadFont(NULL, L"zap-light16.psf", imageHandle, systemTable);
	if(consoleFont == NULL || consoleFont->GlyphBuffer == NULL) {
		Print(L"System console font was not found, or invalid!\r\n");
		return EFI_NOT_FOUND;
	}
	Print(L"Console font loaded, character size: %d\r\n", consoleFont->Header->CharacterSize);
	Print(L"Glyph buffer location: 0x%016x\r\n", consoleFont->GlyphBuffer);
	// I see a few problems with this code now. Specifically the kernel loading code.
	// We're ignoring the memory map and blindly loading at offset 0
	// This is not EfiConventionalMemory (Type 7)
	// This is likely the source of the random kernel crashes.
	// Relocatable kernel?
	EFI_MEMORY_DESCRIPTOR* map = NULL;
	UINTN mapSize, mapKey, descriptorSize;
	UINT32 descriptorVersion;
	struct BootMemoryMap *bootMemoryMap = NULL;
	systemTable->BootServices->AllocatePool(EfiLoaderData, sizeof(struct BootMemoryMap), (void**)&bootMemoryMap);
	systemTable->BootServices->GetMemoryMap(&mapSize, map, &mapKey, &descriptorSize, &descriptorVersion); // This first call populates everything but map, since map is NULL
	systemTable->BootServices->AllocatePool(EfiLoaderData, mapSize, (void**)&map); // Allocate space for map based on the previous call
	systemTable->BootServices->GetMemoryMap(&mapSize, map, &mapKey, &descriptorSize, &descriptorVersion); // And pass everything we did before, this time actually populating the map.
	//DumpMemoryMap(map, mapSize, descriptorSize, systemTable);
	//while(1);
	systemTable->BootServices->ExitBootServices(imageHandle, mapKey);

	bootMemoryMap->MemoryMap = (void*)map; // Convert this to the kernels representation of the memory map. The data structure is the same.
	bootMemoryMap->MemoryMapSize = mapSize;
	bootMemoryMap->MemoryMapDescriptorSize = descriptorSize;
	

	// Setup the frame buffer. If it fails, oh well, no graphics for you.
	struct KernelParameters *kernelParameters = &gKernelParameters;
	kernelParameters->FrameBuffer = frameBuffer;
	kernelParameters->Font = consoleFont;
	kernelParameters->BootMemoryMap = bootMemoryMap;

	// Time to terminate boot services.
	

	// Transfer execution to the kernel.
	KernelStart kernelStart = (KernelStart)kernelStartAddress;
	kernelStart(kernelParameters);

	//Print(L"Kernel returned: %d", returnValue);

	// We should exit boot services here, before entering the kernel, as we've done our job.
	// but we're not quite ready yet. we don't have a memory map setup yet.
	// systemTable->BootServices->ExitBootServices(imageHandle, 0);
	// Early in development, we'll allow kernel returns, in the future we'll return void from the kernel, and if we get here
	// we'll ask EFI to reboot the machine, and failing that, we'll drop to assembly and execute HLT because something is
	// horribly, HORRIBLY wrong.

	//systemTable->RuntimeServices->ResetSystem(EfiResetWarm, EFI_SUCCESS, 0, NULL);
	/* If this code gets hit, your EFI implementation is horribly broken. :) */
	return EFI_SUCCESS;
}
