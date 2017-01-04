#include <stdio.h>
#include <string.h>
#include <math.h>
#include "rodent.h"

int good_hist = 24576;
int search_skill = 100;
int razor_margin[5] = { 0, 300, 360, 420, 480 };
int fut_margin[7] = { 0, 100, 150, 200, 250, 300, 350 };
double lmr_size[2][MAX_PLY][MAX_MOVES];

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
  new_pos->phase = old_pos->phase;

  new_pos->side = old_pos->side;
  new_pos->c_flags = old_pos->c_flags;
  new_pos->ep_sq = old_pos->ep_sq;
  new_pos->rev_moves = old_pos->rev_moves;
  new_pos->head = old_pos->head;
  new_pos->key = old_pos->key;

  for (int i = 0; i < 256; i++) {
    new_pos->rep_list[i] = old_pos->rep_list[i];
  }
}

void cEngine::Think(POS *p, int *pv) {

  POS curr[1];
  pv[0] = 0;
  pv[1] = 0;
  int move = 0;
  int score, mv_type;
  MOVES m[1];

  CopyPos(p, curr);

  // Safeguard against pathological situations at ultra-short
  // time control when search does not return a move.
  // First, copy move from transposition table to pv slot.
  // If that fails, just pick the first generated move.

  TransRetrieve(curr->key, &move, &score, -INF, INF, 1, 0);
  if (Legal(curr, move)) pv[0] = move;
  else {
    InitMoves(p, m, move, 0);
    while ((move = NextMove(m, &mv_type))) {
      if (Legal(curr, move)) {
        pv[0] = move;
        break;
      }
    }
  }

  // Conduct real search that will overwrite 

  ClearHist();
  local_nodes = 0;
  Iterate(curr, pv);
}

void cEngine::Iterate(POS *p, int *pv) {

  int val = 0, cur_val = 0;

  for (root_depth = 1; root_depth <= search_depth; root_depth++) {
    printf("info depth %d\n", root_depth);

    if (search_skill > 6) cur_val = Widen(p, root_depth, pv, val);
    else                  cur_val = Search(p, 0, -INF, INF, root_depth, 0, pv);

    // Abort search on finding checkmate score

    if (cur_val > MAX_EVAL || cur_val < -MAX_EVAL) {
      int maxMateDepth = (MATE - Abs(cur_val) + 1) + 1;
      maxMateDepth *= 4;
      maxMateDepth /= 3;
      if (maxMateDepth <= root_depth) break;
    }

	// Don't start a new iteration if 90% of time has elapsed

	if (GetMS() - start_time >= (move_time * 9) / 10)
      abort_search = true;

    if (abort_search) break;
    else depth_reached = root_depth;

    val = cur_val;
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
      cur_val = Search(p, 0, alpha, beta, depth, 0, pv);
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

  int best, score, null_score, move, new_depth, reduction, new_pv[MAX_PLY];
  int is_pv = (alpha != beta - 1);
  int fl_check, fl_prunable_node, fl_prunable_move, mv_type, mv_hist_score;
  int fl_futility = 0;
  int eval = 0;
  int mv_tried = 0;
  int mv_list[256];
  int quiet_tried = 0;
  MOVES m[1];
  UNDO u[1];
  eData e;

  // QUIESCENCE SEARCH ENTRY POINT

  if (depth <= 0)
    return Quiesce(p, ply, alpha, beta, pv);

  // QUICK EXIT

  nodes++;
  local_nodes++;
  CheckTimeout(ply, pv);
  if (abort_search) return 0;
  if (ply) *pv = 0;
  if (IsDraw(p) && ply) return 0;

  // TRANSPOSITION TABLE READ

  move = 0;

  if (TransRetrieve(p->key, &move, &score, alpha, beta, depth, ply)) {
    if (score > beta && move > 0) UpdateHist(p, move, depth, ply);
    if (!is_pv && search_skill > 0) return score;
  }

  // SAFEGUARD AGAINST REACHING MAX_PLY LIMIT

  if (ply >= MAX_PLY - 1)
    return Evaluate(p, &e);

  fl_check = InCheck(p);

  // CAN WE PRUNE THIS NODE?

  fl_prunable_node = !fl_check 
                   && !is_pv 
                   && alpha > -MAX_EVAL
                   && beta < MAX_EVAL;

  // GET NODE EVAL IF WE EXPECT TO PRUNE OR REDUCE

  if (fl_prunable_node
  && (!was_null || depth <= 6))
    eval = Evaluate(p, &e);

  // BETA PRUNUNG / STATIC NULL MOVE

  if (fl_prunable_node
  && search_skill > 4
  && depth <= 3
  && !was_null) {
    int sc = eval - 120 * depth;
    if (sc > beta) return sc;
  }

  // NULL MOVE

  if (fl_prunable_node
  && search_skill > 1
  && depth > 1
  && !was_null
  && MayNull(p)
  && beta <= eval) {

    new_depth = depth - 3 - depth / 4;

    if (TransRetrieve(p->key, &move, &null_score, alpha, beta, new_depth, ply)) {
      if (null_score < beta) goto avoid_null;
    }

    DoNull(p, u);
    score = -Search(p, ply + 1, -beta, -beta + 1, new_depth, 1, new_pv);
    UndoNull(p, u);
    if (abort_search) return 0;
    if (score >= beta) {
      TransStore(p->key, 0, score, LOWER, depth, ply);
      return score;
    }
  }

  avoid_null:

  // RAZORING

  if (fl_prunable_node
  && search_skill > 5
  && !was_null
  && !move
  && !(PcBb(p, p->side, P) & bbRelRank[p->side][RANK_7]) // no pawns to promote in one move
  && depth <= 4) {
    int threshold = beta - razor_margin[depth];
    if (eval < threshold) {
      score = Quiesce(p, ply, alpha, beta, pv);
      if (score < threshold) return score;
    }
  }

  // MAIN LOOP

  best = -INF;
  InitMoves(p, m, move, ply);
  while ((move = NextMove(m, &mv_type))) {

    // SET FUTILITY PRUNING FLAG BEFORE TRYING THE FIRST QUIET MOVE

    if (mv_type == MV_NORMAL 
    && quiet_tried == 0) {
      if (fl_prunable_node
      && depth <= 6) {
        if (eval + fut_margin[depth] < beta) fl_futility = 1;
      }
    }

    // MAKE MOVE AND GATHER MOVE STATISTICS

    mv_hist_score = history[p->pc[Fsq(move)]][Tsq(move)];
    DoMove(p, move, u);
    if (Illegal(p)) { UndoMove(p, move, u); continue; }
    mv_list[mv_tried] = move;
    mv_tried++;
    if (mv_type == MV_NORMAL) quiet_tried++;

    // CAN WE PRUNE THIS MOVE?

    fl_prunable_move = !InCheck(p)
                    && (mv_type == MV_NORMAL)
                    && (mv_hist_score < good_hist);

    // SET NEW DEPTH

    new_depth = depth - 1 + InCheck(p);

    // FUTILITY PRUNING

    if (fl_futility
    &&  fl_prunable_move
    &&  mv_tried > 1) {
      UndoMove(p, move, u); continue;
    }

    // LATE MOVE PRUNING

    if (fl_prunable_node
    && search_skill > 3
    && depth < 4
    && quiet_tried > 3 * depth
    && fl_prunable_move) {
      UndoMove(p, move, u); continue;
    }

    // LMR 1: NORMAL MOVES
    // TODO: alpha/beta <> MAX_EVAL conditions

    reduction = 0;

    if (depth > 2
    && mv_tried > 3
    && search_skill > 2
    && !fl_check
    && lmr_size[is_pv][depth][mv_tried] > 0
    && fl_prunable_move
    && (mv_hist_score < good_hist)
    && MoveType(move) != CASTLE) {
      reduction = lmr_size[is_pv][depth][mv_tried];

      if (new_depth - reduction > 2
      && mv_hist_score < 0)
        reduction += 1;

      new_depth = new_depth - reduction;
    }

    research:

    // PVS

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

    // UNMAKE MOVE

    UndoMove(p, move, u);
    if (abort_search) return 0;

    // BETA CUTOFF

    if (score >= beta) {
      UpdateHist(p, move, depth, ply);
      for (int mv = 0; mv < mv_tried; mv++)
        DecreaseHist(p, mv_list[mv], depth);
      TransStore(p->key, move, score, LOWER, depth, ply);

      // At root, change the best move and show the new pv

      if (!ply) {
        BuildPv(pv, new_pv, move);
        DisplayPv(score, pv);
      }

      return score;
    }

    // BEST MOVE CHANGE

    if (score > best) {
      best = score;
      if (score > alpha) {
        alpha = score;
        BuildPv(pv, new_pv, move);
        if (!ply) DisplayPv(score, pv);
      }
    }
  }

  // RETURN CORRECT CHECKMATE/STALEMATE SCORE

  if (best == -INF)
    return InCheck(p) ? -MATE + ply : 0;

  // SAVE RESULT TO TRANSPOSITION TABLE

  if (*pv) {
    UpdateHist(p, *pv, depth, ply);
    for (int mv = 0; mv < mv_tried; mv++)
      DecreaseHist(p, mv_list[mv], depth);
    TransStore(p->key, *pv, best, EXACT, depth, ply);
  } else
    TransStore(p->key, 0, best, UPPER, depth, ply);

  return best;
}

int cEngine::Quiesce(POS *p, int ply, int alpha, int beta, int *pv) {

  int best, score, move, new_pv[MAX_PLY];
  MOVES m[1];
  UNDO u[1];
  eData e;

  nodes++;
  local_nodes++;
  CheckTimeout(ply, pv);
  if (abort_search) return 0;
  *pv = 0;
  if (IsDraw(p)) return 0;

  if (ply >= MAX_PLY - 1) return Evaluate(p, &e);

  best = Evaluate(p, &e);
  if (best >= beta) return best;
  if (best > alpha) alpha = best;

  InitCaptures(p, m);
  while ((move = NextCapture(m))) {
    DoMove(p, move, u);
    if (Illegal(p)) { UndoMove(p, move, u); continue; }
    score = -Quiesce(p, ply + 1, -beta, -alpha, new_pv);
    UndoMove(p, move, u);
    if (abort_search) return 0;
    if (score >= beta)
      return score;
    if (score > best) {
      best = score;
      if (score > alpha) {
        alpha = score;
        BuildPv(pv, new_pv, move);
      }
    }
  }
  return best;
}

int cEngine::IsDraw(POS *p) {

  // DRAW BY 50 MOVE RULE

  if (p->rev_moves > 100) return 1;

  // DRAW BY REPETITION

  for (int i = 4; i <= p->rev_moves; i += 2)
    if (p->key == p->rep_list[p->head - i])
      return 1;

  // DRAW BY INSUFFICIENT MATERIAL

 if (p->cnt[WC][Q] + p->cnt[BC][Q] + p->cnt[WC][R] + p->cnt[BC][R] == 0) {

    if (!Illegal(p)) {
      if (p->cnt[WC][P] + p->cnt[BC][P] == 0) {
        if (p->cnt[WC][N] + p->cnt[BC][N] + p->cnt[WC][B] + p->cnt[BC][B] < 2) return 1; // KK, KmK
      }
    }
  }

  // DEFAULT: NO DRAW

  return 0;
}

int GetNps(int elapsed) {

  U64 nps = 0;
  if (elapsed) nps = ((U64)nodes * 1000) / elapsed;
  return (int)nps;
}

void cEngine::DisplayPv(int score, int *pv) {

  char *type, pv_str[512];
  int elapsed = GetMS() - start_time;
  int nps = GetNps(elapsed);

  type = "mate";
  if (score < -MAX_EVAL)
    score = (-MATE - score) / 2;
  else if (score > MAX_EVAL)
    score = (MATE - score + 1) / 2;
  else
    type = "cp";
  PvToStr(pv, pv_str);
  printf("info depth %d time %d nodes %d nps %d score %s %d pv %s\n",
      root_depth, elapsed, nodes, nps, type, score, pv_str);
}

void cEngine::CheckTimeout(int ply, int *pv) {

  char command[80];

  if (pv[0] == 0) return; // search has to find a move

  if ((local_nodes & 31 || root_depth == 1)
  && ply > 4) return;

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
