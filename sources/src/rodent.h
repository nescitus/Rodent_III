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
// 6757 lines

// b15: 34.009.825

#pragma once

#if !(__cplusplus >= 201103L || _MSVC_LANG >= 201402)
    #error Rodent requires C++11 compatible compiler.
#endif

// catching memory leaks using MS Visual Studio
// https://docs.microsoft.com/en-us/visualstudio/debugger/finding-memory-leaks-using-the-crt-library
#if defined(_MSC_VER) && !defined(NDEBUG)
    #define _CRTDBG_MAP_ALLOC
    #include <stdlib.h>
    #include <crtdbg.h>
#endif

#include <cstdint>
#include <cinttypes>

using U64 = uint64_t;

// define how Rodent is to be compiled

#define USE_MAGIC
#ifndef NO_MM_POPCNT
    #define USE_MM_POPCNT
#endif
#define USE_FIRST_ONE_INTRINSICS
//#define USE_TUNING // needs epd.cpp, long compile time, huge file!!!

#define USE_RISKY_PARAMETER

// max size of an opening book to fully cache in memory (in MB)
#ifndef NO_BOOK_IN_MEMORY
    #define BOOK_IN_MEMORY_MB 16
#endif

#ifndef NO_THREADS
    #include <thread>
    #ifndef USE_THREADS
       #define USE_THREADS
    #endif
    #ifndef NEW_THREADS
        #define NEW_THREADS
    #endif
    #define MAX_THREADS 8
#else
    #undef USE_THREADS
#endif

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

constexpr int PHA_MG = Q;
constexpr int DEF_MG = K;
constexpr int PHA_EG = P;
constexpr int DEF_EG = R;

constexpr int MAX_PLY   = 64;
constexpr int MAX_MOVES = 256;
constexpr int INF       = 32767;
constexpr int MATE      = 32000;
constexpr int MAX_EVAL  = 29999;
constexpr int MAX_HIST  = 1 << 15;

constexpr U64 RANK_1_BB = 0x00000000000000FF;
constexpr U64 RANK_2_BB = 0x000000000000FF00;
constexpr U64 RANK_3_BB = 0x0000000000FF0000;
constexpr U64 RANK_4_BB = 0x00000000FF000000;
constexpr U64 RANK_5_BB = 0x000000FF00000000;
constexpr U64 RANK_6_BB = 0x0000FF0000000000;
constexpr U64 RANK_7_BB = 0x00FF000000000000;
constexpr U64 RANK_8_BB = 0xFF00000000000000;

constexpr U64 FILE_A_BB = 0x0101010101010101;
constexpr U64 FILE_B_BB = 0x0202020202020202;
constexpr U64 FILE_C_BB = 0x0404040404040404;
constexpr U64 FILE_D_BB = 0x0808080808080808;
constexpr U64 FILE_E_BB = 0x1010101010101010;
constexpr U64 FILE_F_BB = 0x2020202020202020;
constexpr U64 FILE_G_BB = 0x4040404040404040;
constexpr U64 FILE_H_BB = 0x8080808080808080;

constexpr U64 DIAG_A1H8_BB = 0x8040201008040201;
constexpr U64 DIAG_A8H1_BB = 0x0102040810204080;
constexpr U64 DIAG_B8H2_BB = 0x0204081020408000;

#define REL_SQ(sq,cl)   ( (sq) ^ ((cl) * 56) )
#define RelSqBb(sq,cl)  ( SqBb(REL_SQ(sq,cl) ) )

constexpr U64 bbWhiteSq = 0x55AA55AA55AA55AA;
constexpr U64 bbBlackSq = 0xAA55AA55AA55AA55;

constexpr U64 bb_rel_rank[2][8] = {
    { RANK_1_BB, RANK_2_BB, RANK_3_BB, RANK_4_BB, RANK_5_BB, RANK_6_BB, RANK_7_BB, RANK_8_BB },
    { RANK_8_BB, RANK_7_BB, RANK_6_BB, RANK_5_BB, RANK_4_BB, RANK_3_BB, RANK_2_BB, RANK_1_BB }
};

constexpr U64 bb_central_file = FILE_C_BB | FILE_D_BB | FILE_E_BB | FILE_F_BB;

constexpr U64 SIDE_RANDOM = ~UINT64_C(0);

constexpr char START_POS[] = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -";

#define SqBb(x)         ((U64)1 << (x))

#define Cl(x)           ((x) & 1)
#define Tp(x)           ((x) >> 1)
#define Pc(x, y)        (((y) << 1) | (x))

#define File(x)         ((x) & 7)
#define Rank(x)         ((x) >> 3)
#define Sq(x, y)        (((y) << 3) | (x))

//#define Abs(x)          ((x) > 0 ? (x) : -(x))
template<typename T> T Abs(const T& x) { return x > 0 ? x : -x; }
//#define Max(x, y)       ((x) > (y) ? (x) : (y))
template<typename T> constexpr const T& Max(const T& x, const T& y) { return x > y ? x : y; }
//#define Min(x, y)       ((x) < (y) ? (x) : (y))
template<typename T> constexpr const T& Min(const T& x, const T& y) { return x < y ? x : y; }

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

#ifndef FORCEINLINE
    #if defined(_MSC_VER)
        #define FORCEINLINE __forceinline
    #else
        #define FORCEINLINE __inline
    #endif
#endif

#ifndef NOINLINE
    #if defined(_MSC_VER)
        #define NOINLINE __declspec(noinline)
    #else
        #define NOINLINE __attribute__((noinline))
    #endif
#endif

// Compiler and architecture dependent versions of FirstOne() function,
// triggered by defines at the top of this file.
#ifdef USE_FIRST_ONE_INTRINSICS

    #if defined(_MSC_VER)

        #include <intrin.h>

        #ifndef _WIN64
            #pragma intrinsic(_BitScanForward)
        #else
            #pragma intrinsic(_BitScanForward64)
        #endif

        static int FORCEINLINE FirstOne(U64 x) {
            unsigned long index;
        #ifndef _WIN64
            if (_BitScanForward(&index, (unsigned long)x)) return index;
            _BitScanForward(&index, x >> 32); return index + 32;
        #else
            _BitScanForward64(&index, x);
            return index;
        #endif
        }

    #elif defined(__GNUC__)

        static inline int FirstOne(U64 x) {

        // workaround for GCC's inability to inline __builtin_ctzll() on x32 (it calls `__ctzdi2` runtime function instead)
        #if !defined(__amd64__) && defined(__i386__) && !defined(__clang__)
            const uint32_t xlo = (uint32_t)x;
            return xlo ? __builtin_ctz(xlo) : __builtin_ctz(x >> 32) + 32;
        #else
            return __builtin_ctzll(x);
        #endif
        }

    #endif

#else
    const int bit_table[64] = {
        0,  1,  2,  7,  3, 13,  8, 19,
        4, 25, 14, 28,  9, 34, 20, 40,
        5, 17, 26, 38, 15, 46, 29, 48,
       10, 31, 35, 54, 21, 50, 41, 57,
       63,  6, 12, 18, 24, 27, 33, 39,
       16, 37, 45, 47, 30, 53, 49, 56,
       62, 11, 23, 32, 36, 44, 52, 55,
       61, 22, 43, 51, 60, 42, 59, 58
    };
    #define FirstOne(x)     bit_table[(((x) & (~(x) + 1)) * (U64)0x0218A392CD3D5DBF) >> 58] // first "1" in a bitboard
#endif

constexpr U64 bbNotA = ~FILE_A_BB; // 0xfefefefefefefefe
constexpr U64 bbNotH = ~FILE_H_BB; // 0x7f7f7f7f7f7f7f7f

#define ShiftNorth(x)   ((x)<<8)
#define ShiftSouth(x)   ((x)>>8)
#define ShiftWest(x)    (((x) & bbNotA)>>1)
#define ShiftEast(x)    (((x) & bbNotH)<<1)
#define ShiftNW(x)      (((x) & bbNotA)<<7)
#define ShiftNE(x)      (((x) & bbNotH)<<9)
#define ShiftSW(x)      (((x) & bbNotA)>>9)
#define ShiftSE(x)      (((x) & bbNotH)>>7)

//#define JustOne(bb)     ((bb) && !((bb) & ((bb)-1)))
//#define MoreThanOne(bb) ((bb) & ((bb) - 1))
template<typename T> bool MoreThanOne(const T& bb) { return bb & (bb - 1); }

class cBitBoard {
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
    void Init();
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
    int PopFirstBit(U64 *bb);

    U64 PawnAttacks(int sd, int sq);
    U64 KingAttacks(int sq);
    U64 KnightAttacks(int sq);
    U64 RookAttacks(U64 occ, int sq);
    U64 BishAttacks(U64 occ, int sq);
    U64 QueenAttacks(U64 occ, int sq);
};

extern cBitBoard BB;

struct UNDO {
    int ttp;
    int c_flags;
    int ep_sq;
    int rev_moves;
    U64 hash_key;
    U64 pawn_key;
};

class POS {
    static int castle_mask[64];
    static U64 zob_piece[12][64];
    static U64 zob_castle[16];
    static U64 zob_ep[8];
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

    static void Init();

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

    void InitHashKey();
    void InitPawnKey();

};

struct MOVES {
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
};

struct ENTRY {
    U64 key;
    short date;
    short move;
    short score;
    unsigned char flags;
    unsigned char depth;
};

struct eData {
    int mg[2];
    int eg[2];
    int mg_pawns[2];
    int eg_pawns[2];
    U64 p_takes[2];
    U64 two_pawns_take[2];
    U64 p_can_take[2];
    U64 all_att[2];
    U64 ev_att[2];
};

struct sEvalHashEntry {
    U64 key;
    int score;
};

struct sPawnHashEntry {
    U64 key;
    int mg_pawns;
    int eg_pawns;
};

enum Values {
    P_MID, P_END, N_MID, N_END, B_MID, B_END, R_MID, R_END, Q_MID, Q_END,               // piece values
    B_PAIR, N_PAIR, R_PAIR, ELEPH, A_EXC, A_TWO, A_MAJ, A_MIN, A_ALL,                   // material adjustments
    N_ATT1, N_ATT2, B_ATT1, B_ATT2, R_ATT1, R_ATT2, Q_ATT1, Q_ATT2,                     // attacks against enemy king zone
    N_CHK, B_CHK, R_CHK, Q_CHK, R_CONTACT, Q_CONTACT,                                   // check threats
    NTR_MG, NTR_EG, BTR_MG, BTR_EG, RTR_MG, RTR_EG, QTR_MG, QTR_EG,                     // king tropism
    N_FWD, B_FWD, R_FWD, Q_FWD, N_OWH, B_OVH, N_REACH, BN_SHIELD,
    N_CL, R_OP, N_TRAP, N_BLOCK, K_NO_LUFT, K_CASTLE,
    B_TRAP_A2, B_TRAP_A3, B_BLOCK, B_FIANCH, B_BADF, B_KING, B_BF_MG, B_BF_EG, B_WING,  // bishop parameters
    B_OPP_P, B_OWN_P, B_REACH, B_TOUCH, B_RETURN,
    P_SH_NONE, P_SH_2, P_SH_3, P_SH_4, P_SH_5, P_SH_6, P_SH_7,                          // king's pawn shield
    P_ST_OPEN, P_ST_3, P_ST_4, P_ST_5,                                                  // pawn storm on enemy king
    ISO_MG, ISO_EG, ISO_OF, BK_MID, BK_END, BK_OPE, DB_MID, DB_END,                     // pawn weaknesses
    PMG2, PMG3, PMG4, PMG5, PMG6, PMG7, PEG2, PEG3, PEG4, PEG5, PEG6, PEG7,             // passed pawns
    P_BL_MUL, P_OURSTOP_MUL, P_OPPSTOP_MUL, P_DEFMUL, P_STOPMUL, P_THR, P_BIND, P_ISL,  // pawn special terms
    ROF_MG, ROF_EG, RGH_MG, RGH_EG, RBH_MG, RBH_EG, RSR_MG, RSR_EG, ROQ_MG, ROQ_EG,     // rook bonuses
    RS2_MG, RS2_EG, QSR_MG, QSR_EG, R_BLOCK, N_OF_VAL                                   // queen and rook bonuses
};

class cParam {
  public:
    int values[N_OF_VAL];
    bool use_book;
    bool verbose_book;
    int book_filter;
    int book_depth;
    int elo;
    bool fl_weakening;
    bool shut_up;
    int time_percentage;
    int draw_score;
    int prog_side;
    int search_skill;
    int nps_limit;
    int eval_blur;
    int hist_perc;
    int hist_limit;
    int keep_pc[7];
    int imbalance[9][9];
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
	int center_weight;
	int pawn_mass_weight;
	int pawn_chains_weight;
    int sd_att[2];
    int sd_mob[2];
    int mg_pst[2][6][64];
    int eg_pst[2][6][64];
    int sp_pst[2][6][64];
    int passed_bonus_mg[2][8];
    int passed_bonus_eg[2][8];
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
#ifdef USE_RISKY_PARAMETER
    int riskydepth;
#endif
    NOINLINE void InitPst();
    NOINLINE void InitMobility();
    NOINLINE void InitBackward();
    NOINLINE void InitPassers();
    NOINLINE void InitMaterialTweaks();
    NOINLINE void InitTables();
    NOINLINE void DefaultWeights();
    NOINLINE void InitAsymmetric(POS *p);
    NOINLINE void SetSpeed(int elo_in);
    NOINLINE int EloToSpeed(int elo_in);
    NOINLINE int EloToBlur(int elo_in);
    NOINLINE int EloToBookDepth(int elo_in);
    NOINLINE void SetVal(int slot, int val);
};

extern cParam Par;

class cDistance {
  public:
    int metric[64][64]; // chebyshev distance for unstoppable passers
    int bonus[64][64];
    void Init();
};

extern cDistance Dist;

class cMask {
  public:
    void Init();

    static constexpr U64 home[2] = { RANK_1_BB | RANK_2_BB | RANK_3_BB | RANK_4_BB,
                                     RANK_8_BB | RANK_7_BB | RANK_6_BB | RANK_5_BB };
    static constexpr U64 away[2] = { RANK_8_BB | RANK_7_BB | RANK_6_BB | RANK_5_BB,
                                     RANK_1_BB | RANK_2_BB | RANK_3_BB | RANK_4_BB };

    static constexpr U64 ks_castle[2] = { SqBb(F1) | SqBb(G1) | SqBb(H1) | SqBb(F2) | SqBb(G2) | SqBb(H2),
                                          SqBb(F8) | SqBb(G8) | SqBb(H8) | SqBb(F7) | SqBb(G7) | SqBb(H7) };
    static constexpr U64 qs_castle[2] = { SqBb(A1) | SqBb(B1) | SqBb(C1) | SqBb(A2) | SqBb(B2) | SqBb(C2),
                                          SqBb(A8) | SqBb(B8) | SqBb(C8) | SqBb(A7) | SqBb(B7) | SqBb(C7) };

    static constexpr U64 outpost_map[2] = { (bb_rel_rank[WC][RANK_4] | bb_rel_rank[WC][RANK_5] | bb_rel_rank[WC][RANK_6]) & bbNotA & bbNotH,
                                            (bb_rel_rank[BC][RANK_4] | bb_rel_rank[BC][RANK_5] | bb_rel_rank[BC][RANK_6]) & bbNotA & bbNotH };

    static constexpr U64 k_side = FILE_F_BB | FILE_G_BB | FILE_H_BB;
    static constexpr U64 q_side = FILE_A_BB | FILE_B_BB | FILE_C_BB;

    static constexpr U64 wb_special = SqBb(A7) | SqBb(A6) | SqBb(B8) | SqBb(H7) | SqBb(H6) | SqBb(G8) | SqBb(C1) | SqBb(F1) | SqBb(G2) | SqBb(B2);
    static constexpr U64 bb_special = SqBb(A2) | SqBb(A3) | SqBb(B1) | SqBb(H2) | SqBb(H3) | SqBb(G1) | SqBb(C8) | SqBb(F8) | SqBb(G7) | SqBb(B7);

    U64 adjacent[8];
    U64 passed[2][64];
    U64 supported[2][64];

    static_assert(WC == 0 && BC == 1, "must be WC == 0 && BC == 1");
};

extern cMask Mask;

#if defined(USE_THREADS) && defined(NEW_THREADS)
    #include <atomic>

    typedef std::atomic<bool>     glob_bool;
    typedef std::atomic<int>      glob_int;
    typedef std::atomic<uint64_t> glob_U64;
#else
    typedef bool glob_bool;
    typedef int  glob_int;
    typedef U64  glob_U64;
#endif

class cGlobals {
  public:
    glob_U64 nodes;
    glob_bool abort_search;
    glob_bool is_testing;
    bool elo_slider;
    bool is_console;
    bool is_tuning;
    glob_bool pondering;
    bool reading_personality;
    bool use_books_from_pers;
    bool should_clear;
    bool goodbye;
    bool use_personality_files;
    bool show_pers_file;
    glob_int depth_reached;
    int moves_from_start; // to restrict book depth for weaker levels
    int thread_no;

    void ClearData();
    void Init();
};

extern cGlobals Glob;

#ifdef USEGEN
    #define GIMMESIZE
    #include "book_gen.h"
    #undef GIMMESIZE
#endif

#ifdef PACKSTRUCT
    #pragma pack(push, 1)
    struct sBookEntry {
        U64 hash;
        uint16_t move;
        int16_t freq;
    };
    #pragma pack(pop)
#else
    struct sBookEntry {
        U64 hash;
        int move;
        int freq;
    };
#endif

struct sInternalBook {
  public:

    int n_of_records;

#ifdef USEGEN
    sBookEntry internal_book[BOOKSIZE];
    void Init() const;
#else
    sBookEntry internal_book[48000];
    void Init();
    bool LineToInternal(const char *ptr, int excludedColor);
    void MoveToInternal(U64 hashKey, int move, int val);
#endif

    int MoveFromInternal(POS *p, bool print_output) const;
};

#define ZEROARRAY(x) memset(x, 0, sizeof(x));

extern
#ifdef USEGEN
    const
#endif
sInternalBook InternalBook;

void CheckTimeout();

constexpr int EVAL_HASH_SIZE = 512 * 512 / 4;
constexpr int PAWN_HASH_SIZE = 512 * 512 / 4;

class cEngine {
    sEvalHashEntry EvalTT[EVAL_HASH_SIZE];
    sPawnHashEntry PawnTT[PAWN_HASH_SIZE];
    int history[12][64];
    int killer[MAX_PLY][2];
    int refutation[64][64];
    //int local_nodes;
    const int thread_id;
    int root_depth;
    bool fl_root_choice;

    void InitCaptures(POS *p, MOVES *m);
    void InitMoves(POS *p, MOVES *m, int trans_move, int ref_move, int ref_sq, int ply);
    int NextMove(MOVES *m, int *flag);
    int NextSpecialMove(MOVES *m, int *flag);
    int NextCapture(MOVES *m);
    void ScoreCaptures(MOVES *m);
    void ScoreQuiet(MOVES *m);
    int SelectBest(MOVES *m);
    int BadCapture(POS *p, int move);
    int MvvLva(POS *p, int move);
    void ClearHist();
    void AgeHist();
    void ClearEvalHash();
    void ClearPawnHash();
    int Refutation(int move);
    void UpdateHistory(POS *p, int last_move, int move, int depth, int ply);
    void DecreaseHistory(POS *p, int move, int depth);
    void TrimHist();

    void Iterate(POS *p, int *pv);
    int Widen(POS *p, int depth, int *pv, int lastScore);
    int Search(POS *p, int ply, int alpha, int beta, int depth, int was_null, int last_move, int last_capt_sq, int *pv);
    int QuiesceChecks(POS *p, int ply, int alpha, int beta, int *pv);
    int QuiesceFlee(POS *p, int ply, int alpha, int beta, int *pv);
    int Quiesce(POS *p, int ply, int alpha, int beta, int *pv);
    bool IsDraw(POS *p);
    bool KPKdraw(POS *p, int sd);
    void DisplayPv(int score, int *pv);
    void Slowdown();

    int Evaluate(POS *p, eData *e);
#ifdef USE_RISKY_PARAMETER
    int EvalScaleByDepth(POS *p, int ply, int eval);
#endif
    int EvaluateChains(POS *p, int sd);
    void EvaluateMaterial(POS *p, eData *e, int sd);
    void EvaluatePieces(POS *p, eData *e, int sd);
    void EvaluateOutpost(POS *p, eData *e, int pc, int sd, int sq, int *outpost);
    void EvaluatePawns(POS *p, eData *e, int sd);
    void EvaluatePassers(POS *p, eData *e, int sd);
    void EvaluateKing(POS *p, eData *e, int sd);
    void EvaluateKingFile(POS *p, int sd, U64 bb_file, int *shield, int *storm);
    int EvaluateFileShelter(U64 bb_own_pawns, int sd);
    int EvaluateFileStorm(U64 bb_opp_pawns, int sd);
    void EvaluatePawnStruct(POS *p, eData *e);
    void EvaluateUnstoppable(eData *e, POS *p);
    void EvaluateThreats(POS *p, eData *e, int sd);
    int ScalePawnsOnly(POS *p, int sd, int op);
    int ScaleKBPK(POS *p, int sd, int op);
    int ScaleKNPK(POS *p, int sd, int op);
    int ScaleKRPKR(POS *p, int sd, int op);
    int ScaleKQKRP(POS *p, int sd, int op);
    void EvaluateBishopPatterns(POS *p, eData *e);
    void EvaluateKnightPatterns(POS *p, eData *e);
    void EvaluateCentralPatterns(POS *p, eData *e);
    void EvaluateKingPatterns(POS *p, eData *e);
    int Interpolate(POS *p, eData *e);
    int GetDrawFactor(POS *p, int sd);
    int CheckmateHelper(POS *p);
    void Add(eData *e, int sd, int mg_val, int eg_val);
    void Add(eData *e, int sd, int val);
    void AddPawns(eData *e, int sd, int mg_val, int eg_val);
    bool NotOnBishColor(POS *p, int bish_side, int sq);
    bool DifferentBishops(POS *p);

    static const int razor_margin[];
    static const int fut_margin[];
	static const int selective_depth;
	static const int snp_depth;      // max depth at which static null move pruning is applied
	static const int razor_depth;    // max depth at which razoring is applied
	static const int fut_depth;      // max depth at which futility pruning is applied
    static int lmr_size[2][MAX_PLY][MAX_MOVES];

  public:

    static void InitSearch();

    int pv_eng[MAX_PLY];
    int dp_completed;

    cEngine(const cEngine&) = delete;
    cEngine& operator=(const cEngine&) = delete;
    cEngine(int th = 0): thread_id(th) { ClearAll(); };

#ifdef USE_THREADS
    std::thread worker;
    void StartThinkThread(POS *p) {
        dp_completed = 0;
        worker = std::thread([&] { Think(p); });
    }

    ~cEngine() { WaitThinkThread(); };  // should fix crash on windows on console closing
    void WaitThinkThread() { if (worker.joinable()) worker.join(); }
#endif

    void Bench(int depth);
    void ClearAll();
    void Think(POS *p);
    double TexelFit(POS *p, int *pv);
};

#ifdef USE_THREADS
    #include <list>
    extern std::list<cEngine> Engines;
#else
    extern cEngine EngineSingle;
#endif

void PrintVersion();

int BulletCorrection(int time);
int Clip(int sc, int lim);
void AllocTrans(unsigned int mbsize);
bool Attacked(POS *p, int sq, int sd);
U64 AttacksFrom(POS *p, int sq);
U64 AttacksTo(POS *p, int sq);
void BuildPv(int *dst, int *src, int move);
void ClearTrans();
void ClearPosition(POS *p);
void DisplayCurrmove(int move, int tried);
int DrawScore(POS *p);
void ExtractMove(int *pv);
int *GenerateCaptures(POS *p, int *list);
int *GenerateQuiet(POS *p, int *list);
int *GenerateSpecial(POS *p, int *list);
bool CanDiscoverCheck(POS *p, U64 bb_checkers, int op, int from); // for GenerateSpecial()
int GetMS();
U64 GetNps(int elapsed);
bool InputAvailable();
bool Legal(POS *p, int move);
void MoveToStr(int move, char *move_str);
void ParseGo(POS *p, const char *ptr);
void ParseMoves(POS *p, const char *ptr);
void ParsePosition(POS *p, const char *ptr);
void ParseSetoption(const char *);
const char *ParseToken(const char *, char *);
void PrintBoard(POS *p);
void PrintMove(int move);
void PrintUciOptions();
void PvToStr(int *, char *);
U64 Random64();
void ReadLine(char *, int);
void ReadPersonality(const char *fileName);
void SetPosition(POS *p, const char *epd);
void SetMoveTime(int base, int inc, int movestogo);
void SetPieceValue(int pc, int val, int slot);
int StrToMove(POS *p, char *move_str);
int Swap(POS *p, int from, int to);
bool TransRetrieve(U64 key, int *move, int *score, int alpha, int beta, int depth, int ply);
void TransRetrieveMove(U64 key, int *move);
void TransStore(U64 key, int move, int score, int flags, int depth, int ply);
void UciLoop();
void WasteTime(int miliseconds);
void PrintBb(U64 bbTest);
int big_random(int n);

extern const int tp_value[7];
extern const int ph_value[7];
extern int move_time;
extern int move_nodes;
extern int search_depth;
extern int start_time;

extern unsigned int tt_size;
extern unsigned int tt_mask;
extern int tt_date;

#define MAKESTRHLP(x) #x
#define MAKESTR(x) MAKESTRHLP(x)

// macro BOOKSPATH is where books live, default is relative "books/"
// macro PERSONALITIESPATH is where personalities and `basic.ini` live, default is relative "personalities/"

#if defined(_WIN32) || defined(_WIN64)
    #if defined(BOOKSPATH)
        #define _BOOKSPATH MAKESTR(BOOKSPATH) L""
    #else
        #define _BOOKSPATH L"books\\"
    #endif
    #if defined(PERSONALITIESPATH)
        #define _PERSONALITIESPATH MAKESTR(PERSONALITIESPATH) L""
    #else
        #define _PERSONALITIESPATH L"personalities\\"
    #endif
    #define PrintOverrides() {}
    // change dir and return true on success
    #define ChDirEnv(dummy) false
    bool ChDir(const wchar_t *new_path);
#else
    #if defined(BOOKSPATH)
        #define _BOOKSPATH MAKESTR(BOOKSPATH) ""
    #else
        #define _BOOKSPATH "books/"
    #endif
    #if defined(PERSONALITIESPATH)
        #define _PERSONALITIESPATH MAKESTR(PERSONALITIESPATH) ""
    #else
        #define _PERSONALITIESPATH "personalities/"
    #endif
    void PrintOverrides();
    // change dir and return true on success
    bool ChDirEnv(const char *env_name);
    bool ChDir(const char *new_path);
#endif

#ifndef NDEBUG
    #define printf_debug(...) printf("(debug) " __VA_ARGS__)
#else
    #define printf_debug(...) {}
#endif

// TODO: move from thread by depth, or if equal, by localnodes at the time of pv change
// TODO: perhaps don't search moves that has been searched by another thread to greater depth
// TODO: changing tt date of used entries (thx Kestutis)
// TODO: IID at cut nodes
// TODO: continuation move
// TODO: easy move code
// TODO: no book moves in analyze mode
// TODO: fix small bug: engine crashes on empty book file path or empty personality file path
// TODO: minor defended by pawn and something else (to decrease the probability of getting doubled pawns)
// TODO: rook and queen on7th rank

double TexelSigmoid(int score, double k);
