#include "rodent.h"

void SetPosition(POS *p, char *epd) {

  int i, j, pc;
  static const char pc_char[13] = "PpNnBbRrQqKk";

  for (int i = 0; i < 2; i++) {
    p->cl_bb[i] = 0;
    p->phase = 0;
    p->mg_sc[i] = 0;
	p->eg_sc[i] = 0;
  }

  for (int i = 0; i < 6; i++)
    p->tp_bb[i] = 0;

  p->c_flags = 0;
  p->rev_moves = 0;
  p->head = 0;

  for (int i = 56; i >= 0; i -= 8) {
    j = 0;
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
        p->phase += ph_value[Tp(pc)];
		p->mg_sc[Cl(pc)] += Par.mg_pst[Cl(pc)][Tp(pc)][i + j];
		p->eg_sc[Cl(pc)] += Par.eg_pst[Cl(pc)][Tp(pc)][i + j];
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
    if (!(p_attacks[Opp(p->side)][p->ep_sq] & PcBb(p, p->side, P)))
      p->ep_sq = NO_SQ;
  }
  p->key = Key(p);
}
