#include "skeleton.h"
#include "eval.h"
#include <stdio.h>


static const U64 bbKingBlockH[2] = { SqBb(H8) | SqBb(H7) | SqBb(G8) | SqBb(G7),
                                     SqBb(H1) | SqBb(H2) | SqBb(G1) | SqBb(G2) };

static const U64 bbKingBlockA[2] = { SqBb(A8) | SqBb(A7) | SqBb(B8) | SqBb(B7),
SqBb(A1) | SqBb(A2) | SqBb(B1) | SqBb(B2) };

void cParam::DefaultWeights(void) {
   own_att = 100;
   opp_att = 100;
   own_mob = 100;
   opp_mob = 100;
   tropism = 20;
   forwardness = 0;
   passers = 100;
   lines = 100;
}

void cParam::Init(void) {

  int r_delta, f_delta;
  static const int pst_weight[3] = { 80, 80, 100 };
  int pst_type = 2;

  for (int sq = 0; sq < 64; sq++) {
    for (int sd = 0; sd < 2; sd++) {
 
      mg_pst[sd][P][REL_SQ(sq, sd)] = pc_value[P] + ( pstPawnMg  [pst_type][sq] * pst_weight[pst_type]) / 100;
      eg_pst[sd][P][REL_SQ(sq, sd)] = pc_value[P] + ( pstPawnEg  [pst_type][sq] * pst_weight[pst_type]) / 100;
      mg_pst[sd][N][REL_SQ(sq, sd)] = pc_value[N] + ( pstKnightMg[pst_type][sq] * pst_weight[pst_type]) / 100;
      eg_pst[sd][N][REL_SQ(sq, sd)] = pc_value[N] + ( pstKnightEg[pst_type][sq] * pst_weight[pst_type]) / 100;
      mg_pst[sd][B][REL_SQ(sq, sd)] = pc_value[B] + ( pstBishopMg[pst_type][sq] * pst_weight[pst_type]) / 100;
      eg_pst[sd][B][REL_SQ(sq, sd)] = pc_value[B] + ( pstBishopEg[pst_type][sq] * pst_weight[pst_type]) / 100;
      mg_pst[sd][R][REL_SQ(sq, sd)] = pc_value[R] + ( pstRookMg  [pst_type][sq] * pst_weight[pst_type]) / 100;
      eg_pst[sd][R][REL_SQ(sq, sd)] = pc_value[R] + ( pstRookEg  [pst_type][sq] * pst_weight[pst_type]) / 100;
      mg_pst[sd][Q][REL_SQ(sq, sd)] = pc_value[Q] + ( pstQueenMg [pst_type][sq] * pst_weight[pst_type]) / 100;
      eg_pst[sd][Q][REL_SQ(sq, sd)] = pc_value[Q] + ( pstQueenEg [pst_type][sq] * pst_weight[pst_type]) / 100;
      mg_pst[sd][K][REL_SQ(sq, sd)] = pc_value[Q] + ( pstKingMg  [pst_type][sq] * pst_weight[pst_type]) / 100;
      eg_pst[sd][K][REL_SQ(sq, sd)] = pc_value[Q] + ( pstKingEg  [pst_type][sq] * pst_weight[pst_type]) / 100;

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
    np_table[i] = adj[i] * 6;
    rp_table[i] = adj[i] * 3;
  }

  // Init distance tables (for evaluating king tropism and unstoppable passers)

  for (int sq1 = 0; sq1 < 64; ++sq1) {
    for (int sq2 = 0; sq2 < 64; ++sq2) {
      r_delta = Abs(Rank(sq1) - Rank(sq2));
      f_delta = Abs(File(sq1) - File(sq2));
      dist[sq1][sq2] = 14 - (r_delta + f_delta);
      chebyshev_dist[sq1][sq2] = Max(r_delta, f_delta);
    }
  }

}

void cEngine::Init(int th) {

  thread_id = th;
}

void cEngine::EvaluateMaterial(POS * p, eData *e, int sd) {

  int op = Opp(sd);

  // Piece configurations

  int tmp = Par.np_table[p->cnt[sd][P]] * p->cnt[sd][N]   // knights lose value as pawns disappear
          - Par.rp_table[p->cnt[sd][P]] * p->cnt[sd][R];  // rooks gain value as pawns disappear

  if (p->cnt[sd][N] > 1) tmp += -10;
  if (p->cnt[sd][R] > 1) tmp += -5;

  if (p->cnt[sd][B] > 1)                                  // Bishop pair
    Add(e, sd, 50, 60);

  // "elephantiasis correction" for queen, idea by H.G.Mueller (nb. rookVsQueen doesn't help)

  if (p->cnt[sd][Q])
    tmp -= 5 * (p->cnt[op][N] + p->cnt[op][B]);

  Add(e, sd, tmp);
}

void cEngine::EvaluatePieces(POS *p, eData *e, int sd) {

  U64 pieces, attack, control, contact, zone, file, stop;
  int sq, cnt, att, wood, tmp, mul, own_p_cnt, opp_p_cnt;
  int r_on_7th = 0;
  int mob_mg = 0;
  int mob_eg = 0;
  int tropism_mg = 0;
  int tropism_eg = 0;
  int lines_mg = 0;
  int lines_eg = 0;
  int fwd_weight = 0;
  int fwd_cnt = 0;

  int op = Opp(sd);
  int ksq = KingSq(p, op);
  zone = BB.KingAttacks(ksq);
  zone = zone |= BB.ShiftFwd(zone, op);
  att = 0;

  U64 n_checks = BB.KnightAttacks(ksq) & ~p->cl_bb[sd] & ~e->p_takes[op];
  U64 b_checks = BB.BishAttacks(OccBb(p), ksq) & ~p->cl_bb[sd] & ~e->p_takes[op];
  U64 r_checks = BB.RookAttacks(OccBb(p), ksq) & ~p->cl_bb[sd] & ~e->p_takes[op];
  U64 q_checks = r_checks & b_checks;

  // Knight eval

  pieces = p->Knights(sd);
  while (pieces) {
    sq = BB.PopFirstBit(&pieces);                       // get square
    tropism_mg += 3 * Par.dist[sq][ksq];                // midgame king tropism
    tropism_eg += 3 * Par.dist[sq][ksq];                // endgame king tropism
    if (SqBb(sq) & Mask.away[sd]) {                     // forwardness
      fwd_weight += 1;
      fwd_cnt += 1;
    }

    control = BB.KnightAttacks(sq) & ~p->cl_bb[sd];     // get control bitboard
    if (!(control  &~e->p_takes[op] & Mask.away[sd]))   // we do not attack enemy half of the board
      Add(e, sd, -5);
    e->all_att[sd] |= BB.KnightAttacks(sq);
    e->ev_att[sd] |= BB.KnightAttacks(sq);
    if (control & n_checks) att += 4;                   // check threats
    att += 6 * BB.PopCnt(control & zone);               // king attack
    cnt = BB.PopCnt(control &~e->p_takes[op]);          // get mobility count
	mob_mg += 4 * (cnt - 4);
	mob_eg += 4 * (cnt - 4);
    ScoreOutpost(p, e, sd, N, sq);                      // outpost
  }

  // Bishop eval

  pieces = p->Bishops(sd);
  while (pieces) {
    sq = BB.PopFirstBit(&pieces);                       // get square
    tropism_mg += 2 * Par.dist[sq][ksq];                // midgame king tropism
    tropism_eg += 1 * Par.dist[sq][ksq];                // endgame king tropism
    if (SqBb(sq) & Mask.away[sd]) {                     // forwardness
      fwd_weight += 1;
      fwd_cnt += 1;
    }
    control = BB.BishAttacks(OccBb(p), sq);             // get control bitboard
    e->all_att[sd] |= control;
    e->ev_att[sd] |= control;
    if (!(control & Mask.away[sd] )) Add(e, sd, -5);    // we do not attack enemy half of the board
    if (control & b_checks) att += 4;                   // check threats
    attack = BB.BishAttacks(OccBb(p) ^ p->Queens(sd), sq);  // get king attack bitboard
    att += 6 * BB.PopCnt(attack & zone);                // evaluate king attacks
    cnt = BB.PopCnt(control &~e->p_takes[op]);          // get mobility count
	mob_mg += 5 * (cnt - 6);
	mob_eg += 5 * (cnt - 6);
    ScoreOutpost(p, e, sd, B, sq);                      // outpost

    // Pawns on the same square color as our bishop
    if (bbWhiteSq & SqBb(sq)) {
      own_p_cnt = BB.PopCnt(bbWhiteSq & p->Pawns(sd)) - 4;
      opp_p_cnt = BB.PopCnt(bbWhiteSq & p->Pawns(op)) - 4;
    } else {
      own_p_cnt = BB.PopCnt(bbBlackSq & p->Pawns(sd)) - 4;
      opp_p_cnt = BB.PopCnt(bbBlackSq & p->Pawns(op)) - 4;
    }

    Add(e, sd, -3 * own_p_cnt - opp_p_cnt);

  }

  // Rook eval

  pieces = p->Rooks(sd);
  while (pieces) {
    sq = BB.PopFirstBit(&pieces);                       // get square
    tropism_mg += 2 * Par.dist[sq][ksq];                // midgame king tropism
    tropism_eg += 1 * Par.dist[sq][ksq];                // endgame king tropism
    if (SqBb(sq) & Mask.away[sd]) {                     // forwardness
      fwd_weight += 2;
      fwd_cnt += 1;
    }
    control = BB.RookAttacks(OccBb(p), sq);             // get control bitboard
    e->all_att[sd] |= control;
    e->ev_att[sd] |= control;
    if ((control & ~p->cl_bb[sd] & r_checks)
    && p->Queens(sd)) {
      att += 9;                                         // check threat bonus
      contact = (control & BB.KingAttacks(ksq)) & r_checks;  // get contact check bitboard

      while (contact) {
        int contactSq = BB.PopFirstBit(&contact);       // find a potential contact check
        if (Swap(p, sq, contactSq) >= 0) {              // rook exchanges are also accepted
          att += 24;
          break;
        }
      }
    }
    attack = BB.RookAttacks(OccBb(p) ^ p->StraightMovers(sd), sq);  // get king attack bitboard
    att += 9 * BB.PopCnt(attack & zone);                // king attack
    cnt = BB.PopCnt(control);                           // get mobility count
	mob_mg += 2 * (cnt - 7);
	mob_eg += 4 * (cnt - 7);
                                                        // FILE EVALUATION:

    file = BB.FillNorth(SqBb(sq)) | BB.FillSouth(SqBb(sq));   // get file
	if (file & p->Queens(op)) {                         // enemy queen on rook's file
		lines_mg += 5;
		lines_eg += 5;
	}
    if (!(file & p->Pawns(sd))) {                       // no own pawns on that file
		if (!(file & p->Pawns(op))) {
			lines_mg += 14;
			lines_eg += 14;
		}
      else {                                            // half-open file...
		if (file & (p->Pawns(op) & e->p_takes[op])) {   // ...with defended pawn on it
			lines_mg += 5;
			lines_eg += 5;
		} else {                                        // ...with undefended pawn on it
			lines_mg += 7;
			lines_eg += 7;
		}
      }
  }

    // Rook on the 7th rank attacking pawns or cutting off enemy king

    if (SqBb(sq) & bbRelRank[sd][RANK_7]) {             // rook on 7th rank
      if (p->Pawns(op) & bbRelRank[sd][RANK_7]          // attacking enemy pawns
      || p->Kings(op) & bbRelRank[sd][RANK_8]) {        // or cutting off enemy king
		 lines_mg += 16;
		 lines_eg += 32;
         r_on_7th++;
      }
    }
  }

  // Queen eval

  pieces = p->Queens(sd);
  while (pieces) {
    sq = BB.PopFirstBit(&pieces);                       // get square
    tropism_mg += 2 * Par.dist[sq][ksq];                // midgame king tropism
    tropism_eg += 4 * Par.dist[sq][ksq];                // endgame king tropism
    if (SqBb(sq) & Mask.away[sd]) {                     // forwardness
      fwd_weight += 4;
      fwd_cnt += 1;
    }
    control = BB.QueenAttacks(OccBb(p), sq);            // get control bitboard
    e->all_att[sd] |= control;
    if (control & q_checks) {                           // check threat bonus
      att += 12;

      contact = control & BB.KingAttacks(ksq);          // queen contact checks
      while (contact) {
        int contactSq = BB.PopFirstBit(&contact);       // find potential contact check square 
        if (Swap(p, sq, contactSq) >= 0) {              // if check doesn't lose material, evaluate
          att += 36;
          break;
        }
      }
    }

    attack = BB.BishAttacks(OccBb(p) ^ p->DiagMovers(sd), sq);
    attack |= BB.RookAttacks(OccBb(p) ^ p->StraightMovers(sd), sq);
    att += 15 * BB.PopCnt(attack & zone);               // king attack
    cnt = BB.PopCnt(control);                           // get mobility count
	mob_mg += 1 * (cnt - 14);
	mob_eg += 2 * (cnt - 14);

    if (SqBb(sq) & bbRelRank[sd][RANK_7]) {             // queen on 7th rank
      if (p->Pawns(op) & bbRelRank[sd][RANK_7]          // attacking enemy pawns
      ||  p->Kings(op) & bbRelRank[sd][RANK_8]) {       // or cutting off enemy king
		 lines_mg += 4;
		 lines_eg += 8;
      }
    }
  } // end of queen eval

  // Composite factors

  if (r_on_7th > 1) {  // two rooks on 7th rank
	  lines_mg += 8;
	  lines_eg += 16;
  }

  Add(e, sd, (Par.sd_mob[sd] * mob_mg) / 100, (Par.sd_mob[sd] * mob_eg) / 100);
  Add(e, sd, (Par.tropism * tropism_mg) / 100, (Par.tropism * tropism_eg) / 100);
  Add(e, sd, (Par.lines * lines_mg) / 100, (Par.lines * lines_eg) / 100);
  Add(e, sd, (Par.forwardness * fwd_bonus[fwd_cnt] * fwd_weight) / 100, 0);

  // King attack eval

  if (att > 399) att = 399;
  if (p->cnt[sd][Q] == 0) att = 0;
  Add(e, sd, (Par.danger[att] * Par.sd_att[sd]) / 100);

}

void cEngine::ScoreOutpost(POS *p, eData *e, int sd, int pc, int sq) {

  if (SqBb(sq) & Mask.home[sd]) {
    U64 stop = BB.ShiftFwd(SqBb(sq), sd);             // get square in front of a minor
    if (stop & p->Pawns(sd))                          // is it occupied by own pawn?
    Add(e, sd, 5);                                    // bonus for a pawn shielding a minor
  }

  int tmp = Par.sp_pst[sd][pc][sq];                   // get base outpost bonus
  if (tmp) {
    int mul = 0;                                      // reset outpost multiplier
    if (SqBb(sq) & ~e->p_can_take[Opp(sd)]) mul += 2; // is piece in hole of enemy pawn structure?
    if (SqBb(sq) & e->p_takes[sd]) mul += 1;          // is piece defended by own pawn?
    if (SqBb(sq) & e->two_pawns_take[sd]) mul += 1;   // is piece defended by two pawns?
    Add(e, sd, (tmp * mul) / 2);                      // add outpost bonus
  }
}

void cEngine::EvaluatePawns(POS *p, eData *e, int sd) {

  U64 pieces, front_span, fl_phalanx;
  int sq, fl_unopposed;
  int op = Opp(sd);

  pieces = p->Pawns(sd);
  while (pieces) {
    sq = BB.PopFirstBit(&pieces);
    front_span = BB.GetFrontSpan(SqBb(sq), sd);
    fl_unopposed = ((front_span & p->Pawns(op) == 0));
    fl_phalanx = (BB.ShiftSideways(SqBb(sq)) & p->Pawns(sd));

    // candidate passers

    if (fl_unopposed) {
      if (fl_phalanx) {
      if (BB.PopCnt((Mask.passed[sd][sq] & p->Pawns(op))) == 1)
        AddPawns(e, sd, passed_bonus_mg[sd][Rank(sq)] / 3, passed_bonus_eg[sd][Rank(sq)] / 3);
      }
    }

    // doubled pawn

    if (front_span & p->Pawns(sd))
      AddPawns(e, sd, -12, -24);

    // supported pawn

    if (fl_phalanx)                     AddPawns(e, sd, pstPhalanxPawn[REL_SQ(sq, sd)], pstPhalanxPawn[REL_SQ(sq, sd)]); // scores twice !!!
    else if (SqBb(sq) & e->p_takes[sd]) AddPawns(e, sd, pstDefendedPawn[REL_SQ(sq, sd)], pstDefendedPawn[REL_SQ(sq, sd)]);
	
    // isolated and weak pawn

    if (!(Mask.adjacent[File(sq)] & p->Pawns(sd)))
      AddPawns(e, sd, -10 - 10 * fl_unopposed, -20);
    else if (!(Mask.supported[sd][sq] & p->Pawns(sd)))
      AddPawns(e, sd, -8 - 8 * fl_unopposed, -8);
  }
}

void cEngine::EvaluatePassers(POS *p, eData *e, int sd) {

  U64 pieces, stop, mul;
  int sq, mg_tmp, eg_tmp, fl_unopposed;
  int op = Opp(sd);
  int mg_tot = 0;
  int eg_tot = 0;

  pieces = p->Pawns(sd);
  while (pieces) {
    sq = BB.PopFirstBit(&pieces);

    // passed pawns

    if (!(Mask.passed[sd][sq] & p->Pawns(op))) {
      mul = 100;
      stop = BB.ShiftFwd(SqBb(sq), sd);

      if (stop & OccBb(p)) mul -= 20;   // blocked passers score less

      else if ((stop & e->all_att[sd])  // our control of stop square
           && (stop & ~e->all_att[op])) mul += 10;

      mg_tmp = passed_bonus_mg[sd][Rank(sq)];
      eg_tmp = passed_bonus_eg[sd][Rank(sq)] 
             -((passed_bonus_eg[sd][Rank(sq)] * Par.dist[sq][p->king_sq[op]]) / 30);
	  mg_tot += (mg_tmp * mul) / 100;
	  eg_tot += (eg_tmp * mul) / 100;
    }
  }

  Add(e, sd, (mg_tot * Par.passers) / 100, (eg_tot * Par.passers) / 100);
}

void cEngine::ScoreUnstoppable(eData *e, POS * p) {

  U64 bbPieces, bbSpan, bbProm;
  int w_dist = 8;
  int b_dist = 8;
  int sq, ksq, psq, tempo, prom_dist;

  // White unstoppable passers

  if (p->cnt[BC][N] + p->cnt[BC][B] + p->cnt[BC][R] + p->cnt[BC][Q] == 0) {
    ksq = KingSq(p, BC);
    if (p->side == BC) tempo = 1; else tempo = 0;
    bbPieces = p->Pawns(WC);
    while (bbPieces) {
      sq = BB.PopFirstBit(&bbPieces);
      if (!(Mask.passed[WC][sq] & p->Pawns(BC))) {
        bbSpan = BB.GetFrontSpan(SqBb(sq), WC);
        psq = ((WC - 1) & 56) + (sq & 7);
        prom_dist = Min(5, Par.chebyshev_dist[sq][psq]);

        if (prom_dist < (Par.chebyshev_dist[ksq][psq] - tempo)) {
          if (bbSpan & p->Kings(WC)) prom_dist++;
          w_dist = Min(w_dist, prom_dist);
        }
      }
    }
  }

  // Black unstoppable passers

  if (p->cnt[WC][N] + p->cnt[WC][B] + p->cnt[WC][R] + p->cnt[WC][Q] == 0) {
    ksq = KingSq(p, WC);
    if (p->side == WC) tempo = 1; else tempo = 0;
    bbPieces = p->Pawns(BC);
    while (bbPieces) {
      sq = BB.PopFirstBit(&bbPieces);
      if (!(Mask.passed[BC][sq] & p->Pawns(WC))) {
        bbSpan = BB.GetFrontSpan(SqBb(sq), BC);
        if (bbSpan & p->Kings(WC)) tempo -= 1;
        psq = ((BC - 1) & 56) + (sq & 7);
        prom_dist = Min(5, Par.chebyshev_dist[sq][psq]);

        if (prom_dist < (Par.chebyshev_dist[ksq][psq] - tempo)) {
          if (bbSpan & p->Kings(BC)) prom_dist++;
          b_dist = Min(b_dist, prom_dist);
        }
      }
    }
  }

  if (w_dist < b_dist - 1) Add(e, WC, 0, 500);
  if (b_dist < w_dist - 1) Add(e, BC, 0, 500);
}


void cEngine::Add(eData *e, int sd, int mg_val, int eg_val) {

  e->mg[sd] += mg_val;
  e->eg[sd] += eg_val;
}

void cEngine::Add(eData *e, int sd, int val) {

  e->mg[sd] += val;
  e->eg[sd] += val;
}

void cEngine::AddPawns(eData *e, int sd, int mg_val, int eg_val) {

  e->mg_pawns[sd] += mg_val;
  e->eg_pawns[sd] += eg_val;
}

int cEngine::Interpolate(POS * p, eData * e) {

  int mg_tot = e->mg[WC] - e->mg[BC];
  int eg_tot = e->eg[WC] - e->eg[BC];
  int mg_phase = Min(p->phase, 24);
  int eg_phase = 24 - mg_phase;

   return (mg_tot * mg_phase + eg_tot * eg_phase) / 24;
}

void cEngine::EvaluatePatterns(POS * p, eData * e) {

  U64 king_mask, rook_mask;
  static const U64 wb_mask = { SqBb(A7) | SqBb(B8) | SqBb(H7) | SqBb(G8) | SqBb(C1) | SqBb(F1) | SqBb(G2) | SqBb(B2) };
  static const U64 bb_mask = { SqBb(A2) | SqBb(B1) | SqBb(H2) | SqBb(G1) | SqBb(C8) | SqBb(F8) | SqBb(G7) | SqBb(B7) };

  // Rook blocked by uncastled king

  king_mask = SqBb(F1) | SqBb(G1);
  rook_mask = SqBb(G1) | SqBb(H1) | SqBb(H2);

  if ((p->Kings(WC) & king_mask)
  &&  (p->Rooks(WC) & rook_mask)) Add(e, WC, -50, 0);

  king_mask = SqBb(B1) | SqBb(C1);
  rook_mask = SqBb(A1) | SqBb(B1) | SqBb(A2);

  if ((p->Kings(WC) & king_mask)
  &&  (p->Rooks(WC) & rook_mask)) Add(e, WC, -50, 0);

  king_mask = SqBb(F8) | SqBb(G8);
  rook_mask = SqBb(G8) | SqBb(H8) | SqBb(H7);

  if ((p->Kings(BC) & king_mask)
  &&  (p->Rooks(BC) & rook_mask)) Add(e, BC, -50, 0);

  king_mask = SqBb(B8) | SqBb(C8);
  rook_mask = SqBb(C8) | SqBb(B8) | SqBb(B7);

  if ((p->Kings(BC) & king_mask)
  &&  (p->Rooks(BC) & rook_mask)) Add(e, BC, -50, 0);

  // trapped knight

  if (IsOnSq(p, WC, N, A7) && IsOnSq(p, BC, P, A6) && IsOnSq(p, BC, P, B7)) Add(e, WC, -150);
  if (IsOnSq(p, WC, N, H7) && IsOnSq(p, BC, P, H6) && IsOnSq(p, BC, P, G7)) Add(e, WC, -150);
  if (IsOnSq(p, BC, N, A2) && IsOnSq(p, WC, P, A3) && IsOnSq(p, WC, P, B2)) Add(e, BC, -150);
  if (IsOnSq(p, BC, N, H2) && IsOnSq(p, WC, P, H3) && IsOnSq(p, WC, P, G2)) Add(e, BC, -150);

  if (p->Bishops(WC) & wb_mask) {

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

  if (p->Bishops(BC) & bb_mask) {

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

int cEngine::GetDrawFactor(POS * p, int sd) {

  int op = Opp(sd);

  if (p->phase == 0) {
    if (p->cnt[sd][P] == 1    // TODO: all pawns of a stronger side on a rim
    && p->cnt[op][P] == 0) {  // TODO: accept pawns for a weaker side

      if (p->Pawns(sd) & FILE_H_BB
      &&  p->Kings(op) & bbKingBlockH[sd]) return 0;

      if (p->Pawns(sd) & FILE_A_BB
      &&  p->Kings(op) & bbKingBlockA[sd]) return 0;
    }
  }

  if (p->phase == 1) {
    if ( p->cnt[sd][B] == 1
	&& p->cnt[sd][P] == 1) { // TODO: all pawns of a stronger side on a rim

	  if (p->Pawns(sd) & FILE_H_BB
		  && NotOnBishColor(p, sd, REL_SQ(H8, sd))
		  && p->Kings(op)  & bbKingBlockH[sd]) return 0;

	  if (p->Pawns(sd) & FILE_A_BB
		  && NotOnBishColor(p, sd, REL_SQ(A8, sd))
		  && p->Kings(op)  & bbKingBlockA[sd]) return 0;
}
  }

  if (p->phase < 2) {
    if (p->Pawns(sd) == 0) return 0;                                                                 // KK, KmK, KmKp, KmKpp
  }

  if (p->phase == 2) {

    if (p->cnt[sd][N] == 2 && p->cnt[sd][P] == 0) {
      if (p->cnt[op][P] == 0) return 0;                                                              // KNNK(m)
      else return 8;                                                                                 // KNNK(m)(p)
    }

    if (p->cnt[sd][B] == 1                                                                           // KBPKm, king blocks
    && p->cnt[op][B] + p->cnt[op][N] == 1
    && p->cnt[sd][P] == 1
    && p->cnt[op][P] == 0
    && (SqBb(p->king_sq[op]) & BB.GetFrontSpan(p->Pawns(sd), sd))
    && NotOnBishColor(p, sd, p->king_sq[op]))
    return 0;

    if (p->cnt[sd][B] == 1                                                                           // KBPKm, king blocks
    && p->cnt[op][B] == 1
    && DifferentBishops(p) ) return 32;                                                              // BOC
  }

  if (p->phase == 3 && p->cnt[sd][P] == 0) {
    if (p->cnt[sd][R] == 1 && p->cnt[op][B] + p->cnt[op][N] == 1) return 16;                         // KRKm(p)
    if (p->cnt[sd][B] + p->cnt[sd][N] == 2 && p->cnt[op][B] == 1) return 8;                          // KmmKB(p)
    if (p->cnt[sd][B] == 1 && p->cnt[sd][N] == 1 && p->cnt[op][B] + p->cnt[op][N] == 1) return 8;    // KBNKm(p)
  }

  if (p->phase == 4 & p->cnt[sd][R] == 1 && p->cnt[op][R] == 1                                       // KRPKR
  && p->cnt[sd][P] == 1 && p->cnt[op][P] == 0) {
    if ((SqBb(p->king_sq[op]) & BB.GetFrontSpan(p->Pawns(sd), sd)))
       return 32; // 1/2
  }

  // KRMKR(p)

  if (p->phase == 5 && p->cnt[sd][P] == 0) {
    if (p->cnt[sd][R] == 1 && p->cnt[sd][B] + p->cnt[sd][N] == 1 && p->cnt[op][R] == 1) return 16;
  }

  return 64;
}

int cEngine::NotOnBishColor(POS * p, int bishSide, int sq) {

  if (((bbWhiteSq & p->Bishops(bishSide)) == 0)
  && (SqBb(sq) & bbWhiteSq)) return 1;

  if (((bbBlackSq & p->Bishops(bishSide)) == 0)
  && (SqBb(sq) & bbBlackSq)) return 1;

  return 0;
}

int cEngine::DifferentBishops(POS * p) {

  if ((bbWhiteSq & p->Bishops(WC)) && (bbBlackSq & p->Bishops(BC))) return 1;
  if ((bbBlackSq & p->Bishops(WC)) && (bbWhiteSq & p->Bishops(BC))) return 1;
  return 0;
}

void cEngine::ScoreHanging(POS *p, eData *e, int sd) {

  int pc, sq, sc;
  int op = Opp(sd);
  U64 bbHanging = p->cl_bb[op]    & ~e->p_takes[op];
  U64 bbThreatened = p->cl_bb[op] & e->p_takes[sd];
  bbHanging |= bbThreatened;      // piece attacked by our pawn isn't well defended
  bbHanging &= e->all_att[sd];    // hanging piece has to be attacked
  bbHanging &= ~p->Pawns(op);     // currently we don't evaluate threats against pawns

  U64 bbDefended = p->cl_bb[op] & e->all_att[op];
  bbDefended &= e->ev_att[sd];    // N, B, R attacks (pieces attacked by pawns are scored as hanging)
  bbDefended &= ~e->p_takes[sd];  // no defense against pawn attack
  bbDefended &= ~p->Pawns(op);    // currently we don't evaluate threats against pawns

  // hanging pieces (attacked and undefended)

  while (bbHanging) {
    sq = BB.PopFirstBit(&bbHanging);
    pc = TpOnSq(p, sq);
    sc = tp_value[pc] / 64;
    Add(e, sd, 10 + sc, 18 + sc);
  }

  // defended pieces under attack

  while (bbDefended) {
    sq = BB.PopFirstBit(&bbDefended);
    pc = TpOnSq(p, sq);
    sc = tp_value[pc] / 96;
    Add(e, sd, 5 + sc, 9 + sc);
  }
}

int cEngine::Evaluate(POS *p, eData *e) {

  // try retrieving score from per-thread eval hashtable

  int addr = p->hash_key % EVAL_HASH_SIZE;

  if (EvalTT[addr].key == p->hash_key) {
    int sc = EvalTT[addr].score;
    return p->side == WC ? sc : -sc;
  }

  // clear eval data  

  e->mg[WC] = p->mg_sc[WC];
  e->mg[BC] = p->mg_sc[BC];
  e->eg[WC] = p->eg_sc[WC];
  e->eg[BC] = p->eg_sc[BC];

  // init helper bitboards

  e->p_takes[WC] = BB.GetWPControl(p->Pawns(WC));
  e->p_takes[BC] = BB.GetBPControl(p->Pawns(BC));
  e->p_can_take[WC] = BB.FillNorth(e->p_takes[WC]);
  e->p_can_take[BC] = BB.FillSouth(e->p_takes[BC]);
  e->two_pawns_take[WC] = BB.GetDoubleWPControl(p->Pawns(WC));
  e->two_pawns_take[BC] = BB.GetDoubleBPControl(p->Pawns(BC));
  e->all_att[WC] = e->p_takes[WC] | BB.KingAttacks(KingSq(p, WC));
  e->all_att[BC] = e->p_takes[BC] | BB.KingAttacks(KingSq(p, BC));
  e->ev_att[WC] = 0ULL;
  e->ev_att[BC] = 0ULL;

  // run all the evaluation subroutines

  EvaluateMaterial(p, e, WC);
  EvaluateMaterial(p, e, BC);
  EvaluatePieces(p, e, WC);
  EvaluatePieces(p, e, BC);
  ScorePawnStruct(p, e);
  EvaluatePassers(p, e, WC);
  EvaluatePassers(p, e, BC);
  EvaluatePatterns(p, e);
  ScoreUnstoppable(e, p);
  ScoreHanging(p, e, WC);
  ScoreHanging(p, e, BC);
  Add(e, p->side, 14, 7); // tempo bonus

  e->mg[WC] += e->mg_pawns[WC];
  e->mg[BC] += e->mg_pawns[BC];
  e->eg[WC] += e->eg_pawns[WC];
  e->eg[BC] += e->eg_pawns[BC];
  
  // Interpolate between midgame and endgame scores

  int score = Interpolate(p, e);

  // Material imbalance evaluation

  int minorBalance = p->cnt[WC][N] - p->cnt[BC][N] + p->cnt[WC][B] - p->cnt[BC][B];
  int majorBalance = p->cnt[WC][R] - p->cnt[BC][R] + 2 * p->cnt[WC][Q] - 2 * p->cnt[BC][Q];

  int x = Max(majorBalance + 4, 0);
  if (x > 8) x = 8;

  int y = Max(minorBalance + 4, 0);
  if (y > 8) y = 8;

  score += imbalance[x][y];

  // Ensure that returned value doesn't exceed mate score

  score = Clip(score, MAX_EVAL);

  // Decrease score for drawish endgames

  int draw_factor = 64;
  if (score > 0) draw_factor = GetDrawFactor(p, WC);
  if (score < 0) draw_factor = GetDrawFactor(p, BC);
  score = (score * draw_factor) / 64;

  // Save eval score in the evaluation hash table

  EvalTT[addr].key = p->hash_key;
  EvalTT[addr].score = score;

  // Return score relative to the side to move

  return p->side == WC ? score : -score;
}
