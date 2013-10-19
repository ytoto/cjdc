#!/bin/sh
make CC='diet gcc' CFLAGS='-Os -Wall' clean cjdc
strip -s -R .note -R .comment -R .eh_frame -R .note.gnu.build-id cjdc
# verify elf binary with: readelf -e cjdc
