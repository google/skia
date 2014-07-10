/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPixelInfo_DEFINED
#define SkPixelInfo_DEFINED

#include "SkImageInfo.h"

struct SkPixelInfo {
    SkColorType fColorType;
    SkAlphaType fAlphaType;
    size_t      fRowBytes;

    static bool CopyPixels(const SkImageInfo& dstInfo, void* dstPixels, size_t dstRowBytes,
                           const SkImageInfo& srcInfo, const void* srcPixels, size_t srcRowBytes,
                           SkColorTable* srcCTable = NULL);
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

#endif
