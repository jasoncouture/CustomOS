
OSNAME = CustomOS

GNUEFI = ../gnu-efi
OVMFDIR = ../OVMFbin
LDS = kernel.ld
CC = gcc
LD = ld

CFLAGS = -ffreestanding -fshort-wchar -fno-inline
LDFLAGS = -T $(LDS) -static -Bsymbolic -nostdlib



SRCDIR := src
OBJDIR := lib
BUILDDIR = bin
BOOTEFI := $(GNUEFI)/x86_64/bootloader/main.efi

rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

SRC = $(call rwildcard,$(SRCDIR),*.c)
CPPSRC = $(call rwildcard,$(SRCDIR),*.cpp)          
COBJS = $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SRC))
CPPOBJS = $(patsubst $(SRCDIR)/%.cpp, $(OBJDIR)/%.o, $(CPPSRC))
OBJS = $(COBJS) $(CPPOBJS)
DIRS = $(wildcard $(SRCDIR)/*)
KERNEL = $(BUILDDIR)/kernel.elf
FONT = $(BUILDDIR)/zap-light16.psf
OSIMAGE = $(BUILDDIR)/$(OSNAME).img
OSIMAGETEMP = $(OSIMAGE).tmp

all: setup link

setup:
	@mkdir -p $(BUILDDIR)
	@mkdir -p $(SRCDIR)
	@mkdir -p $(OBJDIR)

build: $(OBJS)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@ echo === COMPILING $^
	@ mkdir -p $(@D)
	@ $(CC) $(CFLAGS) -c $^ -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@ echo === COMPILING $^
	@ mkdir -p $(@D)
	@ $(CC) $(CFLAGS) -c $^ -o $@

$(KERNEL): $(OBJS)
	@ echo === LINKING $(OBJS)
	@ $(LD) $(LDFLAGS) -o $(KERNEL) $(OBJS)
	
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
