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
#include "eval.h"
#include <cstdlib>
#include <cmath>

void cParam::DefaultWeights() {

    // Switch off weakening parameters

    search_skill = 10;
    nps_limit = 0;
    fl_weakening = false;
    elo = 2800;
    eval_blur = 0;
    book_depth = 256;

    // Opening book

    use_book = true;
    book_filter = 20;

    // Timing

    time_percentage = 100;

    // Piece values
	// 0.059156

    values[P_MID] = 95;   // 95
    values[N_MID] = 310;  // 310
    values[B_MID] = 321;  // 320
    values[R_MID] = 514;  // 515
    values[Q_MID] = 1000;

    values[P_END] = 109;  // 106
    values[N_END] = 305;  // 305
    values[B_END] = 320;  // 320
    values[R_END] = 526;  // 520
    values[Q_END] = 1011; // 1010

    // Tendency to keep own pieces

    keep_pc[P] = 0;
    keep_pc[N] = 0;
    keep_pc[B] = 0;
    keep_pc[R] = 0;
    keep_pc[Q] = 0;
    keep_pc[K] = 0;
    keep_pc[K + 1] = 0;

    // Material adjustments

    values[B_PAIR]  = 50;
    values[N_PAIR]  = -9;
    values[R_PAIR]  = -9;
    values[ELEPH]  = 4;  // queen loses that much with each enemy minor on the board
    values[A_EXC]  = 26; // exchange advantage additional bonus
    values[A_MIN] = 53;  // additional bonus for minor piece advantage
    values[A_MAJ] = 60;  // additional bonus for major piece advantage
    values[A_TWO] = 44;  // additional bonus for two minors for a rook
    values[A_ALL] = 80;  // additional bonus for advantage in both majors and minors
    values[N_CL]  = 7;   // knight gains this much with each own pawn present on th board
    values[R_OP]  = 3;   // rook loses that much with each own pawn present on the board

    // King attack values

	// "_ATT1" values are awarded for attacking squares not defended by enemy pawns
	// "_ATT2" values are awarded for attacking squares defended by enemy pawns
	// "_CHK"  values are awarded for threatening check to enemy king
	// "_CONTACT" values are awarded for contact checks threats
	//
	// All these values are NOT the actual bonuses; their sum is used as index 
	// to a non-linear king safety table. Tune them with extreme caution.

	values[N_ATT1] = 6;  // 6
	values[N_ATT2] = 3;  // 2
	values[B_ATT1] = 6;  // 6
	values[B_ATT2] = 2;  // 2
    values[R_ATT1] = 9;  // 9
    values[R_ATT2] = 4;  // 3
    values[Q_ATT1] = 16; // 15
    values[Q_ATT2] = 5;  // 5

	values[N_CHK] = 4;   // 4
	values[B_CHK] = 6;   // 4
	values[R_CHK] = 11;  // 9
	values[Q_CHK] = 12;  // 12

	values[R_CONTACT] = 24; // 24
	values[Q_CONTACT] = 36; // 36

    // King tropism

    values[NTR_MG] = 3;
    values[NTR_EG] = 3;
    values[BTR_MG] = 2;
    values[BTR_EG] = 1;
    values[RTR_MG] = 2;
    values[RTR_EG] = 1;
    values[QTR_MG] = 2;
    values[QTR_EG] = 4;

    // Varia

    mat_weight = 100;
	pst_weight = 80;
    pst_style = 0;
    mob_style = 0;         // 1 is only marginally behind

#ifdef USE_RISKY_PARAMETER
    riskydepth = 0;
#endif
    draw_score = 0;
    shut_up = false;           // suppress displaing info currmove etc.

    // Asymmetric weights - the core of personality mechanism

    own_att_weight = 110;
    opp_att_weight = 100;
    own_mob_weight = 100;
    opp_mob_weight = 110;

    // Positional weights

    threats_weight = 107;
    tropism_weight = 20;
    forward_weight = 0;
    passers_weight = 100;
	pawn_mass_weight = 100; // seems optimal
	pawn_chains_weight = 100;
    outposts_weight = 95;
    lines_weight = 100;
    struct_weight = 100;
    shield_weight = 120;
    storm_weight = 100;

    // Pawn structure parameters

    values[DB_MID] = -12;  // doubled
    values[DB_END] = -24;
    values[ISO_MG] = -10;  // isolated
    values[ISO_EG] = -20;
    values[ISO_OF] = -10;  // additional midgame penalty for isolated pawn on an open file
    values[BK_MID] = -8;   // backward
    values[BK_END] = -8;
    values[BK_OPE] = -8;   // additional midgame penalty for backward pawn on an open file
	values[P_BIND] = 5;    // two pawns control central square
	values[P_ISL] = 7;     // penalty for each pawn island

    // Passed pawn parameters

    values[PMG2] = 12;
    values[PMG3] = 12;
	values[PMG4] = 30;
	values[PMG5] = 50;
	values[PMG6] = 80;
	values[PMG7] = 130;

	values[PEG2] = 24;
	values[PEG3] = 24;
	values[PEG4] = 60;
	values[PEG5] = 100;
	values[PEG6] = 160;
	values[PEG7] = 260;

    // Knight parameters

    values[N_TRAP] = -150;
    values[N_BLOCK] = -20; // knight blocks c pawn in queen pawn openings
	values[N_OWH] = -5;    // knight can move only to own half of the board
	values[N_REACH] = 2;   // knight can reach an outpost square


    // Bishop parameters

    values[B_FIANCH] = 4;  // general bonus for fianchettoed bishop
    values[B_KING] = 5;    // fianchettoed bishop near king: 0
    values[B_BADF] = -20;  // enemy pawns hamper fianchettoed bishop
    values[B_TRAP_A2] = -150;
    values[B_TRAP_A3] = -50;
    values[B_BLOCK] = -50; // blocked pawn at d2/e2 hampers bishop's develomement
    values[B_BF_MG] = -10; // fianchettoed bishop blocked by own pawn (ie. Bg2, Pf3)
    values[B_BF_EG] = -20;
    values[B_WING] = 10;   // bishop on "expected" wing (ie. Pe4, Bc5/b5/a4/b3/c2)
	values[B_OVH] = -5;    // bishop can move only to own half of the board
	values[B_REACH] = 2;   // bishop can reach an outpost square
	values[B_TOUCH] = 4;   // two bishops on adjacent squares

    // Rook parameters

    values[RSR_MG] = 16; // rook on 7th rank
    values[RSR_EG] = 32;
    values[RS2_MG] = 8;  // additional bonus for two rooks on 7th rank
    values[RS2_EG] = 16;
    values[ROF_MG] = 14; // rook on open file
    values[ROF_EG] = 14;
    values[RGH_MG] = 7;  // rook on half-open file with undefended enemy pawn
    values[RGH_EG] = 7;
    values[RBH_MG] = 5;  // rook on half-open file with defended enemy pawn
    values[RBH_EG] = 5;
    values[ROQ_MG] = 5;  // rook and queen on the same file, open or closed
    values[ROQ_EG] = 5;
    values[R_BLOCK] = -50;

    // Queen parameters

    values[QSR_MG] = 4;  // queen on the 7th rank
    values[QSR_EG] = 8;

    // King parameters

    values[K_NO_LUFT] = -15;
    values[K_CASTLE] = 10;

    // Forwardness parameters

    values[N_FWD] = 1;
    values[B_FWD] = 1;
    values[R_FWD] = 2;
    values[Q_FWD] = 4;

    // Specialized functions

    InitPst();
    InitMobility();
    InitMaterialTweaks();
    InitBackward();
	InitPassers();

    // History limit to prunings and reductions

    hist_perc = 175;
    hist_limit = 24576;

    // when testing a personality, place changes in relation to default below:

}

void cParam::InitPassers() {

	passed_bonus_mg[WC][0] = 0;                passed_bonus_mg[BC][7] = 0;
	passed_bonus_mg[WC][1] = values[PMG2];     passed_bonus_mg[BC][6] = values[PMG2];
	passed_bonus_mg[WC][2] = values[PMG3];     passed_bonus_mg[BC][5] = values[PMG3];
	passed_bonus_mg[WC][3] = values[PMG4];     passed_bonus_mg[BC][4] = values[PMG4];
	passed_bonus_mg[WC][4] = values[PMG5];     passed_bonus_mg[BC][3] = values[PMG5];
	passed_bonus_mg[WC][5] = values[PMG6];     passed_bonus_mg[BC][2] = values[PMG6];
	passed_bonus_mg[WC][6] = values[PMG7];     passed_bonus_mg[BC][1] = values[PMG7];
	passed_bonus_mg[WC][7] = 0;                passed_bonus_mg[BC][0] = 0;

	passed_bonus_eg[WC][0] = 0;                passed_bonus_eg[BC][7] = 0;
	passed_bonus_eg[WC][1] = values[PEG2];     passed_bonus_eg[BC][6] = values[PEG2];
	passed_bonus_eg[WC][2] = values[PEG3];     passed_bonus_eg[BC][5] = values[PEG3];
	passed_bonus_eg[WC][3] = values[PEG4];     passed_bonus_eg[BC][4] = values[PEG4];
	passed_bonus_eg[WC][4] = values[PEG5];     passed_bonus_eg[BC][3] = values[PEG5];
	passed_bonus_eg[WC][5] = values[PEG6];     passed_bonus_eg[BC][2] = values[PEG6];
	passed_bonus_eg[WC][6] = values[PEG7];     passed_bonus_eg[BC][1] = values[PEG7];
	passed_bonus_eg[WC][7] = 0;                passed_bonus_eg[BC][0] = 0;
}

void cParam::InitBackward() {

    // add file-dependent component to backward pawns penalty
    // (assuming backward pawns on central files are bigger liability)

    backward_malus_mg[FILE_A] = values[BK_MID] + 3;
    backward_malus_mg[FILE_B] = values[BK_MID] + 1;
    backward_malus_mg[FILE_C] = values[BK_MID] - 1;
    backward_malus_mg[FILE_D] = values[BK_MID] - 3;
    backward_malus_mg[FILE_E] = values[BK_MID] - 3;
    backward_malus_mg[FILE_F] = values[BK_MID] - 1;
    backward_malus_mg[FILE_G] = values[BK_MID] + 1;
    backward_malus_mg[FILE_H] = values[BK_MID] + 3;
}

void cParam::InitPst() {

    for (int sq = 0; sq < 64; sq++) {
        for (int sd = 0; sd < 2; sd++) {

            mg_pst[sd][P][REL_SQ(sq, sd)] = ((values[P_MID] * mat_weight) / 100) + (pstPawnMg  [pst_style][sq] * pst_weight) / 100;
            eg_pst[sd][P][REL_SQ(sq, sd)] = ((values[P_END] * mat_weight) / 100) + (pstPawnEg  [pst_style][sq] * pst_weight) / 100;
            mg_pst[sd][N][REL_SQ(sq, sd)] = ((values[N_MID] * mat_weight) / 100) + (pstKnightMg[pst_style][sq] * pst_weight) / 100;
            eg_pst[sd][N][REL_SQ(sq, sd)] = ((values[N_END] * mat_weight) / 100) + (pstKnightEg[pst_style][sq] * pst_weight) / 100;
            mg_pst[sd][B][REL_SQ(sq, sd)] = ((values[B_MID] * mat_weight) / 100) + (pstBishopMg[pst_style][sq] * pst_weight) / 100;
            eg_pst[sd][B][REL_SQ(sq, sd)] = ((values[B_END] * mat_weight) / 100) + (pstBishopEg[pst_style][sq] * pst_weight) / 100;
            mg_pst[sd][R][REL_SQ(sq, sd)] = ((values[R_MID] * mat_weight) / 100) + (pstRookMg  [pst_style][sq] * pst_weight) / 100;
            eg_pst[sd][R][REL_SQ(sq, sd)] = ((values[R_END] * mat_weight) / 100) + (pstRookEg  [pst_style][sq] * pst_weight) / 100;
            mg_pst[sd][Q][REL_SQ(sq, sd)] = ((values[Q_MID] * mat_weight) / 100) + (pstQueenMg [pst_style][sq] * pst_weight) / 100;
            eg_pst[sd][Q][REL_SQ(sq, sd)] = ((values[Q_END] * mat_weight) / 100) + (pstQueenEg [pst_style][sq] * pst_weight) / 100;
            mg_pst[sd][K][REL_SQ(sq, sd)] = (pstKingMg  [pst_style][sq] * pst_weight) / 100;
            eg_pst[sd][K][REL_SQ(sq, sd)] = (pstKingEg  [pst_style][sq] * pst_weight) / 100;

            sp_pst[sd][N][REL_SQ(sq, sd)] = pstKnightOutpost[sq];
            sp_pst[sd][B][REL_SQ(sq, sd)] = pstBishopOutpost[sq];
            sp_pst[sd][DEF_MG][REL_SQ(sq, sd)] = pstDefendedPawnMg[sq];
            sp_pst[sd][PHA_MG][REL_SQ(sq, sd)] = pstPhalanxPawnMg[sq];
            sp_pst[sd][DEF_EG][REL_SQ(sq, sd)] = pstDefendedPawnEg[sq];
            sp_pst[sd][PHA_EG][REL_SQ(sq, sd)] = pstPhalanxPawnEg[sq];
        }
    }
}

void cParam::InitMobility() {

    for (int i = 0; i < 9; i++) {
        n_mob_mg[i] = Par.mob_style == 0 ? 4 * (i - 4) : n_mob_mg_decreasing[i];
        n_mob_eg[i] = Par.mob_style == 0 ? 4 * (i - 4) : n_mob_eg_decreasing[i];
    }

    for (int i = 0; i < 14; i++) {
        b_mob_mg[i] = Par.mob_style == 0 ? 5 * (i - 6) : b_mob_mg_decreasing[i];
        b_mob_eg[i] = Par.mob_style == 0 ? 5 * (i - 6) : b_mob_eg_decreasing[i];
    }

    for (int i = 0; i < 15; i++) {
        r_mob_mg[i] = Par.mob_style == 0 ? 2 * (i - 7) : r_mob_mg_decreasing[i];
        r_mob_eg[i] = Par.mob_style == 0 ? 4 * (i - 7) : r_mob_eg_decreasing[i];
    }

    for (int i = 0; i < 28; i++) {
        q_mob_mg[i] = Par.mob_style == 0 ? 1 * (i - 14) : q_mob_mg_decreasing[i];
        q_mob_eg[i] = Par.mob_style == 0 ? 2 * (i - 14) : q_mob_mg_decreasing[i];
    }

}

void cParam::InitMaterialTweaks() {

    // Init tables for adjusting piece values
    // according to the number of own pawns

    for (int i = 0; i < 9; i++) {
        np_table[i] = adj[i] * values[N_CL];
        rp_table[i] = adj[i] * values[R_OP];
    }

    // Init imbalance table, so that we can expose option for exchange delta

    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {

            // insert original values
            imbalance[i][j] = imbalance_data[i][j];

            // insert value defined in Par.values

            if (imbalance_data[i][j] == A_EXC) imbalance[i][j] = values[A_EXC];
            if (imbalance_data[i][j] == -A_EXC) imbalance[i][j] = -values[A_EXC];
            if (imbalance_data[i][j] == A_MIN) imbalance[i][j] = values[A_MIN];
            if (imbalance_data[i][j] == -A_MIN) imbalance[i][j] = -values[A_MIN];
            if (imbalance_data[i][j] == A_MAJ) imbalance[i][j] = values[A_MAJ];
            if (imbalance_data[i][j] == -A_MAJ) imbalance[i][j] = -values[A_MAJ];
            if (imbalance_data[i][j] == A_TWO) imbalance[i][j] = values[A_TWO];
            if (imbalance_data[i][j] == -A_TWO) imbalance[i][j] = -values[A_TWO];
            if (imbalance_data[i][j] == A_ALL) imbalance[i][j] = values[A_ALL];
            if (imbalance_data[i][j] == -A_ALL) imbalance[i][j] = -values[A_ALL];
        }
    }
}

void cParam::InitTables() {

    // Init king attack table

    for (int t = 0, i = 1; i < 511; ++i) {
        t = (int)Min(1280.0, Min((0.027 * i * i), t + 8.0));
        danger[i] = (t * 100) / 256; // rescale to centipawns
    }
}

void cParam::SetSpeed(int elo) {
    nps_limit = 0;
    eval_blur = 0;

    if (fl_weakening) {
        nps_limit = EloToSpeed(elo);
        eval_blur = EloToBlur(elo);
        book_depth = EloToBookDepth(elo);
    }
}

int cParam::EloToSpeed(int elo) {

    // this formula abuses Michael Byrne's code from CraftySkill.
	// He used  it to calculate max nodes per elo. By  dividing,
	// I derive speed that yields similar result in standart blitz.
    // Formula has a little bit of built-in randomness.

	int lower_elo = elo - 25;
	int upper_elo = elo + 25;
	int use_rating = rand() % (upper_elo - lower_elo + 1) + lower_elo;
	int search_nodes = pow(1.0069555500567, (((use_rating) / 1200) - 1)
		+ (use_rating - 1200)) * 128;

	return search_nodes / 7;
}

int cParam::EloToBlur(int elo) {

    // Weaker levels get their evaluation blurred

    if (elo < 1500) return (1500 - elo) / 4;
    return 0;
}

int cParam::EloToBookDepth(int elo) {
    if (elo < 2000) return (elo - 700) / 100;
    return 256;
}

void cDistance::Init() {

    // Init distance tables

    for (int sq1 = 0; sq1 < 64; ++sq1) {
        for (int sq2 = 0; sq2 < 64; ++sq2) {
            int r_delta = Abs(Rank(sq1) - Rank(sq2));
            int f_delta = Abs(File(sq1) - File(sq2));
            bonus[sq1][sq2] = 14 - (r_delta + f_delta);  // for king tropism evaluation
            metric[sq1][sq2] = Max(r_delta, f_delta);    // chebyshev distance for unstoppable passers
        }
    }
}

void cParam::SetVal(int slot, int val) {
    values[slot] = val;
}
