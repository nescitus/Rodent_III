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
#include <stdio.h>
#include <math.h>

void cParam::DefaultWeights(void) {
 
   // Switch off weakening parameters

   search_skill = 10;
   nps_limit = 0;
   fl_weakening = 0;
   elo = 2800;
   eval_blur = 0;

   // Opening book

   use_book = 1;
   book_filter = 20;

   // Timing

   time_percentage = 100;

   // Piece values

   pc_value[P] = 100;
   pc_value[N] = 325;
   pc_value[B] = 335;
   pc_value[R] = 500;
   pc_value[Q] = 1000;
   pc_value[K] = 0;
   pc_value[K+1] = 0;

   // Tendency to keep own pieces

   keep_pc[P] = 0;
   keep_pc[N] = 0;
   keep_pc[B] = 0;
   keep_pc[R] = 0;
   keep_pc[Q] = 0;
   keep_pc[K] = 0;
   keep_pc[K+1] = 0;

   // Material adjustments

   bish_pair = 50;
   exchange_imbalance = 25;
   n_likes_closed = 6;
   r_likes_open = 3;

   // Varia

   mat_weight = 100;
   pst_weight = 80;
   pst_style = 0;
   mob_style = 0;         // 1 is only marginally behind
   protecting_bishop = 0; // flavour option
   riskydepth = 0;
   draw_score = 0;
   shut_up = 0;           // surpress displaing info currmove etc.

   // Asymmetric weights - the core of personality mechanism

   own_att_weight = 110;
   opp_att_weight = 100;
   own_mob_weight = 100;
   opp_mob_weight = 110;

   // Positional weights

   threats_weight = 100;
   tropism_weight = 20;
   forward_weight = 0;
   passers_weight = 100;
   outposts_weight = 100;
   lines_weight = 100;
   struct_weight = 100;
   shield_weight = 120;
   storm_weight = 100;

   // Pawn structure parameters

   doubled_mg = -12;
   doubled_eg = -24;
   isolated_mg = -10;
   isolated_eg = -20;
   isolated_open = -10;
   backward_mg = -8;
   backward_eg = -8;
   backward_open = -8;

   // Specialized functions

   InitPst();
   InitMobility();
   InitMaterialTweaks();
   InitBackward();

   // History limit to prunings and reductions

   hist_perc = 175;
   hist_limit = 24576;

   // when testing a personality, place changes in relation to default here

}

void cParam::InitBackward(void) {

   backward_malus_mg[FILE_A] = backward_mg + 3;
   backward_malus_mg[FILE_B] = backward_mg + 1;
   backward_malus_mg[FILE_C] = backward_mg - 1;
   backward_malus_mg[FILE_D] = backward_mg - 3;
   backward_malus_mg[FILE_E] = backward_mg - 3;
   backward_malus_mg[FILE_F] = backward_mg - 1;
   backward_malus_mg[FILE_G] = backward_mg + 1;
   backward_malus_mg[FILE_H] = backward_mg + 3;
}

void cParam::InitPst(void) {

  for (int sq = 0; sq < 64; sq++) {
    for (int sd = 0; sd < 2; sd++) {
 
      mg_pst[sd][P][REL_SQ(sq, sd)] = ((pc_value[P] * mat_weight) / 100) + ( pstPawnMg  [pst_style][sq] * pst_weight) / 100;
      eg_pst[sd][P][REL_SQ(sq, sd)] = ((pc_value[P] * mat_weight) / 100) + ( pstPawnEg  [pst_style][sq] * pst_weight) / 100;
      mg_pst[sd][N][REL_SQ(sq, sd)] = ((pc_value[N] * mat_weight) / 100) + ( pstKnightMg[pst_style][sq] * pst_weight) / 100;
      eg_pst[sd][N][REL_SQ(sq, sd)] = ((pc_value[N] * mat_weight) / 100) + ( pstKnightEg[pst_style][sq] * pst_weight) / 100;
      mg_pst[sd][B][REL_SQ(sq, sd)] = ((pc_value[B] * mat_weight) / 100) + ( pstBishopMg[pst_style][sq] * pst_weight) / 100;
      eg_pst[sd][B][REL_SQ(sq, sd)] = ((pc_value[B] * mat_weight) / 100) + ( pstBishopEg[pst_style][sq] * pst_weight) / 100;
      mg_pst[sd][R][REL_SQ(sq, sd)] = ((pc_value[R] * mat_weight) / 100) + ( pstRookMg  [pst_style][sq] * pst_weight) / 100;
      eg_pst[sd][R][REL_SQ(sq, sd)] = ((pc_value[R] * mat_weight) / 100) + ( pstRookEg  [pst_style][sq] * pst_weight) / 100;
      mg_pst[sd][Q][REL_SQ(sq, sd)] = ((pc_value[Q] * mat_weight) / 100) + ( pstQueenMg [pst_style][sq] * pst_weight) / 100;
      eg_pst[sd][Q][REL_SQ(sq, sd)] = ((pc_value[Q] * mat_weight) / 100) + ( pstQueenEg [pst_style][sq] * pst_weight) / 100;
      mg_pst[sd][K][REL_SQ(sq, sd)] =                                      ( pstKingMg  [pst_style][sq] * pst_weight) / 100;
      eg_pst[sd][K][REL_SQ(sq, sd)] =                                      ( pstKingEg  [pst_style][sq] * pst_weight) / 100;

      sp_pst[sd][N][REL_SQ(sq, sd)] = pstKnightOutpost[sq];
      sp_pst[sd][B][REL_SQ(sq, sd)] = pstBishopOutpost[sq];
      sp_pst[sd][DEF_MG][REL_SQ(sq, sd)] = pstDefendedPawnMg[sq];
      sp_pst[sd][PHA_MG][REL_SQ(sq, sd)] = pstPhalanxPawnMg[sq];
      sp_pst[sd][DEF_EG][REL_SQ(sq, sd)] = pstDefendedPawnEg[sq];
      sp_pst[sd][PHA_EG][REL_SQ(sq, sd)] = pstPhalanxPawnEg[sq];
    }
  }
}

void cParam::InitMobility(void) {

  for (int i = 0; i < 9; i++) {
    n_mob_mg[i] = Par.mob_style == 0 ? 4 * (i-4) : n_mob_mg_decreasing[i];
    n_mob_eg[i] = Par.mob_style == 0 ? 4 * (i-4) : n_mob_eg_decreasing[i];
  }
    
  for (int i = 0; i < 14; i++) {
    b_mob_mg[i] = Par.mob_style == 0 ? 5 * (i-6) : b_mob_mg_decreasing[i];
    b_mob_eg[i] = Par.mob_style == 0 ? 5 * (i-6) : b_mob_eg_decreasing[i];
  }

  for (int i = 0; i < 15; i++) {
    r_mob_mg[i] = Par.mob_style == 0 ? 2 * (i-7) : r_mob_mg_decreasing[i];
    r_mob_eg[i] = Par.mob_style == 0 ? 4 * (i-7) : r_mob_eg_decreasing[i];
  }

  for (int i = 0; i < 28; i++) {
    q_mob_mg[i] = Par.mob_style == 0 ? 1 * (i-14) : q_mob_mg_decreasing[i];
    q_mob_eg[i] = Par.mob_style == 0 ? 2 * (i-14) : q_mob_mg_decreasing[i];
  }

}

void cParam::InitMaterialTweaks(void) {

  // Init tables for adjusting piece values 
  // according to the number of own pawns

  for (int i = 0; i < 9; i++) {
    np_table[i] = adj[i] * n_likes_closed;
    rp_table[i] = adj[i] * r_likes_open;
  }

  // Init imbalance table, so that we can expose option for exchange delta

  for (int i = 0; i < 9; i++) {
    for (int j = 0; j < 9; j++) {

      // insert original values
      imbalance[i][j] = imbalance_data[i][j];

      // insert value defined by the user
      if (imbalance[i][j] == Ex) imbalance[i][j] = exchange_imbalance;
      if (imbalance[i][j] == -Ex) imbalance[i][j] = -exchange_imbalance;
    }
  }
}

void cParam::InitTables(void) {

  // Init king attack table

  for (int t = 0, i = 1; i < 511; ++i) {
    t = (int)Min(1280.0, Min((0.027 * i * i), t + 8.0));
    danger[i] = (t * 100) / 256; // rescale to centipawns
  }
}

void cParam::SetSpeed(int elo) {
   nps_limit = 0;
   eval_blur = 0;

   if (Par.fl_weakening) {
      nps_limit = EloToSpeed(elo);
	  Par.eval_blur = EloToBlur(elo);
   }
}

int cParam::EloToSpeed(int elo) {

  if (elo >= 1800) {
    int mul = (elo - 1600) / 2;
	return (10 * mul);
  }

  if (elo >= 1000) {
    return 50 + (80 * (elo - 1000) / 100);
  }

  int result = 300 + (int) pow(2, (elo - 799) / 85);
  result *= 0.23;

  if (elo < 1400) result *= 0.95;
  if (elo < 1300) result *= 0.95;
  if (elo < 1200) result *= 0.95;
  if (elo < 1100) result *= 0.95;
  if (elo < 1000) result *= 0.95;
  if (elo <  900) result *= 0.95;

  return result;
}

int cParam::EloToBlur(int elo) {
  if (elo < 2000) return (2000 - elo) / 5;
  return 0;
}

void cEngine::Init(int th) {

  thread_id = th;
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