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

#include "rodent.h"
#include "book.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>

sPersAliases pers_aliases;

void PrintUciOptions() {

    printf("option name Hash type spin default 16 min 1 max 4096\n");
#ifdef USE_THREADS
    printf("option name Threads type spin default %d min 1 max %d\n", Glob.thread_no, MAX_THREADS);
#endif
    printf("option name Clear Hash type button\n");

    if (Glob.use_personality_files) {
        // it's unclear if the default 'rodent.txt' will be loaded by request from the GUI
        // something should be done with it: 1. read it here or
        // 2. (better imo) change to 'default ---' to avoid confusion
        if (pers_aliases.count == 0 || Glob.show_pers_file)
            printf("option name PersonalityFile type string default rodent.txt\n");
        if (pers_aliases.count != 0) {
            printf("option name Personality type combo default ---"); // `---` in case we want PersonalityFile
            for (int i = 0; i < pers_aliases.count; i++)
                printf(" var %s", pers_aliases.alias[i]);
            printf("\n");
        }
    } else {

        printf("option name PawnValue type spin default %d min 0 max 1200\n", Par.values[P_MID]);
        printf("option name KnightValue type spin default %d min 0 max 1200\n", Par.values[N_MID]);
        printf("option name BishopValue type spin default %d min 0 max 1200\n", Par.values[B_MID]);
        printf("option name RookValue type spin default %d min 0 max 1200\n", Par.values[R_MID]);
        printf("option name QueenValue type spin default %d min 0 max 1200\n", Par.values[Q_MID]);

        printf("option name KeepPawn type spin default %d min 0 max 500\n", Par.keep_pc[P]);
        printf("option name KeepKnight type spin default %d min 0 max 500\n", Par.keep_pc[N]);
        printf("option name KeepBishop type spin default %d min 0 max 500\n", Par.keep_pc[B]);
        printf("option name KeepRook type spin default %d min 0 max 500\n", Par.keep_pc[R]);
        printf("option name KeepQueen type spin default %d min 0 max 500\n", Par.keep_pc[Q]);

        printf("option name BishopPair type spin default %d min -100 max 100\n", Par.values[B_PAIR]);
        printf("option name ExchangeImbalance type spin default %d min -200 max 200\n", Par.values[A_EXC]);
        printf("option name KnightLikesClosed type spin default %d min 0 max 10\n", Par.values[N_CL]);

        printf("option name Material type spin default %d min 0 max 500\n", Par.mat_weight);
        printf("option name PstStyle type spin default %d min 0 max 3\n", Par.pst_style);
        printf("option name PiecePlacement type spin default %d min 0 max 500\n", Par.pst_weight);
        printf("option name OwnAttack type spin default %d min 0 max 500\n", Par.own_att_weight);
        printf("option name OppAttack type spin default %d min 0 max 500\n", Par.opp_att_weight);
        printf("option name OwnMobility type spin default %d min 0 max 500\n", Par.own_mob_weight);
        printf("option name OppMobility type spin default %d min 0 max 500\n", Par.opp_mob_weight);
        printf("option name KingTropism type spin default %d min 0 max 500\n", Par.tropism_weight);
        printf("option name Forwardness type spin default %d min -500 max 500\n", Par.forward_weight);
        printf("option name PiecePressure type spin default %d min 0 max 500\n", Par.threats_weight);

        printf("option name PassedPawns type spin default %d min 0 max 500\n", Par.passers_weight);
        printf("option name PawnStructure type spin default %d min 0 max 500\n", Par.struct_weight);
        printf("option name PawnShield type spin default %d min 0 max 500\n", Par.shield_weight);
        printf("option name PawnStorm type spin default %d min 0 max 500\n", Par.storm_weight);
        printf("option name Outposts type spin default %d min 0 max 500\n", Par.outposts_weight);
        printf("option name Lines type spin default %d min 0 max 500\n", Par.lines_weight);
        printf("option name Fianchetto type spin default %d min 0 max 100\n", Par.values[B_KING]);

        printf("option name Contempt type spin default %d min -500 max 500\n", Par.draw_score);

        if (!Glob.elo_slider) {
            printf("option name EvalBlur type spin default %d min 0 max 5000000\n", Par.eval_blur);
            printf("option name NpsLimit type spin default %d min 0 max 5000000\n", Par.nps_limit);
        } else {
            printf("option name UCI_LimitStrength type check default %s\n", Par.fl_weakening ? "true" : "false");
            printf("option name UCI_Elo type spin default %d min 800 max 2800\n", Par.elo);
        }

        printf("option name SlowMover type spin default %d min 10 max 500\n", Par.time_percentage);
        printf("option name Selectivity type spin default %d min 10 max 500\n", Par.hist_perc);
        printf("option name SearchSkill type spin default %d min 0 max 10\n", Par.search_skill);
#ifdef USE_RISKY_PARAMETER
        printf("option name RiskyDepth type spin default %d min 0 max 10\n", Par.riskydepth);
#endif
    }
    printf("option name UseBook type check default %s\n", Par.use_book ? "true" : "false");
    printf("option name MainBookFile type string default %s\n", MainBook.bookName);
    printf("option name GuideBookFile type string default %s\n", GuideBook.bookName);

}

static void valuebool(bool& param, char *val) {

    for (int i = 0; val[i]; i++)
        val[i] = tolower(val[i]);

    if (strcmp(val, "true")  == 0) param = true;
    if (strcmp(val, "false") == 0) param = false;
}

static char *pseudotrimstring(char *in_str) {

    for (int last = (int)strlen(in_str)-1; last >= 0 && in_str[last] == ' '; last--)
        in_str[last] = '\0';

    while (*in_str == ' ') in_str++;

    return in_str;
}

void ParseSetoption(const char *ptr) {

    char *name, *value;

    char *npos = (char *)strstr(ptr, " name ");  // len(" name ") == 6, len(" value ") == 7
    char *vpos = (char *)strstr(ptr, " value "); // sorry for an ugly "unconst"

    if ( !npos ) return; // if no 'name'

    if ( vpos ) {
        *vpos = '\0';
        value = pseudotrimstring(vpos + 7);
    }
    else value = npos; // fake, just to prevent possible crash if misusing

    name = pseudotrimstring(npos + 6);

    for (int i = 0; name[i]; i++)   // make `name` lowercase
        name[i] = tolower(name[i]); // we can't lowercase `value` 'coz paths are case-sensitive on linux

#ifndef NDEBUG
    printf( "(debug) setoption name: '%s' value: '%s'\n", name, value );
#endif

    if (strcmp(name, "hash") == 0)                                           {
        AllocTrans(atoi(value));
#ifdef USE_THREADS
    } else if (strcmp(name, "threads") == 0)                                 {
        Glob.thread_no = (atoi(value));
        if (Glob.thread_no > MAX_THREADS) Glob.thread_no = MAX_THREADS;

        if (Glob.thread_no != (int)Engines.size()) {
            Engines.clear();
            for (int i = 0; i < Glob.thread_no; i++)
                Engines.emplace_back(i);
        }
#endif
    } else if (strcmp(name, "clear hash") == 0)                              {
        ClearTrans();
    } else if (strcmp(name, "pawnvaluemg") == 0)                             {
        Par.values[P_MID] = atoi(value);
        Par.InitPst();
        Glob.should_clear = true;
    } else if (strcmp(name, "pawnvalueeg") == 0)                             {
        Par.values[P_END] = atoi(value);
        Par.InitPst();
        Glob.should_clear = true;
    } else if (strcmp(name, "pawnvalue") == 0)                               {
        SetPieceValue(P, atoi(value), P_MID);
    } else if (strcmp(name, "knightvaluemg") == 0)                           {
        Par.values[N_MID] = atoi(value);
        Par.InitPst();
        Glob.should_clear = true;
    } else if (strcmp(name, "knightvalueeg") == 0)                           {
        Par.values[N_END] = atoi(value);
        Par.InitPst();
        Glob.should_clear = true;
    } else if (strcmp(name, "knightvalue") == 0)                             {
        SetPieceValue(N, atoi(value), N_MID);
    } else if (strcmp(name, "bishopvaluemg") == 0)                           {
        Par.values[B_MID] = atoi(value);
        Par.InitPst();
        Glob.should_clear = true;
    } else if (strcmp(name, "bishopvalueeg") == 0)                           {
        Par.values[B_END] = atoi(value);
        Par.InitPst();
        Glob.should_clear = true;
    } else if (strcmp(name, "bishopvalue") == 0)                             {
        SetPieceValue(B, atoi(value), B_MID);
    } else if (strcmp(name, "rookvaluemg") == 0)                             {
        Par.values[R_MID] = atoi(value);
        Par.InitPst();
        Glob.should_clear = true;
    } else if (strcmp(name, "rookvalueeg") == 0)                             {
        Par.values[R_END] = atoi(value);
        Par.InitPst();
        Glob.should_clear = true;
    } else if (strcmp(name, "rookvalue") == 0)                               {
        SetPieceValue(R, atoi(value), R_MID);
    } else if (strcmp(name, "queenvaluemg") == 0)                            {
        Par.values[Q_MID] = atoi(value);
        Par.InitPst();
        Glob.should_clear = true;
    } else if (strcmp(name, "queenvalueeg") == 0)                            {
        Par.values[Q_END] = atoi(value);
        Par.InitPst();
        Glob.should_clear = true;
    } else if (strcmp(name, "queenvalue") == 0)                              {
        SetPieceValue(Q, atoi(value), Q_MID);
    } else if (strcmp(name, "keeppawn") == 0)                                {
        Par.keep_pc[P] = atoi(value);
        Glob.should_clear = true;
    } else if (strcmp(name, "keepknight") == 0)                              {
        Par.keep_pc[N] = atoi(value);
        Glob.should_clear = true;
    } else if (strcmp(name, "keepbishop") == 0)                              {
        Par.keep_pc[B] = atoi(value);
        Glob.should_clear = true;
    } else if (strcmp(name, "keeprook") == 0)                                {
        Par.keep_pc[R] = atoi(value);
        Glob.should_clear = true;
    } else if (strcmp(name, "keepqueen") == 0)                               {
        Par.keep_pc[Q] = atoi(value);
        Glob.should_clear = true;
    } else if (strcmp(name, "bishoppair") == 0)                              {
        Par.values[B_PAIR] = atoi(value);
        Glob.should_clear = true;
    } else if (strcmp(name, "exchangeimbalance") == 0)                       {
        Par.values[A_EXC] = atoi(value);
        Par.InitMaterialTweaks();
        Glob.should_clear = true;
    } else if (strcmp(name, "knightlikesclosed") == 0)                       {
        Par.values[N_CL] = atoi(value);
        Par.InitMaterialTweaks();
        Glob.should_clear = true;
    } else if (strcmp(name, "rooklikesopen") == 0)                           {
        Par.values[R_OP] = atoi(value);
        Par.InitMaterialTweaks();
        Glob.should_clear = true;
    } else if (strcmp(name, "material") == 0)                                {
        Par.mat_weight = atoi(value);
        Par.InitPst();
        Glob.should_clear = true;
    } else if (strcmp(name, "pieceplacement") == 0)                          {
        Par.pst_weight = atoi(value);
        Par.InitPst();
        Glob.should_clear = true;
    } else if (strcmp(name, "ownattack") == 0)                               {
        Par.own_att_weight = atoi(value);
        Glob.should_clear = true;
    } else if (strcmp(name, "oppattack") == 0)                               {
        Par.opp_att_weight = atoi(value);
        Glob.should_clear = true;
    } else if (strcmp(name, "ownmobility") == 0)                             {
        Par.own_mob_weight = atoi(value);
        Glob.should_clear = true;
    } else if (strcmp(name, "oppmobility") == 0)                             {
        Par.opp_mob_weight = atoi(value);
        Glob.should_clear = true;
    } else if (strcmp(name, "kingtropism") == 0)                             {
        Par.tropism_weight = atoi(value);
        Glob.should_clear = true;
    } else if (strcmp(name, "forwardness") == 0)                             {
        Par.forward_weight = atoi(value);
        Glob.should_clear = true;
    } else if (strcmp(name, "piecepressure") == 0)                           {
        Par.threats_weight = atoi(value);
        Glob.should_clear = true;
    } else if (strcmp(name, "passedpawns") == 0)                             {
        Par.passers_weight = atoi(value);
        Glob.should_clear = true;
    } else if (strcmp(name, "pawnstructure") == 0)                           {
        Par.struct_weight = atoi(value);
        Glob.should_clear = true;
    } else if (strcmp(name, "pawnmass") == 0)                                {
        Par.pawn_mass_weight = atoi(value);
        Glob.should_clear = true;
    } else if (strcmp(name, "pawnchains") == 0)                              {
        Par.pawn_chains_weight = atoi(value);
        Glob.should_clear = true;
    } else if (strcmp(name, "pawnshield") == 0)                              {
        Par.shield_weight = atoi(value);
        Glob.should_clear = true;
    } else if (strcmp(name, "pawnstorm") == 0)                               {
        Par.storm_weight = atoi(value);
        Glob.should_clear = true;
    } else if (strcmp(name, "outposts") == 0)                                {
        Par.outposts_weight = atoi(value);
        Glob.should_clear = true;
    } else if (strcmp(name, "lines") == 0)                                   {
        Par.lines_weight = atoi(value);
        Glob.should_clear = true;
    } else if (strcmp(name, "fianchetto") == 0)                              {
        Par.values[B_KING] = atoi(value);
        Glob.should_clear = true;
    } else if (strcmp(name, "returningb") == 0)                              {
        Par.values[B_RETURN] = atoi(value);
        Glob.should_clear = true;
    } else if (strcmp(name, "doubledpawnmg") == 0)                           {
        Par.values[DB_MID] = atoi(value);
        Glob.should_clear = true;
    } else if (strcmp(name, "doubledpawneg") == 0)                           {
        Par.values[DB_END] = atoi(value);
        Glob.should_clear = true;
    } else if (strcmp(name, "isolatedpawnmg") == 0)                          {
        Par.values[ISO_MG] = atoi(value);
        Glob.should_clear = true;
    } else if (strcmp(name, "isolatedpawneg") == 0)                          {
        Par.values[ISO_EG] = atoi(value);
        Glob.should_clear = true;
    } else if (strcmp(name, "isolatedopenmg") == 0)                          {
        Par.values[ISO_OF] = atoi(value);
        Glob.should_clear = true;
    } else if (strcmp(name, "backwardpawnmg") == 0)                          {
        Par.values[BK_MID] = atoi(value);
        Par.InitBackward();
        Glob.should_clear = true;
    } else if (strcmp(name, "backwardpawneg") == 0)                          {
        Par.values[BK_END] = atoi(value);
        Glob.should_clear = true;
    } else if (strcmp(name, "backwardopenmg") == 0)                          {
        Par.values[BK_OPE] = atoi(value);
        Glob.should_clear = true;
    } else if (strcmp(name, "pststyle") == 0)                                {
        Par.pst_style = atoi(value);
        Par.InitPst();
        Glob.should_clear = true;
    } else if (strcmp(name, "mobilitystyle") == 0)                           {
        Par.mob_style = atoi(value);
        Par.InitMobility();
        Glob.should_clear = true;

    // Here starts a block of very detailed options that are not meant
    // to be exposed to the end user, but are sybject to manual Texel tuning.

    } else if (strcmp(name, "na1") == 0 )                                    {
        Par.values[N_ATT1] = atoi(value);
        Glob.should_clear = true;
    } else if (strcmp(name, "na2") == 0 )                                    {
        Par.values[N_ATT2] = atoi(value);
        Glob.should_clear = true;
    } else if (strcmp(name, "ba1") == 0 )                                    {
        Par.values[B_ATT1] = atoi(value);
        Glob.should_clear = true;
    } else if (strcmp(name, "ba2") == 0 )                                    {
        Par.values[B_ATT2] = atoi(value);
        Glob.should_clear = true;
    } else if (strcmp(name, "ra1") == 0 )                                    {
        Par.values[R_ATT1] = atoi(value);
        Glob.should_clear = true;
    } else if (strcmp(name, "ra2") == 0 )                                    {
        Par.values[R_ATT2] = atoi(value);
        Glob.should_clear = true;
    } else if (strcmp(name, "qa1") == 0 )                                    {
        Par.values[Q_ATT1] = atoi(value);
        Glob.should_clear = true;
    } else if (strcmp(name, "qa2") == 0 )                                    {
        Par.values[Q_ATT2] = atoi(value);
        Glob.should_clear = true;
    } else if (strcmp(name, "nch") == 0 )                                    {
        Par.values[N_CHK] = atoi(value);
        Glob.should_clear = true;
    } else if (strcmp(name, "bch") == 0 )                                    {
        Par.values[B_CHK] = atoi(value);
        Glob.should_clear = true;
    } else if (strcmp(name, "rch") == 0 )                                    {
        Par.values[R_CHK] = atoi(value);
        Glob.should_clear = true;
    } else if (strcmp(name, "qch") == 0 )                                    {
        Par.values[Q_CHK] = atoi(value);
        Glob.should_clear = true;
    } else if (strcmp(name, "qcon") == 0 )                                   {
        Par.values[Q_CONTACT] = atoi(value);
        Glob.should_clear = true;
    } else if (strcmp(name, "rcon") == 0 )                                   {
        Par.values[R_CONTACT] = atoi(value);
        Glob.should_clear = true;
    } else if (strcmp(name, "shn") == 0 )                                    {
        Par.values[P_SH_NONE] = atoi(value);
        Glob.should_clear = true;
    } else if (strcmp(name, "sh2") == 0 )                                    {
        Par.values[P_SH_2] = atoi(value);
        Glob.should_clear = true;
    } else if (strcmp(name, "sh3") == 0 )                                    {
        Par.values[P_SH_3] = atoi(value);
        Glob.should_clear = true;
    } else if (strcmp(name, "sh4") == 0 )                                    {
        Par.values[P_SH_4] = atoi(value);
        Glob.should_clear = true;
    } else if (strcmp(name, "sh5") == 0 )                                    {
        Par.values[P_SH_5] = atoi(value);
        Glob.should_clear = true;
    } else if (strcmp(name, "sh6") == 0 )                                    {
        Par.values[P_SH_6] = atoi(value);
        Glob.should_clear = true;
    } else if (strcmp(name, "sh7") == 0 )                                    {
        Par.values[P_SH_7] = atoi(value);
        Glob.should_clear = true;
    } else if (strcmp(name, "stn") == 0 )                                    {
        Par.values[P_ST_OPEN] = atoi(value);
        Glob.should_clear = true;
    } else if (strcmp(name, "st3") == 0 )                                    {
        Par.values[P_ST_3] = atoi(value);
        Glob.should_clear = true;
    } else if (strcmp(name, "st4") == 0 )                                    {
        Par.values[P_ST_4] = atoi(value);
        Glob.should_clear = true;
    } else if (strcmp(name, "st5") == 0 )                                    {
        Par.values[P_ST_5] = atoi(value);
        Glob.should_clear = true;
    } else if (strcmp(name, "pmg2") == 0 )                                   {
        Par.values[PMG2] = atoi(value);
        Par.InitPassers();
        Glob.should_clear = true;
    } else if (strcmp(name, "pmg3") == 0 )                                   {
        Par.values[PMG3] = atoi(value);
        Par.InitPassers();
        Glob.should_clear = true;
    } else if (strcmp(name, "pmg4") == 0 )                                   {
        Par.values[PMG4] = atoi(value);
        Par.InitPassers();
        Glob.should_clear = true;
    } else if (strcmp(name, "pmg5") == 0 )                                   {
        Par.values[PMG5] = atoi(value);
        Par.InitPassers();
        Glob.should_clear = true;
    } else if (strcmp(name, "pmg6") == 0 )                                   {
        Par.values[PMG6] = atoi(value);
        Par.InitPassers();
        Glob.should_clear = true;
    } else if (strcmp(name, "pmg7") == 0 )                                   {
        Par.values[PMG7] = atoi(value);
        Par.InitPassers();
        Glob.should_clear = true;
    } else if (strcmp(name, "peg2") == 0 )                                   {
        Par.values[PEG2] = atoi(value);
        Par.InitPassers();
        Glob.should_clear = true;
    } else if (strcmp(name, "peg3") == 0 )                                   {
        Par.values[PEG3] = atoi(value);
        Par.InitPassers();
        Glob.should_clear = true;
    } else if (strcmp(name, "peg4") == 0 )                                   {
        Par.values[PEG4] = atoi(value);
        Par.InitPassers();
        Glob.should_clear = true;
    } else if (strcmp(name, "peg5") == 0 )                                   {
        Par.values[PEG5] = atoi(value);
        Par.InitPassers();
        Glob.should_clear = true;
    } else if (strcmp(name, "peg6") == 0 )                                   {
        Par.values[PEG6] = atoi(value);
        Par.InitPassers();
        Glob.should_clear = true;
    } else if (strcmp(name, "peg7") == 0 )                                   {
        Par.values[PEG7] = atoi(value);
        Par.InitPassers();
        Glob.should_clear = true;
    } else if (strcmp(name, "pbl") == 0 )                                    {
        Par.values[P_BL_MUL] = atoi(value);
        Glob.should_clear = true;
    } else if (strcmp(name, "pstopus") == 0 )                                {
        Par.values[P_OURSTOP_MUL] = atoi(value);
        Glob.should_clear = true;
    } else if (strcmp(name, "pstopthem") == 0 )                              {
        Par.values[P_OPPSTOP_MUL] = atoi(value);
        Glob.should_clear = true;
    } else if (strcmp(name, "pdefmul") == 0 )                                {
        Par.values[P_DEFMUL] = atoi(value);
        Glob.should_clear = true;
    } else if (strcmp(name, "pstopmul") == 0 )                               {
        Par.values[P_STOPMUL] = atoi(value);
        Glob.should_clear = true;
    } else if (strcmp(name, "pthr") == 0 )                                   {
        Par.values[P_THR] = atoi(value);
        Glob.should_clear = true;
    } else if (strcmp(name, "minorup") == 0)                                 {
        Par.values[A_MIN] = atoi(value);
		Par.InitMaterialTweaks();
        Glob.should_clear = true;
    } else if (strcmp(name, "majorup") == 0)                                 {
        Par.values[A_MAJ] = atoi(value);
		Par.InitMaterialTweaks();
        Glob.should_clear = true;
    } else if (strcmp(name, "bothup") == 0)                                  {
        Par.values[A_ALL] = atoi(value);
		Par.InitMaterialTweaks();
        Glob.should_clear = true;
    } else if (strcmp(name, "twominors") == 0)                               {
        Par.values[A_TWO] = atoi(value);
		Par.InitMaterialTweaks();
        Glob.should_clear = true;

    // Here starts a block of non-eval options

    } else if (strcmp(name, "guidebookfile") == 0)                           {
        if (!Glob.separate_books || !Glob.reading_personality)
            GuideBook.SetBookName(value);
    } else if (strcmp(name, "mainbookfile") == 0)                            {
        if (!Glob.separate_books || !Glob.reading_personality)
            MainBook.SetBookName(value);
    } else if (strcmp(name, "contempt") == 0)                                {
        Par.draw_score = atoi(value);
        Glob.should_clear = true;
    } else if (strcmp(name, "evalblur") == 0)                                {
        Par.eval_blur = atoi(value);
        Glob.should_clear = true;
    } else if (strcmp(name, "npslimit") == 0)                                {
        Par.nps_limit = atoi(value);
    } else if (strcmp(name, "uci_elo") == 0)                                 {
        Par.elo = atoi(value);
        Par.SetSpeed(Par.elo);
    } else if (strcmp(name, "uci_limitstrength") == 0)                       {
        valuebool(Par.fl_weakening, value);
    } else if (strcmp(name, "usebook") == 0)                                 {
        valuebool(Par.use_book, value);
    } else if (strcmp(name, "searchskill") == 0)                             {
        Par.search_skill = atoi(value);
        Glob.should_clear = true;
#ifdef USE_RISKY_PARAMETER
    } else if (strcmp(name, "riskydepth") == 0)                              {
        Par.riskydepth = atoi(value);
        Glob.should_clear = true;
#endif
    } else if (strcmp(name, "slowmover") == 0)                               {
        Par.time_percentage = atoi(value);
    } else if (strcmp(name, "selectivity") == 0)                             {
        Par.hist_perc = atoi(value);
        Par.hist_limit = -MAX_HIST + ((MAX_HIST * Par.hist_perc) / 100);
        Glob.should_clear = true;
    } else if (strcmp(name, "personalityfile") == 0)                         {
        ReadPersonality(value);
    } else if (strcmp(name, "personality") == 0 )                            {
        for (int i = 0; i < pers_aliases.count; i++)
            if (strcmp(pers_aliases.alias[i], value) == 0) {
                ReadPersonality(pers_aliases.path[i]);
                break;
            }
    }
}

// @brief function used to preserve personalities with old settings
// and to decrease number of parameters in UCI options panel

void SetPieceValue(int pc, int val, int slot) {

    Par.values[slot] = val;

    // Function SetPieceValue() modifies both midgame and endgame piece values.
    // It is tricky, so this ugly code ensures the same proportion between midgame
    // and endgame piece values as in default settings. Midgame and endgame piece
    // values can be set independently from each other using personality files.

    int eg_val = val;
    if (pc == P) eg_val = val + ((11 * val) / 95);
    if (pc == N) eg_val = val - (( 5 * val) / 320);
    if (pc == B) eg_val = val;
    if (pc == R) eg_val = val + (( 5 * val) / 515);
    if (pc == Q) eg_val = val + ((10 * val) / 1000);

    Par.values[slot + 1] = eg_val;
    Par.InitPst();
    Glob.should_clear = true;
}

void ReadPersonality(const char *fileName) {

    FILE *personalityFile = fopen(fileName, "r");

    printf("info string reading \'%s\' (%s)\n", fileName, personalityFile == NULL ? "failure" : "success");

    // exit if this personality file doesn't exist
    if (personalityFile == NULL)
        return;

    // set flag in case we want to disable some options while reading personality from a file
    Glob.reading_personality = true;

    char line[256], token[180]; int cnt = 0; char *pos;

    while (fgets(line, sizeof(line), personalityFile)) {    // read options line by line

        while ((pos = strpbrk(line, "\r\n"))) *pos = '\0'; // clean the sh!t

        // do we pick opening book within a personality?
        if (strstr(line, "PERSONALITY_BOOKS")) Glob.separate_books = false;
        if (strstr(line, "GENERAL_BOOKS"))     Glob.separate_books = true;

        // how we go about weakening the engine?
        if (strstr(line, "ELO_SLIDER")) Glob.elo_slider = true;
        if (strstr(line, "NPS_BLUR"))   Glob.elo_slider = false;

        // which UCI options are exposed to the user?
        if (strstr(line, "HIDE_OPTIONS")) Glob.use_personality_files = true;
        if (strstr(line, "SHOW_OPTIONS")) Glob.use_personality_files = false;

        if (strstr(line, "HIDE_PERSFILE")) Glob.show_pers_file = false;

        // aliases for personalities
        pos = strchr(line, '=');
        if (pos) {
            *pos = '\0';
            strncpy(pers_aliases.alias[cnt], line, PERSALIAS_ALEN-1); // -1 coz `strncpy` has a very unexpected glitch
            strncpy(pers_aliases.path[cnt], pos+1, PERSALIAS_PLEN-1); // see the C11 language standard, note 308
            cnt++;
            continue;
        }

        const char *ptr = ParseToken(line, token);
        if (strcmp(token, "setoption") == 0)
            ParseSetoption(ptr);
    }

    if (cnt) { // add a fake alias to allow to use PersonalityFile, ReadPersonality will fail on it keeping PersonalityFile values
        strcpy(pers_aliases.alias[cnt], "---");
        strcpy(pers_aliases.path[cnt], "///");
        cnt++;
    }
    if (cnt != 0) pers_aliases.count = cnt;
    fclose(personalityFile);
    Glob.reading_personality = false;
}
