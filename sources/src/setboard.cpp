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

void ClearPosition(POS *p) {

  for (int sd = 0; sd < 2; sd++) {
    p->cl_bb[sd] = 0;
    p->mg_sc[sd] = 0;
	p->eg_sc[sd] = 0;
  }

  for (int pc = 0; pc < 6; pc++) {
    p->tp_bb[pc] = 0ULL;
    p->cnt[WC][pc] = 0;
    p->cnt[BC][pc] = 0;
  }

  p->c_flags = 0;
  p->rev_moves = 0;
  p->head = 0;
  p->phase = 0;
}

void SetPosition(POS *p, char *epd) {

  int pc;
  static const char pc_char[13] = "PpNnBbRrQqKk";

  ClearPosition(p);

  for (int i = 56; i >= 0; i -= 8) {
    int j = 0;
    while (j < 8) {
      if (*epd >= '1' && *epd <= '8')
        for (pc = 0; pc < *epd - '0'; pc++) {
          p->pc[i + j] = NO_PC;
          j++;
        }
      else {
        for (pc = 0; pc_char[pc] != *epd; pc++)
          ;
        p->pc[i + j] = pc;
        p->cl_bb[Cl(pc)] ^= SqBb(i + j);
        p->tp_bb[Tp(pc)] ^= SqBb(i + j);

        if (Tp(pc) == K)
          p->king_sq[Cl(pc)] = i + j;

        p->mg_sc[Cl(pc)] += Par.mg_pst[Cl(pc)][Tp(pc)][i + j];
		p->eg_sc[Cl(pc)] += Par.eg_pst[Cl(pc)][Tp(pc)][i + j];
		p->phase += ph_value[Tp(pc)];
		p->cnt[Cl(pc)][Tp(pc)]++;
        j++;
      }
      epd++;
    }
    epd++;
  }
  if (*epd++ == 'w')
    p->side = WC;
  else
    p->side = BC;
  epd++;
  if (*epd == '-')
    epd++;
  else {
    if (*epd == 'K') {
      p->c_flags |= 1;
      epd++;
    }
    if (*epd == 'Q') {
      p->c_flags |= 2;
      epd++;
    }
    if (*epd == 'k') {
      p->c_flags |= 4;
      epd++;
    }
    if (*epd == 'q') {
      p->c_flags |= 8;
      epd++;
    }
  }
  epd++;
  if (*epd == '-')
    p->ep_sq = NO_SQ;
  else {
    p->ep_sq = Sq(*epd - 'a', *(epd + 1) - '1');
    if (!(BB.PawnAttacks(Opp(p->side),p->ep_sq) & p->Pawns(p->side)))
      p->ep_sq = NO_SQ;
  }
  p->hash_key = InitHashKey(p);
  p->pawn_key = InitPawnKey(p);
}
