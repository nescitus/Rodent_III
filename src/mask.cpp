#include "skeleton.h"

void cMask::Init(void) {
  
  int j,k;

  home[WC] = RANK_1_BB | RANK_2_BB | RANK_3_BB | RANK_4_BB;
  home[BC] = RANK_8_BB | RANK_7_BB | RANK_6_BB | RANK_5_BB;
  away[WC] = RANK_8_BB | RANK_7_BB | RANK_6_BB | RANK_5_BB;
  away[BC] = RANK_1_BB | RANK_2_BB | RANK_3_BB | RANK_4_BB;
  qs_castle[WC] = SqBb(A1) | SqBb(B1) | SqBb(C1) | SqBb(A2) | SqBb(B2) | SqBb(C2);
  qs_castle[BC] = SqBb(A8) | SqBb(B8) | SqBb(C8) | SqBb(A7) | SqBb(B7) | SqBb(C7);
  ks_castle[WC] = SqBb(F1) | SqBb(G1) | SqBb(H1) | SqBb(F2) | SqBb(G2) | SqBb(H2);
  ks_castle[BC] = SqBb(F8) | SqBb(G8) | SqBb(H8) | SqBb(F7) | SqBb(G7) | SqBb(H7);

  // adjacent files (for isolated pawn detection)

  for (int i = 0; i < 8; i++) {
    adjacent[i] = 0;
    if (i > 0) adjacent[i] |= FILE_A_BB << (i - 1);
    if (i < 7) adjacent[i] |= FILE_A_BB << (i + 1);
  }

  // supported mask for weak pawns detection

  for (int sq = 0; sq < 64; sq++) {
    supported[WC][sq] = BB.ShiftSideways(SqBb(sq));
    supported[WC][sq] |= BB.FillSouth(supported[WC][sq]);

    supported[BC][sq] = BB.ShiftSideways(SqBb(sq));
    supported[BC][sq] |= BB.FillNorth(supported[BC][sq]);
  }

  // passed pawn masks

 for (int i = 0; i < 64; i++) {
    passed[WC][i] = 0;
    for (j = File(i) - 1; j <= File(i) + 1; j++) {
      if ((File(i) == FILE_A && j == -1) ||
          (File(i) == FILE_H && j == 8))
        continue;
      for (k = Rank(i) + 1; k <= RANK_8; k++)
        passed[WC][i] |= SqBb(Sq(j, k));
    }
  }
  for (int i = 0; i < 64; i++) {
    passed[BC][i] = 0;
    for (j = File(i) - 1; j <= File(i) + 1; j++) {
      if ((File(i) == FILE_A && j == -1) ||
          (File(i) == FILE_H && j == 8))
        continue;
      for (k = Rank(i) - 1; k >= RANK_1; k--)
        passed[BC][i] |= SqBb(Sq(j, k));
    }
  }
}