/*
 * Copyright 2011 Google Inc.
 * Copyright 2012 Mozilla Foundation
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTypes.h"

#include "mozilla/mozalloc.h"
#include "mozilla/mozalloc_abort.h"
#include "mozilla/mozalloc_oom.h"

void sk_abort_no_print() {
    mozalloc_abort("Abort from sk_abort");
}

void sk_out_of_memory(void) {
    SkDEBUGFAIL("sk_out_of_memory");
    mozalloc_handle_oom(0);
}

void* sk_malloc_throw(size_t size) {
    return sk_malloc_flags(size, SK_MALLOC_THROW);
}

void* sk_realloc_throw(void* addr, size_t size) {
    return moz_xrealloc(addr, size);
}

void sk_free(void* p) {
    free(p);
}

void* sk_malloc_flags(size_t size, unsigned flags) {
    return (flags & SK_MALLOC_THROW) ? moz_xmalloc(size) : malloc(size);
}

void* sk_calloc(size_t size) {
    return calloc(size, 1);
}

void* sk_calloc_throw(size_t size) {
    return moz_xcalloc(size, 1);
}
