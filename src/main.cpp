#include "rodent.h"

cParam Par;
cEngine Engine1, Engine2;

int main() {
  thread_no = 1;
  Init();
  Par.Default(); // must be called before other initialization routines of Par class
  Par.InitPst();
  Par.Init();
  InitSearch();
  UciLoop();
  return 0;
}
