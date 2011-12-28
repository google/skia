/* libs/graphics/ports/SkMemory_brew.cpp
 *
 * Copyright 2009, The Android Open Source Project
 * Copyright 2009, Company 100, Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTypes.h"

#ifdef SK_BUILD_FOR_BREW

#include <AEEStdLib.h>

void sk_throw() {
    SkDEBUGFAIL("sk_throw");
    abort();
}

void sk_out_of_memory(void) {
    SkDEBUGFAIL("sk_out_of_memory");
    abort();
}

void* sk_malloc_throw(size_t size) {
    return sk_malloc_flags(size, SK_MALLOC_THROW);
}

void* sk_realloc_throw(void* addr, size_t size) {
    void* p = REALLOC(addr, size | ALLOC_NO_ZMEM);
    if (size == 0) {
        return p;
    }
    if (p == NULL) {
        sk_throw();
    }
    return p;
}

void sk_free(void* p) {
    FREEIF(p);
}

void* sk_malloc_flags(size_t size, unsigned flags) {
    void* p = MALLOC(size | ALLOC_NO_ZMEM);
    if (p == NULL) {
        if (flags & SK_MALLOC_THROW) {
            sk_throw();
        }
    }
    return p;
}

#endif
