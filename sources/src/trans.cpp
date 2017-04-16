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
#include "chessheapclass.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>

ChessHeapClass chc;

void AllocTrans(int mbsize) {

    for (tt_size = 2; tt_size <= mbsize; tt_size *= 2)
        ;

    tt_size /= 2;

    if (chc.Alloc(tt_size))
        printf("info string %zuMB of memory allocated\n", tt_size);
    else
        printf("info string memory allocation error\n");

    tt_size = tt_size * (1024 * 1024 / sizeof(ENTRY));
    tt_mask = tt_size - 4;
}

void ClearTrans(void) {

    tt_date = 0;

    chc.ZeroMem();
}

int TransRetrieve(U64 key, int *move, int *score, int alpha, int beta, int depth, int ply) {

    if (!chc.success) return 0;

    ENTRY *entry;

    entry = chc[key & tt_mask];
    for (int i = 0; i < 4; i++) {
        if (entry->key == key) {
            entry->date = tt_date;
            *move = entry->move;
            if (entry->depth >= depth) {
                *score = entry->score;
                if (*score < -MAX_EVAL)
                    *score += ply;
                else if (*score > MAX_EVAL)
                    *score -= ply;
                if ((entry->flags & UPPER && *score <= alpha)
                        || (entry->flags & LOWER && *score >= beta)) {
                    //entry->date = tt_date; // refreshing entry TODO: test at 4 threads, at 1 thread it's a wash
                    return 1;
                }
            }
            break;
        }
        entry++;
    }
    return 0;
}

void TransRetrieveMove(U64 key, int *move) {

    if (!chc.success) return;

    ENTRY *entry;

    entry = chc[key & tt_mask];
    for (int i = 0; i < 4; i++) {
        if (entry->key == key) {
            entry->date = tt_date; // TODO: test without this line (very low priority, long test)
            *move = entry->move;
            break;
        }
        entry++;
    }
}

void TransStore(U64 key, int move, int score, int flags, int depth, int ply) {

    if (!chc.success) return;

    ENTRY *entry, *replace;
    int oldest, age;

    if (score < -MAX_EVAL)
        score -= ply;
    else if (score > MAX_EVAL)
        score += ply;
    replace = NULL;
    oldest = -1;
    entry = chc[key & tt_mask];
    for (int i = 0; i < 4; i++) {
        if (entry->key == key) {
            if (!move) move = entry->move;
            replace = entry;
            break;
        }
        age = ((tt_date - entry->date) & 255) * 256 + 255 - entry->depth;
        if (age > oldest) {
            oldest = age;
            replace = entry;
        }
        entry++;
    }
    replace->key = key; replace->date = tt_date; replace->move = move;
    replace->score = score; replace->flags = flags; replace->depth = depth;
}
