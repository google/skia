#include "Test.h"
#include "SkVarAlloc.h"

DEF_TEST(VarAlloc, r) {
    SkVarAlloc va(4/*start allocating at 16B*/);
    char* p = va.alloc(128, SK_MALLOC_THROW);
    sk_bzero(p, 128);  // Just checking this is safe.

#if !defined(SK_BUILD_FOR_ANDROID) && !defined(__UCLIBC__)
    // This method will always return 0 on Android and UCLIBC platforms.
    REPORTER_ASSERT(r, va.approxBytesAllocated() >= 128);
#endif
}
