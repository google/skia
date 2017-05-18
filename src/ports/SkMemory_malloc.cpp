/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkMalloc.h"

#include "SkTypes.h"

#include <cstdlib>

#define SK_DEBUGFAILF(fmt, ...) \
    SkASSERT((SkDebugf(fmt"\n", __VA_ARGS__), false))

static inline void sk_out_of_memory(size_t size) {
    SK_DEBUGFAILF("sk_out_of_memory (asked for " SK_SIZE_T_SPECIFIER " bytes)",
                  size);
#if defined(IS_FUZZING)
    exit(1);
#else
    abort();
#endif
}

static inline void* throw_on_failure(size_t size, void* p) {
    if (size > 0 && p == nullptr) {
        // If we've got a nullptr here, the only reason we should have failed is running out of RAM.
        sk_out_of_memory(size);
    }
    return p;
}

void sk_abort_no_print() {
#if defined(SK_BUILD_FOR_WIN) && defined(SK_IS_BOT)
    // do not display a system dialog before aborting the process
    _set_abort_behavior(0, _WRITE_ABORT_MSG);
#endif
#if defined(SK_DEBUG) && defined(SK_BUILD_FOR_WIN)
    __debugbreak();
#endif
#if defined(IS_FUZZING)
    exit(1);
#else
    abort();
#endif
}

void sk_out_of_memory(void) {
    SkDEBUGFAIL("sk_out_of_memory");
#if defined(IS_FUZZING)
    exit(1);
#else
    abort();
#endif
}

void* sk_malloc_throw(size_t size) {
    return sk_malloc_flags(size, SK_MALLOC_THROW);
}

void* sk_realloc_throw(void* addr, size_t size) {
    return throw_on_failure(size, realloc(addr, size));
}

void sk_free(void* p) {
    if (p) {
        free(p);
    }
}

void* sk_malloc_flags(size_t size, unsigned flags) {
    void* p = malloc(size);
    if (flags & SK_MALLOC_THROW) {
        return throw_on_failure(size, p);
    } else {
        return p;
    }
}

void* sk_calloc(size_t size) {
    return calloc(size, 1);
}

void* sk_calloc_throw(size_t size) {
    return throw_on_failure(size, sk_calloc(size));
}
