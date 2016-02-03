/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkMSAN_DEFINED
#define SkMSAN_DEFINED

#include <stddef.h>  // size_t

// Typically declared in LLVM's msan_interface.h.  Easier for us to just re-declare.
extern "C" {
    void __msan_check_mem_is_initialized(const volatile void*, size_t);
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

#endif//SkMSAN_DEFINED
