#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <thread>
#include <iostream>
using namespace std;
#include "rodent.h"

void ReadLine(char *str, int n) {
  char *ptr;

  if (fgets(str, n, stdin) == NULL)
    exit(0);
  if ((ptr = strchr(str, '\n')) != NULL)
    *ptr = '\0';
}

char *ParseToken(char *string, char *token) {

  while (*string == ' ')
    string++;
  while (*string != ' ' && *string != '\0')
    *token++ = *string++;
  *token = '\0';
  return string;
}

int BulletCorrection(int time) {

  if (time < 200)       return (time * 23) / 32;
  else if (time <  400) return (time * 26) / 32;
  else if (time < 1200) return (time * 29) / 32;
  else return time;
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
      printf("id name Rodent III 0.030\n");
      printf("id author Pablo Vazquez, Pawel Koziol\n");
	  printf("option name Threads type spin default %d min 1 max 2\n", thread_no);
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
    } else if (strcmp(token, "step") == 0) {
      ParseMoves(p, ptr);
    } else if (strcmp(token, "print") == 0) {
      PrintBoard(p);
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
  } else if (strcmp(name, "Threads") == 0) {
    thread_no = (atoi(value));
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
    ParseMoves(p, ptr);
}

void ParseMoves(POS *p, char *ptr) {

	char token[80];
	UNDO u[1];

    for (;;) {
      ptr = ParseToken(ptr, token);
      if (*token == '\0')
        break;
      DoMove(p, StrToMove(p, token), u);
      if (p->rev_moves == 0)
        p->head = 0;
    }
}

void task2(POS * p, int *pv) {
  Engine2.Think(p, pv);
}

void ExtractMove(int pv[MAX_PLY]) {

  char bestmove_str[6], ponder_str[6];

  MoveToStr(pv[0], bestmove_str);
  if (pv[1]) {
    MoveToStr(pv[1], ponder_str);
    printf("bestmove %s ponder %s\n", bestmove_str, ponder_str);
  }
  else
    printf("bestmove %s\n", bestmove_str);
}

void ParseGo(POS *p, char *ptr) {

  char token[80];
  int wtime, btime, winc, binc, movestogo, time, inc, pv[MAX_PLY], pv2[MAX_PLY];

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
    if (move_time > time) move_time = time;

	// assign less time per move while using extremely short time controls

	move_time = BulletCorrection(move_time);

	// while in time trouble, try to save a bit on increment

	if (move_time < inc)  move_time -= ((inc * 4) / 5);

	// safeguard against a lag

    move_time -= 10;

	// ensure that we have non-negative time

    if (move_time < 0) move_time = 0;
  }

  // thread-independent stuff to be done before searching

  tt_date = (tt_date + 1) & 255;
  nodes = 0;
  abort_search = 0;
  start_time = GetMS();
  Engine1.depth_reached = 0;
  Engine2.depth_reached = 0;

  if (thread_no == 1) {
    Engine1.Think(p, pv);
	ExtractMove(pv);
	return;
  }

  if (thread_no == 2) {
    thread t2(task2, p, pv2);
	Engine1.Think(p, pv);
    t2.join();
  }

  if (thread_no == 2 && Engine2.depth_reached > Engine1.depth_reached) {
  // if second thread managed to search to the greater depth than the first thread
	  ExtractMove(pv2);
  } else { 
  // we are searching single-threaded or the first thread got at least the same depth as the second thread
	  ExtractMove(pv);
  }
}

void PrintBoard(POS *p) {

  char *piece_name[] = { "P ", "p ", "N ", "n ", "B ", "b ", "R ", "r ", "Q ", "q ", "K ", "k ", ". " };

  printf("--------------------------------------------\n");
  for (int sq = 0; sq < 64; sq++) { 
    printf(piece_name[p->pc[sq ^ (BC * 56)]]);
    if ((sq + 1) % 8 == 0) printf(" %d\n", 9 - ((sq + 1) / 8));
  }

  printf("\na b c d e f g h\n\n--------------------------------------------\n");
  printf("%d%d", p->cnt[WC][P], p->cnt[BC][P]);
}
