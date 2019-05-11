/*
Rodent, a UCI chess playing engine derived from Sungorus 1.4
Copyright (C) 2009-2011 Pablo Vazquez (Sungorus author)
Copyright (C) 2011-2019 Pawel Koziol

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
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>

const int cEngine::mscSnpDepth = 3;       // max depth at which static null move pruning is applied
const int cEngine::mscRazorDepth = 4;     // max depth at which razoring is applied
const int cEngine::mscFutDepth = 6;       // max depth at which futility pruning is applied

// this variable controls when evaluation function needs to be called for the sake of pruning
const int cEngine::mscSelectiveDepth = Max(Max(mscSnpDepth, mscRazorDepth), mscFutDepth);

const int cEngine::mscRazorMargin[5] = { 0, 300, 360, 420, 480 };
const int cEngine::mscFutMargin[7] = { 0, 100, 150, 200, 250, 300, 400 };
int cEngine::msLmrSize[2][MAX_PLY][MAX_MOVES];

void cParam::InitAsymmetric(POS *p) {

    programSide = p->mSide;

    if (programSide == WC) { // TODO: no if/else, but progside/op_side
        sideAttack[WC] = values[W_OWN_ATT];
        sideAttack[BC] = values[W_OPP_ATT];
        sideMobility[WC] = values[W_OWN_MOB];
        sideMobility[BC] = values[W_OPP_MOB];
    }
    else {
        sideAttack[BC] = values[W_OWN_ATT];
        sideAttack[WC] = values[W_OPP_ATT];
        sideMobility[BC] = values[W_OWN_MOB];
        sideMobility[WC] = values[W_OPP_MOB];
    }
}

void cGlobals::ClearData() {

    Trans.Clear();
#ifndef USE_THREADS
    EngineSingle.ClearAll();
#else
    for (auto& engine : Engines) {
        engine.ClearAll();
    }
#endif
    shouldClear = false;
}

bool cGlobals::MoveToAvoid(int move) {

    for (int i = 0; i <= MAX_PV; i++) {
        if (avoidMove[i] == move) return true;
    }

    return false;
}

void cGlobals::ClearAvoidList() {

    for (int i = 0; i <= MAX_PV; i++) {
        avoidMove[i] = 0;
    }
}

void cGlobals::SetAvoidMove(int loc, int move) {
    avoidMove[loc] = move;
}

void cEngine::InitSearch() { // static init function

    // Set depth of late move reduction (formula based on Stockfish)

    for (int depth = 0; depth < MAX_PLY; depth++)
        for (int moveCount = 0; moveCount < MAX_MOVES; moveCount++) {

            int r = 0;

            if (depth != 0 && moveCount != 0) {
                // +-inf to int is undefined
                r = (int)(0.33 + (log(Min(moveCount, 63)) * log(Min(depth, 63)) / 2.00));
            }

            msLmrSize[0][depth][moveCount] = r;     // zero window node
            msLmrSize[1][depth][moveCount] = r - 1; // principal variation node (checking for pos. values is in `Search()`)

            // reduction cannot exceed actual depth
            
            if (msLmrSize[0][depth][moveCount] > depth - 1) {
                msLmrSize[0][depth][moveCount] = depth - 1;
            }

            if (msLmrSize[1][depth][moveCount] > depth - 1) {
                msLmrSize[1][depth][moveCount] = depth - 1;
            }
        }
}

void cEngine::Think(POS *p) {

    POS curr[1];
    mPvEng[0] = 0; // clear engine's move
    mPvEng[1] = 0; // clear ponder move
    Glob.ClearAvoidList();
    Glob.scoreJump = false;
    mFlRootChoice = false;
    *curr = *p;
    AgeHist();
    Iterate(curr, mPvEng);
    mEngSide = p->mSide;
}

void cEngine::MultiPv(POS * p, int * pv) {

    int val[MAX_PV + 1];
    int bestPv = 1;
    int bestScore;

    Line line[MAX_PV + 1];

    for (int i = 0; i <= MAX_PV; i++) {
        val[i] = 0;
    }

    for (mRootDepth = 1; mRootDepth <= msSearchDepth; mRootDepth++) {
        Glob.ClearAvoidList();
        bestScore = -INF;
        bestPv = 0;

        val[1] = Widen(p, mRootDepth, line[1].pv, val[1]);

        if (Glob.abortSearch) {
            break;
        }

        if (val[1] > bestScore) {
            bestPv = 1;
            bestScore = val[1];
        };

        Glob.SetAvoidMove(1, line[1].pv[0]);

        for (int i = 2; i <= Glob.multiPv; i++) {

            val[i] = Widen(p, mRootDepth, line[i].pv, val[i]);

            if (Glob.abortSearch) {
                break;
            }

            if (val[i] > bestScore) {
                bestPv = i;
                bestScore = val[i];
            };

            Glob.SetAvoidMove(i, line[i].pv[0]);
        }

        if (Glob.abortSearch) {
            break;
        }

        for (int i = Glob.multiPv; i > 0; i--) {
            if (p->Legal(line[i].pv[0])) {
                DisplayPv(i, val[i], line[i].pv);
            }
        }

        pv = line[1].pv;
    }

    if (bestPv == 0) {
        ExtractMove(line[1].pv);
    } else {
        ExtractMove(line[bestPv].pv);
    }
}

void cEngine::Iterate(POS *p, int *pv) {

    int cur_val = 0;
    int depthCounter = 0;

    // Lazy SMP works best with some depth variance,
    // so every other thread will search to depth + 1

    int offset = mcThreadId & 0x01;

    for (mRootDepth = 1 + offset; mRootDepth <= msSearchDepth; mRootDepth++) {

        tDepth[mcThreadId] = mRootDepth;
        depthCounter = 0;
        for (int j = 0; j < Glob.thread_no; j++) {
            if (tDepth[j] >= mRootDepth) depthCounter++;
        }

        // skip depth if it already has good coverage in multi-threaded mode

        if (mRootDepth > 5
        && mRootDepth < msSearchDepth
        && Glob.thread_no > 1
        && depthCounter > Glob.thread_no / 2) continue;

        // If a thread is lagging behind too much, which makes it unlikely
        // to contribute to the final result, skip the iteration.

        if (Glob.depthReached > mDpCompleted + 1) {
            mDpCompleted++;
            continue;
        }

        // Perform actual search

        printf("info depth %d\n", mRootDepth);

        if (Par.searchSkill > 6) {
            cur_val = Widen(p, mRootDepth, pv, cur_val);
        } else {
            cur_val = SearchRoot(p, 0, -INF, INF, mRootDepth, pv);
        }

        if (Glob.abortSearch) {
            break;
        }

        // Shorten search if there is only one root move available

        if (mRootDepth >= 8 && mFlRootChoice == false) {
            break;
        }

        // Abort search on finding checkmate score

        if (cur_val > MAX_EVAL || cur_val < -MAX_EVAL) {
            int max_mate_depth = (MATE - Abs(cur_val) + 1) + 1;
            max_mate_depth *= 4;
            max_mate_depth /= 3;
            if (max_mate_depth <= mRootDepth) {
                mDpCompleted = mRootDepth;
                break;
            }
        }

        // Set information about depth

        mDpCompleted = mRootDepth;
        if (Glob.depthReached < mDpCompleted) {
            Glob.depthReached = mDpCompleted;
        }
    }

    if (!Par.shut_up) Glob.abortSearch = true; // for correct exit from fixed depth search
}

// Aspiration search, progressively widening the window (based on Senpai 1.0)

int cEngine::Widen(POS *p, int depth, int *pv, int lastScore) {

    int cur_val = lastScore, alpha, beta;

    if (depth > 6 && lastScore < MAX_EVAL) {
        for (int margin = 8; margin < 500; margin *= 2) {
            alpha = lastScore - margin;
            beta = lastScore + margin;
            cur_val = SearchRoot(p, 0, alpha, beta, depth, pv);
            if (Glob.abortSearch) break;

            // score drops

            if (cur_val < alpha && margin > 50) {
                Glob.scoreJump = true;
            }

            // score increases (this seems to hurt at more threads, need a confirmation run)

            if (cur_val > beta
            && margin > 50
            && Glob.thread_no == 1) {
                Glob.scoreJump = true;
            }

            // we have finished within the window and can return

            if (cur_val > alpha && cur_val < beta) {
                return cur_val;
            }

            // break in order to verify mate searching with infinite bounds

            if (cur_val > MAX_EVAL) {
                break;
            }
        }
    }

    cur_val = SearchRoot(p, 0, -INF, INF, depth, pv); // full window search
    return cur_val;
}

int cEngine::SearchRoot(POS *p, int ply, int alpha, int beta, int depth, int *pv) {

    int best, score = -INF, move, newDepth, new_pv[MAX_PLY];
    int mv_type, reduction, victim, last_capt, hashFlag;
    int singMove = -1, singScore = -INF;
    int mv_tried = 0;
    int mv_played[MAX_MOVES];
    int quiet_tried = 0;
    int mv_hist_score = 0;
    MOVES m[1];
    UNDO u[1];
    eData e;
    int hashScore = -INF;

    bool flagInCheck;
    bool flagExtended;
    bool isPv = (alpha != beta - 1);
    bool canSing = false;

    // EARLY EXIT AND NODE INITIALIZATION

    Glob.nodes++;
    Slowdown();
    if (Glob.abortSearch && mRootDepth > 1) return 0;
    if (ply) *pv = 0;
    if (p->IsDraw() && ply) return p->DrawScore();
    move = 0;

    // RETRIEVE MOVE FROM TRANSPOSITION TABLE

    if (Trans.Retrieve(p->mHashKey, &move, &hashScore, &hashFlag, alpha, beta, depth, ply)) {

        if (hashScore >= beta) {
            UpdateHistory(p, -1, move, depth, ply);
        }

        if (!isPv && Par.searchSkill > 0) {
            return hashScore;
        }

    }

    // PREPARE FOR SINGULAR EXTENSION, SENPAI-STYLE

    if (isPv && depth > 5) {
        if (Trans.Retrieve(p->mHashKey, &singMove, &singScore, &hashFlag, alpha, beta, depth - 4, ply)) {
            if (hashFlag & LOWER) {
                canSing = true;
            }
        }
    }

    // SAFEGUARD AGAINST REACHING MAX PLY LIMIT

    if (ply >= MAX_PLY - 1) {
        return Evaluate(p, &e);
    }

    flagInCheck = p->InCheck();

    // INTERNAL ITERATIVE DEEPENING

    if (isPv
    && !flagInCheck
    && !move
    && depth > 6) {
        Search(p, ply, alpha, beta, depth - 2, false, -1, -1, pv);
        Trans.RetrieveMove(p->mHashKey, &move);
    }

    // PREPARE FOR MAIN SEARCH

    best = -INF;
    InitMoves(p, m, move, Refutation(move), -1, ply);

    // MAIN LOOP

    while ((move = NextMove(m, &mv_type, ply))) {

        // MAKE MOVE

        mv_hist_score = mHistory[p->mPc[Fsq(move)]][Tsq(move)];
        victim = p->TpOnSq(Tsq(move));

        // set last capture square (needed for extension)

        if (victim != NO_TP) {
            last_capt = Tsq(move);
        } else {
            last_capt = -1;
        }

        p->DoMove(move, u);

        if (p->Illegal()) {
            p->UndoMove(move, u);
            continue;
        }

        // DON'T SEARCH THE SAME MOVES IN MULTI-PV MODE 

        if (Glob.MoveToAvoid(move)) {
            p->UndoMove(move, u);
            continue;
        }

        // GATHER INFO ABOUT THE MOVE

        flagExtended = false;
        mv_played[mv_tried] = move;
        mv_tried++;
        if (!ply && mv_tried > 1) mFlRootChoice = true;
        if (mv_type == MV_NORMAL) quiet_tried++;
        if (ply == 0 && !Par.shut_up && depth > 16 && Glob.thread_no == 1)
            DisplayCurrmove(move, mv_tried);

        // SET NEW SEARCH DEPTH

        newDepth = depth - 1;

        // EXTENSIONS

        // 1. check extension, applied in pv nodes or at low depth

        if (isPv || depth < 8) {
            newDepth += p->InCheck();
            flagExtended = true;
        };

        // 2. pawn to 7th rank extension at the tips of pv-line

        if (isPv
        && depth < 6
        && p->TpOnSq(Tsq(move)) == P
        && (SqBb(Tsq(move)) & (RANK_2_BB | RANK_7_BB))) {
            newDepth += 1;
            flagExtended = true;
        };

        // 3. singular extension, Senpai-style

        if (isPv && depth > 5 && move == singMove && canSing && flagExtended == false) {
            int new_alpha = -singScore - 50;
            int mockPv;
            int sc = Search(p, ply + 1, new_alpha, new_alpha + 1, depth - 4, false, -1, -1, &mockPv);
            if (sc <= new_alpha) {
                newDepth += 1;
                flagExtended = true;
            }
        }

        // LMR 1: NORMAL MOVES

        reduction = 0;

        if (depth > 2
        && Par.searchSkill > 2
        && mv_tried > 3
        && !flagInCheck
        && !p->InCheck()
        && msLmrSize[isPv][depth][mv_tried] > 0
        && mv_type == MV_NORMAL
        && mv_hist_score < Par.histLimit
        && MoveType(move) != CASTLE) {

            // read reduction amount from the table

            reduction = (int)msLmrSize[isPv][depth][mv_tried];

            // increase reduction on bad history score

            if (mv_hist_score < 0
            && newDepth - reduction >= 2)
                reduction++;

            newDepth = newDepth - reduction;
        }

    research:

        // PRINCIPAL VARIATION SEARCH

        if (best == -INF)
            score = -Search(p, ply + 1, -beta, -alpha, newDepth, false, move, last_capt, new_pv);
        else {
            score = -Search(p, ply + 1, -alpha - 1, -alpha, newDepth, false, move, last_capt, new_pv);
            if (!Glob.abortSearch && score > alpha && score < beta)
                score = -Search(p, ply + 1, -beta, -alpha, newDepth, false, move, last_capt, new_pv);
        }

        // DON'T REDUCE A MOVE THAT SCORED ABOVE ALPHA

        if (score > alpha && reduction) {
            newDepth = newDepth + reduction;
            reduction = 0;
            goto research;
        }

        // UNDO MOVE

        p->UndoMove(move, u);
        if (Glob.abortSearch && mRootDepth > 1) return 0;

        // BETA CUTOFF

        if (score >= beta) {
            if (!flagInCheck) {
                UpdateHistory(p, -1, move, depth, ply);
                for (int mv = 0; mv < mv_tried; mv++) {
                    DecreaseHistory(p, mv_played[mv], depth);
                }
            }
            Trans.Store(p->mHashKey, move, score, LOWER, depth, ply);

            // At root, change the best move and show the new pv

            if (!ply) {
                BuildPv(pv, new_pv, move);
                if (Glob.multiPv == 1) DisplayPv(0, score, pv);
            }

            return score;
        }

        // NEW BEST MOVE

        if (score > best) {
            best = score;
            if (score > alpha) {
                alpha = score;
                BuildPv(pv, new_pv, move);
                if (Glob.multiPv == 1) DisplayPv(0, score, pv);
            }
        }

    } // end of main loop

    // RETURN CORRECT CHECKMATE/STALEMATE SCORE

    if (best == -INF)
        return p->InCheck() ? -MATE + ply : p->DrawScore();

    // SAVE RESULT IN THE TRANSPOSITION TABLE

    if (*pv) {
        if (!flagInCheck) {
            UpdateHistory(p, -1, *pv, depth, ply);
            for (int mv = 0; mv < mv_tried; mv++) {
                DecreaseHistory(p, mv_played[mv], depth);
            }
        }
        Trans.Store(p->mHashKey, *pv, best, EXACT, depth, ply);
    }
    else
        Trans.Store(p->mHashKey, 0, best, UPPER, depth, ply);

    return best;
}


int cEngine::Search(POS *p, int ply, int alpha, int beta, int depth, bool wasNull, int lastMove, int lastCaptSquare, int *pv) {

    int best, score = -INF, nullScore, move, newDepth, newPv[MAX_PLY];
    int moveType, reduction, victim, lastCaptTarget, hashFlag, nullHashFlag;
    int nullRefutation = -1, refutationSqare = -1, singMove = -1, singScore = -INF;
    int movesTried = 0;
    int movesPlayed[MAX_MOVES];
    int quietTried = 0;
    int moveHistScore = 0;
    MOVES m[1];
    UNDO u[1];
    eData e;
    int moveSEEscore = 0; // see score of a bad capture
    int hashScore = -INF;

    bool flagInCheck;
    bool flagExtended;
    bool flagFutility = false;
    bool didNull = false;
    bool isPv = (alpha != beta - 1);
    bool canSing = false;

    // QUIESCENCE SEARCH ENTRY POINT

    if (depth <= 0) {
        return QuiesceChecks(p, ply, alpha, beta, pv);
    }

    // EARLY EXIT AND NODE INITIALIZATION

    Glob.nodes++;
    Slowdown();
    if (Glob.abortSearch && mRootDepth > 1) return 0;
    *pv = 0;
    if (p->IsDraw()) return p->DrawScore();
    move = 0;

    // MATE DISTANCE PRUNING

    int checkmatingScore = MATE - ply;
    if (checkmatingScore < beta) {
        beta = checkmatingScore;
        if (alpha >= checkmatingScore) {
            return alpha;
        }
    }

    int checkmatedScore = -MATE + ply;
    if (checkmatedScore > alpha) {
        alpha = checkmatedScore;
        if (beta <= checkmatedScore) {
            return beta;
        }
    }

    bool hasTT = false;

    // RETRIEVE MOVE FROM TRANSPOSITION TABLE

    if (Trans.Retrieve(p->mHashKey, &move, &hashScore, &hashFlag, alpha, beta, depth, ply)) {

        hasTT = true;

        if (hashScore >= beta) {
            UpdateHistory(p, lastMove, move, depth, ply);
        }

        if (!isPv && Par.searchSkill > 0) {
            return hashScore;
        }
    }

    // PREPARE FOR SINGULAR EXTENSION, SENPAI-STYLE

    if (isPv && depth > 5) {
        if (Trans.Retrieve(p->mHashKey, &singMove, &singScore, &hashFlag, alpha, beta, depth - 4, ply)) {
            if (hashFlag & LOWER) {
                canSing = true;
            }
        }
    }

    // SAFEGUARD AGAINST REACHING MAX PLY LIMIT

    if (ply >= MAX_PLY - 1) {
        int eval = Evaluate(p, &e);
        return eval;
    }

    flagInCheck = p->InCheck();

    // CAN WE PRUNE THIS NODE?

    int flagPrunableNode = !flagInCheck
        && !isPv
        && alpha > -MAX_EVAL
        && beta < MAX_EVAL;

    // GET EVAL SCORE FOR PRUNING/REDUCTION DECISIONS

    int eval = 0;
    if (flagInCheck) eval = -INF;
    else eval = Evaluate(p, &e);

    // ADJUST EVAL USING HASH SCORE,THEN SAVE TO STACK

    if (hasTT) {
        if (hashFlag & (hashScore > eval ? LOWER : UPPER))
            eval = hashScore;
    }

    mEvalStack[ply] = eval;

    // CHECK IF SCORE IS IMPROVING

    bool improving = false;

    if (ply > 2
    && !flagInCheck
    && eval > mEvalStack[ply - 2]) {
        improving = true;
    }

    // BETA PRUNING / STATIC NULL MOVE

    if (flagPrunableNode
    && Par.searchSkill > 7
    && depth <= 7
    && eval < MAX_EVAL
    && p->MayNull()
    && !wasNull) {
        int sc = eval - (175 - 50 * improving) * depth;
        if (sc > beta) return sc;
    }

    // NULL MOVE

    if (depth > 1
    && Par.searchSkill > 1
    && !wasNull
    && flagPrunableNode
    && p->MayNull()
    && eval >= beta) {

        didNull = true;

        // null move depth reduction - modified Stockfish formula

        newDepth = SetNullReductionDepth(depth, eval, beta);

        // omit null move search if normal search to the same depth wouldn't exceed beta
        // (sometimes we can check it for free via hash table)

        if (Trans.Retrieve(p->mHashKey, &move, &nullScore, &nullHashFlag, alpha, beta, newDepth, ply)) {
            if (nullScore < beta) goto avoidNull;
        }

        p->DoNull(u);
        if (newDepth <= 0) score = -QuiesceChecks(p, ply + 1, -beta, -beta + 1, newPv);
        else                score = -Search(p, ply + 1, -beta, -beta + 1, newDepth, true, 0, -1, newPv);

        // get location of a piece whose capture refuted null move
        // its escape will be prioritised in the move ordering

        Trans.Retrieve(p->mHashKey, &nullRefutation, &nullScore, &nullHashFlag, alpha, beta, depth, ply);
        if (nullRefutation > 0) refutationSqare = Tsq(nullRefutation);

        p->UndoNull(u);

        if (Glob.abortSearch && mRootDepth > 1) {
            return 0;
        }

        // do not return unproved mate scores, Stockfish-style

        if (score >= MAX_EVAL) {
            score = beta;
        }

        if (score >= beta) {

            // verification search

            if (newDepth > 6 && Par.searchSkill > 9)
                score = Search(p, ply, alpha, beta, newDepth - 5, true, lastMove, lastCaptSquare, pv);

            if (Glob.abortSearch && mRootDepth > 1) return 0;
            if (score >= beta) return score;
        }
    } // end of null move code

avoidNull:

    // RAZORING (based on Toga II 3.0)

    if (flagPrunableNode
    && Par.searchSkill > 3
    && !move
    && !wasNull
    && !(p->Pawns(p->mSide) & bb_rel_rank[p->mSide][RANK_7]) // no pawns to promote in one move
    && depth <= mscRazorDepth) {
        int threshold = beta - mscRazorMargin[depth];

        if (eval < threshold) {
            score = QuiesceChecks(p, ply, alpha, beta, pv);
            if (score < threshold) return score;
        }
    } // end of razoring code

    // INTERNAL ITERATIVE DEEPENING

    if (isPv
    && !flagInCheck
    && !move
    && depth > 6) {
        Search(p, ply, alpha, beta, depth - 2, false, -1, lastCaptSquare, pv);
        Trans.RetrieveMove(p->mHashKey, &move);
    }

    // TODO: internal iterative deepening in cut nodes

    // PREPARE FOR MAIN SEARCH

    best = -INF;
    InitMoves(p, m, move, Refutation(move), refutationSqare, ply);

    // MAIN LOOP

    while ((move = NextMove(m, &moveType, ply))) {

        // SET FUTILITY PRUNING FLAG
        // before the first applicable move is tried

        if (moveType == MV_NORMAL
        && Par.searchSkill > 4
        && quietTried == 0
        && flagPrunableNode
        && depth <= mscFutDepth) {
            if (eval + mscFutMargin[depth] < beta) flagFutility = true;
        }

        // GET MOVE HISTORY SCORE

        moveHistScore = mHistory[p->mPc[Fsq(move)]][Tsq(move)];

        // GET SEE SCORE OF A BAD CAPTURE

        if (moveType == MV_BADCAPT) {
            moveSEEscore = p->Swap(Fsq(move), Tsq(move));
        }

        // SAVE INFORMATION ABOUT A POSSIBLE CAPTURE VICTIM

        victim = p->TpOnSq(Tsq(move));
        if (victim != NO_TP) {
            lastCaptTarget = Tsq(move);
        } else {
            lastCaptTarget = -1;
        }

        // MAKE MOVE

        p->DoMove(move, u);
        if (p->Illegal()) { 
            p->UndoMove(move, u); 
            continue; 
        }

        // GATHER INFO ABOUT THE MOVE

        flagExtended = false;
        movesPlayed[movesTried] = move;
        movesTried++;
        
        if (!ply && movesTried > 1) {
            mFlRootChoice = true;
        }

        if (moveType == MV_NORMAL) {
            quietTried++;
        }

        if (ply == 0 && !Par.shut_up && depth > 16 && Glob.thread_no == 1) {
            DisplayCurrmove(move, movesTried);
        }

        // SET NEW SEARCH DEPTH

        newDepth = depth - 1;

        // EXTENSIONS

        // 1. check extension, applied in pv nodes or at low depth

        if (isPv || depth < 8) {
            newDepth += p->InCheck();
            flagExtended = true;
        };

        // 2. recapture extension in pv-nodes

        if (isPv && Tsq(move) == lastCaptSquare) {
            newDepth += 1;
            flagExtended = true;
        };

        // 3. pawn to 7th rank extension at the tips of pv-line

        if (isPv
        && depth < 6
        && p->TpOnSq(Tsq(move)) == P
        && (SqBb(Tsq(move)) & (RANK_2_BB | RANK_7_BB))) {
            newDepth += 1;
            flagExtended = true;
        };

        // 4. singular extension, Senpai-style

        if (isPv
        && depth > 5
        && move == singMove
        && canSing
        /*&& flExtended == false*/) {
            int newAlpha = -singScore - 50;
            int mockPv;
            int sc = Search(p, ply+1, newAlpha, newAlpha + 1, depth - 4, false, -1, -1, &mockPv);
            if (sc <= newAlpha) {
                newDepth += 1;
                flagExtended = true;
            }
        }

        // FUTILITY PRUNING

        if (flagFutility
        && !p->InCheck()
        && moveHistScore < Par.histLimit
        && (moveType == MV_NORMAL)
        && movesTried > 1) {
            p->UndoMove(move, u);
            continue;
        }

        // LATE MOVE PRUNING

        const int lmpTable[][10 + 1] = {
            { 0, 3, 4, 6, 10, 15, 21, 28, 36, 45, 55 },
            { 0, 5, 6, 9, 15, 23, 32, 42, 54, 68, 83 }
        };

        if (flagPrunableNode
        && Par.searchSkill > 5
        && depth <= 10
        && quietTried > lmpTable[improving][depth]
        && !p->InCheck()
        && moveHistScore < Par.histLimit
        && moveType == MV_NORMAL) {
            p->UndoMove(move, u);
            continue;
        }

        // SEE pruning of bad captures

        if (flagPrunableNode
        && (moveType == MV_BADCAPT)
        && !p->InCheck()
        && depth <= 3
        && !isPv
        && alpha > -MAX_EVAL
        && beta < MAX_EVAL) {
            if (moveSEEscore > 150 * depth) { // yes, sign is correct
                p->UndoMove(move, u);
                continue;
            }
        }

        // LMR 1: NORMAL MOVES

        reduction = 0;

        if (depth > 2
        && Par.searchSkill > 2
        && movesTried > 3
        && !flagInCheck
        && !p->InCheck()
        && msLmrSize[isPv][depth][movesTried] > 0
        && moveType == MV_NORMAL
        && moveHistScore < Par.histLimit
        && MoveType(move) != CASTLE) {

            // read reduction amount from the table

            reduction = (int)msLmrSize[isPv][depth][movesTried];

            // increase reduction if move has a bad history score...

            if (moveHistScore < 0) {
                reduction++;
            }

            // ...in two steps (in non-pv nodes)

            if (!isPv && moveHistScore < -MAX_HIST / 2) {
                reduction++;
            }

            // increase reduction if score is dropping within search

            if (!isPv && !improving) {
                reduction++;
            }

            // reduction cannot exceed actual depth

            if (reduction >= newDepth) {
                reduction = newDepth - 1;
            }

            newDepth = newDepth - reduction;
        }

        // LMR 2: MARGINAL REDUCTION OF BAD CAPTURES

        if (depth > 2
        && Par.searchSkill > 8
        && movesTried > 6
        && alpha > -MAX_EVAL && beta < MAX_EVAL
        && !flagInCheck
        && !p->InCheck()
        && (moveType == MV_BADCAPT)
        && !isPv) {
            reduction = 1;
            newDepth -= reduction;
        }

    research:

        // PRINCIPAL VARIATION SEARCH

        if (best == -INF)
            score = -Search(p, ply + 1, -beta, -alpha, newDepth, false, move, lastCaptTarget, newPv);
        else {
            score = -Search(p, ply + 1, -alpha - 1, -alpha, newDepth, false, move, lastCaptTarget, newPv);
            if (!Glob.abortSearch && score > alpha && score < beta)
                score = -Search(p, ply + 1, -beta, -alpha, newDepth, false, move, lastCaptTarget, newPv);
        }

        // DON'T REDUCE A MOVE THAT SCORED ABOVE ALPHA

        if (score > alpha && reduction) {
            newDepth = newDepth + reduction;
            reduction = 0;
            goto research;
        }

        // UNDO MOVE

        p->UndoMove(move, u);
        if (Glob.abortSearch && mRootDepth > 1) return 0;

        // BETA CUTOFF

        if (score >= beta) {
            if (!flagInCheck) {
                UpdateHistory(p, lastMove, move, depth, ply);
                for (int mv = 0; mv < movesTried; mv++) {
                    DecreaseHistory(p, movesPlayed[mv], depth);
                }
            }
            Trans.Store(p->mHashKey, move, score, LOWER, depth, ply);

            return score;
        }

        // NEW BEST MOVE

        if (score > best) {
            best = score;
            if (score > alpha) {
                alpha = score;
                BuildPv(pv, newPv, move);
            }
        }

    } // end of main loop

    // RETURN CORRECT CHECKMATE/STALEMATE SCORE

    if (best == -INF) {
        return p->InCheck() ? -MATE + ply : p->DrawScore();
    }

    // SAVE RESULT IN THE TRANSPOSITION TABLE

    if (*pv) {
        if (!flagInCheck) {
            UpdateHistory(p, lastMove, *pv, depth, ply);
            for (int mv = 0; mv < movesTried; mv++) {
                DecreaseHistory(p, movesPlayed[mv], depth);
            }
        }
        Trans.Store(p->mHashKey, *pv, best, EXACT, depth, ply);
    } else
        Trans.Store(p->mHashKey, 0, best, UPPER, depth, ply);

    return best;
}

int cEngine::SetNullReductionDepth(int depth, int eval, int beta) {

    return depth - ((823 + 67 * depth) / 256)
                 - Min(3, (eval - beta) / 200);
}

U64 GetNps(int elapsed) {

    U64 nps = 0;
    if (elapsed) {
        nps = (Glob.nodes * 1000) / elapsed;
    }
    return nps;
}

void DisplayCurrmove(int move, int tried) {

    if (!Glob.isConsole) {
        printf("info currmove ");
        PrintMove(move);
        printf(" currmovenumber %d \n", tried);
    }
}

void cEngine::DisplayPv(int multipv, int score, int *pv) {

    // don't display information from threads that are late

    if (mRootDepth < Glob.depthReached) return;

    const char *type; 
    char pvString[512];
    int elapsed = GetMS() - msStartTime;
    U64 nps = GetNps(elapsed);

    type = "mate";
    if (score < -MAX_EVAL)
        score = (-MATE - score) / 2;
    else if (score > MAX_EVAL)
        score = (MATE - score + 1) / 2;
    else
        type = "cp";

    PvToStr(pv, pvString);

    if (multipv == 0)
        printf("info depth %d time %d nodes %" PRIu64 " nps %" PRIu64 " score %s %d pv %s\n",
                mRootDepth, elapsed, (U64)Glob.nodes, nps, type, score, pvString);
    else
        printf("info depth %d multipv %d time %d nodes %" PRIu64 " nps %" PRIu64 " score %s %d pv %s\n",
                mRootDepth, multipv, elapsed, (U64)Glob.nodes, nps, type, score, pvString);
}

void CheckTimeout() {

    char command[80];

    if (InputAvailable()) {
        ReadLine(command, sizeof(command));
        if (strcmp(command, "stop") == 0)
            Glob.abortSearch = true;
        else if (strcmp(command, "quit") == 0) {
#ifndef USE_THREADS
            exit(0);
#else
            Glob.abortSearch = true;
            Glob.goodbye = true; // will crash if just `exit()`. should wait until threads are terminated
#endif
        }
        else if (strcmp(command, "ponderhit") == 0)
            Glob.pondering = false;
    }

    int time = cEngine::msMoveTime;
    if (Glob.scoreJump && Glob.timeTricks) time *= 2;

    if (!Glob.pondering && cEngine::msMoveTime >= 0 && GetMS() - cEngine::msStartTime >= time)
        Glob.abortSearch = true;
}

void cEngine::Slowdown() {

    // Handling search limited by the number of nodes

    if (msMoveNodes > 0) {
        if (Glob.nodes >= (unsigned)msMoveNodes)
            Glob.abortSearch = true;
    }

    // Handling slowdown for weak levels

    if (Par.npsLimit && mRootDepth > 1) {
        int time = GetMS() - msStartTime + 1;
        int nps = (int)GetNps(time);
        while (nps > Par.npsLimit) {
            WasteTime(10);
            time = GetMS() - msStartTime + 1;
            nps = (int)GetNps(time);
            if ((!Glob.pondering && msMoveTime >= 0 && GetMS() - msStartTime >= msMoveTime)) {
                Glob.abortSearch = true;
                return;
            }
        }
    }

    // If Rodent is compiled as a single-threaded engine, Slowdown()
    // function assumes additional role and enforces time control
    // handling.

#ifndef USE_THREADS
    if ((!(Glob.nodes & 2047))
    && !Glob.is_testing
    &&   mRootDepth > 1) CheckTimeout();
#endif

    // for MultiPv

    if (Glob.multiPv > 1) {
        if ((!(Glob.nodes & 2047))
        && !Glob.isTesting
        &&   mRootDepth > 1) CheckTimeout();
    }

}

int POS::DrawScore() const {

    if (mSide == Par.programSide) return -Par.drawScore;
    else                          return  Par.drawScore;
}
