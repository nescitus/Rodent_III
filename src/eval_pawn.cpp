#include "skeleton.h"

static const U64 bbKingSide = FILE_F_BB | FILE_G_BB | FILE_H_BB;
static const U64 bbQueenSide = FILE_A_BB | FILE_B_BB | FILE_C_BB;

static const int empty_ks[64] = {  
   0,   0,   0,   0,   0, -10, -20, -30,
   0,   0,   0,   0,   0, -10, -20, -30,
   0,   0,   0,   0,   0, -10, -20, -30,
   0,   0,   0,   0,   0, -10, -20, -30,
   0,   0,   0,   0,   0, -10, -20, -30,
   0,   0,   0,   0,   0, -10, -20, -30,
   0,   0,   0,   0,   0, -10, -20, -30,
   0,   0,   0,   0,   0, -10, -20, -30};

static const int empty_qs[64] = {
 -30, -20, -10,   0,   0,   0,   0,   0,
 -30, -20, -10,   0,   0,   0,   0,   0,
 -30, -20, -10,   0,   0,   0,   0,   0,
 -30, -20, -10,   0,   0,   0,   0,   0,
 -30, -20, -10,   0,   0,   0,   0,   0,
 -30, -20, -10,   0,   0,   0,   0,   0,
 -30, -20, -10,   0,   0,   0,   0,   0,
 -30, -20, -10,   0,   0,   0,   0,   0};

void cEngine::ScorePawnStruct(POS * p, eData * e) {

  // Try to retrieve score from pawn hashtable

  int addr = p->pawn_key % PAWN_HASH_SIZE;

  if (PawnTT[addr].key == p->pawn_key) {
    e->mg_pawns[WC] = PawnTT[addr].mg_pawns;
    e->eg_pawns[WC] = PawnTT[addr].eg_pawns;
	e->mg_pawns[BC] = 0;
	e->eg_pawns[BC] = 0;
    return;
  }

  e->mg_pawns[WC] = 0;
  e->mg_pawns[BC] = 0;
  e->eg_pawns[WC] = 0;
  e->eg_pawns[BC] = 0;

  EvaluatePawns(p, e, WC);
  EvaluatePawns(p, e, BC);
  EvaluateKing(p, e, WC);
  EvaluateKing(p, e, BC);
  ScoreChains(p, e);

  // King on a wing without pawns

  U64 bbAllPawns = p->Pawns(WC) | p->Pawns(BC);

  if (bbAllPawns) {
    if (!(bbAllPawns & bbKingSide)) {
      AddPawns(e, WC, empty_ks[p->king_sq[WC]], empty_ks[p->king_sq[WC]]);
      AddPawns(e, BC, empty_ks[p->king_sq[BC]], empty_ks[p->king_sq[BC]]);
    }

    if (!(bbAllPawns & bbQueenSide)) {
      AddPawns(e, WC, empty_qs[p->king_sq[WC]], empty_qs[p->king_sq[WC]]);
      AddPawns(e, BC, empty_qs[p->king_sq[BC]], empty_qs[p->king_sq[BC]]);
    }
  }

  // Save stuff in pawn hashtable

  PawnTT[addr].key = p->pawn_key;
  PawnTT[addr].mg_pawns = e->mg_pawns[WC] - e->mg_pawns[BC];
  PawnTT[addr].eg_pawns = e->eg_pawns[WC] - e->eg_pawns[BC];

}

void cEngine::EvaluateKing(POS *p, eData *e, int sd) {

  const int qCastle[2] = { B1, B8 };
  const int kCastle[2] = { G1, G8 };
  U64 king_file, next_file;
  int shield = 0;
  int storm = 0;
  int sq = KingSq(p, sd);

  // Normalize king square for pawn shield evaluation,
  // to discourage shuffling the king between g1 and h1.

  if (SqBb(sq) & Mask.ks_castle[sd]) sq = kCastle[sd];
  if (SqBb(sq) & Mask.qs_castle[sd]) sq = qCastle[sd];

  // Evaluate shielding and storming pawns on each file.

  king_file = BB.FillNorth(SqBb(sq)) | BB.FillSouth(SqBb(sq));
  ScoreKingFile(p, sd, king_file, &shield, &storm);

  next_file = ShiftEast(king_file);
  if (next_file) ScoreKingFile(p, sd, next_file, &shield, &storm);

  next_file = ShiftWest(king_file);
  if (next_file) ScoreKingFile(p, sd, next_file, &shield, &storm);

  AddPawns(e, sd, shield + storm, 0);
}

void cEngine::ScoreKingFile(POS * p, int sd, U64 bbFile, int *shield, int *storm) {

  int shelter = ScoreFileShelter(bbFile &  p->Pawns(sd), sd);
  if (p->Kings(sd) & bbFile) shelter = ((shelter * 120) / 100);
  //if (bbFile & bbCentralFile) shelter /= 2;
  *shield += shelter;
  *storm += ScoreFileStorm(bbFile & p->Pawns(Opp(sd)), sd);
}

int cEngine::ScoreFileShelter(U64 bbOwnPawns, int sd) {

  if (!bbOwnPawns) return -36;
  if (bbOwnPawns & bbRelRank[sd][RANK_2]) return    2;
  if (bbOwnPawns & bbRelRank[sd][RANK_3]) return  -11;
  if (bbOwnPawns & bbRelRank[sd][RANK_4]) return  -20;
  if (bbOwnPawns & bbRelRank[sd][RANK_5]) return  -27;
  if (bbOwnPawns & bbRelRank[sd][RANK_6]) return  -32;
  if (bbOwnPawns & bbRelRank[sd][RANK_7]) return  -35;
	return 0;
}

int cEngine::ScoreFileStorm(U64 bbOppPawns, int sd) {

  if (!bbOppPawns) return -16;
  if (bbOppPawns & bbRelRank[sd][RANK_3]) return -32;
  if (bbOppPawns & bbRelRank[sd][RANK_4]) return -16;
  if (bbOppPawns & bbRelRank[sd][RANK_5]) return -8;
	return 0;
}

void cEngine::ScoreChains(POS *p, eData *e) {
  int tmp;

  static const int c1 = 13;
  static const int c2 = 5;
  static const int c3 = 4;
  static const int c4 = 12;
  static const int c5 = 4;
  static const int nc = 10;

  /*
  if (p->Kings(BC) & Mask.ks_castle[BC]) {
	  if (IsOnSq(p, WC, P, D4) && IsOnSq(p, WC, P, E5)) {
		  if (IsOnSq(p, BC, P, D5) && IsOnSq(p, BC, P, E6)) {
			  tmp = c1;
			  AddPawns(e, WC, tmp, 0);
		  }
	  }
  }

  if (p->Kings(BC) & Mask.qs_castle[BC]) {
	  if (IsOnSq(p, WC, P, E4) && IsOnSq(p, WC, P, D5)) {
		  if (IsOnSq(p, BC, P, E5) && IsOnSq(p, BC, P, D6)) {
			  tmp = c1;
			  AddPawns(e, WC, tmp, 0);
		  }
	  }
  }

  if (p->Kings(WC) & Mask.ks_castle[WC]) {
	  if (IsOnSq(p, BC, P, D5) && IsOnSq(p, BC, P, E4)) {
		  if (IsOnSq(p, WC, P, D4) && IsOnSq(p, WC, P, E3)) {
			  tmp = c1;
			  AddPawns(e, BC, tmp, 0);
		  }
	  }
  }

  if (p->Kings(WC) & Mask.qs_castle[WC]) {
	  if (IsOnSq(p, BC, P, E5) && IsOnSq(p, BC, P, D4)) {
		  if (IsOnSq(p, WC, P, E4) && IsOnSq(p, WC, P, D3)) {
			  tmp = c1;
			  AddPawns(e, BC, tmp, 0);
		  }
	  }
  }
  */

  if (p->Kings(WC) & Mask.ks_castle[WC]) {
    if (IsOnSq(p, BC, P, D6) && IsOnSq(p, BC, P, E5) && IsOnSq(p, BC, P, F4)) {
		if (IsOnSq(p, WC, P, D5) && IsOnSq(p, WC, P, F3)) {
		   tmp = c1;
		   if (IsOnSq(p, WC, P, E4)) tmp += c2;
		   if (IsOnSq(p, BC, P, G5)) tmp += c3;
		   if (IsOnSq(p, BC, P, G4)) tmp += c4;
		   if (IsOnSq(p, BC, P, H5)) tmp += c5;
		   if (IsOnSq(p, BC, P, H4)) tmp = -nc; // wrong treatment of a chain
		   AddPawns(e, BC, tmp, 0);
		}
    }
  }

  if (p->Kings(BC) & Mask.ks_castle[BC]) {
    if (IsOnSq(p, WC, P, D3) && IsOnSq(p, WC, P, E4) && IsOnSq(p, WC, P, F5)) {
		if (IsOnSq(p, BC, P, D4) && IsOnSq(p, BC, P, F6)) {
		   tmp = c1;
		   if (IsOnSq(p, BC, P, E5)) tmp += c2;
		   if (IsOnSq(p, WC, P, G4)) tmp += c3;
		   if (IsOnSq(p, WC, P, G5)) tmp += c4;
		   if (IsOnSq(p, WC, P, H4)) tmp += c5;
		   if (IsOnSq(p, WC, P, H5)) tmp = -nc; // wrong treatment of a chain
		   AddPawns(e, WC, tmp, 0);
		}
    }
  }
}