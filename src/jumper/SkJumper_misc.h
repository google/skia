/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkJumper_misc_DEFINED
#define SkJumper_misc_DEFINED

#include "SkJumper.h"  // for memcpy()

// Miscellany used by SkJumper_stages.cpp and SkJumper_vectors.h.

// Every function in this file should be marked static and inline using SI.
#define SI static inline

template <typename T, typename P>
SI T unaligned_load(const P* p) {  // const void* would work too, but const P* helps ARMv7 codegen.
    T v;
    memcpy(&v, p, sizeof(v));
    return v;
}

template <typename Dst, typename Src>
SI Dst bit_cast(const Src& src) {
    static_assert(sizeof(Dst) == sizeof(Src), "");
    return unaligned_load<Dst>(&src);
}

template <typename Dst, typename Src>
SI Dst widen_cast(const Src& src) {
    static_assert(sizeof(Dst) > sizeof(Src), "");
    Dst dst;
    memcpy(&dst, &src, sizeof(Src));
    return dst;
}

#endif//SkJumper_misc_DEFINED
