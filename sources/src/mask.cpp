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

void cMask::Init(void) {

  // Kingside/queenside
  
  k_side = FILE_F_BB | FILE_G_BB | FILE_H_BB;
  q_side = FILE_A_BB | FILE_B_BB | FILE_C_BB;

  // Own/enemy half of the board

  home[WC] = RANK_1_BB | RANK_2_BB | RANK_3_BB | RANK_4_BB;
  home[BC] = RANK_8_BB | RANK_7_BB | RANK_6_BB | RANK_5_BB;
  away[WC] = RANK_8_BB | RANK_7_BB | RANK_6_BB | RANK_5_BB;
  away[BC] = RANK_1_BB | RANK_2_BB | RANK_3_BB | RANK_4_BB;

  // Castling zones

  qs_castle[WC] = SqBb(A1) | SqBb(B1) | SqBb(C1) | SqBb(A2) | SqBb(B2) | SqBb(C2);
  qs_castle[BC] = SqBb(A8) | SqBb(B8) | SqBb(C8) | SqBb(A7) | SqBb(B7) | SqBb(C7);
  ks_castle[WC] = SqBb(F1) | SqBb(G1) | SqBb(H1) | SqBb(F2) | SqBb(G2) | SqBb(H2);
  ks_castle[BC] = SqBb(F8) | SqBb(G8) | SqBb(H8) | SqBb(F7) | SqBb(G7) | SqBb(H7);

  // Adjacent files (for isolated pawn detection)

  for (int col = 0; col < 8; col++) {
    adjacent[col] = 0;
    if (col > 0) adjacent[col] |= FILE_A_BB << (col - 1);
    if (col < 7) adjacent[col] |= FILE_A_BB << (col + 1);
  }

  // Supported mask (for weak pawns detection)

  for (int sq = 0; sq < 64; sq++) {
    supported[WC][sq] = BB.ShiftSideways(SqBb(sq));
    supported[WC][sq] |= BB.FillSouth(supported[WC][sq]);

    supported[BC][sq] = BB.ShiftSideways(SqBb(sq));
    supported[BC][sq] |= BB.FillNorth(supported[BC][sq]);
  }

  // Init mask for passed pawn detection

  for (int sq = 0; sq < 64; sq++) {
    passed[WC][sq] = BB.FillNorthExcl(SqBb(sq));
    passed[WC][sq] |= BB.ShiftSideways(passed[WC][sq]);
    passed[BC][sq] = BB.FillSouthExcl(SqBb(sq));
    passed[BC][sq] |= BB.ShiftSideways(passed[BC][sq]);
  }

  // Squares requiring bishop pattern evaluation

  wb_special = { SqBb(A7) | SqBb(A6) | SqBb(B8) | SqBb(H7) | SqBb(H6) | SqBb(G8) | SqBb(C1) | SqBb(F1) | SqBb(G2) | SqBb(B2) };
  bb_special = { SqBb(A2) | SqBb(A3) | SqBb(B1) | SqBb(H2) | SqBb(H3) | SqBb(G1) | SqBb(C8) | SqBb(F8) | SqBb(G7) | SqBb(B7) };

}