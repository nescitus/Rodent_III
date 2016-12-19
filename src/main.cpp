#include "rodent.h"

cParam Par;
cEngine Engine1, Engine2;

int main() {
  thread_no = 2;
  Init();
  Par.Init();
  InitSearch();
  UciLoop();
  return 0;
}
