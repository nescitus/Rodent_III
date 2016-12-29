#include "rodent.h"

void DoMove(POS *p, int move, UNDO *u) {

  int sd = p->side;
  int op = Opp(sd);
  int fsq = Fsq(move);
  int tsq = Tsq(move);
  int ftp = TpOnSq(p, fsq);
  int ttp = TpOnSq(p, tsq);

  u->ttp = ttp;
  u->c_flags = p->c_flags;
  u->ep_sq = p->ep_sq;
  u->rev_moves = p->rev_moves;
  u->key = p->key;

  p->rep_list[p->head++] = p->key;
  if (ftp == P || ttp != NO_TP) p->rev_moves = 0;
  else                          p->rev_moves++;

  p->key ^= zob_castle[p->c_flags];
  p->c_flags &= c_mask[fsq] & c_mask[tsq];
  p->key ^= zob_castle[p->c_flags];
  if (p->ep_sq != NO_SQ) {
    p->key ^= zob_ep[File(p->ep_sq)];
    p->ep_sq = NO_SQ;
  }

  p->pc[fsq] = NO_PC;
  p->pc[tsq] = Pc(sd, ftp);
  p->key ^= zob_piece[Pc(sd, ftp)][fsq] ^ zob_piece[Pc(sd, ftp)][tsq];
  p->cl_bb[sd] ^= SqBb(fsq) | SqBb(tsq);
  p->tp_bb[ftp] ^= SqBb(fsq) | SqBb(tsq);
  p->mg_sc[sd] += Par.mg_pst[sd][ftp][tsq] - Par.mg_pst[sd][ftp][fsq];
  p->eg_sc[sd] += Par.eg_pst[sd][ftp][tsq] - Par.eg_pst[sd][ftp][fsq];

  if (ftp == K)
    p->king_sq[sd] = tsq;

  if (ttp != NO_TP) {
    p->key ^= zob_piece[Pc(op, ttp)][tsq];
    p->cl_bb[op] ^= SqBb(tsq);
    p->tp_bb[ttp] ^= SqBb(tsq);
    p->phase -= ph_value[ttp];
	p->mg_sc[op] -= Par.mg_pst[op][ttp][tsq];
	p->eg_sc[op] -= Par.eg_pst[op][ttp][tsq];
	p->cnt[op][ttp]--;
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

    p->pc[fsq] = NO_PC;
    p->pc[tsq] = Pc(sd, R);
    p->key ^= zob_piece[Pc(sd, R)][fsq] ^ zob_piece[Pc(sd, R)][tsq];
    p->cl_bb[sd] ^= SqBb(fsq) | SqBb(tsq);
    p->tp_bb[R] ^= SqBb(fsq) | SqBb(tsq);
	p->mg_sc[sd] += Par.mg_pst[sd][R][tsq] - Par.mg_pst[sd][R][fsq];
	p->eg_sc[sd] += Par.eg_pst[sd][R][tsq] - Par.eg_pst[sd][R][fsq];
    break;

  case EP_CAP:
    tsq ^= 8;
    p->pc[tsq] = NO_PC;
    p->key ^= zob_piece[Pc(op, P)][tsq];
    p->cl_bb[op] ^= SqBb(tsq);
    p->tp_bb[P] ^= SqBb(tsq);
    p->phase -= ph_value[P];
	p->cnt[op][P]--;
	p->mg_sc[op] -= Par.mg_pst[op][P][tsq];
	p->eg_sc[op] -= Par.eg_pst[op][P][tsq];
    break;

  case EP_SET:
    tsq ^= 8;
    if (p_attacks[sd][tsq] & PcBb(p, op, P)) {
      p->ep_sq = tsq;
      p->key ^= zob_ep[File(tsq)];
    }
    break;

  case N_PROM: case B_PROM: case R_PROM: case Q_PROM:
    ftp = PromType(move);
    p->pc[tsq] = Pc(sd, ftp);
    p->key ^= zob_piece[Pc(sd, P)][tsq] ^ zob_piece[Pc(sd, ftp)][tsq];
    p->tp_bb[P] ^= SqBb(tsq);
    p->tp_bb[ftp] ^= SqBb(tsq);
    p->phase += ph_value[ftp] - ph_value[P];
    p->cnt[sd][P]--;
    p->cnt[sd][ftp]++;
	p->mg_sc[sd] += Par.mg_pst[sd][ftp][tsq] - Par.mg_pst[sd][P][tsq];
	p->eg_sc[sd] += Par.eg_pst[sd][ftp][tsq] - Par.eg_pst[sd][P][tsq];
    break;
  }
  p->side ^= 1;
  p->key ^= SIDE_RANDOM;
}

void DoNull(POS *p, UNDO *u) {

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
