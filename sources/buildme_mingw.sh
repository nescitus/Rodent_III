#!/bin/sh

gcc -O3 -s -march=core2 -fno-stack-protector -fno-exceptions -DNDEBUG -DNO_THREADS -D_FORTIFY_SOURCE=0 src/*.cpp -static -o rodent_$MSYSTEM_CARCH.exe
