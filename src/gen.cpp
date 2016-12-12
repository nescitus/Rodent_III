#include "rodent.h"

int *GenerateCaptures(POS *p, int *list)
{
  U64 pieces, moves;
  int side, from, to;

  side = p->side;
  if (side == WC) {
    moves = ((PcBb(p, WC, P) & ~FILE_A_BB & RANK_7_BB) << 7) & p->cl_bb[BC];
    while (moves) {
      to = FirstOne(moves);
      *list++ = (Q_PROM << 12) | (to << 6) | (to - 7);
      *list++ = (R_PROM << 12) | (to << 6) | (to - 7);
      *list++ = (B_PROM << 12) | (to << 6) | (to - 7);
      *list++ = (N_PROM << 12) | (to << 6) | (to - 7);
      moves &= moves - 1;
    }
    moves = ((PcBb(p, WC, P) & ~FILE_H_BB & RANK_7_BB) << 9) & p->cl_bb[BC];
    while (moves) {
      to = FirstOne(moves);
      *list++ = (Q_PROM << 12) | (to << 6) | (to - 9);
      *list++ = (R_PROM << 12) | (to << 6) | (to - 9);
      *list++ = (B_PROM << 12) | (to << 6) | (to - 9);
      *list++ = (N_PROM << 12) | (to << 6) | (to - 9);
      moves &= moves - 1;
    }
    moves = ((PcBb(p, WC, P) & RANK_7_BB) << 8) & UnoccBb(p);
    while (moves) {
      to = FirstOne(moves);
      *list++ = (Q_PROM << 12) | (to << 6) | (to - 8);
      *list++ = (R_PROM << 12) | (to << 6) | (to - 8);
      *list++ = (B_PROM << 12) | (to << 6) | (to - 8);
      *list++ = (N_PROM << 12) | (to << 6) | (to - 8);
      moves &= moves - 1;
    }
    moves = ((PcBb(p, WC, P) & ~FILE_A_BB & ~RANK_7_BB) << 7) & p->cl_bb[BC];
    while (moves) {
      to = FirstOne(moves);
      *list++ = (to << 6) | (to - 7);
      moves &= moves - 1;
    }
    moves = ((PcBb(p, WC, P) & ~FILE_H_BB & ~RANK_7_BB) << 9) & p->cl_bb[BC];
    while (moves) {
      to = FirstOne(moves);
      *list++ = (to << 6) | (to - 9);
      moves &= moves - 1;
    }
    if ((to = p->ep_sq) != NO_SQ) {
      if (((PcBb(p, WC, P) & ~FILE_A_BB) << 7) & SqBb(to))
        *list++ = (EP_CAP << 12) | (to << 6) | (to - 7);
      if (((PcBb(p, WC, P) & ~FILE_H_BB) << 9) & SqBb(to))
        *list++ = (EP_CAP << 12) | (to << 6) | (to - 9);
    }
  } else {
    moves = ((PcBb(p, BC, P) & ~FILE_A_BB & RANK_2_BB) >> 9) & p->cl_bb[WC];
    while (moves) {
      to = FirstOne(moves);
      *list++ = (Q_PROM << 12) | (to << 6) | (to + 9);
      *list++ = (R_PROM << 12) | (to << 6) | (to + 9);
      *list++ = (B_PROM << 12) | (to << 6) | (to + 9);
      *list++ = (N_PROM << 12) | (to << 6) | (to + 9);
      moves &= moves - 1;
    }
    moves = ((PcBb(p, BC, P) & ~FILE_H_BB & RANK_2_BB) >> 7) & p->cl_bb[WC];
    while (moves) {
      to = FirstOne(moves);
      *list++ = (Q_PROM << 12) | (to << 6) | (to + 7);
      *list++ = (R_PROM << 12) | (to << 6) | (to + 7);
      *list++ = (B_PROM << 12) | (to << 6) | (to + 7);
      *list++ = (N_PROM << 12) | (to << 6) | (to + 7);
      moves &= moves - 1;
    }
    moves = ((PcBb(p, BC, P) & RANK_2_BB) >> 8) & UnoccBb(p);
    while (moves) {
      to = FirstOne(moves);
      *list++ = (Q_PROM << 12) | (to << 6) | (to + 8);
      *list++ = (R_PROM << 12) | (to << 6) | (to + 8);
      *list++ = (B_PROM << 12) | (to << 6) | (to + 8);
      *list++ = (N_PROM << 12) | (to << 6) | (to + 8);
      moves &= moves - 1;
    }
    moves = ((PcBb(p, BC, P) & ~FILE_A_BB & ~RANK_2_BB) >> 9) & p->cl_bb[WC];
    while (moves) {
      to = FirstOne(moves);
      *list++ = (to << 6) | (to + 9);
      moves &= moves - 1;
    }
    moves = ((PcBb(p, BC, P) & ~FILE_H_BB & ~RANK_2_BB) >> 7) & p->cl_bb[WC];
    while (moves) {
      to = FirstOne(moves);
      *list++ = (to << 6) | (to + 7);
      moves &= moves - 1;
    }
    if ((to = p->ep_sq) != NO_SQ) {
      if (((PcBb(p, BC, P) & ~FILE_A_BB) >> 9) & SqBb(to))
        *list++ = (EP_CAP << 12) | (to << 6) | (to + 9);
      if (((PcBb(p, BC, P) & ~FILE_H_BB) >> 7) & SqBb(to))
        *list++ = (EP_CAP << 12) | (to << 6) | (to + 7);
    }
  }
  pieces = PcBb(p, side, N);
  while (pieces) {
    from = FirstOne(pieces);
    moves = n_attacks[from] & p->cl_bb[Opp(side)];
    while (moves) {
      to = FirstOne(moves);
      *list++ = (to << 6) | from;
      moves &= moves - 1;
    }
    pieces &= pieces - 1;
  }
  pieces = PcBb(p, side, B);
  while (pieces) {
    from = FirstOne(pieces);
    moves = BAttacks(OccBb(p), from) & p->cl_bb[Opp(side)];
    while (moves) {
      to = FirstOne(moves);
      *list++ = (to << 6) | from;
      moves &= moves - 1;
    }
    pieces &= pieces - 1;
  }
  pieces = PcBb(p, side, R);
  while (pieces) {
    from = FirstOne(pieces);
    moves = RAttacks(OccBb(p), from) & p->cl_bb[Opp(side)];
    while (moves) {
      to = FirstOne(moves);
      *list++ = (to << 6) | from;
      moves &= moves - 1;
    }
    pieces &= pieces - 1;
  }
  pieces = PcBb(p, side, Q);
  while (pieces) {
    from = FirstOne(pieces);
    moves = QAttacks(OccBb(p), from) & p->cl_bb[Opp(side)];
    while (moves) {
      to = FirstOne(moves);
      *list++ = (to << 6) | from;
      moves &= moves - 1;
    }
    pieces &= pieces - 1;
  }
  moves = k_attacks[KingSq(p, side)] & p->cl_bb[Opp(side)];
  while (moves) {
    to = FirstOne(moves);
    *list++ = (to << 6) | KingSq(p, side);
    moves &= moves - 1;
  }
  return list;
}

int *GenerateQuiet(POS *p, int *list)
{
  U64 pieces, moves;
  int side, from, to;

  side = p->side;
  if (side == WC) {
    if ((p->c_flags & 1) && !(OccBb(p) & (U64)0x0000000000000060))
      if (!Attacked(p, E1, BC) && !Attacked(p, F1, BC))
        *list++ = (CASTLE << 12) | (G1 << 6) | E1;
    if ((p->c_flags & 2) && !(OccBb(p) & (U64)0x000000000000000E))
      if (!Attacked(p, E1, BC) && !Attacked(p, D1, BC))
        *list++ = (CASTLE << 12) | (C1 << 6) | E1;
    moves = ((((PcBb(p, WC, P) & RANK_2_BB) << 8) & UnoccBb(p)) << 8) & UnoccBb(p);
    while (moves) {
      to = FirstOne(moves);
      *list++ = (EP_SET << 12) | (to << 6) | (to - 16);
      moves &= moves - 1;
    }
    moves = ((PcBb(p, WC, P) & ~RANK_7_BB) << 8) & UnoccBb(p);
    while (moves) {
      to = FirstOne(moves);
      *list++ = (to << 6) | (to - 8);
      moves &= moves - 1;
    }
  } else {
    if ((p->c_flags & 4) && !(OccBb(p) & (U64)0x6000000000000000))
      if (!Attacked(p, E8, WC) && !Attacked(p, F8, WC))
        *list++ = (CASTLE << 12) | (G8 << 6) | E8;
    if ((p->c_flags & 8) && !(OccBb(p) & (U64)0x0E00000000000000))
      if (!Attacked(p, E8, WC) && !Attacked(p, D8, WC))
        *list++ = (CASTLE << 12) | (C8 << 6) | E8;
    moves = ((((PcBb(p, BC, P) & RANK_7_BB) >> 8) & UnoccBb(p)) >> 8) & UnoccBb(p);
    while (moves) {
      to = FirstOne(moves);
      *list++ = (EP_SET << 12) | (to << 6) | (to + 16);
      moves &= moves - 1;
    }
    moves = ((PcBb(p, BC, P) & ~RANK_2_BB) >> 8) & UnoccBb(p);
    while (moves) {
      to = FirstOne(moves);
      *list++ = (to << 6) | (to + 8);
      moves &= moves - 1;
    }
  }
  pieces = PcBb(p, side, N);
  while (pieces) {
    from = FirstOne(pieces);
    moves = n_attacks[from] & UnoccBb(p);
    while (moves) {
      to = FirstOne(moves);
      *list++ = (to << 6) | from;
      moves &= moves - 1;
    }
    pieces &= pieces - 1;
  }
  pieces = PcBb(p, side, B);
  while (pieces) {
    from = FirstOne(pieces);
    moves = BAttacks(OccBb(p), from) & UnoccBb(p);
    while (moves) {
      to = FirstOne(moves);
      *list++ = (to << 6) | from;
      moves &= moves - 1;
    }
    pieces &= pieces - 1;
  }
  pieces = PcBb(p, side, R);
  while (pieces) {
    from = FirstOne(pieces);
    moves = RAttacks(OccBb(p), from) & UnoccBb(p);
    while (moves) {
      to = FirstOne(moves);
      *list++ = (to << 6) | from;
      moves &= moves - 1;
    }
    pieces &= pieces - 1;
  }
  pieces = PcBb(p, side, Q);
  while (pieces) {
    from = FirstOne(pieces);
    moves = QAttacks(OccBb(p), from) & UnoccBb(p);
    while (moves) {
      to = FirstOne(moves);
      *list++ = (to << 6) | from;
      moves &= moves - 1;
    }
    pieces &= pieces - 1;
  }
  moves = k_attacks[KingSq(p, side)] & UnoccBb(p);
  while (moves) {
    to = FirstOne(moves);
    *list++ = (to << 6) | KingSq(p, side);
    moves &= moves - 1;
  }
  return list;
}
