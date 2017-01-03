#include "rodent.h"
#include "eval.h"

void cEngine::ScorePatterns(POS * p, eData * e) {

  U64 king_mask, rook_mask;
  static const U64 wb_mask = { SqBb(A7) | SqBb(B8) | SqBb(H7) | SqBb(G8) | SqBb(C1) | SqBb(F1) | SqBb(G2) | SqBb(B2) };
  static const U64 bb_mask = { SqBb(A2) | SqBb(B1) | SqBb(H2) | SqBb(G1) | SqBb(C8) | SqBb(F8) | SqBb(G7) | SqBb(B7) };

  // Rook blocked by uncastled king

  king_mask = SqBb(F1) | SqBb(G1);
  rook_mask = SqBb(G1) | SqBb(H1) | SqBb(H2);

  if ((PcBb(p, WC, K) & king_mask)
  &&  (PcBb(p, WC, R) & rook_mask)) Add(e, WC, -50, 0);

  king_mask = SqBb(B1) | SqBb(C1);
  rook_mask = SqBb(A1) | SqBb(B1) | SqBb(A2);

  if ((PcBb(p, WC, K) & king_mask)
  &&  (PcBb(p, WC, R) & rook_mask)) Add(e, WC, -50, 0);

  king_mask = SqBb(F8) | SqBb(G8);
  rook_mask = SqBb(G8) | SqBb(H8) | SqBb(H7);

  if ((PcBb(p, BC, K) & king_mask)
  &&  (PcBb(p, BC, R) & rook_mask)) Add(e, BC, -50, 0);

  king_mask = SqBb(B8) | SqBb(C8);
  rook_mask = SqBb(C8) | SqBb(B8) | SqBb(B7);

  if ((PcBb(p, BC, K) & king_mask)
  && (PcBb(p, BC, R) & rook_mask)) Add(e, BC, -50, 0);

  // trapped knight

  if (IsOnSq(p, WC, N, A7) && IsOnSq(p, BC, P, A6) && IsOnSq(p, BC, P, B7)) Add(e, WC, -150);
  if (IsOnSq(p, WC, N, H7) && IsOnSq(p, BC, P, H6) && IsOnSq(p, BC, P, G7)) Add(e, WC, -150);
  if (IsOnSq(p, BC, N, A2) && IsOnSq(p, WC, P, A3) && IsOnSq(p, WC, P, B2)) Add(e, BC, -150);
  if (IsOnSq(p, BC, N, H2) && IsOnSq(p, WC, P, H3) && IsOnSq(p, WC, P, G2)) Add(e, BC, -150);

  if (PcBb(p, WC, B) & wb_mask) {

    // white bishop trapped

    if (IsOnSq(p, WC, B, A7) && IsOnSq(p, BC, P, B6) ) Add(e, WC, -150);
    if (IsOnSq(p, WC, B, B8) && IsOnSq(p, BC, P, C7) ) Add(e, WC, -150);
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
    }

    if (IsOnSq(p, WC, B, G2)) {
      if (IsOnSq(p, WC, P, F3)) Add(e, WC, -10, -20);
      if (IsOnSq(p, WC, P, G3) && (IsOnSq(p, WC, P, H2) || IsOnSq(p, WC, P, F2))) Add(e, WC,  4);
      if (IsOnSq(p, BC, P, E4) && (IsOnSq(p, BC, P, D5) || IsOnSq(p, BC, P, F5))) Add(e, WC, -20);
    }
  }

  if (PcBb(p, BC, B) & bb_mask) {

    // black bishop trapped

    if (IsOnSq(p, BC, B, A2) && IsOnSq(p, WC, P, B3) ) Add(e, BC, -150);
    if (IsOnSq(p, BC, B, B1) && IsOnSq(p, WC, P, C2) ) Add(e, BC, -150);
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
    }
    if (IsOnSq(p, BC, B, G7)) {
      if (IsOnSq(p, BC, P, F6)) Add(e, BC, -10, -20);
      if (IsOnSq(p, BC, P, G6) && (IsOnSq(p, BC, P, H7) || IsOnSq(p, BC, P, G6))) Add(e, BC,  4);
      if (IsOnSq(p, WC, P, E5) && (IsOnSq(p, WC, P, D4) || IsOnSq(p, WC, P, F4))) Add(e, BC, -20);
    }
  }

  // Castling rights

  if (IsOnSq(p, WC, K, E1)) {
    if ((p->c_flags & W_KS) || (p->c_flags & W_QS)) Add(e, WC, 10, 0);
  }

  if (IsOnSq(p, BC, K, E8)) {
    if ((p->c_flags & B_KS) || (p->c_flags & B_QS)) Add(e, BC, 10, 0);
  }
}