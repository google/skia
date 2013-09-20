#include "Test.h"
#include "TestClassDef.h"

DEF_TEST(memory_calloc, reporter) {
    const size_t kNum = 200;
    char* zeros = (char*)sk_calloc_throw(kNum*sizeof(char));

    for (size_t i = 0; i < kNum; i++) {
        REPORTER_ASSERT(reporter, 0 == zeros[i]);
    }
    sk_free(zeros);
}
