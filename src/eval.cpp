#include "rodent.h"
#include "eval.h"

static const int passed_bonus_mg[2][8] = {
  { 0, 12, 12, 30, 50, 80, 130, 0 },
  { 0, 120, 80, 50, 30, 12, 12, 0 }
};

static const int passed_bonus_eg[2][8] = {
  { 0, 16, 16, 39, 65, 104, 156, 0 },
  { 0, 156, 104, 65, 39, 16, 16, 0 }
};

static const U64 bbQSCastle[2] = { SqBb(A1) | SqBb(B1) | SqBb(C1) | SqBb(A2) | SqBb(B2) | SqBb(C2),
                                   SqBb(A8) | SqBb(B8) | SqBb(C8) | SqBb(A7) | SqBb(B7) | SqBb(C7)
                                 };
static const U64 bbKSCastle[2] = { SqBb(F1) | SqBb(G1) | SqBb(H1) | SqBb(F2) | SqBb(G2) | SqBb(H2),
                                   SqBb(F8) | SqBb(G8) | SqBb(H8) | SqBb(F7) | SqBb(G7) | SqBb(H7)
                                 };

static const U64 bbCentralFile = FILE_C_BB | FILE_D_BB | FILE_E_BB | FILE_F_BB;

void cEngine::ScoreMaterial(POS *p, eData *e, int sd) {

  int tmp = Par.np_table[p->cnt[sd][P]] * p->cnt[sd][N]   // knights lose value as pawns disappear
	      - Par.rp_table[p->cnt[sd][P]] * p->cnt[sd][R];  // rooks gain value as pawns disappear

  if (p->cnt[sd][B] == 2) tmp += 50;                      // bishop pair
  if (p->cnt[sd][N] == 2) tmp -= 10;                      // knight pair
  if (p->cnt[sd][R] == 2) tmp -= 5;                       // rook pair

  tmp = ((tmp * Par.mat_weight) / 100);

  Add(e, sd, tmp, tmp);

}

void cEngine::ScorePieces(POS *p, eData *e, int sd) {

  U64 bb_pieces, bb_attacks, bb_control;
  int op, sq, ksq, cnt, own_pawn_cnt, opp_pawn_cnt;
  int tropism_mg = 0;
  int tropism_eg = 0;
  int mob_mg = 0;
  int mob_eg = 0;
  int att = 0;

  // Init score with data from board class

  e->mg_sc[sd] = p->mg_sc[sd];
  e->eg_sc[sd] = p->eg_sc[sd];

  // Init variables

  op = Opp(sd);
  ksq = KingSq(p, op);
  U64 n_checks = n_attacks[ksq] & ~p->cl_bb[sd] & ~e->pawn_takes[op];
  U64 b_checks = BAttacks(OccBb(p), ksq) & ~p->cl_bb[sd] & ~e->pawn_takes[op];
  U64 r_checks = RAttacks(OccBb(p), ksq) & ~p->cl_bb[sd] & ~e->pawn_takes[op];
  U64 q_checks = r_checks & b_checks;

  // Init enemy king zone for attack evaluation. We mark squares where the king
  // can move plus two or three more squares facing enemy position.

  U64 bb_zone = k_attacks[ksq];
  (sd == WC) ? bb_zone |= ShiftSouth(bb_zone) : bb_zone |= ShiftNorth(bb_zone);

  // KNIGHT EVAL

  bb_pieces = PcBb(p, sd, N);
  while (bb_pieces) {
    sq = PopFirstBit(&bb_pieces);

    // knight king attack score

    bb_control = n_attacks[sq] & ~p->cl_bb[sd];
	att += 6 * PopCnt(bb_control & bb_zone);
	if (bb_control & ~p->cl_bb[sd] & n_checks) att += 4;  // check threats

    // knight mobility score

    cnt = PopCnt(bb_control &~e->pawn_takes[op]);
	mob_mg += 4 * (cnt - 4);
	mob_eg += 4 * (cnt - 4);

	// knight king tropism score

	tropism_mg += 3 * Par.dist[sq][ksq];
	tropism_eg += 3 * Par.dist[sq][ksq];

	// knight outpost

	ScoreOutpost(p, e, sd, N, sq);
  }

  // BISHOP EVAL

  bb_pieces = PcBb(p, sd, B);
  while (bb_pieces) {
    sq = PopFirstBit(&bb_pieces);

    // bishop king attack score

    bb_attacks = BAttacks(OccBb(p) ^ PcBb(p, sd, Q), sq);
	att += 6 * PopCnt(bb_attacks & bb_zone);

    // bishop mobility score

    bb_control = BAttacks(OccBb(p), sq);
	if (bb_control & ~p->cl_bb[sd] & b_checks) att += 4;
    cnt = PopCnt(bb_control);
	mob_mg += 5 * (cnt - 7);
	mob_eg += 5 * (cnt - 7);

    // bishop king tropism score

	tropism_mg += 2 * Par.dist[sq][ksq];
	tropism_eg += 1 * Par.dist[sq][ksq];

	// bishop outpost

	ScoreOutpost(p, e, sd, B, sq);

    // pawns on the same square color as our bishop
  
    if (bbWhiteSq & SqBb(sq)) {
      own_pawn_cnt = PopCnt(bbWhiteSq & PcBb(p, sd, P)) - 4;
      opp_pawn_cnt = PopCnt(bbWhiteSq & PcBb(p, op, P)) - 4;
    } else {
      own_pawn_cnt = PopCnt(bbBlackSq & PcBb(p, sd, P)) - 4;
      opp_pawn_cnt = PopCnt(bbBlackSq & PcBb(p, op, P)) - 4;
    }

	Add(e, sd, -3 * own_pawn_cnt - opp_pawn_cnt);
  }

  // ROOK EVAL

  bb_pieces = PcBb(p, sd, R);
  while (bb_pieces) {
    sq = PopFirstBit(&bb_pieces);

   // rook king attack score

    bb_attacks = RAttacks(OccBb(p) ^ PcBb(p, sd, Q) ^ PcBb(p, sd, R), sq);
	att += 9 * PopCnt(bb_attacks & bb_zone);

    // rook mobility score

    bb_control = RAttacks(OccBb(p), sq);
	if (bb_control & ~p->cl_bb[sd] & r_checks) att += 9;
    cnt = PopCnt(bb_control);
	mob_mg += 2 * (cnt - 7);
	mob_eg += 4 * (cnt - 7);

    // rook on (half) open file

    U64 r_file = FillNorth(SqBb(sq)) | FillSouth(SqBb(sq));
    if (!(r_file & PcBb(p, sd, P))) {
      int fl_on_king = ((r_file && bb_zone) != 0);
      if (!(r_file & PcBb(p, op, P))) Add(e, sd, 12 + 6 * fl_on_king, 12);
      else                            Add(e, sd,  6 + 3 * fl_on_king,  6);
    }

    // rook on 7th rank attacking pawns or cutting off enemy king

    if (SqBb(sq) & bbRelRank[sd][RANK_7]) {
      if (PcBb(p, op, P) & bbRelRank[sd][RANK_7]
      ||  PcBb(p, op, K) & bbRelRank[sd][RANK_8]) {
          Add(e, sd, 16, 32);
      }
    }

	// rook king tropism score

	tropism_mg += 2 * Par.dist[sq][ksq];
	tropism_eg += 1 * Par.dist[sq][ksq];
  }

  // QUEEN EVAL
  
  bb_pieces = PcBb(p, sd, Q);
  while (bb_pieces) {
    sq = PopFirstBit(&bb_pieces);

    // queen king attack score

    bb_attacks  = BAttacks(OccBb(p) ^ PcBb(p, sd, B) ^ PcBb(p, sd, Q), sq);
    bb_attacks |= RAttacks(OccBb(p) ^ PcBb(p, sd, R) ^ PcBb(p, sd, Q), sq);
	att += 15 * PopCnt(bb_attacks & bb_zone);

    // queen mobility score

	bb_control = QAttacks(OccBb(p), sq);
	if (bb_control & ~p->cl_bb[sd] & q_checks) att += 12;
    cnt = PopCnt(bb_control);
	mob_mg += 1 * (cnt - 14);
	mob_eg += 2 * (cnt - 14);

	// queen king tropism score

	tropism_mg += 2 * Par.dist[sq][ksq];
	tropism_eg += 4 * Par.dist[sq][ksq];
  }

  // final calculation of king attack score

  if (PcBb(p, sd, Q)) {
    int att_score = (Par.danger[att] * Par.sd_att[sd]) / 100;
    Add(e, sd, att_score, att_score);
  }

  // final calculation of other factors

  Add(e, sd, (Par.sd_mob[sd] * mob_mg) / 100, (Par.sd_mob[sd] * mob_eg) / 100);
  Add(e, sd, (Par.tropism_weight * tropism_mg) / 100, (Par.tropism_weight * tropism_eg) / 100);

}

void cEngine::ScorePawns(POS *p, eData *e, int sd) {

  U64 bb_pieces, bb_span;
  int op = Opp(sd);
  int sq, fl_unopposed;

  bb_pieces = PcBb(p, sd, P);
  while (bb_pieces) {
    sq = PopFirstBit(&bb_pieces);
    bb_span = GetFrontSpan(SqBb(sq), sd);
    fl_unopposed = ((bb_span & PcBb(p, op, P)) == 0);

    // DOUBLED PAWNS

    if (bb_span & PcBb(p, sd, P))
      Add(e, sd, -10, -20);
    
    // ISOLATED PAWNS

    if (!(adjacent_mask[File(sq)] & PcBb(p, sd, P))) {
      e->mg_sc[sd] -= (10 + 10 * fl_unopposed);
      e->eg_sc[sd] -= 20;
    }

    // WEAK PAWNS

    else if ((support_mask[sd][sq] & PcBb(p, sd, P)) == 0) {
      e->mg_sc[sd] -= (8 + 8 * fl_unopposed);
      e->eg_sc[sd] -= 10;
    }
  }
}

void cEngine::ScorePassers(POS *p, eData *e, int sd) {

  U64 bb_pieces, bb_span;
  int op = Opp(sd);
  int sq, fl_unopposed;

  bb_pieces = PcBb(p, sd, P);
  while (bb_pieces) {
    sq = PopFirstBit(&bb_pieces);

    if (!(passed_mask[sd][sq] & PcBb(p, Opp(sd), P))) {
      e->mg_sc[sd] += passed_bonus_mg[sd][Rank(sq)];
      e->eg_sc[sd] += passed_bonus_eg[sd][Rank(sq)];
    }
  }
}

void cEngine::ScoreKing(POS *p, eData *e, int sd) {

  const int startSq[2] = { E1, E8 };
  const int qCastle[2] = { B1, B8 };
  const int kCastle[2] = { G1, G8 };

  int sq = KingSq(p, sd);

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

  e->mg_sc[sd] += result;
}

int cEngine::EvalKingFile(POS * p, int sd, U64 bbFile) {

  int shelter = EvalFileShelter(bbFile & PcBb(p, sd, P), sd);
  int storm   = EvalFileStorm  (bbFile & PcBb(p, Opp(sd), P), sd);
  if (bbFile & bbCentralFile) return (shelter / 2) + storm;
  else return shelter + storm;
}

int cEngine::EvalFileShelter(U64 bbOwnPawns, int sd) {

  if (!bbOwnPawns) return -36;
  if (bbOwnPawns & bbRelRank[sd][RANK_2]) return    2;
  if (bbOwnPawns & bbRelRank[sd][RANK_3]) return  -11;
  if (bbOwnPawns & bbRelRank[sd][RANK_4]) return  -20;
  if (bbOwnPawns & bbRelRank[sd][RANK_5]) return  -27;
  if (bbOwnPawns & bbRelRank[sd][RANK_6]) return  -32;
  if (bbOwnPawns & bbRelRank[sd][RANK_7]) return  -35;
  return 0;
}

int cEngine::EvalFileStorm(U64 bbOppPawns, int sd) {

  if (!bbOppPawns) return -16;
  if (bbOppPawns & bbRelRank[sd][RANK_3]) return -32;
  if (bbOppPawns & bbRelRank[sd][RANK_4]) return -16;
  if (bbOppPawns & bbRelRank[sd][RANK_5]) return -8;
  return 0;
}

void cEngine::ScoreOutpost(POS * p, eData * e, int sd, int pc, int sq) {

  int mul = 0;
  int tmp = Par.sp_pst[sd][pc][sq];
  if (tmp) {
    if (SqBb(sq) & ~e->pawn_can_take[Opp(sd)]) mul += 2;  // in the hole of enemy pawn structure
    if (SqBb(sq) & e->pawn_takes[sd]) mul += 1;           // defended by own pawn
    if (SqBb(sq) & e->two_pawns_take[sd]) mul += 1;       // defended by two pawns

    tmp *= mul;
    tmp /= 2;

    Add(e, sd, tmp);
  }

  // Pawn in front of a minor

  if (SqBb(sq) & bbHomeZone[sd]) {
    U64 bbStop = ShiftFwd(SqBb(sq), sd);
    if (bbStop & PcBb(p, sd, P))
      Add(e, sd, 5);
  }
}

void cEngine::Add(eData * e, int sd, int mg, int eg) {

  e->mg_sc[sd] += mg;
  e->eg_sc[sd] += eg;
}

void cEngine::Add(eData * e, int sd, int val) {

  e->mg_sc[sd] += val;
  e->eg_sc[sd] += val;
}

int cEngine::Interpolate(POS * p, eData *e) {

  int mg_tot = e->mg_sc[WC] - e->mg_sc[BC];
  int eg_tot = e->eg_sc[WC] - e->eg_sc[BC];
  int mg_phase = Min(p->phase, 24);
  int eg_phase = 24 - mg_phase;

  return (mg_tot * mg_phase + eg_tot * eg_phase) / 24;
}


int cEngine::Evaluate(POS *p, eData *e) {

  // Try retrieving score from per-thread eval hashtable

  int addr = p->key % EVAL_HASH_SIZE;

  if (EvalTT[addr].key == p->key) {
    int sc = EvalTT[addr].score;
    return p->side == WC ? sc : -sc;
  }

  // Init helper bitboards

  e->pawn_takes[WC] = GetWPControl(PcBb(p, WC, P));
  e->pawn_takes[BC] = GetBPControl(PcBb(p, BC, P));
  e->two_pawns_take[WC] = GetDoubleWPControl(PcBb(p, WC, P));
  e->two_pawns_take[BC] = GetDoubleBPControl(PcBb(p, BC, P));
  e->pawn_can_take[WC] = FillNorth(e->pawn_takes[WC]);
  e->pawn_can_take[BC] = FillSouth(e->pawn_takes[BC]);

  // Run eval subroutines

  ScoreMaterial(p, e, WC);
  ScoreMaterial(p, e, BC);
  ScorePieces(p, e, WC);
  ScorePieces(p, e, BC);
  ScorePawns(p, e, WC);
  ScorePawns(p, e, BC);
  ScoreKing(p, e, WC);
  ScoreKing(p, e, BC);
  ScorePatterns(p, e);
  ScorePassers(p, e, WC);
  ScorePassers(p, e, BC);

  // Interpolate between midgame and endgame score

  int score = Interpolate(p, e);

  // Material imbalance evaluation

  int minorBalance = p->cnt[WC][N] - p->cnt[BC][N] + p->cnt[WC][B] - p->cnt[BC][B];
  int majorBalance = p->cnt[WC][R] - p->cnt[BC][R] + 2 * p->cnt[WC][Q] - 2 * p->cnt[BC][Q];

  int x = Max(majorBalance + 4, 0);
  if (x > 8) x = 8;

  int y = Max(minorBalance + 4, 0);
  if (y > 8) y = 8;

  score += imbalance[x][y];

  // Take care of drawish positions

  int scale = 64;
  if (score > 0) scale = GetDrawFactor(p, WC);
  if (score < 0) scale = GetDrawFactor(p, BC);
  score = (score * scale) / 64;

  // Make sure eval does not exceed mate score

  if (score < -MAX_EVAL)
    score = -MAX_EVAL;
  else if (score > MAX_EVAL)
    score = MAX_EVAL;

  // Save eval score in the evaluation hash table

  EvalTT[addr].key = p->key;
  EvalTT[addr].score = score;

  // Return score relative to the side to move

  return p->side == WC ? score : -score;
}
