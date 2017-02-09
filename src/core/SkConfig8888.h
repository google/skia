/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPixelInfo_DEFINED
#define SkPixelInfo_DEFINED

#include "SkImageInfo.h"
#include "SkTemplates.h"

class SkColorTable;

struct SkPixelInfo {
    SkColorType fColorType;
    SkAlphaType fAlphaType;
    size_t      fRowBytes;

    static bool CopyPixels(const SkImageInfo& dstInfo, void* dstPixels, size_t dstRowBytes,
                           const SkImageInfo& srcInfo, const void* srcPixels, size_t srcRowBytes,
                           SkColorTable* srcCTable = nullptr);
};

static inline void SkRectMemcpy(void* dst, size_t dstRB, const void* src, size_t srcRB,
                                size_t bytesPerRow, int rowCount) {
    SkASSERT(bytesPerRow <= dstRB);
    SkASSERT(bytesPerRow <= srcRB);
    if (bytesPerRow == dstRB && bytesPerRow == srcRB) {
        memcpy(dst, src, bytesPerRow * rowCount);
        return;
    }

    for (int i = 0; i < rowCount; ++i) {
        memcpy(dst, src, bytesPerRow);
        dst = SkTAddOffset<void>(dst, dstRB);
        src = SkTAddOffset<const void>(src, srcRB);
    }
}

#endif
