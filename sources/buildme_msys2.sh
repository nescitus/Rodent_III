#!/bin/bash

CFG="-DUSEGEN -DUSE_THREADS"
CC=g++
EXENAME=rodentiii

if [[ "$MSYSTEM_CARCH" == "i686" ]]; then
	LSA="-Wl,--large-address-aware,--gc-sections"
else
	LSA="-Wl,--gc-sections"
fi

function buildexe {

	$CC -Ofast $2 -s -march=$1 -std=c++14 -fno-rtti -fno-stack-protector -fno-exceptions -fno-unwind-tables -fno-asynchronous-unwind-tables -fno-ident -fwhole-program $DBG -D_FORTIFY_SOURCE=0 $CFG -I . src/combined.cpp -static $LSA $NTS -o ${EXENAME}_${MSYSTEM_CARCH}_${1}${3}.exe
}

function buildprof {

	buildexe $1 -fprofile-generate

	echo "   Profiling ..."
	echo bench | ./${EXENAME}_${MSYSTEM_CARCH}_$1.exe > /dev/null

	if [[ $? -eq 0 ]]; then
		rm ${EXENAME}_${MSYSTEM_CARCH}_$1.exe
		echo "   Using profile ..."
		buildexe ${1} -fprofile-use _pgo
	else
		rm ${EXENAME}_${MSYSTEM_CARCH}_$1.exe
		echo "   Profiling error! Rebuilding executable without PGO ..."
		buildexe ${1}
	fi

	rm -f *.gcda
}

archs=()

PGO=false
BGN=true
DBG=-DNDEBUG
NTS=-Wl,--insert-timestamp

for iter in "$@"; do
	if [[ "$iter" == "pgo" ]]; then
		PGO=true
	elif [[ "$iter" == "nobook" ]]; then
		BGN=false
	elif [[ "$iter" == "debug" ]]; then
		DBG=-DDEBUG
	elif [[ "$iter" == "nts" ]]; then
		NTS=-Wl,--no-insert-timestamp
	else
		archs+=($iter)
	fi
done

# Add default required archs here (see https://gcc.gnu.org/onlinedocs/gcc/x86-Options.html)
# archs=(core2 nehalem skylake bdver2 znver1)
if [[ ${#archs[@]} -eq 0 ]]; then
	archs=(core2 bdver2)
fi

echo Going to build for [${archs[*]}]...
echo PGO = $PGO
echo BGN = $BGN
#echo DBG = $DBG
#echo NTS = $NTS

rm -f src/combined.cpp

cat src/*.cpp > src/combined.cpp

if [[ "$BGN" == "true" ]]; then
	mv src/book_gen.h src/book_gen.h.bk
	# Internal book generator
	echo -n "Building instrumental internal book generator binary ... "
	g++ -Ofast -march=native -std=c++14 -fno-rtti -fno-stack-protector -fno-exceptions -fwhole-program -DBOOKGEN -DNDEBUG -DNO_THREADS -D_FORTIFY_SOURCE=0 src/combined.cpp -static -o ${EXENAME}_bookgen.exe
	echo "generating ..."
	./${EXENAME}_bookgen.exe > /dev/null
	rm ${EXENAME}_bookgen.exe
fi

for arch in "${archs[@]}"; do
	echo '->' "Building $MSYSTEM_CARCH for $arch ..."

	if [[ "$PGO" == "true" ]]; then
		buildprof $arch
	else
		buildexe $arch
	fi

	echo '<-' "Done."
done

if [[ "$BGN" == "true" ]]; then
	rm book_gen.h
	mv src/book_gen.h.bk src/book_gen.h
fi

rm src/combined.cpp
