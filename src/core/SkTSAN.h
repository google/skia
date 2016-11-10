/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTSAN_DEFINED
#define SkTSAN_DEFINED

#if !defined(__has_feature)
    #define __has_feature(x) 0
#endif

#if __has_feature(thread_sanitizer)
    extern "C" void AnnotateBenignRaceSized(const char* file, int line,
                                            const volatile void* ptr, long size,
                                            const char* desc);
#else
    static inline void AnnotateBenignRaceSized(const char* file, int line,
                                               const volatile void* ptr, long size,
                                               const char* desc) {}
#endif


#endif//SkTSAN_DEFINED
