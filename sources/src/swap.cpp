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

int POS::Swap(int from, int to) {

    int side_l, ply, type, score[32];
    U64 attackers, occ, type_bb;

    attackers = AttacksTo(to);
    occ = OccBb();
    score[0] = tp_value[TpOnSq(to)];
    type = TpOnSq(from);
    occ ^= SqBb(from);

    // find all attackers

    attackers |= (BB.BishAttacks(occ, to) & (tp_bb[B] | tp_bb[Q])) |
                 (BB.RookAttacks(occ, to) & (tp_bb[R] | tp_bb[Q]));
    attackers &= occ;

    side_l = ((SqBb(from) & cl_bb[BC]) == 0); // so that we can call Swap() out of turn
    ply = 1;

    // iterate through attackers

    while (attackers & cl_bb[side_l]) {

        // break on king capture

        if (type == K) {
            score[ply++] = INF;
            break;
        }

        score[ply] = -score[ply - 1] + tp_value[type];

        // find next weakest attacker

        for (type = P; type <= K; type++)
            if ((type_bb = PcBb(side_l, type) & attackers))
                break;

        // eliminate it from consideration

        #pragma warning( suppress : 4146 )
        occ ^= type_bb & -type_bb;

        // has new attacker been discovered?

        attackers |= (BB.BishAttacks(occ, to) & (tp_bb[B] | tp_bb[Q])) |
                     (BB.RookAttacks(occ, to) & (tp_bb[R] | tp_bb[Q]));
        attackers &= occ;

        side_l ^= 1;
        ply++;
    }

    // unwind score stack, updating value

    while (--ply)
        score[ply - 1] = -Max(-score[ply - 1], score[ply]);

    return score[0];
}
