#include "rodent.h"

int Mobility(POS *p, int side)
{
  U64 pieces;
  int from, mob;

  mob = 0;
  pieces = PcBb(p, side, B);
  while (pieces) {
    from = FirstOne(pieces);
    mob += PopCnt(BAttacks(OccBb(p), from)) * 4;
    pieces &= pieces - 1;
  }
  pieces = PcBb(p, side, R);
  while (pieces) {
    from = FirstOne(pieces);
    mob += PopCnt(RAttacks(OccBb(p), from)) * 2;
    pieces &= pieces - 1;
  }
  pieces = PcBb(p, side, Q);
  while (pieces) {
    from = FirstOne(pieces);
    mob += PopCnt(QAttacks(OccBb(p), from));
    pieces &= pieces - 1;
  }
  return mob;
}

int EvaluatePawns(POS *p, int side)
{
  U64 pieces;
  int from, score;

  score = 0;
  pieces = PcBb(p, side, P);
  while (pieces) {
    from = FirstOne(pieces);
    if (!(passed_mask[side][from] & PcBb(p, Opp(side), P)))
      score += passed_bonus[side][Rank(from)];
    if (!(adjacent_mask[File(from)] & PcBb(p, side, P)))
      score -= 20;
    pieces &= pieces - 1;
  }
  return score;
}

int EvaluateKing(POS *p, int side)
{
  if (!PcBb(p, Opp(side), Q) || p->mat[Opp(side)] <= 1600)
    return 0;
  return -2 * pst[K][KingSq(p, side)];
}

int Evaluate(POS *p)
{
  int score;

  score = p->mat[WC] - p->mat[BC];
  if (score > -200 && score < 200)
    score += Mobility(p, WC) - Mobility(p, BC);
  score += p->pst[WC] - p->pst[BC];
  score += EvaluatePawns(p, WC) - EvaluatePawns(p, BC);
  score += EvaluateKing(p, WC) - EvaluateKing(p, BC);
  if (score < -MAX_EVAL)
    score = -MAX_EVAL;
  else if (score > MAX_EVAL)
    score = MAX_EVAL;
  return p->side == WC ? score : -score;
}
