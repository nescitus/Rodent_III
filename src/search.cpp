#include <stdio.h>
#include <string.h>
#include <math.h>
#include "skeleton.h"

int razor_margin[5] = { 0, 300, 360, 420, 480 };
int fut_margin[7] = { 0, 100, 150, 200, 250, 300, 350 };
double lmr_size[2][MAX_PLY][MAX_MOVES];
const int hist_limit = 24576;

void cParam::InitAsymmetric(POS * p) {

  prog_side = p->side;

  if (prog_side == WC) {
    sd_att[WC] = own_att;
    sd_att[BC] = opp_att;
    sd_mob[WC] = own_mob;
    sd_mob[BC] = opp_mob;
  } else {
    sd_att[BC] = own_att;
    sd_att[WC] = opp_att;
    sd_mob[BC] = own_mob;
    sd_mob[WC] = opp_mob;
  }
}

void cGlobals::ClearData(void) {
  // TODO: clear node count
}

void InitSearch(void) {

  // Set depth of late move reduction using modified Stockfish formula

  for (int dp = 0; dp < MAX_PLY; dp++)
    for (int mv = 0; mv < MAX_MOVES; mv++) {

      double r = log((double)dp) * log((double)Min(mv, 63)) / 2;
      if (r < 0.80) r = 0; // TODO: test without

      lmr_size[0][dp][mv] = r;             // zero window node
      lmr_size[1][dp][mv] = Max(r - 1, 0); // principal variation node

      for (int node = 0; node <= 1; node++) {
        if (lmr_size[node][dp][mv] < 1) lmr_size[node][dp][mv] = 0; // ultra-small reductions make no sense

        if (lmr_size[node][dp][mv] > dp - 1) // reduction cannot exceed actual depth
          lmr_size[node][dp][mv] = dp - 1;
     }
  }
}

void CopyPos(POS * old_pos, POS * new_pos) {

  new_pos->cl_bb[WC] = old_pos->cl_bb[WC];
  new_pos->cl_bb[BC] = old_pos->cl_bb[BC];

  for (int tp = 0; tp < 6; tp++) {
    new_pos->tp_bb[tp] = old_pos->tp_bb[tp];
    new_pos->cnt[WC][tp] = old_pos->cnt[WC][tp];
    new_pos->cnt[BC][tp] = old_pos->cnt[BC][tp];
  }

  for (int sq = 0; sq < 64; sq++) {
    new_pos->pc[sq] = old_pos->pc[sq];
  }

  new_pos->king_sq[WC] = old_pos->king_sq[WC];
  new_pos->king_sq[BC] = old_pos->king_sq[BC];

  new_pos->mg_sc[WC] = old_pos->mg_sc[WC];
  new_pos->mg_sc[BC] = old_pos->mg_sc[BC];
  new_pos->eg_sc[WC] = old_pos->eg_sc[WC];
  new_pos->eg_sc[BC] = old_pos->eg_sc[BC];

  new_pos->side = old_pos->side;
  new_pos->c_flags = old_pos->c_flags;
  new_pos->ep_sq = old_pos->ep_sq;
  new_pos->rev_moves = old_pos->rev_moves;
  new_pos->head = old_pos->head;
  new_pos->hash_key = old_pos->hash_key;
  new_pos->pawn_key = old_pos->pawn_key;
  new_pos->phase = old_pos->phase;

  for (int i = 0; i < 256; i++) {
    new_pos->rep_list[i] = old_pos->rep_list[i];
  }
}

void cEngine::Think(POS *p, int *pv) {

  POS curr[1];
  pv[0] = 0;
  pv[1] = 0;

  CopyPos(p, curr);
  ClearHist();

  // get a fallback move

  if (thread_id == 0)
    Search(curr, 0, -INF, INF, 1, 0, pv);

  Iterate(curr, pv);
}

void cEngine::Iterate(POS *p, int *pv) {

  int cur_val = 0;
  int offset = 0;
  if (thread_id == 1 || thread_id == 3) offset = 1;

  for (root_depth = 1; root_depth <= search_depth; root_depth++) {
    printf("info depth %d\n", root_depth);
    cur_val = Widen(p, root_depth + offset, pv, cur_val);
    if (abort_search) break;
    dp_completed = root_depth + offset;
  }
}

int cEngine::Widen(POS *p, int depth, int * pv, int lastScore) {
  
  // Function performs aspiration search, progressively widening the window.
  // Code structure modelled after Senpai 1.0.

  int cur_val = lastScore, alpha, beta;

  if (depth > 6 && lastScore < MAX_EVAL) {
    for (int margin = 10; margin < 500; margin *= 2) {
      alpha = lastScore - margin;
      beta  = lastScore + margin;
      cur_val = Search(p, 0, alpha, beta, depth, 0,  pv);
      if (abort_search) break;
      if (cur_val > alpha && cur_val < beta) 
      return cur_val;                // we have finished within the window
      if (cur_val > MAX_EVAL) break; // verify mate searching with infinite bounds
    }
  }

  cur_val = Search(p, 0, -INF, INF, depth, 0, pv); // full window search
  return cur_val;
}

int cEngine::Search(POS *p, int ply, int alpha, int beta, int depth, int was_null, int *pv) {

  int best, score, null_score, move, new_depth, new_pv[MAX_PLY];
  int mv_type, fl_check, reduction;
  int is_pv = (alpha != beta - 1);
  int null_refutation = -1, ref_sq = -1;
  int mv_tried = 0;
  int mv_played[MAX_MOVES];
  int quiet_tried = 0;
  int fl_futility = 0;
  int mv_hist_score = 0;
  MOVES m[1];
  UNDO u[1];
  eData e;

  // QUIESCENCE SEARCH ENTRY POINT

  if (depth <= 0) return QuiesceChecks(p, ply, alpha, beta, pv);

  // EARLY EXIT AND NODE INITIALIZATION

  nodes++;
  local_nodes++;
  CheckTimeout(ply);
  if (abort_search) return 0;
  if (ply) *pv = 0;
  if (IsDraw(p) && ply) return 0;
  move = 0;

  // MATE DISTANCE PRUNING

  if (ply) {
    int checkmatingScore = MATE - ply;
    if (checkmatingScore < beta) {
      beta = checkmatingScore;
      if (alpha >= checkmatingScore)
        return alpha;
    }

    int checkmatedScore = -MATE + ply;
    if (checkmatedScore > alpha) {
      alpha = checkmatedScore;
      if (beta <= checkmatedScore)
        return beta;
    }
  }

  // RETRIEVE MOVE FROM TRANSPOSITION TABLE

  if (TransRetrieve(p->hash_key, &move, &score, alpha, beta, depth, ply)) {
    if (score >= beta) UpdateHistory(p, move, depth, ply);
    if (!is_pv) return score;
  }

  // SAFEGUARD AGAINST REACHING MAX PLY LIMIT

  if (ply >= MAX_PLY - 1) return Evaluate(p, &e);

  fl_check = InCheck(p);

  // CAN WE PRUNE THIS NODE?

  int fl_prunable_node = !fl_check 
                      && !is_pv 
                      && alpha > -MAX_EVAL 
                      && beta < MAX_EVAL;

  // GET EVAL SCORE IF NEEDED FOR PRUNING/REDUCTION DECISIONS

  int eval = 0;
  if (fl_prunable_node
  && (!was_null || depth <= 6)) eval = Evaluate(p, &e);

  // BETA PRUNING / STATIC NULL MOVE

  if (fl_prunable_node
  && depth <= 3
  && !was_null) {
    int sc = eval - 120 * depth;
    if (sc > beta) return sc;
  }

  // NULL MOVE

  if (depth > 1 
  && !was_null
  && fl_prunable_node
  && MayNull(p)
  && eval >= beta) {
    new_depth = depth - ((823 + 67 * depth) / 256); // simplified Stockfish formula

    // omit null move search if normal search to the same depth wouldn't exceed beta
    // (sometimes we can check it for free via hash table)

    if (TransRetrieve(p->hash_key, &move, &null_score, alpha, beta, new_depth, ply)) {
      if (null_score < beta) goto avoid_null;
    }

    DoNull(p, u);
    if (new_depth <= 0) score = -QuiesceChecks(p, ply + 1, -beta, -beta + 1, new_pv);
    else                score = -Search(p, ply + 1, -beta, -beta + 1, new_depth, 1, new_pv);

    TransRetrieve(p->hash_key, &null_refutation, &null_score, alpha, beta, depth, ply);
    if (null_refutation > 0) ref_sq = Tsq(null_refutation);
    UndoNull(p, u);

    if (abort_search) return 0;
    if (score >= beta) {

      // verification search

      if (new_depth > 6) 
      score = Search(p, ply, alpha, beta, new_depth-5, 1, pv);
      if (abort_search) return 0;
      if (score >= beta) return score;
    }
  }

  avoid_null:

  // RAZORING BASED ON TOGA II 3.0

  if (fl_prunable_node
  && !move
  && !was_null
  && !(p->Pawns(p->side) & bbRelRank[p->side][RANK_7]) // no pawns to promote in one move
  && depth <= 4) {
    int threshold = beta - razor_margin[depth];

    if (eval < threshold) {
      score = QuiesceChecks(p, ply, alpha, beta, pv);
      if (score < threshold) return score;
    }
  } // end of razoring code 

  // INTERNAL ITERATIVE DEEPENING

  if (is_pv && !fl_check && !move && depth > 6) {
     Search(p, ply, alpha, beta, depth-2, 0, pv);
     TransRetrieve(p->hash_key, &move, &score, alpha, beta, depth-2, ply);
  }

  // PREPARE FOR MAIN SEARCH

  best = -INF;
  InitMoves(p, m, move, ref_sq, ply);

  // MAIN LOOP

  while ((move = NextMove(m, &mv_type))) {

    // SET FUTILITY PRUNING FLAG
    // before the first applicable move is tried

    if (mv_type == MV_NORMAL 
    && quiet_tried == 0
    && fl_prunable_node
    && depth <= 6) {
      if (eval + fut_margin[depth] < beta) fl_futility = 1;
    }

    // MAKE MOVE    

    mv_hist_score = history[p->pc[Fsq(move)]][Tsq(move)];
    DoMove(p, move, u);
    if (Illegal(p)) { UndoMove(p, move, u); continue; }

    // GATHER INFO ABOUT THE MOVE

    mv_played[mv_tried] = move;
    mv_tried++;
    if (mv_type == MV_NORMAL) quiet_tried++;

    // SET NEW SEARCH DEPTH

    new_depth = depth - 1;
    if (is_pv || depth < 9) new_depth += InCheck(p); // check extension

    // FUTILITY PRUNING

    if (fl_futility
    && !InCheck(p)
    && mv_hist_score < hist_limit
    && (mv_type == MV_NORMAL)
    &&  mv_tried > 1) {
      UndoMove(p, move, u); continue;
    }

    // LATE MOVE PRUNING

    if (fl_prunable_node
    && depth < 4
    && quiet_tried > 3 * depth
    && !InCheck(p)
    && mv_hist_score < hist_limit
    && mv_type == MV_NORMAL) {
      UndoMove(p, move, u); continue;
    }

    // LMR 1: NORMAL MOVES

    reduction = 0;

    if (depth > 2
    && mv_tried > 3
    && !fl_check
    && !InCheck(p)
    && lmr_size[is_pv][depth][mv_tried] > 0
    && mv_type == MV_NORMAL
    && mv_hist_score < hist_limit
    && MoveType(move) != CASTLE) {
      reduction = lmr_size[is_pv][depth][mv_tried];

      // increase reduction on bad history score

      if (mv_hist_score < 0
      && new_depth - reduction > 2)
        reduction++;

      new_depth = new_depth - reduction;
    }

    // LMR 2: MARGINAL REDUCTION OF BAD CAPTURES

    if (depth > 2
    && mv_tried > 6
    && alpha > -MAX_EVAL && beta < MAX_EVAL
    && !fl_check
    && !InCheck(p)
    && (mv_type == MV_BADCAPT)
    && !is_pv) {
      reduction = 1;
      new_depth -= reduction;
    }

    research:

    // PRINCIPAL VARIATION SEARCH

    if (best == -INF)
      score = -Search(p, ply + 1, -beta, -alpha, new_depth, 0, new_pv);
    else {
      score = -Search(p, ply + 1, -alpha - 1, -alpha, new_depth, 0, new_pv);
      if (!abort_search && score > alpha && score < beta)
        score = -Search(p, ply + 1, -beta, -alpha, new_depth, 0, new_pv);
    }

    // DON'T REDUCE A MOVE THAT SCORED ABOVE ALPHA

    if (score > alpha && reduction) {
      new_depth = new_depth + reduction;
      reduction = 0;
      goto research;
    }

    // UNDO MOVE

    UndoMove(p, move, u);
    if (abort_search) return 0;

    // BETA CUTOFF

    if (score >= beta) {
      if (!fl_check) {
        UpdateHistory(p, move, depth, ply);
        for (int mv = 0; mv < mv_tried; mv++) {
           DecreaseHistory(p, mv_played[mv], depth);
        }
      }
      TransStore(p->hash_key, move, score, LOWER, depth, ply);

      // At root, change the best move and show the new pv

      if (!ply) {
        BuildPv(pv, new_pv, move);
        DisplayPv(score, pv);
      }

      return score;
    }

    // NEW BEST MOVE

    if (score > best) {
      best = score;
      if (score > alpha) {
        alpha = score;
        BuildPv(pv, new_pv, move);
        if (!ply) DisplayPv(score, pv);
      }
    }

  } // end of main loop

  // RETURN CORRECT CHECKMATE/STALEMATE SCORE
  
  if (best == -INF)
    return InCheck(p) ? -MATE + ply : 0;

  // SAVE RESULT IN THE TRANSPOSITION TABLE

  if (*pv) {
    if (!fl_check) {
      UpdateHistory(p, *pv, depth, ply);
      for (int mv = 0; mv < mv_tried; mv++) {
        DecreaseHistory(p, mv_played[mv], depth);
      }
    }
    TransStore(p->hash_key, *pv, best, EXACT, depth, ply);
  } else
    TransStore(p->hash_key, 0, best, UPPER, depth, ply);

  return best;
}

int cEngine::IsDraw(POS *p) {

  // Draw by 50 move rule

  if (p->rev_moves > 100) return 1;

  // Draw by repetition

  for (int i = 4; i <= p->rev_moves; i += 2)
    if (p->hash_key == p->rep_list[p->head - i]) return 1;

  // With no major pieces on the board, we have some heuristic draws to consider

  if (p->cnt[WC][Q] + p->cnt[BC][Q] + p->cnt[WC][R] + p->cnt[BC][R] == 0) {

    // Draw by insufficient material (bare kings or Km vs K)

    if (!Illegal(p)) {
      if (p->cnt[WC][P] + p->cnt[BC][P] == 0) {
        if (p->cnt[WC][N] + p->cnt[BC][N] + p->cnt[WC][B] + p->cnt[BC][B] <= 1) return 1; // KK, KmK
      }
    }

    // Trivially drawn KPK endgames

    if (p->cnt[WC][B] + p->cnt[BC][B] + p->cnt[WC][N] + p->cnt[BC][N] == 0) {
      if (p->cnt[WC][P] + p->cnt[BC][P] == 1) {
        if (p->cnt[WC][P] == 1 ) return KPKdraw(p, WC); // exactly one white pawn
        if (p->cnt[BC][P] == 1 ) return KPKdraw(p, BC); // exactly one black pawn
      }
    } // pawns only
  }


  // Default: no draw

  return 0;
}

int cEngine::KPKdraw(POS *p, int sd) {

  int op = Opp(sd);
  U64 bbPawn = p->Pawns(sd);
  U64 bbStrongKing = p->Kings(sd);
  U64 bbWeakKing = p->Kings(op);

  // opposition through a pawn

  if (p->side == sd
  && (bbWeakKing & BB.ShiftFwd(bbPawn, sd))
  && (bbStrongKing & BB.ShiftFwd(bbPawn, op))
  ) return 1;
  
  // weaker side can create opposition through a pawn in one move

  if (p->side == op
  && (BB.KingAttacks(p->king_sq[op]) & BB.ShiftFwd(bbPawn, sd))
  && (bbStrongKing & BB.ShiftFwd(bbPawn, op))
  ) if (!Illegal(p)) return 1;

  // opposition next to a pawn
  
  if (p->side == sd
  && (bbStrongKing & BB.ShiftSideways(bbPawn))
  && (bbWeakKing & BB.ShiftFwd(BB.ShiftFwd(bbStrongKing,sd) ,sd)) 
  ) return 1;

  // TODO: pawn checks king

  return 0;
}

U64 GetNps(int elapsed) {

  U64 nps = 0;
  if (elapsed) nps = (nodes * 1000) / elapsed;
  return nps;
}

void cEngine::DisplayPv(int score, int *pv)
{
  char *type, pv_str[512];
  int elapsed = GetMS() - start_time;
  U64 nps = GetNps(elapsed);

  type = "mate";
  if (score < -MAX_EVAL)
    score = (-MATE - score) / 2;
  else if (score > MAX_EVAL)
    score = (MATE - score + 1) / 2;
  else
    type = "cp";

  PvToStr(pv, pv_str);

#if defined _WIN32 || defined _WIN64 
  printf("info depth %d time %d nodes %I64d nps %I64d score %s %d pv %s\n",
      root_depth, elapsed, nodes, nps, type, score, pv_str);
#else
  printf("info depth %d time %d nodes %lld nps %lld score %s %d pv %s\n",
      root_depth, elapsed, nodes, nps, type, score, pv_str);
#endif
}

void cEngine::CheckTimeout(int ply)
{
  char command[80];

  if ( (local_nodes & 4095 || root_depth == 1) 
  && ply > 3)
    return;

  if (InputAvailable()) {
    ReadLine(command, sizeof(command));
    if (strcmp(command, "stop") == 0)
      abort_search = 1;
    else if (strcmp(command, "ponderhit") == 0)
      pondering = 0;
  }
  if (!pondering && move_time >= 0 && GetMS() - start_time >= move_time)
    abort_search = 1;
}
