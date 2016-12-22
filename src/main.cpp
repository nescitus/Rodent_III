#include "skeleton.h"

cGlobals Glob;
cEngine Engine1;
cEngine Engine2;
cEngine Engine3;
cEngine Engine4;
cBitBoard BB;
cParam Par;
cMask Mask;

int main() {

  BB.Init();
  InitSearch();
  Init();
  Glob.thread_no = 1;
  Par.DefaultWeights(); // must be called before Par.Init()
  Par.Init();
  Mask.Init();
  Engine1.Init(0);
  Engine2.Init(1);
  Engine3.Init(2);
  Engine4.Init(3);
  UciLoop();
  return 0;
}
