MAKE = make

all:
	@ $(MAKE) -C gnu-efi
	@ $(MAKE) -C gnu-efi bootloader
	@ $(MAKE) -C kernel

bootloader:
	@ $(MAKE) -C gnu-efi bootloader

kernel:
	@ $(MAKE) -C kernel build

clean:
	@ $(MAKE) -C gnu-efi clean
	@ $(MAKE) -C kernel clean