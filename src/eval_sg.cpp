#include "rodent.h"
#include "eval.h"

int ScorePiecesSg(POS *p, int sd) {

  U64 pieces;
  int sq, mob, cnt;

  mob = 0;
  pieces = PcBb(p, sd, B);
  while (pieces) {
    sq = PopFirstBit(&pieces);
	cnt = PopCnt(BAttacks(OccBb(p), sq));
    mob += 4 * cnt;
  }

  pieces = PcBb(p, sd, R);
  while (pieces) {
    sq = PopFirstBit(&pieces);
	cnt = PopCnt(RAttacks(OccBb(p), sq));
    mob += 2 * cnt;
  }

  pieces = PcBb(p, sd, Q);
  while (pieces) {
    sq = PopFirstBit(&pieces);
	cnt = PopCnt(QAttacks(OccBb(p), sq));
    mob += 1 * cnt;
  }

  return mob;
}

int ScorePawnsSg(POS *p, int sd) {

  U64 pieces;
  int from, score;

  score = 0;
  pieces = PcBb(p, sd, P);
  while (pieces) {
    from = PopFirstBit(&pieces);

    if (!(passed_mask[sd][from] & PcBb(p, Opp(sd), P)))
      score += passed_bonus[sd][Rank(from)];

    if (!(adjacent_mask[File(from)] & PcBb(p, sd, P)))
      score -= 20;
  }
  return score;
}

int ScoreKingSg(POS *p, int sd) {

  if (!PcBb(p, Opp(sd), Q) || p->mat[Opp(sd)] <= 1600)
    return 0;
  return -2 * pst[K][KingSq(p, sd)];
}

int ScoreLikeSungorus(POS * p) {

  int score = p->mat[WC] - p->mat[BC];

  if (score > -200 && score < 200)
    score += ScorePiecesSg(p, WC) - ScorePiecesSg(p, BC);

  score += p->pst[WC] - p->pst[BC];
  score += ScorePawnsSg(p, WC) - ScorePawnsSg(p, BC);
  score += ScoreKingSg(p, WC) - ScoreKingSg(p, BC);

  return score;
}