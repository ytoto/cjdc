#!/bin/sh
# http://c.h4ck.me/dev/dietlibc/
make CC='diet gcc -m32 -mtune=i686' CFLAGS='-Os -Wall' clean cjdc
strip -s -R .note -R .comment -R .eh_frame -R .note.gnu.build-id cjdc
mv cjdc cjdc-i686
