@echo off

set mp3_dir=Z:\osdev\student-distrib
set qemu_dir=C:\qemu-1.5.0-win32-sdl

%qemu_dir%\qemu-system-i386w.exe -hda %mp3_dir%\mp3.img -m 256 -gdb tcp:127.0.0.1:1234 -S -name mp3
