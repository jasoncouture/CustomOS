MAKE = make
OBJDIR := lib
BUILDDIR = bin

OSNAME = CustomOS
FONT = $(BUILDDIR)/zap-light16.psf
OSIMAGE = $(BUILDDIR)/$(OSNAME).img
OSIMAGETEMP = $(OSIMAGE).tmp
KERNEL = kernel/bin/kernel.elf
BOOTEFI = gnu-efi/x86_64/bootloader/main.efi

all: $(OSIMAGE)

bootloader: $(BOOTEFI)
$(BOOTEFI):
	@ $(MAKE) -C gnu-efi
	@ $(MAKE) -C gnu-efi bootloader

kernel: $(KERNEL)
$(KERNEL):
	@ $(MAKE) -C kernel link

diskimage: $(OSIMAGE)

clean:
	@ $(MAKE) -C gnu-efi clean
	@ $(MAKE) -C kernel clean
	@ rm -rfv $(BUILDDIR)/*

dirs:
	@ mkdir -p $(BUILDDIR)

font: $(FONT)

$(FONT): dirs
	@ cp zap-light16.psf $(FONT)


$(OSIMAGE): $(BOOTEFI) $(KERNEL) $(FONT)
	@ echo === Building disk image $(OSIMAGE)
	@ dd if=/dev/zero of=$(OSIMAGETEMP) bs=16384 count=4096
	@ echo === Formatting $(OSIMAGE)
	@ mkfs -t fat -F 32 -n "EFI_SYSTEM" -v $(OSIMAGETEMP)
	@ echo === Creating /EFI/BOOT in $(OSIMAGE)
	@ mmd -i $(OSIMAGETEMP) ::/EFI ::/EFI/BOOT
	@ echo === Copying $(BOOTEFI) to /EFI/BOOT in $(OSIMAGE)
	@ mcopy -i $(OSIMAGETEMP) $(BOOTEFI) ::/EFI/BOOT
	@ echo === Copying startup.nsh, $(KERNEL), $(FONT) to / in $(OSIMAGE)
	@ mcopy -i $(OSIMAGETEMP) startup.nsh $(KERNEL) $(FONT) ::
	@ mv $(OSIMAGETEMP) $(OSIMAGE)

run: $(OSIMAGE)
	qemu-system-x86_64 -drive file=$(BUILDDIR)/$(OSNAME).img -m 256M -cpu qemu64 -drive if=pflash,format=raw,unit=0,file="$(OVMFDIR)/OVMF_CODE-pure-efi.fd",readonly=on -drive if=pflash,format=raw,unit=1,file="$(OVMFDIR)/OVMF_VARS-pure-efi.fd" -net none

winrun: $(OSIMAGE)
	@ ./run.bat