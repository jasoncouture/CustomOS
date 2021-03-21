set OSNAME=CustomOS
set BUILDDIR=bin
set OVMFDIR=OVMFbin
set PATH=C:\Program Files\qemu;%PATH%

start "QEMU" /I qemu-system-x86_64.exe -s -S -monitor stdio -cdrom %BUILDDIR%\%OSNAME%.iso -m 512m -cpu qemu64 -drive if=pflash,format=raw,unit=0,file=%OVMFDIR%\OVMF_CODE-pure-efi.fd,readonly=on -drive if=pflash,format=raw,unit=1,file=%OVMFDIR%\OVMF_VARS-pure-efi.fd -net none -L "C:\Program Files\qemu"

timeout /t 1 /nobreak 