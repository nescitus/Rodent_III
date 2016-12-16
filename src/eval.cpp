#include "rodent.h"
#include "eval.h"

#define REL_SQ(sq,cl)   ( sq ^ (cl * 56) )

int mg_sc[2];
int eg_sc[2];
int phase;

void cParam::Init(void) {

  for (int sq = 0; sq < 64; sq++) {
    for (int sd = 0; sd < 2; sd++) {

      mg_pst[sd][P][REL_SQ(sq, sd)] = tp_value[P] + pstPawnMg[sq];
      eg_pst[sd][P][REL_SQ(sq, sd)] = tp_value[P] + pstPawnEg[sq];
      mg_pst[sd][N][REL_SQ(sq, sd)] = tp_value[N] + pstKnightMg[sq];
      eg_pst[sd][N][REL_SQ(sq, sd)] = tp_value[N] + pstKnightEg[sq];
      mg_pst[sd][B][REL_SQ(sq, sd)] = tp_value[B] + pstBishopMg[sq];
      eg_pst[sd][B][REL_SQ(sq, sd)] = tp_value[B] + pstBishopEg[sq];
      mg_pst[sd][R][REL_SQ(sq, sd)] = tp_value[R] + pstRookMg[sq];
      eg_pst[sd][R][REL_SQ(sq, sd)] = tp_value[R] + pstRookEg[sq];
      mg_pst[sd][Q][REL_SQ(sq, sd)] = tp_value[Q] + pstQueenMg[sq];
      eg_pst[sd][Q][REL_SQ(sq, sd)] = tp_value[Q] + pstQueenEg[sq];
      mg_pst[sd][K][REL_SQ(sq, sd)] = pstKingMg[sq];
      eg_pst[sd][K][REL_SQ(sq, sd)] = pstKingEg[sq];

    }
  }
}

void ScorePst(POS * p, int sd) {
   
  U64 pieces;
  int sq;

  mg_sc[sd] = 0;
  eg_sc[sd] = 0;

  pieces = PcBb(p, sd, P);
  while (pieces) {
    sq = PopFirstBit(&pieces);
	mg_sc[sd] += Par.mg_pst[sd][P][sq];
	eg_sc[sd] += Par.eg_pst[sd][P][sq];
  }

  pieces = PcBb(p, sd, N);
  while (pieces) {
    sq = PopFirstBit(&pieces);
	mg_sc[sd] += Par.mg_pst[sd][N][sq];
	eg_sc[sd] += Par.eg_pst[sd][N][sq];
	phase += 1;
  }

  pieces = PcBb(p, sd, B);
  while (pieces) {
    sq = PopFirstBit(&pieces);
	mg_sc[sd] += Par.mg_pst[sd][B][sq];
	eg_sc[sd] += Par.eg_pst[sd][B][sq];
	phase += 1;
  }

  pieces = PcBb(p, sd, R);
  while (pieces) {
    sq = PopFirstBit(&pieces);
	mg_sc[sd] += Par.mg_pst[sd][R][sq];
	eg_sc[sd] += Par.eg_pst[sd][R][sq];
	phase += 2;
  }

  pieces = PcBb(p, sd, Q);
  while (pieces) {
    sq = PopFirstBit(&pieces);
	mg_sc[sd] += Par.mg_pst[sd][Q][sq];
	eg_sc[sd] += Par.eg_pst[sd][Q][sq];
	phase += 4;
  }

  pieces = PcBb(p, sd, K);
  while (pieces) {
    sq = PopFirstBit(&pieces);
	mg_sc[sd] += Par.mg_pst[sd][K][sq];
	eg_sc[sd] += Par.eg_pst[sd][K][sq];
  }
}

int ScorePieces(POS *p, int sd) {

  U64 pieces, control;
  int sq, mob, cnt;
  mg_sc[sd] = 0;
  eg_sc[sd] = 0;

  mob = 0;

 pieces = PcBb(p, sd, P);
  while (pieces) {
    sq = PopFirstBit(&pieces);
	mg_sc[sd] += Par.mg_pst[sd][P][sq];
	eg_sc[sd] += Par.eg_pst[sd][P][sq];
  }

  pieces = PcBb(p, sd, N);
  while (pieces) {
    sq = PopFirstBit(&pieces);
	control = n_attacks[sq] & ~p->cl_bb[sd];
	cnt = PopCnt(control);
	mg_sc[sd] += 4 * (cnt - 4);
	eg_sc[sd] += 4 * (cnt - 4);
	mg_sc[sd] += Par.mg_pst[sd][N][sq];
	eg_sc[sd] += Par.eg_pst[sd][N][sq];
	phase += 1;
  }

  pieces = PcBb(p, sd, B);
  while (pieces) {
    sq = PopFirstBit(&pieces);
	cnt = PopCnt(BAttacks(OccBb(p), sq));
	mg_sc[sd] += 5 * (cnt - 7);
	eg_sc[sd] += 5 * (cnt - 7);
	mg_sc[sd] += Par.mg_pst[sd][B][sq];
	eg_sc[sd] += Par.eg_pst[sd][B][sq];
	phase += 1;
  }

  pieces = PcBb(p, sd, R);
  while (pieces) {
    sq = PopFirstBit(&pieces);
	cnt = PopCnt(RAttacks(OccBb(p), sq));
	mg_sc[sd] += 2 * (cnt - 7);
	eg_sc[sd] += 4 * (cnt - 7);
	mg_sc[sd] += Par.mg_pst[sd][R][sq];
	eg_sc[sd] += Par.eg_pst[sd][R][sq];
	phase += 2;
  }

  pieces = PcBb(p, sd, Q);
  while (pieces) {
    sq = PopFirstBit(&pieces);
	cnt = PopCnt(QAttacks(OccBb(p), sq));
	mg_sc[sd] += 1 * (cnt - 14);
	eg_sc[sd] += 2 * (cnt - 14);
	mg_sc[sd] += Par.mg_pst[sd][Q][sq];
	eg_sc[sd] += Par.eg_pst[sd][Q][sq];
	phase += 4;
  }

  pieces = PcBb(p, sd, K);
  while (pieces) {
	  sq = PopFirstBit(&pieces);
	  mg_sc[sd] += Par.mg_pst[sd][K][sq];
	  eg_sc[sd] += Par.eg_pst[sd][K][sq];
  }

  return mob;
}

int ScorePawns(POS *p, int sd) {

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

int ScoreKing(POS *p, int sd) {

  if (!PcBb(p, Opp(sd), Q) || p->mat[Opp(sd)] <= 1600)
    return 0;
  return -2 * pst[K][KingSq(p, sd)];
}

int ScoreLikeIdiot(POS * p) {

  int score = p->mat[WC] - p->mat[BC];
  int randomMod = (80 / 2) - (p->key % 80);
  score += randomMod;
  return score;
}

int Interpolate(POS * p) {

  int mg_tot = mg_sc[WC] - mg_sc[BC];
  int eg_tot = eg_sc[WC] - eg_sc[BC];
  int mg_phase = Min(phase, 24);
  int eg_phase = 24 - mg_phase;

  return (mg_tot * mg_phase + eg_tot * eg_phase) / 24;
}

int ScoreLikeUfo(POS * p) {

  phase = 0;
  ScorePst(p, WC);
  ScorePst(p, BC);
  return Interpolate(p);
}

int ScoreLikeRodent(POS * p) {

  int score = 0;
  phase = 0;

  score += ScorePieces(p, WC) - ScorePieces(p, BC);
  score += ScorePawns(p, WC) - ScorePawns(p, BC);
  //score += ScoreKing(p, WC) - ScoreKing(p, BC);

  score += Interpolate(p);

  return score;
}
int Evaluate(POS *p) {

  int score = ScoreLikeRodent(p);

  // Make sure eval does not exceed mate score

  if (score < -MAX_EVAL)
    score = -MAX_EVAL;
  else if (score > MAX_EVAL)
    score = MAX_EVAL;

  // Return score relative to the side to move

  return p->side == WC ? score : -score;
}
