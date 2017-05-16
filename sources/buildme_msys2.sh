#!/bin/bash

if [[ "$MSYSTEM_CARCH" == "i686" ]]; then
	LSA="-Wl,--large-address-aware,--gc-sections"
else
	LSA="-Wl,--gc-sections"
fi

if [[ "$1" == *"t"* ]]; then
	#THR="-DUSE_THREADS"
	CCGCC="g++ -fno-rtti"
	CCCLANG="clang++ -fno-rtti"
else
	THR="-DNO_THREADS"
	CCGCC="gcc"
	CCCLANG="clang"
fi

if [[ "$1" == *"n"* ]]; then
	NTHR="-DNEW_THREADS"
else
	NTHR=""
fi

mv src/book_gen.h src/book_gen.h.bk

case "$1" in
	1 )
		echo "Building using mingw..."

		gcc -Ofast -s -march=core2 -fno-stack-protector -fno-exceptions -flto -DNDEBUG -DNO_THREADS -D_FORTIFY_SOURCE=0 src/*.cpp -static $LSA -o rodent_$MSYSTEM_CARCH.exe
		;;

	2 )
		echo "Building using clang with binary internal book..."

		clang -O2 -std=c++14 -march=core2 -fno-stack-protector -fno-exceptions -DBOOKGEN -DNDEBUG -DNO_THREADS -D_FORTIFY_SOURCE=0 src/*.cpp -static -o rodent_$MSYSTEM_CARCH.exe

		./rodent_$MSYSTEM_CARCH.exe

		clang -Ofast -s -std=c++14 -march=core2 -fno-stack-protector -fno-exceptions -DUSEGEN -DNDEBUG -DNO_THREADS -D_FORTIFY_SOURCE=0 -I . src/*.cpp -static $LSA -o rodent_$MSYSTEM_CARCH.exe

		rm book_gen.h
		;;

	3 )
		echo "Building using clang..."

		clang -Ofast -s -std=c++14 -march=core2 -fno-stack-protector -fno-exceptions -DNDEBUG -DNO_THREADS -D_FORTIFY_SOURCE=0 src/*.cpp -static $LSA -o rodent_$MSYSTEM_CARCH.exe
		;;

	*d* )
		echo "Building debug using mingw..."

		g++ -Og -g -march=core2 $THR $NTHR src/*.cpp -static $LSA -o rodent_debug_$MSYSTEM_CARCH.exe
		;;

	*a* )
		echo "Building using mingw with binary internal book (amalgamated)..."

		cat src/*.cpp > src/combined.cpp

		gcc -O2 -march=core2 -fno-stack-protector -fno-exceptions -fwhole-program -DBOOKGEN -DNDEBUG -DNO_THREADS -D_FORTIFY_SOURCE=0 src/combined.cpp -static -o rodent_$MSYSTEM_CARCH.exe

		./rodent_$MSYSTEM_CARCH.exe

		$CCGCC -Ofast -s -march=core2 -fno-stack-protector -fno-exceptions -fno-unwind-tables -fno-asynchronous-unwind-tables -fno-ident -fwhole-program -DUSEGEN -DNDEBUG $THR $NTHR -D_FORTIFY_SOURCE=0 -I . src/combined.cpp -static $LSA -o rodent_$MSYSTEM_CARCH.exe

		rm book_gen.h
		rm src/combined.cpp
		;;

	*c* )
		echo "Building using clang with binary internal book (amalgamated)..."

		cat src/*.cpp > src/combined.cpp

		clang -std=c++14 -O2 -march=core2 -fno-stack-protector -fno-exceptions -DBOOKGEN -DNDEBUG -DNO_THREADS -D_FORTIFY_SOURCE=0 src/combined.cpp -static -o rodent_$MSYSTEM_CARCH.exe

		./rodent_$MSYSTEM_CARCH.exe

		$CCCLANG -std=c++14 -Ofast -s -march=core2 -fno-stack-protector -fno-exceptions -fno-unwind-tables -fno-asynchronous-unwind-tables -fno-ident -DUSEGEN -DNDEBUG $THR $NTHR -D_FORTIFY_SOURCE=0 -I . src/combined.cpp -static $LSA -o rodent_$MSYSTEM_CARCH.exe

		rm book_gen.h
		rm src/combined.cpp
		;;

	* )
		echo "Building using mingw with binary internal book..."

		gcc -O2 -march=core2 -fno-stack-protector -fno-exceptions -DBOOKGEN -DNDEBUG -DNO_THREADS -D_FORTIFY_SOURCE=0 src/*.cpp -static -o rodent_$MSYSTEM_CARCH.exe

		./rodent_$MSYSTEM_CARCH.exe

		$CCGCC -Ofast -s -march=core2 -fno-stack-protector -fno-exceptions -flto -DUSEGEN -DNDEBUG $THR $NTHR -D_FORTIFY_SOURCE=0 -I . src/*.cpp -static $LSA -o rodent_$MSYSTEM_CARCH.exe

		rm book_gen.h

esac

mv src/book_gen.h.bk src/book_gen.h
