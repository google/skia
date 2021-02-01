/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkConvertPixels_DEFINED
#define SkConvertPixels_DEFINED

#include "include/core/SkImageInfo.h"
#include "include/private/SkTemplates.h"

class SkColorTable;

bool SK_WARN_UNUSED_RESULT SkConvertPixels(
        const SkImageInfo& dstInfo,       void* dstPixels, size_t dstRowBytes,
        const SkImageInfo& srcInfo, const void* srcPixels, size_t srcRowBytes);

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
