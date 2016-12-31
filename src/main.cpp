#include "rodent.h"

cParam Par;
cEngine Engine1, Engine2;

int main() {
  thread_no = 1;
  Init();
  Par.Default(); // must be called before Par.Init()
  Par.Init();
  InitSearch();
  UciLoop();
  return 0;
}
