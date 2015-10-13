#include <stdio.h>

extern int foo(void);
extern int bar(void);
extern int foobar(void);
extern int barbar(void);
extern int baz(void);

int main()
{
   printf("foo: %d bar: %d foobar: %d barbar: %d baz: %d\n", foo(), bar(), foobar(), barbar(), baz());
   return 0;
}
