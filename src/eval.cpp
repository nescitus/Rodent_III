#include "rodent.h"
#include "eval.h"

static const int passed_bonus_mg[2][8] = {
  { 0, 12, 12, 30, 50, 80, 130, 0 },
  { 0, 120, 80, 50, 30, 12, 12, 0 }
};

const int passed_bonus_eg[2][8] = {
  { 0, 16, 16, 39, 65, 104, 156, 0 },
  { 0, 156, 104, 65, 39, 16, 16, 0 }
};

static const int att_weight[16] = {
	0, 0, 128, 192, 224, 240, 248, 252, 254, 255, 256, 256, 256, 256, 256, 256,
};

#define REL_SQ(sq,cl)   ( sq ^ (cl * 56) )

static const U64 bbQSCastle[2] = { SqBb(A1) | SqBb(B1) | SqBb(C1) | SqBb(A2) | SqBb(B2) | SqBb(C2),
                                   SqBb(A8) | SqBb(B8) | SqBb(C8) | SqBb(A7) | SqBb(B7) | SqBb(C7)
                                 };
static const U64 bbKSCastle[2] = { SqBb(F1) | SqBb(G1) | SqBb(H1) | SqBb(F2) | SqBb(G2) | SqBb(H2),
                                   SqBb(F8) | SqBb(G8) | SqBb(H8) | SqBb(F7) | SqBb(G7) | SqBb(H7)
                                 };

static const U64 bbCentralFile = FILE_C_BB | FILE_D_BB | FILE_E_BB | FILE_F_BB;

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

  // Init support mask (for detecting weak pawns)

  for (int sq = 0; sq < 64; sq++) {
    support_mask[WC][sq] = ShiftWest(SqBb(sq)) | ShiftEast(SqBb(sq));
    support_mask[WC][sq] |= FillSouth(support_mask[WC][sq]);

    support_mask[BC][sq] = ShiftWest(SqBb(sq)) | ShiftEast(SqBb(sq));
    support_mask[BC][sq] |= FillNorth(support_mask[BC][sq]);
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

void ScorePieces(POS *p, int sd) {

  U64 pieces, bb_attacks, bb_control;
  int op, sq, ksq, cnt;
  int att = 0;
  int wood = 0;
  mg_sc[sd] = 0;
  eg_sc[sd] = 0;

  // Init variables

  op = Opp(sd);
  ksq = KingSq(p, op);

  // Init enemy king zone for attack evaluation. We mark squares where the king
  // can move plus two or three more squares facing enemy position.

  U64 bb_zone = k_attacks[ksq];
  (sd == WC) ? bb_zone |= ShiftSouth(bb_zone) : bb_zone |= ShiftNorth(bb_zone);

  // KNIGHT EVAL

  pieces = PcBb(p, sd, N);
  while (pieces) {
    sq = PopFirstBit(&pieces);

    // knight king attack score

    bb_control = n_attacks[sq] & ~p->cl_bb[sd];
    if (bb_control & bb_zone) {
      wood++;
      att += 1;
    }

    // knight mobility score

    cnt = PopCnt(bb_control);
    mg_sc[sd] += 4 * (cnt - 4);
    eg_sc[sd] += 4 * (cnt - 4);

    // knight piece/square score

    mg_sc[sd] += Par.mg_pst[sd][N][sq];
    eg_sc[sd] += Par.eg_pst[sd][N][sq];
    phase += 1;
  }

  // BISHOP EVAL

  pieces = PcBb(p, sd, B);
  while (pieces) {
    sq = PopFirstBit(&pieces);

    // bishop king attack score

    bb_attacks = BAttacks(OccBb(p) ^ PcBb(p, sd, Q), sq);
    if (bb_attacks & bb_zone) {
      wood++;
      att += 1;
    }

    // bishop mobility score

    cnt = PopCnt(BAttacks(OccBb(p), sq));
    mg_sc[sd] += 5 * (cnt - 7);
    eg_sc[sd] += 5 * (cnt - 7);

    // bishop piece/square score

    mg_sc[sd] += Par.mg_pst[sd][B][sq];
    eg_sc[sd] += Par.eg_pst[sd][B][sq];
    phase += 1;
  }

  // ROOK EVAL

  pieces = PcBb(p, sd, R);
  while (pieces) {
    sq = PopFirstBit(&pieces);

   // rook king attack score

    bb_attacks = RAttacks(OccBb(p) ^ PcBb(p, sd, Q) ^ PcBb(p, sd, R), sq);
    if (bb_attacks & bb_zone) {
      wood++;
      att += 2;
    }

    // rook mobility score

    cnt = PopCnt(RAttacks(OccBb(p), sq));
    mg_sc[sd] += 2 * (cnt - 7);
    eg_sc[sd] += 4 * (cnt - 7);

    // rook piece/square score

    mg_sc[sd] += Par.mg_pst[sd][R][sq];
    eg_sc[sd] += Par.eg_pst[sd][R][sq];
    phase += 2;
  }

  // QUEEN EVAL
  
  pieces = PcBb(p, sd, Q);
  while (pieces) {
    sq = PopFirstBit(&pieces);

    // queen king attack score

    bb_attacks  = BAttacks(OccBb(p) ^ PcBb(p, sd, B) ^ PcBb(p, sd, Q), sq);
    bb_attacks |= RAttacks(OccBb(p) ^ PcBb(p, sd, R) ^ PcBb(p, sd, Q), sq);
    if (bb_attacks & bb_zone) {
      wood++;
      att += 4;
    }

    // queen mobility score

    cnt = PopCnt(QAttacks(OccBb(p), sq));
    mg_sc[sd] += 1 * (cnt - 14);
    eg_sc[sd] += 2 * (cnt - 14);

    // queen piece/square score

    mg_sc[sd] += Par.mg_pst[sd][Q][sq];
    eg_sc[sd] += Par.eg_pst[sd][Q][sq];
    phase += 4;
  }

  // king attack - 

  if (PcBb(p, sd, Q)) {
    int att_score = (att * 20 * att_weight[wood]) / 256;
    mg_sc[sd] += att_score;
    eg_sc[sd] += att_score;
  }

}

void ScorePawns(POS *p, int sd) {

  U64 pieces;
  int sq;

  pieces = PcBb(p, sd, P);
  while (pieces) {
    sq = PopFirstBit(&pieces);

	// PIECE SQUARE TABLE

	mg_sc[sd] += Par.mg_pst[sd][P][sq];
	eg_sc[sd] += Par.eg_pst[sd][P][sq];

    // PASSED PAWNS

    if (!(passed_mask[sd][sq] & PcBb(p, Opp(sd), P))) {
      mg_sc[sd] += passed_bonus_mg[sd][Rank(sq)];
      eg_sc[sd] += passed_bonus_eg[sd][Rank(sq)];
    }
    
    // ISOLATED PAWNS

    if (!(adjacent_mask[File(sq)] & PcBb(p, sd, P))) {
      mg_sc[sd] -= 20;
      eg_sc[sd] -= 20;
    }

	// WEAK PAWNS

    else if ((support_mask[sd][sq] & PcBb(p, sd, P)) == 0) {
      mg_sc[sd] -= 8;
      eg_sc[sd] -= 8;
    }
  }
}

void ScoreKing(POS *p, int sd) {

  const int startSq[2] = { E1, E8 };
  const int qCastle[2] = { B1, B8 };
  const int kCastle[2] = { G1, G8 };

  int sq = KingSq(p, sd);
  mg_sc[sd] += Par.mg_pst[sd][K][sq];
  eg_sc[sd] += Par.eg_pst[sd][K][sq];

  // Normalize king square for pawn shield evaluation,
  // to discourage shuffling the king between g1 and h1.

  if (SqBb(sq) & bbKSCastle[sd]) sq = kCastle[sd];
  if (SqBb(sq) & bbQSCastle[sd]) sq = qCastle[sd];

  // Evaluate shielding and storming pawns on each file.

  U64 bbKingFile = FillNorth(SqBb(sq)) | FillSouth(SqBb(sq));
  int result = EvalKingFile(p, sd, bbKingFile);

  U64 bbNextFile = ShiftEast(bbKingFile);
  if (bbNextFile) result += EvalKingFile(p, sd, bbNextFile);

  bbNextFile = ShiftWest(bbKingFile);
  if (bbNextFile) result += EvalKingFile(p, sd, bbNextFile);

  mg_sc[sd] += result;
}

int EvalKingFile(POS * p, int sd, U64 bbFile) {

  int shelter = EvalFileShelter(bbFile & PcBb(p, sd, P), sd);
  int storm   = EvalFileStorm  (bbFile & PcBb(p, Opp(sd), P), sd);
  if (bbFile & bbCentralFile) return (shelter / 2) + storm;
  else return shelter + storm;
}

int EvalFileShelter(U64 bbOwnPawns, int sd) {

  if (!bbOwnPawns) return -36;
  if (bbOwnPawns & bbRelRank[sd][RANK_2]) return    2;
  if (bbOwnPawns & bbRelRank[sd][RANK_3]) return  -11;
  if (bbOwnPawns & bbRelRank[sd][RANK_4]) return  -20;
  if (bbOwnPawns & bbRelRank[sd][RANK_5]) return  -27;
  if (bbOwnPawns & bbRelRank[sd][RANK_6]) return  -32;
  if (bbOwnPawns & bbRelRank[sd][RANK_7]) return  -35;
  return 0;
}

int EvalFileStorm(U64 bbOppPawns, int sd) {

  if (!bbOppPawns) return -16;
  if (bbOppPawns & bbRelRank[sd][RANK_3]) return -32;
  if (bbOppPawns & bbRelRank[sd][RANK_4]) return -16;
  if (bbOppPawns & bbRelRank[sd][RANK_5]) return -8;
  return 0;
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

  // cleanup
  phase = 0;

  ScorePieces(p, WC); 
  ScorePieces(p, BC);
  ScorePawns(p, WC); 
  ScorePawns(p, BC);
  ScoreKing(p, WC); 
  ScoreKing(p, BC);

  return Interpolate(p);
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
