#!/bin/bash

KPATH="./denv/linux-6.15.8"

gdb -ex "add-auto-load-safe-path $KPATH/scripts/gdb/vmlinux-gdb.py" \
    -ex "source $KPATH/vmlinux-gdb.py" \
    -ex "target remote :1234" \
    -ex "lx-symbols"\
    -ex "c" \
    $KPATH/vmlinux
