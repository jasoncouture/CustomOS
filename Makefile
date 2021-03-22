MAKE = make
OBJDIR := lib
BUILDDIR = bin

OSNAME = CustomOS
FONT = $(BUILDDIR)/zap-light16.psf
OSIMAGE = $(BUILDDIR)/$(OSNAME).img
OSCDIMAGE = $(BUILDDIR)/$(OSNAME).iso
OSIMAGETEMP = $(OSIMAGE).tmp
KERNEL = kernel/bin/kernel.elf
BOOTEFI = gnu-efi/x86_64/bootloader/main.efi

all: iso

bootloader:
	@ $(MAKE) -C gnu-efi
	@ $(MAKE) -C gnu-efi bootloader

kernel: bootloader
	@ $(MAKE) -C kernel link
	
diskimage: kernel font
	@ echo === Building disk image $(OSIMAGE)
	@ cp $(BOOTEFI) $(BUILDDIR)/BOOTx64.efi
	@ dd if=/dev/zero of=$(OSIMAGETEMP) bs=16384 count=4096
	@ echo === Formatting $(OSIMAGE)
	@ mkfs -t fat -F 32 -n "EFI_SYSTEM" -v $(OSIMAGETEMP)
	@ echo === Creating /EFI/BOOT in $(OSIMAGE)
	@ mmd -i $(OSIMAGETEMP) ::/EFI ::/EFI/BOOT
	@ echo === Copying $(BOOTEFI) to /EFI/BOOT in $(OSIMAGE)
	@ mcopy -i $(OSIMAGETEMP) $(BUILDDIR)/BOOTx64.efi ::/EFI/BOOT
	@ echo === $(KERNEL), $(FONT) to / in $(OSIMAGE)
	@ mcopy -i $(OSIMAGETEMP) $(KERNEL) $(FONT) ::
	@ mv $(OSIMAGETEMP) $(OSIMAGE)

iso: diskimage
	@ echo === Building ISO Image $(OSCDIMAGE)
	@ mkdir -p $(BUILDDIR)/iso/EFI/BOOT
	@ cp $(OSIMAGE) $(BUILDDIR)/iso/boot-image.bin
	@ cp $(BOOTEFI) $(BUILDDIR)/iso/EFI/BOOT/BOOTx64.efi
	@ cp $(FONT) $(BUILDDIR)/iso/
	@ cp $(KERNEL) $(BUILDDIR)/iso/
	@ mkisofs -e boot-image.bin -no-emul-boot -r -l -udf -o $(OSCDIMAGE) $(BUILDDIR)/iso/
clean:
	@ $(MAKE) -C gnu-efi clean
	@ $(MAKE) -C kernel clean
	@ rm -rfv $(BUILDDIR)/*

dirs:
	@ mkdir -p $(BUILDDIR)

font: dirs
	@ cp zap-light16.psf $(FONT)

run: iso
	qemu-system-x86_64 -drive file=$(BUILDDIR)/$(OSNAME).img -m 256M -cpu qemu64 -drive if=pflash,format=raw,unit=0,file="$(OVMFDIR)/OVMF_CODE-pure-efi.fd",readonly=on -drive if=pflash,format=raw,unit=1,file="$(OVMFDIR)/OVMF_VARS-pure-efi.fd" -net none

winrun: iso
	@ ./run.bat

windebug: iso
	@ ./debug.bat

vsdebug: iso
	@ ./vsdebug.bat
