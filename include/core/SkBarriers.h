#ifndef SkBarriers_DEFINED
#define SkBarriers_DEFINED

// This file is not part of the public Skia API.
#include "SkTypes.h"

#if SK_HAS_COMPILER_FEATURE(thread_sanitizer)
    #include "../ports/SkBarriers_tsan.h"
#elif defined(SK_CPU_ARM32) || defined(SK_CPU_ARM64)
    #include "../ports/SkBarriers_arm.h"
#else
    #include "../ports/SkBarriers_x86.h"
#endif

#endif//SkBarriers_DEFINED
