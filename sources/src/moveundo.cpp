/*
Rodent, a UCI chess playing engine derived from Sungorus 1.4
Copyright (C) 2009-2011 Pablo Vazquez (Sungorus author)
Copyright (C) 2011-2017 Pawel Koziol

Rodent is free software: you can redistribute it and/or modify it under the terms of the GNU
General Public License as published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

Rodent is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along with this program.
If not, see <http://www.gnu.org/licenses/>.
*/

#include "rodent.h"

void POS::UndoMove(int move, UNDO *u) {

  int sd = Opp(side);
  int op = Opp(sd);
  int fsq = Fsq(move);
  int tsq = Tsq(move);
  int ftp = Tp(pc[tsq]); // moving piece
  int ttp = u->ttp;

  c_flags = u->c_flags;
  ep_sq = u->ep_sq;
  rev_moves = u->rev_moves;
  hash_key = u->hash_key;
  pawn_key = u->pawn_key;

  head--;

  pc[fsq] = Pc(sd, ftp);
  pc[tsq] = NO_PC;
  cl_bb[sd] ^= SqBb(fsq) | SqBb(tsq);
  tp_bb[ftp] ^= SqBb(fsq) | SqBb(tsq);
  mg_sc[sd] += Par.mg_pst[sd][ftp][fsq] - Par.mg_pst[sd][ftp][tsq];
  eg_sc[sd] += Par.eg_pst[sd][ftp][fsq] - Par.eg_pst[sd][ftp][tsq];

  // Change king location

  if (ftp == K) king_sq[sd] = fsq;

  // Uncapture enemy piece

  if (ttp != NO_TP) {
    pc[tsq] = Pc(op, ttp);
    cl_bb[op] ^= SqBb(tsq);
    tp_bb[ttp] ^= SqBb(tsq);
    mg_sc[op] += Par.mg_pst[op][ttp][tsq];
	eg_sc[op] += Par.eg_pst[op][ttp][tsq];
	phase += ph_value[ttp];
	cnt[op][ttp]++;
  }

  switch (MoveType(move)) {

  case NORMAL:
    break;

  case CASTLE:

    // define complementary rook move

    switch (tsq) {
      case C1: { fsq = A1; tsq = D1; break; }
      case G1: { fsq = H1; tsq = F1; break; }
      case C8: { fsq = A8; tsq = D8; break; }
      case G8: { fsq = H8; tsq = F8; break; }
    }

    pc[tsq] = NO_PC;
    pc[fsq] = Pc(sd, R);
    cl_bb[sd] ^= SqBb(fsq) | SqBb(tsq);
    tp_bb[R] ^= SqBb(fsq) | SqBb(tsq);
    mg_sc[sd] += Par.mg_pst[sd][R][fsq] - Par.mg_pst[sd][R][tsq];
	eg_sc[sd] += Par.eg_pst[sd][R][fsq] - Par.eg_pst[sd][R][tsq];
    break;

  case EP_CAP:
    tsq ^= 8;
    pc[tsq] = Pc(op, P);
    cl_bb[op] ^= SqBb(tsq);
    tp_bb[P] ^= SqBb(tsq);
    mg_sc[op] += Par.mg_pst[op][P][tsq];
	eg_sc[op] += Par.eg_pst[op][P][tsq];
	phase += ph_value[P];
	cnt[op][P]++;
    break;

  case EP_SET:
    break;

  case N_PROM: case B_PROM: case R_PROM: case Q_PROM:
    pc[fsq] = Pc(sd, P);
    tp_bb[P] ^= SqBb(fsq);
    tp_bb[ftp] ^= SqBb(fsq);
    mg_sc[sd] += Par.mg_pst[sd][P][fsq] - Par.mg_pst[sd][ftp][fsq];
	eg_sc[sd] += Par.eg_pst[sd][P][fsq] - Par.eg_pst[sd][ftp][fsq];
	phase += ph_value[P] - ph_value[ftp];
	cnt[sd][P]++;
	cnt[sd][ftp]--;
    break;
  }

  side ^= 1;
}

void POS::UndoNull(UNDO *u) {

  ep_sq = u->ep_sq;
  hash_key = u->hash_key;
  head--;
  rev_moves--;
  side ^= 1;
}
