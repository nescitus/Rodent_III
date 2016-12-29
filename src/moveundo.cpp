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
  p->mg_sc[sd] += Par.mg_pst[sd][ftp][fsq] - Par.mg_pst[sd][ftp][tsq];
  p->eg_sc[sd] += Par.eg_pst[sd][ftp][fsq] - Par.eg_pst[sd][ftp][tsq];

  if (ftp == K)
    p->king_sq[sd] = fsq;

  if (ttp != NO_TP) {
    p->pc[tsq] = Pc(op, ttp);
    p->cl_bb[op] ^= SqBb(tsq);
    p->tp_bb[ttp] ^= SqBb(tsq);
    p->phase += ph_value[ttp];
	p->cnt[op][ttp]++;
	p->mg_sc[op] += Par.mg_pst[op][ttp][tsq];
	p->eg_sc[op] += Par.eg_pst[op][ttp][tsq];
  }
  switch (MoveType(move)) {
  case NORMAL:
    break;

  case CASTLE:

    switch (tsq) {
      case C1: { fsq = A1; tsq = D1; break; }
      case G1: { fsq = H1; tsq = F1; break; }
      case C8: { fsq = A8; tsq = D8; break; }
      case G8: { fsq = H8; tsq = F8; break; }
    }

    p->pc[tsq] = NO_PC;
    p->pc[fsq] = Pc(sd, R);
    p->cl_bb[sd] ^= SqBb(fsq) | SqBb(tsq);
    p->tp_bb[R] ^= SqBb(fsq) | SqBb(tsq);
	p->mg_sc[sd] += Par.mg_pst[sd][R][fsq] - Par.mg_pst[sd][R][tsq];
	p->eg_sc[sd] += Par.eg_pst[sd][R][fsq] - Par.eg_pst[sd][R][tsq];
    break;

  case EP_CAP:
    tsq ^= 8;
    p->pc[tsq] = Pc(op, P);
    p->cl_bb[op] ^= SqBb(tsq);
    p->tp_bb[P] ^= SqBb(tsq);
    p->phase += ph_value[P];
	p->cnt[op][P]++;
	p->mg_sc[op] += Par.mg_pst[op][P][tsq];
	p->eg_sc[op] += Par.eg_pst[op][P][tsq];
    break;

  case EP_SET:
    break;

  case N_PROM: case B_PROM: case R_PROM: case Q_PROM:
    p->pc[fsq] = Pc(sd, P);
    p->tp_bb[P] ^= SqBb(fsq);
    p->tp_bb[ftp] ^= SqBb(fsq);
    p->phase += ph_value[P] - ph_value[ftp];
	p->cnt[sd][P]++;
	p->cnt[sd][ftp]--;
	p->mg_sc[sd] += Par.mg_pst[sd][P][fsq] - Par.mg_pst[sd][ftp][fsq];
	p->eg_sc[sd] += Par.eg_pst[sd][P][fsq] - Par.eg_pst[sd][ftp][fsq];
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
