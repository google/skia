/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTSAN_DEFINED
#define SkTSAN_DEFINED

#ifndef __has_feature
    #define __has_feature(x) 0
#endif

#if __has_feature(thread_sanitizer)
    #define SK_NO_SANITIZE_THREAD __attribute__((no_sanitize("thread")))
#else
    #define SK_NO_SANITIZE_THREAD
#endif

#endif//SkTSAN_DEFINED
