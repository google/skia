/*
 * Copyright 2025 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSpanPriv_DEFINED
#define SkSpanPriv_DEFINED

#include "include/core/SkSpan.h"
#include "include/private/base/SkMalloc.h"

#include <algorithm>

class SkSpanPriv {
public:
    template <typename T> static bool EQ(SkSpan<T> a, SkSpan<T> b) {
        if (a.size() != b.size()) {
            return false;
        }
        if (a.empty()) {
            return true;
        }
        return (a.data() == b.data()) || std::equal(a.begin(), a.end(), b.begin());
    }

    template <typename T> static void Copy(SkSpan<T> dst, SkSpan<const T> src) {
        SkASSERT(dst.size() == src.size());
        sk_careful_memcpy(dst.data(), src.data(), src.size_bytes());
    }
};

#endif
