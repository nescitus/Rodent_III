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

void cEngine::EvaluateBishopPatterns(POS * p, eData * e) {

  if (p->Bishops(WC) & Mask.wb_special) {

    // white bishop trapped

    if (IsOnSq(p, WC, B, A6) && IsOnSq(p, BC, P, B5) ) Add(e, WC, -50);
    if (IsOnSq(p, WC, B, A7) && IsOnSq(p, BC, P, B6) ) Add(e, WC, -150);
    if (IsOnSq(p, WC, B, B8) && IsOnSq(p, BC, P, C7) ) Add(e, WC, -150);
	if (IsOnSq(p, WC, B, H6) && IsOnSq(p, BC, P, G5) ) Add(e, WC, -50);
    if (IsOnSq(p, WC, B, H7) && IsOnSq(p, BC, P, G6) ) Add(e, WC, -150);
    if (IsOnSq(p, WC, B, G8) && IsOnSq(p, BC, P, F7) ) Add(e, WC, -150);

    // white bishop blocked on its initial square by own pawn

    if (IsOnSq(p, WC, B, C1) && IsOnSq(p, WC, P, D2) && (SqBb(D3) & OccBb(p)))
      Add(e, WC, -50, 0);
    if (IsOnSq(p, WC, B, F1) && IsOnSq(p, WC, P, E2) && (SqBb(E3) & OccBb(p)))
      Add(e, WC, -50, 0);

    // white bishop fianchettoed

    if (IsOnSq(p, WC, B, B2)) {
      if (IsOnSq(p, WC, P, C3)) Add(e, WC, -10, -20);
      if (IsOnSq(p, WC, P, B3) && (IsOnSq(p, WC, P, A2) || IsOnSq(p, WC, P, C2))) Add(e, WC,  4);
      if (IsOnSq(p, BC, P, D4) && (IsOnSq(p, BC, P, E5) || IsOnSq(p, BC, P, C5))) Add(e, WC, -20);
	  if (p->Kings(WC) & Mask.qs_castle[WC]) Add(e, WC, Par.protecting_bishop, 0);
    }

    if (IsOnSq(p, WC, B, G2)) {
      if (IsOnSq(p, WC, P, F3)) Add(e, WC, -10, -20);
      if (IsOnSq(p, WC, P, G3) && (IsOnSq(p, WC, P, H2) || IsOnSq(p, WC, P, F2))) Add(e, WC,  4);
      if (IsOnSq(p, BC, P, E4) && (IsOnSq(p, BC, P, D5) || IsOnSq(p, BC, P, F5))) Add(e, WC, -20);
	  if (p->Kings(WC) & Mask.ks_castle[WC]) Add(e, WC, Par.protecting_bishop, 0);
    }
  }

  if (p->Bishops(BC) & Mask.bb_special) {

    // black bishop trapped

    if (IsOnSq(p, BC, B, A3) && IsOnSq(p, WC, P, B4) ) Add(e, BC, -50);
    if (IsOnSq(p, BC, B, A2) && IsOnSq(p, WC, P, B3) ) Add(e, BC, -150);
    if (IsOnSq(p, BC, B, B1) && IsOnSq(p, WC, P, C2) ) Add(e, BC, -150);
	if (IsOnSq(p, BC, B, H3) && IsOnSq(p, WC, P, G4) ) Add(e, BC, -50);
    if (IsOnSq(p, BC, B, H2) && IsOnSq(p, WC, P, G3) ) Add(e, BC, -150);
    if (IsOnSq(p, BC, B, G1) && IsOnSq(p, WC, P, F2) ) Add(e, BC, -150);

    // black bishop blocked on its initial square by own pawn

    if (IsOnSq(p, BC, B, C8) && IsOnSq(p, BC, P, D7) && (SqBb(D6) & OccBb(p)))
      Add(e, BC, -50, 0);
    if (IsOnSq(p, BC, B, F8) && IsOnSq(p, BC, P, E7) && (SqBb(E6) & OccBb(p)))
      Add(e, BC, -50, 0);

    // black bishop fianchettoed

    if (IsOnSq(p, BC, B, B7)) { 
      if (IsOnSq(p, BC, P, C6)) Add(e, BC, -10, -20);
      if (IsOnSq(p, BC, P, B6) && (IsOnSq(p, BC, P, A7) || IsOnSq(p, BC, P, C7))) Add(e, BC,  4);
      if (IsOnSq(p, WC, P, D5) && (IsOnSq(p, WC, P, E4) || IsOnSq(p, WC, P, C4))) Add(e, BC, -20); 
	  if (p->Kings(BC) & Mask.qs_castle[BC]) Add(e, BC, Par.protecting_bishop, 0);
    }
    if (IsOnSq(p, BC, B, G7)) {
      if (IsOnSq(p, BC, P, F6)) Add(e, BC, -10, -20);
      if (IsOnSq(p, BC, P, G6) && (IsOnSq(p, BC, P, H7) || IsOnSq(p, BC, P, G6))) Add(e, BC,  4);
      if (IsOnSq(p, WC, P, E5) && (IsOnSq(p, WC, P, D4) || IsOnSq(p, WC, P, F4))) Add(e, BC, -20);
	  if (p->Kings(BC) & Mask.ks_castle[BC]) Add(e, BC, Par.protecting_bishop, 0);
    }
  }

}

void cEngine::EvaluateKnightPatterns(POS * p, eData * e) {

  // trapped knight

  if (IsOnSq(p, WC, N, A7) && IsOnSq(p, BC, P, A6) && IsOnSq(p, BC, P, B7)) Add(e, WC, -150);
  if (IsOnSq(p, WC, N, H7) && IsOnSq(p, BC, P, H6) && IsOnSq(p, BC, P, G7)) Add(e, WC, -150);
  if (IsOnSq(p, BC, N, A2) && IsOnSq(p, WC, P, A3) && IsOnSq(p, WC, P, B2)) Add(e, BC, -150);
  if (IsOnSq(p, BC, N, H2) && IsOnSq(p, WC, P, H3) && IsOnSq(p, WC, P, G2)) Add(e, BC, -150);
}

void cEngine::EvaluateKingPatterns(POS * p, eData * e) {

  U64 king_mask, rook_mask;

  if (p->Kings(WC) & RANK_1_BB) {

    // White castled king that cannot escape upwards
  
    if (IsOnSq(p, WC, K, H1) && IsOnSq(p, WC, P, H2) && IsOnSq(p, WC, P, G2))
      Add(e, WC, -15);

    if (IsOnSq(p, WC, K, G1) && IsOnSq(p, WC, P, H2) && IsOnSq(p, WC, P, G2) && IsOnSq(p, WC, P, F2))
      Add(e, WC, -15);

    if (IsOnSq(p, WC, K, A1) && IsOnSq(p, WC, P, A2) && IsOnSq(p, WC, P, B2))
      Add(e, WC, -15);

    if (IsOnSq(p, WC, K, B1) && IsOnSq(p, WC, P, A2) && IsOnSq(p, WC, P, B2) && IsOnSq(p, WC, P, C2))
      Add(e, WC, -15);

    // White rook blocked by uncastled king

    king_mask = SqBb(F1) | SqBb(G1);
    rook_mask = SqBb(G1) | SqBb(H1) | SqBb(H2);

    if ((p->Kings(WC) & king_mask)
    && (p->Rooks(WC) & rook_mask)) Add(e, WC, -50, 0);

    king_mask = SqBb(B1) | SqBb(C1);
    rook_mask = SqBb(A1) | SqBb(B1) | SqBb(A2);

    if ((p->Kings(WC) & king_mask)
    && (p->Rooks(WC) & rook_mask)) Add(e, WC, -50, 0);

    // White castling rights

    if (IsOnSq(p, WC, K, E1)) {
      if ((p->c_flags & W_KS) || (p->c_flags & W_QS)) Add(e, WC, 10, 0);
    }
  }

  if (p->Kings(BC) & RANK_8_BB) {

    // Black castled king that cannot escape upwards

    if (IsOnSq(p, BC, K, H8) && IsOnSq(p, BC, P, H7) && IsOnSq(p, BC, P, G7))
      Add(e, BC, -15);

    if (IsOnSq(p, BC, K, G8) && IsOnSq(p, BC, P, H7) && IsOnSq(p, BC, P, G7) && IsOnSq(p, BC, P, F7))
      Add(e, BC, -15);

    if (IsOnSq(p, BC, K, A8) && IsOnSq(p, BC, P, A7) && IsOnSq(p, BC, P, B7))
      Add(e, BC, -15);

    if (IsOnSq(p, BC, K, B8) && IsOnSq(p, BC, P, A7) && IsOnSq(p, BC, P, B7) && IsOnSq(p, BC, P, C7))
      Add(e, BC, -15);

    // Black rook blocked by uncastled king

    king_mask = SqBb(F8) | SqBb(G8);
    rook_mask = SqBb(G8) | SqBb(H8) | SqBb(H7);

    if ((p->Kings(BC) & king_mask)
    && (p->Rooks(BC) & rook_mask)) Add(e, BC, -50, 0);

    king_mask = SqBb(B8) | SqBb(C8);
    rook_mask = SqBb(B8) | SqBb(A8) | SqBb(A7);

    if ((p->Kings(BC) & king_mask)
    && (p->Rooks(BC) & rook_mask)) Add(e, BC, -50, 0);

    // Black castling rights

    if (IsOnSq(p, BC, K, E8)) {
      if ((p->c_flags & B_KS) || (p->c_flags & B_QS)) Add(e, BC, 10, 0);
    }
  }
}

void cEngine::EvaluateCentralPatterns(POS * p, eData * e) {

  // Bishop and central pawn
  
  if (IsOnSq(p, WC, P, D4)) {
    if (p->Bishops(WC) & (SqBb(H2) | SqBb(G3) | SqBb(F4) | SqBb(G5) | SqBb(H4))) Add(e, WC, 10, 0);
  }

  if (IsOnSq(p, WC, P, E4)) {
    if (p->Bishops(WC) & (SqBb(A2) | SqBb(B3) | SqBb(C4) | SqBb(B5) | SqBb(A4))) Add(e, WC, 10, 0);
  }

  if (IsOnSq(p, BC, P, D5)) {
    if (p->Bishops(BC) & (SqBb(H7) | SqBb(G6) | SqBb(F5) | SqBb(G4) | SqBb(H5))) Add(e, BC, 10, 0);
  }

  if (IsOnSq(p, BC, P, E5)) {
    if (p->Bishops(BC) & (SqBb(A7) | SqBb(B6) | SqBb(C5) | SqBb(B4) | SqBb(A5))) Add(e, BC, 10, 0);
  }

  // Knight blocking c pawn

  if (IsOnSq(p, WC, P, C2) && IsOnSq(p, WC, P, D4) && IsOnSq(p, WC, N, C3)) {
     if ((p->Pawns(WC) & SqBb(E4)) == 0) Add (e, WC, -20, 0);
  }
  if (IsOnSq(p, BC, P, C7) && IsOnSq(p, BC, P, D5) && IsOnSq(p, BC, N, C6)) {
	  if ((p->Pawns(BC) & SqBb(E5)) == 0) Add(e, BC, -20, 0);
  }
}