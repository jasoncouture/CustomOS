set OSNAME=CustomOS
set BUILDDIR=bin
set OVMFDIR=OVMFbin

qemu-system-x86_64 -s -d int,cpu_reset -D qemu-debug.log -drive file=%BUILDDIR%\%OSNAME%.img,format=raw -m 1G -cpu qemu64 -drive if=pflash,format=raw,unit=0,file=%OVMFDIR%\OVMF_CODE-pure-efi.fd,readonly=on -drive if=pflash,format=raw,unit=1,file=%OVMFDIR%\OVMF_VARS-pure-efi.fd -net none -L "C:\Program Files\qemu"
REM qemu-system-x86_64 -s -drive file=%BUILDDIR%\%OSNAME%.img,format=raw -m 512M -cpu qemu64 -drive if=pflash,format=raw,unit=0,file=%OVMFDIR%\OVMF_CODE-pure-efi.fd,readonly=on -drive if=pflash,format=raw,unit=1,file=%OVMFDIR%\OVMF_VARS-pure-efi.fd -net none -L "C:\Program Files\qemu"

