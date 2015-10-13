
#include "foo.h"
#include "bar.h"

int main(int, char **)
{
  Foo f;
  Bar b;
  return f.foo() + b.bar();
}
