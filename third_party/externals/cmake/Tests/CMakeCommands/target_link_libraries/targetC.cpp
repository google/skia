
#include "depG.h"

#include "foo.h"
#include "bar.h"

#ifndef TEST_DEF
#error Expected TEST_DEF definition
#endif

int main(int, char **)
{
  DepG g;

  return g.foo();
}
