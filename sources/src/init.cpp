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

void POS::Init() { // static init function

    for (int sq = 0; sq < 64; sq++)
        msCastleMask[sq] = 15;

    msCastleMask[A1] = 13;
    msCastleMask[E1] = 12;
    msCastleMask[H1] = 14;
    msCastleMask[A8] = 7;
    msCastleMask[E8] = 3;
    msCastleMask[H8] = 11;

    for (int i = 0; i < 12; i++)
        for (int j = 0; j < 64; j++)
            msZobPiece[i][j] = Random64();
    for (int i = 0; i < 16; i++)
        msZobCastle[i] = Random64();
    for (int i = 0; i < 8; i++)
        msZobEp[i] = Random64();
}
