/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPixelInfo_DEFINED
#define SkPixelInfo_DEFINED

#include "SkImageInfo.h"

class SkColorTable;

struct SkPixelInfo {
    SkColorType fColorType;
    SkAlphaType fAlphaType;
    size_t      fRowBytes;

    static bool CopyPixels(const SkImageInfo& dstInfo, void* dstPixels, size_t dstRowBytes,
                           const SkImageInfo& srcInfo, const void* srcPixels, size_t srcRowBytes,
                           SkColorTable* srcCTable = nullptr);
};

struct SkDstPixelInfo : SkPixelInfo {
    void* fPixels;
};

struct SkSrcPixelInfo : SkPixelInfo {
    const void* fPixels;

    // Guaranteed to work even if src.fPixels and dst.fPixels are the same
    // (but not if they overlap partially)
    bool convertPixelsTo(SkDstPixelInfo* dst, int width, int height) const;
};

static inline void SkRectMemcpy(void* dst, size_t dstRB, const void* src, size_t srcRB,
                                size_t bytesPerRow, int rowCount) {
    SkASSERT(bytesPerRow <= srcRB);
    SkASSERT(bytesPerRow <= dstRB);
    for (int i = 0; i < rowCount; ++i) {
        memcpy(dst, src, bytesPerRow);
        dst = (char*)dst + dstRB;
        src = (const char*)src + srcRB;
    }
}

#endif
