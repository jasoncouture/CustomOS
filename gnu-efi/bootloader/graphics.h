#pragma once
#include <efi.h>
#include <efilib.h>
#include <efiprot.h>
#include "../../common/include/framebuffer.h"
struct FrameBuffer gFrameBuffer;

struct FrameBuffer *InitializeGraphics()
{
	EFI_GUID graphicsGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
	EFI_GRAPHICS_OUTPUT_PROTOCOL *graphics;
	EFI_STATUS status;

	status = uefi_call_wrapper(BS->LocateProtocol, 3, &graphicsGuid, NULL, (void **)&graphics);
	if (EFI_ERROR(status))
	{
		Print(L"Graphics initializion failure.\r\n");
		return NULL;
	}

	EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *info;
	UINTN SizeOfInfo, numModes;

	status = uefi_call_wrapper(graphics->QueryMode, 4, graphics, graphics->Mode == NULL ? 0 : graphics->Mode->Mode, &SizeOfInfo, &info);
	// this is needed to get the current video mode
	if (status == EFI_NOT_STARTED)
		status = uefi_call_wrapper(graphics->SetMode, 2, graphics, 0);
	if (EFI_ERROR(status))
	{
		Print(L"Unable to get native mode");
		return NULL;
	}
	else
	{
		numModes = graphics->Mode->MaxMode;
	}
	EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *selectedMode = NULL;
	UINTN selectedModeNumber = 0;
	for (UINTN x = 0; x < numModes; x++)
	{
		status = uefi_call_wrapper(graphics->QueryMode, 4, graphics, x, &SizeOfInfo, &info);
		if (EFI_ERROR(status))
		{
			continue;
		}

		if (info->PixelFormat == 0 //EFI_GRAPHICS_PIXEL_FORMAT::PixelRedGreenBlueReserved8BitPerColor
			||
			info->PixelFormat == 1 // EFI_GRAPHICS_PIXEL_FORMAT::PixelBlueGreenRedReserved8BitPerColor
		)
		{
			if (selectedMode == NULL ||
				(info->HorizontalResolution < 1920 && info->VerticalResolution < 1080 &&
				 (info->HorizontalResolution * info->VerticalResolution) >
					 (selectedMode->HorizontalResolution * selectedMode->VerticalResolution)))
				{
					selectedMode = info;
					selectedModeNumber = x;
				}
		}
	}
	status = uefi_call_wrapper(graphics->SetMode, 2, graphics, selectedModeNumber);

	gFrameBuffer.BaseAddress = (void *)graphics->Mode->FrameBufferBase;
	gFrameBuffer.Size = graphics->Mode->FrameBufferSize;
	gFrameBuffer.Width = graphics->Mode->Info->HorizontalResolution;
	gFrameBuffer.Height = graphics->Mode->Info->VerticalResolution;
	gFrameBuffer.PixelsPerScanLine = graphics->Mode->Info->PixelsPerScanLine;
	gFrameBuffer.PixelFormat = graphics->Mode->Info->PixelFormat;
	return &gFrameBuffer;
}