#!/bin/bash

CFG="-DUSEGEN -DUSE_THREADS -DNEW_THREADS"
CC=g++
EXENAME=rodentiii

if [[ "$MSYSTEM_CARCH" == "i686" ]]; then
	LSA="-Wl,--large-address-aware,--gc-sections"
else
	LSA="-Wl,--gc-sections"
fi

function buildexe {

	$CC -Ofast $2 -s -march=$1 -fno-rtti -fno-stack-protector -fno-exceptions -fno-unwind-tables -fno-asynchronous-unwind-tables -fno-ident -fwhole-program -DNDEBUG -D_FORTIFY_SOURCE=0 $CFG -I . src/combined.cpp -static $LSA -o ${EXENAME}_${MSYSTEM_CARCH}_${1}${3}.exe
}

function buildprof {

	buildexe $1 -fprofile-generate

	echo Profiling...
	echo bench | ./${EXENAME}_${MSYSTEM_CARCH}_$1.exe > /dev/null

	if [[ $? -eq 0 ]]; then
		rm ${EXENAME}_${MSYSTEM_CARCH}_$1.exe
		echo Using profile...
		buildexe ${1} -fprofile-use _pgo
	else
		rm ${EXENAME}_${MSYSTEM_CARCH}_$1.exe
		echo Profiling error!
		buildexe ${1}
	fi

	rm -f *.gcda
}

# Add default required archs here (see https://gcc.gnu.org/onlinedocs/gcc/x86-Options.html)

#archsDEF=(core2 nehalem skylake bdver2 znver1)
archsDEF=(core2 bdver2)

if [[ $# -ne 0 ]]; then
	if [[ $# -eq 1 ]] && [[ "$1" == "pgo" ]]; then
		archsDEF+=("pgo")
	else
		archsDEF=("$@")
	fi
fi

archs=()
PGO=false
for arch in "${archsDEF[@]}"; do
	if [[ "$arch" == "pgo" ]]; then
		PGO=true
	else
		archs+=($arch)
	fi
done

echo Going to build for [${archs[*]}]...
echo PGO = $PGO

mv src/book_gen.h src/book_gen.h.bk
cat src/*.cpp > src/combined.cpp

# Internal book generator
echo Building instrumental internal book generator binary ...
gcc -Ofast -march=native -fno-stack-protector -fno-exceptions -fwhole-program -DBOOKGEN -DNDEBUG -DNO_THREADS -D_FORTIFY_SOURCE=0 src/combined.cpp -static -o ${EXENAME}_bookgen.exe
./${EXENAME}_bookgen.exe > /dev/null
rm ${EXENAME}_bookgen.exe

for arch in "${archs[@]}"; do
	echo '->' Building $MSYSTEM_CARCH for $arch ...

	if [[ "$PGO" == "true" ]]; then
		buildprof $arch
	else
		buildexe $arch
	fi

	echo '->' Done.
done

rm book_gen.h
rm src/combined.cpp
mv src/book_gen.h.bk src/book_gen.h
