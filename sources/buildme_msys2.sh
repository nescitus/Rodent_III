#!/bin/bash

CFG="-DUSEGEN -DUSE_THREADS -DNEW_THREADS"
CC=g++

if [[ "$MSYSTEM_CARCH" == "i686" ]]; then
	LSA="-Wl,--large-address-aware,--gc-sections"
else
	LSA="-Wl,--gc-sections"
fi

function buildexe {
	echo Building $MSYSTEM_CARCH for $1 ...
	$CC -Ofast -s -march=$1 -fno-rtti -fno-stack-protector -fno-exceptions -fno-unwind-tables -fno-asynchronous-unwind-tables -fno-ident -fwhole-program -DNDEBUG -D_FORTIFY_SOURCE=0 $CFG -I . src/combined.cpp -static $LSA -o rodent_"$MSYSTEM_CARCH"_$1.exe
}

mv src/book_gen.h src/book_gen.h.bk
cat src/*.cpp > src/combined.cpp

# Internal book generator
echo Building instrumental binary ...
gcc -O2 -march=core2 -fno-stack-protector -fno-exceptions -fwhole-program -DBOOKGEN -DNDEBUG -DNO_THREADS -D_FORTIFY_SOURCE=0 src/combined.cpp -static -o rodent_bookgen.exe
./rodent_bookgen.exe > /dev/null
rm rodent_bookgen.exe


# Add required archs here (see https://gcc.gnu.org/onlinedocs/gcc/x86-Options.html)

#archs=(core2 nehalem skylake bdver2 znver1)
archs=(core2 bdver2)

for arch in "${archs[@]}"; do
	buildexe $arch
done


rm book_gen.h
rm src/combined.cpp
mv src/book_gen.h.bk src/book_gen.h
