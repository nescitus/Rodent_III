/*
Rodent, a UCI chess playing engine derived from Sungorus 1.4
Copyright (C) 2009-2011 Pablo Vazquez (Sungorus author)
Copyright (C) 2011-2017 Pawel Koziol

Rodent is free software: you can redistribute it and/or modify it under the terms of the GNU
General Public License as published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

Rodent is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along with this program.
If not, see <http://www.gnu.org/licenses/>.
*/

#include "rodent.h"
#include "book.h"
#include <cstdlib>

cGlobals Glob;

#ifdef USE_THREADS
    #include <list>
    std::list<cEngine> Engines(1);
#else
    cEngine EngineSingle(0);
#endif
cBitBoard BB;
cParam Par;
cMask Mask;
cDistance Dist;
sBook GuideBook;
sBook MainBook;

#ifndef USEGEN
    sInternalBook InternalBook;
#else
    #include "book_gen.h"
#endif

void PrintVersion() {

    printf("id name Rodent III 0.244"

#if !(defined(_WIN64) || defined(__x86_64__))
            " 32-bit"
#else
            " 64-bit"
#endif

#if   defined(__clang__)
            "/CLANG " __clang_version__
#elif defined(__MINGW32__)
            "/MINGW " __VERSION__
#elif defined(__GNUC__)
            "/GCC " __VERSION__
#elif defined(_MSC_VER)
            "/MSVS"
    #if   _MSC_VER == 1900
                "2015"
    #elif _MSC_VER >= 1910
                "2017"
    #endif
#endif

#if (defined(_MSC_VER) && defined(USE_MM_POPCNT)) || (defined(__GNUC__) && defined(__POPCNT__))
            "/POPCNT"
#elif defined(__GNUC__) && defined(__SSSE3__) // we are using custom SSSE3 popcount implementation
            "/SSSE3"
#endif

                        "\n");
}

int main() {

    // catching memory leaks using MS Visual Studio
#if defined(_MSC_VER) && !defined(NDEBUG)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    srand(GetMS());

    BB.Init();
    cEngine::InitSearch();
    POS::Init();
    Glob.Init();
    Par.DefaultWeights();
    Par.InitTables();
	Par.use_ponder = false;
    Mask.Init();
    Dist.Init();

	Par.use_book = true;
	Par.verbose_book = false;

    //PrintVersion();

if (Glob.is_noisy) {
#if defined(_WIN32) || defined(_WIN64)
    printf("info string opening books path is '%ls' (%s)\n", _BOOKSPATH, ChDir(_BOOKSPATH) ? "exists" : "doesn't exist");
    printf("info string personalities path is '%ls' (%s)\n", _PERSONALITIESPATH, ChDir(_PERSONALITIESPATH) ? "exists" : "doesn't exist");
#else
    printf("info string opening books path is '%s' (%s)\n", _BOOKSPATH, ChDir(_BOOKSPATH) ? "exists" : "doesn't exist");
    printf("info string personalities path is '%s' (%s)\n", _PERSONALITIESPATH, ChDir(_PERSONALITIESPATH) ? "exists" : "doesn't exist");
#endif
}

    PrintOverrides(); // print books and pers paths overrides (26/08/17: linux only)

#ifndef BOOKGEN
    GuideBook.SetBookName("guide.bin");
    MainBook.SetBookName("rodent.bin");
    ReadPersonality("basic.ini");

    // reading default personality
    if (Glob.use_personality_files)
        ReadPersonality("default.txt");
#endif

    InternalBook.Init();

#ifndef BOOKGEN
    UciLoop();
#endif
}

void cGlobals::Init() {

	is_noisy = false;
    is_testing = false;
    is_tuning = false;
    reading_personality = false;
    use_personality_files = true;
    use_books_from_pers = true;
    show_pers_file = true;
    thread_no = 1;
	time_buffer = 10; // blitz under Arena would require something like 200, but it's user's job
	game_key = 0;

    // Clearing  and  setting threads  may  be  necessary
    // if we need a compile using a bigger default number
    // of threads for testing purposes

#ifdef USE_THREADS
    if (thread_no > 1) { //-V547 get rid of PVS Studio warning
        Engines.clear();
        for (int i = 0; i < thread_no; i++)
            Engines.emplace_back(i);
    }
#endif

    should_clear = false;
    is_console = true;
    elo_slider = true;
	multiPv = 1;
}

bool cGlobals::CanReadBook() {
    return (use_books_from_pers == reading_personality || !use_personality_files);
}