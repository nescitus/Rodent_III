#include "skeleton.h"

void Init(void)
{
  int i, j, k, l, x, y;
  static const int dirs[4][2] = {{1, -1}, {16, -16}, {17, -17}, {15, -15}};
  static const int k_moves[8] = {-17, -16, -15, -1, 1, 15, 16, 17};

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
