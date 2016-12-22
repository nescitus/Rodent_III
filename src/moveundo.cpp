#include "rodent.h"

void UndoMove(POS *p, int move, UNDO *u) {

  int sd = Opp(p->side);
  int op = p->side;
  int fsq = Fsq(move);
  int tsq = Tsq(move);
  int ftp = TpOnSq(p, tsq);
  int ttp = u->ttp;

  p->c_flags = u->c_flags;
  p->ep_sq = u->ep_sq;
  p->rev_moves = u->rev_moves;
  p->key = u->key;
  p->head--;
  p->pc[fsq] = Pc(sd, ftp);
  p->pc[tsq] = NO_PC;
  p->cl_bb[sd] ^= SqBb(fsq) | SqBb(tsq);
  p->tp_bb[ftp] ^= SqBb(fsq) | SqBb(tsq);
  p->pst[sd] += pst[ftp][fsq] - pst[ftp][tsq];

  if (ftp == K)
    p->king_sq[sd] = fsq;

  if (ttp != NO_TP) {
    p->pc[tsq] = Pc(op, ttp);
    p->cl_bb[op] ^= SqBb(tsq);
    p->tp_bb[ttp] ^= SqBb(tsq);
    p->mat[op] += tp_value[ttp];
    p->pst[op] += pst[ttp][tsq];
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
    p->pc[fsq] = Pc(sd, R);
    p->cl_bb[sd] ^= SqBb(fsq) | SqBb(tsq);
    p->tp_bb[R] ^= SqBb(fsq) | SqBb(tsq);
    p->pst[sd] += pst[R][fsq] - pst[R][tsq];
    break;

  case EP_CAP:
    tsq ^= 8;
    p->pc[tsq] = Pc(op, P);
    p->cl_bb[op] ^= SqBb(tsq);
    p->tp_bb[P] ^= SqBb(tsq);
    p->mat[op] += tp_value[P];
    p->pst[op] += pst[P][tsq];
    break;

  case EP_SET:
    break;

  case N_PROM: case B_PROM: case R_PROM: case Q_PROM:
    p->pc[fsq] = Pc(sd, P);
    p->tp_bb[P] ^= SqBb(fsq);
    p->tp_bb[ftp] ^= SqBb(fsq);
    p->mat[sd] += tp_value[P] - tp_value[ftp];
    p->pst[sd] += pst[P][fsq] - pst[ftp][fsq];
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
