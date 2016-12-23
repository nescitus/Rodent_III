#include "rodent.h"

void Init(void) {

  int i, j, k, l, x, y;
  static const int dirs[4][2] = {{1, -1}, {16, -16}, {17, -17}, {15, -15}};
  static const int n_moves[8] = {-33, -31, -18, -14, 14, 18, 31, 33};
  static const int k_moves[8] = {-17, -16, -15, -1, 1, 15, 16, 17};

  for (i = 0; i < 64; i++) {
    line_mask[0][i] = RANK_1_BB << (i & 070);
    line_mask[1][i] = FILE_A_BB << (i & 007);
    j = File(i) - Rank(i);
    if (j > 0)
      line_mask[2][i] = DIAG_A1H8_BB >> (j * 8);
    else
      line_mask[2][i] = DIAG_A1H8_BB << (-j * 8);
    j = File(i) - (RANK_8 - Rank(i));
    if (j > 0)
      line_mask[3][i] = DIAG_A8H1_BB << (j * 8);
    else
      line_mask[3][i] = DIAG_A8H1_BB >> (-j * 8);
  }
  for (i = 0; i < 4; i++)
    for (j = 0; j < 64; j++)
      for (k = 0; k < 64; k++) {
        attacks[i][j][k] = 0;
        for (l = 0; l < 2; l++) {
          x = Map0x88(j) + dirs[i][l];
          while (!Sq0x88Off(x)) {
            y = Unmap0x88(x);
            attacks[i][j][k] |= SqBb(y);
            if ((k << 1) & (1 << (i != 1 ? File(y) : Rank(y))))
              break;
            x += dirs[i][l];
          }
        }
      }

  for (int sq = 0; sq < 64; sq++) {
    p_attacks[WC][sq] = ShiftNE(SqBb(sq)) | ShiftNW(SqBb(sq));
    p_attacks[BC][sq] = ShiftSE(SqBb(sq)) | ShiftSW(SqBb(sq));
  }

  for (i = 0; i < 64; i++) {
    n_attacks[i] = 0;
    for (j = 0; j < 8; j++) {
      x = Map0x88(i) + n_moves[j];
      if (!Sq0x88Off(x))
        n_attacks[i] |= SqBb(Unmap0x88(x));
    }
  }

  for (i = 0; i < 64; i++) {
    k_attacks[i] = 0;
    for (j = 0; j < 8; j++) {
      x = Map0x88(i) + k_moves[j];
      if (!Sq0x88Off(x))
        k_attacks[i] |= SqBb(Unmap0x88(x));
    }
  }

  for (i = 0; i < 8; i++) {
    adjacent_mask[i] = 0;
    if (i > 0)
      adjacent_mask[i] |= FILE_A_BB << (i - 1);
    if (i < 7)
      adjacent_mask[i] |= FILE_A_BB << (i + 1);
  }

  for (i = 0; i < 64; i++)
    c_mask[i] = 15;

  c_mask[A1] = 13;
  c_mask[E1] = 12;
  c_mask[H1] = 14;
  c_mask[A8] = 7;
  c_mask[E8] = 3;
  c_mask[H8] = 11;

  for (i = 0; i < 12; i++)
    for (j = 0; j < 64; j++)
      zob_piece[i][j] = Random64();
  for (i = 0; i < 16; i++)
    zob_castle[i] = Random64();
  for (i = 0; i < 8; i++)
    zob_ep[i] = Random64();
}
