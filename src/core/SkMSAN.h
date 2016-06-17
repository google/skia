/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkMSAN_DEFINED
#define SkMSAN_DEFINED

#include "SkTypes.h"

// Typically declared in LLVM's msan_interface.h.  Easier for us to just re-declare.
extern "C" {
    void __msan_check_mem_is_initialized(const volatile void*, size_t);
    void __msan_unpoison                (const volatile void*, size_t);
}

// Code that requires initialized inputs can call this to make it clear that
// the blame for use of uninitialized data belongs further up the call stack.
static inline void sk_msan_assert_initialized(const void* begin, const void* end) {
#if defined(__has_feature)
    #if __has_feature(memory_sanitizer)
        __msan_check_mem_is_initialized(begin, (const char*)end - (const char*)begin);
    #endif
#endif
}

// Lie to MSAN that this range of memory is initialized.
// This can hide serious problems if overused.  Every use of this should refer to a bug.
static inline void sk_msan_mark_initialized(const void* begin, const void* end, const char* skbug) {
    SkASSERT(skbug && 0 != strcmp(skbug, ""));
#if defined(__has_feature)
    #if __has_feature(memory_sanitizer)
        __msan_unpoison(begin, (const char*)end - (const char*)begin);
    #endif
#endif
}

#endif//SkMSAN_DEFINED
