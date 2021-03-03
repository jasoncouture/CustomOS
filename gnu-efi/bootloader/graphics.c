#include <efi.h>
#include <efilib.h>
#include "../../common/include/framebuffer.h"
FrameBuffer gFrameBuffer;

FrameBuffer *InitializeGraphics()
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

	Print(L"Graphics initialized\r\n");

	gFrameBuffer.BaseAddress = (void *)graphics->Mode->FrameBufferBase;
	gFrameBuffer.Size = graphics->Mode->FrameBufferSize;
	gFrameBuffer.Width = graphics->Mode->Info->HorizontalResolution;
	gFrameBuffer.Height = graphics->Mode->Info->VerticalResolution;
	gFrameBuffer.PixelsPerScanLine = graphics->Mode->Info->PixelsPerScanLine;
	gFrameBuffer.PixelFormat = graphics->Mode->Info->PixelFormat;
	return &gFrameBuffer;
}