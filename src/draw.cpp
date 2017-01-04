#include "rodent.h"

int cEngine::GetDrawFactor(POS * p, int sd) {

  int op = Opp(sd);

  if (p->phase < 2 && p->cnt[sd][P] == 0) return 0;                                                  // KK, KmK, KmKp(p)

  if (p->phase == 2) {
    if (p->cnt[sd][N] == 2 && p->cnt[sd][P] == 0) {
      if (p->cnt[op][P] == 0) return 0;                                                              // KNNK(m)
      else return 8;                                                                                 // KNNK(m)(p)
    }
  }

  if (p->phase == 3 && p->cnt[sd][P] == 0) {
    if (p->cnt[sd][R] == 1 && p->cnt[op][B] + p->cnt[op][N] == 1) return 16;                         // KRKm(p)
    if (p->cnt[sd][B] + p->cnt[sd][N] == 2 && p->cnt[op][B] == 1) return 8;                          // KmmKB(p)
    if (p->cnt[sd][B] == 1 && p->cnt[sd][N] == 1 && p->cnt[op][B] + p->cnt[op][N] == 1) return 8;    // KBNKm(p)
  }

  if (p->phase == 5 && p->cnt[sd][P] == 0) {
    if (p->cnt[sd][R] == 1 && p->cnt[sd][B] + p->cnt[sd][N] == 1 && p->cnt[op][R] == 1) return 16;   // KRMKR(p)
  }

  return 64;
}