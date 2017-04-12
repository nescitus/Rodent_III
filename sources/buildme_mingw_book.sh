#!/bin/sh

gcc -O -march=core2 -fno-stack-protector -fno-exceptions -DBOOKGEN -DNDEBUG -DNO_THREADS -D_FORTIFY_SOURCE=0 src/*.cpp -static -o rodent_$MSYSTEM_CARCH.exe

echo quit | ./rodent_$MSYSTEM_CARCH.exe

gcc -Ofast -s -march=core2 -fno-stack-protector -fno-exceptions -DUSEGEN -I . -DNO_THREADS -D_FORTIFY_SOURCE=0 src/*.cpp -static -o rodent_$MSYSTEM_CARCH.exe
