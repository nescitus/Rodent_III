#include "rodent.h"

void UndoMove(POS *p, int move, UNDO *u) {

  int side = Opp(p->side);
  int fsq = Fsq(move);
  int tsq = Tsq(move);
  int ftp = TpOnSq(p, tsq);
  int ttp = u->ttp;

  p->c_flags = u->c_flags;
  p->ep_sq = u->ep_sq;
  p->rev_moves = u->rev_moves;
  p->key = u->key;
  p->head--;
  p->pc[fsq] = Pc(side, ftp);
  p->pc[tsq] = NO_PC;
  p->cl_bb[side] ^= SqBb(fsq) | SqBb(tsq);
  p->tp_bb[ftp] ^= SqBb(fsq) | SqBb(tsq);
  p->pst[side] += pst[ftp][fsq] - pst[ftp][tsq];

  if (ftp == K)
    p->king_sq[side] = fsq;

  if (ttp != NO_TP) {
    p->pc[tsq] = Pc(Opp(side), ttp);
    p->cl_bb[Opp(side)] ^= SqBb(tsq);
    p->tp_bb[ttp] ^= SqBb(tsq);
    p->mat[Opp(side)] += tp_value[ttp];
    p->pst[Opp(side)] += pst[ttp][tsq];
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
    p->pc[tsq] = NO_PC;
    p->pc[fsq] = Pc(side, R);
    p->cl_bb[side] ^= SqBb(fsq) | SqBb(tsq);
    p->tp_bb[R] ^= SqBb(fsq) | SqBb(tsq);
    p->pst[side] += pst[R][fsq] - pst[R][tsq];
    break;

  case EP_CAP:
    tsq ^= 8;
    p->pc[tsq] = Pc(Opp(side), P);
    p->cl_bb[Opp(side)] ^= SqBb(tsq);
    p->tp_bb[P] ^= SqBb(tsq);
    p->mat[Opp(side)] += tp_value[P];
    p->pst[Opp(side)] += pst[P][tsq];
    break;

  case EP_SET:
    break;

  case N_PROM: case B_PROM: case R_PROM: case Q_PROM:
    p->pc[fsq] = Pc(side, P);
    p->tp_bb[P] ^= SqBb(fsq);
    p->tp_bb[ftp] ^= SqBb(fsq);
    p->mat[side] += tp_value[P] - tp_value[ftp];
    p->pst[side] += pst[P][fsq] - pst[ftp][fsq];
    break;
  }
  p->side ^= 1;
}

void UndoNull(POS *p, UNDO *u) {

  p->ep_sq = u->ep_sq;
  p->key = u->key;
  p->head--;
  p->rev_moves--;
  p->side ^= 1;
}
