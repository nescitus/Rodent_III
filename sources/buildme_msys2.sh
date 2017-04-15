#!/bin/sh

if [ "$MSYSTEM_CARCH" = "i686" ]; then
	LSA="-Wl,--large-address-aware"
fi

case "$1" in
	1 )
		echo "Building using mingw..."

		gcc -Ofast -s -march=core2 -fno-stack-protector -fno-exceptions -DNDEBUG -DNO_THREADS -D_FORTIFY_SOURCE=0 src/*.cpp -static $LSA -o rodent_$MSYSTEM_CARCH.exe
		;;

	2 )
		echo "Building using clang with binary internal book..."

		clang -O -std=c++14 -march=core2 -fno-stack-protector -fno-exceptions -DBOOKGEN -DNDEBUG -DNO_THREADS -D_FORTIFY_SOURCE=0 src/*.cpp -static -o rodent_$MSYSTEM_CARCH.exe

		echo quit | ./rodent_$MSYSTEM_CARCH.exe

		clang -Ofast -s -std=c++14 -march=core2 -fno-stack-protector -fno-exceptions -DUSEGEN -I . -DNO_THREADS -D_FORTIFY_SOURCE=0 src/*.cpp -static $LSA -o rodent_$MSYSTEM_CARCH.exe

		rm book_gen.h
		;;

	3 )
		echo "Building using clang..."

		clang -Ofast -s -std=c++14 -march=core2 -fno-stack-protector -fno-exceptions -DNDEBUG -DNO_THREADS -D_FORTIFY_SOURCE=0 src/*.cpp -static $LSA -o rodent_$MSYSTEM_CARCH.exe
		;;

	d )
		echo "Building debug using mingw..."

		g++ -Og -g -march=core2 -DNO_THREADS src/*.cpp -static $LSA -o rodent_debug_$MSYSTEM_CARCH.exe
		;;

	* )
		echo "Building using mingw with binary internal book..."

		gcc -O -march=core2 -fno-stack-protector -fno-exceptions -DBOOKGEN -DNDEBUG -DNO_THREADS -D_FORTIFY_SOURCE=0 src/*.cpp -static -o rodent_$MSYSTEM_CARCH.exe

		echo quit | ./rodent_$MSYSTEM_CARCH.exe

		gcc -Ofast -s -march=core2 -fno-stack-protector -fno-exceptions -DUSEGEN -I . -DNO_THREADS -D_FORTIFY_SOURCE=0 src/*.cpp -static $LSA -o rodent_$MSYSTEM_CARCH.exe

		rm book_gen.h

esac
