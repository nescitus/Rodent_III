#include <string.h>
#if defined(_WIN32) || defined(_WIN64)
#  include <windows.h>
#else
#  include <unistd.h>
#  include <sys/time.h>
#endif
#include "rodent.h"

int InputAvailable(void) {

#if defined(_WIN32) || defined(_WIN64)
  static int init = 0, pipe;
  static HANDLE inh;
  DWORD dw;

  if (!init) {
    init = 1;
    inh = GetStdHandle(STD_INPUT_HANDLE);
    pipe = !GetConsoleMode(inh, &dw);
    if (!pipe) {
      SetConsoleMode(inh, dw & ~(ENABLE_MOUSE_INPUT | ENABLE_WINDOW_INPUT));
      FlushConsoleInputBuffer(inh);
    }
  }
  if (pipe) {
    if (!PeekNamedPipe(inh, NULL, 0, NULL, &dw, NULL))
      return 1;
    return dw > 0;
  } else {
    GetNumberOfConsoleInputEvents(inh, &dw);
    return dw > 1;
  }
#else
  fd_set readfds;
  struct timeval tv;

  FD_ZERO(&readfds);
  FD_SET(STDIN_FILENO, &readfds);
  tv.tv_sec = 0;
  tv.tv_usec = 0;
  select(STDIN_FILENO + 1, &readfds, NULL, NULL, &tv);
  return FD_ISSET(STDIN_FILENO, &readfds);
#endif
}

int GetMS(void) {

#if defined(_WIN32) || defined(_WIN64)
  return GetTickCount();
#else
  struct timeval tv;

  gettimeofday(&tv, NULL);
  return tv.tv_sec * 1000 + tv.tv_usec / 1000;
#endif
}

U64 Random64(void) {

  static U64 next = 1;

  next = next * 1103515245 + 12345;
  return next;
}

U64 Key(POS *p) {

  int i;
  U64 key;

  key = 0;
  for (i = 0; i < 64; i++)
    if (p->pc[i] != NO_PC)
      key ^= zob_piece[p->pc[i]][i];
  key ^= zob_castle[p->c_flags];
  if (p->ep_sq != NO_SQ)
    key ^= zob_ep[File(p->ep_sq)];
  if (p->side == BC)
    key ^= SIDE_RANDOM;
  return key;
}

int PopCnt(U64 bb) {

  U64 k1 = (U64)0x5555555555555555;
  U64 k2 = (U64)0x3333333333333333;
  U64 k3 = (U64)0x0F0F0F0F0F0F0F0F;
  U64 k4 = (U64)0x0101010101010101;

  bb -= (bb >> 1) & k1;
  bb = (bb & k2) + ((bb >> 2) & k2);
  bb = (bb + (bb >> 4)) & k3;
  return (bb * k4) >> 56;
}

void MoveToStr(int move, char *move_str) {

  static const char prom_char[5] = "nbrq";

  move_str[0] = File(Fsq(move)) + 'a';
  move_str[1] = Rank(Fsq(move)) + '1';
  move_str[2] = File(Tsq(move)) + 'a';
  move_str[3] = Rank(Tsq(move)) + '1';
  move_str[4] = '\0';
  if (IsProm(move)) {
    move_str[4] = prom_char[(move >> 12) & 3];
    move_str[5] = '\0';
  }
}

int StrToMove(POS *p, char *move_str) {

  int from = Sq(move_str[0] - 'a', move_str[1] - '1');
  int to = Sq(move_str[2] - 'a', move_str[3] - '1');
  int type = NORMAL;

  if (TpOnSq(p, from) == K && Abs(to - from) == 2)
    type = CASTLE;

  else if (TpOnSq(p, from) == P) {
    if (to == p->ep_sq) 
      type = EP_CAP;
    else if (Abs(to - from) == 16)
      type = EP_SET;
    else if (move_str[4] != '\0')
      switch (move_str[4]) {
      case 'n':
        type = N_PROM;
        break;
      case 'b':
        type = B_PROM;
        break;
      case 'r':
        type = R_PROM;
        break;
      case 'q':
        type = Q_PROM;
        break;
      }
  }
  return (type << 12) | (to << 6) | from;
}

void PvToStr(int *pv, char *pv_str) {

  int *movep;
  char move_str[6];

  pv_str[0] = '\0';
  for (movep = pv; *movep; movep++) {
    MoveToStr(*movep, move_str);
    strcat(pv_str, move_str);
    strcat(pv_str, " ");
  }
}

void BuildPv(int *dst, int *src, int move) {

  *dst++ = move;
  while ((*dst++ = *src++))
    ;
}

int PopFirstBit(U64 * bb) {

  U64 bbLocal = *bb;
  *bb &= (*bb - 1);
  return FirstOne(bbLocal);
}