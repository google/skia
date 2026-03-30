/*
 * Copyright 2017 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkWritePixelsRec_DEFINED
#define SkWritePixelsRec_DEFINED

#include "include/core/SkImageInfo.h"
#include "include/core/SkPixmap.h"

#include <cstddef>

/**
 *  Helper class to package and trim the parameters passed to writePixels()
 */
struct SkWritePixelsRec {
    /**
     *  @param x, y  The offset into the destination. Negative values are supported; the portion
     *               of the rectangle that is "off-screen" (negative x or y) will cause the
     *               corresponding area in the source pixels to be ignored.
     */
    SkWritePixelsRec(const SkImageInfo& info, const void* pixels, size_t rowBytes, int x, int y)
        : fPixels(pixels)
        , fRowBytes(rowBytes)
        , fInfo(info)
        , fX(x)
        , fY(y)
    {}

    SkWritePixelsRec(const SkPixmap& pm, int x, int y)
        : fPixels(pm.addr())
        , fRowBytes(pm.rowBytes())
        , fInfo(pm.info())
        , fX(x)
        , fY(y)
    {}

    const void* fPixels;
    size_t      fRowBytes;
    SkImageInfo fInfo;
    int         fX;
    int         fY;

    /*
     *  On true, may have modified its fields (except fRowBytes) to make it a legal subset
     *  of the specified dst width/height. Negative fX or fY will cause fPixels to be
     *  incremented and fInfo to be reduced to account for the portion that is "off-screen".
     *
     *  On false, leaves self unchanged, but indicates that it does not overlap dst, or
     *  is not valid (e.g. bad fInfo) for writePixels().
     */
    bool trim(int dstWidth, int dstHeight);
};

#endif
