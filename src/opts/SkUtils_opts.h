/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkUtils_opts_DEFINED
#define SkUtils_opts_DEFINED

#include <stdint.h>
#include "SkNx.h"

namespace SK_OPTS_NS {

    template <typename T>
    static void memsetT(T buffer[], T value, int count) {
    #if defined(__AVX__)
        static const int N = 32 / sizeof(T);
    #else
        static const int N = 16 / sizeof(T);
    #endif
        while (count >= N) {
            SkNx<N,T>(value).store(buffer);
            buffer += N;
            count  -= N;
        }
        while (count --> 0) {
            *buffer++ = value;
        }
    }

    static void memset16(uint16_t buffer[], uint16_t value, int count) {
        memsetT(buffer, value, count);
    }
    static void memset32(uint32_t buffer[], uint32_t value, int count) {
        memsetT(buffer, value, count);
    }
    static void memset64(uint64_t buffer[], uint64_t value, int count) {
        memsetT(buffer, value, count);
    }

}

#endif//SkUtils_opts_DEFINED
