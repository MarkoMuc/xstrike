#!/bin/bash

KDIR_PATH=./denv/linux-6.15.8/
MUSL_GCC=~/Software/musl-cross-make/output/bin/x86_64-linux-musl-gcc

make -C "$KDIR_PATH"  M=$(pwd) || exit 1;

$MUSL_GCC -static -o ./tests/test-dev ./tests/test.c || exit 1;

 rustc --target x86_64-unknown-linux-musl \
     -C target-cpu=generic \
     -C linker=rust-lld \
     -C link-self-contained=yes \
     ./tests/test.rs -o ./tests/rtest-dev || exit 1;

[ -n "$1" ] && exit 1;

cp ./tests/test-dev ./denv/ramdisk/bin 
cp ./tests/rtest-dev ./denv/ramdisk/bin 

cp xstrike.ko ./denv/ramdisk/
pushd ./denv/ramdisk/
find . -print0 | cpio --null -ov --format=newc | gzip -9 > ../ramdisk.cpio.gz
popd
qemu-system-x86_64 \
     -kernel ./denv/linux-6.15.8/arch/x86/boot/bzImage \
     -nographic \
     -append 'console=ttyS0 loglevel=15' \
     -initrd ./denv/ramdisk.cpio.gz
