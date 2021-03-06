
OSNAME = CustomOS

GNUEFI = ../gnu-efi
OVMFDIR = ../OVMFbin
LDS = kernel.ld
CC = gcc
ASMC = nasm
LD = ld

CFLAGS = -ffreestanding -fshort-wchar -fno-inline -fno-exceptions -mno-red-zone
CFLAGS += -fstack-protector-strong -fno-unwind-tables -g -Wno-address-of-packed-member
CFLAGS += -Wundef -Wcast-qual -Wwrite-strings -Wimplicit-fallthrough -Wno-nonnull-compare 
CFLAGS += -Wno-expansion-to-defined -I../common/include -Iinclude
CXXFLAGS = $(CFLAGS)
CXXFLAGS += -Wno-deprecated-copy -fno-rtti
LDFLAGS = -T $(LDS) -static -Bsymbolic -nostdlib --no-dynamic-linker
DEPFLAGS = -MM -MD -MP -MF
ASMFLAGS = -w-label-redef-late -f elf64


SRCDIR := src
OBJDIR := lib
BUILDDIR = bin
BOOTEFI := $(GNUEFI)/x86_64/bootloader/main.efi

rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

ASMSRC = $(call rwildcard,$(SRCDIR),*.asm)
SRC = $(call rwildcard,$(SRCDIR),*.c)
CPPSRC = $(call rwildcard,$(SRCDIR),*.cpp)
OBJS = $(patsubst $(SRCDIR)/%.asm, $(OBJDIR)/%_asm.o, $(ASMSRC))
OBJS += $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SRC))
OBJS += $(patsubst $(SRCDIR)/%.cpp, $(OBJDIR)/%.o, $(CPPSRC))
DIRS = $(wildcard $(SRCDIR)/*)
KERNEL = $(BUILDDIR)/kernel.elf
KERNELSYM = $(BUILDDIR)/kernel.sym
FONT = $(BUILDDIR)/zap-light16.psf
OSIMAGE = $(BUILDDIR)/$(OSNAME).img
OSIMAGETEMP = $(OSIMAGE).tmp

all: setup link

setup:
	@mkdir -p $(BUILDDIR)
	@mkdir -p $(SRCDIR)
	@mkdir -p $(OBJDIR)

build: $(OBJS)

# pull in dependency info for *existing* .o files
-include $(OBJS:.o=.o.d)

$(OBJDIR)/interrupts/interrupts.o: $(SRCDIR)/interrupts/interrupts.cpp
	@ echo === COMPILING $<
	@ mkdir -p $(@D)
	@ $(CC) $(CXXFLAGS) -c $< -o $@
	@ $(CC) $(CXXFLAGS) $< -MT $@ $(DEPFLAGS) $@.d

$(OBJDIR)/%_asm.o: $(SRCDIR)/%.asm
	@ echo === ASSEMBLING $<
	@ mkdir -p $(@D)
	@ $(ASMC) $(ASMFLAGS) $< -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@ echo === COMPILING $<
	@ mkdir -p $(@D)
	@ $(CC) $(CXXFLAGS) -c $< -o $@
	@ $(CC) $(CXXFLAGS) $< -MT $@ $(DEPFLAGS) $@.d

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@ echo === COMPILING $<
	@ mkdir -p $(@D)
	@ $(CC) $(CFLAGS) -c $< -o $@
	@ $(CC) $(CFLAGS) $< -MT $@ $(DEPFLAGS) $@.d

$(KERNEL): $(OBJS)
	@ echo === LINKING $(OBJS)
	@ $(LD) $(LDFLAGS) -o $(KERNEL) $(OBJS)
#	@ strip $(KERNEL)
	
link: $(KERNEL)



clean: clean-build clean-objs

clean-build:
	@ echo === CLEANING $(BUILDDIR)
	@ rm -rfv $(BUILDDIR)/*
	@ mkdir -p $(BUILDDIR)

clean-objs:
	@ echo === CLEANING $(OBJDIR)
	@ rm -rfv $(OBJDIR)/*
	@ mkdir -p $(OBJDIR)
