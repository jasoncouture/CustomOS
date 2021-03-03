
OSNAME = CustomOS

GNUEFI = ../gnu-efi
OVMFDIR = ../OVMFbin
LDS = kernel.ld
CC = gcc
LD = ld

CFLAGS = -ffreestanding -fshort-wchar
LDFLAGS = -T $(LDS) -static -Bsymbolic -nostdlib



SRCDIR := src
OBJDIR := lib
BUILDDIR = bin
BOOTEFI := $(GNUEFI)/x86_64/bootloader/main.efi

rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

SRC = $(call rwildcard,$(SRCDIR),*.c)          
OBJS = $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SRC))
DIRS = $(wildcard $(SRCDIR)/*)
KERNEL = $(BUILDDIR)/kernel.elf
FONT = $(BUILDDIR)/zap-light16.psf
OSIMAGE = $(BUILDDIR)/$(OSNAME).img
OSIMAGETEMP = $(OSIMAGE).tmp

all: buildimage

setup:
	@mkdir -p $(BUILDDIR)
	@mkdir -p $(SRCDIR)
	@mkdir -p $(OBJDIR)

build: $(OBJS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@ echo === COMPILING $^
	@ mkdir -p $(@D)
	@ $(CC) $(CFLAGS) -c $^ -o $@

$(KERNEL): $(OBJS)
	@ echo === LINKING $(OBJS)
	@ $(LD) $(LDFLAGS) -o $(KERNEL) $(OBJS)

$(FONT):
	@ echo === Creating $(FONT)
	@ cp zap-light16.psf $(FONT)
	
link: $(KERNEL)

buildimage: $(KERNEL) $(FONT) $(OSIMAGE)


$(OSIMAGE):
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

clean: clean-build clean-objs

clean-build:
	@ echo === CLEANING $(BUILDDIR)
	@ rm -rfv $(BUILDDIR)
	@ mkdir -p $(BUILDDIR)

clean-objs:
	@ echo === CLEANING $(OBJDIR)
	@ rm -rfv $(OBJDIR)
	@ mkdir -p $(OBJDIR)
