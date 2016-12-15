#include "rodent.h"

cParam Par;

int main() {

  Init();
  Par.Init();
  InitSearch();
  UciLoop();
  return 0;
}
