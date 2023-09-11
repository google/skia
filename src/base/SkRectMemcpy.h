/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRectMemcpy_DEFINED
#define SkRectMemcpy_DEFINED

#include "include/private/base/SkAssert.h"
#include "include/private/base/SkTemplates.h"

#include <cstring>

static inline void SkRectMemcpy(void* dst, size_t dstRB, const void* src, size_t srcRB,
                                size_t trimRowBytes, int rowCount) {
    SkASSERT(trimRowBytes <= dstRB);
    SkASSERT(trimRowBytes <= srcRB);
    if (trimRowBytes == dstRB && trimRowBytes == srcRB) {
        memcpy(dst, src, trimRowBytes * rowCount);
        return;
    }

    for (int i = 0; i < rowCount; ++i) {
        memcpy(dst, src, trimRowBytes);
        dst = SkTAddOffset<void>(dst, dstRB);
        src = SkTAddOffset<const void>(src, srcRB);
    }
}

#endif
