#include "rodent.h"

void DoMove(POS *p, int move, UNDO *u)
{
  int side, fsq, tsq, ftp, ttp;

  side = p->side;
  fsq = Fsq(move);
  tsq = Tsq(move);
  ftp = TpOnSq(p, fsq);
  ttp = TpOnSq(p, tsq);
  u->ttp = ttp;
  u->c_flags = p->c_flags;
  u->ep_sq = p->ep_sq;
  u->rev_moves = p->rev_moves;
  u->key = p->key;
  p->rep_list[p->head++] = p->key;
  if (ftp == P || ttp != NO_TP)
    p->rev_moves = 0;
  else
    p->rev_moves++;
  p->key ^= zob_castle[p->c_flags];
  p->c_flags &= c_mask[fsq] & c_mask[tsq];
  p->key ^= zob_castle[p->c_flags];
  if (p->ep_sq != NO_SQ) {
    p->key ^= zob_ep[File(p->ep_sq)];
    p->ep_sq = NO_SQ;
  }
  p->pc[fsq] = NO_PC;
  p->pc[tsq] = Pc(side, ftp);
  p->key ^= zob_piece[Pc(side, ftp)][fsq] ^ zob_piece[Pc(side, ftp)][tsq];
  p->cl_bb[side] ^= SqBb(fsq) | SqBb(tsq);
  p->tp_bb[ftp] ^= SqBb(fsq) | SqBb(tsq);
  p->pst[side] += pst[ftp][tsq] - pst[ftp][fsq];
  if (ftp == K)
    p->king_sq[side] = tsq;
  if (ttp != NO_TP) {
    p->key ^= zob_piece[Pc(Opp(side), ttp)][tsq];
    p->cl_bb[Opp(side)] ^= SqBb(tsq);
    p->tp_bb[ttp] ^= SqBb(tsq);
    p->mat[Opp(side)] -= tp_value[ttp];
    p->pst[Opp(side)] -= pst[ttp][tsq];
  }
  switch (MoveType(move)) {
  case NORMAL:
    break;
  case CASTLE:
    if (tsq > fsq) {
      fsq += 3;
      tsq -= 1;
    } else {
      fsq -= 4;
      tsq += 1;
    }
    p->pc[fsq] = NO_PC;
    p->pc[tsq] = Pc(side, R);
    p->key ^= zob_piece[Pc(side, R)][fsq] ^ zob_piece[Pc(side, R)][tsq];
    p->cl_bb[side] ^= SqBb(fsq) | SqBb(tsq);
    p->tp_bb[R] ^= SqBb(fsq) | SqBb(tsq);
    p->pst[side] += pst[R][tsq] - pst[R][fsq];
    break;
  case EP_CAP:
    tsq ^= 8;
    p->pc[tsq] = NO_PC;
    p->key ^= zob_piece[Pc(Opp(side), P)][tsq];
    p->cl_bb[Opp(side)] ^= SqBb(tsq);
    p->tp_bb[P] ^= SqBb(tsq);
    p->mat[Opp(side)] -= tp_value[P];
    p->pst[Opp(side)] -= pst[P][tsq];
    break;
  case EP_SET:
    tsq ^= 8;
    if (p_attacks[side][tsq] & PcBb(p, Opp(side), P)) {
      p->ep_sq = tsq;
      p->key ^= zob_ep[File(tsq)];
    }
    break;
  case N_PROM: case B_PROM: case R_PROM: case Q_PROM:
    ftp = PromType(move);
    p->pc[tsq] = Pc(side, ftp);
    p->key ^= zob_piece[Pc(side, P)][tsq] ^ zob_piece[Pc(side, ftp)][tsq];
    p->tp_bb[P] ^= SqBb(tsq);
    p->tp_bb[ftp] ^= SqBb(tsq);
    p->mat[side] += tp_value[ftp] - tp_value[P];
    p->pst[side] += pst[ftp][tsq] - pst[P][tsq];
    break;
  }
  p->side ^= 1;
  p->key ^= SIDE_RANDOM;
}

void DoNull(POS *p, UNDO *u)
{
  u->ep_sq = p->ep_sq;
  u->key = p->key;
  p->rep_list[p->head++] = p->key;
  p->rev_moves++;
  if (p->ep_sq != NO_SQ) {
    p->key ^= zob_ep[File(p->ep_sq)];
    p->ep_sq = NO_SQ;
  }
  p->side ^= 1;
  p->key ^= SIDE_RANDOM;
}
