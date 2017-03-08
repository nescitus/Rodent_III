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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <thread>
#include <iostream>
#include "rodent.h"
#include "book.h"

#if defined(_WIN32) || defined(_WIN64)
#  define WINDOWS_BUILD
#else
#  include <unistd.h>
#endif

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

    // boolean option: strength limit

    if (strstr(command, "setoption name UCI_LimitStrength value"))
      Par.fl_weakening = (strstr(command, "value true") != 0);
    if (strstr(command, "setoption name uci_limitstrength value"))
      Par.fl_weakening = (strstr(command, "value true") != 0);

	// boolean option: opening book usage

    if (strstr(command, "setoption name UseBook value"))
      Par.use_book = (strstr(command, "value true") != 0);
    if (strstr(command, "setoption name usebook value"))
      Par.use_book = (strstr(command, "value true") != 0);

    if (strcmp(token, "uci") == 0) {
      printf("id name Rodent III 0.171\n");  
	  Glob.is_console = 0;
      printf("id author Pawel Koziol (based on Sungorus 1.4 by Pablo Vazquez)\n");
      PrintUciOptions();
      printf("uciok\n");
    } else if (strcmp(token, "isready") == 0) {
      printf("readyok\n");
    } else if (strcmp(token, "setoption") == 0) {
      ParseSetoption(ptr);
    } else if (strcmp(token, "position") == 0) {
      ParsePosition(p, ptr);
    } else if (strcmp(token, "go") == 0) {
      ParseGo(p, ptr);
    } else if (strcmp(token, "print") == 0) {
      PrintBoard(p);
    } else if (strcmp(token, "step") == 0) {
      ParseMoves(p, ptr);
    } else if (strcmp(token, "bench") == 0) {
      ptr = ParseToken(ptr, token);
      Engine1.Bench(atoi(token));
    } else if (strcmp(token, "quit") == 0) {
      exit(0);
    }
  }
}

void ParseMoves(POS *p, char *ptr) {
  
  char token[180];
  UNDO u[1];

  for (;;) {

    // Get next move to parse

    ptr = ParseToken(ptr, token);

    // No more moves!

    if (*token == '\0') break;

    p->DoMove(StrToMove(p, token), u);

    // We won't be taking back moves beyond this point:

    if (p->rev_moves == 0) p->head = 0;
  }
}

void ParsePosition(POS *p, char *ptr) {

  char token[80], fen[80];

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

void timer_task() {

  Glob.abort_search = 0;

  while (Glob.abort_search == 0) {
  #if defined(_WIN32) || defined(_WIN64)
    _sleep(5);
  #else
    usleep(5 * 1000);
  #endif
    CheckTimeout();
  }
}

int BulletCorrection(int time) {

  if (time < 200)       return (time * 23) / 32;
  else if (time <  400) return (time * 26) / 32;
  else if (time < 1200) return (time * 29) / 32;
  else return time;
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

void SetMoveTime(int base, int inc, int movestogo) {
  
  if (base >= 0) {
    if (movestogo == 1) base -= Min(1000, base / 10);
    move_time = (base + inc * (movestogo - 1)) / movestogo;

	// make a percentage correction to playing speed (unless too risky)

	if (2 * move_time > base) {
	  move_time *= Par.time_percentage;
	  move_time /= 100;
	}

    // ensure that our limit does not exceed total time available
    
	if (move_time > base) move_time = base;

    // safeguard against a lag
    
	move_time -= 10;

    // ensure that we have non-negative time

    if (move_time < 0) move_time = 0;

    // assign less time per move on extremely short time controls
    
	move_time = BulletCorrection(move_time);
  }
}

void ParseGo(POS *p, char *ptr) {

  char token[80], bestmove_str[6], ponder_str[6];
  int wtime, btime, winc, binc, movestogo, strict_time;
  int pv[MAX_PLY], pv2[MAX_PLY], pv3[MAX_PLY], pv4[MAX_PLY];
  int move_from_book = 0;

  move_time = -1;
  move_nodes = 0;
  Glob.pondering = 0;
  wtime = -1;
  btime = -1;
  winc = 0;
  binc = 0;
  movestogo = 40;
  strict_time = 0;
  search_depth = 64;
  Par.shut_up = 0;

  for (;;) {
    ptr = ParseToken(ptr, token);
    if (*token == '\0')
      break;
    if (strcmp(token, "ponder") == 0) {
      Glob.pondering = 1;
    } else if (strcmp(token, "depth") == 0) {
      ptr = ParseToken(ptr, token);
      search_depth = atoi(token);
	  strict_time = 1;
    } else if (strcmp(token, "movetime") == 0) {
      ptr = ParseToken(ptr, token);
      move_time = atoi(token);
	  strict_time = 1;
    } else if (strcmp(token, "nodes") == 0) {
      ptr = ParseToken(ptr, token);
      move_nodes = atoi(token);
	  move_time = 99999999;
	  strict_time = 1;
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

  // set move time

  if (!strict_time) {
    int base = p->side == WC ? wtime : btime;
    int inc = p->side == WC ? winc : binc;
    SetMoveTime(base, inc, movestogo);
  }

  // set global variables

  start_time = GetMS();
  tt_date = (tt_date + 1) & 255;
  Glob.nodes = 0;
  Glob.abort_search = 0;
  Glob.depth_reached = 0;
  if (Glob.should_clear)
    Glob.ClearData(); // options has been changed and old tt scores are no longer reliable
  Par.InitAsymmetric(p);

  int best_eng = 1;
  int best_depth = 0;

  // get book move

  if (Par.use_book) {

    pv[0] = GuideBook.GetPolyglotMove(p, 1);
    if (!pv[0]) pv[0] = MainBook.GetPolyglotMove(p, 1);

    if (pv[0]) {
      MoveToStr(pv[0], bestmove_str);
	  printf("bestmove %s\n", bestmove_str);
	  move_from_book = 1;
	  goto done;
    }
  }

  // Set engine-dependent variables

  Engine1.dp_completed = 0;
  Engine2.dp_completed = 0;
  Engine3.dp_completed = 0;
  Engine4.dp_completed = 0;

  // Search using the designated number of threads

  if (Glob.thread_no == 1) {
    thread t(timer_task);
    thread t1(task1, p, pv);
    t1.join();
    t.join();
  }

  if (Glob.thread_no == 2) {
    thread t(timer_task);
    thread t1(task1, p, pv);
    thread t2(task2, p, pv2);
    t1.join();
    t2.join();
    t.join();
  }

  if (Glob.thread_no == 3) {
    thread t(timer_task);
    thread t1(task1, p, pv);
    thread t2(task2, p, pv2);
    thread t3(task2, p, pv3);
    t1.join();
    t2.join();
    t3.join();
    t.join();
  }

  if (Glob.thread_no == 4) {
    thread t(timer_task);
    thread t1(task1, p, pv);
    thread t2(task2, p, pv2);
    thread t3(task2, p, pv3);
    thread t4(task2, p, pv4);
    t1.join();
    t2.join();
    t3.join();
    t4.join();
    t.join();
  }

  done:

  if (!move_from_book) {
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

}

void cEngine::Bench(int depth) {

  POS p[1];
  int pv[MAX_PLY];
  char *test[] = {
    "r1bqkbnr/pp1ppppp/2n5/2p5/4P3/5N2/PPPP1PPP/RNBQKB1R w KQkq -",       // 1.e4 c5 2.Nf3 Nc6
    "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -",   // multiple captures
    "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -",                              // rook endgame
    "4rrk1/pp1n3p/3q2pQ/2p1pb2/2PP4/2P3N1/P2B2PP/4RRK1 b - - 7 19",
    "rq3rk1/ppp2ppp/1bnpb3/3N2B1/3NP3/7P/PPPQ1PP1/2KR3R w - - 7 14",      // knight pseudo-sack
    "r1bq1r1k/1pp1n1pp/1p1p4/4p2Q/4Pp2/1BNP4/PPP2PPP/3R1RK1 w - - 2 14",  // pawn chain
    "r3r1k1/2p2ppp/p1p1bn2/8/1q2P3/2NPQN2/PPP3PP/R4RK1 b - - 2 15",
    "r1bbk1nr/pp3p1p/2n5/1N4p1/2Np1B2/8/PPP2PPP/2KR1B1R w kq - 0 13",
    "r1bq1rk1/ppp1nppp/4n3/3p3Q/3P4/1BP1B3/PP1N2PP/R4RK1 w - - 1 16",     // attack for pawn
    "4r1k1/r1q2ppp/ppp2n2/4P3/5Rb1/1N1BQ3/PPP3PP/R5K1 w - - 1 17",        // exchange sack
    "2rqkb1r/ppp2p2/2npb1p1/1N1Nn2p/2P1PP2/8/PP2B1PP/R1BQK2R b KQ - 0 11",
    "r1bq1r1k/b1p1npp1/p2p3p/1p6/3PP3/1B2NN2/PP3PPP/R2Q1RK1 w - - 1 16",  // white pawn center
    "3r1rk1/p5pp/bpp1pp2/8/q1PP1P2/b3P3/P2NQRPP/1R2B1K1 b - - 6 22",
    "r1q2rk1/2p1bppp/2Pp4/p6b/Q1PNp3/4B3/PP1R1PPP/2K4R w - - 2 18",
    "4k2r/1pb2ppp/1p2p3/1R1p4/3P4/2r1PN2/P4PPP/1R4K1 b - - 3 22",         // endgame
    "3q2k1/pb3p1p/4pbp1/2r5/PpN2N2/1P2P2P/5PP1/Q2R2K1 b - - 4 26",        // both queens en prise
    NULL
  }; // test positions taken from DiscoCheck by Lucas Braesch

  if (depth == 0) depth = 8; // so that you can call bench without parameters
  ClearTrans();
  ClearAll();
  Par.shut_up = 1;

  printf("Bench test started (depth %d): \n", depth);

  Glob.nodes = 0;
  start_time = GetMS();
  search_depth = depth;

  // search each position to desired depth

  for (int i = 0; test[i]; ++i) {
    printf("%s", test[i]);
    SetPosition(p, test[i]);
    printf("\n");
	Glob.depth_reached = 0;
    Iterate(p, pv);
  }

  // calculate and print statistics

  int end_time = GetMS() - start_time;
  int nps = (Glob.nodes * 1000) / (end_time + 1);

  printf("%llu nodes searched in %d, speed %u nps (Score: %.3f)\n", Glob.nodes, end_time, nps, (float)nps / 430914.0);
}

void PrintBoard(POS *p) {

  char *piece_name[] = { "P ", "p ", "N ", "n ", "B ", "b ", "R ", "r ", "Q ", "q ", "K ", "k ", ". " };

  printf("--------------------------------------------\n");
  for (int sq = 0; sq < 64; sq++) {
    printf("%s", piece_name[p->pc[sq ^ (BC * 56)]]);
    if ((sq + 1) % 8 == 0) printf(" %d\n", 9 - ((sq + 1) / 8));
  }

  printf("\na b c d e f g h\n\n--------------------------------------------\n");
}