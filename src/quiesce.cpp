#include "skeleton.h"

int cEngine::QuiesceChecks(POS *p, int ply, int alpha, int beta, int *pv) {

  int best, score, null_score, move, new_depth, new_pv[MAX_PLY];
  int mv_type, fl_check;
  int is_pv = (alpha != beta - 1);
  MOVES m[1];
  UNDO u[1];
  eData e;

  if (InCheck(p)) return QuiesceFlee(p, ply, alpha, beta, pv);

  // EARLY EXIT AND NODE INITIALIZATION

  nodes++;
  local_nodes++;
  CheckTimeout(ply);
  if (abort_search) return 0;
  *pv = 0;
  if (IsDraw(p) && ply) return 0;
  move = 0;

  best = Evaluate(p, &e);
  if (best >= beta) return best;
  if (best > alpha) alpha = best;

  // RETRIEVE MOVE FROM TRANSPOSITION TABLE

  if (TransRetrieve(p->hash_key, &move, &score, alpha, beta, 0, ply)) {
    if (score >= beta) UpdateHistory(p, move, 1, ply);
    if (!is_pv) return score;
  }

  // SAFEGUARD AGAINST REACHING MAX PLY LIMIT

  if (ply >= MAX_PLY - 1) return Evaluate(p, &e);

  fl_check = InCheck(p);

  // PREPARE FOR MAIN SEARCH

  InitMoves(p, m, move, -1, ply);

  // MAIN LOOP

  while ((move = NextSpecialMove(m, &mv_type))) {

    // MAKE MOVE    

    DoMove(p, move, u);
    if (Illegal(p)) { UndoMove(p, move, u); continue; }

	if (mv_type == MV_NORMAL && !InCheck(p)) { UndoMove(p, move, u); continue; }
	//if (mv_type == MV_BADCAPT & !InCheck(p)) { UndoMove(p, move, u); continue; }
	if (mv_type == MV_KILLER && !InCheck(p)) { UndoMove(p, move, u); continue; }

	// SET NEW SEARCH DEPTH

    new_depth = InCheck(p);

	// somehow calling Search() here works better than calling Quiesce()

      score = -Quiesce(p, ply + 1, -beta, -alpha, new_pv);

	// UNDO MOVE

    UndoMove(p, move, u);
    if (abort_search) return 0;

	// BETA CUTOFF

    if (score >= beta) {
      TransStore(p->hash_key, move, score, LOWER, 0, ply);
      return score;
    }

	// NEW BEST MOVE

    if (score > best) {
      best = score;
      if (score > alpha) {
        alpha = score;
        BuildPv(pv, new_pv, move);
      }
    }

  } // end of main loop

  // RETURN CORRECT CHECKMATE/STALEMATE SCORE
  
  if (best == -INF)
    return InCheck(p) ? -MATE + ply : 0;

  // SAVE RESULT IN THE TRANSPOSITION TABLE

  if (*pv) TransStore(p->hash_key, *pv, best, EXACT, 0, ply);
  else     TransStore(p->hash_key,   0, best, UPPER, 0, ply);

  return best;
}

int cEngine::QuiesceFlee(POS *p, int ply, int alpha, int beta, int *pv) {

  int best, score, null_score, move, new_depth, new_pv[MAX_PLY];
  int mv_type, fl_check;
  int is_pv = (alpha != beta - 1);
  MOVES m[1];
  UNDO u[1];
  eData e;

  // EARLY EXIT AND NODE INITIALIZATION

  nodes++;
  local_nodes++;
  CheckTimeout(ply);
  if (abort_search) return 0;
  *pv = 0;
  if (IsDraw(p) && ply) return 0;
  move = 0;

  // RETRIEVE MOVE FROM TRANSPOSITION TABLE

  if (TransRetrieve(p->hash_key, &move, &score, alpha, beta, 0, ply)) {
    if (score >= beta) UpdateHistory(p, move, 1, ply);
    if (!is_pv) return score;
  }

  // SAFEGUARD AGAINST REACHING MAX PLY LIMIT

  if (ply >= MAX_PLY - 1) return Evaluate(p, &e);

  fl_check = InCheck(p);

  // PREPARE FOR MAIN SEARCH

  best = -INF;
  InitMoves(p, m, move, -1, ply);

  // MAIN LOOP

  while ((move = NextMove(m, &mv_type))) {

    // MAKE MOVE    

    DoMove(p, move, u);
    if (Illegal(p)) { UndoMove(p, move, u); continue; }

	// SET NEW SEARCH DEPTH

    new_depth = InCheck(p);

	score = -Quiesce(p, ply + 1, -beta, -alpha, new_pv);

	// UNDO MOVE

    UndoMove(p, move, u);
    if (abort_search) return 0;

	// BETA CUTOFF

    if (score >= beta) {
      TransStore(p->hash_key, move, score, LOWER, 0, ply);
      return score;
    }

	// NEW BEST MOVE

    if (score > best) {
      best = score;
      if (score > alpha) {
        alpha = score;
        BuildPv(pv, new_pv, move);
      }
    }

  } // end of main loop

  // RETURN CORRECT CHECKMATE/STALEMATE SCORE
  
  if (best == -INF)
    return InCheck(p) ? -MATE + ply : 0;

  // SAVE RESULT IN THE TRANSPOSITION TABLE

  if (*pv) TransStore(p->hash_key, *pv, best, EXACT, 0, ply);
  else     TransStore(p->hash_key,   0, best, UPPER, 0, ply);

  return best;
}

int cEngine::Quiesce(POS *p, int ply, int alpha, int beta, int *pv)
{
  int best, score, move, new_pv[MAX_PLY];
  int is_pv = (alpha != beta - 1);
  MOVES m[1];
  UNDO u[1];
  eData e;

  if (InCheck(p)) return QuiesceFlee(p, ply, alpha, beta, pv);

  nodes++;
  local_nodes++;
  CheckTimeout(ply);

  if (abort_search) return 0;
  *pv = 0;
  if (IsDraw(p)) return 0;

  if (ply >= MAX_PLY - 1)
    return Evaluate(p, &e);

  best = Evaluate(p, &e);
  if (best >= beta) return best;
  if (best > alpha) alpha = best;

#ifdef TT_IN_QS
  if (TransRetrieve(p->hash_key, &move, &score, alpha, beta, 0, ply)) {
	  if (!is_pv) return score;
  }
#endif

  InitCaptures(p, m);
  while ((move = NextCapture(m))) {
    DoMove(p, move, u);
    if (Illegal(p)) { UndoMove(p, move, u); continue; }
    score = -Quiesce(p, ply + 1, -beta, -alpha, new_pv);
    UndoMove(p, move, u);
    if (abort_search) return 0;
	if (score >= beta) {
#ifdef TT_IN_QS
		TransStore(p->hash_key, move, score, LOWER, 0, ply);
#endif
		return score;
	}
    if (score > best) {
      best = score;
      if (score > alpha) {
        alpha = score;
        BuildPv(pv, new_pv, move);
      }
    }
  }

  // SAVE RESULT IN THE TRANSPOSITION TABLE

#ifdef TT_IN_QS
  if (*pv) TransStore(p->hash_key, *pv, best, EXACT, 0, ply);
  else     TransStore(p->hash_key,   0, best, UPPER, 0, ply);
#endif

  return best;
}