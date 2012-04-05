#include <stdio.h>
#include <stdint.h>
#include "forrest.h"

int main(int argc, char **argv) {
  FORREST *f;
  f = forrest_alloc();

  forrest_rm_tree(f);

  forrest_cleanup(f);
  return 0;
}
