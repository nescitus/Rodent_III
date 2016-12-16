#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rodent.h"

void ReadLine(char *str, int n)
{
  char *ptr;

  if (fgets(str, n, stdin) == NULL)
    exit(0);
  if ((ptr = strchr(str, '\n')) != NULL)
    *ptr = '\0';
}

char *ParseToken(char *string, char *token)
{
  while (*string == ' ')
    string++;
  while (*string != ' ' && *string != '\0')
    *token++ = *string++;
  *token = '\0';
  return string;
}

void UciLoop(void) {

  char command[4096], token[80], *ptr;
  POS p[1];

  setbuf(stdin, NULL);
  setbuf(stdout, NULL);
  SetPosition(p, START_POS);
  AllocTrans(16);
  for (;;) {
    ReadLine(command, sizeof(command));
    ptr = ParseToken(command, token);
    if (strcmp(token, "uci") == 0) {
      printf("id name Rodent III 0.009\n");
      printf("id author Pablo Vazquez, Pawel Koziol\n");
      printf("option name Hash type spin default 16 min 1 max 4096\n");
      printf("option name Clear Hash type button\n");
      printf("uciok\n");
    } else if (strcmp(token, "isready") == 0) {
      printf("readyok\n");
    } else if (strcmp(token, "setoption") == 0) {
      ParseSetoption(ptr);
    } else if (strcmp(token, "position") == 0) {
      ParsePosition(p, ptr);
    } else if (strcmp(token, "go") == 0) {
      ParseGo(p, ptr);
    } else if (strcmp(token, "quit") == 0) {
      exit(0);
    }
  }
}

void ParseSetoption(char *ptr) {

  char token[80], name[80], value[80];

  ptr = ParseToken(ptr, token);
  name[0] = '\0';
  for (;;) {
    ptr = ParseToken(ptr, token);
    if (*token == '\0' || strcmp(token, "value") == 0)
      break;
    strcat(name, token);
    strcat(name, " ");
  }
  name[strlen(name) - 1] = '\0';
  if (strcmp(token, "value") == 0) {
    value[0] = '\0';
    for (;;) {
      ptr = ParseToken(ptr, token);
      if (*token == '\0')
        break;
      strcat(value, token);
      strcat(value, " ");
    }
    value[strlen(value) - 1] = '\0';
  }
  if (strcmp(name, "Hash") == 0) {
    AllocTrans(atoi(value));
  } else if (strcmp(name, "Clear Hash") == 0) {
    ClearTrans();
  }
}

void ParsePosition(POS *p, char *ptr) {

  char token[80], fen[80];
  UNDO u[1];

  ptr = ParseToken(ptr, token);
  if (strcmp(token, "fen") == 0) {
    fen[0] = '\0';
    for (;;) {
      ptr = ParseToken(ptr, token);
      if (*token == '\0' || strcmp(token, "moves") == 0)
        break;
      strcat(fen, token);
      strcat(fen, " ");
    }
    SetPosition(p, fen);
  } else {
    ptr = ParseToken(ptr, token);
    SetPosition(p, START_POS);
  }
  if (strcmp(token, "moves") == 0)
    for (;;) {
      ptr = ParseToken(ptr, token);
      if (*token == '\0')
        break;
      DoMove(p, StrToMove(p, token), u);
      if (p->rev_moves == 0)
        p->head = 0;
    }
}

void ParseGo(POS *p, char *ptr) {

  char token[80], bestmove_str[6], ponder_str[6];
  int wtime, btime, winc, binc, movestogo, time, inc, pv[MAX_PLY];

  move_time = -1;
  pondering = 0;
  wtime = -1;
  btime = -1;
  winc = 0;
  binc = 0;
  movestogo = 40;
  search_depth = 64;
  for (;;) {
    ptr = ParseToken(ptr, token);
    if (*token == '\0')
      break;
    if (strcmp(token, "ponder") == 0) {
      pondering = 1;
   } else if (strcmp(token, "depth") == 0) {
      ptr = ParseToken(ptr, token);
      search_depth = atoi(token);
    } else if (strcmp(token, "wtime") == 0) {
      ptr = ParseToken(ptr, token);
      wtime = atoi(token);
    } else if (strcmp(token, "btime") == 0) {
      ptr = ParseToken(ptr, token);
      btime = atoi(token);
    } else if (strcmp(token, "winc") == 0) {
      ptr = ParseToken(ptr, token);
      winc = atoi(token);
    } else if (strcmp(token, "binc") == 0) {
      ptr = ParseToken(ptr, token);
      binc = atoi(token);
    } else if (strcmp(token, "movestogo") == 0) {
      ptr = ParseToken(ptr, token);
      movestogo = atoi(token);
    }
  }
  time = p->side == WC ? wtime : btime;
  inc = p->side == WC ? winc : binc;
  if (time >= 0) {
    if (movestogo == 1) time -= Min(1000, time / 10);
    move_time = (time + inc * (movestogo - 1)) / movestogo;
    if (move_time > time)
      move_time = time;
    move_time -= 10;
    if (move_time < 0)
      move_time = 0;
  }
  Think(p, pv);
  MoveToStr(pv[0], bestmove_str);
  if (pv[1]) {
    MoveToStr(pv[1], ponder_str);
    printf("bestmove %s ponder %s\n", bestmove_str, ponder_str);
  } else
    printf("bestmove %s\n", bestmove_str);
}
