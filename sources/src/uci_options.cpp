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
#include "rodent.h"
#include "book.h"

void PrintUciOptions(void) {

  printf("option name Hash type spin default 16 min 1 max 4096\n");
  printf("option name Threads type spin default %d min 1 max 4\n", Glob.thread_no);
  printf("option name Clear Hash type button\n");

  if (Glob.use_personality_files)
    printf("option name PersonalityFile type string default rodent.txt\n");
  else {

    printf("option name PawnValue type spin default %d min 0 max 1200\n", Par.pc_value[P]);
    printf("option name KnightValue type spin default %d min 0 max 1200\n", Par.pc_value[N]);
    printf("option name BishopValue type spin default %d min 0 max 1200\n", Par.pc_value[B]);
    printf("option name RookValue type spin default %d min 0 max 1200\n", Par.pc_value[R]);
    printf("option name QueenValue type spin default %d min 0 max 1200\n", Par.pc_value[Q]);

    printf("option name KeepPawn type spin default %d min 0 max 500\n", Par.keep_pc[P]);
    printf("option name KeepKnight type spin default %d min 0 max 500\n", Par.keep_pc[N]);
    printf("option name KeepBishop type spin default %d min 0 max 500\n", Par.keep_pc[B]);
    printf("option name KeepRook type spin default %d min 0 max 500\n", Par.keep_pc[R]);
    printf("option name KeepQueen type spin default %d min 0 max 500\n", Par.keep_pc[Q]);

    printf("option name BishopPair type spin default %d min -100 max 100\n", Par.bish_pair);
    printf("option name ExchangeImbalance type spin default %d min -100 max 100\n", Par.exchange_imbalance);
    printf("option name KnightLikesClosed type spin default %d min 0 max 10\n", Par.n_likes_closed);
    printf("option name RookLikesOpen type spin default %d min 0 max 10\n", Par.r_likes_open);

    printf("option name Material type spin default %d min 0 max 500\n", Par.mat_weight);
    printf("option name PiecePlacement type spin default %d min 0 max 500\n", Par.pst_weight);
    printf("option name OwnAttack type spin default %d min 0 max 500\n", Par.own_att_weight);
    printf("option name OppAttack type spin default %d min 0 max 500\n", Par.opp_att_weight);
    printf("option name OwnMobility type spin default %d min 0 max 500\n", Par.own_mob_weight);
    printf("option name OppMobility type spin default %d min 0 max 500\n", Par.opp_mob_weight);
    printf("option name KingTropism type spin default %d min 0 max 500\n", Par.tropism_weight);
    printf("option name Forwardness type spin default %d min 0 max 500\n", Par.forward_weight);
    printf("option name PiecePressure type spin default %d min 0 max 500\n", Par.threats_weight);

    printf("option name PassedPawns type spin default %d min 0 max 500\n", Par.passers_weight);
    printf("option name PaswnStructure type spin default %d min 0 max 500\n", Par.struct_weight);
    printf("option name PawnShield type spin default %d min 0 max 500\n", Par.shield_weight);
    printf("option name PawnStorm type spin default %d min 0 max 500\n", Par.storm_weight);
    printf("option name Outposts type spin default %d min 0 max 500\n", Par.outposts_weight);
    printf("option name Lines type spin default %d min 0 max 500\n", Par.lines_weight);
	printf("option name Fianchetto type spin default %d min 0 max 100\n", Par.protecting_bishop);

    printf("option name PstStyle type spin default %d min 0 max 3\n", Par.pst_style);
    printf("option name MobilityStyle type spin default %d min 0 max 1\n", Par.mob_style);
    printf("option name Contempt type spin default %d min -250 max 250\n", Par.draw_score);

    if (!Glob.elo_slider) {
      printf("option name EvalBlur type spin default %d min 0 max 5000000\n", Par.eval_blur);
      printf("option name NpsLimit type spin default %d min 0 max 5000000\n", Par.nps_limit);
    } else {
      printf("option name UCI_LimitStrength type check default false\n");
      printf("option name UCI_Elo type spin default %d min 800 max 2800\n", Par.elo);
    }

    printf("option name SlowMover type spin default %d min 10 max 500\n", Par.time_percentage);
    printf("option name Selectivity type spin default %d min 10 max 500\n", Par.hist_perc);
    printf("option name SearchSkill type spin default %d min 0 max 10\n", Par.search_skill);
	printf("option name RiskyDepth type spin default %d min 0 max 10\n", Par.riskydepth);
  }
  printf("option name UseBook type check default false\n");
  printf("option name GuideBookFile type string default guide.bin\n");
  printf("option name MainBookFile type string default rodent.bin\n");

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
  if (strcmp(name, "Hash") == 0                     || strcmp(name, "hash") == 0)              {
    AllocTrans(atoi(value));
  } else if (strcmp(name, "Threads") == 0           || strcmp(name, "threads") == 0)           {
    Glob.thread_no = (atoi(value));
  } else if (strcmp(name, "Clear Hash") == 0        || strcmp(name, "clear hash") == 0)        {
    ClearTrans();
  } else if (strcmp(name, "PawnValue") == 0         || strcmp(name, "pawnvalue") == 0)         {
    Par.pc_value[P] = atoi(value);
    Par.InitPst();
    Glob.should_clear = 1;
  } else if (strcmp(name, "KnightValue") == 0       || strcmp(name, "knightvalue") == 0)       {
    Par.pc_value[N] = atoi(value);
    Par.InitPst();
    Glob.should_clear = 1;
  } else if (strcmp(name, "BishopValue") == 0       || strcmp(name, "bishopvalue") == 0)       {
    Par.pc_value[B] = atoi(value);
    Par.InitPst();
    Glob.should_clear = 1;
  } else if (strcmp(name, "RookValue") == 0         || strcmp(name, "rookvalue") == 0)         {
    Par.pc_value[R] = atoi(value);
    Par.InitPst();
    Glob.should_clear = 1;
  } else if (strcmp(name, "QueenValue") == 0        || strcmp(name, "queenvalue") == 0)        {
    Par.pc_value[Q] = atoi(value);
    Par.InitPst();
    Glob.should_clear = 1;
  } else if (strcmp(name, "KeepPawn") == 0          || strcmp(name, "keeppawn") == 0)          {
    Par.keep_pc[P] = atoi(value);
    Glob.should_clear = 1;
  } else if (strcmp(name, "KeepKnight") == 0        || strcmp(name, "keepknight") == 0)        {
    Par.keep_pc[N] = atoi(value);
    Glob.should_clear = 1;
  } else if (strcmp(name, "KeepBishop") == 0        || strcmp(name, "keepbishop") == 0)        {
    Par.keep_pc[B] = atoi(value);
    Glob.should_clear = 1;
  } else if (strcmp(name, "KeepRook") == 0          || strcmp(name, "keeprook") == 0)          {
    Par.keep_pc[R] = atoi(value);
    Glob.should_clear = 1;
  } else if (strcmp(name, "KeepQueen") == 0         || strcmp(name, "keepqueen") == 0)         {
    Par.keep_pc[Q] = atoi(value);
    Glob.should_clear = 1;
  } else if (strcmp(name, "BishopPair") == 0        || strcmp(name, "bishoppair") == 0)        {
    Par.bish_pair = atoi(value);
    Glob.should_clear = 1;
  } else if (strcmp(name, "ExchangeImbalance") == 0 || strcmp(name, "exchangeimbalance") == 0) {
    Par.exchange_imbalance = atoi(value);
	Par.InitMaterialTweaks();
    Glob.should_clear = 1;
  } else if (strcmp(name, "KnightLikesClosed") == 0 || strcmp(name, "knightlikesclosed") == 0) {
    Par.n_likes_closed = atoi(value);
	Par.InitMaterialTweaks();
    Glob.should_clear = 1;
  } else if (strcmp(name, "RookLikesOpen") == 0     || strcmp(name, "rooklikesopen") == 0)  {
    Par.r_likes_open = atoi(value);
	Par.InitMaterialTweaks();
    Glob.should_clear = 1;
  } else if (strcmp(name, "Material") == 0          || strcmp(name, "material") == 0)       {
    Par.mat_weight = atoi(value);
    Par.InitPst();
    Glob.should_clear = 1;
  } else if (strcmp(name, "PiecePlacement") == 0    || strcmp(name, "pieceplacement") == 0) {
    Par.pst_weight = atoi(value);
    Par.InitPst();
    Glob.should_clear = 1;
  } else if (strcmp(name, "OwnAttack") == 0         || strcmp(name, "ownattack") == 0)      {
    Par.own_att_weight = atoi(value);
    Glob.should_clear = 1;
  } else if (strcmp(name, "OppAttack") == 0         || strcmp(name, "oppattack") == 0)      {
    Par.opp_att_weight = atoi(value);
    Glob.should_clear = 1;
  } else if (strcmp(name, "OwnMobility") == 0       || strcmp(name, "ownmobility") == 0)    {
    Par.own_mob_weight = atoi(value);
    Glob.should_clear = 1;
  } else if (strcmp(name, "OppMobility") == 0       || strcmp(name, "oppmobility") == 0)    {
    Par.opp_mob_weight = atoi(value);
    Glob.should_clear = 1;
  } else if (strcmp(name, "KingTropism") == 0       || strcmp(name, "kingtropism") == 0)    {
    Par.tropism_weight = atoi(value);
    Glob.should_clear = 1;
  } else if (strcmp(name, "Forwardness") == 0       || strcmp(name, "forwardness") == 0)    {
    Par.forward_weight = atoi(value);
    Glob.should_clear = 1;
  } else if (strcmp(name, "PiecePressure") == 0     || strcmp(name, "piecepressure") == 0)  {
    Par.threats_weight = atoi(value);
    Glob.should_clear = 1;
  } else if (strcmp(name, "PassedPawns") == 0       || strcmp(name, "passedpawns") == 0)    {
    Par.passers_weight = atoi(value);
    Glob.should_clear = 1;
  } else if (strcmp(name, "PawnStructure") == 0     || strcmp(name, "pawnstructure") == 0)  {
    Par.struct_weight = atoi(value);
	Glob.should_clear = 1;
  } else if (strcmp(name, "PawnShield") == 0        || strcmp(name, "pawnshield") == 0)     {
    Par.shield_weight = atoi(value);
	Glob.should_clear = 1;
  } else if (strcmp(name, "PawnStorm") == 0         || strcmp(name, "pawnstorm") == 0)      {
    Par.storm_weight = atoi(value);
	Glob.should_clear = 1;
  } else if (strcmp(name, "Outposts") == 0          || strcmp(name, "outposts") == 0)       {
    Par.outposts_weight = atoi(value);
	Glob.should_clear = 1;
  } else if (strcmp(name, "Lines") == 0             || strcmp(name, "lines") == 0)          {
    Par.lines_weight = atoi(value);
	Glob.should_clear = 1;
  } else if (strcmp(name, "Fianchetto") == 0        || strcmp(name, "fianchetto") == 0)          {
    Par.protecting_bishop = atoi(value);
	Glob.should_clear = 1;
  } else if (strcmp(name, "DoubledPawnMg") == 0     || strcmp(name, "doubledpawnmg") == 0)  {
    Par.doubled_mg = atoi(value);
	Glob.should_clear = 1;
  } else if (strcmp(name, "DoubledPawnEg") == 0     || strcmp(name, "doubledpawneg") == 0)  {
    Par.doubled_eg = atoi(value);
	Glob.should_clear = 1;
  } else if (strcmp(name, "IsolatedPawnMg") == 0    || strcmp(name, "isolatedpawnmg") == 0) {
    Par.isolated_mg = atoi(value);
	Glob.should_clear = 1;
  } else if (strcmp(name, "IsolatedPawnEg") == 0    || strcmp(name, "isolatedpawneg") == 0) {
    Par.isolated_eg = atoi(value);
	Glob.should_clear = 1;
  } else if (strcmp(name, "IsolatedOpenMg") == 0    || strcmp(name, "isolatedopenmg") == 0) {
    Par.isolated_open = atoi(value);
	Glob.should_clear = 1;
  } else if (strcmp(name, "BackwardPawnMg") == 0    || strcmp(name, "backwardpawneg") == 0) {
    Par.backward_mg = atoi(value);
	Par.InitBackward();
    Glob.should_clear = 1;
  } else if (strcmp(name, "BackwardPawnEg") == 0    || strcmp(name, "backwardpawneg") == 0) {
    Par.backward_eg = atoi(value);
    Glob.should_clear = 1;
  } else if (strcmp(name, "BackwardOpenMg") == 0    || strcmp(name, "backwardopenmg") == 0) {
    Par.backward_open = atoi(value);
    Glob.should_clear = 1;
  } else if (strcmp(name, "PstStyle") == 0          || strcmp(name, "pststyle") == 0)       {
    Par.pst_style = atoi(value);
    Par.InitPst();
    Glob.should_clear = 1;
  } else if (strcmp(name, "MobilityStyle") == 0     || strcmp(name, "mobilitystyle") == 0)  {
    Par.mob_style = atoi(value);
    Par.InitMobility();
    Glob.should_clear = 1;
  } else if (strcmp(name, "GuideBookFile") == 0     || strcmp(name, "guidebookfile") == 0)  {
    if (!Glob.separate_books || !Glob.reading_personality) {
      GuideBook.ClosePolyglot();
      GuideBook.bookName = value;
      GuideBook.OpenPolyglot();
	}
  } else if (strcmp(name, "MainBookFile") == 0      || strcmp(name, "mainbookfile") == 0)   {
	if (!Glob.separate_books || !Glob.reading_personality) {
      MainBook.ClosePolyglot();
      MainBook.bookName = value;
      MainBook.OpenPolyglot();
	}
  } else if (strcmp(name, "Contempt") == 0          || strcmp(name, "contempt") == 0 )      {
    Par.draw_score = atoi(value);
	Glob.should_clear = 1;
 } else if (strcmp(name, "EvalBlur") == 0           || strcmp(name, "evalblur") == 0)       {
    Par.eval_blur = atoi(value);
	Glob.should_clear = 1;
  } else if (strcmp(name, "NpsLimit") == 0          || strcmp(name, "npslimit") == 0)       {
    Par.nps_limit = atoi(value);
  } else if (strcmp(name, "UCI_Elo") == 0           || strcmp(name, "uci_elo") == 0) {
    Par.elo = atoi(value);
    Par.SetSpeed(Par.elo);
  } else if (strcmp(name, "SearchSkill") == 0       || strcmp(name, "searchskill") == 0)    {
    Par.search_skill = atoi(value);
	Glob.should_clear = 1;
  } else if (strcmp(name, "RiskyDepth") == 0       || strcmp(name, "riskydepth") == 0)    {
    Par.riskydepth = atoi(value);
	Glob.should_clear = 1;
  } else if (strcmp(name, "SlowMover") == 0         || strcmp(name, "slowmover") == 0)      {
    Par.time_percentage = atoi(value);
  } else if (strcmp(name, "Selectivity") == 0       || strcmp(name, "selectivity") == 0)    {
    Par.hist_perc = atoi(value);
	Par.hist_limit = -MAX_HIST + ((MAX_HIST * Par.hist_perc) / 100);
	Glob.should_clear = 1;
  } else if (strcmp(name, "PersonalityFile") == 0 || strcmp(name, "personalityfile") == 0) {
	 printf("info string reading ");
	 printf(value);
	 printf("\n");
	 ReadPersonality(value);
  }
}

void ReadPersonality(char *fileName) {

  FILE *personalityFile;
  char line[256];
  char token[180], *ptr;

  // exit if this personality file doesn't exist

  if ((personalityFile = fopen(fileName, "r")) == NULL)
    return;

  // set flag in case we want to disable some options while reading
  // personality from a file

  Glob.reading_personality = 1;

  // read options line by line

  while (fgets(line, 256, personalityFile)) {
    ptr = ParseToken(line, token);

	// do we pick opening book within a personality?

    if (strstr(line, "PERSONALITY_BOOKS")) Glob.separate_books = 0;
    if (strstr(line, "GENERAL_BOOKS")) Glob.separate_books = 1;

	// how we go about weakening the engine?

    if (strstr(line, "ELO_SLIDER")) Glob.elo_slider = 1;
    if (strstr(line, "NPS_BLUR")) Glob.elo_slider = 0;

	// which UCI options are exposed to the user?

	if (strstr(line, "HIDE_OPTIONS")) Glob.use_personality_files = 1;
	if (strstr(line, "SHOW_OPTIONS")) Glob.use_personality_files = 0;

    if (strcmp(token, "setoption") == 0)
      ParseSetoption(ptr);
  }

  fclose(personalityFile);
  Glob.reading_personality = 0;
}