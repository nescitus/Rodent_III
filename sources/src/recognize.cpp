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

int cEngine::IsDraw(POS *p) {

  // Draw by 50 move rule

  if (p->rev_moves > 100) return 1;

  // Draw by repetition

  for (int i = 4; i <= p->rev_moves; i += 2)
    if (p->hash_key == p->rep_list[p->head - i]) return 1;

  // With no major pieces on the board, we have some heuristic draws to consider

  if (p->cnt[WC][Q] + p->cnt[BC][Q] + p->cnt[WC][R] + p->cnt[BC][R] == 0) {

    // Draw by insufficient material (bare kings or Km vs K)

    if (!Illegal(p)) {
      if (p->cnt[WC][P] + p->cnt[BC][P] == 0) {
        if (p->cnt[WC][N] + p->cnt[BC][N] + p->cnt[WC][B] + p->cnt[BC][B] <= 1) return 1; // KK, KmK
      }
    }

    // Trivially drawn KPK endgames

    if (p->cnt[WC][B] + p->cnt[BC][B] + p->cnt[WC][N] + p->cnt[BC][N] == 0) {
      if (p->cnt[WC][P] + p->cnt[BC][P] == 1) {
        if (p->cnt[WC][P] == 1 ) return KPKdraw(p, WC); // exactly one white pawn
        if (p->cnt[BC][P] == 1 ) return KPKdraw(p, BC); // exactly one black pawn
      }
    } // pawns only
  }


  // Default: no draw

  return 0;
}

int cEngine::KPKdraw(POS *p, int sd) {

  int op = Opp(sd);
  U64 bbPawn = p->Pawns(sd);
  U64 bbStrongKing = p->Kings(sd);
  U64 bbWeakKing = p->Kings(op);

  // opposition through a pawn

  if (p->side == sd
  && (bbWeakKing & BB.ShiftFwd(bbPawn, sd))
  && (bbStrongKing & BB.ShiftFwd(bbPawn, op))
  ) return 1;
  
  // weaker side can create opposition through a pawn in one move

  if (p->side == op
  && (BB.KingAttacks(p->king_sq[op]) & BB.ShiftFwd(bbPawn, sd))
  && (bbStrongKing & BB.ShiftFwd(bbPawn, op))
  ) if (!Illegal(p)) return 1;

  // opposition next to a pawn
  
  if (p->side == sd
  && (bbStrongKing & BB.ShiftSideways(bbPawn))
  && (bbWeakKing & BB.ShiftFwd(BB.ShiftFwd(bbStrongKing,sd) ,sd)) 
  ) return 1;

  // TODO: pawn checks king

  return 0;
}
