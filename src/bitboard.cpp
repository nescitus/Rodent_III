#include "rodent.h"

#define USE_MM_POPCNT

#if defined(__GNUC__)

int PopCnt(U64 bb) {
  return __builtin_popcountll(bb);
}

#elif defined(USE_MM_POPCNT) && defined(_M_AMD64)  // 64 bit windows
#include <nmmintrin.h>

int PopCnt(U64 bb) {
  return (int)_mm_popcnt_u64(bb);
}

#else

int PopCnt(U64 bb) // general purpose population count
{
  U64 k1 = (U64)0x5555555555555555;
  U64 k2 = (U64)0x3333333333333333;
  U64 k3 = (U64)0x0F0F0F0F0F0F0F0F;
  U64 k4 = (U64)0x0101010101010101;

  bb -= (bb >> 1) & k1;
  bb = (bb & k2) + ((bb >> 2) & k2);
  bb = (bb + (bb >> 4)) & k3;
  return (bb * k4) >> 56;
}

#endif

int PopFirstBit(U64 * bb) {

  U64 bbLocal = *bb;
  *bb &= (*bb - 1);
  return FirstOne(bbLocal);
}

U64 FillNorth(U64 bb) {
  bb |= bb << 8;
  bb |= bb << 16;
  bb |= bb << 32;
  return bb;
}

U64 FillSouth(U64 bb) {
  bb |= bb >> 8;
  bb |= bb >> 16;
  bb |= bb >> 32;
  return bb;
}

U64 FillNorthExcl(U64 bb) {
  return FillNorth(ShiftNorth(bb));
}

U64 FillSouthExcl(U64 bb) {
  return FillSouth(ShiftSouth(bb));
}

U64 GetFrontSpan(U64 bb, int sd) {

  if (sd == WC) return FillNorthExcl(bb);
  else          return FillSouthExcl(bb);
}

U64 GetWPControl(U64 bb) {
  return (ShiftNE(bb) | ShiftNW(bb));
}

U64 GetBPControl(U64 bb) {
  return (ShiftSE(bb) | ShiftSW(bb));
}

U64 GetDoubleWPControl(U64 bb) {
  return (ShiftNE(bb) & ShiftNW(bb));
}

U64 GetDoubleBPControl(U64 bb) {
  return (ShiftSE(bb) & ShiftSW(bb));
}

U64 ShiftFwd(U64 bb, int sd) {

  if (sd == WC) return ShiftNorth(bb);
  return ShiftSouth(bb);
}

U64 ShiftSideways(U64 bb) {
  return (ShiftWest(bb) | ShiftEast(bb));
}