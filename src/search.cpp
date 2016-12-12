#include <stdio.h>
#include <string.h>
#include "rodent.h"

void Think(POS *p, int *pv)
{
  ClearHist();
  tt_date = (tt_date + 1) & 255;
  nodes = 0;
  abort_search = 0;
  start_time = GetMS();
  for (root_depth = 1; root_depth < 256; root_depth++) {
    printf("info depth %d\n", root_depth);
    Search(p, 0, -INF, INF, root_depth, pv);
    if (abort_search)
      break;
  }
}

int Search(POS *p, int ply, int alpha, int beta, int depth, int *pv)
{
  int best, score, move, new_depth, new_pv[MAX_PLY];
  MOVES m[1];
  UNDO u[1];

  if (depth <= 0)
    return Quiesce(p, ply, alpha, beta, pv);
  nodes++;
  Check();
  if (abort_search) return 0;
  if (ply) *pv = 0;
  if (Repetition(p) && ply)
    return 0;
  move = 0;
  if (TransRetrieve(p->key, &move, &score, alpha, beta, depth, ply))
    return score;
  if (ply >= MAX_PLY - 1)
    return Evaluate(p);
  if (depth > 1 && beta <= Evaluate(p) && !InCheck(p) && MayNull(p)) {
    DoNull(p, u);
    score = -Search(p, ply + 1, -beta, -beta + 1, depth - 3, new_pv);
    UndoNull(p, u);
    if (abort_search) return 0;
    if (score >= beta) {
      TransStore(p->key, 0, score, LOWER, depth, ply);
      return score;
    }
  }
  best = -INF;
  InitMoves(p, m, move, ply);
  while ((move = NextMove(m))) {
    DoMove(p, move, u);
    if (Illegal(p)) { UndoMove(p, move, u); continue; }
    new_depth = depth - 1 + InCheck(p);
    if (best == -INF)
      score = -Search(p, ply + 1, -beta, -alpha, new_depth, new_pv);
    else {
      score = -Search(p, ply + 1, -alpha - 1, -alpha, new_depth, new_pv);
      if (!abort_search && score > alpha && score < beta)
        score = -Search(p, ply + 1, -beta, -alpha, new_depth, new_pv);
    }
    UndoMove(p, move, u);
    if (abort_search) return 0;
    if (score >= beta) {
      Hist(p, move, depth, ply);
      TransStore(p->key, move, score, LOWER, depth, ply);
      return score;
    }
    if (score > best) {
      best = score;
      if (score > alpha) {
        alpha = score;
        BuildPv(pv, new_pv, move);
        if (!ply) DisplayPv(score, pv);
      }
    }
  }
  if (best == -INF)
    return InCheck(p) ? -MATE + ply : 0;
  if (*pv) {
    Hist(p, *pv, depth, ply);
    TransStore(p->key, *pv, best, EXACT, depth, ply);
  } else
    TransStore(p->key, 0, best, UPPER, depth, ply);
  return best;
}

int Quiesce(POS *p, int ply, int alpha, int beta, int *pv)
{
  int best, score, move, new_pv[MAX_PLY];
  MOVES m[1];
  UNDO u[1];

  nodes++;
  Check();
  if (abort_search) return 0;
  *pv = 0;
  if (Repetition(p))
    return 0;
  if (ply >= MAX_PLY - 1)
    return Evaluate(p);
  best = Evaluate(p);
  if (best >= beta)
    return best;
  if (best > alpha)
    alpha = best;
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

int Repetition(POS *p)
{
  int i;

  for (i = 4; i <= p->rev_moves; i += 2)
    if (p->key == p->rep_list[p->head - i])
      return 1;
  return 0;
}

void DisplayPv(int score, int *pv)
{
  char *type, pv_str[512];

  type = "mate";
  if (score < -MAX_EVAL)
    score = (-MATE - score) / 2;
  else if (score > MAX_EVAL)
    score = (MATE - score + 1) / 2;
  else
    type = "cp";
  PvToStr(pv, pv_str);
  printf("info depth %d time %d nodes %d score %s %d pv %s\n",
      root_depth, GetMS() - start_time, nodes, type, score, pv_str);
}

void Check(void)
{
  char command[80];

  if (nodes & 4095 || root_depth == 1)
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
