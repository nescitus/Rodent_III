#include "rodent.h"
#include "eval.h"

void cParam::Default(void) {
  mat_weight = 100;
  placement_weight = 80;
  tropism_weight = 0;
  np_bonus = 6;
  rp_malus = 3;
}

void cParam::InitPst(void) {

  int pst_type = 0;

  for (int sq = 0; sq < 64; sq++) {
    for (int sd = 0; sd < 2; sd++) {

      mg_pst[sd][P][REL_SQ(sq, sd)] = ((100 * Par.mat_weight) / 100) + ((pstPawnMg[pst_type][sq] * Par.placement_weight) / 100);
      eg_pst[sd][P][REL_SQ(sq, sd)] = ((100 * Par.mat_weight) / 100) + ((pstPawnEg[pst_type][sq] * Par.placement_weight) / 100);
      mg_pst[sd][N][REL_SQ(sq, sd)] = ((325 * Par.mat_weight) / 100) + ((pstKnightMg[pst_type][sq] * Par.placement_weight) / 100);
      eg_pst[sd][N][REL_SQ(sq, sd)] = ((325 * Par.mat_weight) / 100) + ((pstKnightEg[pst_type][sq] * Par.placement_weight) / 100);
      mg_pst[sd][B][REL_SQ(sq, sd)] = ((325 * Par.mat_weight) / 100) + ((pstBishopMg[pst_type][sq] * Par.placement_weight) / 100);
      eg_pst[sd][B][REL_SQ(sq, sd)] = ((325 * Par.mat_weight) / 100) + ((pstBishopEg[pst_type][sq] * Par.placement_weight) / 100);
      mg_pst[sd][R][REL_SQ(sq, sd)] = ((500 * Par.mat_weight) / 100) + ((pstRookMg[pst_type][sq] * Par.placement_weight) / 100);
      eg_pst[sd][R][REL_SQ(sq, sd)] = ((500 * Par.mat_weight) / 100) + ((pstRookEg[pst_type][sq] * Par.placement_weight) / 100);
      mg_pst[sd][Q][REL_SQ(sq, sd)] = ((975 * Par.mat_weight) / 100) + ((pstQueenMg[pst_type][sq] * Par.placement_weight) / 100);
      eg_pst[sd][Q][REL_SQ(sq, sd)] = ((975 * Par.mat_weight) / 100) + ((pstQueenEg[pst_type][sq] * Par.placement_weight) / 100);
      mg_pst[sd][K][REL_SQ(sq, sd)] = ((pstKingMg[pst_type][sq] * Par.placement_weight) / 100);
      eg_pst[sd][K][REL_SQ(sq, sd)] = ((pstKingEg[pst_type][sq] * Par.placement_weight) / 100);

	  sp_pst[sd][N][REL_SQ(sq, sd)] = pstKnightOutpost[sq];
	  sp_pst[sd][B][REL_SQ(sq, sd)] = pstBishopOutpost[sq];
    }
  }
}

void cParam::Init(void) {

  int r_delta, f_delta;

  // Init king attack table

  for (int t = 0, i = 1; i < 511; ++i) {
    t = Min(1280.0, Min(int(0.027 * i * i), t + 8.0));
    danger[i] = (t * 100) / 256; // rescale to centipawns
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

  // Init tables for adjusting piece values 
  // according to the number of own pawns

  for (int i = 0; i < 9; i++) {
    np_table[i] = adj[i] * np_bonus;
	rp_table[i] = adj[i] * rp_malus;
  }

  // Init support mask (for detecting weak pawns)

  for (int sq = 0; sq < 64; sq++) {
    support_mask[WC][sq] = ShiftWest(SqBb(sq)) | ShiftEast(SqBb(sq));
    support_mask[WC][sq] |= FillSouth(support_mask[WC][sq]);

    support_mask[BC][sq] = ShiftWest(SqBb(sq)) | ShiftEast(SqBb(sq));
    support_mask[BC][sq] |= FillNorth(support_mask[BC][sq]);
  }

  // Init mask for passed pawn detection

  for (int sq = 0; sq < 64; sq++) {
    passed_mask[WC][sq] = FillNorthExcl(SqBb(sq));
    passed_mask[WC][sq] |= ShiftSideways(passed_mask[WC][sq]);
    passed_mask[BC][sq] = FillSouthExcl(SqBb(sq));
    passed_mask[BC][sq] |= ShiftSideways(passed_mask[BC][sq]);
  }
}