#!/bin/bash

set -e

cd -- "$( dirname -- "${BASH_SOURCE[0]}" )/src"
patch degrib/Makefile.in < ../degrib-makefile.patch
patch zlib/contrib/minizip/Makefile.in < ../minizip-makefile.patch
./configure --without-x --with-badtclssh
make
