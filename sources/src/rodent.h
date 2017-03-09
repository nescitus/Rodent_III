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

// REGEX to count all the lines under MSVC 13: ^(?([^\r\n])\s)*[^\s+?/]+[^\n]*$
// 6610 lines

// b15: 36.387.883 / 29,7 / 2.837

#pragma once

// define how Rodent is to be compiled

#define USE_MAGIC
#define USE_MM_POPCNT
#define USE_FIRST_ONE_INTRINSICS

enum eColor {WC, BC, NO_CL};
enum ePieceType {P, N, B, R, Q, K, NO_TP};
enum ePiece {WP, BP, WN, BN, WB, BBi, WR, BR, WQ, BQ, WK, BK, NO_PC};
enum eFile {FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H};
enum eRank {RANK_1, RANK_2, RANK_3, RANK_4, RANK_5, RANK_6, RANK_7, RANK_8};
enum eCastleFlag { W_KS = 1, W_QS = 2, B_KS = 4, B_QS = 8 };
enum eMoveType {NORMAL, CASTLE, EP_CAP, EP_SET, N_PROM, B_PROM, R_PROM, Q_PROM};
enum eMoveFlag { MV_NORMAL, MV_HASH, MV_CAPTURE, MV_REFUTATION, MV_KILLER, MV_BADCAPT };
enum eHashType {NONE, UPPER, LOWER, EXACT};
enum eSquare {
  A1, B1, C1, D1, E1, F1, G1, H1,
  A2, B2, C2, D2, E2, F2, G2, H2,
  A3, B3, C3, D3, E3, F3, G3, H3,
  A4, B4, C4, D4, E4, F4, G4, H4,
  A5, B5, C5, D5, E5, F5, G5, H5,
  A6, B6, C6, D6, E6, F6, G6, H6,
  A7, B7, C7, D7, E7, F7, G7, H7,
  A8, B8, C8, D8, E8, F8, G8, H8,
  NO_SQ
};

#define PHA_MG Q
#define DEF_MG K 
#define PHA_EG P
#define DEF_EG R 

#define MAX_PLY         64
#define MAX_MOVES       256
#define INF             32767
#define MATE            32000
#define MAX_EVAL        29999
#define MAX_HIST        (1 << 15)

typedef unsigned long long U64;

#define RANK_1_BB       (U64)0x00000000000000FF
#define RANK_2_BB       (U64)0x000000000000FF00
#define RANK_3_BB       (U64)0x0000000000FF0000
#define RANK_4_BB       (U64)0x00000000FF000000
#define RANK_5_BB       (U64)0x000000FF00000000
#define RANK_6_BB       (U64)0x0000FF0000000000
#define RANK_7_BB       (U64)0x00FF000000000000
#define RANK_8_BB       (U64)0xFF00000000000000

#define FILE_A_BB       (U64)0x0101010101010101
#define FILE_B_BB       (U64)0x0202020202020202
#define FILE_C_BB       (U64)0x0404040404040404
#define FILE_D_BB       (U64)0x0808080808080808
#define FILE_E_BB       (U64)0x1010101010101010
#define FILE_F_BB       (U64)0x2020202020202020
#define FILE_G_BB       (U64)0x4040404040404040
#define FILE_H_BB       (U64)0x8080808080808080

#define DIAG_A1H8_BB    (U64)0x8040201008040201
#define DIAG_A8H1_BB    (U64)0x0102040810204080
#define DIAG_B8H2_BB    (U64)0x0204081020408000

#define REL_SQ(sq,cl)   ( sq ^ (cl * 56) )
#define RelSqBb(sq,cl)  ( SqBb(REL_SQ(sq,cl) ) )

#define bbWhiteSq       (U64)0x55AA55AA55AA55AA
#define bbBlackSq       (U64)0xAA55AA55AA55AA55

static const U64 bbRelRank[2][8] = { { RANK_1_BB, RANK_2_BB, RANK_3_BB, RANK_4_BB, RANK_5_BB, RANK_6_BB, RANK_7_BB, RANK_8_BB },
                                     { RANK_8_BB, RANK_7_BB, RANK_6_BB, RANK_5_BB, RANK_4_BB, RANK_3_BB, RANK_2_BB, RANK_1_BB } };

static const U64 bbCentralFile = FILE_C_BB | FILE_D_BB | FILE_E_BB | FILE_F_BB;

#define SIDE_RANDOM     (~((U64)0))

#define START_POS       "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -"

#define SqBb(x)         ((U64)1 << (x))

#define Cl(x)           ((x) & 1)
#define Tp(x)           ((x) >> 1)
#define Pc(x, y)        (((y) << 1) | (x))

#define File(x)         ((x) & 7)
#define Rank(x)         ((x) >> 3)
#define Sq(x, y)        (((y) << 3) | (x))

#define Abs(x)          ((x) > 0 ? (x) : -(x))
#define Max(x, y)       ((x) > (y) ? (x) : (y))
#define Min(x, y)       ((x) < (y) ? (x) : (y))
#define Map0x88(x)      (((x) & 7) | (((x) & ~7) << 1))
#define Unmap0x88(x)    (((x) & 7) | (((x) & ~7) >> 1))
#define Sq0x88Off(x)    ((unsigned)(x) & 0x88)

#define Fsq(x)          ((x) & 63)
#define Tsq(x)          (((x) >> 6) & 63)
#define MoveType(x)     ((x) >> 12)
#define IsProm(x)       ((x) & 0x4000)
#define PromType(x)     (((x) >> 12) - 3)

#define Opp(x)          ((x) ^ 1)

#define InCheck(p)      Attacked(p, KingSq(p, p->side), Opp(p->side))
#define Illegal(p)      Attacked(p, KingSq(p, Opp(p->side)), p->side)
#define MayNull(p)      (((p)->cl_bb[(p)->side] & ~((p)->tp_bb[P] | (p)->tp_bb[K])) != 0)

#define PcBb(p, x, y)   ((p)->cl_bb[x] & (p)->tp_bb[y])
#define OccBb(p)        ((p)->cl_bb[WC] | (p)->cl_bb[BC])
#define UnoccBb(p)      (~OccBb(p))
#define TpOnSq(p, x)    (Tp((p)->pc[x]))
#define KingSq(p, x)    ((p)->king_sq[x])
#define IsOnSq(p, sd, pc, sq) ( PcBb(p, sd, pc) & SqBb(sq) )

#ifdef _WIN32
#define FORCEINLINE __forceinline
#else
#define FORCEINLINE __inline
#endif

// Compiler and architecture dependent versions of FirstOne() function,
// triggered by defines at the top of this file.
#ifdef USE_FIRST_ONE_INTRINSICS
#ifdef _WIN32
#include <intrin.h>
#ifdef _WIN64
#pragma intrinsic(_BitScanForward64)
#endif

#ifdef _MSC_VER
#ifndef _WIN64
const int lsb_64_table[64] =
{
	63, 30, 3, 32, 59, 14, 11, 33,
	60, 24, 50, 9, 55, 19, 21, 34,
	61, 29, 2, 53, 51, 23, 41, 18,
	56, 28, 1, 43, 46, 27, 0, 35,
	62, 31, 58, 4, 5, 49, 54, 6,
	15, 52, 12, 40, 7, 42, 45, 16,
	25, 57, 48, 13, 10, 39, 8, 44,
	20, 47, 38, 22, 17, 37, 36, 26
};

/**
* bitScanForward
* @author Matt Taylor (2003)
* @param bb bitboard to scan
* @precondition bb != 0
* @return index (0..63) of least significant one bit
*/
static int FORCEINLINE  bitScanForward(U64 bb) {
	unsigned int folded;
	bb ^= bb - 1;
	folded = (int)bb ^ (bb >> 32);
	return lsb_64_table[folded * 0x78291ACF >> 26];
}
#endif
#endif
static int FORCEINLINE FirstOne(U64 x) {
#ifndef _WIN64
	return bitScanForward(x);
#else
	unsigned long index = -1;
	_BitScanForward64(&index, x);
	return index;
#endif
}

#elif defined(__GNUC__)

static int FORCEINLINE FirstOne(U64 x) {
	int tmp = __builtin_ffsll(x);
	if (tmp == 0) return -1;
	else return tmp - 1;
}

#endif

#else
#define FirstOne(x)     bit_table[(((x) & (~(x) + 1)) * (U64)0x0218A392CD3D5DBF) >> 58] // first "1" in a bitboard
#endif

#define bbNotA          (U64)0xfefefefefefefefe // ~FILE_A_BB
#define bbNotH          (U64)0x7f7f7f7f7f7f7f7f // ~FILE_H_BB

#define ShiftNorth(x)   (x<<8)
#define ShiftSouth(x)   (x>>8)
#define ShiftWest(x)    ((x & bbNotA)>>1)
#define ShiftEast(x)    ((x & bbNotH)<<1)
#define ShiftNW(x)      ((x & bbNotA)<<7)
#define ShiftNE(x)      ((x & bbNotH)<<9)
#define ShiftSW(x)      ((x & bbNotA)>>9)
#define ShiftSE(x)      ((x & bbNotH)>>7)

#define JustOne(bb)     (bb && !(bb & (bb-1)))
#define MoreThanOne(bb) ( bb & (bb - 1) )

typedef class {
private:
	U64 p_attacks[2][64];
	U64 n_attacks[64];
	U64 k_attacks[64];

#ifndef USE_MAGIC
	U64 FillOcclSouth(U64 bb_start, U64 bb_block);
	U64 FillOcclNorth(U64 bb_start, U64 bb_block);
	U64 FillOcclEast(U64 bb_start, U64 bb_block);
	U64 FillOcclWest(U64 bb_start, U64 bb_block);
	U64 FillOcclNE(U64 bb_start, U64 bb_block);
	U64 FillOcclNW(U64 bb_start, U64 bb_block);
	U64 FillOcclSE(U64 bb_start, U64 bb_block);
	U64 FillOcclSW(U64 bb_start, U64 bb_block);
#endif

	U64 GetBetween(int sq1, int sq2);

public:
	U64 bbBetween[64][64];
	void Init(void);
	void Print(U64 bb);
	U64 ShiftFwd(U64 bb, int sd);
	U64 ShiftSideways(U64 bb);
	U64 GetWPControl(U64 bb);
	U64 GetBPControl(U64 bb);
	U64 GetPawnControl(U64 bb, int sd);
	U64 GetDoubleWPControl(U64 bb);
	U64 GetDoubleBPControl(U64 bb);
	U64 GetFrontSpan(U64 bb, int sd);
	U64 FillNorth(U64 bb);
	U64 FillSouth(U64 bb);
	U64 FillNorthSq(int sq);
	U64 FillSouthSq(int sq);
	U64 FillNorthExcl(U64 bb);
	U64 FillSouthExcl(U64 bb);

	int PopCnt(U64);
	int PopFirstBit(U64 * bb);

	U64 PawnAttacks(int sd, int sq);
	U64 KingAttacks(int sq);
	U64 KnightAttacks(int sq);
	U64 RookAttacks(U64 occ, int sq);
	U64 BishAttacks(U64 occ, int sq);
	U64 QueenAttacks(U64 occ, int sq);
} cBitBoard;

extern cBitBoard BB;

typedef struct {
  int ttp;
  int c_flags;
  int ep_sq;
  int rev_moves;
  U64 hash_key;
  U64 pawn_key;
} UNDO;

typedef class {
public:
  U64 cl_bb[2];
  U64 tp_bb[6];
  int pc[64];
  int king_sq[2];
  int phase;
  int cnt[2][6];
  int mg_sc[2];
  int eg_sc[2];
  int side;
  int c_flags;
  int ep_sq;
  int rev_moves;
  int head;
  U64 hash_key;
  U64 pawn_key;
  U64 rep_list[256];

  U64 Pawns(int sd) const { 
    return (cl_bb[sd] & tp_bb[P]);
  }

  U64 Knights(int sd) const {
    return (cl_bb[sd] & tp_bb[N]);
  }

  U64 Bishops(int sd) const {
    return (cl_bb[sd] & tp_bb[B]);
  }

  U64 Rooks(int sd) const {
    return (cl_bb[sd] & tp_bb[R]);
  }

  U64 Queens(int sd) const {
    return (cl_bb[sd] & tp_bb[Q]);
  }

  U64 Kings(int sd) const {
    return (cl_bb[sd] & tp_bb[K]);
  }

  U64 StraightMovers(int sd) const {
    return (cl_bb[sd] & (tp_bb[R] | tp_bb[Q]));
  }

  U64 DiagMovers(int sd) const {
    return (cl_bb[sd] & (tp_bb[B] | tp_bb[Q]));
  }

  void DoMove(int move, UNDO *u);
  void DoNull(UNDO *u);
  void UndoNull(UNDO *u);
  void UndoMove(int move, UNDO *u);

} POS;

typedef struct {
  POS *p;
  int phase;
  int trans_move;
  int ref_move;
  int ref_sq;
  int killer1;
  int killer2;
  int *next;
  int *last;
  int move[MAX_MOVES];
  int value[MAX_MOVES];
  int *badp;
  int bad[MAX_MOVES];
} MOVES;

typedef struct {
  U64 key;
  short date;
  short move;
  short score;
  unsigned char flags;
  unsigned char depth;
} ENTRY;

typedef struct {
  int mg[2];
  int eg[2];
  int mg_pawns[2];
  int eg_pawns[2];
  U64 p_takes[2];
  U64 two_pawns_take[2];
  U64 p_can_take[2];
  U64 all_att[2];
  U64 ev_att[2];
} eData;

struct sEvalHashEntry {
	U64 key;
	int score;
};

struct sPawnHashEntry {
	U64 key;
	int mg_pawns;
	int eg_pawns;
};

typedef class {
public:
  int use_book;
  int book_filter;
  int elo;
  int fl_weakening;
  int shut_up;
  int time_percentage;
  int draw_score;
  int prog_side;
  int search_skill;
  int nps_limit;
  int eval_blur;
  int hist_perc;
  int hist_limit;
  int pc_value[7];
  int keep_pc[7];
  int bish_pair;
  int exchange_imbalance;
  int imbalance[9][9];
  int n_likes_closed;
  int r_likes_open;
  int mat_weight;
  int pst_weight;
  int own_att_weight;
  int opp_att_weight;
  int own_mob_weight;
  int opp_mob_weight;
  int threats_weight;
  int tropism_weight;
  int forward_weight;
  int outposts_weight;
  int lines_weight;
  int shield_weight;
  int storm_weight;
  int struct_weight;
  int passers_weight;
  int doubled_mg;
  int doubled_eg;
  int isolated_mg;
  int isolated_eg;
  int isolated_open;
  int backward_mg;
  int backward_eg;
  int backward_open;
  int sd_att[2];
  int sd_mob[2];
  int mg_pst[2][6][64];
  int eg_pst[2][6][64];
  int sp_pst[2][6][64];
  int mob_style;
  int pst_style;
  int n_mob_mg[9];
  int n_mob_eg[9];
  int b_mob_mg[16];
  int b_mob_eg[16];
  int r_mob_mg[16];
  int r_mob_eg[16];
  int q_mob_mg[32];
  int q_mob_eg[32];
  int danger[512];
  int np_table[9];
  int rp_table[9];
  int backward_malus_mg[8];
  int protecting_bishop;
  int riskydepth;
  void InitPst(void);
  void InitMobility(void);
  void InitBackward(void);
  void InitMaterialTweaks(void);
  void InitTables(void);
  void DefaultWeights(void);
  void InitAsymmetric(POS * p);
  void SetSpeed(int elo);
  int EloToSpeed(int elo);
  int EloToBlur(int elo);
} cParam;

extern cParam Par;

typedef class {
  public:
  int metric[64][64]; // chebyshev distance for unstoppable passers
  int bonus[64][64];
  void Init(void);
} cDistance;

extern cDistance Dist;

typedef class {
public:
  void Init(void);
  U64 k_side;
  U64 q_side;
  U64 home[2];
  U64 away[2];
  U64 ks_castle[2];
  U64 qs_castle[2];
  U64 passed[2][64];
  U64 adjacent[8];
  U64 supported[2][64];
  U64 wb_special;
  U64 bb_special;
} cMask;

extern cMask Mask;

typedef class {
public:
  U64 nodes;
  int abort_search;
  int pondering;
  int separate_books;
  int use_personality_files;
  int reading_personality;
  int elo_slider;
  int is_console;
  int thread_no;
  int should_clear;
  int depth_reached;
  void ClearData(void);
  void Init(void);
} cGlobals;

extern cGlobals Glob;

void CheckTimeout(void);

#define EVAL_HASH_SIZE 512 * 512 / 4
#define PAWN_HASH_SIZE 512 * 512 / 4

typedef class {
public:

  sEvalHashEntry EvalTT[EVAL_HASH_SIZE];
  sPawnHashEntry PawnTT[PAWN_HASH_SIZE];
  int history[12][64];
  int killer[MAX_PLY][2];
  int refutation[64][64];
  int local_nodes;
  int dp_completed;
  int thread_id;
  int root_depth;

  void InitCaptures(POS *p, MOVES *m);
  void InitMoves(POS *p, MOVES *m, int trans_move, int ref_move, int ref_sq, int ply);
  int NextMove(MOVES *m, int * flag);
  int NextSpecialMove(MOVES *m, int * flag);
  int NextCapture(MOVES *m);
  void ScoreCaptures(MOVES *m);
  void ScoreQuiet(MOVES *m);
  int SelectBest(MOVES *m);
  int BadCapture(POS *p, int move);
  int MvvLva(POS *p, int move);
  void ClearHist(void);
  void ClearEvalHash(void);
  void ClearPawnHash(void);
  void ClearAll(void);
  int Refutation(int move);
  void UpdateHistory(POS *p, int last_move, int move, int depth, int ply);
  void DecreaseHistory(POS *p, int move, int depth);
  void TrimHist(void);
  
  void Bench(int depth);
  void Think(POS *p, int *pv);
  void Iterate(POS *p, int *pv);
  int Widen (POS *p, int depth, int * pv, int lastScore);
  int Search(POS *p, int ply, int alpha, int beta, int depth, int was_null, int last_move, int last_capt_sq, int *pv);
  int QuiesceChecks(POS *p, int ply, int alpha, int beta, int *pv);
  int QuiesceFlee(POS *p, int ply, int alpha, int beta, int *pv);
  int Quiesce(POS *p, int ply, int alpha, int beta, int *pv);
  int IsDraw(POS *p);
  int KPKdraw(POS *p, int sd);
  void DisplayPv(int score, int *pv);
  void Slowdown(void);

  void Init(int th);
  int Evaluate(POS * p, eData *e);
  int EvalScaleByDepth(POS *p, int ply, int eval);
  int EvaluateChains(POS *p, int sd);
  void EvaluateMaterial(POS * p, eData *e, int sd);
  void EvaluatePieces(POS *p, eData *e, int sd);
  void EvaluateOutpost(POS *p, eData *e, int pc, int sd, int sq, int * outpost);
  void EvaluatePawns(POS *p, eData *e, int sd);
  void EvaluatePassers(POS *p, eData *e, int sd);
  void EvaluateKing(POS *p, eData *e, int sd);
  void EvaluateKingFile(POS * p, int sd, U64 bb_file, int * shield, int * storm);
  int EvaluateFileShelter(U64 bb_own_pawns, int sd);
  int EvaluateFileStorm(U64 bb_opp_pawns, int sd);
  void EvaluatePawnStruct(POS * p, eData * e);
  void EvaluateUnstoppable(eData *e, POS * p);
  void EvaluateThreats(POS *p, eData *e, int sd);
  int ScalePawnsOnly(POS *p, int sd, int op);
  int ScaleKBPK(POS *p, int sd, int op);
  int ScaleKNPK(POS *p, int sd, int op);
  int ScaleKRPKR(POS *p, int sd, int op);
  int ScaleKQKRP(POS *p, int sd, int op);
  void EvaluateBishopPatterns(POS * p, eData * e);
  void EvaluateKnightPatterns(POS * p, eData * e);
  void EvaluateCentralPatterns(POS * p, eData * e);
  void EvaluateKingPatterns(POS * p, eData * e);
  int Interpolate(POS * p, eData * e);
  int GetDrawFactor(POS * p, int sd);
  int CheckmateHelper(POS *p);
  void Add(eData *e, int sd, int mg_val, int eg_val);
  void Add(eData *e, int sd, int val);
  void AddPawns(eData *e, int sd, int mg_val, int eg_val);
  int NotOnBishColor(POS * p, int bishSide, int sq);
  int DifferentBishops(POS * p);

} cEngine;

extern cEngine Engine1;
extern cEngine Engine2;
extern cEngine Engine3;
extern cEngine Engine4;

void InitSearch(void);
int BulletCorrection(int time);
int Clip(int sc, int lim);
void CopyPos(POS * old_pos, POS * new_pos);
void AllocTrans(int mbsize);
int Attacked(POS *p, int sq, int sd);
U64 AttacksFrom(POS *p, int sq);
U64 AttacksTo(POS *p, int sq);
void BuildPv(int *, int *, int);
void ClearTrans(void);
void ClearPosition(POS * p);
void DisplayCurrmove(int move, int tried);
int DrawScore(POS * p);
void ExtractMove(int pv[MAX_PLY]);
int *GenerateCaptures(POS *p, int *list);
int *GenerateQuiet(POS *p, int *list);
int *GenerateSpecial(POS *p, int *list);
int CanDiscoverCheck(POS *p, U64 bb_checkers, int op, int from); // for GenerateSpecial()
int GetMS(void);
U64 GetNps(int elapsed);
void Init(void);
int InputAvailable(void);
U64 InitHashKey(POS *p);
U64 InitPawnKey(POS *p);
int Legal(POS *p, int move);
void MoveToStr(int move, char *move_str);
void ParseGo(POS *p, char *ptr);
void ParseMoves(POS *p, char *ptr);
void ParsePosition(POS *p, char *ptr);
void ParseSetoption(char *);
char *ParseToken(char *, char *);
void PrintBoard(POS *p);
void PrintMove(int move);
void PrintUciOptions(void);
void PvToStr(int *, char *);
U64 Random64(void);
void ReadLine(char *, int);
void ReadPersonality(char *fileName);
void SetPosition(POS *p, char *epd);
void SetMoveTime(int base, int inc, int movestogo);
int StrToMove(POS *p, char *move_str);
int Swap(POS *p, int from, int to);
int TransRetrieve(U64 key, int *move, int *score, int alpha, int beta, int depth, int ply);
void TransRetrieveMove(U64 key, int *move);
void TransStore(U64 key, int move, int score, int flags, int depth, int ply);
void UciLoop(void);
void WasteTime(int miliseconds);

extern int castle_mask[64];
extern const int bit_table[64];
extern const int tp_value[7];
extern const int ph_value[7];
extern U64 zob_piece[12][64];
extern U64 zob_castle[16];
extern U64 zob_ep[8];
extern int move_time;
extern int move_nodes;
extern int search_depth;
extern int start_time;
extern ENTRY *tt;
extern int tt_size;
extern int tt_mask;
extern int tt_date;

// TODO: move from thread by depth, or if equal, by localnodes at the time of pv change
// TODO: perhaps don't search moves that has been searched by another thread to greater depth
// TODO: changing tt date of used entries (thx Kestutis)
// TODO: IID at cut nodes
// TODO: continuation move
// TODO: single move speedup
// TODO: easy move code
// TODO: stress test of ExtractMove at many threads
// TODO: cleanup of uci.cpp
// TODO: setting path to opening book
// TODO: no book moves in analyze mode
// TODO: fix small bug: engine crashes on empty book file path or empty personality file path
// TODO: transplant Risky parameter