/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkAssert_DEFINED
#define SkAssert_DEFINED

// If SkTypes.h is not being used, these are reasonable defaults for the
// SkDebugf and SK_ABORT macros.

#ifndef SkDebugf
    #include <cstdio>
    #define SkDebugf(...) std::fprintf(stderr, __VA_ARGS__)
#endif

#ifndef SK_ABORT
    #include <cstdlib>
    #define SK_ABORT(msg) do { \
            SkDebugf("%s(%d): fatal error: \"%s\"\n", __FILE__, __LINE__, msg); \
            std::abort(); \
        } while (false)
#endif

// SkASSERT, SkASSERTF and SkASSERT_RELEASE can be used as stand alone assertion expressions, e.g.
//    uint32_t foo(int x) {
//        SkASSERT(x > 4);
//        return x - 4;
//    }
// and are also written to be compatible with constexpr functions:
//    constexpr uint32_t foo(int x) {
//        return SkASSERT(x > 4),
//               x - 4;
//    }
#define SkASSERT_RELEASE(cond) \
        static_cast<void>( (cond) ? (void)0 : []{ SK_ABORT("assert(" #cond ")"); }() )

#ifdef SK_DEBUG
    #define SkASSERT(cond) SkASSERT_RELEASE(cond)
    #define SkASSERTF(cond, fmt, ...) static_cast<void>( (cond) ? (void)0 : [&]{ \
                                          SkDebugf(fmt"\n", __VA_ARGS__);        \
                                          SK_ABORT("assert(" #cond ")");         \
                                      }() )
    #define SkDEBUGFAIL(message)        SK_ABORT(message)
    #define SkDEBUGFAILF(fmt, ...)      SkASSERTF(false, fmt, ##__VA_ARGS__)
    #define SkDEBUGCODE(...)            __VA_ARGS__
    #define SkDEBUGF(args       )       SkDebugf args
    #define SkAssertResult(cond)        SkASSERT(cond)
#else
    #define SkASSERT(cond)            static_cast<void>(0)
    #define SkASSERTF(cond, fmt, ...) static_cast<void>(0)
    #define SkDEBUGFAIL(message)
    #define SkDEBUGFAILF(fmt, ...)
    #define SkDEBUGCODE(...)
    #define SkDEBUGF(args)

    // unlike SkASSERT, this guy executes its condition in the non-debug build.
    // The if is present so that this can be used with functions marked SK_WARN_UNUSED_RESULT.
    #define SkAssertResult(cond)         if (cond) {} do {} while(false)
#endif
#endif  // SkAssert_DEFINED
