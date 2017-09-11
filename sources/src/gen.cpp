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

int *GenerateCaptures(POS *p, int *list) {

    U64 bb_pieces, bb_moves;
    int from, to;

    int sd = p->side;
    int op = Opp(sd);

    if (sd == WC) {
        bb_moves = ((p->Pawns(WC) & ~FILE_A_BB & RANK_7_BB) << 7) & p->cl_bb[BC];
        while (bb_moves) {
            to = BB.PopFirstBit(&bb_moves);
            *list++ = (Q_PROM << 12) | (to << 6) | (to - 7);
            *list++ = (R_PROM << 12) | (to << 6) | (to - 7);
            *list++ = (B_PROM << 12) | (to << 6) | (to - 7);
            *list++ = (N_PROM << 12) | (to << 6) | (to - 7);
        }

        bb_moves = ((p->Pawns(WC) & ~FILE_H_BB & RANK_7_BB) << 9) & p->cl_bb[BC];
        while (bb_moves) {
            to = BB.PopFirstBit(&bb_moves);
            *list++ = (Q_PROM << 12) | (to << 6) | (to - 9);
            *list++ = (R_PROM << 12) | (to << 6) | (to - 9);
            *list++ = (B_PROM << 12) | (to << 6) | (to - 9);
            *list++ = (N_PROM << 12) | (to << 6) | (to - 9);
        }

        bb_moves = ((p->Pawns(WC) & RANK_7_BB) << 8) & p->UnoccBb();
        while (bb_moves) {
            to = BB.PopFirstBit(&bb_moves);
            *list++ = (Q_PROM << 12) | (to << 6) | (to - 8);
            *list++ = (R_PROM << 12) | (to << 6) | (to - 8);
            *list++ = (B_PROM << 12) | (to << 6) | (to - 8);
            *list++ = (N_PROM << 12) | (to << 6) | (to - 8);
        }

        bb_moves = ((p->Pawns(WC) & ~FILE_A_BB & ~RANK_7_BB) << 7) & p->cl_bb[BC];
        while (bb_moves) {
            to = BB.PopFirstBit(&bb_moves);
            *list++ = (to << 6) | (to - 7);
        }

        bb_moves = ((p->Pawns(WC) & ~FILE_H_BB & ~RANK_7_BB) << 9) & p->cl_bb[BC];
        while (bb_moves) {
            to = BB.PopFirstBit(&bb_moves);
            *list++ = (to << 6) | (to - 9);
        }

        if ((to = p->ep_sq) != NO_SQ) {
            if (((p->Pawns(WC) & ~FILE_A_BB) << 7) & SqBb(to))
                *list++ = (EP_CAP << 12) | (to << 6) | (to - 7);
            if (((p->Pawns(WC) & ~FILE_H_BB) << 9) & SqBb(to))
                *list++ = (EP_CAP << 12) | (to << 6) | (to - 9);
        }
    } else {
        bb_moves = ((p->Pawns(BC) & ~FILE_A_BB & RANK_2_BB) >> 9) & p->cl_bb[WC];
        while (bb_moves) {
            to = BB.PopFirstBit(&bb_moves);
            *list++ = (Q_PROM << 12) | (to << 6) | (to + 9);
            *list++ = (R_PROM << 12) | (to << 6) | (to + 9);
            *list++ = (B_PROM << 12) | (to << 6) | (to + 9);
            *list++ = (N_PROM << 12) | (to << 6) | (to + 9);
        }

        bb_moves = ((p->Pawns(BC) & ~FILE_H_BB & RANK_2_BB) >> 7) & p->cl_bb[WC];
        while (bb_moves) {
            to = BB.PopFirstBit(&bb_moves);
            *list++ = (Q_PROM << 12) | (to << 6) | (to + 7);
            *list++ = (R_PROM << 12) | (to << 6) | (to + 7);
            *list++ = (B_PROM << 12) | (to << 6) | (to + 7);
            *list++ = (N_PROM << 12) | (to << 6) | (to + 7);
        }

        bb_moves = ((p->Pawns(BC) & RANK_2_BB) >> 8) & p->UnoccBb();
        while (bb_moves) {
            to = BB.PopFirstBit(&bb_moves);
            *list++ = (Q_PROM << 12) | (to << 6) | (to + 8);
            *list++ = (R_PROM << 12) | (to << 6) | (to + 8);
            *list++ = (B_PROM << 12) | (to << 6) | (to + 8);
            *list++ = (N_PROM << 12) | (to << 6) | (to + 8);
        }

        bb_moves = ((p->Pawns(BC) & ~FILE_A_BB & ~RANK_2_BB) >> 9) & p->cl_bb[WC];
        while (bb_moves) {
            to = BB.PopFirstBit(&bb_moves);
            *list++ = (to << 6) | (to + 9);
        }

        bb_moves = ((p->Pawns(BC) & ~FILE_H_BB & ~RANK_2_BB) >> 7) & p->cl_bb[WC];
        while (bb_moves) {
            to = BB.PopFirstBit(&bb_moves);
            *list++ = (to << 6) | (to + 7);
        }

        if ((to = p->ep_sq) != NO_SQ) {
            if (((p->Pawns(BC) & ~FILE_A_BB) >> 9) & SqBb(to))
                *list++ = (EP_CAP << 12) | (to << 6) | (to + 9);
            if (((p->Pawns(BC) & ~FILE_H_BB) >> 7) & SqBb(to))
                *list++ = (EP_CAP << 12) | (to << 6) | (to + 7);
        }
    }

    // KNIGHT

    bb_pieces = p->Knights(sd);
    while (bb_pieces) {
        from = BB.PopFirstBit(&bb_pieces);
        bb_moves = BB.KnightAttacks(from) & p->cl_bb[Opp(sd)];
        while (bb_moves) {
            to = BB.PopFirstBit(&bb_moves);
            *list++ = (to << 6) | from;
        }
    }

    // BISHOP

    bb_pieces = p->Bishops(sd);
    while (bb_pieces) {
        from = BB.PopFirstBit(&bb_pieces);
        bb_moves = BB.BishAttacks(p->OccBb(), from) & p->cl_bb[op];
        while (bb_moves) {
            to = BB.PopFirstBit(&bb_moves);
            *list++ = (to << 6) | from;
        }
    }

    // ROOK

    bb_pieces = p->Rooks(sd);
    while (bb_pieces) {
        from = BB.PopFirstBit(&bb_pieces);
        bb_moves = BB.RookAttacks(p->OccBb(), from) & p->cl_bb[op];
        while (bb_moves) {
            to = BB.PopFirstBit(&bb_moves);
            *list++ = (to << 6) | from;
        }
    }

    // QUEEN

    bb_pieces = p->Queens(sd);
    while (bb_pieces) {
        from = BB.PopFirstBit(&bb_pieces);
        bb_moves = BB.QueenAttacks(p->OccBb(), from) & p->cl_bb[op];
        while (bb_moves) {
            to = BB.PopFirstBit(&bb_moves);
            *list++ = (to << 6) | from;
        }
    }

    // KING

    bb_moves = BB.KingAttacks(p->KingSq(sd)) & p->cl_bb[op];
    while (bb_moves) {
        to = BB.PopFirstBit(&bb_moves);
        *list++ = (to << 6) | p->KingSq(sd);
    }
    return list;
}

int *GenerateQuiet(POS *p, int *list) {

    U64 bb_pieces, bb_moves;
    int sd, from, to;

    sd = p->side;
    if (sd == WC) {
        if ((p->c_flags & W_KS) && !(p->OccBb() & (U64)0x0000000000000060))
            if (!p->Attacked(E1, BC) && !p->Attacked(F1, BC))
                *list++ = (CASTLE << 12) | (G1 << 6) | E1;
        if ((p->c_flags & W_QS) && !(p->OccBb() & (U64)0x000000000000000E))
            if (!p->Attacked(E1, BC) && !p->Attacked(D1, BC))
                *list++ = (CASTLE << 12) | (C1 << 6) | E1;

        bb_moves = ((((p->Pawns(WC) & RANK_2_BB) << 8) & p->UnoccBb()) << 8) & p->UnoccBb();
        while (bb_moves) {
            to = BB.PopFirstBit(&bb_moves);
            *list++ = (EP_SET << 12) | (to << 6) | (to - 16);
        }

        bb_moves = ((p->Pawns(WC) & ~RANK_7_BB) << 8) & p->UnoccBb();
        while (bb_moves) {
            to = BB.PopFirstBit(&bb_moves);
            *list++ = (to << 6) | (to - 8);
        }
    } else {
        if ((p->c_flags & B_KS) && !(p->OccBb() & (U64)0x6000000000000000))
            if (!p->Attacked(E8, WC) && !p->Attacked(F8, WC))
                *list++ = (CASTLE << 12) | (G8 << 6) | E8;
        if ((p->c_flags & B_QS) && !(p->OccBb() & (U64)0x0E00000000000000))
            if (!p->Attacked(E8, WC) && !p->Attacked(D8, WC))
                *list++ = (CASTLE << 12) | (C8 << 6) | E8;

        bb_moves = ((((p->Pawns(BC) & RANK_7_BB) >> 8) & p->UnoccBb()) >> 8) & p->UnoccBb();
        while (bb_moves) {
            to = BB.PopFirstBit(&bb_moves);
            *list++ = (EP_SET << 12) | (to << 6) | (to + 16);
        }

        bb_moves = ((p->Pawns(BC) & ~RANK_2_BB) >> 8) & p->UnoccBb();
        while (bb_moves) {
            to = BB.PopFirstBit(&bb_moves);
            *list++ = (to << 6) | (to + 8);
        }
    }

    // KNIGHT

    bb_pieces = p->Knights(sd);
    while (bb_pieces) {
        from = BB.PopFirstBit(&bb_pieces);
        bb_moves = BB.KnightAttacks(from) & p->UnoccBb();
        while (bb_moves) {
            to = BB.PopFirstBit(&bb_moves);
            *list++ = (to << 6) | from;
        }
    }

    // BISHOP

    bb_pieces = p->Bishops(sd);
    while (bb_pieces) {
        from = BB.PopFirstBit(&bb_pieces);
        bb_moves = BB.BishAttacks(p->OccBb(), from) & p->UnoccBb();
        while (bb_moves) {
            to = BB.PopFirstBit(&bb_moves);
            *list++ = (to << 6) | from;
        }
    }

    // ROOK

    bb_pieces = p->Rooks(sd);
    while (bb_pieces) {
        from = BB.PopFirstBit(&bb_pieces);
        bb_moves = BB.RookAttacks(p->OccBb(), from) & p->UnoccBb();
        while (bb_moves) {
            to = BB.PopFirstBit(&bb_moves);
            *list++ = (to << 6) | from;
        }
    }

    // QUEEN

    bb_pieces = p->Queens(sd);
    while (bb_pieces) {
        from = BB.PopFirstBit(&bb_pieces);
        bb_moves = BB.QueenAttacks(p->OccBb(), from) & p->UnoccBb();
        while (bb_moves) {
            to = BB.PopFirstBit(&bb_moves);
            *list++ = (to << 6) | from;
        }
    }

    // KING

    bb_moves = BB.KingAttacks(p->KingSq(sd)) & p->UnoccBb();
    while (bb_moves) {
        to = BB.PopFirstBit(&bb_moves);
        *list++ = (to << 6) | p->KingSq(sd);
    }
    return list;
}

int *GenerateSpecial(POS *p, int *list) {

    U64 bb_pieces, bb_moves;
    int from, to;
    int sd = p->side;
    int op = Opp(sd);

    // squares from which normal (non-discovered) checks are possible

    int king_sq = p->KingSq(op);
    U64 n_check = BB.KnightAttacks(king_sq);
    U64 r_check = BB.RookAttacks(p->OccBb(), king_sq);
    U64 b_check = BB.BishAttacks(p->OccBb(), king_sq);
    U64 p_check = BB.ShiftFwd(BB.ShiftSideways(SqBb(king_sq)), op);

    // TODO: discovered checks by a pawn

    if (sd == WC) {

        bb_moves = ((((p->Pawns(WC) & RANK_2_BB) << 8) & p->UnoccBb()) << 8) & p->UnoccBb();
        bb_moves = bb_moves & p_check;
        while (bb_moves) {
            to = BB.PopFirstBit(&bb_moves);
            *list++ = (EP_SET << 12) | (to << 6) | (to - 16);
        }

        bb_moves = ((p->Pawns(WC) & ~RANK_7_BB) << 8) & p->UnoccBb();
        bb_moves = bb_moves & p_check;
        while (bb_moves) {
            to = BB.PopFirstBit(&bb_moves);
            *list++ = (to << 6) | (to - 8);
        }
    } else {
        bb_moves = ((((p->Pawns(BC) & RANK_7_BB) >> 8) & p->UnoccBb()) >> 8) & p->UnoccBb();
        bb_moves = bb_moves & p_check;
        while (bb_moves) {
            to = BB.PopFirstBit(&bb_moves);
            *list++ = (EP_SET << 12) | (to << 6) | (to + 16);
        }

        bb_moves = ((p->Pawns(BC) & ~RANK_2_BB) >> 8) & p->UnoccBb();
        bb_moves = bb_moves & p_check;
        while (bb_moves) {
            to = BB.PopFirstBit(&bb_moves);
            *list++ = (to << 6) | (to + 8);
        }
    }

    // KNIGHT

    bb_pieces = p->Knights(sd);
    while (bb_pieces) {
        from = BB.PopFirstBit(&bb_pieces);

        // are discovered checks possible?

        U64 bb_checkers = p->Queens(sd) | p->Rooks(sd) | p->Bishops(sd);
        bool knight_discovers = CanDiscoverCheck(p, bb_checkers, op, from);

        bb_moves = BB.KnightAttacks(from) & p->UnoccBb();
        if (!knight_discovers) bb_moves = bb_moves & n_check;
        while (bb_moves) {
            to = BB.PopFirstBit(&bb_moves);
            *list++ = (to << 6) | from;
        }
    }

    // BISHOP

    bb_pieces = p->Bishops(sd);
    while (bb_pieces) {
        from = BB.PopFirstBit(&bb_pieces);

        // are straight discovered checks possible?

        bool bish_discovers = CanDiscoverCheck(p, p->StraightMovers(sd), op, from);

        bb_moves = BB.BishAttacks(p->OccBb(), from) & p->UnoccBb();
        if (!bish_discovers) bb_moves = bb_moves & b_check;
        while (bb_moves) {
            to = BB.PopFirstBit(&bb_moves);
            *list++ = (to << 6) | from;
        }
    }

    // ROOK

    bb_pieces = p->Rooks(sd);
    while (bb_pieces) {
        from = BB.PopFirstBit(&bb_pieces);

        // are diagonal discovered checks possible?

        bool rook_discovers = CanDiscoverCheck(p, p->DiagMovers(sd), op, from);

        bb_moves = BB.RookAttacks(p->OccBb(), from) & p->UnoccBb();
        if (!rook_discovers) bb_moves = bb_moves & r_check;
        while (bb_moves) {
            to = BB.PopFirstBit(&bb_moves);
            *list++ = (to << 6) | from;
        }
    }

    // QUEEN

    bb_pieces = p->Queens(sd);
    while (bb_pieces) {
        from = BB.PopFirstBit(&bb_pieces);
        bb_moves = BB.QueenAttacks(p->OccBb(), from) & p->UnoccBb();
        bb_moves = bb_moves & (r_check | b_check);
        while (bb_moves) {
            to = BB.PopFirstBit(&bb_moves);
            *list++ = (to << 6) | from;
        }
    }

    /*

    // TODO: discovered checks by a king

    moves = k_attacks[p->KingSq(sd)] & p->UnoccBb();
    while (moves) {
      to = BB.PopFirstBit(&moves);
      *list++ = (to << 6) | p->KingSq(sd);
    }
    */
    return list;
}

bool CanDiscoverCheck(POS *p, U64 bb_checkers, int op, int from) {

    while (bb_checkers) {
        int checker = BB.PopFirstBit(&bb_checkers);
        U64 bb_ray = BB.bbBetween[checker][p->king_sq[op]];

        if (SqBb(from) & bb_ray) {
            if (BB.PopCnt(bb_ray & p->OccBb()) == 1)
                return true;
        }
    }

    return false;
}
