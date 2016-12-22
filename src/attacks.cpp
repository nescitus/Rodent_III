#include "skeleton.h"

U64 AttacksFrom(POS *p, int sq) {

  switch (TpOnSq(p, sq)) {
  case P:
    return BB.PawnAttacks(Cl(p->pc[sq]),sq);
  case N:
    return BB.KnightAttacks(sq);
  case B:
    return BB.BishAttacks(OccBb(p), sq);
  case R:
    return BB.RookAttacks(OccBb(p), sq);
  case Q:
    return BB.QueenAttacks(OccBb(p), sq);
  case K:
    return BB.KingAttacks(sq);
  }
  return 0;
}

U64 AttacksTo(POS *p, int sq) {

  return (p->Pawns(WC) & BB.PawnAttacks(BC,sq)) |
         (p->Pawns(BC) & BB.PawnAttacks(WC,sq)) |
         (p->tp_bb[N] & BB.KnightAttacks(sq)) |
         ((p->tp_bb[B] | p->tp_bb[Q]) & BB.BishAttacks(OccBb(p), sq)) |
         ((p->tp_bb[R] | p->tp_bb[Q]) & BB.RookAttacks(OccBb(p), sq)) |
         (p->tp_bb[K] & BB.KingAttacks(sq));
}

int Attacked(POS *p, int sq, int sd) {

  return (p->Pawns(sd) & BB.PawnAttacks(Opp(sd),sq)) ||
         (p->Knights(sd) & BB.KnightAttacks(sq)) ||
         (p->DiagMovers(sd) & BB.BishAttacks(OccBb(p), sq)) ||
         (p->StraightMovers(sd)) & BB.RookAttacks(OccBb(p), sq) ||
         (p->Kings(sd) & BB.KingAttacks(sq));
}
