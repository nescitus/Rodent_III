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

void cParam::Default(void) {
  mat_weight = 100;
  placement_weight = 100;
}

void cParam::Init(void) {

  int pst_type = 2;

  for (int sq = 0; sq < 64; sq++) {
    for (int sd = 0; sd < 2; sd++) {

      mg_pst[sd][P][REL_SQ(sq, sd)] = ((100 * Par.mat_weight) / 100) + ((pstPawnMg[pst_type][sq] * Par.placement_weight) / 100);
      eg_pst[sd][P][REL_SQ(sq, sd)] = ((100 * Par.mat_weight) / 100) + ((pstPawnEg[pst_type][sq] * Par.placement_weight) / 100);
      mg_pst[sd][N][REL_SQ(sq, sd)] = ((325 * Par.mat_weight) / 100) + ((pstKnightMg[pst_type][sq] * Par.placement_weight) / 100);
      eg_pst[sd][N][REL_SQ(sq, sd)] = ((325 * Par.mat_weight) / 100) + ((pstKnightEg[pst_type][sq] * Par.placement_weight) / 100);
      mg_pst[sd][B][REL_SQ(sq, sd)] = ((325 * Par.mat_weight) / 100) + ((pstBishopMg[pst_type][sq] * Par.placement_weight) / 100);
      eg_pst[sd][B][REL_SQ(sq, sd)] = ((325 * Par.mat_weight) / 100) + ((pstBishopEg[pst_type][sq] * Par.placement_weight) / 100);
      mg_pst[sd][R][REL_SQ(sq, sd)] = ((500 * Par.mat_weight) / 100) + ((pstRookMg[pst_type][sq] * Par.placement_weight) / 100);
      eg_pst[sd][R][REL_SQ(sq, sd)] = ((500 * Par.mat_weight) / 100) + ((pstRookEg[pst_type][sq] * Par.placement_weight) / 100);
      mg_pst[sd][Q][REL_SQ(sq, sd)] = ((975 * Par.mat_weight) / 100) + ((pstQueenMg[pst_type][sq] * Par.placement_weight) / 100);
      eg_pst[sd][Q][REL_SQ(sq, sd)] = ((975 * Par.mat_weight) / 100) + ((pstQueenEg[pst_type][sq] * Par.placement_weight) / 100);
      mg_pst[sd][K][REL_SQ(sq, sd)] = ((pstKingMg[pst_type][sq] * Par.placement_weight) / 100);
      eg_pst[sd][K][REL_SQ(sq, sd)] = ((pstKingEg[pst_type][sq] * Par.placement_weight) / 100);

	  sp_pst[sd][N][REL_SQ(sq, sd)] = pstKnightOutpost[sq];
	  sp_pst[sd][B][REL_SQ(sq, sd)] = pstBishopOutpost[sq];
    }
  }

  // Init king attack table

  for (int t = 0, i = 1; i < 511; ++i) {
    t = Min(1280.0, Min(int(0.027 * i * i), t + 8.0));
    danger[i] = (t * 100) / 256; // rescale to centipawns
  }

  // Init tables for adjusting piece values 
  // according to the number of own pawns

  for (int i = 0; i < 9; i++) {
    np_table[i] = adj[i] * 6; // TODO: make 6 a variable
    rp_table[i] = adj[i] * 3; // TODO: make 3 a varialbe
  }

  // Init support mask (for detecting weak pawns)

  for (int sq = 0; sq < 64; sq++) {
    support_mask[WC][sq] = ShiftWest(SqBb(sq)) | ShiftEast(SqBb(sq));
    support_mask[WC][sq] |= FillSouth(support_mask[WC][sq]);

    support_mask[BC][sq] = ShiftWest(SqBb(sq)) | ShiftEast(SqBb(sq));
    support_mask[BC][sq] |= FillNorth(support_mask[BC][sq]);
  }

  // Init mask for passed pawn detection

  for (int sq = 0; sq < 64; sq++) {
    passed_mask[WC][sq] = FillNorthExcl(SqBb(sq));
    passed_mask[WC][sq] |= ShiftSideways(passed_mask[WC][sq]);
    passed_mask[BC][sq] = FillSouthExcl(SqBb(sq));
    passed_mask[BC][sq] |= ShiftSideways(passed_mask[BC][sq]);
  }
}

void cEngine::ScorePieces(POS *p, eData *e, int sd) {

  U64 bb_pieces, bb_attacks, bb_control;
  int op, sq, ksq, cnt;
  int att = 0;

  // Init score with data from board class

  e->mg_sc[sd] = p->mg_sc[sd];
  e->eg_sc[sd] = p->eg_sc[sd];

  // Material adjustment

  int tmp = Par.np_table[p->cnt[sd][P]] * p->cnt[sd][N]   // knights lose value as pawns disappear
	      - Par.rp_table[p->cnt[sd][P]] * p->cnt[sd][R];  // rooks gain value as pawns disappear

  if (p->cnt[sd][B] == 2) tmp += 50;                      // bishop pair
  if (p->cnt[sd][N] == 2) tmp -= 10;                      // knight pair
  if (p->cnt[sd][R] == 2) tmp -= 5;                       // rook pair

  tmp = ((tmp * Par.mat_weight) / 100);

  Add(e, sd, tmp, tmp);

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
	if (bb_control & ~p->cl_bb[sd] & n_checks) att += 4;                   // check threats

    // knight mobility score

    cnt = PopCnt(bb_control &~e->pawn_takes[op]);
    Add(e, sd, 4 * (cnt-4), 4 * (cnt-4));

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
    Add(e, sd, 5 * (cnt - 7),  5 * (cnt - 7));

	ScoreOutpost(p, e, sd, B, sq);
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
    Add(e, sd, 2 * (cnt - 7), 4 * (cnt - 7));

    // rook on (half) open file

    U64 r_file = FillNorth(SqBb(sq)) | FillSouth(SqBb(sq));
    if (!(r_file & PcBb(p, sd, P))) {
      if (!(r_file & PcBb(p, op, P))) Add(e, sd, 12, 12);
      else                            Add(e, sd,  6,  6);
    }

    // rook on 7th rank attacking pawns or cutting off enemy king

    if (SqBb(sq) & bbRelRank[sd][RANK_7]) {
      if (PcBb(p, op, P) & bbRelRank[sd][RANK_7]
      ||  PcBb(p, op, K) & bbRelRank[sd][RANK_8]) {
          Add(e, sd, 16, 32);
      }
    }
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
    Add(e, sd, 1 * (cnt - 14), 2 * (cnt - 14));
  }

  // king attack

  if (PcBb(p, sd, Q)) {
    int att_score = Par.danger[att];
    Add(e, sd, att_score, att_score);
  }

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

    // PASSED PAWNS

    if (!(passed_mask[sd][sq] & PcBb(p, Opp(sd), P))) {
      e->mg_sc[sd] += passed_bonus_mg[sd][Rank(sq)];
      e->eg_sc[sd] += passed_bonus_eg[sd][Rank(sq)];
    }
    
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

void cEngine::ScorePatterns(POS * p, eData * e) {

  U64 king_mask, rook_mask;
  static const U64 wb_mask = { SqBb(A7) | SqBb(B8) | SqBb(H7) | SqBb(G8) | SqBb(C1) | SqBb(F1) | SqBb(G2) | SqBb(B2) };
  static const U64 bb_mask = { SqBb(A2) | SqBb(B1) | SqBb(H2) | SqBb(G1) | SqBb(C8) | SqBb(F8) | SqBb(G7) | SqBb(B7) };

  // Rook blocked by uncastled king

  king_mask = SqBb(F1) | SqBb(G1);
  rook_mask = SqBb(G1) | SqBb(H1) | SqBb(H2);

  if ((PcBb(p, WC, K) & king_mask)
  &&  (PcBb(p, WC, R) & rook_mask)) Add(e, WC, -50, 0);

  king_mask = SqBb(B1) | SqBb(C1);
  rook_mask = SqBb(A1) | SqBb(B1) | SqBb(A2);

  if ((PcBb(p, WC, K) & king_mask)
  &&  (PcBb(p, WC, R) & rook_mask)) Add(e, WC, -50, 0);

  king_mask = SqBb(F8) | SqBb(G8);
  rook_mask = SqBb(G8) | SqBb(H8) | SqBb(H7);

  if ((PcBb(p, BC, K) & king_mask)
  &&  (PcBb(p, BC, R) & rook_mask)) Add(e, BC, -50, 0);

  king_mask = SqBb(B8) | SqBb(C8);
  rook_mask = SqBb(C8) | SqBb(B8) | SqBb(B7);

  if ((PcBb(p, BC, K) & king_mask)
  && (PcBb(p, BC, R) & rook_mask)) Add(e, BC, -50, 0);

  // trapped knight

  if (IsOnSq(p, WC, N, A7) && IsOnSq(p, BC, P, A6) && IsOnSq(p, BC, P, B7)) Add(e, WC, -150);
  if (IsOnSq(p, WC, N, H7) && IsOnSq(p, BC, P, H6) && IsOnSq(p, BC, P, G7)) Add(e, WC, -150);
  if (IsOnSq(p, BC, N, A2) && IsOnSq(p, WC, P, A3) && IsOnSq(p, WC, P, B2)) Add(e, BC, -150);
  if (IsOnSq(p, BC, N, H2) && IsOnSq(p, WC, P, H3) && IsOnSq(p, WC, P, G2)) Add(e, BC, -150);

  if (PcBb(p, WC, B) & wb_mask) {

    // white bishop trapped

    if (IsOnSq(p, WC, B, A7) && IsOnSq(p, BC, P, B6) ) Add(e, WC, -150);
    if (IsOnSq(p, WC, B, B8) && IsOnSq(p, BC, P, C7) ) Add(e, WC, -150);
    if (IsOnSq(p, WC, B, H7) && IsOnSq(p, BC, P, G6) ) Add(e, WC, -150);
    if (IsOnSq(p, WC, B, G8) && IsOnSq(p, BC, P, F7) ) Add(e, WC, -150);

    // white bishop blocked on its initial square by own pawn

    if (IsOnSq(p, WC, B, C1) && IsOnSq(p, WC, P, D2) && (SqBb(D3) & OccBb(p)))
      Add(e, WC, -50, 0);
    if (IsOnSq(p, WC, B, F1) && IsOnSq(p, WC, P, E2) && (SqBb(E3) & OccBb(p)))
      Add(e, WC, -50, 0);

    // white bishop fianchettoed

    if (IsOnSq(p, WC, B, B2)) {
      if (IsOnSq(p, WC, P, C3)) Add(e, WC, -10, -20);
      if (IsOnSq(p, WC, P, B3) && (IsOnSq(p, WC, P, A2) || IsOnSq(p, WC, P, C2))) Add(e, WC,  4);
      if (IsOnSq(p, BC, P, D4) && (IsOnSq(p, BC, P, E5) || IsOnSq(p, BC, P, C5))) Add(e, WC, -20);
    }

    if (IsOnSq(p, WC, B, G2)) {
      if (IsOnSq(p, WC, P, F3)) Add(e, WC, -10, -20);
      if (IsOnSq(p, WC, P, G3) && (IsOnSq(p, WC, P, H2) || IsOnSq(p, WC, P, F2))) Add(e, WC,  4);
      if (IsOnSq(p, BC, P, E4) && (IsOnSq(p, BC, P, D5) || IsOnSq(p, BC, P, F5))) Add(e, WC, -20);
    }
  }

  if (PcBb(p, BC, B) & bb_mask) {

    // black bishop trapped

    if (IsOnSq(p, BC, B, A2) && IsOnSq(p, WC, P, B3) ) Add(e, BC, -150);
    if (IsOnSq(p, BC, B, B1) && IsOnSq(p, WC, P, C2) ) Add(e, BC, -150);
    if (IsOnSq(p, BC, B, H2) && IsOnSq(p, WC, P, G3) ) Add(e, BC, -150);
    if (IsOnSq(p, BC, B, G1) && IsOnSq(p, WC, P, F2) ) Add(e, BC, -150);

    // black bishop blocked on its initial square by own pawn

    if (IsOnSq(p, BC, B, C8) && IsOnSq(p, BC, P, D7) && (SqBb(D6) & OccBb(p)))
      Add(e, BC, -50, 0);
    if (IsOnSq(p, BC, B, F8) && IsOnSq(p, BC, P, E7) && (SqBb(E6) & OccBb(p)))
      Add(e, BC, -50, 0);

    // black bishop fianchettoed

    if (IsOnSq(p, BC, B, B7)) { 
      if (IsOnSq(p, BC, P, C6)) Add(e, BC, -10, -20);
      if (IsOnSq(p, BC, P, B6) && (IsOnSq(p, BC, P, A7) || IsOnSq(p, BC, P, C7))) Add(e, BC,  4);
      if (IsOnSq(p, WC, P, D5) && (IsOnSq(p, WC, P, E4) || IsOnSq(p, WC, P, C4))) Add(e, BC, -20); 
    }
    if (IsOnSq(p, BC, B, G7)) {
      if (IsOnSq(p, BC, P, F6)) Add(e, BC, -10, -20);
      if (IsOnSq(p, BC, P, G6) && (IsOnSq(p, BC, P, H7) || IsOnSq(p, BC, P, G6))) Add(e, BC,  4);
      if (IsOnSq(p, WC, P, E5) && (IsOnSq(p, WC, P, D4) || IsOnSq(p, WC, P, F4))) Add(e, BC, -20);
    }
  }

  // Castling rights

  if (IsOnSq(p, WC, K, E1)) {
    if ((p->c_flags & W_KS) || (p->c_flags & W_QS)) Add(e, WC, 10, 0);
  }

  if (IsOnSq(p, BC, K, E8)) {
    if ((p->c_flags & B_KS) || (p->c_flags & B_QS)) Add(e, BC, 10, 0);
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

int cEngine::GetDrawFactor(POS * p, int sd) {

  int op = Opp(sd);

  if (p->phase < 2 && p->cnt[sd][P] == 0) return 0;

  return 64;
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

  ScorePieces(p, e, WC);
  ScorePieces(p, e, BC);
  ScorePawns(p, e, WC);
  ScorePawns(p, e, BC);
  ScoreKing(p, e, WC);
  ScoreKing(p, e, BC);

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
