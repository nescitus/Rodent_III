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
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>

const int cEngine::razor_margin[5] = { 0, 300, 360, 420, 480 };
const int cEngine::fut_margin[7] = { 0, 100, 160, 220, 280, 340, 400 };
int cEngine::lmr_size[2][MAX_PLY][MAX_MOVES];

void cParam::InitAsymmetric(POS *p) {

    prog_side = p->side;

    if (prog_side == WC) { // TODO: no if/else, but progside/op_side
        sd_att[WC] = own_att_weight;
        sd_att[BC] = opp_att_weight;
        sd_mob[WC] = own_mob_weight;
        sd_mob[BC] = opp_mob_weight;
    } else {
        sd_att[BC] = own_att_weight;
        sd_att[WC] = opp_att_weight;
        sd_mob[BC] = own_mob_weight;
        sd_mob[WC] = opp_mob_weight;
    }
}

void cGlobals::ClearData() {

    ClearTrans();
#ifndef USE_THREADS
    EngineSingle.ClearAll();
#else
    for (auto& engine: Engines)
        engine.ClearAll();
#endif
    should_clear = false;
}

void cEngine::InitSearch() { // static init function

    // Set depth of late move reduction (formula based on Stockfish)

    for (int dp = 0; dp < MAX_PLY; dp++)
        for (int mv = 0; mv < MAX_MOVES; mv++) {

            int r;

            if (dp != 0 && mv != 0)
                r = (int)(log((double)dp) * log((double)Min(mv, 63)) / 2.0);
            else              // +-inf to int is undefined
                r = 0;

            lmr_size[0][dp][mv] = r;     // zero window node
            lmr_size[1][dp][mv] = r - 1; // principal variation node (checking for pos. values is in `Search()`)

            // reduction cannot exceed actual depth
            if (lmr_size[0][dp][mv] > dp - 1) lmr_size[0][dp][mv] = dp - 1;
            if (lmr_size[1][dp][mv] > dp - 1) lmr_size[1][dp][mv] = dp - 1;
        }
}

void cEngine::Think(POS *p) {

    POS curr[1];
    pv_eng[0] = 0; // clear engine's move
    pv_eng[1] = 0; // clear ponder move
    fl_root_choice = false;
    *curr = *p;
    AgeHist();
    Iterate(curr, pv_eng);
}

void cEngine::Iterate(POS *p, int *pv) {

    int cur_val = 0;

    // Lazy SMP works best with some depth variance,
    // so every other thread will search to depth + 1

    int offset = thread_id & 0x01;

    for (root_depth = 1 + offset; root_depth <= search_depth; root_depth++) {

        // If a thread is lagging behind too much, which makes it unlikely
        // to contribute to the final result, skip the iteration.

        if (Glob.depth_reached > dp_completed + 1) {
            dp_completed++;
            continue;
        }

        // Perform actual earch

        printf("info depth %d\n", root_depth);
        if (Par.search_skill > 6) cur_val = Widen(p, root_depth, pv, cur_val);
        else                      cur_val = Search(p, 0, -INF, INF, root_depth, 0, -1, -1, pv);
        if (Glob.abort_search) break;

        // Shorten search if there is only one root move available

        if (root_depth >= 8 && fl_root_choice == false) break;

        // Abort search on finding checkmate score

        if (cur_val > MAX_EVAL || cur_val < -MAX_EVAL) {
            int max_mate_depth = (MATE - Abs(cur_val) + 1) + 1;
            max_mate_depth *= 4;
            max_mate_depth /= 3;
            if (max_mate_depth <= root_depth) {
                dp_completed = root_depth;
                break;
            }
        }

        // Set information about depth

        dp_completed = root_depth;
        if (Glob.depth_reached < dp_completed)
            Glob.depth_reached = dp_completed;
    }

    if (!Par.shut_up) Glob.abort_search = true; // for correct exit from fixed depth search
}

// Aspiration search, progressively widening the window (based on Senpai 1.0)

int cEngine::Widen(POS *p, int depth, int *pv, int lastScore) {

    int cur_val = lastScore, alpha, beta;

    if (depth > 6 && lastScore < MAX_EVAL) {
        for (int margin = 10; margin < 500; margin *= 2) {
            alpha = lastScore - margin;
            beta  = lastScore + margin;
            cur_val = Search(p, 0, alpha, beta, depth, 0, -1, -1, pv);
            if (Glob.abort_search) break;
            if (cur_val > alpha && cur_val < beta)
                return cur_val;              // we have finished within the window
            if (cur_val > MAX_EVAL) break;   // verify mate searching with infinite bounds
        }
    }

    cur_val = Search(p, 0, -INF, INF, depth, 0, -1, -1, pv); // full window search
    return cur_val;
}

int cEngine::Search(POS *p, int ply, int alpha, int beta, int depth, int was_null, int last_move, int last_capt_sq, int *pv) {

    int best, score, null_score, move, new_depth, new_pv[MAX_PLY];
    int mv_type, fl_check, reduction, victim, last_capt;
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

    Glob.nodes++;
    local_nodes++;
    Slowdown();
    if (Glob.abort_search && root_depth > 1) return 0;
    if (ply) *pv = 0;
    if (IsDraw(p) && ply) return DrawScore(p);
    move = 0;

    // MATE DISTANCE PRUNING

    if (ply) {
        int checkmating_score = MATE - ply;
        if (checkmating_score < beta) {
            beta = checkmating_score;
            if (alpha >= checkmating_score)
                return alpha;
        }

        int checkmated_score = -MATE + ply;
        if (checkmated_score > alpha) {
            alpha = checkmated_score;
            if (beta <= checkmated_score)
                return beta;
        }
    }

    // RETRIEVE MOVE FROM TRANSPOSITION TABLE

    if (TransRetrieve(p->hash_key, &move, &score, alpha, beta, depth, ply)) {
        if (score >= beta) UpdateHistory(p, last_move, move, depth, ply);
        if (!is_pv && Par.search_skill > 0) return score;
    }

    // SAFEGUARD AGAINST REACHING MAX PLY LIMIT

    if (ply >= MAX_PLY - 1) {
        int eval = Evaluate(p, &e);
#ifdef USE_RISKY_PARAMETER
        eval = EvalScaleByDepth(p, ply, eval);
#endif
        return eval;
    }

    fl_check = InCheck(p);

    // CAN WE PRUNE THIS NODE?

    int fl_prunable_node = !fl_check
                        && !is_pv
                        && alpha > -MAX_EVAL
                        && beta < MAX_EVAL;

    // GET EVAL SCORE IF NEEDED FOR PRUNING/REDUCTION DECISIONS

    int eval = 0;
    if (fl_prunable_node
    && (!was_null || depth <= 6)) {
        eval = Evaluate(p, &e);
#ifdef USE_RISKY_PARAMETER
        eval = EvalScaleByDepth(p, ply, eval);
#endif
    }

    // BETA PRUNING / STATIC NULL MOVE

    if (fl_prunable_node
    && Par.search_skill > 7
    && depth <= 3
    && !was_null) {
        int sc = eval - 120 * depth;
        if (sc > beta) return sc;
    }

    // NULL MOVE

    if (depth > 1
    && Par.search_skill > 1
    && !was_null
    && fl_prunable_node
    && MayNull(p)
    && eval >= beta) {

        // null move depth reduction - modified Stockfish formula

        new_depth = depth - ((823 + 67 * depth) / 256)
                          - Min(3, (eval - beta) / 200);

        // omit null move search if normal search to the same depth wouldn't exceed beta
        // (sometimes we can check it for free via hash table)

        if (TransRetrieve(p->hash_key, &move, &null_score, alpha, beta, new_depth, ply)) {
            if (null_score < beta) goto avoid_null;
        }

        p->DoNull(u);
        if (new_depth <= 0) score = -QuiesceChecks(p, ply + 1, -beta, -beta + 1, new_pv);
        else                score = -Search(p, ply + 1, -beta, -beta + 1, new_depth, 1, 0, -1, new_pv);

        // get location of a piece whose capture refuted null move
        // its escape will be prioritised in the move ordering

        TransRetrieve(p->hash_key, &null_refutation, &null_score, alpha, beta, depth, ply);
        if (null_refutation > 0) ref_sq = Tsq(null_refutation);

        p->UndoNull(u);
        if (Glob.abort_search && root_depth > 1) return 0;

        // do not return unproved mate scores, Stockfish-style

        if (score >= MAX_EVAL) score = beta;

        if (score >= beta) {

            // verification search

            if (new_depth > 6 && Par.search_skill > 9)
                score = Search(p, ply, alpha, beta, new_depth - 5, 1, last_move, last_capt_sq, pv);

            if (Glob.abort_search && root_depth > 1) return 0;
            if (score >= beta) return score;
        }
    } // end of null move code

avoid_null:

    // RAZORING (based on Toga II 3.0)

    if (fl_prunable_node
    && Par.search_skill > 3
    && !move
    && !was_null
    && !(p->Pawns(p->side) & bb_rel_rank[p->side][RANK_7]) // no pawns to promote in one move
    && depth <= 4) {
        int threshold = beta - razor_margin[depth];

        if (eval < threshold) {
            score = QuiesceChecks(p, ply, alpha, beta, pv);
            if (score < threshold) return score;
        }
    } // end of razoring code

    // INTERNAL ITERATIVE DEEPENING

    if (is_pv
    && !fl_check
    && !move
    && depth > 6) {
        Search(p, ply, alpha, beta, depth - 2, 0, -1, last_capt_sq, pv);
        TransRetrieveMove(p->hash_key, &move);
    }

    // TODO: internal iterative deepening in cut nodes

    // PREPARE FOR MAIN SEARCH

    best = -INF;
    InitMoves(p, m, move, Refutation(move), ref_sq, ply);

    // MAIN LOOP

    while ((move = NextMove(m, &mv_type))) {

        // SET FUTILITY PRUNING FLAG
        // before the first applicable move is tried

        if (mv_type == MV_NORMAL
        && Par.search_skill > 4
        && quiet_tried == 0
        && fl_prunable_node
        && depth <= 6) {
           if (eval + fut_margin[depth] < beta) fl_futility = 1;
        }

        // MAKE MOVE

        mv_hist_score = history[p->pc[Fsq(move)]][Tsq(move)];
        victim = TpOnSq(p, Tsq(move));
        if (victim != NO_TP) last_capt = Tsq(move);
        else last_capt = -1;
        p->DoMove(move, u);
        if (Illegal(p)) { p->UndoMove(move, u); continue; }

        // GATHER INFO ABOUT THE MOVE

        mv_played[mv_tried] = move;
        mv_tried++;
        if (!ply && mv_tried > 1) fl_root_choice = true;
        if (mv_type == MV_NORMAL) quiet_tried++;
        if (ply == 0 && !Par.shut_up && depth > 16 && Glob.thread_no == 1)
            DisplayCurrmove(move, mv_tried);

        // SET NEW SEARCH DEPTH

        new_depth = depth - 1;

        // EXTENSIONS

        if (is_pv || depth < 9) {
            new_depth += InCheck(p);                                // check extension, pv or low depth
            if (is_pv && Tsq(move) == last_capt_sq) new_depth += 1; // recapture extension in pv
            if (is_pv && depth < 6 && TpOnSq(p, Tsq(move)) == P     // pawn to 7th extension at the tips of pv
            && (SqBb(Tsq(move)) & (RANK_2_BB | RANK_7_BB))) new_depth += 1;
        }

        // FUTILITY PRUNING

        if (fl_futility
        && !InCheck(p)
        && mv_hist_score < Par.hist_limit
        && (mv_type == MV_NORMAL)
        &&  mv_tried > 1) {
            p->UndoMove(move, u); continue;
        }

        // LATE MOVE PRUNING

        if (fl_prunable_node
        && Par.search_skill > 5
        && depth < 4
        && quiet_tried > 3 * depth
        && !InCheck(p)
        && mv_hist_score < Par.hist_limit
        && mv_type == MV_NORMAL) {
            p->UndoMove(move, u); continue;
        }

        // LMR 1: NORMAL MOVES

        reduction = 0;

        if (depth > 2
        && Par.search_skill > 2
        && mv_tried > 3
        && !fl_check
        && !InCheck(p)
        && lmr_size[is_pv][depth][mv_tried] > 0
        && mv_type == MV_NORMAL
        && mv_hist_score < Par.hist_limit
        && MoveType(move) != CASTLE) {
            reduction = (int)lmr_size[is_pv][depth][mv_tried];

            // increase reduction on bad history score

            if (mv_hist_score < 0
            && new_depth - reduction > 2)
                reduction++;

            // TODO: decrease reduction of moves with good history score

            new_depth = new_depth - reduction;
        }

        // LMR 2: MARGINAL REDUCTION OF BAD CAPTURES

        if (depth > 2
        && Par.search_skill > 8
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
            score = -Search(p, ply + 1, -beta, -alpha, new_depth, 0, move, last_capt, new_pv);
        else {
            score = -Search(p, ply + 1, -alpha - 1, -alpha, new_depth, 0, move, last_capt, new_pv);
            if (!Glob.abort_search && score > alpha && score < beta)
                score = -Search(p, ply + 1, -beta, -alpha, new_depth, 0, move, last_capt, new_pv);
        }

        // DON'T REDUCE A MOVE THAT SCORED ABOVE ALPHA

        if (score > alpha && reduction) {
            new_depth = new_depth + reduction;
            reduction = 0;
            goto research;
        }

        // UNDO MOVE

        p->UndoMove(move, u);
        if (Glob.abort_search && root_depth > 1) return 0;

        // BETA CUTOFF

        if (score >= beta) {
            if (!fl_check) {
                UpdateHistory(p, last_move, move, depth, ply);
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
        return InCheck(p) ? -MATE + ply : DrawScore(p);

    // SAVE RESULT IN THE TRANSPOSITION TABLE

    if (*pv) {
        if (!fl_check) {
            UpdateHistory(p, last_move, *pv, depth, ply);
            for (int mv = 0; mv < mv_tried; mv++) {
                DecreaseHistory(p, mv_played[mv], depth);
            }
        }
        TransStore(p->hash_key, *pv, best, EXACT, depth, ply);
    } else
        TransStore(p->hash_key, 0, best, UPPER, depth, ply);

    return best;
}

U64 GetNps(int elapsed) {

    U64 nps = 0;
    if (elapsed) nps = (Glob.nodes * 1000) / elapsed;
    return nps;
}

void DisplayCurrmove(int move, int tried) {

    if (!Glob.is_console) {
        printf("info currmove ");
        PrintMove(move);
        printf(" currmovenumber %d \n", tried);
    }
}

void cEngine::DisplayPv(int score, int *pv) {

    // don't display information from threads that are late

    if (root_depth < Glob.depth_reached) return;

    const char *type; char pv_str[512];
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

#if __cplusplus >= 201103L || _MSVC_LANG >= 201402
    printf("info depth %d time %d nodes %" PRIu64 " nps %" PRIu64 " score %s %d pv %s\n",
           root_depth, elapsed, (U64)Glob.nodes, nps, type, score, pv_str);
#else
    #if defined _WIN32 || defined _WIN64
        printf("info depth %d time %d nodes %I64d nps %I64d score %s %d pv %s\n",
               root_depth, elapsed, (U64)Glob.nodes, nps, type, score, pv_str);
    #else
        printf("info depth %d time %d nodes %lld nps %lld score %s %d pv %s\n",
               root_depth, elapsed, (U64)Glob.nodes, nps, type, score, pv_str);
    #endif
#endif
}

void CheckTimeout() {

    char command[80];

    if (InputAvailable()) {
        ReadLine(command, sizeof(command));
        if (strcmp(command, "stop") == 0)
            Glob.abort_search = true;
        else if (strcmp(command, "quit") == 0) {
#ifndef USE_THREADS
            exit(0);
#else
            Glob.abort_search = true;
            Glob.goodbye = true; // will crash if just `exit()`. should wait until threads are terminated
#endif
        }
        else if (strcmp(command, "ponderhit") == 0)
            Glob.pondering = false;
    }

    if (!Glob.pondering && move_time >= 0 && GetMS() - start_time >= move_time)
        Glob.abort_search = true;
}

void cEngine::Slowdown() {

    // Handling search limited by the number of nodes

    if (move_nodes > 0) {
        if (Glob.nodes >= move_nodes)
            Glob.abort_search = true;
    }

    // Handling slowdown for weak levels

    if (Par.nps_limit > 0) {
        if (Par.nps_limit && root_depth > 1) {
            int time = GetMS() - start_time + 1;
            int nps = GetNps(time);
            while ((int)nps > Par.nps_limit) {
                WasteTime(10);
                time = GetMS() - start_time + 1;
                nps = GetNps(time);
                if ((!Glob.pondering && move_time >= 0 && GetMS() - start_time >= move_time)) {
                    Glob.abort_search = true;
                    return;
                }
            }
        }
    }

    // If Rodent is compiled as a single-threaded engine, Slowdown()
    // function assumes additional role and it enforces time control
    // handling.

#ifndef USE_THREADS
    if ( (!(Glob.nodes & 2047))
    &&   !Glob.is_testing
    &&   root_depth > 1) CheckTimeout();
#endif

}

int DrawScore(POS *p) {

    if (p->side == Par.prog_side) return -Par.draw_score;
    else                          return  Par.draw_score;
}
