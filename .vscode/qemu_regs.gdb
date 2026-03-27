shell qemu-system-arm -M versatilepb -m 128M -nographic -S -s -kernel build/os_qemu.elf >/tmp/qemu.log 2>&1 &
target remote localhost:1234