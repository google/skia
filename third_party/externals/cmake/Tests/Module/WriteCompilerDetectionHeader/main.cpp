
#include "test_compiler_detection.h"

#define PREFIX TEST
#include "compile_tests.h"

#ifdef TEST_COMPILER_C_STATIC_ASSERT
#error Expect no C features defined
#endif

int main()
{
  return 0;
}
