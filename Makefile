MAKE = make

all: bootloader kernel diskimage

bootloader:
	@ $(MAKE) -C gnu-efi
	@ $(MAKE) -C gnu-efi bootloader

kernel:
	@ $(MAKE) -C kernel link

diskimage:
	@ $(MAKE) -C kernel buildimage

clean:
	@ $(MAKE) -C gnu-efi clean
	@ $(MAKE) -C kernel clean

winrun: all
	@ $(MAKE) -C kernel winrun