#include "skeleton.h"

int Swap(POS *p, int from, int to) {

  int side, ply, type, score[32];
  U64 attackers, occ, type_bb;

  attackers = AttacksTo(p, to);
  occ = OccBb(p);
  score[0] = tp_value[TpOnSq(p, to)];
  type = TpOnSq(p, from);
  occ ^= SqBb(from);
  attackers |= (BB.BishAttacks(occ, to) & (p->tp_bb[B] | p->tp_bb[Q])) |
               (BB.RookAttacks(occ, to) & (p->tp_bb[R] | p->tp_bb[Q]));
  attackers &= occ;
  side = ((SqBb(from) & p->cl_bb[BC]) == 0); // so that we can call Swap() out of turn
  ply = 1;
  while (attackers & p->cl_bb[side]) {
    if (type == K) {
      score[ply++] = INF;
      break;
    }
    score[ply] = -score[ply - 1] + tp_value[type];
    for (type = P; type <= K; type++)
      if ((type_bb = PcBb(p, side, type) & attackers))
        break;
    occ ^= type_bb & -type_bb;
    attackers |= (BB.BishAttacks(occ, to) & (p->tp_bb[B] | p->tp_bb[Q])) |
                 (BB.RookAttacks(occ, to) & (p->tp_bb[R] | p->tp_bb[Q]));
    attackers &= occ;
    side ^= 1;
    ply++;
  }
  while (--ply)
    score[ply - 1] = -Max(-score[ply - 1], score[ply]);
  return score[0];
}
