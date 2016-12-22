#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <thread>
#include <iostream>
#include "skeleton.h"
using namespace std;

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
      printf("id name Skeleton 0.124\n");
      printf("id author Pawel Koziol\n");
      printf("option name Hash type spin default 16 min 1 max 4096\n");
	  printf("option name Threads type spin default %d min 1 max 4\n", Glob.thread_no);
      printf("option name Clear Hash type button\n");
	  printf("option name OwnAttack type spin default %d min 0 max 500\n", Par.own_att);
	  printf("option name OppAttack type spin default %d min 0 max 500\n", Par.opp_att);
	  printf("option name OwnMobility type spin default %d min 0 max 500\n", Par.own_mob);
	  printf("option name OppMobility type spin default %d min 0 max 500\n", Par.opp_mob);
	  printf("option name KingTropism type spin default %d min 0 max 500\n", Par.tropism);
	  printf("option name Forwardness type spin default %d min 0 max 500\n", Par.forwardness);
	  printf("option name PassedPawns type spin default %d min 0 max 500\n", Par.passers);
	  printf("option name Lines type spin default %d min 0 max 500\n", Par.lines);

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
  } else if (strcmp(name, "Threads") == 0) {
      Glob.thread_no = (atoi(value));
  } else if (strcmp(name, "Clear Hash") == 0) {
    ClearTrans();
  } else if (strcmp(name, "OwnAttack") == 0) {
	 Par.own_att = atoi(value);
  } else if (strcmp(name, "OppAttack") == 0) {
     Par.opp_att = atoi(value);
  } else if (strcmp(name, "OwnMobility") == 0) {
     Par.own_mob = atoi(value);
  } else if (strcmp(name, "OppMobility") == 0) {
     Par.opp_mob = atoi(value);
  } else if (strcmp(name, "KingTropism") == 0) {
     Par.tropism = atoi(value);
  } else if (strcmp(name, "Forwardness") == 0) {
     Par.forwardness = atoi(value);
  } else if (strcmp(name, "PassedPawns") == 0) {
	  Par.passers = atoi(value);
  } else if (strcmp(name, "Lines") == 0) {
	  Par.lines = atoi(value);
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

void task1(POS * p, int *pv) {
  Engine1.Think(p, pv);
}

void task2(POS * p, int *pv) {
  Engine2.Think(p, pv);
}

void task3(POS * p, int *pv) {
  Engine3.Think(p, pv);
}

void task4(POS * p, int *pv) {
  Engine4.Think(p, pv);
}

int BulletCorrection(int time) {

  if (time < 200)       return (time * 23) / 32;
  else if (time <  400) return (time * 26) / 32;
  else if (time < 1200) return (time * 29) / 32;
  else return time;
}

void ParseGo(POS *p, char *ptr) {

  char token[80], bestmove_str[6], ponder_str[6];
  int wtime, btime, winc, binc, movestogo, time, inc;
  int pv[MAX_PLY], pv2[MAX_PLY], pv3[MAX_PLY], pv4[MAX_PLY];

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
	move_time = BulletCorrection(move_time);
  }

  // set global variables

  start_time = GetMS();
  tt_date = (tt_date + 1) & 255;
  nodes = 0;
  abort_search = 0;
  Glob.ClearData();
  Par.InitAsymmetric(p);

  // set engine-dependent variables

  Engine1.dp_completed = 0;
  Engine2.dp_completed = 0;
  Engine3.dp_completed = 0;
  Engine3.dp_completed = 0;
  int best_eng = 1;
  int best_depth = 0;

  if (Glob.thread_no == 1) {
	  thread t1(task1, p, pv);
	  t1.join();
  }

  if (Glob.thread_no == 2) {
	  thread t1(task1, p, pv);
	  thread t2(task2, p, pv2);
	  t1.join();
	  t2.join();
  }

  if (Glob.thread_no == 3) {
	  thread t1(task1, p, pv);
	  thread t2(task2, p, pv2);
	  thread t3(task2, p, pv3);
	  t1.join();
	  t2.join();
	  t3.join();
  }

  if (Glob.thread_no == 4) {
	  thread t1(task1, p, pv);
	  thread t2(task2, p, pv2);
	  thread t3(task2, p, pv3);
	  thread t4(task2, p, pv4);
	  t1.join();
	  t2.join();
	  t3.join();
	  t4.join();
  }

  best_depth = Engine1.dp_completed;
  if (Engine2.dp_completed > best_depth) { best_depth = Engine2.dp_completed; best_eng = 2; }
  if (Engine3.dp_completed > best_depth) { best_depth = Engine2.dp_completed; best_eng = 3; }
  if (Engine4.dp_completed > best_depth) { best_depth = Engine2.dp_completed; best_eng = 4; }

  if (best_eng == 4) {
	  MoveToStr(pv4[0], bestmove_str);
	  if (pv4[1]) {
		  MoveToStr(pv4[1], ponder_str);
		  printf("bestmove %s ponder %s\n", bestmove_str, ponder_str);
	  }
	  else
		  printf("bestmove %s\n", bestmove_str);
  }

  if (best_eng == 3) {
	  MoveToStr(pv3[0], bestmove_str);
	  if (pv3[1]) {
		  MoveToStr(pv3[1], ponder_str);
		  printf("bestmove %s ponder %s\n", bestmove_str, ponder_str);
	  }
	  else
		  printf("bestmove %s\n", bestmove_str);
  }

  if (best_eng == 2) {
	  MoveToStr(pv2[0], bestmove_str);
	  if (pv2[1]) {
		  MoveToStr(pv2[1], ponder_str);
		  printf("bestmove %s ponder %s\n", bestmove_str, ponder_str);
	  }
	  else
		  printf("bestmove %s\n", bestmove_str);
  }
  
  if (best_eng == 1) {
	  MoveToStr(pv[0], bestmove_str);
	  if (pv[1]) {
		  MoveToStr(pv[1], ponder_str);
		  printf("bestmove %s ponder %s\n", bestmove_str, ponder_str);
	  }
	  else
		  printf("bestmove %s\n", bestmove_str);
  }
}
