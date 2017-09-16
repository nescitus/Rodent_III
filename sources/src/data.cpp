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

//int max_depth_completed;
int POS::castle_mask[64];

const int tp_value[7] = { 100, 325, 325, 500, 1000,  0,   0 };
const int ph_value[7] = {   0,   1,   1,   2,    4,  0,   0 };

U64 POS::zob_piece[12][64];
U64 POS::zob_castle[16];
U64 POS::zob_ep[8];
int cEngine::move_time;
int cEngine::move_nodes;
int cEngine::search_depth;
int start_time;
ENTRY *tt;
unsigned int tt_size;
unsigned int tt_mask;
int tt_date;
