#!/bin/bash

KDIR_PATH=./denv/linux-6.15.8/
make -C "$KDIR_PATH"  M=$(pwd) || exit 1;

x86_64-linux-musl-gcc -static -o ./tests/test-dev ./tests/test.c || exit 1;

# rustfmt test.rs || exit 1;
# rustc --target x86_64-unknown-linux-musl \
#     -C target-cpu=generic \
#     -C linker=rust-lld \
#     -C link-self-contained=yes \
#     ./tests/test.rs -o ./tests/test-dev || exit 1;

[ -n "$1" ] && exit 1;

cp ./tests/test-dev ./denv/initramfs/bin 

cp xstrike.ko ./denv/initramfs/
pushd ./denv/initramfs/
find . -print0 | cpio --null -ov --format=newc | gzip -9 > ../initramfs.cpio.gz
popd
qemu-system-x86_64 \
     -kernel ./denv/linux-6.15.8/arch/x86/boot/bzImage \
     -nographic \
     -append 'console=ttyS0 loglevel=15' \
     -initrd ./denv/initramfs.cpio.gz
