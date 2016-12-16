#include "rodent.h"

U64 AttacksFrom(POS *p, int sq) {

  switch (TpOnSq(p, sq)) {
  case P:
    return p_attacks[Cl(p->pc[sq])][sq];
  case N:
    return n_attacks[sq];
  case B:
    return BAttacks(OccBb(p), sq);
  case R:
    return RAttacks(OccBb(p), sq);
  case Q:
    return QAttacks(OccBb(p), sq);
  case K:
    return k_attacks[sq];
  }
  return 0;
}

U64 AttacksTo(POS *p, int sq) {

  return (PcBb(p, WC, P) & p_attacks[BC][sq]) |
         (PcBb(p, BC, P) & p_attacks[WC][sq]) |
         (p->tp_bb[N] & n_attacks[sq]) |
         ((p->tp_bb[B] | p->tp_bb[Q]) & BAttacks(OccBb(p), sq)) |
         ((p->tp_bb[R] | p->tp_bb[Q]) & RAttacks(OccBb(p), sq)) |
         (p->tp_bb[K] & k_attacks[sq]);
}

int Attacked(POS *p, int sq, int sd) {

  return (PcBb(p, sd, P) & p_attacks[Opp(sd)][sq]) ||
         (PcBb(p, sd, N) & n_attacks[sq]) ||
         ((PcBb(p, sd, B) | PcBb(p, sd, Q)) & BAttacks(OccBb(p), sq)) ||
         ((PcBb(p, sd, R) | PcBb(p, sd, Q)) & RAttacks(OccBb(p), sq)) ||
         (PcBb(p, sd, K) & k_attacks[sq]);
}
