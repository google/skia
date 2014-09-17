#include "SkMiniData.h"
#include "Test.h"

DEF_TEST(MiniData, r) {
    static const char* s = "abcdefghijklmnopqrstuvwxyz";

    for (size_t len = 0; len <= 26; len++) {
        SkMiniData md(s, len);
        REPORTER_ASSERT(r, md.len() == len);
        REPORTER_ASSERT(r, 0 == memcmp(md.data(), s, len));

        SkMiniData copy(md);
        REPORTER_ASSERT(r, copy.len() == len);
        REPORTER_ASSERT(r, 0 == memcmp(copy.data(), s, len));
    }
}
