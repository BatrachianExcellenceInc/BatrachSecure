#!/usr/bin/env sh
# vim: fileencoding=utf-8

set -e

make clean
rm -f BtxSecure

qmake
make
echo "\n*** RUN ***\n"
./BtxSecure http://localhost:8888 $@

