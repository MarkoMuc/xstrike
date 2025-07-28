#!/bin/bash

make -C ./denv/linux-6.15.8/ M=$(pwd) && \
    cp xstrike.ko ./denv/initramfs/ && \
    cd ./denv/initramfs/ && echo $(pwd) && \
    find . -print0 | cpio --null -ov --format=newc | gzip -9 > ../initramfs.cpio.gz && \
    cd - && \
    qemu-system-x86_64 \
     -kernel ./denv/linux-6.15.8/arch/x86/boot/bzImage \
     -nographic \
     -append 'console=ttyS0 loglevel=7' \
     -initrd ./denv/initramfs.cpio.gz
