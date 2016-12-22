#include "skeleton.h"

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
  u->hash_key = p->hash_key;
  u->pawn_key = p->pawn_key;

  // Handle repetition data

  p->rep_list[p->head++] = p->hash_key;
  if (ftp == P || ttp != NO_TP) p->rev_moves = 0;
  else                          p->rev_moves++;

  // Update pawn hash

  if (ftp == P || ftp == K)
    p->pawn_key ^= zob_piece[Pc(sd, ftp)][fsq] ^ zob_piece[Pc(sd, ftp)][tsq];

  p->hash_key ^= zob_castle[p->c_flags];
  p->c_flags &= c_mask[fsq] & c_mask[tsq];
  p->hash_key ^= zob_castle[p->c_flags];
  if (p->ep_sq != NO_SQ) {
    p->hash_key ^= zob_ep[File(p->ep_sq)];
    p->ep_sq = NO_SQ;
  }
  p->pc[fsq] = NO_PC;
  p->pc[tsq] = Pc(sd, ftp);
  p->hash_key ^= zob_piece[Pc(sd, ftp)][fsq] ^ zob_piece[Pc(sd, ftp)][tsq];
  p->cl_bb[sd] ^= SqBb(fsq) | SqBb(tsq);
  p->tp_bb[ftp] ^= SqBb(fsq) | SqBb(tsq);
  p->mg_sc[sd] += Par.mg_pst[sd][ftp][tsq] - Par.mg_pst[sd][ftp][fsq];
  p->eg_sc[sd] += Par.eg_pst[sd][ftp][tsq] - Par.eg_pst[sd][ftp][fsq];

  if (ftp == K)
    p->king_sq[sd] = tsq;

  if (ttp != NO_TP) {
    p->hash_key ^= zob_piece[Pc(op, ttp)][tsq];
	if (ttp == P)
		p->pawn_key ^= zob_piece[Pc(op, ttp)][tsq];
    p->cl_bb[op] ^= SqBb(tsq);
    p->tp_bb[ttp] ^= SqBb(tsq);
    p->mg_sc[op] -= Par.mg_pst[op][ttp][tsq];
	p->eg_sc[op] -= Par.eg_pst[op][ttp][tsq];
	p->phase -= ph_value[ttp];
	p->cnt[op][ttp]--;
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
    p->pc[tsq] = Pc(sd, R);
    p->hash_key ^= zob_piece[Pc(sd, R)][fsq] ^ zob_piece[Pc(sd, R)][tsq];
    p->cl_bb[sd] ^= SqBb(fsq) | SqBb(tsq);
    p->tp_bb[R] ^= SqBb(fsq) | SqBb(tsq);
    p->mg_sc[sd] += Par.mg_pst[sd][R][tsq] - Par.mg_pst[sd][R][fsq];
	p->eg_sc[sd] += Par.eg_pst[sd][R][tsq] - Par.eg_pst[sd][R][fsq];
    break;

  case EP_CAP:
    tsq ^= 8;
    p->pc[tsq] = NO_PC;
    p->hash_key ^= zob_piece[Pc(op, P)][tsq];
	p->pawn_key ^= zob_piece[Pc(op, P)][tsq];
    p->cl_bb[op] ^= SqBb(tsq);
    p->tp_bb[P] ^= SqBb(tsq);
    p->mg_sc[op] -= Par.mg_pst[op][P][tsq];
	p->eg_sc[op] -= Par.eg_pst[op][P][tsq];
	p->phase -= ph_value[P];
	p->cnt[op][P]--;
    break;

  case EP_SET:
    tsq ^= 8;
    if (BB.PawnAttacks(sd, tsq) & p->Pawns(op)) {
      p->ep_sq = tsq;
      p->hash_key ^= zob_ep[File(tsq)];
    }
    break;

  case N_PROM: case B_PROM: case R_PROM: case Q_PROM:
    ftp = PromType(move);
    p->pc[tsq] = Pc(sd, ftp);
    p->hash_key ^= zob_piece[Pc(sd, P)][tsq] ^ zob_piece[Pc(sd, ftp)][tsq];
	p->pawn_key ^= zob_piece[Pc(sd, P)][tsq];
    p->tp_bb[P] ^= SqBb(tsq);
    p->tp_bb[ftp] ^= SqBb(tsq);
    p->mg_sc[sd] += Par.mg_pst[sd][ftp][tsq] - Par.mg_pst[sd][P][tsq];
	p->eg_sc[sd] += Par.eg_pst[sd][ftp][tsq] - Par.eg_pst[sd][P][tsq];
	p->phase += ph_value[ftp] - ph_value[P];
	p->cnt[sd][P]--;
	p->cnt[sd][ftp]++;
    break;
  }
  p->side ^= 1;
  p->hash_key ^= SIDE_RANDOM;
}

void DoNull(POS *p, UNDO *u) {

  u->ep_sq = p->ep_sq;
  u->hash_key = p->hash_key;
  p->rep_list[p->head++] = p->hash_key;
  p->rev_moves++;
  if (p->ep_sq != NO_SQ) {
    p->hash_key ^= zob_ep[File(p->ep_sq)];
    p->ep_sq = NO_SQ;
  }
  p->side ^= 1;
  p->hash_key ^= SIDE_RANDOM;
}
