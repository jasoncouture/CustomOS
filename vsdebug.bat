set OSNAME=CustomOS
set BUILDDIR=bin
set OVMFDIR=OVMFbin
set PATH=C:\Program Files\qemu;%PATH%

start "QEMU" /I qemu-system-x86_64.exe -s -S -monitor stdio -machine q35 -cdrom %BUILDDIR%\%OSNAME%.iso -m 1g -cpu qemu64 -bios %OVMFDIR%\OVMF_CODE-pure-efi.fd -net none -L "C:\Program Files\qemu"

timeout /t 1 /nobreak 