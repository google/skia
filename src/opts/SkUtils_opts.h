/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkUtils_opts_DEFINED
#define SkUtils_opts_DEFINED

#include <stdint.h>
#include "include/private/SkNx.h"

namespace SK_OPTS_NS {

    template <typename T>
    static void memsetT(T buffer[], T value, int count) {
    #if defined(SK_CPU_SSE_LEVEL) && SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_AVX
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

    /*not static*/ inline void memset16(uint16_t buffer[], uint16_t value, int count) {
        memsetT(buffer, value, count);
    }
    /*not static*/ inline void memset32(uint32_t buffer[], uint32_t value, int count) {
        memsetT(buffer, value, count);
    }
    /*not static*/ inline void memset64(uint64_t buffer[], uint64_t value, int count) {
        memsetT(buffer, value, count);
    }

    template <typename T>
    static void rect_memsetT(T buffer[], T value, int count, size_t rowBytes, int height) {
        while (height --> 0) {
            memsetT(buffer, value, count);
            buffer = (T*)((char*)buffer + rowBytes);
        }
    }

    /*not static*/ inline void rect_memset16(uint16_t buffer[], uint16_t value, int count,
                                             size_t rowBytes, int height) {
        rect_memsetT(buffer, value, count, rowBytes, height);
    }
    /*not static*/ inline void rect_memset32(uint32_t buffer[], uint32_t value, int count,
                                             size_t rowBytes, int height) {
        rect_memsetT(buffer, value, count, rowBytes, height);
    }
    /*not static*/ inline void rect_memset64(uint64_t buffer[], uint64_t value, int count,
                                             size_t rowBytes, int height) {
        rect_memsetT(buffer, value, count, rowBytes, height);
    }

}

#endif//SkUtils_opts_DEFINED
