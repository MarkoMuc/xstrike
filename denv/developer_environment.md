# Development environment and resources

This is a short guide on how to build and run a Linux/BusyBox system in QEMU.

## Resources

- [Guide for running Linux/BusyBox in QEMU](https://gist.github.com/chrisdone/02e165a0004be33734ac2334f215380e).
- [Linux Kernel 6.15](https://www.kernel.org/).
- [BusyBox 1.37](https://busybox.net/).
   - [GitHub Mirror](https://github.com/mirror/busybox).
- [Linux Kernel Documentation](https://docs.kernel.org/index.html):
   - [Building External Modules](https://docs.kernel.org/kbuild/modules.html#building-external-modules).

## Instructions

### Kernel Image

1. Download the Linux Kernel source code.
2. Use `make tinyconfig` to create a starting configuration.
3. Use `make menuconfig` and configure the following:
    - Turn on `initramfs`.
    - Turn on `printk`.
    - Turn on `tty`, `8250/16550 serial drivers` and `Console on 8250/16550 and compatible serial port`.
    - Turn on `Kernel support for ELF binaries` and `Kernel support for scripts starting with #!`.
    - Turn on `Enable loadable module support` and `Module unloading`.
    - Turn on `sysfs` and `proc` support.
    - Turn on  `Maintain a devtmpfs filesystem  to mount at /dev` and `Automount devtmpfs at /dev, after the kernel mounted on the rootfs`.
    - Turn on `Rely on the toolchain's implicit default DWARF version` under `DEBUF_INFO_DWARF_TOOLCHAIN_DEFAULT`.
    - Turn on `CONFIG_GDB_SCRIPTS`.
4. Use `make -j$(nproc)` to compile the kernel image, which can be found in `linux-6.15.8/arch/x86/bzImage`.
5. Use `make scripts_gdb`.
6. On `linux-6.15.8` the `gdb` scripts need to be patches with the patch `./gdb.patch`, more information can be found here: https://lkml.org/lkml/2025/5/31/383
7. The compiled kernel image can be found in `arch/x86/bzImage`.

### BusyBox

1. Start with `make menuconfig`
   1. If `ncurses-devel` is installed, but `make` complains that it can't find it (might be an issue on gcc14+):
      - Fix `scripts/kconfig/lxdialog/check-lxdialog.sh:50` by changing `main() {}` to `int main() {}`.
   2. Turn on `Settings->Build static binary (no shared libs)`.
2. Build with `make -j$(nproc)` and `make install`.
   1. Using `musl` and `musl-gcc` for the compilation is preferred.
   2. On Fedora just using `make CC=musl-gcc install` did not work, `make` can't find Linux kernel headers.
   3. To get around this issue I used [musl-cross-make](https://github.com/richfelker/musl-cross-make):
      1. Clone, `cd` and make a `config.mak` file with the following contents `TARGET = x86_64-linux-musl`.
      2. Build with `make` and `make install`.
      3. Add `/musl-cross-make/output/bin/` to your path.
      4. Build BusyBox with `make CC=x86_64-linux-musl-gcc install`.

### System setup

1. Create a simple shell script, name it `init` and make it executable `chmod +x init`.

```bash
#!/bin/sh

exec /bin/sh
```

2. Move the `init` file into the `initramfs` directory.
3. Copy the `_install` directory out of `busybox` and rename it to `initramfs`.
4. Run `find . -print0 | cpio --null -ov --format=newc | gzip -9 > ../initramfs.cpio.gz`, to create a compressed version.
5. Start the system in `qemu` with:

```BASH
qemu-system-x86_64 -kernel ./linux-6.15.8/arch/x86/boot/bzImage -nographic -append 'console=ttyS0 loglevel=7' -initrd initramfs.cpio.gz
```

## Testing

1. Create a Makefile for building the modules.
   - A super basic Makefile just contains the following line `obj-m  := xstrike.o`
2. Use `make -C ./denv/linux-6.15.8/ M=$(pwd)` to build the module.
3. Move the `xstrike.ko` module into `initramfs` directory and compress it.
4. Start the system in `qemu` and run `insmod xstrike.ko` to load and `rmmod xstrike.ko` to unload the module.
5. Debugging: Start `gdb` with `gdb ./denv/linux/vmlinux` followed by `target remote localhost:1234` and add `-s -S` to the `qemu` command.
6. To use debugging scripts your `gdb` arguments or setup script should look something like:

```bash
gdb -ex "add-auto-load-safe-path $KPATH/scripts/gdb/vmlinux-gdb.py" \
    -ex "source $KPATH/vmlinux-gdb.py" \
    -ex "target remote :1234" \
    -ex "lx-symbols"\
    -ex "c" \
    $KPATH/vmlinux
```
