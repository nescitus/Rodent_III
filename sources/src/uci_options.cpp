/*
Rodent, a UCI chess playing engine derived from Sungorus 1.4
Copyright (C) 2009-2011 Pablo Vazquez (Sungorus author)
Copyright (C) 2011-2018 Pawel Koziol

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

#define PERSALIAS_ALEN       32     // max length for a personality alias
#define PERSALIAS_PLEN       256    // max length for an alias path
#define PERSALIAS_MAXALIASES 100    // max number of aliases
struct {
    char alias[PERSALIAS_MAXALIASES][PERSALIAS_ALEN];
    char path[PERSALIAS_MAXALIASES][PERSALIAS_PLEN];
    int count;
} pers_aliases;

void PrintSingleOption(int ind) {
    printf("option name %s type spin default %d min %d max %d\n",
            paramNames[ind], Par.values[ind], Par.min_val[ind], Par.max_val[ind]);
}

void PrintUciOptions() {

	printf("option name Clear Hash type button\n");
    printf("option name Hash type spin default 16 min 1 max 4096\n");
#ifdef USE_THREADS
    printf("option name Threads type spin default %d min 1 max %d\n", Glob.thread_no, MAX_THREADS);
#endif
    printf("option name MultiPV type spin default %d min 1 max %d\n", Glob.multiPv, MAX_PV);
	printf("option name TimeBuffer type spin default %d min 0 max 1000\n", Glob.time_buffer);

    if (Glob.use_personality_files) {
        if (pers_aliases.count == 0 || Glob.show_pers_file)
            printf("option name PersonalityFile type string default default.txt\n");
        if (pers_aliases.count != 0) {
            printf("option name Personality type combo default ---"); // `---` in case we want PersonalityFile
            for (int i = 0; i < pers_aliases.count; i++)
                printf(" var %s", pers_aliases.alias[i]);
            printf("\n");
        }
    } else {

        printf("option name PawnValue type spin default %d min 0 max 1200\n", V(P_MID));
        printf("option name KnightValue type spin default %d min 0 max 1200\n", V(N_MID));
        printf("option name BishopValue type spin default %d min 0 max 1200\n", V(B_MID));
        printf("option name RookValue type spin default %d min 0 max 1200\n", V(R_MID));
        printf("option name QueenValue type spin default %d min 0 max 1200\n", V(Q_MID));

        printf("option name KeepPawn type spin default %d min 0 max 500\n", Par.keep_pc[P]);
        printf("option name KeepKnight type spin default %d min 0 max 500\n", Par.keep_pc[N]);
        printf("option name KeepBishop type spin default %d min 0 max 500\n", Par.keep_pc[B]);
        printf("option name KeepRook type spin default %d min 0 max 500\n", Par.keep_pc[R]);
        printf("option name KeepQueen type spin default %d min 0 max 500\n", Par.keep_pc[Q]);

        PrintSingleOption(B_PAIR);
        printf("option name ExchangeImbalance type spin default %d min -200 max 200\n", V(A_EXC));
        printf("option name KnightLikesClosed type spin default %d min 0 max 10\n", V(N_CL));

        PrintSingleOption(W_MATERIAL);
        printf("option name PstStyle type spin default %d min 0 max 3\n", Par.pst_style);
        printf("option name PiecePlacement type spin default %d min 0 max 500\n", V(W_PST));
        PrintSingleOption(W_OWN_ATT);
        PrintSingleOption(W_OPP_ATT);
        PrintSingleOption(W_OWN_MOB);
        PrintSingleOption(W_OPP_MOB);
        PrintSingleOption(W_TROPISM);
        PrintSingleOption(W_FWD);
        PrintSingleOption(W_THREATS);

        PrintSingleOption(W_PASSERS);
        PrintSingleOption(W_STRUCT);
        PrintSingleOption(W_SHIELD);
        PrintSingleOption(W_STORM);
        PrintSingleOption(W_OUTPOSTS);
        PrintSingleOption(W_LINES);

        printf("option name Fianchetto type spin default %d min 0 max 100\n", V(B_KING));

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
	printf("option name Verbose type check default %s\n", Glob.is_noisy ? "true" : "false");
    printf("option name Ponder type check default %s\n", Par.use_ponder ? "true" : "false");
    printf("option name UseBook type check default %s\n", Par.use_book ? "true" : "false");
    printf("option name VerboseBook type check default %s\n", Par.verbose_book ? "true" : "false");

    if (!Glob.use_books_from_pers || !Glob.use_personality_files) {
        printf("option name GuideBookFile type string default %s\n", GuideBook.bookName);
        printf("option name MainBookFile type string default %s\n", MainBook.bookName);
    }
}

static void valuebool(bool& param, char *val) {

    for (int i = 0; val[i]; i++)
        val[i] = tolower(val[i]);

    if (strcmp(val, "true")  == 0) param = true;
    if (strcmp(val, "false") == 0) param = false;
}

// @brief set a value that is a part of Par.values[]

static void setvalue(int ind, int val, bool isTable) {

    Par.values[ind] = val;
    if (isTable) Par.InitPst();
    Glob.should_clear = true;
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

    printf_debug("setoption name: '%s' value: '%s'\n", name, value );

    if (strcmp(name, "hash") == 0)                                           {
        Trans.AllocTrans(atoi(value));
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
        Trans.Clear();
    } else if (strcmp(name, "multipv") == 0)                                 {
        Glob.multiPv = atoi(value);
    } else if (strcmp(name, "timebuffer") == 0)                              {
        Glob.time_buffer = atoi(value);
    } else if (strcmp(name, "pawnvaluemg") == 0)                             {
        setvalue(P_MID, atoi(value), true);
    } else if (strcmp(name, "pawnvalueeg") == 0)                             {
        setvalue(P_END, atoi(value), true);
    } else if (strcmp(name, "pawnvalue") == 0)                               {
        SetPieceValue(P, atoi(value), P_MID);
    } else if (strcmp(name, "knightvaluemg") == 0)                           {
        setvalue(N_MID, atoi(value), true);
    } else if (strcmp(name, "knightvalueeg") == 0)                           {
        setvalue(N_END, atoi(value), true);
    } else if (strcmp(name, "knightvalue") == 0)                             {
        SetPieceValue(N, atoi(value), N_MID);
    } else if (strcmp(name, "bishopvaluemg") == 0)                           {
        setvalue(B_MID, atoi(value), true);
    } else if (strcmp(name, "bishopvalueeg") == 0)                           {
        setvalue(B_END, atoi(value), true);
    } else if (strcmp(name, "bishopvalue") == 0)                             {
        SetPieceValue(B, atoi(value), B_MID);
    } else if (strcmp(name, "rookvaluemg") == 0)                             {
        setvalue(R_MID, atoi(value), true);
    } else if (strcmp(name, "rookvalueeg") == 0)                             {
        setvalue(R_END, atoi(value), true);
    } else if (strcmp(name, "rookvalue") == 0)                               {
        SetPieceValue(R, atoi(value), R_MID);
    } else if (strcmp(name, "queenvaluemg") == 0)                            {
        setvalue(Q_MID, atoi(value), true);
    } else if (strcmp(name, "queenvalueeg") == 0)                            {
        setvalue(Q_END, atoi(value), true);
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
        setvalue(B_PAIR, atoi(value), false);
    } else if (strcmp(name, "exchangeimbalance") == 0)                       {
        Par.values[A_EXC] = atoi(value);
        Par.InitMaterialTweaks();
        Glob.should_clear = true;
    } else if (strcmp(name, "minorvsqueen") == 0)                            {
        setvalue(ELEPH, atoi(value), false);
    } else if (strcmp(name, "knightlikesclosed") == 0)                       {
        Par.values[N_CL] = atoi(value);
        Par.InitMaterialTweaks();
        Glob.should_clear = true;
    } else if (strcmp(name, "rooklikesopen") == 0)                           {
        Par.values[R_OP] = atoi(value);
        Par.InitMaterialTweaks();
        Glob.should_clear = true;
    } else if (strcmp(name, "material") == 0)                                {
        setvalue(W_MATERIAL, atoi(value), true);
    } else if (strcmp(name, "pieceplacement") == 0)                          {
        setvalue(W_MATERIAL, atoi(value), true);
    } else if (strcmp(name, "ownattack") == 0)                               {
        setvalue(W_OWN_ATT, atoi(value), false);
    } else if (strcmp(name, "oppattack") == 0)                               {
        setvalue(W_OPP_ATT, atoi(value), false);
    } else if (strcmp(name, "ownmobility") == 0)                             {
        setvalue(W_OWN_MOB, atoi(value), false);
    } else if (strcmp(name, "oppmobility") == 0)                             {
        setvalue(W_OPP_MOB, atoi(value), false);
    } else if (strcmp(name, "kingtropism") == 0)                             {
        setvalue(W_TROPISM, atoi(value), false);
    } else if (strcmp(name, "forwardness") == 0)                             {
        setvalue(W_FWD, atoi(value), false);
    } else if (strcmp(name, "piecepressure") == 0)                           {
        setvalue(W_THREATS, atoi(value), false);
    } else if (strcmp(name, "passedpawns") == 0)                             {
        setvalue(W_PASSERS, atoi(value), false);
    } else if (strcmp(name, "pawnstructure") == 0)                           {
        setvalue(W_STRUCT, atoi(value), false);
    } else if (strcmp(name, "pawnmass") == 0)                                {
        setvalue(W_MASS, atoi(value), false);
    } else if (strcmp(name, "pawnchains") == 0)                              {
        setvalue(W_CHAINS, atoi(value), false);
    } else if (strcmp(name, "pawnshield") == 0)                              {
        setvalue(W_SHIELD, atoi(value), false);
    } else if (strcmp(name, "pawnstorm") == 0)                               {
        setvalue(W_STORM, atoi(value), false);
    } else if (strcmp(name, "outposts") == 0)                                {
        setvalue(W_OUTPOSTS, atoi(value), false);
    } else if (strcmp(name, "lines") == 0)                                   {
        setvalue(W_LINES, atoi(value), false);
    } else if (strcmp(name, "center") == 0)                                  {
        setvalue(W_CENTER, atoi(value), false);
    } else if (strcmp(name, "fianchbase") == 0)                              {
        setvalue(B_FIANCH, atoi(value),false);
    } else if (strcmp(name, "fianchetto") == 0)                              {
        setvalue(B_KING, atoi(value), false);
    } else if (strcmp(name, "returningb") == 0)                              {
        setvalue(B_RETURN, atoi(value), false);
    } else if (strcmp(name, "doubledpawnmg") == 0)                           {
        setvalue(DB_MID, atoi(value), false);
    } else if (strcmp(name, "doubledpawneg") == 0)                           {
        setvalue(DB_END, atoi(value), false);
    } else if (strcmp(name, "isolatedpawnmg") == 0)                          {
        setvalue(ISO_MG, atoi(value), false);
    } else if (strcmp(name, "isolatedpawneg") == 0)                          {
        setvalue(ISO_EG, atoi(value), false);
    } else if (strcmp(name, "isolatedopenmg") == 0)                          {
        setvalue(ISO_OF, atoi(value), false);
    } else if (strcmp(name, "backwardpawnmg") == 0)                          {
        Par.values[BK_MID] = atoi(value);
        Par.InitBackward();
        Glob.should_clear = true;
    } else if (strcmp(name, "backwardpawneg") == 0)                          {
        setvalue(BK_END, atoi(value), false);
    } else if (strcmp(name, "backwardopenmg") == 0)                          {
        setvalue(BK_OPE, atoi(value), false);
    } else if (strcmp(name, "pststyle") == 0)                                {
        Par.pst_style = atoi(value);
        Par.InitPst();
        Glob.should_clear = true;
    } else if (strcmp(name, "mobilitystyle") == 0)                           {
        Par.mob_style = atoi(value);
        Par.InitMobility();
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
    } else if (strcmp(name, "minorbehindpawn") == 0 )                        {
		setvalue(N_SH_MG, atoi(value), false);
		setvalue(N_SH_EG, atoi(value), false);
        setvalue(B_SH_MG, atoi(value), false);
		setvalue(B_SH_EG, atoi(value), false);
    } else if (strcmp(name, "pawnthreat") == 0 )                             {
        setvalue(P_THR, atoi(value), false);

    // Here starts a block of non-eval options

    } else if (strcmp(name, "guidebookfile") == 0)                           {
        if (Glob.CanReadBook() ) GuideBook.SetBookName(value);
    } else if (strcmp(name, "mainbookfile") == 0)                            {
        if (Glob.CanReadBook() ) MainBook.SetBookName(value);
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
    } else if (strcmp(name, "verbose") == 0)                                 {
        valuebool(Glob.is_noisy, value);
    } else if (strcmp(name, "uci_limitstrength") == 0)                       {
        valuebool(Par.fl_weakening, value);
    } else if (strcmp(name, "ponder") == 0)                                  {
        valuebool(Par.use_ponder, value);
    } else if (strcmp(name, "usebook") == 0)                                 {
        valuebool(Par.use_book, value);
    } else if (strcmp(name, "verbosebook") == 0)                             {
        valuebool(Par.verbose_book, value);
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

    FILE *personalityFile = NULL;
    if (isabsolute(fileName)                    // if known locations don't exist we want to load only from absolute paths
        || ChDirEnv("RIIIPERSONALITIES")        // try `RIIIPERSONALITIES` env var first (26/08/17: linux only)
            || ChDir(_PERSONALITIESPATH))       // next built-in path
                personalityFile = fopen(fileName, "r");

    if (Glob.is_noisy)
        printf("info string reading personality '%s' (%s)\n", fileName, personalityFile == NULL ? "failure" : "success");

    // Exit if this personality file doesn't exist

    if (personalityFile == NULL)
        return;

    // It is possible that user will attempt to load a personality of older Rodent version.
    // There is nothing wrong with that, except that there will be some parameters missing.
    // and there will be no way of telling whether previous personality used their default
    // value or not. For that reason now that we found a personality file, we reset params
    // to the values that are best for setting a new personality.
    //
    // Please note that we use Par.InitialPersonalityWeights() and not Par.DefaultWeights().
    // This is because since version 0.214 Par.DefaultWeights() contains automatically tuned
    // values,  which are at times counterintuitive. In particular, king tropism values used
    // there  are negative, and that would break the king tropism functionality intended for
    // Rodent personalities.

    Par.InitialPersonalityWeights();

    // Set flag in case we want to disable some options while reading personality from a file

    Glob.reading_personality = true;

    char line[256], token[180]; int cnt = 0; char *pos;

    while (fgets(line, sizeof(line), personalityFile)) {    // read options line by line

        while ((pos = strpbrk(line, "\r\n"))) *pos = '\0'; // clean the sh!t

        // Do we pick opening book within a personality?

        if (strstr(line, "PERSONALITY_BOOKS")) Glob.use_books_from_pers = true; // DEFAULT
        if (strstr(line, "GENERAL_BOOKS"))     Glob.use_books_from_pers = false;

        // How we go about weakening the engine?

        if (strstr(line, "ELO_SLIDER")) Glob.elo_slider = true; // DEFAULT
        if (strstr(line, "NPS_BLUR"))   Glob.elo_slider = false;

        // Which UCI options are exposed to the user?

        if (strstr(line, "HIDE_OPTIONS")) Glob.use_personality_files = true;
        if (strstr(line, "SHOW_OPTIONS")) Glob.use_personality_files = false; // DEFAULT
        if (strstr(line, "HIDE_PERSFILE")) Glob.show_pers_file = false; // DEFAULT == true

        // Normally initializing a personality begins with loading a *human* default.
        // In order to use a set of parameters, obtained by Texel tuning method,
        // you need a secret code word. Since it affects parameters not exposed
        // for manual tuning, this word must be placed at the very beginning
        // of a personality file:

        if (strstr(line, "AUTOTUNED")) Par.DefaultWeights();

        // Aliases for personalities

        pos = strchr(line, '=');
        if (pos) {
            *pos = '\0';
            strncpy(pers_aliases.alias[cnt], line, PERSALIAS_ALEN-1); // -1 coz `strncpy` has a very unexpected glitch
            strncpy(pers_aliases.path[cnt], pos+1, PERSALIAS_PLEN-1); // see the C11 language standard, note 308
            cnt++;
            continue;
        }

        // Personality files use the same syntax as UCI options parser (yes I have been lazy)

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
    Par.SpeedToBookDepth(Par.nps_limit);
    Glob.reading_personality = false;
}
