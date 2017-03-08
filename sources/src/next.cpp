/*
Rodent, a UCI chess playing engine derived from Sungorus 1.4
Copyright (C) 2009-2011 Pablo Vazquez (Sungorus author)
Copyright (C) 2011-2017 Pawel Koziol

Rodent is free software: you can redistribute it and/or modify it under the terms of the GNU
General Public License as published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

Rodent is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along with this program.
If not, see <http://www.gnu.org/licenses/>.
*/

#include "rodent.h"

void cEngine::InitMoves(POS *p, MOVES *m, int trans_move, int ref_move, int ref_sq, int ply) {

  m->p = p;
  m->phase = 0;
  m->trans_move = trans_move;
  m->ref_move = ref_move;
  m->ref_sq = ref_sq;
  m->killer1 = killer[ply][0];
  m->killer2 = killer[ply][1];
}

int cEngine::NextMove(MOVES *m, int *flag) {

  int move;

  switch (m->phase) {
  case 0: // return transposition table move, if legal
    move = m->trans_move;
    if (move && Legal(m->p, move)) {
      m->phase = 1;
	  *flag = MV_HASH;
      return move;
    }

  case 1: // helper phase: generate captures
    m->last = GenerateCaptures(m->p, m->move);
    ScoreCaptures(m);
    m->next = m->move;
    m->badp = m->bad;
    m->phase = 2;

  case 2: // return good captures, save bad ones on the separate list
    while (m->next < m->last) {
      move = SelectBest(m);
      if (move == m->trans_move)
        continue;
      if (BadCapture(m->p, move)) {
        *m->badp++ = move;
        continue;
      }
	  *flag = MV_CAPTURE;
      return move;
    }

  case 3: // first killer move
    move = m->killer1;
    if (move && move != m->trans_move
    && m->p->pc[Tsq(move)] == NO_PC 
    && Legal(m->p, move)) {
      m->phase = 4;
	  *flag = MV_KILLER;
      return move;
    }

  case 4: // second killer move
    move = m->killer2;
    if (move && move != m->trans_move 
    && m->p->pc[Tsq(move)] == NO_PC 
    && Legal(m->p, move)) {
      m->phase = 5;
	  *flag = MV_KILLER;
      return move;
    }

  case 5: // refutation move
    move = m->ref_move;
    if (move && move != m->trans_move 
    &&  m->p->pc[Tsq(move)] == NO_PC 
    &&  move != m->killer1
    &&  move != m->killer2
    && Legal(m->p, move)) {
      m->phase = 6;
      *flag = MV_NORMAL;
      return move;
    }

  case 6: // helper phase: generate quiet moves
    m->last = GenerateQuiet(m->p, m->move);
    ScoreQuiet(m);
    m->next = m->move;
    m->phase = 7;

  case 7: // return quiet moves
    while (m->next < m->last) {
      move = SelectBest(m);
      if (move == m->trans_move
      ||  move == m->killer1
      ||  move == m->killer2
      ||  move == m->ref_move )
          continue;
	  *flag = MV_NORMAL;
      return move;
    }

    m->next = m->bad;
    m->phase = 8;

  case 8: // return bad captures
    if (m->next < m->badp) {
      *flag = MV_BADCAPT;
      return *m->next++;
    }
  }
  return 0;
}

int cEngine::NextSpecialMove(MOVES *m, int *flag) {

  int move;

  switch (m->phase) {
  case 0: // return transposition table move, if legal 
    move = m->trans_move;
    if (move && Legal(m->p, move)) {
      m->phase = 1;
	  *flag = MV_HASH;
      return move;
    }

  case 1: // helper phase: generate captures
    m->last = GenerateCaptures(m->p, m->move);
    ScoreCaptures(m);
    m->next = m->move;
    m->badp = m->bad;
    m->phase = 2;

  case 2: // return good captures, prune bad ones
    while (m->next < m->last) {
      move = SelectBest(m);
      if (move == m->trans_move)
        continue;
      if (BadCapture(m->p, move)) {
        continue;
      }
	  *flag = MV_CAPTURE;
      return move;
    }

  case 3: // first killer move
    move = m->killer1;
    if (move && move != m->trans_move
    && m->p->pc[Tsq(move)] == NO_PC 
    && Legal(m->p, move)) {
      m->phase = 4;
	  *flag = MV_KILLER;
      return move;
    }

  case 4: // second killer move
    move = m->killer2;
    if (move && move != m->trans_move
    && m->p->pc[Tsq(move)] == NO_PC && Legal(m->p, move)) {
      m->phase = 5;
	  *flag = MV_KILLER;
      return move;
    }

  case 5: // helper phase: generate checking moves
    m->last = GenerateSpecial(m->p, m->move);
    ScoreQuiet(m);
    m->next = m->move;
    m->phase = 6;

  case 6: // return checking moves
    while (m->next < m->last) {
      move = SelectBest(m);
      if (move == m->trans_move
      ||  move == m->killer1
      ||  move == m->killer2)
        continue;
	  *flag = MV_NORMAL;
      return move;
    }
  }
  return 0;
}

void cEngine::InitCaptures(POS *p, MOVES *m) {

  m->p = p;
  m->last = GenerateCaptures(m->p, m->move);
  ScoreCaptures(m);
  m->next = m->move;
}

int cEngine::NextCapture(MOVES *m) {

  int move;

  while (m->next < m->last) {
    move = SelectBest(m);
    return move;
  }
  return 0;
}

void cEngine::ScoreCaptures(MOVES *m) {

  int *movep, *valuep;

  valuep = m->value;
  for (movep = m->move; movep < m->last; movep++)
    *valuep++ = MvvLva(m->p, *movep);
}

void cEngine::ScoreQuiet(MOVES *m) {

  int *movep, *valuep;

  valuep = m->value;
  for (movep = m->move; movep < m->last; movep++) {
    int mv_score = history[m->p->pc[Fsq(*movep)]][Tsq(*movep)]; // use history score
	if (Fsq(*movep) == m->ref_sq) mv_score += 2048;             // but bump up refutation move
    *valuep++ = mv_score;
  }
}

int cEngine::SelectBest(MOVES *m) {

  int *movep, *valuep, aux;

  valuep = m->value + (m->last - m->move) - 1;
  for (movep = m->last - 1; movep > m->next; movep--) {
    if (*valuep > *(valuep - 1)) {
      aux = *valuep;
      *valuep = *(valuep - 1);
      *(valuep - 1) = aux;
      aux = *movep;
      *movep = *(movep - 1);
      *(movep - 1) = aux;
    }
    valuep--;
  }
  return *m->next++;
}

int cEngine::BadCapture(POS *p, int move) {

  int fsq = Fsq(move);
  int tsq = Tsq(move);

  // using tp_value[] instead of pc_value[] means that both BxN and NxB aren't bad

  if (tp_value[TpOnSq(p, tsq)] >= tp_value[TpOnSq(p, fsq)])
    return 0;

  if (MoveType(move) == EP_CAP)
    return 0;

  return Swap(p, fsq, tsq) < 0;
}

int cEngine::MvvLva(POS *p, int move) {

  if (p->pc[Tsq(move)] != NO_PC)
    return TpOnSq(p, Tsq(move)) * 6 + 5 - TpOnSq(p, Fsq(move));

  if (IsProm(move))
    return PromType(move) - 5;

  return 5;
}

void cEngine::ClearHist(void) {

  for (int tp = 0; tp < 12; tp++)
    for (int sq = 0; sq < 64; sq++)
      history[tp][sq] = 0;

  for (int fsq = 0; fsq < 64; fsq++)
    for (int tsq = 0; tsq < 64; tsq++)
      refutation[fsq][tsq] = 0;

  for (int i = 0; i < MAX_PLY; i++) {
    killer[i][0] = 0;
    killer[i][1] = 0;
  }
}

void cEngine::TrimHist(void) {

  for (int tp = 0; tp < 12; tp++)
    for (int sq = 0; sq < 64; sq++)
      history[tp][sq] /= 2;
}

void cEngine::UpdateHistory(POS *p, int last_move, int move, int depth, int ply) {

  // No update on a move that changes material balance

  if (p->pc[Tsq(move)] != NO_PC || IsProm(move) || MoveType(move) == EP_CAP)
    return;

  // Update history table, making sure that scores don't grow too high

  history[p->pc[Fsq(move)]][Tsq(move)] += 2*depth*depth;
  if (history[p->pc[Fsq(move)]][Tsq(move)] > MAX_HIST) TrimHist();

  // Update refutation table, saving new move in the table indexed
  // by the coordinates of last move. last_move == 0 is a null move,
  // special case of last_move == -1 denotes situations when updating
  // refutation table is switched off: at root or in QuiesceFlee()

  if (last_move >= 0)
	  refutation[Fsq(last_move)][Tsq(last_move)] = move;

  // Update killer moves

  if (move != killer[ply][0]) {
    killer[ply][1] = killer[ply][0];
    killer[ply][0] = move;
  }
}

void cEngine::DecreaseHistory(POS *p, int move, int depth) {

  // No update on a move that changes material balance

  if (p->pc[Tsq(move)] != NO_PC || IsProm(move) || MoveType(move) == EP_CAP)
    return;

  // Update history table, making sure that scores don't fall too low

  history[p->pc[Fsq(move)]][Tsq(move)] -= depth*depth;
  if (history[p->pc[Fsq(move)]][Tsq(move)] < -MAX_HIST) TrimHist();
}

int cEngine::Refutation(int move) {
  return refutation[Fsq(move)][Tsq(move)];
}
