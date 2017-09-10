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
#include <cstdio>
#include <cstring>

void ClearPosition(POS *p) {

    *p = {0};

    p->king_sq[WC] = NO_SQ;
    p->king_sq[BC] = NO_SQ;

    for (int sq = 0; sq < 64; sq++)
        p->pc[sq] = NO_PC;

    p->side = WC;
    p->ep_sq = NO_SQ;
}

void SetPosition(POS *p, const char *epd) {

    static const char pc_char[] = "PpNnBbRrQqKk";

    ClearPosition(p);
    Glob.moves_from_start = 0;

    for (int i = 56; i >= 0; i -= 8) {
        int j = 0, pc_loop;
        while (j < 8) {
            if (*epd >= '1' && *epd <= '8')
                for (pc_loop = 0; pc_loop < *epd - '0'; pc_loop++) {
                    p->pc[i + j] = NO_PC;
                    j++;
                }
            else {
                for (pc_loop = 0; pc_char[pc_loop] && pc_char[pc_loop] != *epd; pc_loop++)
                    ;

                if ( !pc_char[pc_loop] ) {
                    printf("info string FEN parsing error\n");
                    SetPosition(p, START_POS);
                    return;
                }

                p->pc[i + j] = pc_loop;
                p->cl_bb[Cl(pc_loop)] ^= SqBb(i + j);
                p->tp_bb[Tp(pc_loop)] ^= SqBb(i + j);

                if (Tp(pc_loop) == K)
                    p->king_sq[Cl(pc_loop)] = i + j;

                p->mg_sc[Cl(pc_loop)] += Par.mg_pst[Cl(pc_loop)][Tp(pc_loop)][i + j];
                p->eg_sc[Cl(pc_loop)] += Par.eg_pst[Cl(pc_loop)][Tp(pc_loop)][i + j];
                p->phase += ph_value[Tp(pc_loop)];
                p->cnt[Cl(pc_loop)][Tp(pc_loop)]++;
                j++;
            }
            epd++;
        }
        epd++;
    }
    if (*epd++ == 'w')
        p->side = WC;
    else
        p->side = BC;
    epd++;
    if (*epd == '-')
        epd++;
    else {
        if (*epd == 'K') {
            p->c_flags |= 1;
            epd++;
        }
        if (*epd == 'Q') {
            p->c_flags |= 2;
            epd++;
        }
        if (*epd == 'k') {
            p->c_flags |= 4;
            epd++;
        }
        if (*epd == 'q') {
            p->c_flags |= 8;
            epd++;
        }
    }
    epd++;
    if (*epd == '-')
        p->ep_sq = NO_SQ;
    else {
        p->ep_sq = Sq(*epd - 'a', *(epd + 1) - '1');
        if (!(BB.PawnAttacks(Opp(p->side), p->ep_sq) & p->Pawns(p->side)))
            p->ep_sq = NO_SQ;
    }
    p->InitHashKey();
    p->InitPawnKey();
}
